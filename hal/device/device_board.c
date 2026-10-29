/* Project/API reading notes: Board support for clocks, peripheral startup, timebase and
 * diagnostic UART.
 * Application/protocol work belongs to custom_func.c; interrupt handlers perform bounded
 * hardware/event work and leave parsing/printing to the foreground.
 */
#include "platform.h"
#include "r_cg_macrodriver.h"
#include "Config_UART0.h"
#include "Config_TAU0_1.h"
#include "r_cg_sau_common.h"
#include "r_cg_tau_common.h"
#include "device_board.h"
extern uint8_t Device_ClockInit(void);
static volatile uint16_t g_tick;
/* Generated drivers remain untouched. These board callbacks own timer/UART
 * setup; CAN interrupt selection belongs to the project profile. */
void R_Config_PORT_Create_UserInit(void)
{
}
void R_Config_UART0_Create_UserInit(void)
{
}
void R_Config_TAU0_1_Create_UserInit(void)
{
}
void R_Systeminit(void)
{
#ifndef F24_DEVICE_APP
    *(volatile uint8_t __far *)BOOT_APP_SHARED_RAM_ADDRESS = BOOT_APP_RAM_OWNER_BOOT;
#endif
    /* Assert standby before enabling its output; generated PORT defaults
     * briefly drove P12 low before CAN TX was initialized. */
    P1 |= 4U;
    PM1 &= 0xFBU;
    P6 |= 0xC0U;
    PM6 &= 0x3FU;
#if !defined(F24_DEVICE_BOOT) || CAN_BOOT_DEBUG_UART
    R_SAU0_Create();
#endif
    R_TAU0_Create();
}
#pragma interrupt Device_TimerIsr
/* Service the hardware timebase/event source with bounded ISR work.
 * Foreground code consumes the resulting timing state.
 */
void __near Device_TimerIsr(void)
{
    ++g_tick;
#ifdef F24_DEVICE_BOOT
    CAN_BootTimerTick();
#endif
}
unsigned int get_tick(void)
{
    return g_tick;
}
uint8_t Device_BoardInit(void)
{
    uint8_t ready;
    ready = Device_ClockInit();
    /* The generated UART divisor is for 115200 at CPU40. Never log at X1/4MHz. */
    if (!ready)
    {
        P6 &= 0xBFU;
        return 0U;
    }
#if !defined(F24_DEVICE_BOOT) || CAN_BOOT_DEBUG_UART
    R_Config_UART0_Start();
    STMK0 = 1U;
    SRMK0 = 1U;
#endif
    R_Config_TAU0_1_Start();
    BSP_EI();
    return 1U;
}
void Device_Log(const char __far *text)
{
    uint32_t budget;
    while (*text != '\0')
    {
        STIF0 = 0U;
        SDR00L = (uint8_t)*text++;
        budget = 200000UL;
        while (!STIF0 && --budget)
        {
            BSP_NOP();
        }
        if (!budget)
        {
            return;
        }
        STIF0 = 0U;
    }
}
void Device_LogHex(uint32_t value)
{
    char buffer[11];
    uint8_t index, digit;
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[10] = '\0';
    for (index = 0U; index < 8U; ++index)
    {
        digit = (uint8_t)((value >> ((7U - index) * 4U)) & 15U);
        buffer[index + 2U] = (char)((digit < 10U) ? ('0' + digit) : ('A' + digit - 10U));
    }
    Device_Log((const char __far *)buffer);
}
void Device_Heartbeat(void)
{
    static uint16_t last;
    if ((uint16_t)(g_tick - last) >= 500U)
    {
        last = g_tick;
        P6 ^= 0xC0U;
    }
}
void Device_Reset(void)
{
    BSP_DI();
    IAWCTL |= 0x80U;
    *(volatile uint8_t __far *)0UL = 0U;
    for (;;)
    {
        BSP_NOP();
    }
}
void Device_RequestBoot(void)
{
    BSP_DI();
    *(volatile uint32_t __far *)(BOOT_APP_SHARED_RAM_ADDRESS + 12UL) =
        BOOT_APP_RESET_TO_BOOT_SIGNATURE;
    Device_Reset();
}
