/**
* @file    wmbus_tim.h
* @author  VMA division - AMS/System Lab - NOIDA
* @version V3.3.0
* @date    27-Apr-2021
* @brief   Interface for timer module.
* @details Timer should tick with a 100us resolution.
*           Timestamp should be token in order to perform the timings of the protocol.
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOURCE CODE IS PROTECTED BY A LICENSE.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*
* <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
*/
/* 
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WMBUS_TIM_H
#define __WMBUS_TIM_H


#ifdef USE_STM32F4XX_NUCLEO
#include "stm32f4xx.h"
#include "stm32f4xx_hal_tim.h"

#endif

#ifdef USE_STM32L1XX_NUCLEO
#include "stm32l1xx.h"
#include "stm32l1xx_hal_tim.h"

#endif

#ifdef USE_STM32L0XX_NUCLEO
#include "stm32l0xx.h"
#include "stm32l0xx_hal_tim.h"

#endif

/**
* @defgroup wmbus_tim_Exported_Variables      WMBus Timer Exported Variables
* @{
*/
extern uint8_t  RxTimeOutCtrFlag;
extern uint32_t RxTimeOutRemaingCtr;

/**
*@}
*/


/**
* @brief  Initialize Timer.
* @param  None
* @retval None
*/
void WMBus_TimInit(void);


/**
* @brief  Get Time Stamp.
* @param  None
* @retval Returns the current value of the Timer-Counter Register
*/
uint32_t WMBus_TimGetTimestamp(void);

/**
* @brief  Calculates the Time difference from a given time Stamp.
* @param  t0: Initial Time Stamp in 100us units.
* @retval Returns the Time Difference.
*/
uint32_t WMBus_TimDiffFromNow(uint32_t t0);

/**
* @brief  Inserts a time delay from a Initial Time Stamp
* @param  InitialTimeStamp: Initial Time Stamp in 100us units.
* @param  ReqTimeDelay: The required Time Delay in 100us units.
* @retval 0/1: Returns 1 When a delay is given else returns 0.
*/
uint8_t TimeDelay(uint32_t InitialTimeStamp,uint32_t ReqTimeDelay);

/**
  * @brief Resets the timer
  *        This function RESET the counter of TIMER to ZERO
  * @param None
  * @retval None
  */
void WMBus_TimReset(void);

/**
  * @brief Resets the timer after n Overflow
  *        This function RESET the counter after n Overflow
  * @param None
  * @retval None
  */
void WMBus_TimResetNOverflow(uint8_t numOverflow);

/**
  * @brief Sets the Rx timeout in Microsecond
  * @param TimeUs : Timeout in microsecond
  * @retval None
  */
void WMBus_S2LPTimerRxTimeoutUs(uint32_t TimeUs);
extern void S2LPTimerComputeRxTimerRegValues(uint32_t plDesiredUsec , uint8_t* pcCounter , uint8_t* pcPrescaler);
/**
*@}
*/

#endif  /* __WMBUS_TIM_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
