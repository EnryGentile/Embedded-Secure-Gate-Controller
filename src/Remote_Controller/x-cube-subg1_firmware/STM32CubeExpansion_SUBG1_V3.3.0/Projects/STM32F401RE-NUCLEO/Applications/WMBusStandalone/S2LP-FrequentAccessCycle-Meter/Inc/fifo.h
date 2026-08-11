/**
******************************************************************************
* @file    fifo.h
* @author  SRA-NOIDA 
* @version V3.3.0
* @date    27-Apr-2021
* @brief   Header file for fifo.c
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

#ifndef __FIFO_H__
#define __FIFO_H__

/*******************************************************************************
 * Include Files
 */
#include <stdint.h>

/*******************************************************************************
 * Type Definitions
 */
typedef struct
{
    uint16_t  size;
    uint8_t*  p_data;
    uint16_t  count;
    uint16_t  front;
    uint16_t  back;
} fifo_t;

/*******************************************************************************
 * Functions
 */
void fifo_init(fifo_t* p_fifo);
 void fifo_queue(fifo_t* p_fifo, uint8_t data);
 uint8_t fifo_dequeue(fifo_t* p_fifo, uint8_t* p_data);
 uint16_t fifo_get_count(const fifo_t* p_fifo);

#endif
