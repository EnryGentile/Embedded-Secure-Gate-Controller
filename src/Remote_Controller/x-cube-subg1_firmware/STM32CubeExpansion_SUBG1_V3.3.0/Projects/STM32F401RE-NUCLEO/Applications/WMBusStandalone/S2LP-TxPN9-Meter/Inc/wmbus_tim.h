/**
******************************************************************************
* @file    wmbus_tim.h
* @author  SRA-NOIDA 
* @version V2.4.3
* @date    27-Apr-2021
* @brief   Timer should tick with a 100us resolution.
*           Timestamp should be token in order to perform the timings of the protocol
*
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2021 STMicroelectronics</center></h2>
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*   1. Redistributions of source code must retain the above copyright notice,
*      this list of conditions and the following disclaimer.
*   2. Redistributions in binary form must reproduce the above copyright notice,
*      this list of conditions and the following disclaimer in the documentation
*      and/or other materials provided with the distribution.
*   3. Neither the name of STMicroelectronics nor the names of its contributors
*      may be used to endorse or promote products derived from this software
*      without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************
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
