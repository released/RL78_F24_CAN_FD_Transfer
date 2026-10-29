/* Project/API reading notes: Device CAN update interface.
 * CAN IDs/rate declarations are configured in the local CAN header; image layout and identity
 * remain in the neutral metadata contract.
 * A valid frame alone does not authorize Flash modification.
 */
#ifndef CAN_BOOT_TRANSPORT_H
#define CAN_BOOT_TRANSPORT_H

#include "can_boot_config.h"
#include "image_metadata_config.h"
#include "image_metadata.h"
#include "boot_app_handoff.h"

#define CAN_BOOT_CMD_HELLO           (0x01U)
#define CAN_BOOT_CMD_START           (0x02U)
#define CAN_BOOT_CMD_END             (0x04U)
#define CAN_BOOT_CMD_ABORT           (0x05U)
#define CAN_BOOT_CMD_ENTER           (0x06U)

#define CAN_BOOT_STATUS_OK           (0x00U)
#define CAN_BOOT_STATUS_BAD_COMMAND  (0x01U)
#define CAN_BOOT_STATUS_BAD_STATE    (0x02U)
#define CAN_BOOT_STATUS_BAD_SEQUENCE (0x03U)
#define CAN_BOOT_STATUS_BAD_SIZE     (0x04U)
#define CAN_BOOT_STATUS_FLASH_ERROR  (0x05U)
#define CAN_BOOT_STATUS_CRC_ERROR    (0x06U)
#define CAN_BOOT_STATUS_BAD_PROTOCOL (0x07U)
#define CAN_BOOT_STATUS_BAD_TARGET   (0x08U)
#define CAN_BOOT_STATUS_BAD_IMAGE    (0x09U)
#define CAN_BOOT_STATUS_VERSION      (0x0EU)
unsigned char CAN_BootInstalledValid(void);

#define CAN_BOOT_RESULT_ABORT        (-1L)
#define CAN_BOOT_RESULT_FLASH_ERROR  (-5L)
#define CAN_BOOT_RESULT_SIZE_ERROR   (-6L)
#define CAN_BOOT_RESULT_CRC_ERROR    (-7L)
#define CAN_BOOT_RESULT_TIMEOUT      (-8L)
#define CAN_BOOT_RESULT_IMAGE_ERROR  (-9L)
#define CAN_BOOT_RESULT_SESSION_TIMEOUT (-10L)

unsigned char CAN_BootInit(void);
void CAN_BootTimerTick(void);
signed long CAN_BootReceive(unsigned long destination,
                            unsigned char flash_init_status,
                            unsigned char timeout_enabled);

#endif
