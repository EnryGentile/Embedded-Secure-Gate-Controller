/**
* @file    wmbus_crc.c
* @author  SRA - NOIDA / RF application group - AMG
* @version V2.5.1
* @date    14-Sept-2020
* @brief   This file contains routines for CRC calculation
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
#include "wmbus_crc.h"

/** @addtogroup WMBUS_Library
* @{
*/


/** @addtogroup WMBus_LinkLayer
*   @{
*/

/** @addtogroup WMBus_CRC
*   @{
*/




/** @defgroup WMBus_CRC_Private_Defines  WMBus CRC Private Defines
* @{
*/
#define CRC_POLYNOM         0x3D65

/**
* @}
*/



/** @defgroup WMBus_CRC_Private_FunctionPrototypes   WMBus CRC Private Function Prototypes
* @{
*/
uint16_t crcCalc(uint16_t crcReg, uint8_t crcData); 

/**
* @}
*/

/** @defgroup WMBus_CRC_Private_Functions   WMBus CRC Private Functions
* @{
*/

/**
* @brief  Calculates the 16-bit CRC. 
* @param  crcData  - Data to perform the CRC-16 operation on.
*         crcReg   - Current or initial value of the CRC calculation
* @retval None                 
*/
void AppendCRC(uint8_t *pStart, uint8_t *pStop)
{
  uint16_t  seed= 0x0000;  
  while (pStart != pStop)
  {
    seed = crcCalc(seed, *pStart);
    pStart++;
  }
  seed =~seed;
  pStop[0]  = (uint8_t)(seed>>8);
  pStop[1]  = (uint8_t)seed;
}

/**
* @brief  Calculates the 16-bit CRC. 
* @param  crcData  - Data to perform the CRC-16 operation on.
*         crcReg   - Current or initial value of the CRC calculation
* @retval 0 or 1
*                       
*/
uint8_t CRCCheck(uint8_t *pStart, uint8_t *pStop)
{
  uint16_t  seed= 0x0000;
  
  while (pStart != pStop)
  {
    seed = crcCalc(seed, *pStart);
    pStart++;
  }
  seed =~seed;
  if((pStop[0]  == (uint8_t)(seed>>8))&&(pStop[1]  == (uint8_t)(seed)))
  {
    return 1;
  }
  return 0;
}

/**
* @brief  Calculates the 16-bit CRC. The function requires
*         that the CRC_POLYNOM is defined,
*           which gives the wanted CRC polynom. 
* @param  crcData  - Data to perform the CRC-16 operation on.
*         crcReg   - Current or initial value of the CRC calculation
* @retval crcReg Current or initial value of the CRC calculation                   
*/
uint16_t crcCalc(uint16_t crcReg, uint8_t crcData) 
{
  uint8_t i;
  
  for (i = 0; i < 8; i++) 
  {
    // If upper most bit is 1
    if (((crcReg & 0x8000) >> 8) ^ (crcData & 0x80))
      crcReg = (crcReg << 1)  ^ CRC_POLYNOM;
    else
      crcReg = (crcReg << 1);
    
    crcData <<= 1;
  }
  
  return crcReg;
}

static uint16_t crc_seed=0;

/**
* @brief  Calculates the 16-bit CRC.
*          The result is set in a static variable that is used as a seed.
*          This is usable for iterative procedures passing data for CRC computing byte by byte.
* @param  crcData  - Data to perform the CRC-16 operation on.
* @retval None.                   
*/
void crcAppend(uint8_t crcData)
{
  crc_seed=crcCalc(crc_seed,crcData);
}

/**
* @brief  Verifies the 16-bit CRC taking as input the CRC for checking.
*         The data passed are compared with the CRC computed until this time by the @ref crcAppend.
* @param  crc  - CRC received bytes.
* @retval None.                   
*/
uint8_t crcVerify(uint8_t* crc)
{
  uint8_t ret=0;
  crc_seed =~crc_seed;
  if((crc[0]  == (uint8_t)(crc_seed>>8))&&(crc[1]  == (uint8_t)(crc_seed)))
  {
    ret=1;
  }
  crc_seed=0;
  
  return ret;
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
