/**
******************************************************************************
* @file    command.h
* @author  SRA-NOIDA 
* @version V3.3.0
* @date    27-Apr-2021
* @brief   Header file for command.c
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

#ifndef __COMMAND_H__
#define __COMMAND_H__

/*******************************************************************************
 * Include Files
 */
#include <stdint.h>

/*******************************************************************************
 * Functions
 */
   
/** @defgroup command_Exported_Variables        Exported Variables
* @{
*/
extern uint8_t TempDataBuff[]; 


typedef enum
{
  DATA_FLAG = 0,
  ALARM_FLAG = 1,
  SND_UD_FLAG = 2,
  SND_UD2_FLAG = 3,
  RESET_LINK_FLAG =4,
  RESET_ALL_FLAG = 5
} OtherCmdFlag;


/**
* @}
*/

/** @defgroup command_Exported_MACRO        Exported MACRO
* @{
*/
#define MAX_COMMAND                    31
#define WMBUS_CMD_OFFSET               0
#define SUPPORT_ERROR_MESSAGE          "\nNOT SUPPORTED\r\n"

#define HW_VERSION                  1
#define FW_MAJOR_VERSION            1
#define FW_MINOR_VERSION            03

#define WMBUS_FUNCT      0 
#define SNIFFER_FUNCT    1
#define RAW_FUNCT        2
 
   
#define ENCKEY_LEN              16   /*16 bytes Encryption Key is used*/
   
   
// General response messages
#define RESPONSE_OK                 "\nOK\n\r"
#define RESPONSE_ERROR              "\nERROR\n\r"
#define RESPONSE_ERROR_PARAM        "\nERROR PARAM\n\r"
#define RESPONSE_ERROR_VALUE        "\nERROR VALUE\n\r"
#define COMMAND_ERROR               "\nCOMMAND ERROR\n\r"

#define HARDWARE_ID    "\nS2-LP\n"
#define FIRMWARE_ID    "\nS2-LP-WMBUS\n" 


/**
* @}
*/

/** @defgroup command_Exported_Functions        Exported Functions
* @{
*/ 
void cmd_process(uint8_t *p_cmd, uint8_t len);
uint8_t cmd_process_escape_seq(uint8_t rx);
uint8_t cmd_process_escape_seq_timeouts(void);
/*
static void WMB_SaveCurrentCfg(uint32_t param);
static void WMB_SelectDevice(uint32_t );
static void WMB_SetMode(uint32_t);
static void WMB_SetPacketFormat(uint32_t);
static void WMB_Update_Parameters(uint32_t );
static void WMB_SetDeviceIdentification(uint32_t param);
static void  WMB_SetOutputPower(uint32_t param);
static void  WMB_SetRSSIThreshold(uint32_t param);
static void WMB_EncryptionEnable(uint32_t param);
static void WMB_SetEncryptionKey(uint32_t param);
static void WMB_GetEncryptionKey(uint32_t param);
static void WMB_GetEncryptionStatus(uint32_t param);
static void  WMB_GetPower(uint32_t param);
static void  WMB_GetRSSIThreshold(uint32_t param);
static void WMB_GetPacketFormat(uint32_t param);
static void WMB_GetMode(uint32_t param);
static void WMB_GetDeviceType(uint32_t param);
static void  WMB_GetOutputPower(uint32_t param);
static void WMB_GetDeviceIdentification(uint32_t param);
static void  WMB_GetRSSI(uint32_t param);
static void WMB_GetVersion(uint32_t param);
static void WMB_SetBaudRate(uint32_t param);
static void WMB_PerformSoftReset(uint32_t param);
static void WMB_LoadDefaultCfg(uint32_t param);
static void WMB_SetModuleFunctionality(uint32_t param);
static void WMB_GetCurrentChannel(uint32_t param);
static void WMB_GetModuleFunctionality(uint32_t param);
static void WMB_SetCurrentChannel(uint32_t param);
static void WMB_GetMeterDatabase(uint32_t param);
*/
void WMB_AddNewMeter(uint32_t param);
void WMB_DeleteMeter(uint32_t param);
void WMB_SetEncryption(uint32_t param);
uint8_t TextToHex(uint8_t *input,uint8_t *output,uint8_t textLen);
void HexToText(uint8_t *input,uint8_t *output,uint8_t len);
void WMB_SetNextCommandFlag(uint32_t param );
void WMB_SetCurrentCommandFlag(uint32_t param );
void WMB_GetHardwareID(uint32_t param);
void WMB_GetFirmwareID(uint32_t param);
uint8_t SubstractVal(uint8_t input);
void WMB_SendMessage(uint32_t param);

/**
* @}
*/
#endif
