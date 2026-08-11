/**
******************************************************************************
* @file    radio-arch.c
* @author  System LAB
* @version V1.0.0
* @date    17-June-2015
* @brief   Source file for SPIRIT1 and S2LP
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
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
/*---------------------------------------------------------------------------*/
/**
 * @addtogroup ST_Radio
 * @ingroup STM32_Contiki_Library
 * @{
 */

#ifdef USE_STM32L1XX_NUCLEO
#include "stm32l1xx.h"
#endif

#ifdef USE_STM32F4XX_NUCLEO
#include "stm32f4xx.h"
#endif
#include "radio-arch.h"
#include "radio-driver.h"
#include "st-lib.h"
/*---------------------------------------------------------------------------*/
extern st_lib_spi_handle_typedef st_lib_p_spi_handle;
/*---------------------------------------------------------------------------*/

/**
 * @brief  radio_arch_refresh_status
 * 		check the status of the Spirit1 radio
 * @param  None
 * @retval uint16_t mcstate
 */
uint16_t
radio_arch_refresh_status(void)
{
#ifdef USE_SPIRIT1_DEFAULT
  SpiritRefreshStatus();
#endif /*#ifdef USE_SPIRIT1_DEFAULT*/

#ifdef USE_S2_LP_DEFAULT
  S2LPRefreshStatus();
#endif /*#ifdef USE_S2_LP_DEFAULT*/
  return (g_xStatus.MC_STATE << 1);
}

/** @} */

