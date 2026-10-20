 /*_____ I N C L U D E S ____________________________________________________*/
#include <stdio.h>

#include "inc_main.h"
#include "can_fd_module.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/
typedef unsigned short WCHAR;


st_can_rx_frame_t rx_frame; 

/*_____ M A C R O S ________________________________________________________*/
#define __T(x)      L ## x
#define _T(x)       __T(x)
/*_____ F U N C T I O N S __________________________________________________*/

extern void dump_buffer_hex(unsigned char *pucBuff, int nBytes);

void can_fd_err_code(e_can_err_t rc)
{
    char* ary[] = 
    {
        "CAN_SUCCESS\0",
        "CAN_SUCCESS_WITH_LOST\0",
        "CAN_ERR_WAITING\0",
        "CAN_ERR_INVALID_ARG\0",
        "CAN_ERR_INVALID_MODE\0",
        "CAN_ERR_ILLEGAL_STS\0",
        "CAN_ERR_BUF_BUSY\0",
        "CAN_ERR_BUF_FULL\0",
        "CAN_ERR_BUF_EMPTY\0",
        "CAN_ERR_OVERWRITE\0",
    };

    printf("%s:%d(%s)\r\n",__func__,rc,ary[rc]);    
}


void can_fd_txb_err_code(e_can_txb_result_t rc)
{
    char* ary[] = 
    {
        "CAN_TXB_TRANSMITTING\0",
        "CAN_TXB_ABORT_OVER\0",
        "CAN_TXB_END\0",
        "CAN_TXB_END_WITH_ABORT\0",
    };

    if (rc == CAN_TXB_ARG_ERROR)
    {
        printf("%s:%d(%s)\r\n",__func__,rc,"CAN_TXB_ARG_ERROR");
    }
    else
    {
        printf("%s:%d(%s)\r\n",__func__,rc,ary[rc]);   
    } 
}


void CAN_Get_Module_Version(void)
{
    unsigned short result; 
    result = R_CAN_GetVersion(); 

    (void)(result);
}

void CAN_Get_Timestamp_Counter(void)
{
    unsigned char value; 
    value = R_CAN_GetTSCounter(); 

    (void)(value);
}

void CAN_Get_Transceiver_Delay_Compensation(void)
{
    unsigned char result; 
    result = R_CAN_GetTDCResult(); 

    (void)(result);
}

void CAN_Get_Bus_Error_Flag(void)
{
    unsigned char errflag; 
    errflag = R_CAN_GetChBusErrFlag();  

    (void)(errflag);
}

void CAN_Get_Channel_Status(void)
{
    unsigned short sts; 
    sts = R_CAN_GetChStatus(); 

    (void)(sts);
}

void CAN_Read_RX_Data_Common_FIFO(void)
{
    e_can_err_t       rtn; 
    // st_can_rx_frame_t rx_frame; 
    can_length_t      length; 

    rtn = R_CAN_ReadCFIFO(&rx_frame, &length);  
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    }
    else 
    { 
        /* process for receive message */ 
    } 
}

void CAN_Read_RX_Data_FIFO(void)
{
    e_can_err_t       rtn; 
    // st_can_rx_frame_t rx_frame; 
    can_length_t      length; 

    rtn = R_CAN_ReadRXMB(CAN_RXFIFO0, &rx_frame, &length); 
    if (rtn == CAN_SUCCESS) 
    { 
        /* process for receive message */ 
        printf("CAN RX:len(%2d)\r\n",length);
    } 
    else
    {
        // can_fd_err_code(rtn); 
        return;
    }

}

void decode_rcv_frame(st_can_rx_frame_t * pFrame)
{
    /*
        ---- ID, RTR and IDE (2 words) ---- 
        uint32_t    ID   :29;   CAN ID                          
        uint32_t    :1;
        uint32_t    RTR  :1;    RTR  0:Data 1:Remote(Classical) 
        uint32_t    IDE  :1;    IDE  0:Standard 1:Extended      

        ---- Classical/FD, DLC (0.5 word) ---- 
        uint8_t     ESI:1;      ESI  0:Error Active 1:Error Passive
        uint8_t     BRS:1;      BRS  0:Only Nominal 1:Use Data Baud Rate
        uint8_t     FDF:1;      FDF  0:Classical 1:CAN-FD       
        uint8_t     :1;
        uint8_t     DLC:4;      DLC  0-15                                          

        ---- Label and Time Stamp (2.5 words) ---- 
        uint8_t     IFL:2;      Information label
        uint8_t     :6;
        uint16_t    LBL;        TX label 
        uint16_t    TS;         Time Stamp                      
    */ 

    printf("IDE:");
    (pFrame->Head.Bits.IDE == 0 ) ? (printf("Standard\r\n")) : (printf("Extend\r\n")) ;
    printf("RTR:");
    (pFrame->Head.Bits.RTR == 0 ) ? (printf("Data\r\n")) : (printf("Remote\r\n")) ;
    
    printf("IDL/ID Data:0x%08X\r\n" , pFrame->Head.Bits.ID);
    printf("DLC/DLC Data:0x%02X\r\n" , pFrame->Head.Bits.DLC);
    printf("LBL/Label Data:0x%02X\r\n" , pFrame->Head.Bits.LBL);
    printf("TS/Timestamp Data:0x%02X\r\n" , pFrame->Head.Bits.TS);

    printf("Data Byte:");
    // dump_buffer_hex((unsigned char *)pFrame->Data.DB,pFrame->Head.Bits.DLC);
    dump_buffer_hex((unsigned char *)rx_frame.Data.DB,pFrame->Head.Bits.DLC);

}


void CAN_Get_RX_Data_Message_Buffer(void)
{
    e_can_err_t       rtn; 
    // st_can_rx_frame_t rx_frame; 
    can_length_t      length; 
    // unsigned char i = 0;

    rtn = R_CAN_ReadRXMB(CAN_RXBUF0, &rx_frame, &length);  
    if (rtn != CAN_SUCCESS)
    {
        // can_fd_err_code(rtn); 
    } 
    else
    {
        #if 1
        printf("%s\r\n",__func__);
        decode_rcv_frame(&rx_frame);
        #else
        /* process for receive message */ 
        printf("dump CAN RX data START\r\n");
        for (i = 0 ; i < length ; i++)
        {
            printf("%2d:0x%02X\r\n",i,rx_frame.Data.DB[i]);
        }
        printf("dump CAN RX data END\r\n");        
        #endif
    }
}

void CAN_Read_TX_History_list(void)
{
    e_can_err_t       rtn; 
    st_can_txhist_t entry; 

    rtn = R_CAN_ReadTxHistory(&entry);  
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    } 
    else
    { 
        if (entry.txbuf_idx == CAN_TXBUF_NOT) 
        { 
            /* process for TX history of CFIFO */ 
        } 
        else 
        { 
            /* process for TX history of TXMB */ 
        } 
    } 
}

void CAN_Abort_TX_Data_Common_FIFO(void)
{
    e_can_err_t  rtn; 
    rtn = R_CAN_AbortCFIFO();   
 
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    } 

    (void)(rtn);
}

void CAN_Send_TX_Data_Common_FIFO(void)
{
    e_can_err_t rtn; 
    st_can_tx_frame_t s_tx_frame;
    unsigned char i = 0;

    s_tx_frame.Head.Bits.IDE   = CAN_IDE_STD_FORMAT; 
    s_tx_frame.Head.Bits.ID    = 0x7FEuL; 
    s_tx_frame.Head.Bits.RTR   = CAN_RTR_DATA_FRAME; 
    s_tx_frame.Head.Bits.FDCTR = CAN_FDCTR_CLASSICAL; 
    s_tx_frame.Head.Bits.DLC   = CAN_DLC_LEN8; 
    s_tx_frame.Head.Bits.THLEN   = CAN_THL_DISABLE; 
    s_tx_frame.Head.Bits.LBL  = 0u; 
    s_tx_frame.Head.Bits.IFL   = 0u; 
    for (i = 0u; i < CAN_DLC_LEN8; i++)  s_tx_frame.Data.DB[i] = 0x22u * (i + 1); 
    rtn = R_CAN_SendByCFIFO(&s_tx_frame); 
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    } 

    (void)(rtn);
}

void CAN_Get_TX_Data_Message_Buffer_Result(void)
{
    e_can_txb_result_t txb_rtn; 
    txb_rtn = R_CAN_GetTXMBResult(CAN_TXBUF0);  
    if (txb_rtn != CAN_TXB_END)
    {
        can_fd_txb_err_code(txb_rtn);
    } 

    (void)(txb_rtn);
}

void CAN_Abort_TX_Data_Message_Buffer(void)
{
    e_can_err_t rtn; 
    rtn = R_CAN_AbortTXMB(CAN_TXBUF0);    
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    }   

    (void)(rtn);
}

void CAN_Send_TX_Data_Message_Buffer_polling2(void)
{
    e_can_err_t rtn; 
    e_can_txb_result_t txb_rtn; 
    st_can_tx_frame_t s_tx_frame;
    unsigned char len = CAN_DLC_LEN8;
    unsigned char txbuf_idx = CAN_TXBUF0;

    static uint8_t cnt = 0;

    s_tx_frame.Head.Bits.IDE   = CAN_IDE_EXT_FORMAT; 
    s_tx_frame.Head.Bits.ID    = 0x110uL; //0x7FEuL; 
    s_tx_frame.Head.Bits.RTR   = CAN_RTR_DATA_FRAME;    //CAN_RTR_ANY_FRAME; 
    s_tx_frame.Head.Bits.FDCTR = CAN_FDCTR_CLASSICAL; 
    s_tx_frame.Head.Bits.DLC   = len; 
    s_tx_frame.Head.Bits.THLEN   = CAN_THL_DISABLE; 
    s_tx_frame.Head.Bits.LBL  = 0u; 
    s_tx_frame.Head.Bits.IFL   = 0u; 
 
    s_tx_frame.Data.DB[0] = 0x5A;
    s_tx_frame.Data.DB[1] = 0x5A;
    s_tx_frame.Data.DB[2] = 0x00 + cnt;
    s_tx_frame.Data.DB[3] = 0x01 + cnt;    
    s_tx_frame.Data.DB[4] = 0x02 + cnt;
    s_tx_frame.Data.DB[5] = 0x03 + cnt;
    s_tx_frame.Data.DB[6] = 0xA5;
    s_tx_frame.Data.DB[7] = 0xA5;

    rtn = R_CAN_SendByTXMB(txbuf_idx, &s_tx_frame);   

    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    }   
    else
    {
        printf("R_CAN_SendByTXMB success\r\n");
        txb_rtn = R_CAN_GetTXMBResult(txbuf_idx);   
        if (txb_rtn != CAN_TXB_END)
        {            
            can_fd_txb_err_code(txb_rtn);
        } 
        else
        {            
            printf("R_CAN_GetTXMBResult success\r\n");
        }
    }

    (void)(rtn);
    
    cnt = (cnt >= 0xFF) ? (0x00) : (cnt + 0x10);
}

void CAN_Send_TX_Data_Message_Buffer_polling1(void)
{
    e_can_err_t rtn; 
    e_can_txb_result_t txb_rtn; 
    st_can_tx_frame_t s_tx_frame;
    unsigned char len = CAN_DLC_LEN8;
    unsigned char txbuf_idx = CAN_TXBUF0;

    static uint8_t cnt = 0;

    s_tx_frame.Head.Bits.IDE   = CAN_IDE_STD_FORMAT; 
    s_tx_frame.Head.Bits.ID    = 0x55uL; //0x7FEuL; 
    s_tx_frame.Head.Bits.RTR   = CAN_RTR_DATA_FRAME;    //CAN_RTR_ANY_FRAME; 
    s_tx_frame.Head.Bits.FDCTR = CAN_FDCTR_CLASSICAL; 
    s_tx_frame.Head.Bits.DLC   = len; 
    s_tx_frame.Head.Bits.THLEN   = CAN_THL_DISABLE; 
    s_tx_frame.Head.Bits.LBL  = 0u; 
    s_tx_frame.Head.Bits.IFL   = 0u; 
 
    s_tx_frame.Data.DB[0] = 0x5A;
    s_tx_frame.Data.DB[1] = 0x5A;
    s_tx_frame.Data.DB[2] = 0x00 + cnt;
    s_tx_frame.Data.DB[3] = 0x01 + cnt;    
    s_tx_frame.Data.DB[4] = 0x02 + cnt;
    s_tx_frame.Data.DB[5] = 0x03 + cnt;
    s_tx_frame.Data.DB[6] = 0xA5;
    s_tx_frame.Data.DB[7] = 0xA5;

    rtn = R_CAN_SendByTXMB(txbuf_idx, &s_tx_frame);   

    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    }   
    else
    {
        printf("R_CAN_SendByTXMB success\r\n");
        txb_rtn = R_CAN_GetTXMBResult(txbuf_idx);   
        if (txb_rtn != CAN_TXB_END)
        {            
            can_fd_txb_err_code(txb_rtn);
        } 
        else
        {            
            printf("R_CAN_GetTXMBResult success\r\n");
        }
    }

    (void)(rtn);
    
    cnt++;
}

void CAN_Send_TX_Data_Message_Buffer(void)
{
    e_can_err_t rtn; 
    e_can_txb_result_t txb_rtn; 
    st_can_tx_frame_t s_tx_frame;
    unsigned char len = CAN_DLC_LEN8;
    unsigned char txbuf_idx = CAN_TXBUF0;

    s_tx_frame.Head.Bits.IDE   = CAN_IDE_EXT_FORMAT; 
    s_tx_frame.Head.Bits.ID    = 0x120uL; //0x7FEuL; 
    s_tx_frame.Head.Bits.RTR   = CAN_RTR_DATA_FRAME;    //CAN_RTR_ANY_FRAME; 
    s_tx_frame.Head.Bits.FDCTR = CAN_FDCTR_CLASSICAL; 
    s_tx_frame.Head.Bits.DLC   = len; 
    s_tx_frame.Head.Bits.THLEN   = CAN_THL_DISABLE; 
    s_tx_frame.Head.Bits.LBL  = 0u; 
    s_tx_frame.Head.Bits.IFL   = 0u; 
 
    s_tx_frame.Data.DB[0] = 0x12;
    s_tx_frame.Data.DB[1] = 0x34;
    s_tx_frame.Data.DB[2] = 0x56;
    s_tx_frame.Data.DB[3] = 0x78;    
    s_tx_frame.Data.DB[4] = 0x9A;
    s_tx_frame.Data.DB[5] = 0xBC;
    s_tx_frame.Data.DB[6] = 0xDE;
    s_tx_frame.Data.DB[7] = 0xF0;

    rtn = R_CAN_SendByTXMB(txbuf_idx, &s_tx_frame);   

    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    }   
    else
    {
        printf("R_CAN_SendByTXMB success\r\n");
        txb_rtn = R_CAN_GetTXMBResult(txbuf_idx);   
        if (txb_rtn != CAN_TXB_END)
        {            
            can_fd_txb_err_code(txb_rtn);
        } 
        else
        {            
            printf("R_CAN_GetTXMBResult success\r\n");
        }
    }

    (void)(rtn);
}


void CAN_Bus_EntrySleepMode(void)
{
    e_can_err_t rtn; 
    rtn = R_CAN_Sleep(CAN_WUP_UNUSE);  
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    } 

    (void)(rtn);
}

void CAN_Bus_Reset(void)
{
    e_can_err_t rtn; 
    rtn = R_CAN_StopComm();  
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    } 

    (void)(rtn);
}

void CAN_Transition_Check(void)
{
    e_can_err_t rtn; 

    rtn = CAN_ERR_WAITING; 
    while (CAN_ERR_WAITING == rtn) 
    { 
        rtn = R_CAN_StartComm(); 
    } 
     
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    } 
    else
    { 
        while (0x0080u != R_CAN_GetChStatus()); /* Wait until communication is ready */ 
    } 

    printf("%s ready\r\n",__func__);
}

e_can_err_t CAN_SetRX_Rule(void)
{
    e_can_err_t rtn; 
    st_can_filter_t filter; 
    st_can_filter_opt_t opt; 
    unsigned char rxbuf_idx = 0;

    filter.IDE_TYPE = CAN_IDE_ANY_FORMAT; 
    filter.RTR_TYPE = CAN_RTR_DATA_FRAME; 
    filter.ID = 0x123u; //0x700u; 
    filter.ID_MASK = CAN_MATCH_NO_ID_BIT; 
    opt.DLC = CAN_DLC_LEN8; 
    opt.LB  = CAN_AFL_LB_NOT_LOOPBACK; 
    opt.LBL = 0xA5A5u; 
    opt.IFL = 2u; 
    rtn = R_CAN_AddRxRule(&filter, CAN_STORE_RM , rxbuf_idx, &opt); //CAN_STORE_RM 
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    } 

    return rtn;
}

/*
    check AN : RS-CANFD Lite Configuration, Reception and Transmission
    Table 1.4 Communication Speed Calculation and Examples of Communication Speed (Nominal) 
    Table 1.5 Communication Speed Calculation and Examples of Communication Speed 
    (Nominal and Data) 
    Table 1.6 Example of Bit Timing Settings 
    
    @40MHz ,
    when 500k bps : 8Tq(10) , 20Tq(4) 
    when 250k bps : 8Tq(20) , 20Tq(8) 

    - 8Tq(10) , sample rate 75%  : SS:1 , TSEG1:5 , TSEG2:2 , SJW:1
    - 20Tq(4) , sample rate 70%  : SS:1 , TSEG1:13 , TSEG2:6 , SJW:1 
    - 20Tq(4) , sample rate 80%  : SS:1 , TSEG1:15 , TSEG2:4 , SJW:3     

    set 250K @ sample rate 75 
*/
void CAN_Speed_Config(void)
{

}

void CAN_Bus_Config(void)
{
    e_can_err_t rtn;
    uint16_t result; 

    // RL78 F24 EVB - CAN transceiver BD41044FJ-C , PIN#8 , pull low
    P1_bit.no2 = 0;
    result = R_CAN_GetVersion(); 
    printf("R_CAN_GetVersion:0x%04X\r\n",result);

    R_CAN_Create();

    rtn = R_CAN_StopComm();  
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    } 

    rtn = R_CAN_SetConfig(); 
    if (rtn != CAN_SUCCESS)
    {
        printf("R_CAN_SetConfig fail(0x%02X)\r\n",rtn);
        return;
    }

    rtn = CAN_SetRX_Rule();
    if (rtn != CAN_SUCCESS)
    {
        printf("CAN_SetRX_Rule fail(0x%02X)\r\n",rtn);
        return;
    }

    CAN_Transition_Check();

}


void my_can_glb_rxfifo_callback(uint16_t arg)
{
    printf("%s:%d(0x%04X)\r\n",__func__,arg,arg);  

}

void my_can_glb_rxmb_callback(uint16_t arg)
{
    e_can_err_t       rtn; 
    // st_can_rx_frame_t rx_frame; 
    can_length_t      length; 
    // unsigned char i = 0;

    rtn = R_CAN_ReadRXMBInHandler(CAN_RXBUF0, &rx_frame, &length); 
    if (rtn != CAN_SUCCESS)
    {
        can_fd_err_code(rtn); 
    } 
    else 
    { 
        #if 1
        printf("%s:%d(0x%04X)\r\n",__func__,arg,arg);  
        decode_rcv_frame(&rx_frame);
        #else
        /* process for receive message */ 
        printf("dump CAN RX data START\r\n");
        for (i = 0 ; i < length ; i++)
        {
            printf("%2d:0x%02X\r\n",i,rx_frame.Data.DB[i]);
        }
        printf("dump CAN RX data END\r\n");
        #endif
    } 


}

void my_can_glb_error_callback(uint16_t arg)
{
    printf("%s:%d(0x%04X)\r\n",__func__,arg,arg);  

}

void my_can_ch0_transmit_callback(uint16_t arg)
{
    printf("%s:%d(0x%04X)\r\n",__func__,arg,arg);  

}

void my_can_ch0_cfifo_rx_callback(uint16_t arg)
{
    printf("%s:%d(0x%04X)\r\n",__func__,arg,arg);  

}

void my_can_ch0_error_callback(uint16_t arg)
{
    char* p;
    switch(arg)
    {
        case CAN_SOURCE_CH_ERR_BUS:
            p = "ERR_BUS";
            break;
        case CAN_SOURCE_CH_ERR_WARNING:
            p = "ERR_WARNING";
            break;
        case CAN_SOURCE_CH_ERR_PASSIVE:
            p = "ERR_PASSIVE";
            break;
        case CAN_SOURCE_CH_ERR_BO_ENTRY:
            p = "ERR_BO_ENTRY";
            break;
        case CAN_SOURCE_CH_ERR_BO_RCVR:
            p = "ERR_BO_RCVR";
            break;
        case CAN_SOURCE_CH_ERR_OVERLOAD:
            p = "ERR_OVERLOAD";
            break;
        case CAN_SOURCE_CH_ERR_BUS_LOCK:
            p = "ERR_BUS_LOCK";
            break;
        case CAN_SOURCE_CH_ERR_ARB_LOST:
            p = "ERR_ARB_LOST";
            break;
        case CAN_SOURCE_CH_ERR_TDCV:
            p = "ERR_TDCV";
            break;
    }

    printf("%s:%d(%s)\r\n",__func__,arg,p);    
    // printf("%s:%d(0x%04X)\r\n",__func__,arg,arg);  

}

