/**
* @file    wmbus_datarecord_utils.c
* @author  SRA - NOIDA / RF application group - AMG
* @version V2.5.1
* @date    14-Sept-2020
* @brief   This file contains header for wmbus_datarecord_utils.c.
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
* @addtogroup wmbus_datarecord_utils    WMBus DataRecord Utils
* @brief Some utilities to manage the some records.
* @{
*/



/* Includes ------------------------------------------------------------------*/
#include "stdint.h"

/**
* @defgroup wmbus_datarecord_utils_exported_defines     WMBus DataRecord Utils Exported Defines
* @{
*/

/**
* @defgroup data_record_types     WMBus DataRecord Types
* @{
*/
#define RECORD_TYPE_ENERGY_WH               0x00    // 10^(nnn-3) [Wh]
#define RECORD_TYPE_ENERGY_JOULE            0x08    // 10^nnn     [J]
#define RECORD_TYPE_VOLUME                  0x10    // 10^(nnn-6) [m^3]
#define RECORD_TYPE_MASS                    0x18    // 10^(nnn-3) [kg]
#define RECORD_TYPE_POWER_W                 0x28    // 10^(nnn-3) [W]
#define RECORD_TYPE_POWER_J_H               0x30    // 10^(nnn)   [J/h]
#define RECORD_TYPE_VOLUME_FLOW             0x38    // 10^(nnn-6) [m^3/h]

/**
*@}
*/

/**
*@}
*/

/** @defgroup datarecord_utils_Exported_Functions      WMBus DataRecord Utils Exported Functions
* @{
*/
uint8_t data_record_bcd8_enc(float data, uint8_t type, uint8_t* buffer);
void data_record_dec(uint8_t* buffer, char* msg);


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




