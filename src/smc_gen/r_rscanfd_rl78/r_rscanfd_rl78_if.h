/**********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO
 * THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2022 Renesas Electronics Corporation. All rights reserved.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : r_rscanfd_rl78_if.h
 * Description  : Interface file to be included by the user when using the RS-CANFD driver module.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * History : DD.MM.YYYY Version  Description
 *         : 20.04.2022 1.00     Initial Release
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "platform.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#ifndef R_RSCANFD_RL78_IF_H
#define R_RSCANFD_RL78_IF_H

/* Version Number of API */
#define CAN_VERSION_MAJOR           (1)             /* Major version */
#define CAN_VERSION_MINOR           (0)             /* Minor version */

/* for can_rxfifo_t */
#define CAN_RXFIFO0                 (0u)            /* RX FIFO 0 */
#define CAN_RXFIFO1                 (1u)            /* RX FIFO 1 */

/* for can_txbuf_t */
#define CAN_TXBUF0                  (0u)            /* TX message buffer 0 */
#define CAN_TXBUF1                  (1u)            /* TX message buffer 1 */
#define CAN_TXBUF2                  (2u)            /* TX message buffer 2 */
#define CAN_TXBUF3                  (3u)            /* TX message buffer 3 */
#define CAN_TXBUF_NOT               (0xFFu)         /* Not TX message buffer (CFIFO) */

/* for can_rxbuf_t */
#define CAN_RXBUF0                  (0u)            /* RX message buffer 0  */
#define CAN_RXBUF1                  (1u)            /* RX message buffer 1  */
#define CAN_RXBUF2                  (2u)            /* RX message buffer 2  */
#define CAN_RXBUF3                  (3u)            /* RX message buffer 3  */
#define CAN_RXBUF4                  (4u)            /* RX message buffer 4  */
#define CAN_RXBUF5                  (5u)            /* RX message buffer 5  */
#define CAN_RXBUF6                  (6u)            /* RX message buffer 6  */
#define CAN_RXBUF7                  (7u)            /* RX message buffer 7  */
#define CAN_RXBUF8                  (8u)            /* RX message buffer 8  */
#define CAN_RXBUF9                  (9u)            /* RX message buffer 9  */
#define CAN_RXBUF10                 (10u)           /* RX message buffer 10 */
#define CAN_RXBUF11                 (11u)           /* RX message buffer 11 */
#define CAN_RXBUF12                 (12u)           /* RX message buffer 12 */
#define CAN_RXBUF13                 (13u)           /* RX message buffer 13 */
#define CAN_RXBUF14                 (14u)           /* RX message buffer 14 */
#define CAN_RXBUF15                 (15u)           /* RX message buffer 15 */
#define CAN_RXBUF_INVALID           (0xFFu)         /* Invalid RX message buffer number */

/* for can_storage_t (storage of R_CAN_AddRxRule()) */
#define CAN_STORE_RM                (0x8000u)       /* only RX Buffer            */
#define CAN_STORE_RF0               (0x0001u)       /* only RX FIFO0             */
#define CAN_STORE_RF1               (0x0002u)       /* only RX FIFO1             */
#define CAN_STORE_CF                (0x0100u)       /* only Common FIFO          */
#define CAN_STORE_RM_AND_RF0        (0x8001u)       /* RX Buffer and RX FIFO0    */
#define CAN_STORE_RM_AND_RF1        (0x8002u)       /* RX Buffer and RX FIFO1    */
#define CAN_STORE_RM_AND_CF         (0x8100u)       /* RX Buffer and Common FIFO */
#define CAN_STORE_RF0_AND_RF1       (0x0003u)       /* RX FIFO0 and RX FIFO1     */
#define CAN_STORE_RF0_AND_CF        (0x0101u)       /* RX FIFO0 and Common FIFO  */
#define CAN_STORE_RF1_AND_CF        (0x0102u)       /* RX FIFO1 and Common FIFO  */

/* for THLEN (st_can_tx_frame_t) */
#define CAN_THLEN_NOT_STORE         (0u)            /* Entry will not be stred in THL */
#define CAN_THLEN_STORE             (1u)            /* Entry will be stred in THL     */

/* for RTR (st_can_tx_frame_t/st_can_rx_frame_t), RTR_TYPE (st_can_filter_t) */
#define CAN_RTR_DATA_FRAME          (0u)            /* RTR for Data Frame                    */
#define CAN_RTR_CLASSICAL_RMT       (1u)            /* RTR for Remete Frame (only classical) */
#define CAN_RTR_ANY_FRAME           (2u)            /* any frame (use only RTR_TYPE)         */

/* for IDE (st_can_tx_frame_t/st_can_rx_frame_t), IDE_TYPE (st_can_filter_t) */
#define CAN_IDE_STD_FORMAT          (0u)            /* IDE for Standard identifier format */
#define CAN_IDE_EXT_FORMAT          (1u)            /* IDE for Extended identifier format */
#define CAN_IDE_ANY_FORMAT          (2u)            /* any format (use only IDE_TYPE)     */

/* for FDCTR (st_can_tx_frame_t) (ESI is effective when ESIC=1 and not error passive) */
#define CAN_FDCTR_CLASSICAL         (0x00u)         /* Classical Frame                            */
#define CAN_FDCTR_FD                (0x06u)         /* FD Frame (use Data Baud Rate)              */
#define CAN_FDCTR_FD_NMNL           (0x04u)         /* FD Frame (only Nominal Baud Rate)          */
#define CAN_FDCTR_FD_ESI            (0x07u)         /* FD Frame (use Data Baud Rate and ESI)      */
#define CAN_FDCTR_FD_NMNL_ESI       (0x05u)         /* FD Frame (only Nominal Baud Rate, use ESI) */

/* for ESI (st_can_rx_frame_t) */
#define CAN_ESI_ERR_ACTIVE          (0u)            /* Error Active Node  */
#define CAN_ESI_ERR_PASSIVE         (1u)            /* Error Passive Node */

/* for BRS (st_can_rx_frame_t) */
#define CAN_BRS_ONLY_NOMINAL        (0u)            /* Only Nominal Baud Rate   */
#define CAN_BRS_USE_DATA_RATE       (1u)            /* Use Data Baud Rate       */

/* for FDF (st_can_rx_frame_t) */
#define CAN_FDF_CLASSICAL           (0u)            /* Classical Frame  */
#define CAN_FDF_FD                  (1u)            /* FD Frame         */

/* for DLC (st_can_tx_frame_t/st_can_rx_frame_t, st_can_filter_opt_t) */
#define CAN_DLC_LEN0                (0u)            /* DLC for  0 byte  */
#define CAN_DLC_LEN1                (1u)            /* DLC for  1 byte  */
#define CAN_DLC_LEN2                (2u)            /* DLC for  2 bytes */
#define CAN_DLC_LEN3                (3u)            /* DLC for  3 bytes */
#define CAN_DLC_LEN4                (4u)            /* DLC for  4 bytes */
#define CAN_DLC_LEN5                (5u)            /* DLC for  5 bytes */
#define CAN_DLC_LEN6                (6u)            /* DLC for  6 bytes */
#define CAN_DLC_LEN7                (7u)            /* DLC for  7 bytes */
#define CAN_DLC_LEN8                (8u)            /* DLC for  8 bytes */
#define CAN_FD_DLC_LEN12            (9u)            /* DLC for 12 bytes (only FD frame) */
#define CAN_FD_DLC_LEN16            (10u)           /* DLC for 16 bytes (only FD frame) */
#define CAN_FD_DLC_LEN20            (11u)           /* DLC for 20 bytes (only FD frame) */
#define CAN_FD_DLC_LEN24            (12u)           /* DLC for 24 bytes (only FD frame) */
#define CAN_FD_DLC_LEN32            (13u)           /* DLC for 32 bytes (only FD frame) */
#define CAN_FD_DLC_LEN48            (14u)           /* DLC for 48 bytes (only FD frame) */
#define CAN_FD_DLC_LEN64            (15u)           /* DLC for 64 bytes (only FD frame) */

/* for THLEN (st_can_tx_frame_t) */
#define CAN_THL_DISABLE             (0u)            /* Not use TX history */
#define CAN_THL_ENABLE              (1u)            /* Use TX history     */

/* for ID_MASK (st_can_filter_t) */
#define CAN_MATCH_NO_ID_BIT         (0x0uL)         /* No ID bit is compared    */
#define CAN_MATCH_ALL_ID_BIT        (0x1FFFFFFFuL)  /* All ID bits are compared */

/* for LB (st_can_filter_opt_t) */
#define CAN_AFL_LB_NOT_LOOPBACK     (0u)            /* Store received frames              */
#define CAN_AFL_LB_LOOPBACK         (1u)            /* Store trasmitted frames (Loopback) */

/* for tx_type (st_can_txhist_t) */
#define CAN_TXTYPE_TXBUF            (0u)            /* Store received frames              */
#define CAN_TXTYPE_CFIFO            (1u)            /* Store received frames              */

/* for wup of R_CAN_Sleep() */
#define CAN_WUP_UNUSE               (0u)            /* Not use CAN wakeup */
#define CAN_WUP_USE                 (1u)            /* Use CAN wakeup     */

/* Interrupt Sources */
#define CAN_SOURCE_NOTHING          (0u)            /* No interrupt source */

/* Interrupt Sources: INTRCANGRFR */
#define CAN_SOURCE_RECV_RF0         (1u)            /* Interrupt of RX FIFO0 */
#define CAN_SOURCE_RECV_RF1         (2u)            /* Interrupt of RX FIFO1 */

/* Interrupt Sources: INTRCANGERR */
#define CAN_SOURCE_GERR_DLC         (0x0001u)       /* DLC Error                   */
#define CAN_SOURCE_GERR_MLT         (0x0002u)       /* Message Lost                */
#define CAN_SOURCE_GERR_MLT_RF0     (0x0010u)       /* Message Lost of RX FIFO0    */
#define CAN_SOURCE_GERR_MLT_RF1     (0x0020u)       /* Message Lost of RX FIFO1    */
#define CAN_SOURCE_GERR_MLT_CF      (0x0100u)       /* Message Lost of Common FIFO */
#define CAN_SOURCE_GERR_TXH_LOST    (0x0004u)       /* TX History List Entry Lost  */
#define CAN_SOURCE_GERR_PLD_OVF     (0x0008u)       /* Payload Overflow            */

/* Interrupt Sources: INTRCAN0TRM */
#define CAN_SOURCE_TRMC_TM0         (0x0001u)       /* Tramsmit Completion of TX Buffer 0 */
#define CAN_SOURCE_TRMC_TM1         (0x0002u)       /* Tramsmit Completion of TX Buffer 1 */
#define CAN_SOURCE_TRMC_TM2         (0x0004u)       /* Tramsmit Completion of TX Buffer 2 */
#define CAN_SOURCE_TRMC_TM3         (0x0008u)       /* Tramsmit Completion of TX Buffer 3 */
#define CAN_SOURCE_TRMA_TM0         (0x0010u)       /* Tramsmit Abort of TX Buffer 0      */
#define CAN_SOURCE_TRMA_TM1         (0x0020u)       /* Tramsmit Abort of TX Buffer 1      */
#define CAN_SOURCE_TRMA_TM2         (0x0040u)       /* Tramsmit Abort of TX Buffer 2      */
#define CAN_SOURCE_TRMA_TM3         (0x0080u)       /* Tramsmit Abort of TX Buffer 3      */
#define CAN_SOURCE_TRM_CF           (0x0100u)       /* TX interrupt of Common FIFO        */
#define CAN_SOURCE_TRM_TXHIST       (0x1000u)       /* TX interrupt of Common FIFO        */

/* Interrupt Sources: INTRCAN0CFR */
#define CAN_SOURCE_RECV_CF          (1u)            /* RX interrupt of Common FIFO */

/* Interrupt Sources: INTRCAN0ERR */
#define CAN_SOURCE_CH_ERR_BUS       (0x0001u)       /* Bus Error        */
#define CAN_SOURCE_CH_ERR_WARNING   (0x0002u)       /* Error Warning    */
#define CAN_SOURCE_CH_ERR_PASSIVE   (0x0004u)       /* Error Passive    */
#define CAN_SOURCE_CH_ERR_BO_ENTRY  (0x0008u)       /* Bus-Off Entry    */
#define CAN_SOURCE_CH_ERR_BO_RCVR   (0x0010u)       /* Bus-Off Recovery */
#define CAN_SOURCE_CH_ERR_OVERLOAD  (0x0020u)       /* Overload         */
#define CAN_SOURCE_CH_ERR_BUS_LOCK  (0x0040u)       /* Bus Lock         */
#define CAN_SOURCE_CH_ERR_ARB_LOST  (0x0080u)       /* Arbitration Lost */
#define CAN_SOURCE_CH_ERR_TDCV      (0x0100u)       /* Transceiver Delay Compensation Violation */

/* Interrupt Sources: INTRCAN0WUP */
#define CAN_SOURCE_WAKEUP           (1u)            /* Wakeup interrupt */

/* Max definition */
#define CAN_MAX_TXBUF_NUM           (4u)            /* Number of TX buffers            */
#define CAN_MAX_RXBUF_NUM           (16u)           /* Number of RX buffers            */
#define CAN_MAX_RXFIFO_NUM          (2u)            /* Number of RX FIFOs              */
#define CAN_MAX_RXRULE_NUM          (16u)           /* Number of Rx rules              */
#define CAN_MAX_BYTE_NUM            (64u)           /* CAN data length byte (64 bytes) */
#define CAN_MAX_WORD_NUM            (32u)           /* CAN data length word (64 bytes) */
#define CAN_HALF_RXBUF_NUM          (8u)            /* Half number of RX buffers       */

/* Frame information words */
#define CAN_TX_HEAD_WORD_NUM        (4u)            /* Number of TX frame information words */
#define CAN_RX_HEAD_WORD_NUM        (5u)            /* Number of RX frame information words */

/**********************************************************************************************************************
 Global Typedef definitions
 *********************************************************************************************************************/
/* Return error codes */
typedef enum
{
    CAN_SUCCESS,                        /* Normal completion                    */
    CAN_SUCCESS_WITH_LOST,              /* Normal completion with message lost  */
    CAN_ERR_WAITING,                    /* Waiting for transition to complete   */
    CAN_ERR_INVALID_ARG,                /* Invalid argument passed to function  */
    CAN_ERR_INVALID_MODE,               /* Invalid mode passed to function      */
    CAN_ERR_ILLEGAL_STS,                /* Failure to change CAN status         */
    CAN_ERR_BUF_BUSY,                   /* TX buffer or RX buffer is busy       */
    CAN_ERR_BUF_FULL,                   /* RX rule or Common FIFO is full       */
    CAN_ERR_BUF_EMPTY,                  /* RX buffer or RX/Common FIFO is empty */
    CAN_ERR_OVERWRITE                   /* RX buffer is overwritten             */
} e_can_err_t;

/* Return tx buffer result */
typedef enum
{
    CAN_TXB_TRANSMITTING   = 0u,        /* Transmission ongoing    */
    CAN_TXB_ABORT_OVER     = 1u,        /* Transmission aborted    */
    CAN_TXB_END            = 2u,        /* Transmission successful */
    CAN_TXB_END_WITH_ABORT = 3u,        /* Transmission successful with abort request */
    CAN_TXB_ARG_ERROR      = -1,        /* Argument error          */
} e_can_txb_result_t;

/* Argument type */
typedef uint8_t     can_rxfifo_t;       /* RX FIFO index   */
typedef uint8_t     can_txbuf_t;        /* TX buffer index */
typedef uint8_t     can_rxbuf_t;        /* RX buffer index */
typedef uint8_t     can_length_t;       /* CAN data length */
typedef uint16_t    can_storage_t;      /* RX storage type */

/* CAN transmit frame information */
typedef union
{
    uint16_t        Word[CAN_TX_HEAD_WORD_NUM]; /* Word access */
    struct
    {
        /* ---- ID, THLEN, RTR and IDE (2 words) ---- */
        uint32_t    ID   :29;   /* CAN ID                          */
        uint32_t    THLEN:1;    /* THLEN  if 1 then store in THL   */
        uint32_t    RTR  :1;    /* RTR  0:Data 1:Remote(Classical) */
        uint32_t    IDE  :1;    /* IDE  0:Standard 1:Extended      */

        /* ---- Classical/FD, DLC (0.5 word) ---- */
        uint8_t     FDCTR:3;    /* FDF/BRS/ESI                     */
        uint8_t     :1;
        uint8_t     DLC  :4;    /* DLC  0-15                       */

        /* ---- Label and Time Stamp (1.5 words) ---- */
        uint8_t     IFL:2;      /* Information label               */
        uint8_t     :6;
        uint16_t    LBL;        /* TX label                        */
    } Bits;                                         /* Bit access  */
} u_can_tx_head_t;

/* CAN receive frame information */
typedef union
{
    uint16_t        Word[CAN_RX_HEAD_WORD_NUM]; /* Word access */
    struct
    {
        /* ---- ID, RTR and IDE (2 words) ---- */
        uint32_t    ID :29;     /* CAN ID                          */
        uint32_t    :1;
        uint32_t    RTR:1;      /* RTR  0:Data 1:Remote(Classical) */
        uint32_t    IDE:1;      /* IDE  0:Standard 1:Extended      */

        /* ---- Classical/FD, DLC (0.5 words) ---- */
        uint8_t     ESI:1;      /* ESI  0:Error Active 1:Error Passive */
        uint8_t     BRS:1;      /* BRS  0:Only Nominal 1:Use Data Baud Rate */
        uint8_t     FDF:1;      /* FDF  0:Classical 1:CAN-FD       */
        uint8_t     :1;
        uint8_t     DLC:4;      /* DLC  0-15                       */

        /* ---- Label and Time Stamp (2.5 words) ---- */
        uint8_t     IFL:2;      /* Information label               */
        uint8_t     :6;
        uint16_t    LBL;        /* RX label                        */
        uint16_t    TS;         /* Time Stamp                      */
    } Bits;                                         /* Bit access  */
} u_can_rx_head_t;

/* CAN payload type */
typedef union
{
    uint16_t DW[CAN_MAX_WORD_NUM];  /* Data Word */
    uint8_t  DB[CAN_MAX_BYTE_NUM];  /* Data Byte */
} u_can_data_t;

/* CAN transmit frame */
typedef struct
{
    u_can_tx_head_t Head;           /* frame information */
    u_can_data_t    Data;           /* frame data        */
} st_can_tx_frame_t;

/* CAN receive frame */
typedef struct
{
    u_can_rx_head_t Head;           /* frame information */
    u_can_data_t    Data;           /* frame data        */
} st_can_rx_frame_t;

/* RX rule filter */
typedef struct
{
    uint32_t ID     :29;    /* CAN ID                                      */
    uint32_t :3;
    uint32_t ID_MASK:29;    /* CAN ID Mask                                 */
    uint32_t :3;
    uint8_t  RTR_TYPE;      /* RTR_TYPE  0:Data 1:Remote (classical) 2:Any */
    uint8_t  IDE_TYPE;      /* IDE_TYPE  0:Standard 1:Extend 2:Any         */
} st_can_filter_t;

/* RX rule filter option */
typedef struct
{
    uint16_t LBL;           /* RX label                     */
    uint16_t IFL :2;        /* Information label            */
    uint16_t DLC :4;        /* DLC (Effective if DCE=1)     */
    uint16_t LB  :1;        /* LB  0:RX frames  1:TX frames (Effective if MME=1) */
    uint16_t :9;
} st_can_filter_opt_t;

/* TX history list entry */
typedef struct
{
    can_txbuf_t     txbuf_idx;  /* TX buffer index      */
    uint16_t        TS;         /* Time Stamp           */
    uint16_t        LBL;        /* TX label             */
    uint16_t        IFL :2;     /* Information label    */
    uint16_t        :14;
} st_can_txhist_t;

/**********************************************************************************************************************
 Exported global variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/
extern void         R_CAN_Create (void);

extern e_can_err_t  R_CAN_SetConfig (void);

extern e_can_err_t  R_CAN_AddRxRule (const st_can_filter_t * p_filter,
    const can_storage_t         storage,
    const can_rxbuf_t           rxbuf_idx,
    const st_can_filter_opt_t * p_opt);

extern e_can_err_t  R_CAN_StartComm (void);

extern e_can_err_t  R_CAN_StopComm (void);

extern e_can_err_t  R_CAN_Sleep (const uint8_t wup);

extern e_can_err_t  R_CAN_SendByTXMB (const can_txbuf_t txbuf_idx,
    const st_can_tx_frame_t * p_frame);

extern e_can_err_t  R_CAN_AbortTXMB (const can_txbuf_t txbuf_idx);

extern e_can_txb_result_t
                    R_CAN_GetTXMBResult (const can_txbuf_t txbuf_idx);

extern e_can_err_t  R_CAN_SendByCFIFO (const st_can_tx_frame_t * p_frame);

extern e_can_err_t  R_CAN_AbortCFIFO (void);

extern e_can_err_t  R_CAN_ReadTxHistory(st_can_txhist_t *p_entry);

extern uint8_t      R_CAN_GetTDCResult (void);

extern e_can_err_t  R_CAN_ReadRXMB (const can_rxbuf_t rxbuf_idx,
    st_can_rx_frame_t * p_frame,
    can_length_t      * p_length);

extern e_can_err_t  R_CAN_ReadRXMBInHandler (const can_rxbuf_t   rxbuf_idx,
    st_can_rx_frame_t * p_frame,
    can_length_t      * p_length);

extern e_can_err_t  R_CAN_ReadRXFIFO (const can_rxfifo_t  rxfifo_idx,
    st_can_rx_frame_t * p_frame,
    can_length_t      * p_length);

extern e_can_err_t  R_CAN_ReadCFIFO (st_can_rx_frame_t * p_frame,
    can_length_t * p_length);

extern uint16_t     R_CAN_GetChStatus (void);

extern uint8_t      R_CAN_GetChBusErrFlag (void);

extern uint16_t     R_CAN_GetTSCounter (void);

extern uint16_t     R_CAN_GetVersion (void);

#endif /* R_RSCANFD_RL78_IF_H */
