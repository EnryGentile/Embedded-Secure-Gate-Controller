/**
******************************************************************************
* @file    wmbus_tim.c
* @author  SRA-NOIDA 
* @version V2.4.3
* @date    27-Apr-2021
* @brief   This file contains routine for Timer. It is an implementation of
*          the timer interface specified by the header file wmbus_tim.h
*          of wM-BUS_Library_2013
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

#include "wmbus_tim.h"
#include "wmbus_phy.h"


#ifdef USE_S2_LP_DEFAULT
#include "S2LP_Timer.h"
#endif

#ifdef USE_STM32F4XX_NUCLEO
#include "stm32f4xx_hal_tim.h"
#endif

#ifdef USE_STM32L1XX_NUCLEO
#include "stm32l1xx_hal_tim.h"
#endif

#ifdef USE_STM32L0XX_NUCLEO
#include "stm32l0xx_hal_tim.h"
#endif

/**
* @addtogroup Application Application
* @{
*/

/**
* @addtogroup wmbus_user_application    WMBus User Application
* @{
*/

/**
* @addtogroup wmbus_tim    WMBus Timer Implementation
* @brief An implementation of the timer interface.
* @details See the file <i>WMBUS_Library_2013/wmbus_tim.h</i> for more details.
* @{
*/


/**
* @defgroup wmbus_tim_Private_Variables      WMBus Timer Private Variables
* @{
*/
uint32_t RxTimeOutRemaingCtr = 0;
static uint32_t NOverflows=0;

/* TIM handle declaration */
TIM_HandleTypeDef    TimHandle;

/**
*@}
*/


/**
* @defgroup wmbus_tim_Private_Defines        WMBus Timer Private Defines
* @{
*/

#define PROTOCOL_TIMER                          TIM2
#define PROTOCOL_TIMER_ISR                      TIM2_IRQHandler
#define PROTOCOL_TIMER_IRQN                     TIM2_IRQn
#define PROTOCOL_TIMER_CLOCK_ENABLE()           __HAL_RCC_TIM2_CLK_ENABLE()

#define NOW()          (NOverflows*0x10000 + __HAL_TIM_GET_COUNTER(&TimHandle))

#define S2LP_MAX_RX_TIMEOUT          3000000   //In us
/**
*@}
*/


/**
* @defgroup wmbus_tim_Exported_Functions         WMBus Timer Exported Functions
* @{
*/

/**
* @brief  Timer Interrupt Handler.
* @param  None
* @retval None
*/
void PROTOCOL_TIMER_ISR(void)   
{

  HAL_TIM_IRQHandler(&TimHandle);
}

/**
* @brief  Counts the Number of overflow.
* @param  None
* @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == PROTOCOL_TIMER)
  {
      NOverflows++;
  }
}

/**
* @brief  Initialize Timer.
* @param  None
* @retval None
*/
void WMBus_TimInit(void)  
{

  /* Set TIMx instance */
  TimHandle.Instance = PROTOCOL_TIMER;

  /* Initialize TIMx peripheral as follows:
       + Period = 0xFFFF - 1
       + Prescaler = (SystemCoreClock/3200) - 1
       + ClockDivision = 0
       + Counter direction = Up
  */
  TimHandle.Init.Period            = 0xFFFF;
  TimHandle.Init.Prescaler         = 3200-1;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;

  HAL_TIM_Base_DeInit(&TimHandle);
  
  if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK)
  {
    /* Initialization Error */
    /*Error_Handler();*/
  }
  
  __HAL_TIM_CLEAR_FLAG(&TimHandle, TIM_FLAG_UPDATE);
  
  /*##-2- Start the TIM Base generation in interrupt mode ####################*/
  /* Start Channel1 */
  if (HAL_TIM_Base_Start_IT(&TimHandle) != HAL_OK)
  {
    /* Starting Error */
    /*Error_Handler();*/
  }

}

/**
* @brief  Get Time Stamp.
* @param  None
* @retval Returns the current value of the Timer-Counter Register
*/
uint32_t WMBus_TimGetTimestamp(void)
{
  return (NOW());
}


/**
* @brief  Calculates the Time difference from a given time Stamp.
* @param  t0: Initial Time Stamp
* @retval Returns the Time Difference.
*/
uint32_t WMBus_TimDiffFromNow(uint32_t t0)
{
  volatile uint32_t now=NOW();
  
  if(now<t0)
  {
    /*
    case in which the current now is after the 32 bits max
             t0       2^32|0        now 
    ---------|------------|----------|---- 
    */
    return(0xffffffff-t0+now);
  }
  
  /* else return the simple difference */
  return(now-t0);
}


/**
* @brief  Inserts a time delay from a Initial Time Stamp
* @param  InitialTimeStamp: Initial Time Stamp 
* @param  ReqTimeDelay: The required Time Delay.
* @retval 0/1: Returns 1 When a delay is given else returns 0.
*/
uint8_t TimeDelay(uint32_t InitialTimeStamp,uint32_t ReqTimeDelay)
{
  volatile uint32_t timeDiff;
  int32_t tmpVar;
  timeDiff = WMBus_TimDiffFromNow(InitialTimeStamp);
  tmpVar = ReqTimeDelay - timeDiff;
  
  if(tmpVar > 0)
  {
    /*Return FALSE*/
    return 0;
  }
  else
  {
    /* Return TRUE*/
    return 1;
  }
}


/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* TIMx Peripheral clock enable */
  PROTOCOL_TIMER_CLOCK_ENABLE();
  
  /*##-2- Configure the NVIC for TIMx ########################################*/
  /* Set the TIMx priority */
  HAL_NVIC_SetPriority(PROTOCOL_TIMER_IRQN, 2, 0);

  /* Enable the TIMx global Interrupt */
  HAL_NVIC_EnableIRQ(PROTOCOL_TIMER_IRQN);
}

/**
  * @brief Resets the timer
  *        This function RESET the counter of TIMER to ZERO
  * @param None
  * @retval None
  */
void WMBus_TimReset(void)
{
  NOverflows = 0;
__HAL_TIM_SET_COUNTER(&TimHandle, 0);
  
}

/**
  * @brief Resets the timer after n Overflow
  *        This function RESET the counter after n Overflow
  * @param None
  * @retval None
  */
void WMBus_TimResetNOverflow(uint8_t numOverflow)
{
  if(numOverflow >= NOverflows)
  {
    NOverflows = 0;
    __HAL_TIM_SET_COUNTER(&TimHandle, 0);
  }
}

#ifdef USE_S2_LP_DEFAULT

/**
  * @brief Sets the RX Timeout for S2LP in MicroSecond
  * @param TimeUs : Time in microSecond.
  * @retval None
  */
void WMBus_S2LPTimerRxTimeoutUs(uint32_t TimeUs)
{
  uint8_t tmp_Counter,tmp_Prescaler;
  RxTimeOutRemaingCtr = TimeUs;
  
  if(RxTimeOutRemaingCtr > S2LP_MAX_RX_TIMEOUT)
  {
    S2LPTimerComputeRxTimerRegValues(S2LP_MAX_RX_TIMEOUT , &tmp_Counter , &tmp_Prescaler);
    S2LPTimerSetRxTimerPrescaler(tmp_Prescaler);
    S2LPRefreshStatus();
    S2LPTimerSetRxTimerCounter(tmp_Counter);
    RxTimeOutRemaingCtr = TimeUs - S2LP_MAX_RX_TIMEOUT;
  }
  else
  {
    RxTimeOutRemaingCtr = 0;
    S2LPTimerComputeRxTimerRegValues(TimeUs , &tmp_Counter , &tmp_Prescaler);
    S2LPTimerSetRxTimerPrescaler(tmp_Prescaler);
    S2LPRefreshStatus();
    S2LPTimerSetRxTimerCounter(tmp_Counter);
  }   
}

#endif

/**
*@}
*/

/**
*@}
*/

/**
*@}
*/

/**
*@}
*/


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
