#ifndef F24_CAN_IMAGE_LAYOUT_H
#define F24_CAN_IMAGE_LAYOUT_H
#include "can_boot_config.h"
#include "image_metadata_config.h"
#include "boot_app_handoff.h"
/* Last 1KiB is protected, including the CS+ debug monitor at 3FF00. */
#define FLASH_AREA IMAGE_APPLICATION_START
#define APP_FLASH_START IMAGE_APPLICATION_START
#define FLASH_UPDATE_MAX_SIZE IMAGE_APPLICATION_CAPACITY
#define FLASH_TOTAL_SIZE IMAGE_CODE_FLASH_SIZE
#define FLASH_END_ADDRESS (APP_FLASH_START+FLASH_UPDATE_MAX_SIZE-1UL)
#define FLASH_BYTE_PER_BLOCK IMAGE_ERASE_BLOCK_SIZE
#define APP_MANIFEST_ADDRESS (APP_FLASH_START+IMAGE_MANIFEST_OFFSET)
#define APP_IMAGE_MIN_SIZE IMAGE_MINIMUM_IMAGE_SIZE
#define APP_CRC_SIZE 4UL
#define FSL_OK 0U
unsigned char Device_FlashInit(void);
unsigned char Device_FlashWrite(unsigned long address, unsigned char __near *data);
unsigned char Device_FlashErase(unsigned long address);
unsigned char CAN_BootInstalledValid(void);
#endif
