#include "platform.h"
#include <stdio.h>
#include "device_board.h"
#include "can_transfer_protocol.h"
#include "can_transfer_config.h"
#include "transfer_diagnostics.h"
int __far putchar(int value)
{
    char text[2];
    text[0] = (char)value; text[1] = '\0';
    Device_Log((const char __far *)text);
    return value;
}
void main(void)
{
    uint8_t ready;
    uint16_t errors;
    transfer_can_diagnostics_t diagnostics;
    if (!Device_BoardInit()) for (;;) { BSP_NOP(); }
    ready = Device_CanInit(CAN_TRANSFER_REQUEST_ID, 0U);
    CAN_TransferProtocol_Init(ready ? CAN_RTN_OK : CAN_RTN_STS_ERROR);
    Device_CanGetDiagnostics(&diagnostics);
    Device_Log("clock status: OK; X1=4M PLL80 CPU/CAN40\r\n");
    printf("CAN init: %s result=0x%04X stage=%u global_status=0x%04X channel_status=0x%04X\r\n",
        ready ? "OK" : "FAIL", (unsigned int)diagnostics.result,
        (unsigned int)diagnostics.stage, (unsigned int)diagnostics.global_status,
        (unsigned int)diagnostics.channel_status);
    printf("CAN config: FD 500K/2M BRS payload=8/64 request=0x%03X response=0x%03X filter=0x%03X/0x7FF STB=P12_%s\r\n",
        (unsigned int)CAN_TRANSFER_REQUEST_ID, (unsigned int)CAN_TRANSFER_RESPONSE_ID,
        (unsigned int)CAN_TRANSFER_REQUEST_ID, diagnostics.standby ? "HIGH" : "LOW");
    Device_Log("hardware_init finish\r\n");
    Device_Log("CAN RX backend: RXMB0 interrupt -> RAM queue=8 -> main; payload=64 TX=polling\r\n");
    for (;;) {
        errors = Device_CanPollErrors();
        if (errors != 0U) CAN_TransferProtocol_OnChannelError(errors);
        CAN_TransferProtocol_Process();
        Device_Heartbeat();
    }
}
