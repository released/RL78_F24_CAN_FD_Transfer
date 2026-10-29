/* Project/API reading notes: CAN transport configuration only: command/response IDs, mode and
 * compiled timing declarations.
 * Keep actual driver timing, Boot/App declarations and Tool selections compatible; neutral image
 * geometry belongs to image_metadata_config.h.
 */
#ifndef CAN_BOOT_CONFIG_H
#define CAN_BOOT_CONFIG_H

/* CAN transport configuration shared by Boot and App.
 * IDs/rates describe the compiled update interface; changing a label does not
 * reconfigure peripheral registers. Keep driver timing and Tool profile aligned.
 * Image format/MCU/Flash/policy are in image_metadata_config.h.
 * RAM handoff/IRQ owner markers are in boot_app_handoff.h.
 */
#define CAN_BOOT_REQUEST_ID (0x700U)
#define CAN_BOOT_RESPONSE_ID (0x701U)
#define CAN_BOOT_PROTOCOL_ID (0x0001U)
#define CAN_BOOT_PROTOCOL_VERSION (0x05U)
#define CAN_BOOT_BLOCK_BYTES (64U)
#define CAN_BOOT_BLOCK_DATA_ID (0x720U)
#define CAN_BOOT_CMD_BLOCK_BEGIN (0x0BU)
#define CAN_BOOT_CMD_BLOCK_COMMIT (0x0CU)
/* Compiled CAN FD capabilities. Nominal bit2=500K; data bit1=2M, BRS.
 * Keep actual peripheral clock/timing and the App ENTER path aligned.
 */
#define CAN_BOOT_CAN_MODE_MASK (0x02U)
#define CAN_BOOT_CAN_ID_STANDARD (0x00U)
#define CAN_BOOT_NOMINAL_BITRATE_MASK (0x00000004UL)
#define CAN_BOOT_DATA_BITRATE_MASK (0x00000002UL)
/* INFO request [07,page]; reply [07,status,page,state,LE32 value].
 * Page identifiers belong to the wire protocol; their image values come from
 * image_metadata_config.h or the validated installed App.
 */
#define CAN_BOOT_CMD_INFO (0x07U)
#define CAN_BOOT_INFO_PAGE_CORE (0x00U)
#define CAN_BOOT_INFO_PAGE_BOOT_VERSION (0x01U)
#define CAN_BOOT_INFO_PAGE_BOOT_BUILD (0x02U)
#define CAN_BOOT_INFO_PAGE_APP_VERSION (0x03U)
#define CAN_BOOT_INFO_PAGE_APP_BUILD (0x04U)
#define CAN_BOOT_INFO_PAGE_APP_START (0x05U)
#define CAN_BOOT_INFO_PAGE_APP_CAPACITY (0x06U)
#define CAN_BOOT_INFO_PAGE_CODE_FLASH (0x07U)
#define CAN_BOOT_INFO_PAGE_BOOT_RESERVED (0x08U)
#define CAN_BOOT_INFO_PAGE_ERASE_BLOCK (0x09U)
#define CAN_BOOT_INFO_PAGE_CAN_CAPS (0x0AU)
#define CAN_BOOT_INFO_PAGE_NOMINAL_MASK (0x0BU)
#define CAN_BOOT_INFO_PAGE_DATA_MASK (0x0CU)
#define CAN_BOOT_INFO_PAGE_REQUEST_ID (0x0DU)
#define CAN_BOOT_INFO_PAGE_RESPONSE_ID (0x0EU)
#define CAN_BOOT_INFO_PAGE_PART_0 (0x0FU)
#define CAN_BOOT_INFO_PAGE_PART_1 (0x10U)
#define CAN_BOOT_INFO_PAGE_PART_2 (0x11U)
#define CAN_BOOT_INFO_PAGE_PART_3 (0x12U)
#define CAN_BOOT_INFO_PAGE_FAMILY_0 (0x13U)
#define CAN_BOOT_INFO_PAGE_FAMILY_1 (0x14U)
#define CAN_BOOT_INFO_PAGE_FAMILY_2 (0x15U)
#define CAN_BOOT_INFO_PAGE_APP_IMAGE_SIZE (0x16U)
#define CAN_BOOT_INFO_PAGE_MIN_IMAGE_SIZE (0x17U)
#define CAN_BOOT_INFO_PAGE_UPDATE_POLICY (0x18U)
#define CAN_BOOT_INFO_PAGE_IRQ_ABI (0x19U)
#define CAN_BOOT_INFO_PAGE_BLOCK (0x1AU)
#define CAN_BOOT_BLOCK_INFO (((unsigned long)CAN_BOOT_BLOCK_DATA_ID << 16) | CAN_BOOT_BLOCK_BYTES)
/* FORCE_START applies to one transaction; no persistent unlock is stored. */
#define CAN_BOOT_CMD_FORCE_START (0x0EU)
#define CAN_BOOT_INFO_PAGE_VERSION_POLICY (0x1DU)
#define CAN_BOOT_CMD_METADATA (0x08U)
#define CAN_BOOT_CMD_SEAL (0x09U)
#define CAN_BOOT_CMD_METADATA_RESET (0x0AU)
#define CAN_BOOT_STATE_APPLICATION (0x01U)
#define CAN_BOOT_STATE_BOOTLOADER (0x02U)
#define CAN_BOOT_DEBUG_UART (0U)
/* Bounded CAN polling; inactivity uses the 1-ms timer while Flash is idle. */
#define CAN_BOOT_CAN_INIT_POLL_LIMIT (10000UL)
#define CAN_BOOT_CAN_TX_POLL_LIMIT (10000UL)
#define CAN_BOOT_INACTIVITY_TIMEOUT_MS (30000U)
/* Receive standard data frames on the configured control request ID. */
#define CAN_BOOT_RX_RULE_ID_LOW (CAN_BOOT_REQUEST_ID)
#define CAN_BOOT_RX_RULE_ID_HIGH (0x0000U)
#define CAN_BOOT_RX_RULE_MASK_LOW (0x07FFU)
#define CAN_BOOT_RX_RULE_MASK_HIGH (0xC000U)

#if (CAN_BOOT_REQUEST_ID > 0x07FFU)
#error CAN_BOOT_REQUEST_ID must be an 11-bit standard CAN identifier.
#endif

#if (CAN_BOOT_RESPONSE_ID > 0x07FFU)
#error CAN_BOOT_RESPONSE_ID must be an 11-bit standard CAN identifier.
#endif

#if (CAN_BOOT_BLOCK_BYTES != 64U) || (CAN_BOOT_BLOCK_DATA_ID > 0x7F8U) ||                          \
    ((CAN_BOOT_BLOCK_DATA_ID & 7U) != 0U)
#error CAN v5 requires64-byte blocks and an aligned standard raw-data base ID.
#endif
#if ((CAN_BOOT_REQUEST_ID & 0x7F8U) == CAN_BOOT_BLOCK_DATA_ID) ||                                  \
    ((CAN_BOOT_RESPONSE_ID & 0x7F8U) == CAN_BOOT_BLOCK_DATA_ID)
#error Block data identifiers must not overlap control or response identifiers.
#endif

#endif
