/* Generated configuration header file - do not edit */
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
 * Copyright (C) 2021 Renesas Electronics Corporation. All rights reserved.
 *********************************************************************************************************************/
/**********************************************************************************************************************
 * File Name    : r_rscanfd_rl78_config.h
 * Description  : This is configuration file for CAN driver.
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 *********************************************************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#ifndef R_RSCANFD_RL78_CONFIG_H
#define R_RSCANFD_RL78_CONFIG_H

/**********************************************************************************************************************
 Configuration Options
 **********************************************************************************************************************/
/* SPECIFY WHETHER TO INCLUDE CODE FOR API PARAMETER CHECKING
 * Setting to BSP_CFG_PARAM_CHECKING_ENABLE utilizes the system default setting
 * Setting to 1 includes parameter checking; 0 compiles out parameter checking
 */
#define CAN_CFG_PARAM_CHECKING_ENABLE   (1)

/**********************************************************************************************************************
 CAN Register Configuration
 **********************************************************************************************************************/
/*
 * CRXD0 pin Configuration
 */
#define CAN_CFG_CRXD0_PU                (0)
#define CAN_CFG_CRXD0_PITHL             (0)

/*
 * Channel 0 Nominal Bitrate Configuration Register (C0NCFGH, C0NCFGL)
 */
#define CAN_CFG_NMNL_TSEG2              (4)
#define CAN_CFG_NMNL_TSEG1              (11)
#define CAN_CFG_NMNL_SJW                (5)
#if (CAN_CFG_NMNL_TSEG2 > 0)
    #define CAN_CFG_NTSEG2              (CAN_CFG_NMNL_TSEG2 - 1)    /* DO NOT EDIT */
#else
    #define CAN_CFG_NTSEG2              (0)                         /* DO NOT EDIT: Fix 0 */
#endif
#if (CAN_CFG_NMNL_TSEG1 > 0)
    #define CAN_CFG_NTSEG1              (CAN_CFG_NMNL_TSEG1 - 1)    /* DO NOT EDIT */
#else
    #define CAN_CFG_NTSEG1              (0)                         /* DO NOT EDIT: Fix 0 */
#endif
#if (CAN_CFG_NMNL_SJW > 0)
    #define CAN_CFG_NSJW                (CAN_CFG_NMNL_SJW - 1)      /* DO NOT EDIT */
#else
    #define CAN_CFG_NSJW                (0)                         /* DO NOT EDIT: Fix 0 */
#endif
#define CAN_CFG_NBRP                    (9)

/*
 * Channel 0 Control Register (C0CTRH, C0CTRL) 
 *   No configuration: ROM, BFT, CTMS, CTME, RTBO, CSLPR, CHMDC
 *   Not support: SOCOIE, EOCOIE
 */
#define CAN_CFG_ERRD                    (0)
#define CAN_CFG_BOM                     (0)
#define CAN_CFG_TDCVFIE                 (0)
#define CAN_CFG_TAIE                    (0)
#define CAN_CFG_ALIE                    (0)
#define CAN_CFG_BLIE                    (0)
#define CAN_CFG_OLIE                    (0)
#define CAN_CFG_BORIE                   (0)
#define CAN_CFG_BOEIE                   (0)
#define CAN_CFG_EPIE                    (0)
#define CAN_CFG_EWIE                    (0)
#define CAN_CFG_BEIE                    (0)

/*
 * Channel 0 Data Bitrate Configuration Register (C0DCFGH, C0DCFGL) 
 */
#define CAN_CFG_DATA_TSEG2              (2)
#define CAN_CFG_DATA_TSEG1              (7)
#define CAN_CFG_DATA_SJW                (1)
#if (CAN_CFG_DATA_TSEG2 > 0)
    #define CAN_CFG_DTSEG2              (CAN_CFG_DATA_TSEG2 - 1)    /* DO NOT EDIT */
#else
    #define CAN_CFG_DTSEG2              (0)                         /* DO NOT EDIT: Fix 0 */
#endif
#if (CAN_CFG_DATA_TSEG1 > 0)
    #define CAN_CFG_DTSEG1              (CAN_CFG_DATA_TSEG1 - 1)    /* DO NOT EDIT */
#else
    #define CAN_CFG_DTSEG1              (0)                         /* DO NOT EDIT: Fix 0 */
#endif
#if (CAN_CFG_DATA_SJW > 0)
    #define CAN_CFG_DSJW                (CAN_CFG_DATA_SJW - 1)      /* DO NOT EDIT */
#else
    #define CAN_CFG_DSJW                (0)                         /* DO NOT EDIT: Fix 0 */
#endif
#define CAN_CFG_DBRP                    (1)

/*
 * Channel 0 CAN-FD Configuration Register (C0FDCFGH, C0FDCFGL) 
 *   Not support: RPNMD, EOCCFG
 */
#define CAN_CFG_CLOE_AND_FDOE           (2)
#define CAN_CFG_TDCE_AND_TDCOC          (0)
#if (CAN_CFG_CLOE_AND_FDOE == 2)        /* Classical CAN only mode? */
    #define CAN_CFG_CLOE                (1)                         /* DO NOT EDIT: Fix 1 */
    #define CAN_CFG_FDOE                (0)                         /* DO NOT EDIT: Fix 0 */
#elif (CAN_CFG_CLOE_AND_FDOE == 1)      /* FD only mode? */
    #define CAN_CFG_CLOE                (0)                         /* DO NOT EDIT: Fix 0 */
    #define CAN_CFG_FDOE                (1)                         /* DO NOT EDIT: Fix 1 */
#else
    #define CAN_CFG_CLOE                (0)                         /* DO NOT EDIT: Fix 0 */
    #define CAN_CFG_FDOE                (0)                         /* DO NOT EDIT: Fix 0 */
#endif
#if (CAN_CFG_TDCE_AND_TDCOC == 2)       /* TDC enabled, Only offset? */
    #define CAN_CFG_TDCE                (1)                         /* DO NOT EDIT: Fix 1 */
    #define CAN_CFG_TDCOC               (1)                         /* DO NOT EDIT: Fix 1 */
#elif (CAN_CFG_TDCE_AND_TDCOC == 1)     /* TDC enabled, Measured + offset ? */
    #define CAN_CFG_TDCE                (1)                         /* DO NOT EDIT: Fix 1 */
    #define CAN_CFG_TDCOC               (0)                         /* DO NOT EDIT: Fix 0 */
#else
    #define CAN_CFG_TDCE                (0)                         /* DO NOT EDIT: Fix 0 */
    #define CAN_CFG_TDCOC               (0)                         /* DO NOT EDIT: Fix 0 */
#endif
#define CAN_CFG_REFE                    (0)
#define CAN_CFG_TDCO                    (0x00)
#define CAN_CFG_ESIC                    (0)

/*
 * Global Configuration Register (GCFGH, GCFGL) 
 */
#define CAN_CFG_ITRCP                   (0)
#define CAN_CFG_TSSS                    (0)
#define CAN_CFG_TSP                     (0)
#define CAN_CFG_CMPOC                   (0)
#define CAN_CFG_DCS                     (0)
#define CAN_CFG_MME                     (0)
#define CAN_CFG_DCE                     (0)
#define CAN_CFG_DRE                     (0)
#define CAN_CFG_TPRI                    (0)

/*
 * Global Control Register (GCTRH, GCTRL) 
 *   No configuration: TSRST, GSLPR, GMDC
 */
#define CAN_CFG_CMPOFIE                 (0)
#define CAN_CFG_THLEIE                  (0)
#define CAN_CFG_MEIE                    (0)
#define CAN_CFG_DEIE                    (0)

/*
 * RX Message Buffer Number Register (RMNB) 
 */
#define CAN_CFG_RMPLS                   (0)
#define CAN_CFG_NRXMB                   (8)

/*
 * RX FIFO Configuration / Control Register k (RFCCk)  [k = 0, 1] 
 *   No configuration: RFE
 *   CAN driver sets RFE=1 when RFDC configuration is not 0.
 */
/* k = 0: Condition for RX FIFO0 */
#define CAN_CFG_RF0_RFIGCV              (0)
#define CAN_CFG_RF0_RFIM                (0)
#define CAN_CFG_RF0_RFDC                (2)
#define CAN_CFG_RF0_RFPLS               (0)
#define CAN_CFG_RF0_RFIE                (0)

/* k = 1: Condition for RX FIFO1 */
#define CAN_CFG_RF1_RFIGCV              (0)
#define CAN_CFG_RF1_RFIM                (0)
#define CAN_CFG_RF1_RFDC                (0)
#define CAN_CFG_RF1_RFPLS               (0)
#define CAN_CFG_RF1_RFIE                (0)

/*
 * Common FIFO Configuration / Control Register (CFCC) 
 *   No configuration: CFE
 *   CAN driver sets CFE=1 when CFDC configuration is not 0.
 */
#define CAN_CFG_CFM                     (1)
#define CAN_CFG_CFITT                   (0)
#define CAN_CFG_CFDC                    (0)
#define CAN_CFG_CFTML                   (0)
#if (CAN_CFG_CFM == 0)
    #define CAN_CFG_CFIGCV              (CAN_CFG_CFIGCV_RX)         /* DO NOT EDIT */
    #define CAN_CFG_CFIM                (CAN_CFG_CFIM_RX)           /* DO NOT EDIT */
#else
    #define CAN_CFG_CFIGCV              (0)                         /* DO NOT EDIT: Fix 0 */
    #define CAN_CFG_CFIM                (CAN_CFG_CFIM_TX)           /* DO NOT EDIT */
#endif
#define CAN_CFG_CFITR                   (0)
#define CAN_CFG_CFITSS                  (0)
#define CAN_CFG_CFPLS                   (0)
#define CAN_CFG_CFTXIE                  (0)
#define CAN_CFG_CFRXIE                  (0)

#define CAN_CFG_CFIGCV_RX               (0)
#define CAN_CFG_CFIM_RX                 (0)
#define CAN_CFG_CFIM_TX                 (0)

/*
 * TX Message Buffer Interrupt Enable Configuration Register (TMIEC) 
 */
#define CAN_CFG_TMIE3                   (0)
#define CAN_CFG_TMIE2                   (0)
#define CAN_CFG_TMIE1                   (0)
#define CAN_CFG_TMIE0                   (0)

/*
 * TX History List Configuration / Control Register (THLCC) 
 */
#define CAN_CFG_THLDTE                  (0)
#define CAN_CFG_THLIM                   (0)
#define CAN_CFG_THLIE                   (0)
#define CAN_CFG_THLE                    (0)

/*
 * Global FD Configuration Register (GFDCFG) 
 */
#define CAN_CFG_TSCCFG                  (0)
#define CAN_CFG_RPED                    (0)

/*
 * RX Message Buffer Interrupt Enable Configuration Register (RMIEC) 
 */
#define CAN_CFG_RMIE_ALL                (1)
#define CAN_CFG_RMIE_VALUE              (0x0000)

/**********************************************************************************************************************
 CAN Interrupt Configuration
 **********************************************************************************************************************/
/*
 * CAN Global Receive FIFO (INTRCANGRFR)
 */
#define CAN_CFG_INTRCANGRFR_USE         (0)                             /* 0: Not Use, 1: Use */
#define CAN_CFG_INTRCANGRFR_LEVEL       (2)                             /* Priority           */
#define CAN_CFG_CALLBACK_GLB_RXFIFO     (my_can_glb_rxfifo_callback)    /* Function Symbol    */

/*
 * CAN Global Receive Message Buffer (INTRCANGRVC)
 */
#define CAN_CFG_INTRCANGRVC_USE         (1)                             /* 0: Not Use, 1: Use */
#define CAN_CFG_INTRCANGRVC_LEVEL       (2)                             /* Priority           */
#define CAN_CFG_CALLBACK_GLB_RXMB       (my_can_glb_rxmb_callback)      /* Function Symbol    */

/*
 * CAN Global Error (INTRCANGERR)
 */
#define CAN_CFG_INTRCANGERR_USE         (1)                             /* 0: Not Use, 1: Use */
#define CAN_CFG_INTRCANGERR_LEVEL       (2)                             /* Priority           */
#define CAN_CFG_CALLBACK_GLB_ERROR      (my_can_glb_error_callback)     /* Function Symbol    */

/*
 * CAN0 Channel Transmit (INTRCAN0TRM)
 */
#define CAN_CFG_INTRCAN0TRM_USE         (1)                             /* 0: Not Use, 1: Use */
#define CAN_CFG_INTRCAN0TRM_LEVEL       (2)                             /* Priority           */
#define CAN_CFG_CALLBACK_CH0_TRANSMIT   (my_can_ch0_transmit_callback)  /* Function Symbol    */

/*
 * CAN0 Common FIFO Receive (INTRCAN0CFR)
 */
#define CAN_CFG_INTRCAN0CFR_USE         (1)                             /* 0: Not Use, 1: Use */
#define CAN_CFG_INTRCAN0CFR_LEVEL       (2)                             /* Priority           */
#define CAN_CFG_CALLBACK_CH0_CFIFO_RX   (my_can_ch0_cfifo_rx_callback)  /* Function Symbol    */

/*
 * CAN0 Channel Error (INTRCAN0ERR)
 */
#define CAN_CFG_INTRCAN0ERR_USE         (1)                             /* 0: Not Use, 1: Use */
#define CAN_CFG_INTRCAN0ERR_LEVEL       (2)                             /* Priority           */
#define CAN_CFG_CALLBACK_CH0_ERROR      (my_can_ch0_error_callback)     /* Function Symbol    */

/*
 * CAN0 Wakeup (INTRCAN0WUP)
 */
#define CAN_CFG_INTRCAN0WUP_USE         (0)                             /* 0: Not Use, 1: Use */
#define CAN_CFG_INTRCAN0WUP_LEVEL       (2)                             /* Priority           */
#define CAN_CFG_CALLBACK_CH0_WAKEUP     (my_can_ch0_wakeup_callback)    /* Function Symbol    */

/**********************************************************************************************************************
 External function Prototypes
 *********************************************************************************************************************/
#if (CAN_CFG_INTRCANGRFR_USE != 0)
extern void CAN_CFG_CALLBACK_GLB_RXFIFO (uint16_t arg);
#endif
#if (CAN_CFG_INTRCANGRVC_USE != 0)
extern void CAN_CFG_CALLBACK_GLB_RXMB (uint16_t arg);
#endif
#if (CAN_CFG_INTRCANGERR_USE != 0)
extern void CAN_CFG_CALLBACK_GLB_ERROR (uint16_t arg);
#endif
#if (CAN_CFG_INTRCAN0TRM_USE != 0)
extern void CAN_CFG_CALLBACK_CH0_TRANSMIT (uint16_t arg);
#endif
#if (CAN_CFG_INTRCAN0CFR_USE != 0)
extern void CAN_CFG_CALLBACK_CH0_CFIFO_RX (uint16_t arg);
#endif
#if (CAN_CFG_INTRCAN0ERR_USE != 0)
extern void CAN_CFG_CALLBACK_CH0_ERROR (uint16_t arg);
#endif
#if (CAN_CFG_INTRCAN0WUP_USE != 0)
extern void CAN_CFG_CALLBACK_CH0_WAKEUP (uint16_t arg);
#endif

#endif /* R_RSCANFD_RL78_CONFIG_H */
