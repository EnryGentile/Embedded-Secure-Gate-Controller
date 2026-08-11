/**
* @file    wmbus_transport_utils.c
* @author  SRA - NOIDA / RF application group - AMG
* @version V2.5.1
* @date    14-Sept-2020
* @brief   This file contains some utility routines used by the application
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2020 STMicroelectronics</center></h2>
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

/**
* @addtogroup Application Application
* @{
*/

/**
* @addtogroup wmbus_user_application    WMBus User Application
* @{
*/

/**
* @addtogroup wmbus_bsp_utils    WMBus BSP Utils
* @{
*/

/* Includes ------------------------------------------------------------------*/
#include "wmbus_transport_utils.h"


/** @defgroup transport_utils_Private_Functions      WMBus Transport Utils Private Functions
* @{
*/

/**
* @brief  This is a lookup table that transforms a CI-field to the correspondent header length.
* @param  ci: CI-field.
* @retval uint32_t header length in bytes.
*/
uint32_t WMBus_GetHeaderLength(uint8_t ci)
{ 
  switch(ci)
  {
  case 0x5A:
  case 0x61:
  case 0x65:
  case 0x6A:
  case 0x6E:
  case 0x74:
  case 0x7A:  
  case 0x7B:  
  case 0x7D:
  case 0x7F:
  case 0x8A:  
    return 4;
    
  case 0x53:
  case 0x5B:
  case 0x60:
  case 0x64:
  case 0x6B:
  case 0x6C:
  case 0x6D:
  case 0x6F:
  case 0x72:
  case 0x73:
  case 0x75:
  case 0x7C:  
  case 0x7E:
  case 0x80:
  case 0x84:  
  case 0x85:
  case 0x8B:
    return 12;
  }
  
  return 0;

}

/**
* @brief  This function extracts the <i>config word</i> from the given frame (according to the CI field
*               that is used to understand the packet header length).
* @param  frame: Frame from where the config word should be extracted.
* @retval uint16_t config_word from the given frame.
*/
uint16_t WMBus_GetConfigWord(Frame_t *frame)
{
  uint16_t config_word=0;
  uint16_t i_start=2,k;
  
  if(frame->ci_field==0x7A)
  {
    i_start=2;
  }
  else if(frame->ci_field==0x78)
  {
    i_start=10;
  }
  else{
    return 0;
  }
  
  k=0;
  for(uint16_t i=i_start;i<i_start+2;i++)
  {
    config_word|=((uint16_t)(frame->data_field.payload[i]))<<(8*(1-k));
    k++;
  }

  return config_word;
}

/**
* @brief  This function extracts the <i>access number</i> from the given frame (according to the CI field
*               that is used to understand the packet header length).
* @param  frame: Frame from where the access number should be extracted.
* @retval uint16_t access number from the given frame.
*/
uint8_t WMBus_GetAccessNum(Frame_t *frame)
{
  uint16_t i_start=0;
  
  if(frame->ci_field==0x7A)
  {
    i_start=0;
  }
  else if(frame->ci_field==0x78)
  {
    i_start=8;
  }
  else{
    return 0;
  }
 
  return frame->data_field.payload[i_start];
}

/**
* @brief  This function sets the given <i>config word</i> inside a given payload.
*         CI filed should be also specified in order to understand where to put the config word inside the payload.
* @param  payload: payload in which to inser the config word.
* @param  ci_field: CI_field of the given WMBUS frame.
* @param  config_word: config word to be set.
* @retval None.
*/
void WMBus_SetConfigWord(uint8_t *payload , uint8_t ci_field, uint16_t config_word)
{
  uint16_t i_start=2,k;
  
  if(ci_field==0x7A)
  {
    i_start=2;
  }
  else if(ci_field==0x78)
  {
    i_start=10;
  }
 
  
  k=0;
  for(uint16_t i=i_start;i<i_start+2;i++)
  {
    payload[i]=(config_word>>(8*(1-k)))&0xFF;
    k++;
  }

}

/**
* @brief  This function sets the given <i>access number</i> inside a given payload.
*         CI filed should be also specified in order to understand where to put the access number inside the payload.
* @param  payload: payload in which to inser the config word.
* @param  ci_field: CI_field of the given WMBUS frame.
* @param  acc_num: config word to be set.
* @retval None.
*/
void WMBus_SetAccessNum(uint8_t *payload , uint8_t ci_field, uint8_t acc_num)
{

  uint16_t i_start=0;
  
  if(ci_field==0x7A)
  {
    i_start=0;
  }
  else if(ci_field==0x78)
  {
    i_start=8;
  }
  
 
  
  payload[i_start]=acc_num;

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
