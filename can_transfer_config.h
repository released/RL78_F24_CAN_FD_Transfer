#ifndef CAN_TRANSFER_CONFIG_H
#define CAN_TRANSFER_CONFIG_H

/*
 * F24 non-destructive FD+BRS500K/2M Custom Transfer configuration.
 * Keep these values synchronized with the PC Custom Transfer v1 contract.
 */
#define CAN_TRANSFER_REQUEST_ID (0x710U)
#define CAN_TRANSFER_RESPONSE_ID (0x711U)

#define CAN_TRANSFER_PROTOCOL_MAJOR (1U)
#define CAN_TRANSFER_PROTOCOL_MINOR (0U)
#define CAN_TRANSFER_TARGET_ID (0xF24BU)

/*
 * Emit one deterministic UART line for each Custom Transfer CAN RX/TX frame.
 * The trace is generated only from main-loop protocol processing, never from
 * an ISR. Keep it enabled when capturing target logs for comparison with the
 * PC tool's Detailed trace. Set it to 0 for timing and queue-stress tests,
 * because blocking 115200-baud UART output intentionally changes throughput.
 */
#ifndef CAN_TRANSFER_UART_TRACE_ENABLE
#define CAN_TRANSFER_UART_TRACE_ENABLE (1U)
#endif

/* Read-only FIFO/TX snapshots for protocol diagnosis. Default follows frame
 * trace. Disable both for throughput measurements. At most three one-second
 * idle snapshots are emitted until another frame is processed.
 */
#ifndef CAN_TRANSFER_RUNTIME_DIAGNOSTICS_ENABLE
#define CAN_TRANSFER_RUNTIME_DIAGNOSTICS_ENABLE (CAN_TRANSFER_UART_TRACE_ENABLE)
#endif

/* Receive only standard data frames sent to CAN_TRANSFER_REQUEST_ID. */
#define CAN_TRANSFER_RX_RULE_ID_LOW (CAN_TRANSFER_REQUEST_ID)
#define CAN_TRANSFER_RX_RULE_ID_HIGH (0x0000U)
#define CAN_TRANSFER_RX_RULE_MASK_LOW (0x07FFU)
#define CAN_TRANSFER_RX_RULE_MASK_HIGH (0xC000U)

#if (CAN_TRANSFER_REQUEST_ID > 0x07FFU)
#error CAN_TRANSFER_REQUEST_ID must be an 11-bit standard CAN identifier.
#endif

#if (CAN_TRANSFER_RESPONSE_ID > 0x07FFU)
#error CAN_TRANSFER_RESPONSE_ID must be an 11-bit standard CAN identifier.
#endif

#endif
