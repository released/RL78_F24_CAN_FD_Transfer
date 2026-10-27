#ifndef CAN_TRANSFER_PROTOCOL_H
#define CAN_TRANSFER_PROTOCOL_H

#include "r_cg_macrodriver.h"
#include "device_board.h"
#include "custom_transfer_fd.h"

#define CAN_TRANSFER_FRAME_SIZE (8U)

#define CAN_TRANSFER_REQUEST_MAGIC (0xA5U)
#define CAN_TRANSFER_RESPONSE_MAGIC (0x5AU)

#define CAN_TRANSFER_CMD_DEVICE_INFO (0x01U)
#define CAN_TRANSFER_CMD_PING (0x02U)
#define CAN_TRANSFER_CMD_ECHO (0x03U)
#define CAN_TRANSFER_CMD_PATTERN_TEST (0x04U)
#define CAN_TRANSFER_CMD_GET_STATS (0x05U)
#define CAN_TRANSFER_CMD_CLEAR_STATS (0x06U)
#define CAN_TRANSFER_CMD_ERROR_RESPONSE (0x7FU)

#define CAN_TRANSFER_STATUS_BAD_FORMAT (0x01U)
#define CAN_TRANSFER_STATUS_BAD_MAGIC (0x02U)
#define CAN_TRANSFER_STATUS_BAD_VERSION (0x03U)
#define CAN_TRANSFER_STATUS_BAD_SEQUENCE (0x04U)
#define CAN_TRANSFER_STATUS_CRC_ERROR (0x05U)
#define CAN_TRANSFER_STATUS_BAD_COMMAND (0x06U)
#define CAN_TRANSFER_STATUS_BAD_ARGUMENT (0x07U)

void CAN_TransferProtocol_Init(Can_RtnType hardware_result);
void CAN_TransferProtocol_Process(void);
void CAN_TransferProtocol_OnChannelError(uint16_t error_flags);

#endif
