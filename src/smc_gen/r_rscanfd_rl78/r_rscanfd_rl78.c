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
 * File Name    : r_rscanfd_rl78.c
 * Description  : This is source file for CAN driver code.
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
#include "r_rscanfd_rl78_if.h"
#include "r_rscanfd_rl78_config.h"
#include "r_rscanfd_rl78_private.h"

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
/* Ushort max */
#define CAN_USHORT_MAX                  (0xFFFFu)

/* Convert bytes to words */
#define CAN_BYTE_TO_WORD(x)             (((x) + 1u) / 2u)

/* Information label bit */
#define CAN_IFL_MSB                     (2u)
#define CAN_IFL_LSB                     (1u)

/* Word position of u_can_tx_head_t */
#define CAN_TX_HEAD_WORD_IDL            (0u)
#define CAN_TX_HEAD_WORD_IDH            (1u)
#define CAN_TX_HEAD_WORD_FDCTR          (2u)

/* Word position of u_can_rx_head_t */
#define CAN_RX_HEAD_WORD_IDL            (0u)
#define CAN_RX_HEAD_WORD_IDH            (1u)
#define CAN_RX_HEAD_WORD_FDCTR          (2u)

/* Data byte of classical frame */
#define CAN_CLASSICAL_MAX               (8u)

/* TXMB interrupt source */
#define CAN_SOURCE_TRM_TM0              (CAN_SOURCE_TRMC_TM0 | CAN_SOURCE_TRMA_TM0)
#define CAN_SOURCE_TRM_TM1              (CAN_SOURCE_TRMC_TM1 | CAN_SOURCE_TRMA_TM1)
#define CAN_SOURCE_TRM_TM2              (CAN_SOURCE_TRMC_TM2 | CAN_SOURCE_TRMA_TM2)
#define CAN_SOURCE_TRM_TM3              (CAN_SOURCE_TRMC_TM3 | CAN_SOURCE_TRMA_TM3)

/* Bit position of major version */
#define CAN_POS_MAJOR_VER               (8u)

/* Bit position of interrupt source argument */
#define CAN_POS_SOURCE_MLT_RF           (4u)
#define CAN_POS_SOURCE_TRMA             (4u)
#define CAN_POS_SOURCE_TRM_CF           (8u)

/**********************************************************************************************************************
 Merge Configuration
 *********************************************************************************************************************/
/* Nominal SJW */
#if (CAN_CFG_NSJW >= 0x40)
#define CAN_USER_NSJW6                  (1u)
#else
#define CAN_USER_NSJW6                  (0u)
#endif
#define CAN_USER_NSJW5_0                (CAN_CFG_NSJW & CAN_MASK_W6)

/* Tx Buffer Interrupt Enable */
#define CAN_U16_USER_TMIEC  \
    (CAN_U16_BIT3(CAN_CFG_TMIE3) | CAN_U16_BIT2(CAN_CFG_TMIE2) | \
    CAN_U16_BIT1(CAN_CFG_TMIE1)  | CAN_U16_BIT0(CAN_CFG_TMIE0))

/* Rx Buffer Interrupt Enable */
#if (CAN_CFG_RMIE_ALL == 0)
    #define CAN_U16_USER_RMIEC_VAL      (0u)
#elif (CAN_CFG_RMIE_ALL == 1)
    #define CAN_U16_USER_RMIEC_VAL      (0xFFFFu)
#else
    #define CAN_U16_USER_RMIEC_VAL      (CAN_CFG_RMIE_VALUE)
#endif
#define CAN_U16_USER_RMIEC_MASK         ((CAN_U32_ONE << CAN_CFG_NRXMB) - CAN_U32_ONE)

/* Global Error Interrupt Enable */
#define CAN_U08_USER_GCTRLH \
    (CAN_U16_BIT3(CAN_CFG_CMPOFIE) | CAN_U16_BIT2(CAN_CFG_THLEIE) | \
    CAN_U16_BIT1(CAN_CFG_MEIE)     | CAN_U16_BIT0(CAN_CFG_DEIE))

/* Channel Error Interrupt Enable (register value) */
#define CAN_U08_USER_C0CTRLH \
    (CAN_U16_BIT7(CAN_CFG_ALIE) | CAN_U16_BIT6(CAN_CFG_BLIE)  | \
    CAN_U16_BIT5(CAN_CFG_OLIE)  | CAN_U16_BIT4(CAN_CFG_BORIE) | \
    CAN_U16_BIT3(CAN_CFG_BOEIE) | CAN_U16_BIT2(CAN_CFG_EPIE)  | \
    CAN_U16_BIT1(CAN_CFG_EWIE)  | CAN_U16_BIT0(CAN_CFG_BEIE))

/* Channel Error Interrupt Enable (OR value) */
#define CAN_USER_ERFL_IE \
    (CAN_CFG_ALIE | CAN_CFG_BLIE | CAN_CFG_OLIE | CAN_CFG_BORIE | \
    CAN_CFG_BOEIE | CAN_CFG_EPIE | CAN_CFG_EWIE | CAN_CFG_BEIE)

/* TDC Violation Interrupt Enable */
#if (CAN_CFG_TDCVFIE != 0)
    #define CAN_U08_USER_IE_C0FDSTSLH   (CAN_U08_C0FDSTSLH_TDCVF_ON)
#else
    #define CAN_U08_USER_IE_C0FDSTSLH   (0u)
#endif

/* Settings of CAN Global Receive FIFO Interrupt */
#if (CAN_CFG_INTRCANGRFR_USE != 0)
    #define CAN_U16_USER_IE_RFISTS      (CAN_U16_BIT1(CAN_CFG_RF1_RFIE) | CAN_U16_BIT0(CAN_CFG_RF0_RFIE))
#endif

/* Settings of CAN Global Receive FIFO Interrupt */
#if (CAN_CFG_INTRCANGRVC_USE != 0)
    #define CAN_U16_USER_IE_RMND        (CAN_U16_USER_RMIEC_VAL & CAN_U16_USER_RMIEC_MASK)
#endif

/* Settings of CAN Global Error */
#if (CAN_CFG_INTRCANGERR_USE != 0)
    #define CAN_U16_USER_IE_GERFLL      (CAN_U08_USER_GCTRLH)

    #define CAN_U16_USER_IE_FMSTS       (0x0103u)
#endif

/* Settings of CAN0 Channel Transmit */
#if (CAN_CFG_INTRCAN0TRM_USE != 0)
    #if (CAN_CFG_THLIE != 0)
    #define CAN_U16_USER_THIF           (CAN_U16_BIT4_ON)
    #else
    #define CAN_U16_USER_THIF           (0u)
    #endif

    #if (CAN_CFG_CFTXIE != 0)
    #define CAN_U16_USER_CFTIF          (CAN_U16_BIT3_ON)
    #else
    #define CAN_U16_USER_CFTIF          (0u)
    #endif

    #if (CAN_CFG_TAIE != 0)
    #define CAN_U16_USER_TAIF           (CAN_U16_BIT1_ON)
    #else
    #define CAN_U16_USER_TAIF           (0u)
    #endif

    #if ((CAN_CFG_TMIE3 | CAN_CFG_TMIE2 | CAN_CFG_TMIE1 | CAN_CFG_TMIE0) != 0)
    #define CAN_U16_USER_TSIF           (CAN_U16_BIT0_ON)
    #else
    #define CAN_U16_USER_TSIF           (0u)
    #endif

    /* TX Interrupt status mask */
    #define CAN_U16_USER_IE_GTINTSTS    (CAN_U16_USER_THIF | CAN_U16_USER_CFTIF | CAN_U16_USER_TAIF | CAN_U16_USER_TSIF)

    /* TX completion mask */
    #define CAN_U16_USER_IE_TMTCSTS     (CAN_U16_USER_TMIEC)
#endif

/* Settings of CAN0 Channel Error Interrupt */
#if (CAN_CFG_INTRCAN0ERR_USE != 0)
    #define CAN_U08_USER_IE_C0ERFLL     (CAN_U08_USER_C0CTRLH)
#endif

/**********************************************************************************************************************
 Local Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Private (static) variables and functions
 *********************************************************************************************************************/
#define CAN_DLC_HIGHER_NUM          (8u)    /* Number of DLC 8 or more */
static const uint8_t s_dlc_to_length[CAN_DLC_HIGHER_NUM] =
{
/*  DLC:8   9    10   11   12   13   14   15  */
        8u, 12u, 16u, 20u, 24u, 32u, 48u, 64u
};

#if ((CAN_CFG_RF0_RFDC | CAN_CFG_RF1_RFDC) != 0)    /* Use RXFIFO? */
static const uint8_t s_rxfifo_pls[CAN_MAX_RXFIFO_NUM] =
{
    CAN_CFG_RF0_RFPLS, CAN_CFG_RF1_RFPLS
};
#endif

/**********************************************************************************************************************
 * Function Name: R_CAN_Create
 * Description  : This function initializes the CAN module.
 * Arguments    : None
 * Return Value : None
 *********************************************************************************************************************/
void R_CAN_Create(void)
{
    /* Clock supply */
    CAN0EN = 1u;        /* supply CAN input clock */

#if (BSP_CFG_PIOR46 == 0)
    /* Set P11 for CRXD0 pin (no PMCxx) */
    PM1_bit.no1    = 1u;                    /* Input mode */
    PU1_bit.no1    = CAN_CFG_CRXD0_PU;      /* On-chip pull-up resistor setting */
    PIM1_bit.no1   = 0u;                    /* Normal input buffer */
    POM1_bit.no1   = 0u;                    /* Normal output mode (not open-drain) */
    PITHL1_bit.no1 = CAN_CFG_CRXD0_PITHL;   /* Schmitt1 or Schmitt3 */

    /* Set P10 for CTXD0 pin (no PMCxx)  */
    P1_bit.no0     = 1u;                    /* Output 1 */
    PM1_bit.no0    = 0u;                    /* Output mode */
    POM1_bit.no0   = 0u;                    /* Normal output mode (not open-drain) */
    PSRSEL_bit.no0 = 0u;                    /* Normal slew rate */

#else
    /* Set P73 for CRXD0 pin */
    PMC7_bit.no3   = 0u;                    /* Digital I/O */
    PM7_bit.no3    = 1u;                    /* Input mode */
    PU7_bit.no3    = CAN_CFG_CRXD0_PU;      /* On-chip pull-up resistor setting */
    PIM7_bit.no3   = 0u;                    /* Normal input buffer */
    POM7_bit.no3   = 0u;                    /* Normal output mode (not open-drain) */
    PITHL7_bit.no3 = CAN_CFG_CRXD0_PITHL;   /* Schmitt1 or Schmitt3 */

    /* Set P72 for CTXD0 pin (no PSRSEL) */
    PMC7_bit.no2    = 0u;                   /* Digital I/O */
    P7_bit.no2      = 1u;                   /* Output 1 */
    PM7_bit.no2     = 0u;                   /* Output mode */
    POM7_bit.no2    = 0u;                   /* Normal output mode (not open-drain) */

#endif

    /* CAN Global Receive FIFO Interrupt */
    RCANGRFRMK = 1u;
    RCANGRFRIF = 0u;

    /* CAN Global Receive Message Buffer Interrupt */
    RCANGRVCMK = 1u;
    RCANGRVCIF = 0u;

    /* CAN Global Error Interrupt */
    RCANGERRMK = 1u;
    RCANGERRIF = 0u;

    /* CAN0 Channel Transmit Interrupt */
    RCAN0TRMMK = 1u;
    RCAN0TRMIF = 0u;

    /* CAN0 Common FIFO Receive Interrupt */
    RCAN0CFRMK = 1u;
    RCAN0CFRIF = 0u;

    /* CAN0 Channel Error Interrupt */
    RCAN0ERRMK = 1u;
    RCAN0ERRIF = 0u;

    /* CAN0 Wakeup Interrupt */
    RCAN0WUPMK = 1u;
    RCAN0WUPIF = 0u;

}
/**********************************************************************************************************************
 End of function R_CAN_Create
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_SetConfig
 * Description  : Initialize CAN controller after reset
 * Arguments    : none 
 * Return Value : CAN_SUCCESS - 
 *                    normal completion
 *                CAN_ERR_INVALID_MODE -
 *                    invalid global mode
 *                CAN_ERR_ILLEGAL_STS -
 *                    failure to change global mode or channel mode
 *********************************************************************************************************************/
e_can_err_t R_CAN_SetConfig(void)
{
    e_can_err_t  rtn;
    uint16_t     try_num;

    /* ==== Check if RAM initialization is finished ==== */
    rtn     = CAN_ERR_ILLEGAL_STS;
    try_num = 0u;
    do
    {
        if ((GSTS & CAN_U16_GSTS_GRAMINIT_ON) == 0u)                        /* RAM initialization is finished? */
        {
            rtn = CAN_SUCCESS;
        }
        else
        {
            try_num++;
        }
    }
    while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_GLB_RAM_INIT));

    /* ==== global mode switch (sleep->reset) ==== */
    if (CAN_SUCCESS == rtn)
    {
        if ((GSTS & CAN_U16_GSTS_GSLPSTS_ON) != 0u)                        /* Global Sleep Mode? */
        {
            GCTRLL = CAN_U08_GCTRLL_RESET_MODE;

            rtn     = CAN_ERR_ILLEGAL_STS;
            try_num = 0u;
            do
            {
                if ((GSTS & CAN_U16_GSTS_GSLPSTS_ON) == 0u)                 /* Exit Sleep mode? */
                {
                    rtn = CAN_SUCCESS;
                }
                else
                {
                    try_num++;
                }
            }
            while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_GLB_SLP_TO_RST));
        }
        else if ((GSTS & CAN_U16_GSTS_GRSTSTS_ON) == 0u)                    /* Not Global Reset Mode? */
        {
            rtn = CAN_ERR_INVALID_MODE;
        }
        else
        {
            ;   /* No operation if Global Reset Mode */
        }
    }
    else
    {
        ;   /* No operation if error has occured */
    }

    /* ==== channel mode switch (sleep->reset) ==== */
    if (CAN_SUCCESS == rtn)
    {
        if ((C0STSL & CAN_U16_C0STSL_CSLPSTS_ON) != 0u)                     /* Channel Sleep Mode? */
        {
            C0CTRLL = CAN_U08_C0CTRLL_RESET_MODE;

            rtn     = CAN_ERR_ILLEGAL_STS;
            try_num = 0u;
            do
            {
                if ((C0STSL & CAN_U16_C0STSL_CSLPSTS_ON) == 0u)             /* Exit Sleep mode? */
                {
                    rtn = CAN_SUCCESS;
                }
                else
                {
                    try_num++;
                }
            }
            while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_CH_SLP_TO_RST));
        }
        else
        {
            ;   /* No operation if Channel Reset Mode */
        }
    }
    else
    {
        ;   /* No operation if error has occured */
    }

    if (CAN_SUCCESS == rtn)
    {
        /* ==== FD / Classical mode setting ==== */
        C0FDCFGHH = (uint8_t)(CAN_U16_BIT6(CAN_CFG_CLOE) | CAN_U16_BIT4(CAN_CFG_FDOE));

        /* ==== Global function setting ==== */
        GCFGH  = (uint16_t)CAN_CFG_ITRCP;
        GCFGL  = (
            CAN_U16_BIT12(CAN_CFG_TSSS)             |
            CAN_U16_POS8 (CAN_CFG_TSP, CAN_MASK_W4) |
            CAN_U16_BIT5 (CAN_CFG_CMPOC)            |
            CAN_U16_BIT4 (CAN_CFG_DCS)              |
            CAN_U16_BIT3 (CAN_CFG_MME)              |
            CAN_U16_BIT2 (CAN_CFG_DRE)              |
            CAN_U16_BIT1 (CAN_CFG_DCE)              |
            CAN_U16_BIT0 (CAN_CFG_TPRI));
        GFDCFG = (
            CAN_U16_POS8 (CAN_CFG_TSCCFG, CAN_MASK_W2) |
            CAN_U16_BIT0 (CAN_CFG_RPED));

        /* ==== Baud Rate and Bit Timing setting ==== */
        C0NCFGH = (
            CAN_U16_POS9 (CAN_CFG_NTSEG2,   CAN_MASK_W7) |
            CAN_U16_POS1 (CAN_CFG_NTSEG1,   CAN_MASK_W8) |
            CAN_U16_BIT0 (CAN_USER_NSJW6));
        C0NCFGL = (
            CAN_U16_POS10(CAN_USER_NSJW5_0, CAN_MASK_W6) |
            CAN_U16_POS0 (CAN_CFG_NBRP,     CAN_MASK_W10));
#if (CAN_CFG_CLOE == 0)                                                     /* Not Classical mode? */
        C0DCFGH = (
            CAN_U16_POS8(CAN_CFG_DSJW,   CAN_MASK_W4) |
            CAN_U16_POS0(CAN_CFG_DTSEG2, CAN_MASK_W4));
        C0DCFGL = (
            CAN_U16_POS8(CAN_CFG_DTSEG1, CAN_MASK_W5) |
            CAN_U16_POS0(CAN_CFG_DBRP,   CAN_MASK_W8));
#endif

        /* ==== Rx rule setting ==== */
        GAFLCFG = CAN_U08_CLR;

        /* ==== buffer setting ==== */
        /* ---- Set RX message buffer ---- */
        RMNB  = (
            CAN_U16_POS8(CAN_CFG_RMPLS, CAN_MASK_W3) |
            CAN_U16_POS0(CAN_CFG_NRXMB, CAN_MASK_W5));
        RMIEC = CAN_U16_USER_RMIEC_VAL & CAN_U16_USER_RMIEC_MASK;

        /* ---- Set Rx FIFO buffer ---- */
        RFCC0 = (
            CAN_U16_POS13(CAN_CFG_RF0_RFIGCV, CAN_MASK_W3) |
            CAN_U16_BIT12(CAN_CFG_RF0_RFIM)                |
            CAN_U16_POS8 (CAN_CFG_RF0_RFDC,   CAN_MASK_W2) | /* Max:11b */
            CAN_U16_POS4 (CAN_CFG_RF0_RFPLS,  CAN_MASK_W3) |
            CAN_U16_BIT1 (CAN_CFG_RF0_RFIE));
        RFCC1 = (
            CAN_U16_POS13(CAN_CFG_RF1_RFIGCV, CAN_MASK_W3) |
            CAN_U16_BIT12(CAN_CFG_RF1_RFIM)                |
            CAN_U16_POS8 (CAN_CFG_RF1_RFDC,   CAN_MASK_W2) | /* Max:11b */
            CAN_U16_POS4 (CAN_CFG_RF1_RFPLS,  CAN_MASK_W3) |
            CAN_U16_BIT1 (CAN_CFG_RF1_RFIE));

        /* ---- Set Common FIFO buffer ---- */
        CFCCH = (
            CAN_U16_POS8 (CAN_CFG_CFITT,  CAN_MASK_W8) |
            CAN_U16_POS5 (CAN_CFG_CFDC,   CAN_MASK_W2) | /* Max:11b */
            CAN_U16_POS0 (CAN_CFG_CFTML,  CAN_MASK_W2));
        CFCCL = (
            CAN_U16_POS13(CAN_CFG_CFIGCV, CAN_MASK_W3) |
            CAN_U16_BIT12(CAN_CFG_CFIM)                |
            CAN_U16_BIT11(CAN_CFG_CFITR)               |
            CAN_U16_BIT10(CAN_CFG_CFITSS)              |
            CAN_U16_BIT8 (CAN_CFG_CFM)                 |
            CAN_U16_POS4 (CAN_CFG_CFPLS,  CAN_MASK_W3) |
            CAN_U16_BIT2 (CAN_CFG_CFTXIE)              |
            CAN_U16_BIT1 (CAN_CFG_CFRXIE));

        /* ---- Set TX message buffer ---- */
        TMIEC = CAN_U16_USER_TMIEC;

        /* ---- Set TX history list ---- */
        THLCC = (
            CAN_U16_BIT10(CAN_CFG_THLDTE) |
            CAN_U16_BIT9 (CAN_CFG_THLIM)  |
            CAN_U16_BIT8 (CAN_CFG_THLIE));

        /* ==== global error interrupt setting ==== */
        GCTRLH = CAN_U08_USER_GCTRLH;

        /* ==== channel function setting (Not support SOCOIE,EOCOIE) ==== */
        C0CTRH  = (
            CAN_U16_BIT7(CAN_CFG_ERRD)             |
            CAN_U16_POS5(CAN_CFG_BOM, CAN_MASK_W2) |
            CAN_U16_BIT3(CAN_CFG_TDCVFIE)          |
            CAN_U16_BIT0(CAN_CFG_TAIE));
        C0CTRLH = CAN_U08_USER_C0CTRLH;

        /* ==== channel FD setting (Not support RPNMD,EOCCFG) ==== */
        C0FDCFGH |= (
            CAN_U16_BIT13(CAN_CFG_REFE) |
            CAN_U16_POS0 (CAN_CFG_TDCO, CAN_MASK_W8)); /* or CLOE/FDOE */
        C0FDCFGL  = (
            CAN_U16_BIT10(CAN_CFG_ESIC) |
            CAN_U16_BIT9 (CAN_CFG_TDCE) |
            CAN_U16_BIT8 (CAN_CFG_TDCOC));

        /* ==== Interrupt Priority Setting ==== */
        /* CAN Global Receive FIFO Interrupt */
        RCANGRFRPR1 = CAN_U08_GET_BIT1(CAN_CFG_INTRCANGRFR_LEVEL);
        RCANGRFRPR0 = CAN_U08_GET_BIT0(CAN_CFG_INTRCANGRFR_LEVEL);

        /* CAN Global Receive Message Buffer Interrupt */
        RCANGRVCPR1 = CAN_U08_GET_BIT1(CAN_CFG_INTRCANGRVC_LEVEL);
        RCANGRVCPR0 = CAN_U08_GET_BIT0(CAN_CFG_INTRCANGRVC_LEVEL);

        /* CAN Global Error Interrupt */
        RCANGERRPR1 = CAN_U08_GET_BIT1(CAN_CFG_INTRCANGERR_LEVEL);
        RCANGERRPR0 = CAN_U08_GET_BIT0(CAN_CFG_INTRCANGERR_LEVEL);

        /* CAN0 Channel Transmit Interrupt */
        RCAN0TRMPR1 = CAN_U08_GET_BIT1(CAN_CFG_INTRCAN0TRM_LEVEL);
        RCAN0TRMPR0 = CAN_U08_GET_BIT0(CAN_CFG_INTRCAN0TRM_LEVEL);

        /* CAN0 Common FIFO Receive Interrupt */
        RCAN0CFRPR1 = CAN_U08_GET_BIT1(CAN_CFG_INTRCAN0CFR_LEVEL);
        RCAN0CFRPR0 = CAN_U08_GET_BIT0(CAN_CFG_INTRCAN0CFR_LEVEL);

        /* CAN0 Channel Error Interrupt */
        RCAN0ERRPR1 = CAN_U08_GET_BIT1(CAN_CFG_INTRCAN0ERR_LEVEL);
        RCAN0ERRPR0 = CAN_U08_GET_BIT0(CAN_CFG_INTRCAN0ERR_LEVEL);

        /* CAN0 Wakeup Interrupt */
        RCAN0WUPPR1 = CAN_U08_GET_BIT1(CAN_CFG_INTRCAN0WUP_LEVEL);
        RCAN0WUPPR0 = CAN_U08_GET_BIT0(CAN_CFG_INTRCAN0WUP_LEVEL);
    }
    else
    {
        ;   /* No operation */
    }

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_SetConfig
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_AddRxRule
 * Description  : This function initializes RX rule table of CAN module.
 * Arguments    : p_filter -
 *                    Pointer to rule filter structure (ID, ID mask, RTR type, IDE type)
 *                p_callback-
 *                    Optional pointer to function called from RXIF or global
 *                    error interrupts.
 *                p_opt -
 *                    Pointer to rule filter options structure (RX label, Information label, DLC, Loopback).
 *                    DLC is effective if DLC check is enabled (DCE=1).
 *                    Loopback is effective if mirror mode is enabled (MME=1).
 *                    Set NULL if not use options.
 * Return Value : CAN_SUCCESS -
 *                    normal completion
 *                CAN_ERR_BUF_FULL -
 *                    Cannot add a rule (RX rule table is full.)
 *                CAN_ERR_INVALID_MODE -
 *                    Not in global reset mode (results from R_CAN_SetConfig())
 *                CAN_ERR_INVALID_ARG -
 *                    invalid argument specification
 *********************************************************************************************************************/
e_can_err_t R_CAN_AddRxRule(const st_can_filter_t * p_filter,
    const can_storage_t         storage,
    const can_rxbuf_t           rxbuf_idx,
    const st_can_filter_opt_t * p_opt)
{
    e_can_err_t rtn = CAN_SUCCESS;
    uint16_t    rule_num;

    /* Get rule number */
    rule_num = GAFLCFG;

    /* Check mode and limit */
    if ((GSTS & CAN_U16_GSTS_MODE_MASK) != CAN_U16_GSTS_GRSTSTS_ON) /* Not Global Reset Mode? */
    {
        rtn = CAN_ERR_INVALID_MODE;                                 /* Return: Invalid current mode */
    }
    else if ((C0STSL & CAN_U16_C0STSL_MODE_MASK) != CAN_U16_C0STSL_CRSTSTS_ON) /* Not Channel Reset Mode? */
    {
        rtn = CAN_ERR_INVALID_MODE;                                 /* Return: Invalid current mode */
    }
    else if (rule_num >= CAN_MAX_RXRULE_NUM)                        /* Rule nunber is max? */
    {
        rtn = CAN_ERR_BUF_FULL;                                     /* Return: Cannot add a rule */
    }
#if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
    else if (NULL == p_filter)                                      /* NULL pointer? */
    {
        rtn = CAN_ERR_INVALID_ARG;                                  /* Return: Invalid argument */
    }
#endif
    else
    {
        ;   /* No operation */
    }

#if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
    /* Check storage */
    if (CAN_SUCCESS == rtn)
    {
        can_storage_t mask_storage = 0u;
        uint8_t       rxbuf_num    = CAN_CFG_NRXMB;

    #if (CAN_CFG_NRXMB > 0)                                         /* Use RX buffer? */
        mask_storage |= CAN_STORE_RM;
    #endif
    #if (CAN_CFG_RF0_RFDC > 0)                                      /* Use RX FIFO0? */
        mask_storage |= CAN_STORE_RF0;
    #endif
    #if (CAN_CFG_RF1_RFDC > 0)                                      /* Use RX FIFO1? */
        mask_storage |= CAN_STORE_RF1;
    #endif
    #if ((CAN_CFG_CFDC > 0) && (CAN_CFG_CFM == CAN_CFM_RX_MODE))    /* Use RX mode CFIFO? */
        mask_storage |= CAN_STORE_CF;
    #endif

        if ((storage & mask_storage) != storage)                /* Store to unuse buffer? */
        {
            rtn = CAN_ERR_INVALID_ARG;                          /* Return: Invalid argument */
        }
        else if (((storage & CAN_STORE_RM) != 0u) && (rxbuf_idx >= rxbuf_num))
        {
            /* Argument error if store to unused RX buffer */
            rtn = CAN_ERR_INVALID_ARG;                          /* Return: Invalid argument */
        }
        else
        {
            ;   /* No operation */
        }
    }
#endif /* end of (CAN_CFG_PARAM_CHECKING_ENABLE == 1) */

    if (CAN_SUCCESS == rtn)
    {
        volatile st_can_rxrule_sfr_t CAN_NO_BIT_ACCESS * p_rxrule_sfr;

        uint16_t id_high;
        uint16_t mask_high;
        uint8_t  rm_storage;
        uint8_t  ifl_and_dlc;
        uint16_t label;

        /* Set IDH, IDHM */
        id_high   = (uint16_t)((p_filter->ID)      >> 16u);         /* Get high word of ID */
        mask_high = (uint16_t)((p_filter->ID_MASK) >> 16u);         /* Get high word of ID_MASK */

        /* Set RTR, RTRM */
        if (p_filter->RTR_TYPE < CAN_RTR_ANY_FRAME)                 /* Not any? (RTR=0 or 1) */
        {
            id_high   |= CAN_U16_BIT14(p_filter->RTR_TYPE);
            mask_high |= CAN_U16_GAFLM0H_GAFLRTRM_ON;
        }

        /* Set IDE, IDEM */
        if (p_filter->IDE_TYPE < CAN_IDE_ANY_FORMAT)                /* Not any? (IDE=0 or 1) */
        {
            id_high   |= CAN_U16_BIT15(p_filter->IDE_TYPE);
            mask_high |= CAN_U16_GAFLM0H_GAFLIDEM_ON;
        }

        /* Set RMV, RMDP */
        if ((storage & CAN_STORE_RM) != 0u)                         /* Store to RX buffer? */
        {
            /* Set value for 8 bit register */
            rm_storage = (uint8_t)(CAN_U08_GAFLP00LH_GAFLRMV_ON | rxbuf_idx);
        }
        else
        {
            rm_storage = 0u;
        }

        if (NULL != p_opt)                                          /* Use option? */
        {
            /* Set LB */
            if (CAN_AFL_LB_LOOPBACK == p_opt->LB)
            {
                id_high |= CAN_U16_GAFLID0H_GAFLLB_ON;
            }

            /* Set IFL1 */
            if ((p_opt->IFL & CAN_IFL_MSB) != 0u)
            {
                mask_high |= CAN_U16_GAFLM0H_GAFLIFL1_ON;
            }

            /* Set DLC, IFL0 */
            ifl_and_dlc = (uint8_t)(p_opt->DLC);

            if ((p_opt->IFL & CAN_IFL_LSB) != 0u)
            {
                /* Set value for 8 bit register */
                ifl_and_dlc = (uint8_t)(CAN_U08_GAFLP00LL_GAFLIFL0_ON | ifl_and_dlc);
            }

            /* Set PTR */
            label = p_opt->LBL;
        }
        else
        {
            ifl_and_dlc = 0u;
            label       = 0u;
        }

        /* ---- Set Rx rule number per channel (Increment) ---- */
        GAFLCFG = rule_num + 1u;

        /* ---- Set write enable of Rule table ---- */
        GAFLECTRH = CAN_U08_GAFLECTRH_AFLDAE_ON;

        /* ---- Select RAM window 0 ---- */
        CFDGRWC = CAN_U16_CFDGRWC_PAGE0;

        /* ---- Get pointer of RX rule register ---- */
        p_rxrule_sfr = &CAN_RXRULE_SFR.RxRule[rule_num];

        /* ---- Set a single Rx rule ---- */
        p_rxrule_sfr->IDL  = (uint16_t)(p_filter->ID & CAN_USHORT_MAX);         /* Get low word of ID */
        p_rxrule_sfr->IDH  = id_high;
        p_rxrule_sfr->ML   = (uint16_t)(p_filter->ID_MASK & CAN_USHORT_MAX);    /* Get low word of ID_MASK */
        p_rxrule_sfr->MH   = mask_high;
        p_rxrule_sfr->P0LL = ifl_and_dlc;
        p_rxrule_sfr->P0LH = rm_storage;
        p_rxrule_sfr->P0H  = label;
        p_rxrule_sfr->P1L  = (storage & (CAN_STORE_CF | CAN_STORE_RF1 | CAN_STORE_RF0));

        /* ---- Set write disable of Rule table ---- */
        GAFLECTRH = CAN_U08_CLR;
    }
    else
    {
        ;   /* No operation if error has occured */
    }

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_AddRxRule
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_StartComm
 * Description  : Start CAN communication
 * Arguments    : none
 * Return Value : CAN_SUCCESS -
 *                    normal completion
 *                CAN_ERR_WAITING -
 *                    wait to change global mode or channel mode
 *                CAN_ERR_INVALID_MODE -
 *                    invalid global mode or channel mode
 *                CAN_ERR_ILLEGAL_STS -
 *                    failure to change global mode or channel mode
 *********************************************************************************************************************/
e_can_err_t R_CAN_StartComm(void)
{
    e_can_err_t rtn = CAN_SUCCESS;
    uint16_t    try_num;

    /* Clear interrupt flag and mask (excluding CAN0 wakeup) */
    {
#if (CAN_CFG_INTRCANGRFR_USE != 0)
        /* CAN Global Receive FIFO */
        RCANGRFRIF = 0u;
        RCANGRFRMK = 0u;
#endif

#if (CAN_CFG_INTRCANGRVC_USE != 0)
        /* CAN Global Receive Message Buffer */
        RCANGRVCIF = 0u;
        RCANGRVCMK = 0u;
#endif

#if (CAN_CFG_INTRCANGERR_USE != 0)
        /* CAN Global Error */
        RCANGERRIF = 0u;
        RCANGERRMK = 0u;
#endif

#if (CAN_CFG_INTRCAN0TRM_USE != 0)
        /* CAN0 Channel Transmit */
        RCAN0TRMIF = 0u;
        RCAN0TRMMK = 0u;
#endif

#if (CAN_CFG_INTRCAN0CFR_USE != 0)
        /* CAN0 Common FIFO Receive */
        RCAN0CFRIF = 0u;
        RCAN0CFRMK = 0u;
#endif

#if (CAN_CFG_INTRCAN0ERR_USE != 0)
        /* CAN0 Channel Error */
        RCAN0ERRIF = 0u;
        RCAN0ERRMK = 0u;
#endif
    }

    /* ==== switch to global operation mode from global reset mode ==== */
    if ((GSTS & CAN_U16_GSTS_MODE_MASK) == CAN_U16_GSTS_GRSTSTS_ON)             /* Global Reset mode? */
    {
        GCTRLL = CAN_U08_GCTRLL_OP_MODE;

        rtn     = CAN_ERR_ILLEGAL_STS;
        try_num = 0u;
        do
        {
            if ((GSTS & CAN_U16_GSTS_GRSTSTS_ON) == 0u)                         /* Global Operation mode? */
            {
                rtn = CAN_SUCCESS;
            }
            else
            {
                try_num++;
            }
        }
        while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_GLB_RST_TO_OP));
    }
    else if ((GSTS & CAN_U16_GSTS_GSLPSTS_ON) != 0u)                            /* Global Sleep mode? */
    {
        rtn = CAN_ERR_INVALID_MODE;
    }
    else
    {
        ;   /* No operation if already Global Operation Mode */
    }

    /* ---- switch to channel operation mode ---- */
    if (CAN_SUCCESS == rtn)
    {
        if ((C0STSL & CAN_U16_C0STSL_MODE_MASK) == CAN_U16_C0STSL_CRSTSTS_ON)   /* Channel Reset Mode? */
        {
            C0CTRLL = CAN_U08_C0CTRLL_OP_MODE;

            rtn     = CAN_ERR_WAITING;
            try_num = 0u;
            do
            {
                if ((C0STSL & CAN_U16_C0STSL_CRSTSTS_ON) == 0u)                 /* Channel Operation mode? */
                {
                    rtn = CAN_SUCCESS;
                }
                else
                {
                    try_num++;
                }
            }
            while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_MIN_TIMES));
        }
        else if ((C0STSL & CAN_U16_C0STSL_CSLPSTS_ON) != 0u)                    /* Channel Sleep Mode? */
        {
            rtn = CAN_ERR_INVALID_MODE;
        }
        else
        {
            ;   /* No operation if already Channel Operation Mode */
        }
    }
    else
    {
        ;   /* No operation if error has ocurred */
    }

    /* ---- set fifo enable ---- */
#if ((CAN_CFG_RF0_RFDC | CAN_CFG_RF1_RFDC | CAN_CFG_CFDC | CAN_CFG_THLE) != 0)
    if (CAN_SUCCESS == rtn)
    {
    #if (CAN_CFG_RF0_RFDC != 0)
        RFCC0 |= CAN_U16_RFCC_RFE_ON;
    #endif

    #if (CAN_CFG_RF1_RFDC != 0)
        RFCC1 |= CAN_U16_RFCC_RFE_ON;
    #endif

    #if (CAN_CFG_CFDC != 0)
        CFCCL |= CAN_U16_CFCCL_CFE_ON;
    #endif

    #if (CAN_CFG_THLE != 0)
        THLCC |= CAN_U16_THLCC_THLE_ON;
    #endif
    }
#endif

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_StartComm
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_StopComm
 * Description  : Stop channel operation (Channel 0)
 * Arguments    : none
 * Return Value : CAN_SUCCESS -
 *                    normal completion
 *                CAN_ERR_WAITING -
 *                    wait to change global mode or channel mode
 *********************************************************************************************************************/
e_can_err_t R_CAN_StopComm(void)
{
    e_can_err_t rtn = CAN_SUCCESS;
    uint16_t    try_num;

    /* ---- switch to channel halt mode ---- */
    if ((C0STSL & (CAN_U16_C0STSL_CHLTSTS_ON | CAN_U16_C0STSL_CRSTSTS_ON)) == 0u)   /* Operation Mode? */
    {
        C0CTRLL = CAN_U08_C0CTRLL_HALT_MODE;

        rtn     = CAN_ERR_WAITING;
        try_num = 0u;
        do
        {
            if ((C0STSL & CAN_U16_C0STSL_CHLTSTS_ON) != 0u)                         /* Halt mode? */
            {
                rtn = CAN_SUCCESS;
            }
            else
            {
                try_num++;
            }
        }
        while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_MIN_TIMES));
    }
    else
    {
        ;   /* No operation */
    }

    /* ---- disable CAN interrupt (excluding CAN0 wakeup) ---- */
    if (CAN_SUCCESS == rtn)                     /* Success of transition to Halt Mode? */
    {
        /* CAN Global Receive FIFO */
        RCANGRFRMK = 1u;
        RCANGRFRIF = 0u;

        /* CAN Global Receive Message Buffer */
        RCANGRVCMK = 1u;
        RCANGRVCIF = 0u;

        /* CAN Global Error */
        RCANGERRMK = 1u;
        RCANGERRIF = 0u;

        /* CAN0 Channel Transmit */
        RCAN0TRMMK = 1u;
        RCAN0TRMIF = 0u;

        /* CAN0 Common FIFO Receive */
        RCAN0CFRMK = 1u;
        RCAN0CFRIF = 0u;

        /* CAN0 Channel Error */
        RCAN0ERRMK = 1u;
        RCAN0ERRIF = 0u;
    }
    else
    {
        ;   /* No operation */
    }

    /* ---- switch to channel reset mode ---- */
    if (CAN_SUCCESS == rtn)
    {
        if ((C0STSL & CAN_U16_C0STSL_CRSTSTS_ON) == 0u)                         /* Not Reset Mode? */
        {
            C0CTRLL = CAN_U08_C0CTRLL_RESET_MODE;

            rtn     = CAN_ERR_WAITING;
            try_num = 0u;
            do
            {
                if ((C0STSL & CAN_U16_C0STSL_CRSTSTS_ON) != 0u)                         /* Reset mode? */
                {
                    rtn = CAN_SUCCESS;
                }
                else
                {
                    try_num++;
                }
            }
            while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_MIN_TIMES));
        }
        else
        {
            ;   /* No operation if Channel Reset/Sleep Mode */
        }
    }
    else
    {
        ;   /* No operation */
    }

    /* ---- switch to global reset mode ---- */
    if (CAN_SUCCESS == rtn)
    {
        if ((GSTS & CAN_U16_GSTS_GRSTSTS_ON) == 0u)                         /* Not Global Reset Mode? */
        {
            GCTRLL = CAN_U08_GCTRLL_RESET_MODE;

            rtn     = CAN_ERR_WAITING;
            try_num = 0u;
            do
            {
                if ((GSTS & CAN_U16_GSTS_GRSTSTS_ON) != 0u)                             /* Global Reset mode? */
                {
                    rtn = CAN_SUCCESS;
                }
                else
                {
                    try_num++;
                }
            }
            while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_MIN_TIMES));
        }
        else
        {
            ;   /* No operation if Global Reset/Sleep Mode */
        }
    }
    else
    {
        ;   /* No operation */
    }

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_StopComm
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_Sleep
 * Description  : Put CAN module to sleep
 * Arguments    : wup -
 *                    Whether to use CAN wakeup (CAN_WUP_UNUSE, CAN_WUP_USE)
 * Return Value : CAN_SUCCESS -
 *                    normal completion
 *                CAN_ERR_INVALID_MODE -
 *                    invalid global mode or channel mode
 *                CAN_ERR_ILLEGAL_STS -
 *                    failure to change global mode or channel mode
 *********************************************************************************************************************/
e_can_err_t R_CAN_Sleep(const uint8_t wup)
{
    e_can_err_t rtn = CAN_SUCCESS;
    uint16_t    try_num;

    /* ---- switch to channel sleep mode ---- */
    if ((C0STSL & CAN_U16_C0STSL_MODE_MASK) == CAN_U16_C0STSL_CRSTSTS_ON)   /* Channel Reset Mode? */
    {
        C0CTRLL = CAN_U08_C0CTRLL_SLEEP_MODE;

        rtn     = CAN_ERR_ILLEGAL_STS;
        try_num = 0u;
        do
        {
            if ((C0STSL & CAN_U16_C0STSL_CSLPSTS_ON) != 0u)                 /* Channel Sleep Mode? */
            {
                rtn = CAN_SUCCESS;
            }
            else
            {
                try_num++;
            }
        }
        while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_CH_RST_TO_SLP));
    }
    else if ((C0STSL & CAN_U16_C0STSL_CSLPSTS_ON) == 0u)                    /* Not Channel Sleep Mode? */
    {
        rtn = CAN_ERR_INVALID_MODE;
    }
    else
    {
        ;   /* No operation if already Channel Sleep Mode */
    }

    /* ---- switch to global sleep mode ---- */
    if (CAN_SUCCESS == rtn)
    {
        if ((GSTS & CAN_U16_GSTS_MODE_MASK) == CAN_U16_GSTS_GRSTSTS_ON)     /* Global Reset Mode? */
        {
            GCTRLL = CAN_U08_GCTRLL_SLEEP_MODE;

            rtn     = CAN_ERR_ILLEGAL_STS;
            try_num = 0u;
            do
            {
                if ((GSTS & CAN_U16_GSTS_GSLPSTS_ON) != 0u)                 /* Global Sleep Mode? */
                {
                    rtn = CAN_SUCCESS;
                }
                else
                {
                    try_num++;
                }
            }
            while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_GLB_RST_TO_SLP));
        }
        else if ((GSTS & CAN_U16_GSTS_GSLPSTS_ON) == 0u)                    /* Not Global Sleep Mode? */
        {
            rtn = CAN_ERR_INVALID_MODE;
        }
        else
        {
            ;   /* No operation if already Global Sleep Mode */
        }
    }
    else
    {
        ;   /* No operation if error has occured */
    }

    /* ---- CAN wakeup settings ---- */
    if (CAN_SUCCESS == rtn)
    {
        if (CAN_WUP_UNUSE == wup)
        {
            CAN0EN = 0u;        /* disable supply CAN input clock */
        }
        else
        {
            /* CAN0 Wakeup */
            RCAN0WUPIF = 0u;
#if (CAN_CFG_INTRCAN0WUP_USE != 0)
            RCAN0WUPMK = 0u;
#else
            RCAN0WUPMK = 1u;
#endif
        }
    }
    else
    {
        ;   /* No operation if error has occured */
    }

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_Sleep
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_SendByTXMB
 * Description  : Transmit a frame by TX message buffer (Channel 0)
 * Arguments    : txbuf_idx -
 *                    TX message buffer index
 *                p_frame -
 *                    pointer to frame to be transmitted
 * Return Value : CAN_SUCCESS -
 *                    normal completion
 *                CAN_ERR_BUF_BUSY -
 *                    TX message buffer is busy (failure to clear TX message buffer status)
 *                CAN_ERR_INVALID_ARG -
 *                    invalid argument specification
 *********************************************************************************************************************/
e_can_err_t R_CAN_SendByTXMB(const can_txbuf_t txbuf_idx,
    const st_can_tx_frame_t * p_frame)
{
    e_can_err_t rtn = CAN_SUCCESS;

    /* ----  Check TX message buffer index ---- */
    if (txbuf_idx >= CAN_MAX_TXBUF_NUM)
    {
        rtn = CAN_ERR_INVALID_ARG;
    }
    else
    {
#if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
        if (NULL == p_frame)
        {
            rtn = CAN_ERR_INVALID_ARG;
        }
    #if ((CAN_CFG_CFDC > 0) && (CAN_CFG_CFM == CAN_CFM_TX_MODE))        /* Use TX mode CFIFO? */
        else if ((can_txbuf_t)CAN_CFG_CFTML == txbuf_idx)
        {
            rtn = CAN_ERR_INVALID_ARG;
        }
    #endif
        else
        {
            ;   /* No operation */
        }
#endif
    }

    /* ---- Clear and check TX message buffer status ---- */
    if (CAN_SUCCESS == rtn)
    {
        /* Clear TX message buffer status */
        CAN_TM_SFR.TMSTSm[txbuf_idx] = CAN_U08_CLR;

        /* Check TX message buffer status */
        if (CAN_U08_CLR != CAN_TM_SFR.TMSTSm[txbuf_idx])                /* Not clear? */
        {
            rtn = CAN_ERR_BUF_BUSY;
        }
        else
        {
            ;   /* No operation */
        }
    }
    else
    {
        ;   /* No operation */
    }

    /* ---- Store message to tx buffer ---- */
    if (CAN_SUCCESS == rtn)
    {
        volatile st_can_msgbuf_sfr_t CAN_NO_BIT_ACCESS * p_msg_sfr;

        uint8_t  fdctr;
        uint8_t  length;
        uint8_t  word_cnt;
        uint16_t idh_mask;
        uint16_t rpage;

        /* ---- FDF/BRS/ESI ---- */
        if (p_frame->Head.Bits.FDCTR >= CAN_FDCTR_FD_NMNL)              /* FD frame? */
        {
            fdctr    = p_frame->Head.Bits.FDCTR;                        /* Set 4 to 7 */
            idh_mask = CAN_U16_TMIDH_RTR_OFF;                           /* Mask RTR */
        }
        else
        {
            fdctr    = CAN_FDCTR_CLASSICAL;                             /* Set 0 if FDCTR=0,1,2,3 */
            idh_mask = CAN_USHORT_MAX;                                  /* No Mask */
        }

        /* ---- Get Length ---- */
        if (p_frame->Head.Bits.DLC < CAN_CLASSICAL_MAX)                 /* DLC: 0 to 7? */
        {
            length = (uint8_t)(p_frame->Head.Bits.DLC);                 /* length=DLC */
        }
        else                                                            /* DLC: 8 or more? */
        {
            if (CAN_FDCTR_CLASSICAL == fdctr)                           /* Classical? */
            {
                length = CAN_CLASSICAL_MAX;                             /* Fixed 8 bytes */
            }
            else
            {
                length = s_dlc_to_length[(p_frame->Head.Bits.DLC - CAN_CLASSICAL_MAX)];
            }
        }

        /* ---- Clear length to 0 if Remote Frame ---- */
        if ((CAN_RTR_CLASSICAL_RMT == p_frame->Head.Bits.RTR) &&        /* Remote Frame? */
            (CAN_FDCTR_CLASSICAL   == fdctr))
        {
            length = 0u;                                                /* No data (remote frame) */
        }

        /* ---- Select RAM window 1 ---- */
        rpage   = CFDGRWC;
        CFDGRWC = CAN_U16_CFDGRWC_PAGE1;

        /* ---- Get pointer of message buffer register ---- */
        p_msg_sfr = &(CAN_RAM_MSG_SFR.TxBufMsg[txbuf_idx]);

        /* ---- Set frame info ---- */
        p_msg_sfr->IDL = 
            p_frame->Head.Word[CAN_TX_HEAD_WORD_IDL];                   /* IDL */
        p_msg_sfr->IDH = 
            (p_frame->Head.Word[CAN_TX_HEAD_WORD_IDH] &
            idh_mask);                                                  /* IDE,THLEN,RTR,IDH */
        p_msg_sfr->PTRHH = 
            (uint8_t)((uint8_t)(p_frame->Head.Bits.DLC) << CAN_POS_TMPTRHH_TMDLC); /* DLC */
        p_msg_sfr->FDCSTSLL = fdctr;                                    /* FDF,BRS,ESI */
        p_msg_sfr->FDCSTSLH = p_frame->Head.Bits.IFL;                   /* IFL */
        p_msg_sfr->FDCSTSH  = p_frame->Head.Bits.LBL;                   /* PTR */

        /* ---- Set frame data ---- */
        for (word_cnt = 0u; word_cnt < CAN_BYTE_TO_WORD(length); word_cnt++ )
        {
            p_msg_sfr->DF[word_cnt] = p_frame->Data.DW[word_cnt];
        }

        /* ---- Restore RAM window ---- */
        CFDGRWC = CAN_U16_CFDGRWC_KEY | rpage;

        /* ---- Set transmission request ---- */
        CAN_TM_SFR.TMCm[txbuf_idx] = CAN_U08_TMC_TMTR_ON;
    }
    else
    {
        ;   /* No operation */
    }

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_SendByTXMB
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_AbortTXMB
 * Description  : Abort a CAN transmission (Channel 0)
 * Arguments    : txbuf_idx -
 *                    TX message buffer index
 * Return Value : CAN_SUCCESS -
 *                    normal completion
 *                CAN_ERR_INVALID_ARG -
 *                    invalid argument specification
 *********************************************************************************************************************/
e_can_err_t R_CAN_AbortTXMB(const can_txbuf_t txbuf_idx)
{
    e_can_err_t rtn = CAN_SUCCESS;

    /* ----  Check TX message buffer index ---- */
    if (txbuf_idx >= CAN_MAX_TXBUF_NUM)
    {
        rtn = CAN_ERR_INVALID_ARG;
    }
#if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
    #if ((CAN_CFG_CFDC > 0) && (CAN_CFG_CFM == CAN_CFM_TX_MODE))    /* Use TX mode CFIFO? */
    else if ((can_txbuf_t)CAN_CFG_CFTML == txbuf_idx)
    {
        rtn = CAN_ERR_INVALID_ARG;
    }
    #endif
#endif
    else
    {
        /* ---- Set transmission abort request ---- */
        CAN_TM_SFR.TMCm[txbuf_idx] = CAN_U08_TMC_TMTAR_ON;
    }

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_AbortTXMB
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_GetTXMBResult
 * Description  : Read the result of transmission from TX message buffer (Channel 0)
 * Arguments    : txbuf_idx -
 *                    TX message buffer index
 * Return Value : CAN_TXB_TRANSMITTING -
 *                    Transmission is in progress
 *                    or no transmit request is present.
 *                CAN_TXB_ABORT_OVER -
 *                    Transmit abort has been completed.
 *                CAN_TXB_END -
 *                    Transmission has been completed
 *                    (without transmit abort request).
 *                CAN_TXB_END_WITH_ABORT -
 *                    Transmission has been completed
 *                    (with transmit abort request).
 *                CAN_TXB_ARG_ERROR -
 *                    invalid argument specification
 *********************************************************************************************************************/
e_can_txb_result_t R_CAN_GetTXMBResult(const can_txbuf_t txbuf_idx)
{
    e_can_txb_result_t result;

    /* ----  Check TX message buffer index ---- */
    if (txbuf_idx >= CAN_MAX_TXBUF_NUM)
    {
        result = CAN_TXB_ARG_ERROR;
    }
#if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
    #if ((CAN_CFG_CFDC > 0) && (CAN_CFG_CFM == CAN_CFM_TX_MODE))    /* Use TX mode CFIFO? */
    else if ((can_txbuf_t)CAN_CFG_CFTML == txbuf_idx)
    {
        result = CAN_TXB_ARG_ERROR;
    }
    #endif
#endif
    else
    {
        uint8_t tmtrf_value;

        /* Get TMTRF value */
        tmtrf_value = ((CAN_TM_SFR.TMSTSm[txbuf_idx] & CAN_U08_TMSTS_TMTRF_MASK) >> CAN_POS_TMSTS_TMTRF);

        /* Set return value */
        result = (e_can_txb_result_t)tmtrf_value;

        /* ---- Tx transmission completed/abort? ---- */
        if (CAN_TXB_TRANSMITTING != result)
        {
            /* Clear TX message buffer status */
            CAN_TM_SFR.TMSTSm[txbuf_idx] = CAN_U08_CLR;
        }
        else
        {
            ;   /* No operation */
        }
    }

    return result;
}
/**********************************************************************************************************************
 End of function R_CAN_GetTXMBResult
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_SendByCFIFO
 * Description  : This function checks to see if the Common FIFO is full.
 *                If not, the appropriate registers are loaded with the message
 *                and marked as ready to send.
 * Arguments    : p_frame
 *                    Pointer to structure containing message info to send
 * Return Value : CAN_SUCCESS
 *                    Successful
 *                CAN_ERR_BUF_FULL
 *                    Cannot send at this time (Common FIFO is full.)
 *                CAN_ERR_INVALID_ARG
 *                    An invalid value passed for one of the parameters.
 *********************************************************************************************************************/
e_can_err_t R_CAN_SendByCFIFO(const st_can_tx_frame_t * p_frame)
{
    e_can_err_t rtn;

#if ((CAN_CFG_CFDC == 0) || (CAN_CFG_CFM != CAN_CFM_TX_MODE))       /* Not use TX mode CFIFO? */
    {
        rtn = CAN_ERR_INVALID_ARG;
    }
#else
    {
        rtn = CAN_SUCCESS;

        /* ---- Check if Common FIFO is full ---- */
        if ((CFSTS & CAN_U16_CFSTS_CFFLL_ON) != 0u)
        {
            rtn = CAN_ERR_BUF_FULL;
        }
        else
        {
    #if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
            if (NULL == p_frame)
            {
                rtn = CAN_ERR_INVALID_ARG;
            }
    #endif
        }

        /* ---- Send message into Common FIFO if it is not full ---- */
        if (CAN_SUCCESS == rtn)
        {
            uint8_t  fdctr;
            uint8_t  length;
            uint8_t  word_cnt;
            uint16_t idh_mask;
            uint16_t rpage;

            /* ---- FDF/BRS/ESI ---- */
            if (p_frame->Head.Bits.FDCTR >= CAN_FDCTR_FD_NMNL)      /* FD frame? */
            {
                fdctr    = p_frame->Head.Bits.FDCTR;                /* Set 4 to 7 */
                idh_mask = CAN_U16_CFIDH_RTR_OFF;                   /* Mask RTR */
            }
            else
            {
                fdctr    = CAN_FDCTR_CLASSICAL;                     /* Set 0 if FDCTR=0,1,2,3 */
                idh_mask = CAN_USHORT_MAX;                          /* No Mask */
            }

            /* ---- Get Length ---- */
            if (p_frame->Head.Bits.DLC < CAN_CLASSICAL_MAX)         /* DLC: 0 to 7? */
            {
                length = p_frame->Head.Bits.DLC;                    /* length=DLC */
            }
            else                                                    /* DLC: 8 or more? */
            {
                if (CAN_FDCTR_CLASSICAL == fdctr)                   /* Classical? */
                {
                    length = CAN_CLASSICAL_MAX;                     /* Fixed 8 bytes */
                }
                else                                                /* CAN FD */
                {
                    length = s_dlc_to_length[(p_frame->Head.Bits.DLC - CAN_CLASSICAL_MAX)];
                }
            }

            /* ---- Clear length to 0 if Remote Frame ---- */
            if ((CAN_RTR_CLASSICAL_RMT == p_frame->Head.Bits.RTR) && /* Remote Frame? */
                (CAN_FDCTR_CLASSICAL   == fdctr))
            {
                length = 0u;                                        /* No data (remote frame) */
            }

            /* ---- Check length max ---- */
            if (length > s_dlc_to_length[CAN_CFG_CFPLS])
            {
                length = s_dlc_to_length[CAN_CFG_CFPLS];
            }

            /* ---- Select RAM window 1 ---- */
            rpage   = CFDGRWC;
            CFDGRWC = CAN_U16_CFDGRWC_PAGE1;

            /* ---- Set frame info ---- */
            CFIDL      = p_frame->Head.Word[CAN_TX_HEAD_WORD_IDL];  /* IDL */
            CFIDH      = (p_frame->Head.Word[CAN_TX_HEAD_WORD_IDH] &
                idh_mask);                                          /* IDE,THLEN,RTR,IDH */
            CFPTRHH    = (uint8_t)((uint8_t)(p_frame->Head.Bits.DLC) << CAN_POS_CFPTRHH_CFDLC); /* DLC */
            CFFDCSTSLL = fdctr;                                     /* FDF,BRS,ESI */
            CFFDCSTSLH = p_frame->Head.Bits.IFL;                    /* IFL */
            CFFDCSTSH  = p_frame->Head.Bits.LBL;                    /* PTR */

            /* ---- Set frame data ---- */
            for (word_cnt = 0u; word_cnt < CAN_BYTE_TO_WORD(length); word_cnt++ )
            {
                CAN_RAM_MSG_SFR.CFIFOMsg.DF[word_cnt] = p_frame->Data.DW[word_cnt];
            }

            /* ---- Restore RAM window ---- */
            CFDGRWC = CAN_U16_CFDGRWC_KEY | rpage;

            /* ---- Increment Tx/Rx FIFO buffer pointer ---- */
            CFPCTRL = CAN_U08_CFPCTRL_FIFO_INC;
        }
        else
        {
            ;   /* No operation */
        }
    }
#endif /* end of (CAN_CFG_CFDC == 0 || CAN_CFG_CFM != CAN_CFM_TX_MODE) */

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_SendByCFIFO
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_AbortCFIFO
 * Description  : Abort Common FIFO transmission (Channel 0)
 * Arguments    : none
 * Return Value : CAN_SUCCESS -
 *                    normal completion
 *                CAN_ERR_BUF_BUSY -
 *                    Common FIFO is busy (failure to set Common FIFO enable)
 *                CAN_ERR_INVALID_ARG -
 *                    invalid argument specification
 *********************************************************************************************************************/
e_can_err_t R_CAN_AbortCFIFO(void)
{
    e_can_err_t rtn;

#if (CAN_CFG_CFDC == 0)
    {
        rtn = CAN_ERR_INVALID_ARG;  /* Not use Common FIFO */
    }
#else
    {
        uint16_t    try_num;

        /* ---- Set disable ---- */
        CFCCLL &= CAN_U08_CFCCL_CFE_OFF;

        /* ---- Wait until FIFO is empty ---- */
        rtn     = CAN_ERR_BUF_BUSY;
        try_num = 0u;
        do
        {
            if ((CFSTS & CAN_U16_CFSTS_CFEMP_ON) != 0u)   /* Empty? */
            {
                rtn = CAN_SUCCESS;
            }
            else
            {
                try_num++;
            }
        }
        while ((CAN_SUCCESS != rtn) && (try_num < CAN_TRY_MIN_TIMES));

        /* ---- Clear FIFO interrupt flag and Message Lost ---- */
        CFSTSL = CAN_U08_CLR;

        /* ---- Set enable ---- */
        if (CAN_SUCCESS == rtn)
        {
            CFCCL |= CAN_U16_CFCCL_CFE_ON;
        }
    }
#endif /* end of (CAN_CFG_CFDC == 0) */

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_AbortCFIFO
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_ReadTxHistory
 * Description  : Read entry from TX history list
 * Arguments    : p_entry -
 *                    pointer to stored TX history list entry
 * Return Value : CAN_SUCCESS -
 *                    A TX history entry is successfully read out.
 *                CAN_SUCCESS_WITH_LOST -
 *                    A TX history entry is successfully read out (with entry lost).
 *                CAN_ERR_BUF_EMPTY -
 *                    No TX history entry is read out.
 *                CAN_ERR_INVALID_ARG -
 *                    invalid argument specification
 *********************************************************************************************************************/
e_can_err_t R_CAN_ReadTxHistory(st_can_txhist_t *p_entry)
{
    e_can_err_t rtn = CAN_SUCCESS;

#if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
    if (NULL == p_entry)
    {
        rtn = CAN_ERR_INVALID_ARG;
    }
    else
#endif
    {
        uint16_t thlsts_value;

        /* ----  Check if any unread entry is available in TX history list ---- */
        thlsts_value = THLSTS;

        if ((thlsts_value & CAN_U16_THLSTS_THLEMP_ON) != 0u)
        {
            rtn = CAN_ERR_BUF_EMPTY;
        }
        else if ((thlsts_value & CAN_U16_THLSTS_THLELT_ON) != 0u)
        {
            /* ---- Clear message lost flag ---- */
            THLSTSL = CAN_U08_THLSTSL_CLR_THLELT;

            /* ----  Set return value ---- */
            rtn = CAN_SUCCESS_WITH_LOST;
        }
        else
        {
            ;   /* No operation */
        }

        if (rtn != CAN_ERR_BUF_EMPTY)
        {
            uint16_t thlacc_value;
            uint16_t rpage;

            /* ---- Select RAM window 1 ---- */
            rpage   = CFDGRWC;
            CFDGRWC = CAN_U16_CFDGRWC_PAGE1;

            /* ---- Read TX history list entry ---- */
            thlacc_value = THLACC0L;
            p_entry->TS  = THLACC0H;
            p_entry->LBL = THLACC1L;
            p_entry->IFL = (THLACC1H & CAN_U16_THLACC1H_TIFL_MASK);

            /* ---- Restore RAM window ---- */
            CFDGRWC = CAN_U16_CFDGRWC_KEY | rpage;

            /* Get Tx buffer type and number */
            if ((thlacc_value & CAN_U16_THLACC0L_BT_MASK) == CAN_U16_THLACC0L_BT_CFIFO)
            {
                /* ----  Not read TX buffer index (CFIFO)  ---- */
                p_entry->txbuf_idx = CAN_TXBUF_NOT;
            }
            else
            {
                /* ----  Read TX buffer index ---- */
                p_entry->txbuf_idx = (thlacc_value & CAN_U16_THLACC0L_BN_MASK) >> CAN_POS_THLACC0L_BN;
            }

            /* ---- Increment TX history list entry pointer ---- */
            THLPCTRL = CAN_U08_THLPCTRL_LIST_INC;
        }
    }

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_ReadTxHistory
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_GetTDCResult
 * Description  : Read transceiver delay compensation result
 * Arguments    : none
 * Return Value : Transceiver delay compensation result (0 to 0xFF)
 *********************************************************************************************************************/
uint8_t R_CAN_GetTDCResult(void)
{
    return  C0FDSTSLL;
}
/**********************************************************************************************************************
 End of function R_CAN_GetTDCResult
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_ReadRXMB
 * Description  : Read message from RX message buffer
 * Arguments    : rxbuf_idx -
 *                    RX message buffer that receives frame
 *                p_frame -
 *                    pointer to stored frame position
 * Return Value : CAN_SUCCESS -
 *                    A frame is successfully read out.
 *                CAN_ERR_BUF_EMPTY -
 *                    No frame is read out.
 *                CAN_ERR_BUF_BUSY -
 *                    RX message buffer is busy (failure to clear Rx complete flag)
 *                CAN_ERR_OVERWRITE -
 *                    A frame is overwritten.
 *                CAN_ERR_INVALID_ARG -
 *                    invalid argument specification
 *********************************************************************************************************************/
e_can_err_t R_CAN_ReadRXMB(const can_rxbuf_t rxbuf_idx,
    st_can_rx_frame_t * p_frame,
    can_length_t      * p_length)
{
    e_can_err_t rtn;

#if (CAN_CFG_NRXMB == 0)
    {
        rtn = CAN_ERR_INVALID_ARG;  /* Not use RX Buffer */
    }
#else
    {
        uint16_t rmnd_pattern;

        /* ----  Check RX message buffer index ---- */
        if (rxbuf_idx >= (can_rxbuf_t)CAN_CFG_NRXMB)
        {
            rmnd_pattern = 0u;

            rtn = CAN_ERR_INVALID_ARG;
        }
    #if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
        else if ((NULL == p_frame) || (NULL == p_length))
        {
            rmnd_pattern = 0u;

            rtn = CAN_ERR_INVALID_ARG;
        }
    #endif
        else
        {
            /* Make 16bits pattern */
            rmnd_pattern = ((uint16_t)1u << rxbuf_idx);

            /* ---- Judge if new messages are available ---- */
            if ((RMND & rmnd_pattern) == 0u)
            {
                rtn = CAN_ERR_BUF_EMPTY;
            }
            else
            {
                /* ---- Clear Rx complete flag of corresponding RX message buffer ---- */
                RMND = (uint16_t)~rmnd_pattern;

                if ((RMND & rmnd_pattern) != 0u)
                {
                    rtn = CAN_ERR_BUF_BUSY;
                }
                else
                {
                    rtn = CAN_SUCCESS;
                }
            }
        }

        /* ---- Read out message from RX message buffer ---- */
        if (CAN_SUCCESS == rtn)
        {
            volatile st_can_msgbuf_sfr_t CAN_NO_BIT_ACCESS * p_msg_sfr;

            uint8_t  length;
            uint8_t  word_cnt;
            uint16_t rpage;

            /* ---- Select RAM window 2 or 3 ---- */
            rpage = CFDGRWC;
            if (rxbuf_idx < CAN_HALF_RXBUF_NUM)         /* RXBUF0, ..., RXBUF7 */
            {
                CFDGRWC = CAN_U16_CFDGRWC_PAGE2;
            }
            else                                        /* RXBUF8, ..., RXBUF15 */
            {
                CFDGRWC = CAN_U16_CFDGRWC_PAGE3;
            }

            /* ---- Get pointer of message buffer register ---- */
            p_msg_sfr = &(CAN_RAM_RM_MSG_SFR.RxBufMsg[rxbuf_idx % CAN_HALF_RXBUF_NUM]);

            /* ---- Read frame data ---- */
            p_frame->Head.Word[CAN_RX_HEAD_WORD_IDL] = p_msg_sfr->IDL;  /* IDL */
            p_frame->Head.Word[CAN_RX_HEAD_WORD_IDH] = p_msg_sfr->IDH;  /* IDE,RTR,IDH */

            p_frame->Head.Word[CAN_RX_HEAD_WORD_FDCTR] =
                (uint16_t)(p_msg_sfr->FDCSTSLL) & CAN_MASK_W3;          /* FDF/BRS/ESI */

            p_frame->Head.Bits.DLC = p_msg_sfr->PTRHH >> CAN_POS_RMPTRHH_RMDLC; /* DLC */
            p_frame->Head.Bits.IFL = p_msg_sfr->FDCSTSLH & CAN_MASK_W2; /* IFL */
            p_frame->Head.Bits.LBL = p_msg_sfr->FDCSTSH;                /* Label */
            p_frame->Head.Bits.TS  = p_msg_sfr->PTRL;                   /* Time Stamp */

            /* ---- Get Length ---- */
            if (p_frame->Head.Bits.DLC < CAN_CLASSICAL_MAX)             /* DLC: 0 to 7? */
            {
                length = p_frame->Head.Bits.DLC;                        /* length=DLC */
            }
            else                                                        /* DLC: 8 or more? */
            {
                if (CAN_FDF_CLASSICAL == p_frame->Head.Bits.FDF)        /* Classical? */
                {
                    length = CAN_CLASSICAL_MAX;                         /* Fixed 8 bytes */
                }
                else                                                    /* CAN FD */
                {
                    length = s_dlc_to_length[(p_frame->Head.Bits.DLC - CAN_CLASSICAL_MAX)];
                }
            }

            /* ---- Clear length to 0 if Remote Frame ---- */
            if ((CAN_RTR_CLASSICAL_RMT == p_frame->Head.Bits.RTR) &&    /* Remote Frame? */
                (CAN_FDF_CLASSICAL     == p_frame->Head.Bits.FDF))
            {
                length = 0u;                                            /* No data (remote frame) */
            }

            /* ---- Check length max ---- */
            if (length > s_dlc_to_length[CAN_CFG_RMPLS])
            {
                length = s_dlc_to_length[CAN_CFG_RMPLS];
            }

            /* ---- Set frame data and length ---- */
            for (word_cnt = 0u; word_cnt < CAN_BYTE_TO_WORD(length); word_cnt++ )
            {
                p_frame->Data.DW[word_cnt] = p_msg_sfr->DF[word_cnt];
            }
            *p_length = length;

            /* ---- Restore RAM window ---- */
            CFDGRWC = CAN_U16_CFDGRWC_KEY | rpage;

            /* ---- Judge if current message is overwritten ---- */
            if ((RMND & rmnd_pattern) != 0u)
            {
                rtn = CAN_ERR_OVERWRITE;
            }
        }
    }
#endif  /* end of (CAN_CFG_NRXMB == 0) */

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_ReadRXMB
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_ReadRXMBInHandler
 * Description  : Read message from RX message buffer (used in CAN_CFG_CALLBACK_GLB_RXBUF)
 * Arguments    : rxbuf_idx -
 *                    RX message buffer that receives frame
 *                p_frame -
 *                    pointer to stored frame position
 * Return Value : CAN_SUCCESS -
 *                    A frame is successfully read out.
 *                CAN_ERR_OVERWRITE -
 *                    A frame is overwritten.
 *                CAN_ERR_INVALID_ARG -
 *                    invalid argument specification
 *********************************************************************************************************************/
#if (CAN_CFG_INTRCANGRVC_USE != 0)
e_can_err_t R_CAN_ReadRXMBInHandler(const can_rxbuf_t rxbuf_idx,
    st_can_rx_frame_t * p_frame,
    can_length_t      * p_length)
{
    e_can_err_t rtn;

    #if (CAN_CFG_NRXMB == 0)
    {
        rtn = CAN_ERR_INVALID_ARG;  /* Not use RX Buffer */
    }
    #else
    {
        volatile st_can_msgbuf_sfr_t CAN_NO_BIT_ACCESS * p_msg_sfr;

        uint8_t  length;
        uint8_t  word_cnt;
        uint16_t rpage;

        /* ----  Check RX message buffer index ---- */
        if (rxbuf_idx >= (can_rxbuf_t)CAN_CFG_NRXMB)
        {
            rtn = CAN_ERR_INVALID_ARG;
        }
    #if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
        else if ((NULL == p_frame) || (NULL == p_length))
        {
            rtn = CAN_ERR_INVALID_ARG;
        }
    #endif
        else
        {
            rtn = CAN_SUCCESS;

            /* ---- Select RAM window 2 or 3 ---- */
            rpage = CFDGRWC;
            if (rxbuf_idx < CAN_HALF_RXBUF_NUM)         /* RXBUF0, ..., RXBUF7 */
            {
                CFDGRWC = CAN_U16_CFDGRWC_PAGE2;
            }
            else                                        /* RXBUF8, ..., RXBUF15 */
            {
                CFDGRWC = CAN_U16_CFDGRWC_PAGE3;
            }

            /* ---- Get pointer of message buffer register ---- */
            p_msg_sfr = &(CAN_RAM_RM_MSG_SFR.RxBufMsg[rxbuf_idx % CAN_HALF_RXBUF_NUM]);

            /* ---- Read frame data ---- */
            p_frame->Head.Word[CAN_RX_HEAD_WORD_IDL] = p_msg_sfr->IDL;      /* IDL */
            p_frame->Head.Word[CAN_RX_HEAD_WORD_IDH] = p_msg_sfr->IDH;      /* IDE,RTR,IDH */

            p_frame->Head.Word[CAN_RX_HEAD_WORD_FDCTR] =
                (uint16_t)(p_msg_sfr->FDCSTSLL) & CAN_MASK_W3;              /* FDF/BRS/ESI */

            p_frame->Head.Bits.DLC = p_msg_sfr->PTRHH >> CAN_POS_RMPTRHH_RMDLC; /* DLC */
            p_frame->Head.Bits.IFL = p_msg_sfr->FDCSTSLH & CAN_MASK_W2;     /* IFL */
            p_frame->Head.Bits.LBL = p_msg_sfr->FDCSTSH;                    /* Label */
            p_frame->Head.Bits.TS  = p_msg_sfr->PTRL;                       /* Time Stamp */

            /* ---- Get Length ---- */
            if (p_frame->Head.Bits.DLC < CAN_CLASSICAL_MAX)     /* DLC: 0 to 7? */
            {
                length = p_frame->Head.Bits.DLC;                /* length=DLC */
            }
            else                                                /* DLC: 8 or more? */
            {
                if (CAN_FDF_CLASSICAL == p_frame->Head.Bits.FDF) /* Classical? */
                {
                    length = CAN_CLASSICAL_MAX;                 /* Fixed 8 bytes */
                }
                else
                {
                    length = s_dlc_to_length[(p_frame->Head.Bits.DLC - CAN_CLASSICAL_MAX)];
                }
            }

            /* ---- Clear length to 0 if Remote Frame ---- */
            if ((CAN_RTR_CLASSICAL_RMT == p_frame->Head.Bits.RTR) &&        /* Remote Frame? */
                (CAN_FDF_CLASSICAL     == p_frame->Head.Bits.FDF))
            {
                length = 0u;                                                /* No data (remote frame) */
            }

            /* ---- Check length max ---- */
            if (length > s_dlc_to_length[CAN_CFG_RMPLS])
            {
                length = s_dlc_to_length[CAN_CFG_RMPLS];
            }

            /* ---- Set frame data and length ---- */
            for (word_cnt = 0u; word_cnt < CAN_BYTE_TO_WORD(length); word_cnt++ )
            {
                p_frame->Data.DW[word_cnt] = p_msg_sfr->DF[word_cnt];
            }
            *p_length = length;

            /* ---- Restore RAM window ---- */
            CFDGRWC = CAN_U16_CFDGRWC_KEY | rpage;

            /* ---- Judge if current message is overwritten ---- */
            if ((RMND & ((uint16_t)1u << rxbuf_idx)) != 0u)
            {
                rtn = CAN_ERR_OVERWRITE;
            }
        }
    }
    #endif  /* end of (CAN_CFG_NRXMB == 0) */

    return rtn;
}
#endif /* end of (CAN_CFG_INTRCANGRVC_USE != 0) */
/**********************************************************************************************************************
 End of function R_CAN_ReadRXMBInHandler
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_ReadRXFIFO
 * Description  : Read message from RXFIFO
 * Arguments    : p_frame -
 *                    pointer to stored frame position
 * Return Value : CAN_SUCCESS -
 *                    A frame is successfully read out.
 *                CAN_SUCCESS_WITH_LOST -
 *                    A frame is successfully read out (with message lost).
 *                CAN_ERR_BUF_EMPTY -
 *                    No frame is read out.
 *                CAN_ERR_INVALID_ARG -
 *                    invalid argument specification
 *********************************************************************************************************************/
e_can_err_t R_CAN_ReadRXFIFO(const can_rxfifo_t rxfifo_idx,
    st_can_rx_frame_t * p_frame,
    can_length_t      * p_length)
{
    e_can_err_t rtn = CAN_SUCCESS;

#if ((CAN_CFG_RF0_RFDC | CAN_CFG_RF1_RFDC) == 0)    /* Not use RXFIFO? */
    {
        rtn = CAN_ERR_INVALID_ARG;
    }
#else
    {
        /* ----  Check RX FIFO index ---- */
        if (rxfifo_idx >= CAN_MAX_RXFIFO_NUM)
        {
            rtn = CAN_ERR_INVALID_ARG;
        }
        else
        {
    #if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
            if ((NULL == p_frame) || (NULL == p_length))
            {
                rtn = CAN_ERR_INVALID_ARG;
            }
        #if (CAN_CFG_RF0_RFDC == 0)
            else if (CAN_RXFIFO0 == rxfifo_idx)
            {
                rtn = CAN_ERR_INVALID_ARG;
            }
        #endif
        #if (CAN_CFG_RF1_RFDC == 0)
            else if (CAN_RXFIFO1 == rxfifo_idx)
            {
                rtn = CAN_ERR_INVALID_ARG;
            }
        #endif
            else
            {
                ;   /* No operation */
            }
    #endif
        }

        if (CAN_SUCCESS == rtn)
        {
            uint16_t rfsts_value;

            /* ----  Check if any unread message is available in Rx FIFO ---- */
            rfsts_value = CAN_RF_SFR.RFSTSk[rxfifo_idx].U16Reg.LowWord;

            if ((rfsts_value & CAN_U16_RFSTS_RFEMP_ON) != 0u)       /* FIFO Empty? */
            {
                rtn = CAN_ERR_BUF_EMPTY;
            }
            else
            {
                /* ---- Check if Rx FIFO has message lost ---- */
                if ((rfsts_value & CAN_U16_RFSTS_RFMLT_ON) != 0u)   /* Message Lost? */
                {
                    /* ---- Clear message lost flag ---- */
                    CAN_RF_SFR.RFSTSk[rxfifo_idx].U08Reg.LByte = CAN_U08_RFSTSL_CLR_RFMLT;

                    /* ----  Set return value ---- */
                    rtn = CAN_SUCCESS_WITH_LOST;
                }
            }
        }

        /* ---- Read out message from Rx FIFO ---- */
        if ((CAN_SUCCESS == rtn) || (CAN_SUCCESS_WITH_LOST == rtn))
        {
            volatile st_can_msgbuf_sfr_t CAN_NO_BIT_ACCESS * p_msg_sfr;

            uint8_t  length;
            uint8_t  rxfifo_pls;
            uint8_t  word_cnt;
            uint16_t rpage;

            /* ---- Select RAM window 1 ---- */
            rpage   = CFDGRWC;
            CFDGRWC = CAN_U16_CFDGRWC_PAGE1;

            /* ---- Get pointer of message buffer register ---- */
            p_msg_sfr = &(CAN_RAM_MSG_SFR.RxFIFOMsg[rxfifo_idx]);

            /* ---- Read frame data ---- */
            p_frame->Head.Word[CAN_RX_HEAD_WORD_IDL] = p_msg_sfr->IDL;      /* IDL */
            p_frame->Head.Word[CAN_RX_HEAD_WORD_IDH] = p_msg_sfr->IDH;      /* IDE,RTR,IDH */

            p_frame->Head.Word[CAN_RX_HEAD_WORD_FDCTR] =
                (uint16_t)(p_msg_sfr->FDCSTSLL) & CAN_MASK_W3;              /* FDF/BRS/ESI */

            p_frame->Head.Bits.DLC = p_msg_sfr->PTRHH >> CAN_POS_RFPTRHH_RFDLC; /* DLC */
            p_frame->Head.Bits.IFL = p_msg_sfr->FDCSTSLH & CAN_MASK_W2;     /* IFL */
            p_frame->Head.Bits.LBL = p_msg_sfr->FDCSTSH;                    /* Label */
            p_frame->Head.Bits.TS  = p_msg_sfr->PTRL;                       /* Time Stamp */

            /* ---- Get Length ---- */
            if (p_frame->Head.Bits.DLC < CAN_CLASSICAL_MAX)                 /* DLC: 0 to 7? */
            {
                length = p_frame->Head.Bits.DLC;                            /* length=DLC */
            }
            else                                                            /* DLC: 8 or more? */
            {
                if (CAN_FDF_CLASSICAL == p_frame->Head.Bits.FDF)            /* Classical? */
                {
                    length = CAN_CLASSICAL_MAX;                             /* Fixed 8 bytes */
                }
                else
                {
                    length = s_dlc_to_length[(p_frame->Head.Bits.DLC - CAN_CLASSICAL_MAX)];
                }
            }

            /* ---- Check length max ---- */
            rxfifo_pls = s_rxfifo_pls[rxfifo_idx];
            if (length > s_dlc_to_length[rxfifo_pls])
            {
                length = s_dlc_to_length[rxfifo_pls];
            }

            /* ---- Clear length to 0 if Remote Frame ---- */
            if ((CAN_RTR_CLASSICAL_RMT == p_frame->Head.Bits.RTR) &&        /* Remote Frame? */
                (CAN_FDF_CLASSICAL     == p_frame->Head.Bits.FDF))
            {
                length = 0u;                                                /* No data (remote frame) */
            }

            /* ---- Set frame data and length ---- */
            for (word_cnt = 0u; word_cnt < CAN_BYTE_TO_WORD(length); word_cnt++ )
            {
                p_frame->Data.DW[word_cnt] = p_msg_sfr->DF[word_cnt];
            }
            *p_length = length;

            /* ---- Restore RAM window ---- */
            CFDGRWC = CAN_U16_CFDGRWC_KEY | rpage;

            /* ---- Increment Rx FIFO buffer pointer ---- */
            CAN_RF_SFR.RFPCTRk[rxfifo_idx].U08Reg.LByte = CAN_U08_RFPCTRL_FIFO_INC;

        }
    }
#endif  /* end of ((CAN_CFG_RF0_RFDC | CAN_CFG_RF1_RFDC) == 0) */

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_ReadRXFIFO
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_ReadCFIFO
 * Description  : Read message from Common FIFO (Channel 0)
 * Arguments    : p_frame -
 *                    pointer to stored frame position
 *                p_length -
 *                    pointer to stored frame length
 * Return Value : CAN_SUCCESS -
 *                    A frame is successfully read out.
 *                CAN_SUCCESS_WITH_LOST -
 *                    A frame is successfully read out (with message lost).
 *                CAN_ERR_BUF_EMPTY -
 *                    No frame is read out.
 *                CAN_ERR_INVALID_ARG -
 *                    invalid argument specification
 *********************************************************************************************************************/
e_can_err_t R_CAN_ReadCFIFO(st_can_rx_frame_t * p_frame,
    can_length_t * p_length)
{
    e_can_err_t rtn;

#if ((CAN_CFG_CFDC == 0) || (CAN_CFG_CFM != CAN_CFM_RX_MODE))   /* Not use RX mode CFIFO? */
    {
        rtn = CAN_ERR_INVALID_ARG;
    }
#else
    {
        uint16_t    cfsts_value;

        rtn = CAN_SUCCESS;

    #if (CAN_CFG_PARAM_CHECKING_ENABLE == 1)
        if ((NULL == p_frame) || (NULL == p_length))
        {
            rtn = CAN_ERR_INVALID_ARG;
        }
        else
    #endif
        {
            /* ----  Check if any unread message is available in Common FIFO ---- */
            cfsts_value = CFSTS;

            if ((cfsts_value & CAN_U16_CFSTS_CFEMP_ON) != 0u)
            {
                rtn = CAN_ERR_BUF_EMPTY;
            }
            else if ((cfsts_value & CAN_U16_CFSTS_CFMLT_ON) != 0u)
            {
                /* ---- Clear message lost flag ---- */
                CFSTSL = CAN_U08_CFSTSL_CLR_CFMLT;

                /* ----  Set return value ---- */
                rtn = CAN_SUCCESS_WITH_LOST;
            }
            else
            {
                ;   /* No operation */
            }
        }

        /* ---- Read out message from Common FIFO ---- */
        if ((CAN_SUCCESS == rtn) || (CAN_SUCCESS_WITH_LOST == rtn))
        {
            uint8_t  length;
            uint8_t  word_cnt;
            uint16_t rpage;

            /* ---- Select RAM window 1 ---- */
            rpage   = CFDGRWC;
            CFDGRWC = CAN_U16_CFDGRWC_PAGE1;

            /* ---- Read frame data ---- */
            p_frame->Head.Word[CAN_RX_HEAD_WORD_IDL]   = CFIDL; /* IDL */
            p_frame->Head.Word[CAN_RX_HEAD_WORD_IDH]   = CFIDH; /* IDE,RTR,IDH */

            p_frame->Head.Word[CAN_RX_HEAD_WORD_FDCTR] =
                (uint16_t)CFFDCSTSLL & CAN_MASK_W3;             /* FDF/BRS/ESI */

            p_frame->Head.Bits.DLC = CFPTRHH >> CAN_POS_CFPTRHH_CFDLC; /* DLC */
            p_frame->Head.Bits.IFL = CFFDCSTSLH & CAN_MASK_W2;  /* IFL */
            p_frame->Head.Bits.LBL = CFFDCSTSH;                 /* Label */
            p_frame->Head.Bits.TS  = CFPTRL;                    /* Time Stamp */

            /* ---- Get Length ---- */
            if (p_frame->Head.Bits.DLC < CAN_CLASSICAL_MAX)     /* DLC: 0 to 7? */
            {
                length = p_frame->Head.Bits.DLC;                /* length=DLC */
            }
            else                                                /* DLC: 8 or more? */
            {
                if (CAN_FDF_CLASSICAL == p_frame->Head.Bits.FDF) /* Classical? */
                {
                    if (CAN_RTR_DATA_FRAME == p_frame->Head.Bits.RTR) /* Data Frame? */
                    {
                        length = CAN_CLASSICAL_MAX;             /* Fixed 8 bytes */
                    }
                    else
                    {
                        length = 0u;                            /* No data (remote frame) */
                    }
                }
                else
                {
                    length = s_dlc_to_length[(p_frame->Head.Bits.DLC - CAN_CLASSICAL_MAX)];
                }
            }

            /* ---- Clear length to 0 if Remote Frame ---- */
            if ((CAN_RTR_CLASSICAL_RMT == p_frame->Head.Bits.RTR) &&        /* Remote Frame? */
                (CAN_FDF_CLASSICAL     == p_frame->Head.Bits.FDF))
            {
                length = 0u;                                                /* No data (remote frame) */
            }

            /* ---- Check length max ---- */
            if (length > s_dlc_to_length[CAN_CFG_CFPLS])
            {
                length = s_dlc_to_length[CAN_CFG_CFPLS];
            }

            /* ---- Set frame data and length ---- */
            for (word_cnt = 0u; word_cnt < CAN_BYTE_TO_WORD(length); word_cnt++ )
            {
                p_frame->Data.DW[word_cnt] = CAN_RAM_MSG_SFR.CFIFOMsg.DF[word_cnt];
            }
            *p_length = length;

            /* ---- Restore RAM window ---- */
            CFDGRWC = CAN_U16_CFDGRWC_KEY | rpage;

            /* ---- Increment Tx/Rx FIFO buffer pointer ---- */
            CFPCTRL = CAN_U08_CFPCTRL_FIFO_INC;
        }
        else
        {
            ;   /* No operation */
        }
    }
#endif /* end of (CAN_CFG_CFDC == 0 || CAN_CFG_CFM != CAN_CFM_RX_MODE) */

    return rtn;
}
/**********************************************************************************************************************
 End of function R_CAN_ReadCFIFO
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_GetChStatus
 * Description  : Read channel status (Channel 0)
 * Arguments    : none
 * Return Value : channel status (<= 0x1FF) -
 *                    normal completion
 *********************************************************************************************************************/
uint16_t R_CAN_GetChStatus(void)
{
    uint16_t    ch_sts_value;

    ch_sts_value = C0STSL;

    if ((ch_sts_value & CAN_U16_C0STSL_ESIF_ON) != 0u)
    {
        C0STSLH = CAN_U08_CLR;
    }
    else
    {
        ;   /* No operation if ESIF=0 */
    }

    return ch_sts_value;
}
/**********************************************************************************************************************
 End of function R_CAN_GetChStatus
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_GetChBusErrFlag
 * Description  : Read channel bus error flags (Channel 0)
 * Arguments    : none
 * Return Value : channel bus error flags
 *********************************************************************************************************************/
uint8_t R_CAN_GetChBusErrFlag(void)
{
    uint8_t ch_erfl_lh_value;

    /* Read channel 0 error flag register (LH byte) */
    ch_erfl_lh_value = C0ERFLLH;

    /* Clear channel 0 error flag (ADERR, B0ERR, B1ERR, CERR, AFERR, FERR, SERR) */
    C0ERFLLH = (~ch_erfl_lh_value) & CAN_MASK_W7;

    return ch_erfl_lh_value;
}
/**********************************************************************************************************************
 End of function R_CAN_ReadChBusErrFlag
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_GetTSCounter
 * Description  : Return the timestamp counter value at that time
 * Arguments    : none
 * Return Value : Timestamp counter value (0 to 0xFFFF)
 *********************************************************************************************************************/
uint16_t R_CAN_GetTSCounter(void)
{
    return GTSC;
}
/**********************************************************************************************************************
 End of function R_CAN_GetTSCounter
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: R_CAN_GetVersion
 * Description  : Returns the version of this module.
 * Arguments    : none
 * Return Value : Version number (upper 8bits: Major version, lower 8bits: Minor version)
 *********************************************************************************************************************/
uint16_t R_CAN_GetVersion(void)
{
    return ((CAN_VERSION_MAJOR << CAN_POS_MAJOR_VER) | CAN_VERSION_MINOR);
}
/**********************************************************************************************************************
 End of function R_CAN_GetVersion
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: r_can_glb_rxfifo_isr
 * Description  : This function is executed when the INTRCANGRFR interrupt is received.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
#if (CAN_CFG_INTRCANGRFR_USE != 0)
R_BSP_PRAGMA_STATIC_INTERRUPT (r_can_glb_rxfifo_isr, CAN_INTRCANGRFR_VECT)
R_BSP_ATTRIB_STATIC_INTERRUPT void r_can_glb_rxfifo_isr(void)
{
    uint16_t  i_sources;
    uint16_t  rfists_value;

    i_sources = CAN_SOURCE_NOTHING;

    /* Read RX FIFO interrupt flag status */
    rfists_value = RFISTS & CAN_U16_USER_IE_RFISTS;

    /* Set interrupt sources and clear interrupt flags */
    while (0u != rfists_value)
    {
        /* Clear FIFO interrupt flag of RX FIFOs */
        if ((rfists_value & CAN_SOURCE_RECV_RF0) != 0u)
        {
            RFSTS0L = CAN_U08_RFSTSL_CLR_RFIF;
        }

        if ((rfists_value & CAN_SOURCE_RECV_RF1) != 0u)
        {
            RFSTS1L = CAN_U08_RFSTSL_CLR_RFIF;
        }

        /* Set interrupt source (Receive RX FIFO0, FIFO1) */
        i_sources |= rfists_value;

        /* Read RX FIFO interrupt flag status again */
        rfists_value = RFISTS & CAN_U16_USER_IE_RFISTS;
    }

    /* Call the user callback function */
    CAN_CFG_CALLBACK_GLB_RXFIFO(i_sources);
}
#endif /* end of (CAN_CFG_INTRCANGRFR_USE != 0) */
/**********************************************************************************************************************
 End of function r_can_glb_rxfifo_isr
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: r_can_glb_rxmb_isr
 * Description  : This function is executed when the INTRCANGRVC interrupt is received.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
#if (CAN_CFG_INTRCANGRVC_USE != 0)
R_BSP_PRAGMA_STATIC_INTERRUPT (r_can_glb_rxmb_isr, CAN_INTRCANGRVC_VECT)
R_BSP_ATTRIB_STATIC_INTERRUPT void r_can_glb_rxmb_isr(void)
{
    uint16_t  i_sources;
    uint16_t  rmnd_value;

    i_sources = CAN_SOURCE_NOTHING;

    /* Read RX buffer new data register */
    rmnd_value = RMND & CAN_U16_USER_IE_RMND;

    /* Set interrupt sources and clear interrupt flags */
    while (0u != rmnd_value)
    {
        /* Set interrupt source (Receive RX buffers) */
        i_sources |= rmnd_value;

        /* Clear new data flags which generate interrupt */
        RMND = ~rmnd_value;

        /* Read RX buffer new data register again */
        rmnd_value = RMND & CAN_U16_USER_IE_RMND;
    }

    /* Call the user callback function */
    CAN_CFG_CALLBACK_GLB_RXMB(i_sources);
}
#endif /* end of (CAN_CFG_INTRCANGRVC_USE != 0) */
/**********************************************************************************************************************
 End of function r_can_glb_rxmb_isr
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: r_can_glb_error_isr
 * Description  : This function is executed when the INTRCANGERR interrupt is received.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
#if (CAN_CFG_INTRCANGERR_USE != 0)
R_BSP_PRAGMA_STATIC_INTERRUPT (r_can_glb_error_isr, CAN_INTRCANGERR_VECT)
R_BSP_ATTRIB_STATIC_INTERRUPT void r_can_glb_error_isr(void)
{
    uint16_t  i_sources;
    uint16_t  gerfl_l_value;

    i_sources = CAN_SOURCE_NOTHING;

    /* Read Global error flags */
    gerfl_l_value = GERFLL & CAN_U16_USER_IE_GERFLL;

    /* Clear interrupt flags */
    while (0u != gerfl_l_value)
    {
        /* Clear Global error flags (DEF, CMPOF) */
        GERFLL = (~gerfl_l_value) & (CAN_U16_BIT3_ON | CAN_U16_BIT0_ON);

        /* Set interrupt source */
        i_sources |= gerfl_l_value;

        /* Clear flags for Global error flag MES */
        if ((gerfl_l_value & CAN_SOURCE_GERR_MLT) != 0u)
        {
            uint16_t  fmsts_value;

            fmsts_value = FMSTS & CAN_U16_USER_IE_FMSTS;

            /* Change interrupt source format */
            fmsts_value = 
                ((fmsts_value & CAN_U16_BIT8_ON) |
                ((fmsts_value & (CAN_U16_BIT1_ON | CAN_U16_BIT0_ON)) << CAN_POS_SOURCE_MLT_RF));

            /* Clear FIFO message lost */
            if ((fmsts_value & CAN_SOURCE_GERR_MLT_RF0) != 0u)
            {
                RFSTS0L = CAN_U08_RFSTSL_CLR_RFMLT;
            }

            if ((fmsts_value & CAN_SOURCE_GERR_MLT_RF1) != 0u)
            {
                RFSTS1L = CAN_U08_RFSTSL_CLR_RFMLT;
            }

            if ((fmsts_value & CAN_SOURCE_GERR_MLT_CF) != 0u)
            {
                CFSTSL  = CAN_U08_CFSTSL_CLR_CFMLT;
            }

            /* Set interrupt source */
            i_sources |= fmsts_value;
        }

        /* Clear flags for Global error flag THLES */
        if ((gerfl_l_value & CAN_SOURCE_GERR_TXH_LOST) != 0u)
        {
            THLSTSL = CAN_U08_THLSTSL_CLR_THLELT;
        }

        /* Read Global error flags again */
        gerfl_l_value = GERFLL & CAN_U16_USER_IE_GERFLL;
    }

    /* Call the user callback function */
    CAN_CFG_CALLBACK_GLB_ERROR(i_sources);
}
#endif /* end of (CAN_CFG_INTRCANGERR_USE != 0) */
/**********************************************************************************************************************
 End of function r_can_glb_error_isr
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: r_can_ch0_transmit_isr
 * Description  : This function is executed when the INTRCAN0TRM interrupt is received.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
#if (CAN_CFG_INTRCAN0TRM_USE != 0)
R_BSP_PRAGMA_STATIC_INTERRUPT (r_can_ch0_transmit_isr, CAN_INTRCAN0TRM_VECT)
R_BSP_ATTRIB_STATIC_INTERRUPT void r_can_ch0_transmit_isr(void)
{
    uint16_t  i_sources;
    uint16_t  gtintsts_value;

    i_sources = CAN_SOURCE_NOTHING;

    /* Read TX interrupt register */
    gtintsts_value = GTINTSTS & CAN_U16_USER_IE_GTINTSTS;

    /* Set interrupt sources and clear interrupt flags */
    while (0u != gtintsts_value)
    {
        uint16_t  sts_value;

        sts_value = 0u;

        if ((gtintsts_value & CAN_U16_GINTSTS_TSIF_ON) != 0u)
        {
            /* Get TX completion status */
            sts_value |= (TMTCSTS & CAN_U16_USER_IE_TMTCSTS);
        }

        if ((gtintsts_value & CAN_U16_GINTSTS_TAIF_ON) != 0u)
        {
            /* Get TX abort status (and shift abort status by 4bit) */
            sts_value |= ((TMTASTS & CAN_U16_TMTASTS_MASK) << CAN_POS_SOURCE_TRMA);
        }

        /* Clear TX buffer status if TX completion or abort  */
        if ((sts_value & CAN_SOURCE_TRM_TM0) != 0u)
        {
            TMSTS0 = CAN_U08_CLR;
        }

        if ((sts_value & CAN_SOURCE_TRM_TM1) != 0u)
        {
            TMSTS1 = CAN_U08_CLR;
        }

        if ((sts_value & CAN_SOURCE_TRM_TM2) != 0u)
        {
            TMSTS2 = CAN_U08_CLR;
        }

        if ((sts_value & CAN_SOURCE_TRM_TM3) != 0u)
        {
            TMSTS3 = CAN_U08_CLR;
        }

        if ((gtintsts_value & CAN_U16_GINTSTS_CFTIF_ON) != 0u)
        {
            /* Clear interrupt flag of CFIFO */
            CFSTSL = CAN_U08_CFSTSL_CLR_TXIF;

            /* Set interrupt source CFIFO */
            sts_value |= CAN_SOURCE_TRM_CF;
        }

        if ((gtintsts_value & CAN_U16_GINTSTS_THIF_ON) != 0u)
        {
            /* Clear interrupt flag of TX history list */
            THLSTSL = CAN_U08_THLSTSL_CLR_THLIF;

            /* Set interrupt source TX history list */
            sts_value |= CAN_SOURCE_TRM_TXHIST;
        }

        /* Set interrupt source (TX buf, CFIFO, TX history) */
        i_sources |= sts_value;

        /* Read TX interrupt register again */
        gtintsts_value = GTINTSTS & CAN_U16_USER_IE_GTINTSTS;
    }

    /* Call the user callback function */
    CAN_CFG_CALLBACK_CH0_TRANSMIT(i_sources);
}
#endif /* end of (CAN_CFG_INTRCAN0TRM_USE != 0) */
/**********************************************************************************************************************
 End of function r_can_ch0_transmit_isr
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: r_can_ch0_cfifo_rx_isr
 * Description  : This function is executed when the INTRCAN0CFR interrupt is received.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
#if (CAN_CFG_INTRCAN0CFR_USE != 0)
R_BSP_PRAGMA_STATIC_INTERRUPT (r_can_ch0_cfifo_rx_isr, CAN_INTRCAN0CFR_VECT)
R_BSP_ATTRIB_STATIC_INTERRUPT void r_can_ch0_cfifo_rx_isr(void)
{
    uint16_t  i_sources;

    i_sources = CAN_SOURCE_NOTHING;

    /* Clear interrupt flags */
    if ((CFSTS & CAN_U16_CFSTS_CFRXIF_ON) != 0u)
    {
        /* Clear RX FIFO interrupt flag of Common FIFO */
        CFSTSL = CAN_U08_CFSTSL_CLR_RXIF;

        /* Set interrupt source (Receive Common FIFO) */
        i_sources = CAN_SOURCE_RECV_CF;
    }

    /* Call the user callback function */
    CAN_CFG_CALLBACK_CH0_CFIFO_RX(i_sources);
}
#endif /* end of (CAN_CFG_INTRCAN0CFR_USE != 0) */
/**********************************************************************************************************************
 End of function r_can_ch0_cfifo_rx_isr
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: r_can_ch0_error_isr
 * Description  : This function is executed when the INTRCAN0ERR interrupt is received.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
#if (CAN_CFG_INTRCAN0ERR_USE != 0)
R_BSP_PRAGMA_STATIC_INTERRUPT (r_can_ch0_error_isr, CAN_INTRCAN0ERR_VECT)
R_BSP_ATTRIB_STATIC_INTERRUPT void r_can_ch0_error_isr(void)
{
    uint8_t  erfl_value;
    uint8_t  fdsts_value;
    uint16_t i_sources;

    i_sources = CAN_SOURCE_NOTHING;

    /* Read channel 0 error flag register (LL byte) */
    erfl_value = C0ERFLLL & CAN_U08_USER_IE_C0ERFLL;

    /* Read TDC violation flag                             */
    /* (Even if TDC is not used, the flag is read always.) */
    fdsts_value = C0FDSTSLH & CAN_U08_USER_IE_C0FDSTSLH;

    /* Set interrupt sources and clear interrupt flags */
    while (0u != (erfl_value | fdsts_value))
    {
        /* Clear channel 0 error flag */
        C0ERFLLL = ~erfl_value;

        /* Set interrupt source (CAN0 channel error) */
        i_sources |= erfl_value;

        /* Read channel 0 error flag register (LL byte) again */
        erfl_value = C0ERFLL & CAN_U08_USER_IE_C0ERFLL;

        if (0u != fdsts_value)  /* TDCVF=1? */
        {
            /* Clear TDC violation flag */
            C0FDSTSLH = CAN_U08_C0FDSTSLH_CLR_TDCVF;

            /* Set interrupt source */
            i_sources |= CAN_SOURCE_CH_ERR_TDCV;
        }

        /* Read TDC violation flag again */
        fdsts_value = C0FDSTSLH & CAN_U08_USER_IE_C0FDSTSLH;
    }

    /* Call the user callback function */
    CAN_CFG_CALLBACK_CH0_ERROR(i_sources);
}
#endif /* end of (CAN_CFG_INTRCAN0ERR_USE != 0) */
/**********************************************************************************************************************
 End of function r_can_ch0_error_isr
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Name: r_can_ch0_wakeup_isr
 * Description  : This function is executed when the INTRCAN0WUP interrupt is received.
 * Arguments    : none
 * Return Value : none
 *********************************************************************************************************************/
#if (CAN_CFG_INTRCAN0WUP_USE != 0)
R_BSP_PRAGMA_STATIC_INTERRUPT (r_can_ch0_wakeup_isr, CAN_INTRCAN0WUP_VECT)
R_BSP_ATTRIB_STATIC_INTERRUPT void r_can_ch0_wakeup_isr(void)
{
    RCAN0WUPMK = 1u;

    /* Call the user callback function */
    CAN_CFG_CALLBACK_CH0_WAKEUP(CAN_SOURCE_WAKEUP);
}
#endif
/**********************************************************************************************************************
 End of function r_can_ch0_wakeup_isr
 *********************************************************************************************************************/
