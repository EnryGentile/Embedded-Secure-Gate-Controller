/**
******************************************************************************
* @file    fifo.c
* @author  SRA-NOIDA 
* @version V3.3.0
* @date    27-Apr-2021
* @brief   Generic byte fifo implementation
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
/*******************************************************************************
 * Include Files
 */
#include <string.h>
#include "fifo.h"


/******************************************************************************/
/**
 * @brief Initialize the FIFO.
 *
 * @param  p_fifo  A pointer to a structure to maintain FIFO data.
 */
void fifo_init(fifo_t* p_fifo)
{
//    __disable_irq();

    if (p_fifo)
    {
        p_fifo->count = 0;
        p_fifo->front = 0;
        p_fifo->back  = 0;
        memset((void*)p_fifo->p_data, 0, p_fifo->size);
    }

//    __enable_irq();
}

/******************************************************************************/
/**
 * @brief Queue a byte at the back of the FIFO.
 *
 * @param  p_fifo  A pointer to the FIFO structure to queue data to.
 * @param  data  The data byte to add to the FIFO.
 */
 void fifo_queue(fifo_t* p_fifo, uint8_t data)
{
//    __disable_irq();

    p_fifo->p_data[p_fifo->back] = data;

    p_fifo->back++;
    p_fifo->back %= p_fifo->size;

    if (p_fifo->count < p_fifo->size)
    {
        p_fifo->count++;
    }
    else
    {
        p_fifo->front++;
        p_fifo->front %= p_fifo->size;
    }

//    __enable_irq();
}

/******************************************************************************/
/**
 * @brief Dequeue and return a pointer the next byte the front of the FIFO.
 *
 * @param  p_fifo  A pointer to the FIFO structure to dequeue data from.
 *
 * @return A pointer to the dequeued data, else 0 if none.
 */
 uint8_t fifo_dequeue(fifo_t* p_fifo, uint8_t* p_data)
{
    uint8_t result = 0;
    
//    __disable_irq();

    if (p_fifo->count)
    {
        *p_data = p_fifo->p_data[p_fifo->front];

        p_fifo->count--;
        p_fifo->front++;
        p_fifo->front %= p_fifo->size;
        result = 1;
    }

//    __enable_irq();

    return result;
}

/******************************************************************************/
/**
 * @brief Get the count of bytes remaining in the FIFO.
 *
 * @param  p_fifo  A pointer to the FIFO structure to get the size of.
 *
 * @return The count of bytes remaining in the FIFO.
 */
 uint16_t fifo_get_count(const fifo_t* p_fifo)
{
    return p_fifo->count;
}
