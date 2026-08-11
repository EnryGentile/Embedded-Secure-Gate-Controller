/**
* @file    wmbus_Linkcommand.c
* @author  SRA - NOIDA / RF application group - AMG
* @version V2.5.1
* @date    14-Sept-2020
* @brief   This file contains routines for LINK LAYER commands
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

/* Includes ------------------------------------------------------------------*/
#include "wmbus_link.h"
#include "wmbus_linkcommand.h"

/** @addtogroup WMBUS_Library
* @{
*/


/** @addtogroup WMBus_LinkLayer
*   @{
*/

/** @addtogroup WMBus_LinkCommand
*   @{
*/


/** @defgroup WMBus_LinkCommand_Private_Variables   WMBus LinkCommand Private Variables
* @{
*/
static uint8_t fcb_fcv=0;
static uint8_t acd_dfc=0;
extern LL_State_t LL_State;
extern Frame_t TxFrame;


/**
* @}
*/


/** @defgroup WMBus_LinkCommand_Private_Functions    WMBus LinkCommand Private Functions
* @{
*/

void WMBus_LinkSetFcv(uint8_t fcv)
{
  if(fcv)
  {
    fcb_fcv |= LINK_C_FIELD_FCV_MASK;
  }
  else
  {
    fcb_fcv &= (~(uint8_t)LINK_C_FIELD_FCV_MASK);
  }
}

void WMBus_LinkSetFcb(uint8_t fcb)
{
  if(fcb)
  {
    fcb_fcv |= LINK_C_FIELD_FCB_MASK;
  }
  else
  {
    fcb_fcv &= (~(uint8_t)LINK_C_FIELD_FCB_MASK);
  }
}

void WMBus_LinkSetAcd(uint8_t acd)
{
  if(acd)
  {
    acd_dfc |= LINK_C_FIELD_ACD_MASK;
  }
  else
  {
    acd_dfc &= (~(uint8_t)LINK_C_FIELD_ACD_MASK);
  }
}

void WMBus_LinkSetDfc(uint8_t dfc)
{
  if(dfc)
  {
    acd_dfc |= LINK_C_FIELD_DFC_MASK;
  }
  else
  {
    acd_dfc &= (~(uint8_t)LINK_C_FIELD_DFC_MASK);
  }
}


LINK_STATUS_t WMBus_LinkSetFrame(uint8_t *payload, uint16_t size, uint8_t ci_field)
{

  /* TxFrame.m_field and TxFrame.a_field now point to the 
  LinkAttr.MField and LinkAttr.AField directly */
  
  TxFrame.data_field.size = size;
  TxFrame.data_field.payload = payload;
  
  TxFrame.ci_field=ci_field;
  
  
  return WMBus_LinkSetTxFrameBuffer(&TxFrame);
  
}


uint8_t WMBus_LinkSendInstallationRequest(uint8_t *appliData,
                                          uint8_t length,
                                          uint8_t ci_field)
{  
  
  if(LL_State!=LL_SM_IDLE)
    return 0; /*Error because SM is not idle*/
  
  /* Set the c field*/
  TxFrame.c_field = LL_SND_IR;
  
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  {
    LL_State = LL_SM_SEND_SND_IR;
    return 1;   /* No Error*/
  }
  
  return 0;   /* Error*/
}


uint8_t WMBus_LinkSendNoReplyApplicationData(uint8_t *appliData,
                                             uint8_t length,
                                             uint8_t ci_field)
{  
  if(LL_State!=LL_SM_IDLE)
    return 0; /*Error because SM is not idle*/
    
  /* Set the c field*/
  TxFrame.c_field = LL_SND_NR | fcb_fcv;
  
  
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  {
    LL_State = LL_SM_SEND_SND_NR;
    return 1;   /* No Error*/
  }
  
  return 0;   /* Error*/
  
}


uint8_t WMBus_LinkSendAccessDemand(uint8_t *appliData,
                                   uint8_t length,
                                   uint8_t ci_field)
{
  
  
  if(LL_State!=LL_SM_IDLE)
    return 0; /* Error because SM is not idle*/
  
 
  
  /* Set the c field*/
  TxFrame.c_field = LL_ACC_DMD | fcb_fcv;
  
  WMBus_LinkSetFrame(appliData, length, ci_field);
  
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  {
    LL_State = LL_SM_SEND_ACC_DMD;   
  
    return 1; /*No error*/     
  }
  
  return 0;   /* Error*/
}


uint8_t WMBus_LinkSendAccess_NoReply(uint8_t *appliData,
                                     uint8_t length,
                                     uint8_t ci_field)
{
  
  
  if(LL_State!=LL_SM_IDLE)
    return 0; /*Error because SM is not idle*/
  
    
  /* Set the c field*/
  TxFrame.c_field = LL_ACC_NR | fcb_fcv;
  
  WMBus_LinkSetFrame(appliData, length, ci_field);
  
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  {
    LL_State = LL_SM_SEND_ACC_NR;
    return 1; /*No error*/   
  }
  
  return 0;   /* Error*/
  
}


uint8_t WMBus_LinkSendResponse_UD(uint8_t *appliData,
                                  uint8_t length,
                                  uint8_t ci_field)
{ 
  
  if(LL_State!=LL_SM_IDLE)
    return 0; /*Error because SM is not idle*/
  
  /* Set the c field*/
  TxFrame.c_field = LL_RSP_UD | acd_dfc;
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  {
    LL_State = LL_SM_SEND_RSP_UD;
  
    return 1; /*No error*/  
  }
  
  return 0;   /* Error*/
}


/**
* @brief  This function will send Ack.
* @param  ci_field: CI Field of Frame 
* @retval Success/Error 
*                       
*/
uint8_t WMBus_LinkSendAck(uint8_t *appliData, uint8_t length, uint8_t ci_field)
{
  
  if(LL_State != LL_SM_IDLE)
    return 0;                /*ERROR, because SM is not IDLE*/
  

  /* Set the c field*/
  TxFrame.c_field = LL_ACK;
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  {
    LL_State = LL_SM_SEND_ACK;
    return 1; /*No error*/
  }
  return 0;   /* Error*/
}

/**
* @brief  This function will send SND-NKE from concentrator to meter
* @param  ci_field: CI Field of Frame
* @retval Success/Error 
*                       
*/
uint8_t WMBus_LinkSendResetRemoteLink(uint8_t *appliData, uint8_t length, uint8_t ci_field)
{
  
  if(LL_State != LL_SM_IDLE)
    return 0;                /*ERROR, because SM is not IDLE*/
  

  /* Set the C Field*/
  TxFrame.c_field = LL_SND_NKE;
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  {
    LL_State = LL_SM_SEND_NKE;
  
    return 1; /*No error*/
  }
  return 0;   /* Error*/
}

/**
* @brief  This function will send SND-UD from concentrator to meter
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                      
*/
uint8_t WMBus_LinkSendUserDataCommand(uint8_t *appliData,
                                      uint8_t length,
                                      uint8_t ci_field)
{
  
  if(LL_State!=LL_SM_IDLE)
    return 0; /*Error because SM is not idle*/
  
  
  TxFrame.c_field = LL_SND_UD2 | fcb_fcv;
  
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  { 
    LL_State = LL_SM_SEND_SND_UD;
    
    return 1; /*No error*/    
  }
  return 0;   /* Error*/
}

/**
* @brief  This function will send SND-UD2 from concentrator to meter
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                       
*/
uint8_t WMBus_LinkSendUserData2Command(uint8_t *appliData,
                                       uint8_t length,
                                       uint8_t ci_field)
{
  if(LL_State!=LL_SM_IDLE)
    return 0; /*ERROR,because SM is not IDLE*/
  
  
  /* Set the c field*/
  TxFrame.c_field = LL_SND_UD2; 
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  { 
    LL_State = LL_SM_SEND_SND_UD2;
    
    return 1; /*No error*/    
  }
  return 0;   /* Error*/
}


/**
* @brief  This function will send REQ-UD1 from concentrator to meter
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                   
*/
uint8_t WMBus_LinkRequestUserData1(uint8_t *appliData,
                                   uint8_t length,
                                   uint8_t ci_field)
{
  
  if(LL_State!=LL_SM_IDLE)
    return 0; /*Error because SM is not idle*/
  
  
  /* Set the c field*/
  TxFrame.c_field = LL_REQ_UD1 | fcb_fcv;
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  { 
    LL_State=LL_SM_SEND_REQ_UD1;
    
    return 1; /*No error*/    
  }
  return 0;   /* Error*/
}

/**
* @brief  This function will send REQ-UD2 from concentrator to meter
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                   
*/
uint8_t WMBus_LinkRequestUserData2(uint8_t *appliData,
                                   uint8_t length,
                                   uint8_t ci_field)
{
  
  if(LL_State!=LL_SM_IDLE)
    return 0; /*Error because SM is not idle*/
  
  /* Set the c field*/
  TxFrame.c_field = LL_REQ_UD2  | fcb_fcv;
  
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  { 
    LL_State = LL_SM_SEND_REQ_UD2;
    
    return 1; /*No error*/    
  }
  return 0;   /* Error*/
}

/**
* @brief  This function will send CNF-IR from concentrator to meter
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                     
*/
uint8_t WMBus_LinkSendConfirmInstallationRequest(uint8_t *appliData,
                                                 uint8_t length,
                                                 uint8_t ci_field)
{
  
  if((LL_State != LL_SM_IDLE) && (LL_State !=LL_SM_TEST_MODE))
    return 0;                /*ERROR, because SM is not IDLE*/
  
  /* Set the c field*/
  TxFrame.c_field = LL_CNF_IR;
  
  if(WMBus_LinkSetFrame(appliData, length, ci_field)==LINK_STATUS_SUCCESS)
  {
    
    LL_State = LL_SM_SEND_CNF_IR;
    return 1; /*No error*/    
  }
  return 0;   /* Error*/
}

/**
* @brief  This function requires a VCO calibration to the PHY layer acting on the
*          link layer state machine.
*         The function returns an error code if the calibration cannot be
*         initiated because the state machine is not is idle.
* @param   None.
* @retval  Success (1) / Error (0)                     
*/
uint8_t WMBus_LinkVcoCalibrationRequest(void)
{
  
  if(LL_State != LL_SM_IDLE)
    return 0;                /*ERROR, because SM is not IDLE*/
  
  /* Move the LL state */
  LL_State = LL_SM_VCO_CAL;
  
  return 1; /*No error*/    
  
}


/**
* @brief  This function sends the wmbus message according to the user.
* @param  param: command select.
* @retval None
*/
void WMBus_SendCommand(uint8_t  param)
{
  switch(param)
  {
  case LL_SND_IR:
    WMBus_LinkSendInstallationRequest(0, 0, CI_RESPONSE_FROM_DEV_SHORT);
    break;
    
  case LL_ACC_DMD:
    WMBus_LinkSendAccessDemand(0, 0, CI_TRANSPORT_FROM_DEV_SHORT);  
    break;
    
  case LL_SND_NR :
    WMBus_LinkSendNoReplyApplicationData(0, 0, CI_RESPONSE_FROM_DEV_SHORT); 
    break;
    
  case LL_ACC_NR :
    WMBus_LinkSendAccess_NoReply(0,0, CI_RESPONSE_FROM_DEV_SHORT);//Check CI
    break;
    
  case LL_RSP_UD:
    WMBus_LinkSendResponse_UD(0,0, CI_RESPONSE_FROM_DEV_SHORT);
    break;
    
  case LL_ACK:
    WMBus_LinkSendAck(0,0, CI_RESPONSE_FROM_DEV_SHORT);
    break;
    
  }
}

/**
* @}
*/


/**
* @}
*/


/**
* @}
*/

/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
