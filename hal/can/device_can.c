/* Project/API reading notes: Device-facing CAN adapter.
 * Return complete frames with their physical channel, preserve bounded receive/diagnostic
 * handling and report real TX completion.
 * Boot/App/Transfer protocol work remains in foreground code.
 */
#include "platform.h"
#include "device_board.h"
#include "r_rscanfd_rl78_if.h"
#include "r_rscanfd_rl78_private.h"
#ifdef F24_DEVICE_TRANSFER
#include "transfer_diagnostics.h"
static volatile uint8_t g_ready;
static uint8_t g_init_stage;
static uint16_t g_init_result;
static volatile uint16_t g_transfer_rx_reads;
static volatile uint16_t g_transfer_rx_overwrites;
static volatile uint16_t g_transfer_rx_before;
static volatile uint16_t g_transfer_rx_after;
static volatile uint16_t g_transfer_rx_irqs;
static volatile uint16_t g_transfer_rx_queued;
static uint16_t g_transfer_rx_dequeued;
static volatile uint16_t g_transfer_rx_drops;
static volatile uint16_t g_transfer_rx_api_errors;
static volatile uint8_t g_transfer_rx_peak;
static volatile uint8_t g_transfer_rx_head;
static volatile uint8_t g_transfer_rx_tail;
static uint8_t g_transfer_tx_status;
typedef struct
{
    st_can_rx_frame_t frame;
    can_length_t length;
} transfer_rx_entry_t;
/* One spare slot separates full from empty. Only the ISR publishes head;
 * only the main loop releases tail. Payload is published before head. */
static volatile transfer_rx_entry_t g_transfer_rx_queue[TRANSFER_CAN_RX_QUEUE_SIZE + 1U];
static st_can_rx_frame_t g_transfer_rx_scratch;
#define SATURATE_INCREMENT(value)                                                                  \
    do                                                                                             \
    {                                                                                              \
        if ((value) != 0xFFFFU)                                                                    \
            ++(value);                                                                             \
    } while (0)
#define RECORD_INIT(stage, result)                                                                 \
    do                                                                                             \
    {                                                                                              \
        g_init_stage = (stage);                                                                    \
        g_init_result = (uint16_t)(result);                                                        \
    } while (0)

static uint8_t rx_queue_next(uint8_t index)
{
    ++index;
    return (index > TRANSFER_CAN_RX_QUEUE_SIZE) ? 0U : index;
}

static uint8_t rx_queue_depth(uint8_t head, uint8_t tail)
{
    return (head >= tail) ? (uint8_t)(head - tail)
                          : (uint8_t)(head + TRANSFER_CAN_RX_QUEUE_SIZE + 1U - tail);
}

void my_can_glb_rxmb_callback(uint16_t sources)
{
    can_length_t length;
    e_can_err_t result;
    uint8_t head, next, depth, index;
    SATURATE_INCREMENT(g_transfer_rx_irqs);
    if (!g_ready || (sources & 1U) == 0U)
    {
        return;
    }
    /* The vendor ISR has already acknowledged RMND. No parsing, UART output,
     * TX wait or FIFO access belongs in this callback. */
    g_transfer_rx_before = RMND;
    result = R_CAN_ReadRXMBInHandler(CAN_RXBUF0, &g_transfer_rx_scratch, &length);
    g_transfer_rx_after = RMND;
    if (result == CAN_ERR_OVERWRITE)
    {
        SATURATE_INCREMENT(g_transfer_rx_overwrites);
        return;
    }
    if (result != CAN_SUCCESS || length > 64U)
    {
        SATURATE_INCREMENT(g_transfer_rx_api_errors);
        return;
    }
    SATURATE_INCREMENT(g_transfer_rx_reads);
    head = g_transfer_rx_head;
    next = rx_queue_next(head);
    if (next == g_transfer_rx_tail)
    {
        SATURATE_INCREMENT(g_transfer_rx_drops);
        return;
    }
    for (index = 0U; index < CAN_RX_HEAD_WORD_NUM; ++index)
    {
        g_transfer_rx_queue[head].frame.Head.Word[index] = g_transfer_rx_scratch.Head.Word[index];
    }
    for (index = 0U; index < length; ++index)
    {
        g_transfer_rx_queue[head].frame.Data.DB[index] = g_transfer_rx_scratch.Data.DB[index];
    }
    g_transfer_rx_queue[head].length = length;
    g_transfer_rx_head = next;
    SATURATE_INCREMENT(g_transfer_rx_queued);
    depth = rx_queue_depth(next, g_transfer_rx_tail);
    if (depth > g_transfer_rx_peak)
    {
        g_transfer_rx_peak = depth;
    }
}

void Device_CanGetDiagnostics(transfer_can_diagnostics_t *diagnostics)
{
    diagnostics->stage = g_init_stage;
    diagnostics->result = g_init_result;
    diagnostics->global_status = GSTS;
    diagnostics->channel_status = C0STSL;
    diagnostics->standby = (uint8_t)((P1 & 4U) != 0U);
}

void Device_CanGetRuntimeDiagnostics(transfer_can_runtime_t *diagnostics)
{
    uint8_t mask;
    /* Briefly mask only RXMB IRQ to snapshot ISR-owned 16-bit counters.
     * Restore the previous mask; do not acknowledge flags or consume data. */
    mask = RCANGRVCMK;
    RCANGRVCMK = 1U;
    diagnostics->rx_buffer_config = RMNB;
    diagnostics->rx_new_data = RMND;
    diagnostics->last_rx_before = g_transfer_rx_before;
    diagnostics->last_rx_after = g_transfer_rx_after;
    diagnostics->rx_reads = g_transfer_rx_reads;
    diagnostics->rx_overwrites = g_transfer_rx_overwrites;
    diagnostics->rx_irqs = g_transfer_rx_irqs;
    diagnostics->rx_queued = g_transfer_rx_queued;
    diagnostics->rx_dequeued = g_transfer_rx_dequeued;
    diagnostics->rx_drops = g_transfer_rx_drops;
    diagnostics->rx_api_errors = g_transfer_rx_api_errors;
    diagnostics->rx_queue_depth = rx_queue_depth(g_transfer_rx_head, g_transfer_rx_tail);
    diagnostics->rx_queue_peak = g_transfer_rx_peak;
    diagnostics->rx_irq_mask = mask;
    diagnostics->rx_irq_pending = RCANGRVCIF;
    diagnostics->rx_irq_enable = RMIEC;
    diagnostics->global_status = GSTS;
    diagnostics->channel_status = C0STSL;
    diagnostics->channel_errors = C0ERFLL;
    diagnostics->ram_window = CFDGRWC;
    diagnostics->tx_control = TMC0;
    diagnostics->tx_status = TMSTS0;
    diagnostics->last_tx_status = g_transfer_tx_status;
    diagnostics->standby = (uint8_t)((P1 & 4U) != 0U);
    RCANGRVCMK = mask;
}

uint16_t Device_CanPollErrors(void)
{
    uint16_t flags;
    /* The low16 register has the same Classical error-bit positions as the
     * protocol trace. Poll both bytes: GetChBusErrFlag reads only bits8..14.
     * Write ones to unobserved flags so a different newly latched flag survives.
     * No driver error ISR reads/clears this register in the polling profile.
     */
    flags = (uint16_t)(C0ERFLL & 0x7FFFU);
    if (flags != 0U)
    {
        C0ERFLL = (uint16_t)(~flags & 0x7FFFU);
    }
    return flags;
}
#else
static volatile uint8_t g_ready;
/* Boot/App own the physical/App table ISR mapping. Keep the vendor ISR off
 * to avoid duplicate vectors. One spare slot gives eight usable entries. */
typedef struct
{
    uint16_t id;
    uint8_t length;
    uint16_t data[32];
} device_rx_entry_t;
static volatile device_rx_entry_t g_rx_queue[9];
static volatile uint8_t g_rx_head, g_rx_tail, g_rx_faults;
/* Saturating discarded-frame observations, available in the linked map. */
volatile uint8_t g_device_rx_drops;

#ifdef __CCRL__
#pragma interrupt Device_CanRxIsr
#endif
/* Capture CAN receive data into the bounded service queues.
 * Parsing and replies run later in foreground context.
 */
void __near Device_CanRxIsr(void)
{
    volatile st_can_msgbuf_sfr_t CAN_NO_BIT_ACCESS *msg;
    uint16_t page;
    uint8_t head, next, length, index, fault;
    if ((RMND & 1U) == 0U)
    {
        return;
    }
    RMND = 0xFFFEU;
    page = CFDGRWC;
    CFDGRWC = CAN_U16_CFDGRWC_PAGE2;
    msg = &CAN_RAM_RM_MSG_SFR.RxBufMsg[0];
    head = g_rx_head;
    next = (head == 8U) ? 0U : (uint8_t)(head + 1U);
    length = (uint8_t)(msg->PTRHH >> CAN_POS_RMPTRHH_RMDLC);
    if (length == 15U)
    {
        length = 64U;
    }
    fault = 0U;
    if (!g_ready || next == g_rx_tail)
    {
        fault = 1U;
    }
    else if (msg->IDH & 0xC000U || (msg->FDCSTSLL & 6U) != 6U || (length > 8U && length != 64U))
    {
        fault = 4U;
    }
    else
    {
        g_rx_queue[head].id = msg->IDL;
        g_rx_queue[head].length = length;
        for (index = 0U; index < (length + 1U) / 2U; ++index)
        {
            g_rx_queue[head].data[index] = msg->DF[index];
        }
        if (RMND & 1U)
        {
            fault = 2U;
        }
        else
        {
            g_rx_head = next;
        }
    }
    CFDGRWC = CAN_U16_CFDGRWC_KEY | page;
    if (fault)
    {
        g_rx_faults |= fault;
        if (g_device_rx_drops != 255U)
        {
            ++g_device_rx_drops;
        }
    }
}

uint8_t Device_CanRxFaults(void)
{
    return g_rx_faults;
}
uint8_t Device_CanRxDrops(void)
{
    return g_device_rx_drops;
}
#define RECORD_INIT(stage, result) ((void)0)
#endif
/* Fixed FD+BRS data-frame subset of the supplied Renesas driver. Keep the
 * generated source intact; access its documented typed message-RAM windows.
 * Accept control0..8 and data64; reject Classical, RTR and non-BRS frames. */
static void add_rule(uint16_t index, uint16_t id, uint16_t mask)
{
    volatile st_can_rxrule_sfr_t CAN_NO_BIT_ACCESS *rule;
    GAFLCFG = index + 1U;
    GAFLECTRH = CAN_U08_GAFLECTRH_AFLDAE_ON;
    CFDGRWC = CAN_U16_CFDGRWC_PAGE0;
    rule = &CAN_RXRULE_SFR.RxRule[index];
    rule->IDL = id;
    rule->IDH = 0U;
    rule->ML = mask;
    rule->MH = CAN_U16_GAFLM0H_GAFLIDEM_ON | CAN_U16_GAFLM0H_GAFLRTRM_ON;
    /* GAFLRMV enables RXMB storage; RMDP=0 selects RXMB0. No FIFO copy. */
    rule->P0LL = 0U;
    rule->P0H = 0U;
    rule->P0LH = CAN_U08_GAFLP00LH_GAFLRMV_ON;
    rule->P1L = 0U;
    GAFLECTRH = 0U;
}
uint8_t Device_CanInit(uint16_t request_id, uint8_t blocks)
{
    e_can_err_t result;
    uint32_t budget;
    g_ready = 0U;
    P1 |= 4U;
#ifndef F24_DEVICE_TRANSFER
    RCANGRVCMK = 1U;
    g_rx_head = 0U;
    g_rx_tail = 0U;
    g_rx_faults = 0U;
    g_device_rx_drops = 0U;
#endif
#ifdef F24_DEVICE_TRANSFER
    RCANGRVCMK = 1U;
    g_transfer_rx_reads = 0U;
    g_transfer_rx_overwrites = 0U;
    g_transfer_rx_before = 0U;
    g_transfer_rx_after = 0U;
    g_transfer_rx_irqs = 0U;
    g_transfer_rx_queued = 0U;
    g_transfer_rx_dequeued = 0U;
    g_transfer_rx_drops = 0U;
    g_transfer_rx_api_errors = 0U;
    g_transfer_rx_peak = 0U;
    g_transfer_rx_head = 0U;
    g_transfer_rx_tail = 0U;
    g_transfer_tx_status = 0U;
#endif
    R_CAN_Create();
    result = R_CAN_SetConfig();
    RECORD_INIT(TRANSFER_CAN_STAGE_CONFIG, result);
    if (result != CAN_SUCCESS)
    {
        return 0U;
    }
    RECORD_INIT(TRANSFER_CAN_STAGE_STATE, CAN_ERR_ILLEGAL_STS);
    if (request_id > 0x7FFU || (GSTS & 7U) != 1U || (C0STSL & 7U) != 1U)
    {
        return 0U;
    }
    add_rule(0U, request_id, 0x7FFU);
    if (blocks)
    {
        add_rule(1U, CAN_BOOT_BLOCK_DATA_ID, 0x7FFU);
    }
    budget = 10000UL;
    do
    {
        result = R_CAN_StartComm();
    } while (result == CAN_ERR_WAITING && --budget);
    /* StartComm enables configured sources. Keep capture off until ready. */
    RCANGRVCMK = 1U;
    RECORD_INIT(TRANSFER_CAN_STAGE_START, result);
    if (result != CAN_SUCCESS)
    {
        return 0U;
    }
    budget = 10000UL;
    RECORD_INIT(TRANSFER_CAN_STAGE_ACTIVE, CAN_ERR_WAITING);
    while ((R_CAN_GetChStatus() & 0x80U) == 0U && --budget)
    {
        BSP_NOP();
    }
    if (!budget)
    {
        return 0U;
    }
    P1 &= 0xFBU;
    g_ready = 1U;
    RCANGRVCMK = 0U;
    RECORD_INIT(TRANSFER_CAN_STAGE_READY, CAN_SUCCESS);
    return 1U;
}
Can_RtnType Device_CanRead(can_frame_t *frame)
{
#ifdef F24_DEVICE_TRANSFER
    st_can_rx_frame_t received = {0};
    can_length_t length;
    uint8_t dlc, index, tail;
    if (!g_ready)
    {
        return CAN_RTN_BUFFER_EMPTY;
    }
    tail = g_transfer_rx_tail;
    if (tail == g_transfer_rx_head)
    {
        return CAN_RTN_BUFFER_EMPTY;
    }
    length = g_transfer_rx_queue[tail].length;
    for (index = 0U; index < CAN_RX_HEAD_WORD_NUM; ++index)
    {
        received.Head.Word[index] = g_transfer_rx_queue[tail].frame.Head.Word[index];
    }
    for (index = 0U; index < length; ++index)
    {
        received.Data.DB[index] = g_transfer_rx_queue[tail].frame.Data.DB[index];
    }
    /* Release only after copying. ISR cannot reuse this slot before tail. */
    g_transfer_rx_tail = rx_queue_next(tail);
    SATURATE_INCREMENT(g_transfer_rx_dequeued);
    dlc = (uint8_t)received.Head.Bits.DLC;
    if (received.Head.Bits.IDE || received.Head.Bits.RTR || !received.Head.Bits.FDF ||
        !received.Head.Bits.BRS || (dlc > 8U && dlc != 15U) || length != (dlc == 15U ? 64U : dlc))
    {
        return CAN_RTN_STS_ERROR;
    }
    frame->IDL = received.Head.Word[0];
    frame->IDH = received.Head.Word[1];
    frame->IDE = (uint8_t)received.Head.Bits.IDE;
    frame->RTR = (uint8_t)received.Head.Bits.RTR;
    frame->DLC = length;
    for (index = 0U; index < length; ++index)
    {
        frame->DB[index] = received.Data.DB[index];
    }
    return CAN_RTN_OK;
#else
    uint16_t data;
    uint8_t tail, index, length;
    if (!g_ready)
    {
        return CAN_RTN_BUFFER_EMPTY;
    }
    tail = g_rx_tail;
    if (tail == g_rx_head)
    {
        return CAN_RTN_BUFFER_EMPTY;
    }
    frame->IDL = g_rx_queue[tail].id;
    frame->IDH = 0U;
    frame->IDE = 0U;
    frame->RTR = 0U;
    length = g_rx_queue[tail].length;
    frame->DLC = length;
    for (index = 0U; index < (length + 1U) / 2U; ++index)
    {
        data = g_rx_queue[tail].data[index];
        frame->DB[index * 2U] = (uint8_t)data;
        frame->DB[index * 2U + 1U] = (uint8_t)(data >> 8);
    }
    g_rx_tail = (tail == 8U) ? 0U : (uint8_t)(tail + 1U);
    return CAN_RTN_OK;
#endif
}
Can_RtnType Device_CanStartTx(const can_frame_t *frame)
{
    volatile st_can_msgbuf_sfr_t CAN_NO_BIT_ACCESS *msg;
    uint16_t page;
    uint8_t index;
#ifdef F24_DEVICE_BOOT
    /* Boot's sole sender builds eight response bytes. Routing/type/length
     * are compile-time constants, not fields copied from a received frame. */
    if (!g_ready)
    {
        return CAN_RTN_STS_ERROR;
    }
#else
    if (!g_ready || (frame->DLC > 8U && frame->DLC != 64U) || frame->IDL > 0x7FFU || frame->IDE ||
        frame->RTR)
    {
        return CAN_RTN_STS_ERROR;
    }
#endif
    if (TMC0 & 1U)
    {
        return CAN_RTN_STS_ERROR;
    }
    TMSTS0 = 0U;
    if (TMSTS0)
    {
        return CAN_RTN_STS_ERROR;
    }
    page = CFDGRWC;
    CFDGRWC = CAN_U16_CFDGRWC_PAGE1;
    msg = &CAN_RAM_MSG_SFR.TxBufMsg[0];
#ifdef F24_DEVICE_BOOT
    msg->IDL = CAN_BOOT_RESPONSE_ID;
    msg->IDH = 0U;
    /* Boot transmits only its fixed eight-byte control/ACK responses. */
    msg->PTRHH = 0x80U;
#else
    msg->IDL = frame->IDL;
    msg->IDH = 0U;
    msg->PTRHH = (uint8_t)((frame->DLC == 64U ? 15U : frame->DLC) << CAN_POS_TMPTRHH_TMDLC);
#endif
    msg->FDCSTSLL = 6U;
    msg->FDCSTSLH = 0U;
    msg->FDCSTSH = 0U;
#ifdef F24_DEVICE_BOOT
    for (index = 0U; index < 4U; ++index)
    {
        msg->DF[index] =
            (uint16_t)frame->DB[index * 2U] | ((uint16_t)frame->DB[index * 2U + 1U] << 8);
    }
#else
    for (index = 0U; index < (frame->DLC + 1U) / 2U; ++index)
    {
        msg->DF[index] =
            (uint16_t)frame->DB[index * 2U] |
            ((index * 2U + 1U < frame->DLC) ? ((uint16_t)frame->DB[index * 2U + 1U] << 8) : 0U);
    }
#endif
    CFDGRWC = CAN_U16_CFDGRWC_KEY | page;
    TMC0 = CAN_U08_TMC_TMTR_ON;
    return CAN_RTN_OK;
}
Can_RtnType Device_CanTxResult(void)
{
    uint8_t result;
#ifdef F24_DEVICE_TRANSFER
    g_transfer_tx_status = TMSTS0;
    result = (uint8_t)((g_transfer_tx_status & CAN_U08_TMSTS_TMTRF_MASK) >> CAN_POS_TMSTS_TMTRF);
#else
    result = (uint8_t)((TMSTS0 & CAN_U08_TMSTS_TMTRF_MASK) >> CAN_POS_TMSTS_TMTRF);
#endif
    if (result)
    {
        TMSTS0 = 0U;
    }
    if (result == 2U)
    {
        return CAN_RTN_TX_END;
    }
    if (!result)
    {
        return CAN_RTN_TRANSMITTING;
    }
    if (result == 3U)
    {
        return CAN_RTN_TX_END_WITH_ABORT_REQ;
    }
    return CAN_RTN_TX_ABORT_OVER;
}
Can_RtnType Device_CanAbortTx(void)
{
    TMC0 = CAN_U08_TMC_TMTAR_ON;
    return CAN_RTN_OK;
}
uint8_t Device_CanSend(can_frame_t *frame)
{
    uint32_t budget;
    Can_RtnType result;
    if (Device_CanStartTx(frame) != CAN_RTN_OK)
    {
        return 0U;
    }
    budget = 200000UL;
    do
    {
        result = Device_CanTxResult();
    } while (result == CAN_RTN_TRANSMITTING && --budget);
    if (result == CAN_RTN_TX_END)
    {
        return 1U;
    }
    (void)Device_CanAbortTx();
    /* Stop the controller on an incomplete abort: no stale buffer reuse. */
    budget = 60000UL;
    while (Device_CanTxResult() == CAN_RTN_TRANSMITTING && --budget)
    {
        BSP_NOP();
    }
    if (!budget)
    {
        Device_CanStop();
    }
    return 0U;
}
void Device_CanStop(void)
{
    RCANGRVCMK = 1U;
    P1 |= 4U;
    if (g_ready)
    {
        g_ready = 0U;
        (void)R_CAN_StopComm();
    }
}
uint16_t Device_CanErrors(void)
{
    return R_CAN_GetChBusErrFlag();
}
