/**
******************************************************************************
* @file    connctionApplyingTPL_conc.c
* @author  System Lab - NOIDA / RF application group - AMG
 * @version V3.3.0
 * @date    27-Apr-2021
* @brief   WMBus Example for Connection applying transport layer (short/long) 
*               for other devices
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
#include "wmbus_cbc.h"
#include "user_config.h"
#include "wmbus_transport_utils.h"
#include "wmbus_datarecord_utils.h"
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
* @addtogroup Connection_Applying_Transport_Layer        Connection Applying Transport Layer
* @{
*/

/**
* @addtogroup connectionApplyingTPL_conc                 Connection Applying Transport Layer Concentrator
* @{
*/




/**
* @defgroup connectionApplyingTPL_conc_Private_Defines            Connection Applying Transport Layer Concentrator Private Defines
* @{
*/



/**
*@}
*/




/**
* @defgroup connectionApplyingTPL_conc_Private_Variables      Connection Applying Transport Layer Concentrator Private Variables
* @{
*/



uint8_t encryption_key[16]={0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
                        0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x11};

uint8_t ConnTPLCompleteFlag = 1;
/**
*@}
*/




/**
* @defgroup connectionApplyingTPL_conc_Private_Functions         Connection Applying Transport Layer Concentrator Private Functions
* @{
*/
  

/**
* @brief  This function is Called from main loop.
* @param  None.
* @retval None.
*/
void OtherApplication(void)
{
  
  static uint32_t count=0;
  
  /* For example toggle a led every time the count reaches 0x5FFFF mult */
  if(count++%0x5FFFF == 0)
  {
    /* according to the application, do something here */
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
    if(ConnTPLCompleteFlag)
    {
      ConnTPLCompleteFlag = 0;
      SystemPower_Config();  
      Enter_LP_mode();
    }
#endif
    
    /*Application buisness */
    OtherApplication();
    
    /* Link layer state machine */
    WMBus_LinkLayer_SM(); 
  }
}




/**
* @brief  This callback is called when SND-NR is received at other device
* @param  frame: Pointer to the frame received.
* @retval Success/Error
*/
void WMBus_LinkSendNoReplyReceived_CB(Frame_t *frame)
{  
  
  static uint8_t payload[4]={0};
  
  WMBus_SetConfigWord(payload,0x7A,0);
  WMBus_SetAccessNum(payload,0x7A,1);
  
  WMBus_LinkSetFcv(1);
  WMBus_LinkSetFcb(1);
  
  /* send a SND_UD with CI=short tl*/
  WMBus_LinkSendUserDataCommand(payload,4,0x7A);
  
  
  
}

/**
* @brief  This callback is called when ACK is Received (at meter / other device)
* @param  frame: Pointer to the Frame.
* @retval Success/Error
*/
void WMBus_LinkAckReceived_CB(Frame_t *frame)
{  
  static uint8_t payload[4]={0};
      
  WMBus_SetConfigWord(payload,0x7A,0);
  WMBus_SetAccessNum(payload,0x7A, 1);
  
  WMBus_LinkSetFcv(1);
  WMBus_LinkSetFcb(1);
  
  WMBus_LinkRequestUserData2(payload, 4, 0x7A);
  
  
  
}


/**
* @brief  This callback is called when ACK is not received at Meter or 
*          Other devices
* @param  None.
* @retval Success/Error
*/
void WMBus_LinkAckTimeout_CB(void)
{

}

/**
* @brief  This callback is called when RSP-UD is received at the other device 
* @param  frame:Pointer to the Frame.
* @retval Success/Error
*/
void WMBus_LinkResponseUdReceived_CB(Frame_t *frame)
{
  static uint8_t num_data_asked=0;
  static uint8_t payload[4]={0};
    
  WMBus_SetConfigWord(payload,0x7A,0);
  WMBus_SetAccessNum(payload,0x7A, WMBus_GetAccessNum(frame));
  
  if(num_data_asked<2)/* ask data 2 times */
  {
    num_data_asked++;
    if(LINK_GET_ACD_BIT(frame->c_field))
    {
      /*If ACD is 1 we should ask for class 1 data*/
      WMBus_LinkRequestUserData1(payload, 4, 0x7A);  
    }
    else
    {
      /*If ACD is 1 we ask for class 2 data*/
      WMBus_LinkRequestUserData2(payload, 4, 0x7A);
    }
  }
  else
  {
    WMBus_SetConfigWord(payload,0x7A,0);
    WMBus_SetAccessNum(payload,0x7A, WMBus_GetAccessNum(frame));
  
    num_data_asked=0;
    WMBus_LinkSendResetRemoteLink(NULL,0,0x7A);
  }
  

}


/**
* @brief  This callback is called when RSP_UD is not received at other device
* @param  None.
* @retval Success/Error
*/
void WMBus_LinkRspUdTimeout_CB(void)
{

}


/**
* @brief  This callback is called when a transmission is completed.
*               In this case we use this callback to decrypt and print data
*               received in this transaction.
* @param  None.
* @retval Success/Error
*/
void WMBus_LinkTxCompleted_CB(Tx_Outcome_t tx_outcome, Frame_t *frame)
{
  uint8_t dec_payload[50]; 
  uint16_t dec_size;
  char msg[100];
  
  Frame_t *last_rx_frame=WMBus_LinkGetLastRxFrame();
  
  switch(last_rx_frame->c_field & 0x4F)
  {
  case LL_SND_NR:
    PRINTF("SND_NR received , SND_UD sent\n\r");
    break;
  case LL_ACK:
    PRINTF("ACK received , REQ-UD2 sent\n\r");
    break;
  case LL_RSP_UD:
    {
      PRINTF("RSP-UD received (ACD bit %d): ",LINK_GET_ACD_BIT(last_rx_frame->c_field));
      if(IS_CBC_5_CONFIG_WORD(WMBus_GetConfigWord(last_rx_frame)))
      {
        
        uint8_t iv[16];
        
        /* set the init vector of CBC5 */
        memcpy(iv,last_rx_frame->m_field,2);
        memcpy(&iv[2],last_rx_frame->a_field,6);
        /* acc num 8 times */
        memset(&iv[8],last_rx_frame->data_field.payload[0],8);
        
        memcpy(dec_payload,last_rx_frame->data_field.payload,4);
        
        /*Decrypt received data*/
        dec_size=4+WMBus_AES_Decrypt_CBC(&last_rx_frame->data_field.payload[4], last_rx_frame->data_field.size-4, encryption_key, &dec_payload[4], iv);
        
        
        PRINTF("Last recv frame decr RAW[");
        for(uint32_t i=0;i<dec_size;i++)
          PRINTF("%.2x,",dec_payload[i]);
        PRINTF("]\n\r");
        data_record_dec(&dec_payload[6], msg);
        PRINTF("DATA RECORD %s\n\r",msg);
        
      }
      
      switch(frame->c_field & 0x4F)
      {
      case LL_SND_NKE:
        {
          PRINTF("Sent SND-NKE\n\r");
          ConnTPLCompleteFlag = 1;
        }
        break;
      case LL_REQ_UD1:
        {
          PRINTF("Sent REQ-UD1\n\r");
        }
        break;
      case LL_REQ_UD2:
        {
          PRINTF("Sent REQ-UD2\n\r");
        }
        break;
      }
      break;
    }
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
