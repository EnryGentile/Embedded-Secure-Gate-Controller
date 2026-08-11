/**
 ******************************************************************************
 * @file    main.c
 * @author  System lab Noida
 * @version V3.3.0
 * @date    27-Apr-2021
 * @brief   Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "cube_hal.h"
#include "wmbus_link.h"

#ifdef USE_SPIRIT1_DEFAULT
#include "radio_shield_config.h"
#include "radio_hal.h"
#endif

#ifdef USE_S2_LP_DEFAULT
#include "user_appli.h"
#include "radio_hal.h"
#include "user_config.h"
#include "uart.h"
#endif

/** @addtogroup USER
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
int main(void) 
{
  /* MCU Configuration----------------------------------------------------------*/
  
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  
  /* Configure the system clock */
  SystemClock_Config();
  
  HAL_EnableDBGStopMode();
  
#ifdef USE_SPIRIT1_DEFAULT
  /* Initialize LEDs*/
  RadioShieldLedInit(RADIO_SHIELD_LED);
#endif
  
  BSP_LED_Init(LED2);
  
  Radio_HAL_Init();
  
  Radio_WMBus_Init();
  
  /* Initialize Buttons*/
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  
#if defined(USE_LOW_POWER_MODE)
  SystemPower_Config();
  Enter_LP_mode();
#endif  
  
#ifdef USE_SPIRIT1_DEFAULT
  while (1)
  {
    Radio_WMBus_On();
    WMBus_LinkLayer_SM();
  }
#endif
  
#ifdef USE_S2_LP_DEFAULT
  /* Initialize UART*/
  uart_init(UART_BAUD_RATE);
  
  /* Overhead Estimation*/
  WMBus_LinkOverHeadEst();
  
  while (1) {
#ifdef DEVICE_TYPE_METER    
    /* Initiate the wM-Bus Packet Transmission*/
    Radio_WMBus_On();
#endif
    /* Runs the Link Layer State Machine of wM-Bus*/
    WMBus_LinkLayer_SM();
    
#ifdef ENABLE_VCOM
    /* Sends the wM-Bus Packets on Terminal*/
    SendDataToTerminal();
#endif 
    /* Runs the AT/Command set for wM-Bus Configuration*/
    app_command_mode();
  }
#endif
  
}


#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
	/* User can add his own implementation to report the file name and line number,
	 ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

}

#endif




/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
