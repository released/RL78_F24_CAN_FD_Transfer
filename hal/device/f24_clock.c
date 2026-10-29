#include "platform.h"
#include "device_board.h"
static uint8_t wait_byte(volatile uint8_t __near *reg, uint8_t mask, uint8_t value, uint32_t budget)
{
    while (budget != 0UL)
    {
        if ((*reg & mask) == value)
        {
            return 1U;
        }
        --budget;
        BSP_NOP();
    }
    return 0U;
}
uint8_t Device_ClockInit(void)
{
    uint8_t delay;
    /* App startup inherits the exact clock contract without changing CMC. */
    if (MDIV == 1U && MCS && !CLS && (PLLSTS & 0x88U) == 0x88U && (CKSEL & 0xE0U) == 0x80U &&
        PLLCTL == 0xEFU && (CMC & 0xC1U) == 0x40U)
    {
        return 1U;
    }
    if (MDIV || MCS || CLS || PLLON || SELPLLS || (CMC & 0xC1U) != 0x40U)
    {
        return 0U;
    }
    OSTS = 7U;
    MSTOP = 0U;
    if (!wait_byte(&OSTC, 0xFFU, 0xFFU, 2000000UL))
    {
        return 0U;
    }
    MCM0 = 1U;
    if (!wait_byte(&CKC, 0x20U, 0x20U, 60000UL))
    {
        return 0U;
    }
    /* 4MHz /1 x20 = 80MHz. LCKSEL=11 supplies a 256us lock wait. */
    CKSEL = (uint8_t)((CKSEL & 0x1FU) | 0x80U);
    PLLCTL = 0xEAU;
    for (delay = 0U; delay < 8U; ++delay)
    {
        BSP_NOP();
    }
    PLLON = 1U;
    if (!wait_byte(&PLLSTS, 0x80U, 0x80U, 60000UL))
    {
        return 0U;
    }
    MDIV = 1U;
    SELPLL = 1U;
    return wait_byte(&PLLSTS, 0x88U, 0x88U, 60000UL);
}
