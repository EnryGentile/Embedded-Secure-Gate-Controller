/**
* @file    wmbus_cbc.h
* @author  SRA - NOIDA / RF application group - AMG
* @version V2.5.1
* @date    14-Sept-2020
* @brief   This file contains header for wmbus_cbc.h.
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
* @addtogroup  wmbus_encryption 	wM-Bus Encryption
* @{
*/
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "wmbus_link.h"

/** @defgroup wmbus_encryption_Exported_Macros      wM-Bus Encryption Exported Macros
* @{
*/
#define IS_CBC_5_CONFIG_WORD(CONFIG_WORD)               ((CONFIG_WORD & 0x0500)?(1):(0))
#define SIZE_IN_CONFIG_WORD(CONF_WORD,SIZE)          {CONF_WORD = 0x0500; CONF_WORD|=((SIZE%16)==0)?((SIZE)/16)<<4:((SIZE)/16+1)<<4;}

/**
*@}
*/

/** @defgroup wmbus_encryption_Exported_Functions      wM-Bus Encryption Exported Functions
* @{
*/

uint16_t WMBus_AES_Encrypt_CBC(uint8_t *payload, uint16_t size, uint8_t *key, uint8_t *enc_payload, uint8_t *iv);
uint16_t WMBus_AES_Decrypt_CBC(uint8_t *payload, uint16_t size, uint8_t *key, uint8_t *dec_payload, uint8_t *iv);

/**
*@}
*/


/**
*@}
*/


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
