/**
* @file    wmbus_linkcommand.h
* @author  SRA- NOIDA
* @version V2.5.1
* @date    14-Sept-2020
* @brief   This file contains function headers for LINK LAYER commands
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WMBUS_LINKCOMMAND_H
#define __WMBUS_LINKCOMMAND_H

/* Includes ------------------------------------------------------------------*/
#include "wmbus_phy.h"

/** @addtogroup WMBUS_Library
* @{
*/


/** @addtogroup WMBus_LinkLayer
*   @{
*/

/** @addtogroup WMBus_LinkCommand         WMBus LinkCommand
* @brief Configuration and management of WMBus Link layer Command.
* @details See the file <i>@ref wmbus_linkcommand.h</i> for more details.
* @{
*/



/** @defgroup WMBus_LinkCommand_Exported_Functions  WMBus LinkCommand Exported Functions
* @{
*/

/**
* @brief  This function will send Installation request 
*         from meter to concentrator.
* @param  *appliData:Application Data pointer
* @param  length: Message Length
* @param  ci_field: CI Filed.
* @retval Success/Error                     
*/
uint8_t WMBus_LinkSendInstallationRequest(uint8_t *appliData,
                                          uint8_t length,
                                          uint8_t ci_field);

/**
* @brief  This function will send No-Reply(Unsolicited) user data request from 
*         meter to concentrator.
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                     
*/
uint8_t WMBus_LinkSendNoReplyApplicationData(uint8_t *appliData, 
                                             uint8_t length,
                                             uint8_t ci_field);

/**
* @brief  This function will send ACC-DMD from meter to concentrator.
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                      
*/
uint8_t WMBus_LinkSendAccessDemand(uint8_t *appliData,
                                   uint8_t length,
                                   uint8_t ci_field);

/**
* @brief  This function will send ACC-NR from meter to concentrator.
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                       
*/
uint8_t WMBus_LinkSendAccess_NoReply(uint8_t *appliData,
                                     uint8_t length,
                                     uint8_t ci_field);

/**
* @brief  This function will send RESP-UD from meter to concentrator.
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                      
*/
uint8_t WMBus_LinkSendResponse_UD(uint8_t *appliData,
                                  uint8_t length,
                                  uint8_t ci_field);

/**
* @brief  This function will send Ack.
* @param  ci_field: CI Field of Frame 
* @retval Success/Error                    
*/
uint8_t WMBus_LinkSendAck(uint8_t *appliData,
                            uint8_t length,
                            uint8_t ci_field);

/**
* @brief  This function will send SND-NKE from concentrator to meter
* @param  ci_field: CI Field of Frame
* @retval Success/Error                      
*/
uint8_t WMBus_LinkSendResetRemoteLink(uint8_t *appliData,
                               uint8_t length,
                               uint8_t ci_field);

/**
* @brief  This function will send SND-UD from concentrator to meter
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                      
*/
uint8_t WMBus_LinkSendUserDataCommand(uint8_t *appliData,
                                      uint8_t length,
                                      uint8_t ci_field);

/**
* @brief  This function will send SND-UD2 from concentrator to meter
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                       
*/
uint8_t WMBus_LinkSendUserData2Command(uint8_t *appliData,
                                       uint8_t length,
                                       uint8_t ci_field);


/**
* @brief  This function will send REQ-UD1 from concentrator to meter
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                   
*/
uint8_t WMBus_LinkRequestUserData1(uint8_t *appliData,
                                   uint8_t length,
                                   uint8_t ci_field);

/**
* @brief  This function will send REQ-UD2 from concentrator to meter
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                   
*/
uint8_t WMBus_LinkRequestUserData2(uint8_t *appliData,
                                   uint8_t length,
                                   uint8_t ci_field);

/**
* @brief  This function will send CNF-IR from concentrator to meter
* @param  *appliData:Application Data pointer
* @param   length: Message Length
* @param   ci_field: CI Filed.
* @retval  Success/Error                     
*/
uint8_t WMBus_LinkSendConfirmInstallationRequest(uint8_t *appliData,
                                                 uint8_t length,
                                                 uint8_t ci_field);

/**
* @brief  Set the FCV bit for the next frame.
* @note This bit is used only for frames from primary stations.
* @param  fcv: FDV bit, 1 or 0.
* @retval None
*/
void WMBus_LinkSetFcv(uint8_t fcv);

/**
* @brief  Set the FCB bit for the next frame.
* @note This bit is used only for frames from primary stations.
* @param  fcb: FDB bit, 1 or 0.
* @retval None
*/
void WMBus_LinkSetFcb(uint8_t fcb);

/**
* @brief  Set the ACD (access demand) bit for the next frame.
* @note This bit is used only for frames from secondary stations.
* @param  acd: ACD bit, 1 or 0.
* @retval None
*/
void WMBus_LinkSetAcd(uint8_t acd);

/**
* @brief  Set the DFC (data flow control) bit for the next frame.
* @note This bit is used only for frames from secondary stations.
* @param  dfc: DFC bit, 1 or 0.
* @retval None
*/
void WMBus_LinkSetDfc(uint8_t dfc);

/**
* @brief  This function requires a VCO calibration to the PHY layer
* @param   None.
* @retval  Success/Error                     
*/
uint8_t WMBus_LinkVcoCalibrationRequest(void);

/**
* @brief  This function sends the wmbus message according to the user.
* @param  param:command select.
* @retval None
*/
void WMBus_SendCommand(uint8_t  param);

/**
* @}
*/

/**
*@}
*/

/**
* @}
*/

/**
* @}
*/

#endif/* __WMBUS_LINKCOMMAND_H*/
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
