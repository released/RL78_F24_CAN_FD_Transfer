#ifndef TRANSFER_TEST_PLATFORM_H
#define TRANSFER_TEST_PLATFORM_H
#include <stdint.h>
#define __far
#define __near
#define BSP_NOP() ((void)0)
#define DI() ((void)0)
#define EI() ((void)0)
#define CAN_NO_BIT_ACCESS

typedef int e_can_err_t;
#define CAN_SUCCESS 0
#define CAN_ERR_WAITING 1
#define CAN_ERR_ILLEGAL_STS 4
#define CAN_U08_GAFLECTRH_AFLDAE_ON 1U
#define CAN_U16_CFDGRWC_PAGE0 0U
#define CAN_U16_CFDGRWC_PAGE1 1U
#define CAN_U16_CFDGRWC_KEY 0xA500U
#define CAN_U16_GAFLM0H_GAFLIDEM_ON 0x8000U
#define CAN_U16_GAFLM0H_GAFLRTRM_ON 0x4000U
#define CAN_STORE_RF0 1U
#define CAN_U08_GAFLP00LH_GAFLRMV_ON 0x80U
#define CAN_U16_RFSTS_RFEMP_ON 1U
#define CAN_U16_RFSTS_RFMLT_ON 4U
#define CAN_U08_RFSTSL_CLR_RFMLT 0xFBU
#define CAN_POS_RFPTRHH_RFDLC 4U
#define CAN_U08_RFPCTRL_FIFO_INC 0xFFU
#define CAN_POS_TMPTRHH_TMDLC 4U
#define CAN_U08_TMC_TMTR_ON 1U
#define CAN_U08_TMC_TMTAR_ON 2U
#define CAN_U08_TMSTS_TMTRF_MASK 6U
#define CAN_POS_TMSTS_TMTRF 1U

typedef struct {
    uint16_t IDL, IDH, ML, MH, P0LL, P0LH, P0H, P1L;
} st_can_rxrule_sfr_t;
typedef struct {
    uint16_t IDL, IDH, PTRL, DF[32];
    uint8_t PTRHH, FDCSTSLL, FDCSTSLH, FDCSTSH;
} st_can_msgbuf_sfr_t;
typedef struct { st_can_rxrule_sfr_t RxRule[16]; } test_rules_t;
typedef struct { st_can_msgbuf_sfr_t RxFIFOMsg[2], TxBufMsg[4]; } test_ram_t;
extern volatile test_rules_t CAN_RXRULE_SFR;
extern volatile test_ram_t CAN_RAM_MSG_SFR;
extern volatile uint16_t GAFLCFG, CFDGRWC, GSTS, C0STSL, C0ERFLL, RFSTS0;
extern volatile uint16_t RFCC0, RFPCTR0;
extern volatile uint8_t P1, GAFLECTRH, RFSTS0L, RFPCTR0L, TMC0, TMSTS0;
void R_CAN_Create(void);
e_can_err_t R_CAN_SetConfig(void);
e_can_err_t R_CAN_StartComm(void);
e_can_err_t R_CAN_StopComm(void);
uint8_t R_CAN_GetChStatus(void);
uint8_t R_CAN_GetChBusErrFlag(void);
typedef struct { st_can_msgbuf_sfr_t RxBufMsg[8]; } test_rxmb_ram_t;
extern volatile test_rxmb_ram_t CAN_RAM_RM_MSG_SFR;
extern volatile uint16_t RMNB, RMIEC;
extern volatile uint8_t RCANGRVCMK, RCANGRVCIF;
volatile uint16_t *Model_RmndRegister(void);
#define RMND (*Model_RmndRegister())
#define CAN_U16_CFDGRWC_PAGE2 2U
#define CAN_POS_RMPTRHH_RMDLC 4U

#ifdef F24_DEVICE_TRANSFER
/* Generated from the selected project's real vendor interface header. */
#define CAN_RX_HEAD_WORD_NUM 5U
#define CAN_MAX_WORD_NUM 32U
#define CAN_MAX_BYTE_NUM 64U
#include "vendor_rx_types.inc"
typedef uint8_t can_rxbuf_t;
typedef uint8_t can_length_t;
#define CAN_RXBUF0 0U
#define CAN_HALF_RXBUF_NUM 8U
#define CAN_ERR_INVALID_ARG 3
#define CAN_ERR_BUF_EMPTY 8
#define CAN_ERR_BUF_BUSY 9
#define CAN_ERR_OVERWRITE 10
#define CAN_CFG_PARAM_CHECKING_ENABLE 1
#define CAN_CFG_NRXMB 1
#define CAN_CFG_RMPLS 7
#define CAN_U16_CFDGRWC_PAGE3 3U
#define CAN_RX_HEAD_WORD_IDL 0U
#define CAN_RX_HEAD_WORD_IDH 1U
#define CAN_RX_HEAD_WORD_FDCTR 2U
#define CAN_MASK_W2 3U
#define CAN_MASK_W3 7U
#define CAN_CLASSICAL_MAX 8U
#define CAN_FDF_CLASSICAL 0U
#define CAN_RTR_CLASSICAL_RMT 1U
#define CAN_BYTE_TO_WORD(value) (((value) + 1U) / 2U)
#define CAN_SOURCE_NOTHING 0U
#define CAN_U16_USER_IE_RMND 1U
#define CAN_CFG_CALLBACK_GLB_RXMB my_can_glb_rxmb_callback
#define R_BSP_PRAGMA_STATIC_INTERRUPT(name, vector)
#define R_BSP_ATTRIB_STATIC_INTERRUPT static
e_can_err_t R_CAN_ReadRXMBInHandler(can_rxbuf_t buffer, st_can_rx_frame_t *frame,
                          can_length_t *length);
#endif
#endif
