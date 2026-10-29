/* Actual compact Boot/App adapter and block helpers, not physical CAN/RFD. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "can_boot_transport.h"
#define F13_CRC_HOST_TEST
#include "sw_crc32.c"
#include "can_boot_block.inc"
#include "device_app_vectors.h"
#include "device_can.c"
volatile test_rules_t CAN_RXRULE_SFR;
volatile test_ram_t CAN_RAM_MSG_SFR;
volatile test_rxmb_ram_t CAN_RAM_RM_MSG_SFR;
volatile uint16_t GAFLCFG, CFDGRWC, GSTS, C0STSL, C0ERFLL, RFSTS0;
volatile uint16_t RFCC0, RFPCTR0, RMNB, RMIEC;
volatile uint8_t P1, GAFLECTRH, RFSTS0L, RFPCTR0L, TMC0, TMSTS0;
volatile uint8_t RCANGRVCMK, RCANGRVCIF;
static uint16_t g_rmnd;
static unsigned g_access;
static int g_overwrite, g_config_error, g_start_error, g_global_irq = 1;
volatile uint16_t *Model_RmndRegister(void)
{
    g_rmnd &= 1U;
    ++g_access;
    if (g_overwrite && g_access == 3U) g_rmnd = 1U;
    return &g_rmnd;
}
void R_CAN_Create(void) { RCANGRVCMK=1U; RCANGRVCIF=0U; g_rmnd=0U; }
e_can_err_t R_CAN_SetConfig(void) { RMNB=0x0701U; RMIEC=1U; return g_config_error ? CAN_ERR_ILLEGAL_STS : CAN_SUCCESS; }
e_can_err_t R_CAN_StartComm(void) { return g_start_error ? CAN_ERR_ILLEGAL_STS : CAN_SUCCESS; }
e_can_err_t R_CAN_StopComm(void) { return CAN_SUCCESS; }
uint8_t R_CAN_GetChStatus(void) { return 0x80U; }
uint8_t R_CAN_GetChBusErrFlag(void) { return 0U; }
static void dispatch(void)
{
    if (g_global_irq && !RCANGRVCMK && g_rmnd) {
        RCANGRVCIF=0U; g_access=0U;
        Device_CanRxIsr(); g_rmnd &= 1U;
    }
}
static void feed(uint16_t id, uint8_t dlc, uint8_t tag)
{
    unsigned i;
    volatile st_can_msgbuf_sfr_t *msg;
    msg=&CAN_RAM_RM_MSG_SFR.RxBufMsg[0];
    msg->IDL=id; msg->IDH=0U; msg->PTRHH=(uint8_t)(dlc<<4); msg->FDCSTSLL=6U;
    for(i=0U;i<32U;++i) msg->DF[i]=(uint16_t)(tag | (i<<8));
    g_rmnd=1U; RCANGRVCIF=1U; dispatch();
}
static void test_queue(void)
{
    struct { uint32_t before; can_frame_t frame; uint32_t after; } guard;
    unsigned i,j;
    uint8_t expected;
    guard.before=0x12345678UL; guard.after=0x87654321UL;
    GSTS=C0STSL=1U; assert(Device_CanInit(0x700U,1U));
    assert(GAFLCFG==2U && CAN_RXRULE_SFR.RxRule[0].IDL==0x700U &&
           CAN_RXRULE_SFR.RxRule[1].IDL==0x720U);
    for(i=0U;i<2U;++i) assert(CAN_RXRULE_SFR.RxRule[i].P0LH==0x80U &&
        CAN_RXRULE_SFR.RxRule[i].P1L==0U && CAN_RXRULE_SFR.RxRule[i].ML==0x7FFU &&
        CAN_RXRULE_SFR.RxRule[i].MH==0xC000U);
    RFPCTR0L=0x55U; RFCC0=0U;
    /* A masked interrupt cannot be bypassed by a main-loop hardware read. */
    RCANGRVCMK=1U; feed(0x700U,8U,0U);
    assert(Device_CanRead(&guard.frame)==CAN_RTN_BUFFER_EMPTY && g_rmnd==1U);
    RCANGRVCMK=0U; dispatch();
    assert(Device_CanRead(&guard.frame)==CAN_RTN_OK && g_rmnd==0U);
    for(i=0U;i<512U;++i) {
        CFDGRWC=CAN_U16_CFDGRWC_PAGE1;
        feed(0x720U,(uint8_t)(i%2U ? 15U : 8U),(uint8_t)i);
        assert(Device_CanRead(&guard.frame)==CAN_RTN_OK);
        assert(guard.frame.IDL==0x720U && guard.frame.DLC==(i%2U ? 64U : 8U));
        for(j=0U;j<guard.frame.DLC;j+=2U) assert(guard.frame.DB[j]==(uint8_t)i);
        assert(guard.before==0x12345678UL && guard.after==0x87654321UL);
        assert(CFDGRWC==(CAN_U16_CFDGRWC_KEY|CAN_U16_CFDGRWC_PAGE1) && RFPCTR0L==0x55U);
    }
    for(i=0U;i<9U;++i) feed(0x700U,15U,(uint8_t)i);
    assert(g_device_rx_drops==1U && Device_CanRxFaults()==1U);
    for(i=0U;i<8U;++i) {
        assert(Device_CanRead(&guard.frame)==CAN_RTN_OK && guard.frame.DB[0]==i);
        for(j=0U;j<32U;++j) assert(guard.frame.DB[j*2U+1U]==j);
    }
    assert(Device_CanRead(&guard.frame)==CAN_RTN_BUFFER_EMPTY);
    for(i=0U;i<16U;++i) {
        expected=(uint8_t)(i<=8U || i==15U);
        feed(0x700U,(uint8_t)i,0U);
        assert((Device_CanRead(&guard.frame)==CAN_RTN_OK)==expected);
    }
    for(i=0U;i<16U;++i) {
        g_rmnd=1U; g_access=0U;
        CAN_RAM_RM_MSG_SFR.RxBufMsg[0].PTRHH=0x80U;
        CAN_RAM_RM_MSG_SFR.RxBufMsg[0].FDCSTSLL=(uint8_t)i;
        Device_CanRxIsr();
        assert((Device_CanRead(&guard.frame)==CAN_RTN_OK)==((i&6U)==6U));
    }
    CAN_RAM_RM_MSG_SFR.RxBufMsg[0].FDCSTSLL=6U;
    for(i=1U;i<4U;++i) {
        CAN_RAM_RM_MSG_SFR.RxBufMsg[0].IDH=(uint16_t)(i<<14);
        g_rmnd=1U; g_access=0U; Device_CanRxIsr();
        assert(Device_CanRead(&guard.frame)==CAN_RTN_BUFFER_EMPTY);
    }
    g_overwrite=1; feed(0x700U,15U,0U); g_overwrite=0;
    assert(Device_CanRead(&guard.frame)==CAN_RTN_BUFFER_EMPTY && (Device_CanRxFaults()&2U));
    g_device_rx_drops=255U; g_overwrite=1; feed(0x700U,15U,0U); g_overwrite=0;
    assert(g_device_rx_drops==255U); g_rmnd=0U;
    feed(0x700U,8U,0U); Device_CanStop();
    assert(RCANGRVCMK==1U && Device_CanRead(&guard.frame)==CAN_RTN_BUFFER_EMPTY);
    assert(Device_CanInit(0x700U,0U) && GAFLCFG==1U && !Device_CanRxFaults() && !g_device_rx_drops);
    assert(Device_CanRead(&guard.frame)==CAN_RTN_BUFFER_EMPTY);
    g_config_error=1; assert(!Device_CanInit(0x700U,1U) && RCANGRVCMK==1U); g_config_error=0;
    g_start_error=1; assert(!Device_CanInit(0x700U,1U) && RCANGRVCMK==1U); g_start_error=0;
    assert(Device_CanInit(0x700U,1U));
}
static void test_burst(void)
{
    can_frame_t frame;
    unsigned i;
    unsigned long crc;
    unsigned char data[64];
    for(i=0U;i<64U;++i) data[i]=(unsigned char)(i%2U ? i/2U : 0x55U);
    crc=F13_Crc32Bytes(data,64UL);
    /* The whole three-frame burst arrives before main-loop consumption. */
    feed(0x700U,8U,0x0BU); feed(0x720U,15U,0x55U); feed(0x700U,8U,0x0CU);
    assert(Device_CanRead(&frame)==CAN_RTN_OK && frame.DB[0]==0x0BU);
    BlockBegin(0UL,crc,0UL,128UL);
    assert(Device_CanRead(&frame)==CAN_RTN_OK && frame.IDL==0x720U && frame.DLC==64U);
    assert(memcmp(frame.DB,data,64U)==0);
    BlockSegment(0U,frame.DLC,frame.DB);
    assert(Device_CanRead(&frame)==CAN_RTN_OK && frame.DB[0]==0x0CU);
    assert(BlockCheck(0UL,crc)==CAN_BOOT_STATUS_OK);
    BlockSegment(0U,64U,data); assert(BlockCheck(0UL,crc)==CAN_BOOT_STATUS_OK);
    data[0]^=1U; BlockSegment(0U,64U,data);
    assert(BlockCheck(0UL,crc)!=CAN_BOOT_STATUS_OK); data[0]^=1U;
    BlockBegin(0UL,crc,0UL,128UL); assert(BlockCheck(0UL,crc)!=CAN_BOOT_STATUS_OK);
    /* Flash DI does not make a single RXMB store a three-frame burst.
     * Normal host waits for COMMIT ACK. A modeled stray COMMIT alone cannot
     * satisfy a fresh block's data/CRC guards after interrupts resume. */
    g_global_irq=0; feed(0x700U,8U,0x0CU);
    assert(Device_CanRead(&frame)==CAN_RTN_BUFFER_EMPTY);
    g_global_irq=1; dispatch();
    assert(Device_CanRead(&frame)==CAN_RTN_OK && BlockCheck(0UL,crc)!=CAN_BOOT_STATUS_OK);
    assert(!Device_CanRxFaults());
}
static void test_tx_vectors(void)
{
    can_frame_t frame;
    unsigned long table[64],saved;
    unsigned i,j;
    memset(&frame,0,sizeof(frame)); frame.IDL=0x701U;
    for(i=0U;i<64U;++i) frame.DB[i]=(uint8_t)i;
    for(i=0U;i<=64U;++i) {
        TMC0=0U; frame.DLC=(uint8_t)i;
#ifdef F24_DEVICE_BOOT
        assert(Device_CanStartTx(&frame)==CAN_RTN_OK);
        assert(CAN_RAM_MSG_SFR.TxBufMsg[0].PTRHH==0x80U && CAN_RAM_MSG_SFR.TxBufMsg[0].IDL==0x701U);
        assert(CAN_RAM_MSG_SFR.TxBufMsg[0].DF[3]==0x0706U);
#else
        assert((Device_CanStartTx(&frame)==CAN_RTN_OK)==(i<=8U || i==64U));
        if(i==64U) assert(CAN_RAM_MSG_SFR.TxBufMsg[0].PTRHH==0xF0U && CAN_RAM_MSG_SFR.TxBufMsg[0].DF[31]==0x3F3EU);
#endif
    }
    for(i=0U;i<64U;++i) table[i]=((0x2200UL+i*4UL)<<8)|0xECUL;
    assert(Device_AppVectorsValid(table,0x2200UL,0x3000UL));
    for(i=0U;i<64U;++i) {
        saved=table[i];
        for(j=0U;j<3U;++j) {
            table[i]=j==0U ? saved^1UL : (((j==1U ? 0x21FFUL : 0x3000UL)<<8)|0xECUL);
            assert(!Device_AppVectorsValid(table,0x2200UL,0x3000UL));
        }
        table[i]=saved;
    }
    table[63]=(0x2FFFUL<<8)|0xECUL; assert(Device_AppVectorsValid(table,0x2200UL,0x3000UL));
}
int main(void)
{
    assert(sizeof(unsigned long)==4U);
    test_queue(); test_burst(); test_tx_vectors();
    puts("Production compact FD RX ISR:512 receptions,queue/full/order/overwrite/flags/stop,three-frame burst,block guards,TX and64-slot vector validation PASS");
    return 0;
}
