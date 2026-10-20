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
 * File Name    : r_rscanfd_rl78_private_compiler.h
 * Description  : Internal compiler-dependent definitions for the CAN driver.
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

/**********************************************************************************************************************
 Macro definitions
 *********************************************************************************************************************/
#ifndef R_RSCANFD_RL78_PRIVATE_COMPILER_H
#define R_RSCANFD_RL78_PRIVATE_COMPILER_H

/* IAR keyword for SFR access */
#if defined(__CCRL__)
#define CAN_NO_BIT_ACCESS

#elif defined(__ICCRL78__)
#define CAN_NO_BIT_ACCESS               __no_bit_access

#else
#error Not support compliers other than CC-RL or IAR
#endif

/* CAN interrupt vector */
#if defined(__CCRL__)
#define CAN_INTRCANGRVC_VECT            INTRCANGRVC
#define CAN_INTRCANGRFR_VECT            INTRCANGRFR
#define CAN_INTRCANGERR_VECT            INTRCANGERR
#define CAN_INTRCAN0TRM_VECT            INTRCAN0TRM
#define CAN_INTRCAN0CFR_VECT            INTRCAN0CFR
#define CAN_INTRCAN0ERR_VECT            INTRCAN0ERR
#define CAN_INTRCAN0WUP_VECT            INTRCAN0WUP

#elif defined(__ICCRL78__)
#define CAN_INTRCANGRVC_VECT            (INTRCANGRVC_vect)
#define CAN_INTRCANGRFR_VECT            (INTRCANGRFR_vect)
#define CAN_INTRCANGERR_VECT            (INTRCANGERR_vect)
#define CAN_INTRCAN0TRM_VECT            (INTRCAN0TRM_vect)
#define CAN_INTRCAN0CFR_VECT            (INTRCAN0CFR_vect)
#define CAN_INTRCAN0ERR_VECT            (INTRCAN0ERR_vect)
#define CAN_INTRCAN0WUP_VECT            (INTRCAN0WUP_vect)

#else
#error Not support compliers other than CC-RL or IAR
#endif

/* Register mapping: RX FIFO registers */
#define CAN_RF_SFR      (*(volatile __near CAN_NO_BIT_ACCESS st_can_rf_sfr_t *)&RFCC0)

/* Register mapping: TX buffer registers */
#define CAN_TM_SFR      (*(volatile __near CAN_NO_BIT_ACCESS st_can_tm_sfr_t *)&TMC0)

/* Register mapping:  Message buffer registers (RX FIFO, Common FIFO, TX Buffer) */
#define CAN_RAM_MSG_SFR (*(volatile __near CAN_NO_BIT_ACCESS  st_can_ram_msg_sfr_t *)&RFID0L)

/* Register mapping:  Message buffer registers (RX Buffer) */
#define CAN_RAM_RM_MSG_SFR \
                        (*(volatile __near CAN_NO_BIT_ACCESS st_can_ram_rm_msg_sfr_t *)&RMID0L)

/* Register mapping: RX rule registers */
#define CAN_RXRULE_SFR  (*(volatile __near CAN_NO_BIT_ACCESS st_can_ram_rxrule_sfr_t *)&GAFLID0L)

/**********************************************************************************************************************
 Global Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 Exported global functions
 *********************************************************************************************************************/

#endif /* R_RSCANFD_RL78_PRIVATE_COMPILER_H */
