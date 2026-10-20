/*_____ I N C L U D E S ____________________________________________________*/

#include "r_rscanfd_rl78_if.h"
#include "r_rscanfd_rl78_config.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

void CAN_Get_Module_Version(void);

void CAN_Get_Timestamp_Counter(void);

void CAN_Get_Transceiver_Delay_Compensation(void);

void CAN_Get_Bus_Error_Flag(void);

void CAN_Get_Channel_Status(void);

void CAN_Read_RX_Data_Common_FIFO(void);

void CAN_Read_RX_Data_FIFO(void);

void CAN_Get_RX_Data_Message_Buffer(void);

void CAN_Read_TX_History_list(void);

void CAN_Abort_TX_Data_Common_FIFO(void);

void CAN_Send_TX_Data_Common_FIFO(void);

void CAN_Get_TX_Data_Message_Buffer_Result(void);

void CAN_Abort_TX_Data_Message_Buffer(void);

void CAN_Send_TX_Data_Message_Buffer_polling2(void);
void CAN_Send_TX_Data_Message_Buffer_polling1(void);
void CAN_Send_TX_Data_Message_Buffer(void);

void CAN_Bus_EntrySleepMode(void);

void CAN_Bus_Reset(void);

void CAN_Bus_Config(void);

extern void my_can_glb_rxfifo_callback(uint16_t arg);
extern void my_can_glb_rxmb_callback(uint16_t arg);
extern void my_can_glb_error_callback(uint16_t arg);
extern void my_can_ch0_transmit_callback(uint16_t arg);
extern void my_can_ch0_cfifo_rx_callback(uint16_t arg);
extern void my_can_ch0_error_callback(uint16_t arg);