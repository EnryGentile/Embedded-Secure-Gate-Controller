/**
******************************************************************************
* @file    installationReq_meter.c
* @author  SLAB NOIDA
* @version V3.3.0
* @date    15-Feb-2021
* @brief   WMBus Example of send installation request from Meter Devices. 
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


/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "wmbus_phy.h"
#include "wmbus_link.h"
#include "wmbus_linkcommand.h"
#include "wmbus_tim.h"
#include "user_config.h"
#include "uart.h"
#include "radio_hal.h"

/**
* @addtogroup Application Application
* @{
*/


/**
* @addtogroup WMBus_Examples WMBus Examples
* @{
*/


/**
* @addtogroup Installation_Request        Installation Request
* @{
*/

/**
* @addtogroup installationReq_meter                 Installation Request Meter
* @{
*/
uint8_t InstallReqCompleteFlag = 1;

/**
* @defgroup installationReq_meter_Private_Functions         Installation Request Meter Private Functions
* @{
*/


/**
* @brief  This function is used to capture external inputs.
* @param  None.
* @retval None.
*/
void SndIrMeterApplication(void)
{
  static uint32_t SndIrReqCtr=0;
   
  if(SW1_PUSHED)
  {
    SW1_PUSHED = 0;   
        
    WMBus_LinkSendInstallationRequest(NULL, 0, 0x78); 
    
    PRINTF("transmission %ld -- Sending SND_IR\n\r",SndIrReqCtr++);
  }   
  
}


/**
* @brief  This is the main application routine.
* @param  None.
* @retval None.
*/
int main (void)
{ 
/* MCU Configuration--------------------------------------------------------*/
  /* Reset of all peripherals, Initializes the Flash interface and the Systick*/
  HAL_Init();
  
  /* Configure the system clock */
  SystemClock_Config();
    
  //HAL_EnableDBGStopMode();

  /* Initialize LEDs*/
  BSP_LED_Init(LED2);
  
  /* Initialize HAL for S2-LP*/
  Radio_HAL_Init();

  /* Initialize UART*/
  uart_init(UART_BAUD_RATE);
  
  uint8_t a_field[6]={0x78,0x56,0x34,0x12,0x33,0x03};
  uint8_t m_field[2]={0x93,0x15};
  
  /* Initialization of the link Layer first */
  WMBus_LinkInit_Test_Mode(m_field, a_field, METER, WMBUS_FRAME_FORMAT);
   
  WMBus_PhyInit_TM(DEVICE_WMBUS_MODE, METER, RXPER);
  
  /* main loop. 
  Here the application should never block the cpu 
  and the link layer state machine as well. */
  while(1)
  {
    
    /* Link layer state machine : analyze PER packet */
    WMBus_LinkLayer_SM();

  }
}

/**
* @brief  This callback is called when Test mode frame is received at meter
* @param  frame:Pointer to the Frame.
* @retval Sucess/Error
*/
void WMBus_LinkRxPERTestModeFrameReceived_CB(Frame_t *frame)
{
  PRINTF("Test Mode Frame received.\n\r");
  

}  


/**
* @brief  This callback is called when Confirm Installation is received at meter
* @param  frame:Pointer to the Frame.
* @retval Sucess/Error
*/
void WMBus_LinkConfirmInstallReqReceived_CB(Frame_t *frame)
{
  InstallReqCompleteFlag = 1;
  PRINTF("Installation Request accepted.\n\r");
}

/**
* @brief  This callback is called when Confirm Installation 
*         is not rceived at meter before TimoOut.
* @param  None.
* @retval Sucess/Error.
*/
void WMBus_LinkCnfIRTimeout_CB(void)
{
  InstallReqCompleteFlag = 1;
  PRINTF("Installation Request failed.\n\r");
}

/**
* @brief  This callback is called when SND-NKE is Received at meter side
* @param  frame:Pointer to the Frame.
* @retval Sucess/Error
*/
void WMBus_LinkSendNkeReceived_CB(Frame_t *frame)
{  
  PRINTF("Link RESET received.\n\r");
}

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


/**
*@}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
