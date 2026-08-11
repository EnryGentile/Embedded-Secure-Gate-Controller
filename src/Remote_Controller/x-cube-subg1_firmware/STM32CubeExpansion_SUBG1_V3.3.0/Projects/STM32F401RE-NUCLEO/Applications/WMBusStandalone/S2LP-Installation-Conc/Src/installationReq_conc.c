/**
******************************************************************************
* @file    installationReq_conc.c
* @author  SLAB NOIDA
* @version V3.3.0
* @date    27-Apr-2021
* @brief   WMBus Example for installation request reception for other device.
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
#include "uart.h"
#include "user_config.h"
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
* @addtogroup installationReq_conc                 Installation Request Concentrator
* @{
*/




/**
* @defgroup installationReq_conc_Private_Variables      Installation Request Concentrator Private Variables
* @{
*/
static uint8_t refuseCount = 0;
static uint8_t maxRefuseCount=3;
uint8_t InstallReqCompleteFlag = 1;
/**
*@}
*/



/**
* @defgroup installationReq_conc_Private_Functions         Installation Request Concentrator Private Functions
* @{
*/

/**
* @brief  This function is called from main loop.
* @param  None.
* @retval None.
*/
void SndIrOtherApplication(void)
{
  static uint32_t count=0,cal_count=0;
  
  /* For example toggle a led every time the count reaches 0x5FFFF mult */
  if(count++%0x5FFFF == 0)
  {
    cal_count++;
  } 
  
  if(cal_count>9)
  {
    /*if(WMBus_LinkVcoCalibrationRequest())
    {
      WMBus_LinkLayer_SM();
      cal_count=0;
    }*/
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
    
  HAL_EnableDBGStopMode();

  /* Initialize LEDs*/
  BSP_LED_Init(LED2);
  
  /* Initialize HAL for S2-LP*/
  Radio_HAL_Init();
  
  /* Initialize wM-Bus Radio*/
  Radio_WMBus_Init();
  
  /* Initialize Buttons*/
  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  
  /* Initialize UART*/
  uart_init(UART_BAUD_RATE);
  
  /* Overhead Estimation*/
  WMBus_LinkOverHeadEst();
  
  /* ON the LED2*/
  BSP_LED_On(LED2);
  
  /* main loop. 
  Here the application should never block the cpu 
  and the link layer state machine as well. */
  while(1)
  {
#ifdef USE_LOW_POWER_MODE
    if(InstallReqCompleteFlag)
    {
      InstallReqCompleteFlag = 0;
      SystemPower_Config();  
      Enter_LP_mode();
    }
#endif
    /* Application buisness */
    SndIrOtherApplication();
    
    /* Link layer state machine */
    WMBus_LinkLayer_SM(); 
  }
}




/**
* @brief  This callback is called when SND-IR is received at the other device
* @param  frame: Pointer to the Frame.
* @retval Success (send CNF_IR) or Error .
*/
void WMBus_LinkInstallReqReceived_CB(Frame_t *frame)
{
  /*Determines how many times the other device refuses
    the installation request*/   
  if(refuseCount<maxRefuseCount)
  {
    refuseCount++;
    WMBus_LinkSendResetRemoteLink(NULL,0,0x78);
  }
  else
  {
    refuseCount = 0;
    WMBus_LinkSendConfirmInstallationRequest(NULL, 0, 0x78);
  }
  
  /* we don't print anything here because, since some submodes have tight 
      tRo, we should prepare the transmission frame without waste time with printf.
    printf are postposed to TxCompleted event . */
  
}

void WMBus_LinkTxCompleted_CB(Tx_Outcome_t tx_outcome, Frame_t *frame)
{
  /* Here we have all the time for printf */
  switch(frame->c_field & 0x4F)
  {
  case LL_SND_NKE:
    PRINTF("reception %d/%d - REFUSING CONNECTION\r\n",refuseCount,maxRefuseCount+1);
    break;
  
  case LL_CNF_IR:
    {
      Frame_t *rx_frame=WMBus_LinkGetLastRxFrame();
      PRINTF("reception %d/%d - ACCEPTING CONNECTION\r\n",maxRefuseCount+1,maxRefuseCount+1);
      PRINTF("frame received:\n\r C-field: 0x%.2x\n\r",rx_frame->c_field);
      PRINTF(" M-field: [0x%.2x, 0x%.2x]\n\r",rx_frame->m_field[0],rx_frame->m_field[1]);
      PRINTF(" A-field: [");
      for(uint32_t i=0;i<ADDR_ATTR_SIZE;i++)
        PRINTF("0x%.2x ",rx_frame->a_field[i]);
      PRINTF("]\n\rRSSI: %ld dBm\n\r",(int32_t)(rx_frame->RSSI_field));
      PRINTF("]\n\r");
      InstallReqCompleteFlag = 1;
    }
    break;
  }
}

/* patch  : case of gateway with unidirectional mode */
void WMBus_LinkRxCompleted_SNDIR_CB(void)
{
      if(refuseCount<maxRefuseCount)
      {
        refuseCount++;
        PRINTF("reception %d/%d - REFUSING CONNECTION\r\n",refuseCount,maxRefuseCount+1);
        }
      else
      {
  
      Frame_t *rx_frame=WMBus_LinkGetLastRxFrame();
      PRINTF("reception %d/%d - ACCEPTING CONNECTION\r\n",maxRefuseCount+1,maxRefuseCount+1);
      PRINTF("frame received:\n\r C-field: 0x%.2x\n\r",rx_frame->c_field);
      PRINTF(" M-field: [0x%.2x, 0x%.2x]\n\r",rx_frame->m_field[0],rx_frame->m_field[1]);
      PRINTF(" A-field: [");
      for(uint32_t i=0;i<ADDR_ATTR_SIZE;i++)
        PRINTF("0x%.2x ",rx_frame->a_field[i]);
      PRINTF("]\n\rRSSI: %ld dBm\n\r",(int32_t)(rx_frame->RSSI_field));
      PRINTF("]\n\r");
      InstallReqCompleteFlag = 1;
      refuseCount = 0;
      }
  
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
