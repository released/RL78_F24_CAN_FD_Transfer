/* Project/API reading notes: Project-owned initialization and foreground work.
 * The selected main/startup adapter calls hardware_init() once and loop() repeatedly.
 * Keep application changes here so generated entry sources remain replaceable; lower-level APIs
 * live in this project's hal directories.
 */
#include "platform.h"
#include <stdio.h>
#include "device_board.h"
#include "can_transfer_protocol.h"
#include "can_transfer_config.h"
#include "transfer_diagnostics.h"
int __far putchar(int value)
{
    char text[2];
    text[0] = (char)value;
    text[1] = '\0';
    Device_Log((const char __far *)text);
    return value;
}

static uint8_t g_custom_ready;
static uint16_t g_custom_errors;
static transfer_can_diagnostics_t g_custom_diagnostics;

/* Initialize the role selected by this project before foreground processing.
 * Preserve clock, image/handoff and peripheral/IRQ enable order when adding application
 * initialization.
 */
void hardware_init(void)
{
    if (!Device_BoardInit())
    {
        for (;;)
        {
            BSP_NOP();
        }
    }
    g_custom_ready = Device_CanInit(CAN_TRANSFER_REQUEST_ID, 0U);
    CAN_TransferProtocol_Init(g_custom_ready ? CAN_RTN_OK : CAN_RTN_STS_ERROR);
    Device_CanGetDiagnostics(&g_custom_diagnostics);
    Device_Log("clock status: OK; X1=4M PLL80 CPU/CAN40\r\n");
    printf("CAN init: %s result=0x%04X stage=%u global_status=0x%04X channel_status=0x%04X\r\n",
           g_custom_ready ? "OK" : "FAIL", (unsigned int)g_custom_diagnostics.result,
           (unsigned int)g_custom_diagnostics.stage,
           (unsigned int)g_custom_diagnostics.global_status,
           (unsigned int)g_custom_diagnostics.channel_status);
    printf(
        "CAN config: FD 500K/2M BRS payload=8/64 request=0x%03X response=0x%03X filter=0x%03X/0x7FF STB=P12_%s\r\n",
        (unsigned int)CAN_TRANSFER_REQUEST_ID, (unsigned int)CAN_TRANSFER_RESPONSE_ID,
        (unsigned int)CAN_TRANSFER_REQUEST_ID, g_custom_diagnostics.standby ? "HIGH" : "LOW");
    Device_Log("hardware_init finish\r\n");
    Device_Log("CAN RX backend: RXMB0 interrupt -> RAM queue=8 -> main; payload=64 TX=polling\r\n");
}

/* Foreground service point called repeatedly by the selected main adapter.
 * Keep protocol/application work and text logging bounded so pending traffic and deferred actions
 * can continue.
 */
void loop(void)
{

    g_custom_errors = Device_CanPollErrors();
    if (g_custom_errors != 0U)
    {
        CAN_TransferProtocol_OnChannelError(g_custom_errors);
    }
    CAN_TransferProtocol_Process();
    Device_Heartbeat();
}
