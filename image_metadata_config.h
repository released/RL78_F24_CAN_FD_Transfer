/* Project/API reading notes: Project-local image contract: format, MCU/Flash geometry, IRQ ABI
 * and required Boot identity.
 * Addresses and capacities use bytes.
 * Changing platform/layout values requires compatible Boot/App/linker/package settings; normal
 * App release edits use app_image_config.h.
 */
#ifndef IMAGE_METADATA_CONFIG_H
#define IMAGE_METADATA_CONFIG_H

/*
 * Boot/App image contract, independent of the peripheral driver.
 * App release settings belong to the App's app_image_config.h.
 * CAN routing and protocol settings belong to can_boot_config.h.
 * The generator combines these inputs into the App metadata region.
 * Clean rebuild Boot and App after changing a shared contract.
 * Field/address guide:
 * https://released.github.io/slide_boot_loader_for_RL78/#device-app-config
 */

/* Serialized format: core 128 bytes, total region 256 bytes at App+0x100.
 * Example: magic 43 41 4E 00 is an image tag, not a CAN port number.
 * Format/flags/checksum/offsets form the Tool/Boot/App decoding contract.
 * Do not change one value without updating every producer and validator.
 */
#define IMAGE_FORMAT_VERSION (0x06U)
#define IMAGE_MANIFEST_OFFSET (0x00100UL)
#define IMAGE_CHECKSUM_CRC32 (0x01U)
#define IMAGE_VERSION_CONTROL_OFFSET (0x74UL)
#define IMAGE_MANIFEST_MAGIC (0x004E4143UL) /* "CAN\0" */
#define IMAGE_MANIFEST_SIZE (128U)
#define IMAGE_METADATA_REGION_SIZE (256U)
#define IMAGE_MANIFEST_FLAGS (0x0003U)
/* Diagnostic result detail: 1=field/policy/IRQ/layout codes, 0=BAD_IMAGE.
 * Both settings perform identical validation. This is a ROM option, not a
 * serialized metadata field; preserve the project's Boot budget guard.
 */
#define IMAGE_METADATA_DETAILED_STATUS (0U)
#define IMAGE_MANIFEST_FORMAT_WORD                                                                 \
    ((unsigned long)IMAGE_FORMAT_VERSION | ((unsigned long)IMAGE_MANIFEST_SIZE << 8) |             \
     ((unsigned long)IMAGE_MANIFEST_FLAGS << 16))
#define IMAGE_MANIFEST_FORMAT_OFFSET (0x04UL)
#define IMAGE_MANIFEST_PROTOCOL_OFFSET (0x08UL)
#define IMAGE_MANIFEST_TARGET_OFFSET (0x0CUL)
#define IMAGE_MANIFEST_VERSION_OFFSET (0x10UL)
#define IMAGE_MANIFEST_BUILD_OFFSET (0x14UL)
#define IMAGE_MANIFEST_SIZE_OFFSET (0x18UL)
#define IMAGE_MANIFEST_APP_START_OFFSET (0x1CUL)
#define IMAGE_MANIFEST_CAPACITY_OFFSET (0x20UL)
#define IMAGE_MANIFEST_CODE_FLASH_OFFSET (0x24UL)
#define IMAGE_MANIFEST_BOOT_SIZE_OFFSET (0x28UL)
#define IMAGE_MANIFEST_ERASE_SIZE_OFFSET (0x2CUL)
#define IMAGE_MANIFEST_NOMINAL_OFFSET (0x30UL)
#define IMAGE_MANIFEST_DATA_OFFSET (0x34UL)
#define IMAGE_MANIFEST_REQUEST_ID_OFFSET (0x38UL)
#define IMAGE_MANIFEST_RESPONSE_ID_OFFSET (0x3CUL)
#define IMAGE_MANIFEST_PART_OFFSET (0x40UL)
#define IMAGE_MANIFEST_FAMILY_OFFSET (0x50UL)
#define IMAGE_MANIFEST_MIN_SIZE_OFFSET (0x5CUL)

/* MCU/Flash engineering configuration. Addresses are bytes.
 * APPLICATION_CAPACITY is a maximum, not a required image size; post-build
 * derives actual size from the map and appends a four-byte CRC32.
 * Example: App start 0x2000 puts metadata at 0x2100 and code at 0x2200.
 * A partition change also requires linker/ftable/output-range/Flash guards.
 * TARGET_ID identifies this contract; changing it alone does not port an MCU.
 */
#define IMAGE_TARGET_ID (0xF24BU)
#define IMAGE_APPLICATION_START (0x02000UL)
#define IMAGE_APPLICATION_CAPACITY (0x3DC00UL)
#define IMAGE_CODE_FLASH_SIZE (0x40000UL)
#define IMAGE_RESERVED_SIZE (0x02000UL)
#define IMAGE_ERASE_BLOCK_SIZE (0x00400UL)
#define IMAGE_MINIMUM_IMAGE_SIZE (0x00204UL)

/* Little-endian, NUL-padded ASCII MCU part (16 bytes) and family (12 bytes).
 * Example: bytes 52 4C 37 38 encode "RL78"; update for the exact target part.
 */
#define IMAGE_MCU_PART_0 (0x31463752UL)
#define IMAGE_MCU_PART_1 (0x50463432UL)
#define IMAGE_MCU_PART_2 (0x4678354AUL)
#define IMAGE_MCU_PART_3 (0x00000042UL)
#define IMAGE_MCU_FAMILY_0 (0x38374C52UL)
#define IMAGE_MCU_FAMILY_1 (0x3432462FUL)
#define IMAGE_MCU_FAMILY_2 (0x00000000UL)

/* UPD1 policy and fixed IRQ ABI. Policy 0x01012003 packs version3/size32/
 * DEVICE_APP/allowed-CAN. This describes update permission, not driver pins.
 * IRQ ABI identifies the vector/startup contract. A new value needs paired
 * Boot/App support and Tool recognition; it does not generate new IRQ stubs.
 * App version-control 0/1 comes from app_image_config.h at metadata+0x74.
 */
#define IMAGE_POLICY_WORD (0x01012003UL)
#define IMAGE_IRQ_ABI (0x31563246UL)

/* Boot's own release identity, independent of APP_IMAGE_VERSION/BUILD.
 * Version 0x00MMmmpp encodes major.minor.patch, e.g. 0x00010700 = 1.7.0.
 * Build uses YYYYMMDD by convention, not for version ordering.
 * The producer writes this Boot version as the App's minimum Boot version.
 * A newer minimum is rejected by an older Boot; build dates need not match.
 */
#define IMAGE_BOOT_VERSION (0x00010201UL)
#define IMAGE_BOOT_BUILD (20261022UL)

#endif
