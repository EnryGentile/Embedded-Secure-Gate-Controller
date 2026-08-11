/**
* @file    wmbus_crc.h
* @author  SRA - NOIDA / RF application group - AMG
* @version V2.5.1
* @date    14-Sept-2020
* @brief   This file contains routines for CRC computation
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
#ifndef __WMBUS_CRC_H
#define __WMBUS_CRC_H
/* Includes ------------------------------------------------------------------*/
#include "wmbus_link.h"

/** @addtogroup WMBUS_Library
* @{
*/

/** @addtogroup WMBus_LinkLayer
*   @{
*/

/** @addtogroup WMBus_CRC         WMBus CRC
* @brief Configuration and management of WMBus Link layer CRC.
* @details See the file <i>@ref wmbus_crc.h</i> for more details.
* @{
*/



/** @defgroup WMBus_CRC_Exported_Functions        WMBus CRC Exported Functions
* @{
*/
void AppendCRC(uint8_t *pStart, uint8_t *pStop);
uint8_t CRCCheck(uint8_t *pStart, uint8_t *pStop);

void crcAppend(uint8_t crcData);
uint8_t crcVerify(uint8_t* crc);

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

#endif  /*__WMBUS_CRC_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
