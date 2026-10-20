#ifndef F24_TRANSFER_DIAGNOSTICS_H
#define F24_TRANSFER_DIAGNOSTICS_H

#include <stdint.h>

/* Transfer-only observations; no change to the Custom Transfer wire format.
 * Stages identify the last completed/failed initialization check. Result is
 * the Renesas CAN result, including WAITING for an active-state timeout.
 */
#define TRANSFER_CAN_STAGE_CONFIG  (1U)
#define TRANSFER_CAN_STAGE_STATE   (2U)
#define TRANSFER_CAN_STAGE_START   (3U)
#define TRANSFER_CAN_STAGE_ACTIVE  (4U)
#define TRANSFER_CAN_STAGE_READY   (5U)
#define TRANSFER_CAN_RX_QUEUE_SIZE (8U)

typedef struct {
    uint16_t result;
    uint16_t global_status;
    uint16_t channel_status;
    uint8_t stage;
    uint8_t standby;
} transfer_can_diagnostics_t;

void Device_CanGetDiagnostics(transfer_can_diagnostics_t *diagnostics);

/* Main-loop snapshot briefly masks/restores RXMB IRQ for coherent counters.
 * It never acknowledges RMND/CPU flags or consumes a queued frame.
 * rx_reads counts successful RXMB API returns; rx_overwrites counts detected
 * overwrite errors. Both saturate at 0xFFFF, not physical frame counters.
 * last_rx_* brackets InHandler after vendor ISR acknowledgement; zero before
 * the read is normal. irq/queued/dequeued/drops/api_errors also saturate.
 * Full RAM queue drops the incoming frame, preserving the queued order.
 * last_tx_status preserves the
 * status sampled by TX polling before its normal completion-flag clear.
 */
typedef struct {
    uint16_t rx_buffer_config;
    uint16_t rx_new_data;
    uint16_t last_rx_before;
    uint16_t last_rx_after;
    uint16_t rx_reads;
    uint16_t rx_overwrites;
    uint16_t rx_irqs;
    uint16_t rx_queued;
    uint16_t rx_dequeued;
    uint16_t rx_drops;
    uint16_t rx_api_errors;
    uint16_t rx_irq_enable;
    uint16_t global_status;
    uint16_t channel_status;
    uint16_t channel_errors;
    uint16_t ram_window;
    uint8_t tx_control;
    uint8_t tx_status;
    uint8_t last_tx_status;
    uint8_t standby;
    uint8_t rx_queue_depth;
    uint8_t rx_queue_peak;
    uint8_t rx_irq_mask;
    uint8_t rx_irq_pending;
} transfer_can_runtime_t;

void Device_CanGetRuntimeDiagnostics(transfer_can_runtime_t *diagnostics);
/* Main-loop capture of C0ERFL bits0..14, followed by write-zero clearing of
 * only the observed bits. Multiple events can share one latched indication.
 * Bit8 is the raw SERR bit here, not the driver's callback-only TDCV bit.
 */
uint16_t Device_CanPollErrors(void);

#endif
