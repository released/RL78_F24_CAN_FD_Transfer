/* MCU/ISP compatibility profile. The parser itself has no CAN dependency. */
/* Project/API reading notes: Expected values for this project's serialized image contract.
 * Root image_metadata_config.h and transport configuration supply platform values; App release
 * identity comes from generated build inputs.
 * Reserved bytes remain part of the validation contract.
 */
#include "image_metadata.h"
#include "image_metadata_config.h"
#include "can_boot_config.h"

const unsigned long g_image_metadata_contract[32] = {
    IMAGE_MANIFEST_MAGIC,       /* 0x00: image tag */
    IMAGE_MANIFEST_FORMAT_WORD, /* 0x04: format/core length/flags */
    (unsigned long)CAN_BOOT_PROTOCOL_ID | ((unsigned long)CAN_BOOT_PROTOCOL_VERSION << 16) |
        ((unsigned long)IMAGE_CHECKSUM_CRC32 << 24), /* 0x08: protocol/checksum */
    (unsigned long)IMAGE_TARGET_ID | ((unsigned long)CAN_BOOT_CAN_MODE_MASK << 16) |
        ((unsigned long)CAN_BOOT_CAN_ID_STANDARD << 24), /* 0x0C: target/mode/ID type */
    0UL,                                                 /* 0x10: App version; range checked */
    0UL,                                                 /* 0x14: App build; range checked */
    0UL,                                                 /* 0x18: actual size; range checked */
    IMAGE_APPLICATION_START,                             /* 0x1C: absolute App start */
    IMAGE_APPLICATION_CAPACITY,                          /* 0x20: maximum App bytes */
    IMAGE_CODE_FLASH_SIZE,                               /* 0x24: physical code Flash bytes */
    IMAGE_RESERVED_SIZE,                                 /* 0x28: Boot reserve bytes */
    IMAGE_ERASE_BLOCK_SIZE,                              /* 0x2C: erase block bytes */
    CAN_BOOT_NOMINAL_BITRATE_MASK,                       /* 0x30: nominal rate mask */
    CAN_BOOT_DATA_BITRATE_MASK,                          /* 0x34: data-phase rate mask */
    CAN_BOOT_REQUEST_ID,                                 /* 0x38: control request ID */
    CAN_BOOT_RESPONSE_ID,                                /* 0x3C: response ID */
    IMAGE_MCU_PART_0,
    IMAGE_MCU_PART_1,
    IMAGE_MCU_PART_2,
    IMAGE_MCU_PART_3, /* 0x40..0x4F: MCU part ASCII */
    IMAGE_MCU_FAMILY_0,
    IMAGE_MCU_FAMILY_1,
    IMAGE_MCU_FAMILY_2,       /* 0x50..0x5B: MCU family ASCII */
    IMAGE_MINIMUM_IMAGE_SIZE, /* 0x5C: minimum image bytes */
    0x31445055UL,             /* 0x60: UPD1 policy tag */
    IMAGE_POLICY_WORD,        /* 0x64: policy version/size/role/transports */
    IMAGE_IRQ_ABI,            /* 0x68: fixed IRQ/startup ABI */
    IMAGE_BOOT_VERSION,       /* 0x6C: supported Boot; incoming minimum <= this */
    0UL,                      /* 0x70: device UART baud; this CAN profile uses0 */
    0UL,                      /* 0x74: version control; input0/1 checked separately */
    0UL,
    0UL /* 0x78..0x7F: required zero */
};
/* Metadata0x80..0xFF must be FF; validated by the common C module. */
