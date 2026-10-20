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
 * File Name    : r_rscanfd_rl78_private.h
 * Description  : Internal structures and definitions for the CAN driver.
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
#include "r_rscanfd_rl78_private_compiler.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#ifndef R_RSCANFD_RL78_PRIVATE_H
#define R_RSCANFD_RL78_PRIVATE_H

/* 16bit value with 1bit ON */
#define CAN_U16_BIT0_ON             (0x0001u)               /* only bit0 ON  */
#define CAN_U16_BIT1_ON             (0x0002u)               /* only bit1 ON  */
#define CAN_U16_BIT2_ON             (0x0004u)               /* only bit2 ON  */
#define CAN_U16_BIT3_ON             (0x0008u)               /* only bit3 ON  */
#define CAN_U16_BIT4_ON             (0x0010u)               /* only bit4 ON  */
#define CAN_U16_BIT5_ON             (0x0020u)               /* only bit5 ON  */
#define CAN_U16_BIT6_ON             (0x0040u)               /* only bit6 ON  */
#define CAN_U16_BIT7_ON             (0x0080u)               /* only bit7 ON  */
#define CAN_U16_BIT8_ON             (0x0100u)               /* only bit8 ON  */
#define CAN_U16_BIT9_ON             (0x0200u)               /* only bit9 ON  */
#define CAN_U16_BIT10_ON            (0x0400u)               /* only bit10 ON */
#define CAN_U16_BIT11_ON            (0x0800u)               /* only bit11 ON */
#define CAN_U16_BIT12_ON            (0x1000u)               /* only bit12 ON */
#define CAN_U16_BIT13_ON            (0x2000u)               /* only bit13 ON */
#define CAN_U16_BIT14_ON            (0x4000u)               /* only bit14 ON */
#define CAN_U16_BIT15_ON            (0x8000u)               /* only bit15 ON */

/* Convert 1bit value to 16bit value */
#define CAN_U16_BIT0(x)             (((x) & 1u) << 0u)      /* Set bit0  to multiple bit */
#define CAN_U16_BIT1(x)             (((x) & 1u) << 1u)      /* Set bit1  to multiple bit */
#define CAN_U16_BIT2(x)             (((x) & 1u) << 2u)      /* Set bit2  to multiple bit */
#define CAN_U16_BIT3(x)             (((x) & 1u) << 3u)      /* Set bit3  to multiple bit */
#define CAN_U16_BIT4(x)             (((x) & 1u) << 4u)      /* Set bit4  to multiple bit */
#define CAN_U16_BIT5(x)             (((x) & 1u) << 5u)      /* Set bit5  to multiple bit */
#define CAN_U16_BIT6(x)             (((x) & 1u) << 6u)      /* Set bit6  to multiple bit */
#define CAN_U16_BIT7(x)             (((x) & 1u) << 7u)      /* Set bit7  to multiple bit */
#define CAN_U16_BIT8(x)             (((x) & 1u) << 8u)      /* Set bit8  to multiple bit */
#define CAN_U16_BIT9(x)             (((x) & 1u) << 9u)      /* Set bit9  to multiple bit */
#define CAN_U16_BIT10(x)            (((x) & 1u) << 10u)     /* Set bit10 to multiple bit */
#define CAN_U16_BIT11(x)            (((x) & 1u) << 11u)     /* Set bit11 to multiple bit */
#define CAN_U16_BIT12(x)            (((x) & 1u) << 12u)     /* Set bit12 to multiple bit */
#define CAN_U16_BIT13(x)            (((x) & 1u) << 13u)     /* Set bit13 to multiple bit */
#define CAN_U16_BIT14(x)            (((x) & 1u) << 14u)     /* Set bit14 to multiple bit */
#define CAN_U16_BIT15(x)            (((x) & 1u) << 15u)     /* Set bit15 to multiple bit */

/* Convert multiple bit value to 16bit value */
#define CAN_U16_POS0(x, mask)       (((x) & (mask)) << 0u)  /* Set bit0  */
#define CAN_U16_POS1(x, mask)       (((x) & (mask)) << 1u)  /* Set bit1  */
#define CAN_U16_POS2(x, mask)       (((x) & (mask)) << 2u)  /* Set bit2  */
#define CAN_U16_POS3(x, mask)       (((x) & (mask)) << 3u)  /* Set bit3  */
#define CAN_U16_POS4(x, mask)       (((x) & (mask)) << 4u)  /* Set bit4  */
#define CAN_U16_POS5(x, mask)       (((x) & (mask)) << 5u)  /* Set bit5  */
#define CAN_U16_POS6(x, mask)       (((x) & (mask)) << 6u)  /* Set bit6  */
#define CAN_U16_POS7(x, mask)       (((x) & (mask)) << 7u)  /* Set bit7  */
#define CAN_U16_POS8(x, mask)       (((x) & (mask)) << 8u)  /* Set bit8  */
#define CAN_U16_POS9(x, mask)       (((x) & (mask)) << 9u)  /* Set bit9  */
#define CAN_U16_POS10(x, mask)      (((x) & (mask)) << 10u) /* Set bit10 */
#define CAN_U16_POS11(x, mask)      (((x) & (mask)) << 11u) /* Set bit11 */
#define CAN_U16_POS12(x, mask)      (((x) & (mask)) << 12u) /* Set bit12 */
#define CAN_U16_POS13(x, mask)      (((x) & (mask)) << 13u) /* Set bit13 */
#define CAN_U16_POS14(x, mask)      (((x) & (mask)) << 14u) /* Set bit14 */
#define CAN_U16_POS15(x, mask)      (((x) & (mask)) << 15u) /* Set bit15 */

/* Configuration mask value */
#define CAN_MASK_W1                 (0x01u)                 /* width 1 bit  */
#define CAN_MASK_W2                 (0x03u)                 /* width 2 bits */
#define CAN_MASK_W3                 (0x07u)                 /* width 3 bits */
#define CAN_MASK_W4                 (0x0Fu)                 /* width 4 bits */
#define CAN_MASK_W5                 (0x1Fu)                 /* width 5 bits */
#define CAN_MASK_W6                 (0x3Fu)                 /* width 6 bits */
#define CAN_MASK_W7                 (0x7Fu)                 /* width 7 bits */
#define CAN_MASK_W8                 (0xFFu)                 /* width 8 bits */
#define CAN_MASK_W9                 (0x1FFu)                /* width 9 bits */
#define CAN_MASK_W10                (0x3FFu)                /* width 10 bits */

/* Get 1bit value of 16bit value */
#define CAN_U08_GET_BIT0(x)         (((x) >> 0u) & 1u)      /* Get bit0 */
#define CAN_U08_GET_BIT1(x)         (((x) >> 1u) & 1u)      /* Get bit1 */

/* Clear value */
#define CAN_U08_CLR                 (0x00u)                 /* for 8bits access  */
#define CAN_U16_CLR                 (0x0000u)               /* for 16bits access */

/* 32bits calculation */
#define CAN_U32_ONE                 (1uL)

/**********************************************************************************************************************
 Macro definitions (SFR definitions)
 *********************************************************************************************************************/
/* ---- C0CTRH, C0CTRL ---- */
#define CAN_U08_C0CTRLL_SLEEP_MODE      (0x05u)             /* CSLPR=1, CHMDC=1 */
#define CAN_U08_C0CTRLL_HALT_MODE       (0x02u)             /* CSLPR=0, CHMDC=2 */
#define CAN_U08_C0CTRLL_RESET_MODE      (0x01u)             /* CSLPR=0, CHMDC=1 */
#define CAN_U08_C0CTRLL_OP_MODE         (0x00u)             /* CSLPR=0, CHMDC=0 */

/* ---- C0STSH, C0STSL ---- */
#define CAN_U16_C0STSL_MODE_MASK        (0x0007u)           /* CSLPSTS, CHLTSTS, CRSTSTS */
#define CAN_U16_C0STSL_ESIF_ON          (CAN_U16_BIT8_ON)
#define CAN_U16_C0STSL_COMSTS_ON        (CAN_U16_BIT7_ON)
#define CAN_U16_C0STSL_CSLPSTS_ON       (CAN_U16_BIT2_ON)
#define CAN_U16_C0STSL_CHLTSTS_ON       (CAN_U16_BIT1_ON)
#define CAN_U16_C0STSL_CRSTSTS_ON       (CAN_U16_BIT0_ON)

/* ---- C0FDSTSH, C0FDSTSL ---- */
#define CAN_U08_C0FDSTSLH_TDCVF_ON      (0x80u)
#define CAN_U08_C0FDSTSLH_CLR_TDCVF     (0x03u)             /* Clear C0FDSTSLH without SOCO and EOCO */

/* ---- GCTRH, GCTRL ---- */
#define CAN_U08_GCTRLL_SLEEP_MODE       (0x05u)             /* GSLPR=1, GMDC=1 */
#define CAN_U08_GCTRLL_RESET_MODE       (0x01u)             /* GSLPR=0, GMDC=1 */
#define CAN_U08_GCTRLL_OP_MODE          (0x00u)             /* GSLPR=0, GMDC=0 */

/* ---- GINTSTS ---- */
#define CAN_U16_GINTSTS_THIF_ON         (CAN_U16_BIT4_ON)
#define CAN_U16_GINTSTS_CFTIF_ON        (CAN_U16_BIT3_ON)
#define CAN_U16_GINTSTS_TAIF_ON         (CAN_U16_BIT1_ON)
#define CAN_U16_GINTSTS_TSIF_ON         (CAN_U16_BIT0_ON)

/* ---- GSTS ---- */
#define CAN_U16_GSTS_MODE_MASK          (0x0007u)           /* GSLPSTS, GHLTSTS, GRSTSTS */
#define CAN_U16_GSTS_GRAMINIT_ON        (CAN_U16_BIT3_ON)
#define CAN_U16_GSTS_GSLPSTS_ON         (CAN_U16_BIT2_ON)
#define CAN_U16_GSTS_GRSTSTS_ON         (CAN_U16_BIT0_ON)

/* ---- GAFLECTR ---- */
#define CAN_U08_GAFLECTRH_AFLDAE_ON     (0x01u)             /* AFLDAE=1 */

/* ---- GAFLIDiH, GAFLIDiL ---- */
#define CAN_U16_GAFLID0H_GAFLLB_ON      (CAN_U16_BIT13_ON)

/* ---- GAFLMiH, GAFLMiL ---- */
#define CAN_U16_GAFLM0H_GAFLIDEM_ON     (CAN_U16_BIT15_ON)
#define CAN_U16_GAFLM0H_GAFLRTRM_ON     (CAN_U16_BIT14_ON)
#define CAN_U16_GAFLM0H_GAFLIFL1_ON     (CAN_U16_BIT13_ON)

/* ---- GAFLP0iH, GAFLP0iL ---- */
#define CAN_U08_GAFLP00LH_GAFLRMV_ON    (0x80u)             /* GAFLRMV=1 */
#define CAN_U08_GAFLP00LL_GAFLIFL0_ON   (0x80u)             /* GAFLIFL0=1 */

/* ---- RFCCk ---- */
#define CAN_U16_RFCC_RFE_ON             (CAN_U16_BIT0_ON)

/* ---- RFSTSk ---- */
#define CAN_U16_RFSTS_RFMLT_ON          (CAN_U16_BIT2_ON)
#define CAN_U16_RFSTS_RFFLL_ON          (CAN_U16_BIT1_ON)
#define CAN_U16_RFSTS_RFEMP_ON          (CAN_U16_BIT0_ON)
#define CAN_U08_RFSTSL_CLR_RFIF         (0x04u)             /* Clear RFSTSL without RFMLT */
#define CAN_U08_RFSTSL_CLR_RFMLT        (0x08u)             /* Clear RFSTSL without RFIF  */

/* ---- RFPCTRk ---- */
#define CAN_U08_RFPCTRL_FIFO_INC        (0xFFu)             /* FIFO pointer inclement value */

/* ---- CFCC ---- */
#define CAN_CFM_RX_MODE                 (0)
#define CAN_CFM_TX_MODE                 (1)
#define CAN_U16_CFCCL_CFE_ON            (CAN_U16_BIT0_ON)
#define CAN_U08_CFCCL_CFE_OFF           (0xFEu)             /* Clear only CFE */

/* ---- CFSTS ---- */
#define CAN_U16_CFSTS_CFRXIF_ON         (CAN_U16_BIT3_ON)
#define CAN_U16_CFSTS_CFMLT_ON          (CAN_U16_BIT2_ON)
#define CAN_U16_CFSTS_CFFLL_ON          (CAN_U16_BIT1_ON)
#define CAN_U16_CFSTS_CFEMP_ON          (CAN_U16_BIT0_ON)
#define CAN_U08_CFSTSL_CLR_TXIF         (0x0Cu)             /* Clear CFSTSL without RXIF, CFMLT    */
#define CAN_U08_CFSTSL_CLR_RXIF         (0x14u)             /* Clear CFSTSL without TXIF, CFMLT    */
#define CAN_U08_CFSTSL_CLR_CFMLT        (0x18u)             /* Clear CFSTSL without CFTXIF, CFRXIF */

/* ---- CFPCTR ---- */
#define CAN_U08_CFPCTRL_FIFO_INC        (0xFFu)             /* FIFO pointer inclement value */

/* ---- TMCm ---- */
#define CAN_U08_TMC_TMTAR_ON            (0x02u)             /* TMTAR=1 */
#define CAN_U08_TMC_TMTR_ON             (0x01u)             /* TMTR=1  */

/* ---- TMSTSm ---- */
#define CAN_U08_TMSTS_TMTRF_MASK        (0x06u)             /* TMTRF[1:0] */
#define CAN_POS_TMSTS_TMTRF             (1u)

/* ---- TMTASTS ---- */
#define CAN_U16_TMTASTS_MASK            (0x000Fu)

/* ---- THLCC ---- */
#define CAN_U16_THLCC_THLE_ON           (CAN_U16_BIT0_ON)

/* ---- THLSTS ---- */
#define CAN_U16_THLSTS_THLEMP_ON        (CAN_U16_BIT0_ON)
#define CAN_U16_THLSTS_THLELT_ON        (CAN_U16_BIT2_ON)
#define CAN_U08_THLSTSL_CLR_THLIF       (0x04u)             /* Clear only THLIF  */
#define CAN_U08_THLSTSL_CLR_THLELT      (0x08u)             /* Clear only THLELT */

/* ---- THLACC0H, THLACC0L ---- */
#define CAN_U16_THLACC0L_BN_MASK        (0x0018u)           /* BN[1:0] */
#define CAN_POS_THLACC0L_BN             (3u)
#define CAN_U16_THLACC0L_BT_MASK        (0x0003u)           /* BT[1:0] */
#define CAN_U16_THLACC0L_BT_TXMB        (1u)
#define CAN_U16_THLACC0L_BT_CFIFO       (2u)

/* ---- THLACC1H, THLACC1L ---- */
#define CAN_U16_THLACC1H_TIFL_MASK      (0x0003u)           /* TIFL[1:0] */

/* ---- THLPCTR ---- */
#define CAN_U08_THLPCTRL_LIST_INC       (0xFFu)             /* List pointer inclement value */

/* ---- CFDGRWC ---- */
#define CAN_U16_CFDGRWC_KEY             (0xC400u)           /* Key code for rewriting */
#define CAN_U16_CFDGRWC_PAGE0           (0xC400u)           /* Page 0 selected        */
#define CAN_U16_CFDGRWC_PAGE1           (0xC401u)           /* Page 1 selected        */
#define CAN_U16_CFDGRWC_PAGE2           (0xC402u)           /* Page 2 selected        */
#define CAN_U16_CFDGRWC_PAGE3           (0xC403u)           /* Page 3 selected        */

/* ---- RMPTRH, RMPTRL ---- */
#define CAN_POS_RMPTRHH_RMDLC           (4u)

/* ---- RFPTRH, RFPTRL ---- */
#define CAN_POS_RFPTRHH_RFDLC           (4u)

/* ---- CFIDH, CFIDL ---- */
#define CAN_U16_CFIDH_RTR_OFF           (0xBFFFu)           /* Clear RTR */

/* ---- CFPTRH, CFPTRL ---- */
#define CAN_POS_CFPTRHH_CFDLC           (4u)

/* ---- TMIDmH, TMIDmL ---- */
#define CAN_U16_TMIDH_RTR_OFF           (0xBFFFu)           /* Clear RTR */

/* ---- TMPTRmH ---- */
#define CAN_POS_TMPTRHH_TMDLC           (4u)

/* ---- Number of try read access ---- */
/*
 * If the max transition time is a multiple of the peripheral clock cycle,
 * the number of cycles is taken as the number of reads.
 * Since it takes at least one peripheral clock cycle to read the register,
 * the register is read once per cycle.
 *
 * If the max transition time is a multiple of the CAN bit time,
 * the number of readings is limited to 10 times.
 */
#define CAN_TRY_GLB_RAM_INIT            (520u)          /* 520 PCLK cycle */
#define CAN_TRY_GLB_SLP_TO_RST          (3u)            /* 3 peripheral clock cycle */
#define CAN_TRY_GLB_RST_TO_SLP          (3u)            /* 3 peripheral clock cycle */
#define CAN_TRY_GLB_RST_TO_OP           (10u)           /* 10 peripheral clock cycle */
#define CAN_TRY_CH_SLP_TO_RST           (3u)            /* 3 peripheral clock cycle */
#define CAN_TRY_CH_RST_TO_SLP           (3u)            /* 3 peripheral clock cycle */
#define CAN_TRY_MIN_TIMES               (10u)           /* Common minimum 10 times */

/**********************************************************************************************************************
 Global Typedef definitions
 *********************************************************************************************************************/
/* for low word only register */
typedef union
{
    struct
    {
        uint16_t LowWord;       /* 16-bit Access (Low) */
        uint16_t :16;
    } U16Reg;                   /* 16-bit Access       */
    struct
    {
        uint8_t  LByte;         /* 8-bit Access (Low)  */
        uint8_t  HByte;         /* 8-bit Access (High) */
        uint16_t :16;
    } U08Reg;                   /* 8-bit Access        */
} u_can_low_reg_t;

/* for RX FIFO registers */
typedef struct
{
    volatile u_can_low_reg_t RFCCk[CAN_MAX_RXFIFO_NUM];
    volatile u_can_low_reg_t RFSTSk[CAN_MAX_RXFIFO_NUM];
    volatile u_can_low_reg_t RFPCTRk[CAN_MAX_RXFIFO_NUM];
} st_can_rf_sfr_t;

/* for TX buffer registers */
typedef struct
{
    volatile uint8_t TMCm[CAN_MAX_TXBUF_NUM];
    volatile uint8_t TMSTSm[CAN_MAX_TXBUF_NUM];
} st_can_tm_sfr_t;

/* for RX rule register */
typedef struct
{
    volatile uint16_t IDL;
    volatile uint16_t IDH;
    volatile uint16_t ML;
    volatile uint16_t MH;
    volatile uint8_t  P0LL;
    volatile uint8_t  P0LH;
    volatile uint16_t P0H;
    volatile uint16_t P1L;
    volatile uint16_t :16;
} st_can_rxrule_sfr_t;

/* for RX rule registers */
typedef struct
{
    volatile st_can_rxrule_sfr_t  RxRule[CAN_MAX_RXRULE_NUM];
} st_can_ram_rxrule_sfr_t;

/* for Message buffer registers */
typedef struct
{
    volatile uint16_t IDL;
    volatile uint16_t IDH;
    volatile uint16_t PTRL;
    volatile uint8_t  :8;
    volatile uint8_t  PTRHH;
    volatile uint8_t  FDCSTSLL;
    volatile uint8_t  FDCSTSLH;
    volatile uint16_t FDCSTSH;
    volatile uint16_t DF[CAN_MAX_WORD_NUM];
} st_can_msgbuf_sfr_t;

/* for Message buffer registers (RX FIFO, Common FIFO, TX Buffer) */
typedef struct
{
    volatile st_can_msgbuf_sfr_t  RxFIFOMsg[CAN_MAX_RXFIFO_NUM];
    volatile st_can_msgbuf_sfr_t  CFIFOMsg;
    volatile st_can_msgbuf_sfr_t  TxBufMsg[CAN_MAX_TXBUF_NUM];
} st_can_ram_msg_sfr_t;

/* for Message buffer registers (RX Buffer) */
typedef struct
{
    volatile st_can_msgbuf_sfr_t  RxBufMsg[CAN_HALF_RXBUF_NUM];
} st_can_ram_rm_msg_sfr_t;

/**********************************************************************************************************************
 Exported global variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/

#endif /* R_RSCANFD_RL78_PRIVATE_H */
