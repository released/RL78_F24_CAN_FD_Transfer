#ifndef F24_DEVICE_PREINCLUDE_H
#define F24_DEVICE_PREINCLUDE_H
/* Load this project's own generated SC settings before the board overrides. */
#include "r_bsp_config.h"
#include "r_rscanfd_rl78_config.h"
#if (BSP_CFG_FMX_HZ != 4000000) || (BSP_CFG_HISYSCLK_SOURCE != 1)
#error F24_device_requires_the_target_board_4MHz_X1_configuration
#endif
/* Do not enter generated, unbounded crystal/PLL waits before main. */
#undef BSP_CFG_MAINCLK_SOURCE
#define BSP_CFG_MAINCLK_SOURCE (0)
#undef BSP_CFG_HISYSCLK_OPERATION
#define BSP_CFG_HISYSCLK_OPERATION (1)
#undef BSP_CFG_PLL_OPERATION
#define BSP_CFG_PLL_OPERATION (0)
#undef BSP_CFG_PLL_MODE
#define BSP_CFG_PLL_MODE (0)
#undef BSP_CFG_FMP_DIVIDE
#define BSP_CFG_FMP_DIVIDE (0)
#undef BSP_CFG_PIOR46
#define BSP_CFG_PIOR46 (0)
#ifdef F24_DEVICE_APP
/* Boot hands over a locked 80MHz PLL / CPU40. CMC is write-once. */
#undef BSP_CFG_STARTUP_DISABLE
#define BSP_CFG_STARTUP_DISABLE (1)
#endif
/* fCAN=PLL80/2=40MHz: 40M / (4 * (1+15+4)) = 500 kbit/s. */
#undef CAN_CFG_NBRP
#define CAN_CFG_NBRP (3)
#undef CAN_CFG_NMNL_TSEG1
#define CAN_CFG_NMNL_TSEG1 (15)
#undef CAN_CFG_NMNL_TSEG2
#define CAN_CFG_NMNL_TSEG2 (4)
#undef CAN_CFG_NMNL_SJW
#define CAN_CFG_NMNL_SJW (1)
#undef CAN_CFG_CLOE_AND_FDOE
#define CAN_CFG_CLOE_AND_FDOE (1)
/* These derived switches were already evaluated by the generated header. */
#undef CAN_CFG_CLOE
#define CAN_CFG_CLOE (0)
#undef CAN_CFG_FDOE
#define CAN_CFG_FDOE (1)
/* fCAN40 / (2 * (1+7+2)) = 2 Mbit/s, sample point80%. */
#undef CAN_CFG_DBRP
#define CAN_CFG_DBRP (1)
#undef CAN_CFG_DATA_TSEG1
#define CAN_CFG_DATA_TSEG1 (7)
#undef CAN_CFG_DATA_TSEG2
#define CAN_CFG_DATA_TSEG2 (2)
#undef CAN_CFG_DATA_SJW
#define CAN_CFG_DATA_SJW (1)
/* RXMB0 holds complete FD data, with no receive FIFO copy. */
#undef CAN_CFG_RF0_RFPLS
#define CAN_CFG_RF0_RFPLS (7)
/* Protocol frames go to the RAM queue through the selected RXMB ISR. */
#undef CAN_CFG_RF0_RFDC
#define CAN_CFG_RF0_RFDC (0)
/* All FD profiles capture one 64-byte RXMB0; timing settings are shared. */
#undef CAN_CFG_RMPLS
#define CAN_CFG_RMPLS (7)
#undef CAN_CFG_NRXMB
#define CAN_CFG_NRXMB (1)
#undef CAN_CFG_RMIE_ALL
#define CAN_CFG_RMIE_ALL (2)
#undef CAN_CFG_RMIE_VALUE
#define CAN_CFG_RMIE_VALUE (1)
#undef CAN_CFG_RF0_RFDC
#define CAN_CFG_RF0_RFDC (0)
/* Transfer uses vendor vect= ISR; Boot/App map their no-vect ISR explicitly.
 * RMIEC is configured above independently of driver ISR generation. */
#undef CAN_CFG_INTRCANGRVC_USE
#ifdef F24_DEVICE_TRANSFER
#define CAN_CFG_INTRCANGRVC_USE (1)
#else
#define CAN_CFG_INTRCANGRVC_USE (0)
#endif
#undef CAN_CFG_INTRCANGERR_USE
#define CAN_CFG_INTRCANGERR_USE (0)
#undef CAN_CFG_INTRCAN0TRM_USE
#define CAN_CFG_INTRCAN0TRM_USE (0)
#undef CAN_CFG_INTRCAN0CFR_USE
#define CAN_CFG_INTRCAN0CFR_USE (0)
#undef CAN_CFG_INTRCAN0ERR_USE
#define CAN_CFG_INTRCAN0ERR_USE (0)
#endif
