#ifndef F24_DEVICE_BOARD_H
#define F24_DEVICE_BOARD_H
#include <stdint.h>
#include "can_boot_config.h"
#include "image_metadata_config.h"
#include "boot_app_handoff.h"
/* Compatibility data structure at the protocol/HAL boundary, not an SFR map. */
typedef struct {
    uint16_t IDL, IDH, TS, LBL;
    uint8_t IDE, RTR, THDSE, DLC;
    /* DLC at this HAL boundary is the decoded byte length (0..8 or64). */
    uint8_t DB[64];
} can_frame_t;
typedef uint8_t Can_RtnType;
#define CAN_RTN_OK 0U
#define CAN_RTN_OK_WITH_LOST 1U
#define CAN_RTN_BUFFER_EMPTY 2U
#define CAN_RTN_STS_ERROR 3U
#define CAN_RTN_TRANSMITTING 4U
#define CAN_RTN_TX_END 5U
#define CAN_RTN_TX_ABORT_OVER 6U
#define CAN_RTN_TX_END_WITH_ABORT_REQ 7U
#define CAN_CH0 0U
uint8_t Device_BoardInit(void);
uint8_t Device_CanInit(uint16_t request_id, uint8_t blocks);
Can_RtnType Device_CanRead(can_frame_t *frame);
/* Boot sends fixed standard RESPONSE_ID/FD+BRS/DLC8 with DB[0..7];
 * App/Transfer use the caller's validated ID/type/decoded length. */
Can_RtnType Device_CanStartTx(const can_frame_t *frame);
Can_RtnType Device_CanTxResult(void);
Can_RtnType Device_CanAbortTx(void);
uint8_t Device_CanSend(can_frame_t *frame);
void Device_CanStop(void);
uint16_t Device_CanErrors(void);
#ifndef F24_DEVICE_TRANSFER
/* Sticky RX faults since init: bit0 queue full/not-ready, bit1 overwrite,
 * bit2 invalid FD header. Normal request/reply operation returns zero. */
uint8_t Device_CanRxFaults(void);
/* Discard observations saturate at255 and reset only at CAN init. */
uint8_t Device_CanRxDrops(void);
#endif
unsigned int get_tick(void);
void Device_Log(const char __far *text);
void Device_LogHex(uint32_t value);
void Device_Reset(void);
void Device_RequestBoot(void);
void Device_Jump(void);
void Device_Heartbeat(void);
void CAN_BootTimerTick(void);
#endif
