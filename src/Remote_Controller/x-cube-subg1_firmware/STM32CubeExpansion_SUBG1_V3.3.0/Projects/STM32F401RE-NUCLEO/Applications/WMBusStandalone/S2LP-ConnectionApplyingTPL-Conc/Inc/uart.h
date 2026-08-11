/**
******************************************************************************
* @file    uart.h
* @author  SRA-NOIDA 
* @version V3.3.0
* @date    27-Apr-2021
* @brief   Header file for uart.c
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

#ifndef __UART_H__
#define __UART_H__

/*******************************************************************************
 * Include Files
 */
#include <stdint.h>
#include "fifo.h"


#ifdef ENABLE_VCOM
#define PRINTF(...)             printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

   
#ifdef IAR
#include <yfuns.h>
#else
   //#define size_t int
#define _LLIO_STDIN  0
#define _LLIO_STDOUT 1
#define _LLIO_STDERR 2
#define _LLIO_ERROR ((size_t)-1) /* For __read and __write. */
#endif
   
#ifdef STM32L053xx  
#define  TERMINAL_BUFF_SIZE                     600 //kg
#else
#define  TERMINAL_BUFF_SIZE                   	600                   
#endif

#define NUCLEO_UARTx_RX_QUEUE_SIZE              (1224)
//#define NUCLEO_UARTx_TX_QUEUE_SIZE              (3*1024)  


#define RECEIVE_QUEUE_SIZE              NUCLEO_UARTx_RX_QUEUE_SIZE



#define MAX_ECHO_BUFFER_SIZE            100
#define RX_FIFO_SIZE                    256
#define TX_FIFO_SIZE                    256
                                  
#define USARTx                           USART2
   
#define UART_TX_PORT                     GPIOA
#define USARTx_TX_PIN                    GPIO_PIN_2                
#define USARTx_TX_GPIO_PORT              GPIOA                       
#define USARTx_TX_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define USARTx_TX_SOURCE                 GPIO_PinSource2

#ifdef USE_STM32F4XX_NUCLEO
#define USARTx_TX_AF                     GPIO_AF7_USART2
#endif

#ifdef USE_STM32L1XX_NUCLEO
#define USARTx_TX_AF                     GPIO_AF7_USART2
#endif

#ifdef USE_STM32L0XX_NUCLEO
#define USARTx_TX_AF                     GPIO_AF4_USART2
#endif


#define UART_RX_PORT                     GPIOA
#define USARTx_RX_PIN                    GPIO_PIN_3                
#define USARTx_RX_GPIO_PORT              GPIOA                    
#define USARTx_RX_GPIO_CLK               RCC_AHBPeriph_GPIOA
#define USARTx_RX_SOURCE                 GPIO_PinSource3

#ifdef USE_STM32F4XX_NUCLEO
#define USARTx_RX_AF                     GPIO_AF7_USART2
#endif

#ifdef USE_STM32L1XX_NUCLEO
#define USARTx_RX_AF                     GPIO_AF7_USART2
#endif

#ifdef USE_STM32L0XX_NUCLEO
#define USARTx_RX_AF                     GPIO_AF4_USART2
#endif

   
   
#define USARTx_IRQn                      USART2_IRQn 
#define USARTx_IRQHandler                USART2_IRQHandler
   



#ifdef USE_STM32F4XX_NUCLEO
/* Definition for USARTx's DMA */
#define USARTx_TX_DMA_CHANNEL             DMA1_Stream6
#define USARTx_RX_DMA_CHANNEL             DMA1_Stream5
   
#define USARTx_DMA_CHANNEL                DMA_CHANNEL_4   

/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn                DMA1_Stream6_IRQn
#define USARTx_DMA_RX_IRQn                DMA1_Stream5_IRQn
#define USARTx_DMA_TX_IRQHandler          DMA1_Stream6_IRQHandler
#define USARTx_DMA_RX_IRQHandler          DMA1_Stream5_IRQHandler

#endif

#ifdef USE_STM32L1XX_NUCLEO
/* Definition for USARTx's DMA */
#define USARTx_TX_DMA_CHANNEL             DMA1_Channel7
#define USARTx_RX_DMA_CHANNEL             DMA1_Channel6

/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn                DMA1_Channel7_IRQn
#define USARTx_DMA_RX_IRQn                DMA1_Channel6_IRQn
#define USARTx_DMA_TX_IRQHandler          DMA1_Channel7_IRQHandler
#define USARTx_DMA_RX_IRQHandler          DMA1_Channel6_IRQHandler

#endif

#ifdef USE_STM32L0XX_NUCLEO
/* Definition for USARTx's DMA */
#define USARTx_TX_DMA_CHANNEL             DMA1_Channel4
#define USARTx_RX_DMA_CHANNEL             DMA1_Channel5

/* Definition for USARTx's NVIC */
#define USARTx_DMA_TX_IRQn                DMA1_Channel4_5_6_7_IRQn
#define USARTx_DMA_RX_IRQn                DMA1_Channel4_5_6_7_IRQn
#define USARTx_DMA_TX_RX_IRQHandler       DMA1_Channel4_5_6_7_IRQHandler

#endif




#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE() 

#define USARTx_FORCE_RESET()             __HAL_RCC_USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __HAL_RCC_USART2_RELEASE_RESET()
   
/*******************************************************************************
 * Functions
 */
fifo_t* uart_get_rx_fifo(void);
void uart_init(uint32_t baud_rate);
void uart_set_baud_rate(uint32_t baud_rate);
void uart_sendDMA(uint8_t* p_buf, uint16_t len);
void USART1_HandlerUser(void);
void SendTimeStamp(void);
void SendDataToTerminal(void);

#ifdef __CC_ARM
int fputc(int ch, FILE *f);
#endif

#ifdef __GNUC__
int _execve(char *name, char **argv, char **env);
uint8_t __io_getcharNonBlocking(uint8_t *data);
int __io_putchar(int ch);
int __io_getchar(void);
void __io_flush( void );

#else
/* IAR Standard library hook for serial output  */
size_t __write(int handle, const unsigned char * buffer, size_t size);
void __io_putchar( char c );
size_t __read(int handle, unsigned char * buffer, size_t size);
int __io_getchar(void);
void __io_flush( void );
uint8_t __io_getcharNonBlocking(uint8_t *data);
#endif

#ifdef __GNUC__
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

#else
void __io_putchar( char c );
#endif

#ifdef __GNUC__
  /* With GCC, small printf (option LD Linker->Libraries->Small printf*/
 int __io_putchar(int ch);
#else
void __io_putchar( char c );
#endif
int __io_getchar(void);
void __io_flush( void );

#endif
