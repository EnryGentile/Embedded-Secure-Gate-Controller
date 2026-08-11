/**
* @file    wmbus_datarecord_utils.c
* @author  SRA - NOIDA / RF application group - AMG
* @version V2.5.1
* @date    14-Sept-2020
* @brief   This file contains some utility routines used by the application.
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
* @{
*/

/* Includes ------------------------------------------------------------------*/
#include "wmbus_datarecord_utils.h"
#include "string.h"
#include "stdio.h"


/**
* @defgroup wmbus_datarecord_utils_Private_Variables      WMBus DataRecord Utils Private Defines
* @{
*/
#define RECORD_COD_INT8        1
#define RECORD_COD_INT16       2
#define RECORD_COD_INT24       3
#define RECORD_COD_INT32       4
#define RECORD_COD_REAL32      5
#define RECORD_COD_INT48       6
#define RECORD_COD_INT64       7
#define RECORD_COD_BCD2        9
#define RECORD_COD_BCD4        10
#define RECORD_COD_BCD6        11
#define RECORD_COD_BCD8        12
#define RECORD_COD_BCD12       14

/**
*@}
*/


/** @defgroup datarecord_utils_Private_Functions      WMBus DataRecord Utils Private Functions
* @{
*/


static uint32_t data_bcd_encode(unsigned char *bcd_data, size_t bcd_data_size, int value)
{
    uint32_t v0, v1, v2, x1, x2;
    size_t i;

    if (bcd_data && bcd_data_size && (value >= 0))
    {
        v2 = value;

        for (i = 0; i < bcd_data_size; i++)
        {
            v0 = v2;
            v1 = (uint32_t)(v0 / 10);
            v2 = (uint32_t)(v1 / 10);

            x1 = v0 - v1 * 10;
            x2 = v1 - v2 * 10;

            bcd_data[/*bcd_data_size-1-*/i] = (x2 << 4) | x1;
        }

        return 0;
    }

    return 1;
}


static uint32_t data_bcd_decode(unsigned char *bcd_data, size_t bcd_data_size)
{
    long long val = 0;
    size_t i;

    if (bcd_data)
    {
        for (i = bcd_data_size; i > 0; i--)
        {
            val = (val * 10) + ((bcd_data[i-1]>>4) & 0xF);
            val = (val * 10) + ( bcd_data[i-1]     & 0xF);
        }

        return val;
    }

    return 1;
}

/**
* @brief  This routine decodes a data record returning a string from a given buffer.
* @param  buffer: the received application payload.
* @param  msg: the decoded string.
* @retval None
*/
void data_record_dec(uint8_t* buffer, char* msg)
{
  if((buffer[0]&0x0F)!=RECORD_COD_BCD8)
  {
    strcpy(msg,"{}");
    return;
  } 

  char meas[25];
  int32_t nnn,k=0;
  uint32_t nd=0;

  
  nnn=(buffer[1]&0x07);
  switch(buffer[1] & 0xF8)
  {
  case RECORD_TYPE_ENERGY_JOULE:
    k=0;
    strcpy(meas,"J energy");
    break;
  case RECORD_TYPE_POWER_J_H:
    k=0;
    strcpy(meas,"J/h power");
    break;
  case RECORD_TYPE_ENERGY_WH:
    k=3;
    strcpy(meas,"Wh energy");
    break;
  case RECORD_TYPE_MASS:
    k=3;
    strcpy(meas,"kg mass");
    break;
  case RECORD_TYPE_POWER_W:
    k=3;
    strcpy(meas,"W power");
    break;
  case RECORD_TYPE_VOLUME:
    k=6;
    strcpy(meas,"m^3 volume");
    break;
  case RECORD_TYPE_VOLUME_FLOW:
    k=6;
    strcpy(meas,"m^3/h volume flow");
    break;
  }
  
  nd=data_bcd_decode(&buffer[2],4);
  
  sprintf(msg, "{%lu * 10^%lu %s}",nd,nnn-k,meas);
  
}


/**
* @brief  This routine encodes a data record returning inserting it in a given buffer.
* @param  data: a float representing the measure to the encoded.
* @param  type: the measure type. This parameter can be a value of @ref data_record_types.
* @param  buffer: the destination buffer to be used as applicaiton payload.
* @retval None
*/
uint8_t data_record_bcd8_enc(float data, uint8_t type, uint8_t* buffer)
{
  int32_t mult=1;
  int32_t e,nnn;
  uint8_t k=0;
  uint32_t nd;
  float fnd;
  
  switch(type)
  {
  case RECORD_TYPE_ENERGY_WH:
  case RECORD_TYPE_MASS:
  case RECORD_TYPE_POWER_W:
    k=3;
    break;
  case RECORD_TYPE_VOLUME:
  case RECORD_TYPE_VOLUME_FLOW:
    k=6;
    break;
  }
  buffer[0]=RECORD_COD_BCD8;
  
    
  // determine the nnn...
  for(e=0;e<=7;e++)
  {
    fnd=data*mult;
    nd=(int32_t)fnd;
    if(fnd-(int32_t)(nd)==0)
      break;
    mult*=10;
    
  }
  nnn=k-e;
  if(nnn<0)
  {
    nnn=k;
    nd=(uint32_t)data;
  }
  
  buffer[1]=type|nnn;
  
  
  data_bcd_encode(&buffer[2], 4, nd);
  
  
  return 6;
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
