#include "platform.h"
#include "device_board.h"
#include "user_def.h"
#include "r_rfd_common_api.h"
#include "r_rfd_common_control_api.h"
#include "r_rfd_common_get_api.h"
#include "r_rfd_code_flash_api.h"
/* This whole call chain and RFD callees execute from RAM while CF is busy. */
#pragma section text DRAM
static void __far fail_reset(void)
{
    *(volatile uint32_t __far *)(BOOT_APP_SHARED_RAM_ADDRESS + 12UL) =
        BOOT_APP_RESET_TO_BOOT_SIGNATURE;
    IAWCTL |= 0x80U;
    *(volatile uint8_t __far *)0UL = 0U;
    for (;;)
    {
        BSP_NOP();
    }
}
static uint8_t __far wait_end(void)
{
    uint32_t budget;
    uint8_t error;
    budget = 2000000UL;
    while (R_RFD_CheckCFDFSeqEndStep1() == R_RFD_ENUM_RET_STS_BUSY)
    {
        if (!--budget)
        {
            fail_reset();
        }
    }
    budget = 2000000UL;
    while (R_RFD_CheckCFDFSeqEndStep2() == R_RFD_ENUM_RET_STS_BUSY)
    {
        if (!--budget)
        {
            fail_reset();
        }
    }
    R_RFD_GetSeqErrorStatus(&error);
    R_RFD_ClearSeqRegister();
    return (uint8_t)(error == 0U);
}
static uint8_t __far execute(uint32_t address, const uint8_t *data, uint8_t erase)
{
    uint8_t result, bytes[4];
    e_rfd_ret_t mode;
    result = 0U;
    if (!erase)
    {
        bytes[0] = data[0];
        bytes[1] = data[1];
        bytes[2] = data[2];
        bytes[3] = data[3];
    }
    mode = R_RFD_SetFlashMemoryMode(R_RFD_ENUM_FLASH_MODE_CODE_PROGRAMMING);
    if (mode == R_RFD_ENUM_RET_STS_OK)
    {
        if (erase)
        {
            R_RFD_EraseCodeFlashReq((uint16_t)(address >> 10));
        }
        else
        {
            R_RFD_WriteCodeFlashReq(address, bytes);
        }
        result = wait_end();
    }
    mode = R_RFD_SetFlashMemoryMode(R_RFD_ENUM_FLASH_MODE_CODE_TO_NONPROGRAMMABLE);
    if (mode != R_RFD_ENUM_RET_STS_OK ||
        R_RFD_CheckFlashMemoryMode(R_RFD_ENUM_FLASH_MODE_CODE_TO_NONPROGRAMMABLE) !=
            R_RFD_ENUM_RET_STS_OK)
    {
        fail_reset();
    }
    return result;
}
#pragma section
unsigned char Device_FlashInit(void)
{
    return (R_RFD_Init(40U) == R_RFD_ENUM_RET_STS_OK) ? FSL_OK : 1U;
}
unsigned char Device_FlashErase(unsigned long address)
{
    if (address < APP_FLASH_START || address >= APP_FLASH_START + FLASH_UPDATE_MAX_SIZE ||
        address % FLASH_BYTE_PER_BLOCK)
    {
        return 1U;
    }
    return execute(address, 0, 1U) ? FSL_OK : 1U;
}
unsigned char Device_FlashWrite(unsigned long address, unsigned char __near *data)
{
    if (address < APP_FLASH_START || address > APP_FLASH_START + FLASH_UPDATE_MAX_SIZE - 4UL ||
        address % 4UL)
    {
        return 1U;
    }
    return execute(address, data, 0U) ? FSL_OK : 1U;
}
