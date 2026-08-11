/**
 * @file    SPIRIT_Util.h
 * @author  LowPower RF BU - AMG
 * @version 1.1.0
 * @date    July 1, 2016
 * @brief   Identification functions for S2-LP DK.
  ******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2019 STMicroelectronics</center></h2>
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
*
 * <h2><center>&copy; COPYRIGHT 2019 STMicroelectronics</center></h2>
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef S2LP_SDK_UTIL_H_
#define S2LP_SDK_UTIL_H_


/* Includes ------------------------------------------------------------------*/
#include "S2LP_Config.h"
#include "radio_eeprom.h"
#include "radio_timer.h"
#include "radio_gpio.h"

#ifdef __cplusplus
  "C" {
#endif


/**
 * @addtogroup SPIRIT_DK
 * @{
 */


/**
 * @defgroup SDK_SPIRIT_MANAGEMENT
 * @{
 */

/*Structure to manage External PA */
typedef enum
{
  RANGE_EXT_NONE = 0,
  RANGE_EXT_SKYWORKS_868,
  RANGE_EXT_SKYWORKS_SKY66420 = 3,
} RangeExtType;

typedef enum
{
  S2LP_CUT_2_1 = 0x91,
  S2LP_CUT_2_0 = 0x81,
  S2LP_CUT_3_0 = 0xC1,
} S2LPCutType;

typedef enum
{
  PA_SHUTDOWN     = 0x00,
  PA_TX_BYPASS    = 0x01,
  PA_TX     	  = 0x02,
  PA_RX     	  = 0x03,
} PA_OperationType;
/**
 * @addgroup SDK_SPIRIT_MANAGEMENT_FUNCTIONS
 * @{
 */
void S2LPManagementEnableTcxo(void);
void S2LPManagementIdentificationRFBoard(void);
void S2LPManagementSetBand(uint8_t value);
uint8_t S2LPManagementGetBand(void);
void S2LPManagementSetOffset(int32_t value);
int32_t S2LPManagementGetOffset(void);
void S2LPManagementRcoCalibration(void);
void S2LPManagementRangeExtInit(void);
RangeExtType S2LPManagementGetRangeExtender(void);
S2LPCutType S2LPManagementGetCut(void);
uint8_t S2LPManagementGetTcxo(void);
void S2LPManagementTcxoInit(void);
uint8_t S2LPQiGetLqi(void);
void Config_RangeExt(PA_OperationType operation,RangeExtType rangeExtType);
/**
* @}
*/

/**
* @}
*/

/**
* @}
*/

#ifdef __cplusplus
}
#endif


#endif


 /******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
