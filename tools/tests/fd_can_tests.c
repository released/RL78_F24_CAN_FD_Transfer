/* Production FD register adapter with mock SFRs; not electrical evidence. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "device_can.c"
volatile test_rules_t CAN_RXRULE_SFR;
volatile test_ram_t CAN_RAM_MSG_SFR;
volatile uint16_t GAFLCFG, CFDGRWC, GSTS, C0STSL, C0ERFLL, RFSTS0;
volatile uint16_t RFCC0, RFPCTR0;
volatile uint8_t P1, GAFLECTRH, RFSTS0L, RFPCTR0L, TMC0, TMSTS0;
#ifdef F24_DEVICE_TRANSFER
volatile test_rxmb_ram_t CAN_RAM_RM_MSG_SFR;
volatile uint16_t RMNB, RMIEC;
volatile uint8_t RCANGRVCMK, RCANGRVCIF;
static volatile uint16_t g_rmnd;
static unsigned int g_vendor_calls;
static int g_vendor_failure, g_tracking, g_overwrite, g_config_failure, g_start_failure;
static uint8_t g_short_length;
/* Model implemented RMND bits and the overwrite check in the actual vendor
 * InHandler. The actual vendor ISR below performs the acknowledgement. */
volatile uint16_t *Model_RmndRegister(void)
{
    g_rmnd &= 1U;
    if (g_tracking && g_overwrite) g_rmnd |= 1U;
    return &g_rmnd;
}
#define R_CAN_ReadRXMBInHandler Model_VendorReadRXMBInHandler
#include "vendor_read_rxmb.inc"
#undef R_CAN_ReadRXMBInHandler
e_can_err_t R_CAN_ReadRXMBInHandler(can_rxbuf_t buffer, st_can_rx_frame_t *frame,
                                   can_length_t *length)
{
    e_can_err_t result;
    ++g_vendor_calls;
    assert(g_rmnd == 0U);
    if (g_vendor_failure) return CAN_ERR_ILLEGAL_STS;
    g_tracking = 1;
    result = Model_VendorReadRXMBInHandler(buffer, frame, length);
    g_tracking = 0;
    if (g_short_length && result == CAN_SUCCESS) *length = g_short_length;
    return result;
}
static void deliver_irq(void)
{
    if (RCANGRVCMK == 0U && RMIEC == 1U && g_rmnd != 0U) {
        RCANGRVCIF = 0U;
        r_can_glb_rxmb_isr();
    }
}
#define TEST_RX_MSG CAN_RAM_RM_MSG_SFR.RxBufMsg[0]
static void receive_pending(void)
{
    g_rmnd = 1U;
    RCANGRVCIF = 1U;
    deliver_irq();
}
#else
#define TEST_RX_MSG CAN_RAM_MSG_SFR.RxFIFOMsg[0]
static void receive_pending(void) { RFSTS0 = 0x0100U; }
#endif
void R_CAN_Create(void)
{
#ifdef F24_DEVICE_TRANSFER
    RCANGRVCMK = 1U;
    RCANGRVCIF = 0U;
    g_rmnd = 0U;
#endif
}
e_can_err_t R_CAN_SetConfig(void)
{
#ifdef F24_DEVICE_TRANSFER
    RMNB = 0x0701U; RMIEC = 1U;
    if (g_config_failure) return CAN_ERR_ILLEGAL_STS;
#endif
    return CAN_SUCCESS;
}
e_can_err_t R_CAN_StartComm(void)
{
#ifdef F24_DEVICE_TRANSFER
    RCANGRVCMK = 0U;
    if (g_start_failure) return CAN_ERR_ILLEGAL_STS;
#endif
    return CAN_SUCCESS;
}
e_can_err_t R_CAN_StopComm(void) { return CAN_SUCCESS; }
uint8_t R_CAN_GetChStatus(void) { return 0x80U; }
uint8_t R_CAN_GetChBusErrFlag(void) { return 0U; }
#ifdef F24_DEVICE_TRANSFER
static void test_irq_queue(void)
{
    can_frame_t frame;
    transfer_can_runtime_t first, last;
    struct { uint32_t before; can_frame_t data; uint32_t after; } guarded;
    unsigned int i, j, calls;
    /* Masked IRQ: main-loop reads cannot poll the hardware as a fallback. */
    RCANGRVCMK = 1U;
    calls = g_vendor_calls;
    TEST_RX_MSG.PTRHH = 0x80U;
    receive_pending();
    assert(Device_CanRead(&frame) == CAN_RTN_BUFFER_EMPTY);
    assert(g_vendor_calls == calls && g_rmnd == 1U && RCANGRVCIF == 1U);
    Device_CanGetRuntimeDiagnostics(&first);
    assert(RCANGRVCMK == 1U && g_rmnd == 1U && RCANGRVCIF == 1U);
    RCANGRVCMK = 0U; deliver_irq();
    assert(Device_CanRead(&frame) == CAN_RTN_OK && g_rmnd == 0U);
    assert(g_vendor_calls == calls + 1U);
    /* IRQ source dispatch with no RXMB0 data must not invoke InHandler. */
    my_can_glb_rxmb_callback(0U);
    assert(g_vendor_calls == calls + 1U);
    /* Repeat beyond 16/32/64, wrap indices, and preserve main TX RAM page. */
    guarded.before=0x12345678UL; guarded.after=0x87654321UL;
    Device_CanGetRuntimeDiagnostics(&first);
    for (i = 0U; i < 512U; ++i) {
        TEST_RX_MSG.PTRHH = (uint8_t)(i % 2U ? 0xF0U : 0x80U);
        TEST_RX_MSG.DF[0] = (uint16_t)i;
        CFDGRWC = CAN_U16_CFDGRWC_PAGE1;
        receive_pending();
        assert(CFDGRWC == (CAN_U16_CFDGRWC_KEY | CAN_U16_CFDGRWC_PAGE1));
        assert(Device_CanRead(&guarded.data) == CAN_RTN_OK);
        assert(guarded.data.DLC == (i % 2U ? 64U : 8U));
        assert(guarded.data.DB[0] == (uint8_t)i && guarded.data.IDL == 0x710U);
        assert(guarded.before == 0x12345678UL && guarded.after == 0x87654321UL);
        assert(RFPCTR0L == 0xA5U);
    }
    Device_CanGetRuntimeDiagnostics(&last);
    assert(last.rx_reads == first.rx_reads + 512U);
    assert(last.rx_irqs == first.rx_irqs + 512U && last.rx_queue_depth == 0U);
    assert(last.last_rx_before == 0U && last.last_rx_after == 0U);
    assert(last.rx_drops == 0U && last.rx_queue_peak == 1U);
    /* Eight usable entries. Drop ninth/newest, never overwrite queued data. */
    TEST_RX_MSG.PTRHH = 0xF0U;
    for (i = 0U; i < 9U; ++i) {
        for (j = 0U; j < 32U; ++j) TEST_RX_MSG.DF[j]=(uint16_t)(i + (j << 8));
        receive_pending();
    }
    Device_CanGetRuntimeDiagnostics(&last);
    assert(last.rx_queue_depth == 8U && last.rx_queue_peak == 8U && last.rx_drops == 1U);
    for (i = 0U; i < 8U; ++i) {
        assert(Device_CanRead(&frame) == CAN_RTN_OK && frame.DLC == 64U);
        for (j = 0U; j < 32U; ++j)
            assert(frame.DB[j*2U] == i && frame.DB[j*2U+1U] == j);
    }
    assert(Device_CanRead(&frame) == CAN_RTN_BUFFER_EMPTY);
    g_vendor_failure=1; receive_pending(); g_vendor_failure=0;
    assert(Device_CanRead(&frame) == CAN_RTN_BUFFER_EMPTY && g_transfer_rx_api_errors == 1U);
    g_overwrite=1; receive_pending(); g_overwrite=0;
    assert(Device_CanRead(&frame) == CAN_RTN_BUFFER_EMPTY && g_transfer_rx_overwrites == 1U);
    g_rmnd=0U;
    g_short_length=63U; receive_pending(); g_short_length=0U;
    assert(Device_CanRead(&frame) == CAN_RTN_STS_ERROR);
    g_short_length=65U; receive_pending(); g_short_length=0U;
    assert(Device_CanRead(&frame) == CAN_RTN_BUFFER_EMPTY && g_transfer_rx_api_errors == 2U);
    assert(Model_VendorReadRXMBInHandler(1U, (st_can_rx_frame_t *)0,
                                       (can_length_t *)0) == CAN_ERR_INVALID_ARG);
    assert(Model_VendorReadRXMBInHandler(0U, (st_can_rx_frame_t *)0,
                                       (can_length_t *)0) == CAN_ERR_INVALID_ARG);
    /* Observation does not acknowledge flags, consume entries or alter TX. */
    receive_pending();
    RCANGRVCIF=1U; g_rmnd=1U; TMC0=1U; TMSTS0=4U;
    C0ERFLL=0x201FU; RFCC0=0U; RFSTS0=1U; RFPCTR0=0x55U;
    Device_CanGetRuntimeDiagnostics(&first);
    Device_CanGetRuntimeDiagnostics(&last);
    assert(RCANGRVCMK == 0U && RCANGRVCIF == 1U && g_rmnd == 1U);
    assert(first.rx_dequeued == last.rx_dequeued && last.rx_queue_depth == 1U);
    assert(TMC0 == 1U && TMSTS0 == 4U && C0ERFLL == 0x201FU);
    assert(RFCC0 == 0U && RFSTS0 == 1U && RFPCTR0 == 0x55U);
    assert(Device_CanTxResult() == CAN_RTN_TX_END && TMSTS0 == 0U);
    Device_CanGetRuntimeDiagnostics(&last);
    assert(last.last_tx_status == 4U && last.rx_irq_enable == 1U);
    g_rmnd=0U; RCANGRVCIF=0U; TMC0=0U;
    assert(Device_CanRead(&frame) == CAN_RTN_OK);
    /* All long-lived counters saturate; stop/reinit discards stale entries. */
    g_transfer_rx_irqs=g_transfer_rx_reads=g_transfer_rx_queued=0xFFFFU;
    g_transfer_rx_dequeued=0xFFFFU;
    receive_pending();
    assert(Device_CanRead(&frame) == CAN_RTN_OK);
    assert(g_transfer_rx_irqs == 0xFFFFU && g_transfer_rx_reads == 0xFFFFU &&
           g_transfer_rx_queued == 0xFFFFU && g_transfer_rx_dequeued == 0xFFFFU);
    for (i=0U; i<8U; ++i) receive_pending();
    g_transfer_rx_drops=0xFFFFU; receive_pending();
    assert(g_transfer_rx_drops == 0xFFFFU);
    g_transfer_rx_overwrites=0xFFFFU; g_overwrite=1; receive_pending(); g_overwrite=0;
    assert(g_transfer_rx_overwrites == 0xFFFFU); g_rmnd=0U;
    g_transfer_rx_api_errors=0xFFFFU; g_vendor_failure=1; receive_pending(); g_vendor_failure=0;
    assert(g_transfer_rx_api_errors == 0xFFFFU);
    Device_CanStop();
    assert(RCANGRVCMK == 1U && !g_ready && Device_CanRead(&frame) == CAN_RTN_BUFFER_EMPTY);
    assert(Device_CanInit(0x710U, 0U) && Device_CanRead(&frame) == CAN_RTN_BUFFER_EMPTY);
    Device_CanGetRuntimeDiagnostics(&last);
    assert(last.rx_irqs == 0U && last.rx_queue_depth == 0U && last.rx_queue_peak == 0U);
    g_config_failure=1;
    assert(!Device_CanInit(0x710U, 0U) && RCANGRVCMK == 1U);
    g_config_failure=0; g_start_failure=1;
    assert(!Device_CanInit(0x710U, 0U) && RCANGRVCMK == 1U);
    g_start_failure=0;
    assert(Device_CanInit(0x710U, 0U) && RCANGRVCMK == 0U);
    puts("Actual vendor RXMB ISR/InHandler:512 receptions,8/64 bytes,queue wrap/full/order,masked IRQ,no polling fallback,overwrite/errors,saturation,stop/reinit PASS");
}
#endif
int main(void)
{
    can_frame_t frame;
    unsigned int i;
    GSTS=1U; C0STSL=1U;
#ifdef F24_DEVICE_TRANSFER
    assert(Device_CanInit(0x710U,0U));
    assert(GAFLCFG==1U && CAN_RXRULE_SFR.RxRule[0].IDL==0x710U);
    assert(CAN_RXRULE_SFR.RxRule[0].ML==0x7FFU && CAN_RXRULE_SFR.RxRule[0].MH==0xC000U);
    assert(CAN_RXRULE_SFR.RxRule[0].P0LH==0x80U && CAN_RXRULE_SFR.RxRule[0].P1L==0U);
    assert(RMIEC==1U && RCANGRVCMK==0U);
    RFPCTR0L=0xA5U;
#else
    assert(Device_CanInit(0x700U,1U));
    assert(CAN_RXRULE_SFR.RxRule[1].IDL==0x720U);
    assert(CAN_RXRULE_SFR.RxRule[1].ML==0x7FFU);
#endif
    TEST_RX_MSG.IDL=0x720U;
    TEST_RX_MSG.PTRHH=0xF0U;
    TEST_RX_MSG.FDCSTSLL=6U;
    for(i=0U;i<32U;++i) TEST_RX_MSG.DF[i]=(uint16_t)(i*2U+((i*2U+1U)<<8));
    receive_pending();
    assert(Device_CanRead(&frame)==CAN_RTN_OK && frame.DLC==64U);
    for(i=0U;i<64U;++i) assert(frame.DB[i]==i);
#ifdef F24_DEVICE_TRANSFER
    assert(g_vendor_calls==1U && g_rmnd==0U && RFPCTR0L==0xA5U);
    assert(Device_CanRead(&frame)==CAN_RTN_BUFFER_EMPTY);
#else
    assert(RFPCTR0L==CAN_U08_RFPCTRL_FIFO_INC);
#endif
    assert(Device_CanStartTx(&frame)==CAN_RTN_OK);
    assert(CAN_RAM_MSG_SFR.TxBufMsg[0].PTRHH==0xF0U);
    assert(CAN_RAM_MSG_SFR.TxBufMsg[0].FDCSTSLL==6U);
    assert(CAN_RAM_MSG_SFR.TxBufMsg[0].DF[31]==0x3F3EU);
    TMC0=0U;
    for(i=0U;i<16U;++i) {
        TEST_RX_MSG.PTRHH=(uint8_t)(i<<4);
        receive_pending();
        assert((Device_CanRead(&frame)==CAN_RTN_OK)==(i<=8U || i==15U));
    }
    TEST_RX_MSG.PTRHH=0x80U;
    for(i=0U;i<16U;++i) {
        TEST_RX_MSG.FDCSTSLL=(uint8_t)i;
        receive_pending();
        assert((Device_CanRead(&frame)==CAN_RTN_OK)==((i&6U)==6U));
    }
    TEST_RX_MSG.FDCSTSLL=6U;
    TEST_RX_MSG.IDH=0x4000U; receive_pending();
    assert(Device_CanRead(&frame)==CAN_RTN_STS_ERROR);
    TEST_RX_MSG.IDH=0x8000U; receive_pending();
    assert(Device_CanRead(&frame)==CAN_RTN_STS_ERROR);
    frame.IDL=0x700U; frame.IDE=frame.RTR=0U;
    for(i=0U;i<=64U;++i) {
        TMC0=0U; frame.DLC=(uint8_t)i;
        assert((Device_CanStartTx(&frame)==CAN_RTN_OK)==(i<=8U || i==64U));
    }
#ifdef F24_DEVICE_TRANSFER
    TEST_RX_MSG.IDL=0x710U; TEST_RX_MSG.IDH=0U;
    g_transfer_rx_peak=0U;
    test_irq_queue();
#endif
    puts("Production FD HAL:64-byte RX/TX,DLC/flags/RTR/IDE/filter guards PASS");
    return 0;
}
