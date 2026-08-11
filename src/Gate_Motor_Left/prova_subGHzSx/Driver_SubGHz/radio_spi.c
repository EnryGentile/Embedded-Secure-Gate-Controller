/**
******************************************************************************
* @file    radio_spi.c
* @author  Central Labs
* @version V1.0.0
* @date    18-Apr-2018
* @brief   This file provides code for the configuration of the SPI instances.                     
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT(c) 2018 STMicroelectronics</center></h2>
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
*     without specific prior written permission.
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
#include "radio_spi.h"
#include "main.h"                  // Importa i nomi dei pin (es. CSN_Pin)
extern SPI_HandleTypeDef hspi1;    // Dice al file: "Guarda che hspi1 esiste già nel main!"
/**
* @addtogroup BSP
* @{
*/


/**
* @addtogroup X-NUCLEO-IDS01Ax
* @{
*/


/**
* @defgroup RADIO_SPI_Private_TypesDefinitions       RADIO_SPI Private Types Definitions
* @{
*/

/**
* @}
*/


/**
* @defgroup RADIO_SPI_Private_Defines                RADIO_SPI Private Defines
* @{
*/

/**
* @}
*/


/**
* @defgroup RADIO_SPI_Private_Macros                 RADIO_SPI Private Macros
* @{
*/

/**
* @}
*/


/**
* @defgroup RADIO_SPI_Private_Variables              RADIO_SPI Private Variables
* @{
*/
SPI_HandleTypeDef pSpiHandle;
/**
* @}
*/


/**
* @defgroup RADIO_SPI_Private_FunctionPrototypes     RADIO_SPI Private Function Prototypes
* @{
*/
void RadioSpiInit(void);
/* void HAL_SPI_MspDeInit(SPI_HandleTypeDef* pSpiHandle); */
static void SPI_Error(void);
StatusBytes RadioSpiWriteRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes RadioSpiReadRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes RadioSpiCommandStrobes(uint8_t cCommandCode);
StatusBytes RadioSpiWriteFifo(uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes RadioSpiReadFifo(uint8_t cNbBytes, uint8_t* pcBuffer);

/**
* @}
*/


/**
* @defgroup RADIO_SPI_Private_Functions              RADIO_SPI Private Functions
* @{
*/


/**
* @brief  Initializes SPI HAL.
* @param  None
* @retval None
*/
void RadioSpiInit(void)
{
  if (HAL_SPI_GetState(&pSpiHandle) == HAL_SPI_STATE_RESET)
  {
    /* SPI Config */
    pSpiHandle.Instance               = RADIO_SPI;
    pSpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    pSpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    pSpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    pSpiHandle.Init.CLKPolarity       = SPI_POLARITY_LOW;
    pSpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
    pSpiHandle.Init.CRCPolynomial     = 7;
    pSpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    pSpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    pSpiHandle.Init.NSS               = SPI_NSS_SOFT;       
    pSpiHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
    pSpiHandle.Init.Mode              = SPI_MODE_MASTER;
    
    //HAL_SPI_MspInit(&pSpiHandle);
    HAL_SPI_Init(&pSpiHandle);
  }
}



/**
* @}
*/


/**
* @brief  SPI Write a byte to device
* @param  Value: value to be written
* @retval None
*/


/**
* @brief  SPI error treatment function
* @param  None
* @retval None
*/
static void SPI_Error(void)
{
  /* De-initialize the SPI communication BUS */
  HAL_SPI_DeInit(&pSpiHandle);
  
  /* Re-Initiaize the SPI communication BUS */
  RadioSpiInit();
}


/**
* @brief  Write single or multiple RF Transceivers register
* @param  cRegAddress: base register's address to be write
* @param  cNbBytes: number of registers and bytes to be write
* @param  pcBuffer: pointer to the buffer of values have to be written into registers
* @retval StatusBytes
*/
StatusBytes RadioSpiWriteRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer)
{
	__HAL_SPI_CLEAR_OVRFLAG(&hspi1);
	// Questo resetta l'errore se la memoria si è intasata
  uint8_t aHeader[2] = {0};
  uint16_t tmpstatus = 0x0000;
  StatusBytes *pStatus=(StatusBytes *)&tmpstatus;
  
  uint8_t dummy = 0x00; // Il nostro cestino per svuotare il tubo!
  
  aHeader[0] = WRITE_HEADER;
  aHeader[1] = cRegAddress;
  
  //SPI_ENTER_CRITICAL();
  
  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_RESET);
  
  //sostituisco a tutti i HAL_MAX_DELAY 1000 (ultimo parametro della trasmitReceive)

  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&aHeader[0], (uint8_t *)&(tmpstatus), 1, HAL_MAX_DELAY);
  tmpstatus = tmpstatus << 8;  
  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&aHeader[1], (uint8_t *)&tmpstatus, 1, HAL_MAX_DELAY);
  
  for (int index = 0; index < cNbBytes; index++)
  {
    // Usiamo TransmitReceive invece di Transmit per evitare l'Overrun Error!
    HAL_SPI_TransmitReceive(&hspi1, &pcBuffer[index], &dummy, 1, HAL_MAX_DELAY);
  }
  
  while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) == SET);
  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_SET);
  
  //SPI_EXIT_CRITICAL();
  
  return *pStatus;
}


/**
* @brief  Read single or multiple SPIRIT1 register
* @param  cRegAddress: base register's address to be read
* @param  cNbBytes: number of registers and bytes to be read
* @param  pcBuffer: pointer to the buffer of registers' values read
* @retval StatusBytes
*/
StatusBytes RadioSpiReadRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer)
{
	__HAL_SPI_CLEAR_OVRFLAG(&hspi1);
	// Questo resetta l'errore se la memoria si è intasata
  uint16_t tmpstatus = 0x00;
  StatusBytes *pStatus = (StatusBytes *)&tmpstatus;
  
  uint8_t aHeader[2] = {0};
  uint8_t dummy = 0xFF;
  
  aHeader[0] = READ_HEADER;
  aHeader[1] = cRegAddress;
  
  //SPI_ENTER_CRITICAL();
  
  /* 1. Abbasso il Chip Select per iniziare */
  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_RESET);
  

  //sostituisco a tutti i HAL_DELAY_MAX 1000 (ultimo parametro della trasmitReceive)
  /* 2. Invio l'intestazione e leggo lo stato */
  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&aHeader[0], (uint8_t *)&(tmpstatus), 1, 1000);
  tmpstatus = tmpstatus << 8;  
  
  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&aHeader[1], (uint8_t *)&tmpstatus, 1, 1000);
  
  /* 3. Leggo i dati richiesti scambiandoli con byte fittizi (dummy) */
  for (int index = 0; index < cNbBytes; index++)
  { 
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&dummy, (uint8_t *)&(pcBuffer)[index], 1, 1000);
  } 
  
  /* 4. Aspetto che la SPI sia libera */
  while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) == SET);
  
  /* 5. Alzo il Chip Select */
  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_SET);
  
  //SPI_EXIT_CRITICAL();
  
  return *pStatus;
}


/**
* @brief  Send a command
* @param  cCommandCode: command code to be sent
* @retval StatusBytes
*/
StatusBytes RadioSpiCommandStrobes(uint8_t cCommandCode)
{
  uint8_t aHeader[2] = {0};
  uint16_t tmpstatus = 0x0000;
  StatusBytes *pStatus=(StatusBytes *)&tmpstatus;

  aHeader[0] = COMMAND_HEADER;
  aHeader[1] = cCommandCode;

  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_RESET);
  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&aHeader[0], (uint8_t *)&(tmpstatus), 1, HAL_MAX_DELAY);
  tmpstatus = tmpstatus << 8;
  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&aHeader[1], (uint8_t *)&tmpstatus, 1, HAL_MAX_DELAY);
  while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) == SET);
  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_SET);

  return *pStatus;
}


/**
* @brief  Write data into TX FIFO
* @param  cNbBytes: number of bytes to be written into TX FIFO
* @param  pcBuffer: pointer to data to write
* @retval StatusBytes
*/
StatusBytes RadioSpiWriteFifo(uint8_t cNbBytes, uint8_t* pcBuffer)
{
  uint8_t aHeader[2] = {0};
  uint16_t tmpstatus = 0x0000;
  StatusBytes *pStatus=(StatusBytes *)&tmpstatus;

  uint8_t dummy = 0x00; // Il cestino

  aHeader[0] = WRITE_HEADER;
  aHeader[1] = LINEAR_FIFO_ADDRESS;

  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_RESET);

  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&aHeader[0], (uint8_t *)&(tmpstatus), 1, HAL_MAX_DELAY);
  tmpstatus = tmpstatus << 8;
  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&aHeader[1], (uint8_t *)&tmpstatus, 1, HAL_MAX_DELAY);

  for (int index = 0; index < cNbBytes; index++) {
    // Usiamo TransmitReceive
    HAL_SPI_TransmitReceive(&hspi1, &pcBuffer[index], &dummy, 1, HAL_MAX_DELAY);
  }

  while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) == SET);
  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_SET);

  return *pStatus;
}

/**
* @brief  Read data from RX FIFO
* @param  cNbBytes: number of bytes to read from RX FIFO
* @param  pcBuffer: pointer to data read from RX FIFO
* @retval StatusBytes
*/
StatusBytes RadioSpiReadFifo(uint8_t cNbBytes, uint8_t* pcBuffer)
{
  uint16_t tmpstatus = 0x00;
  StatusBytes *pStatus = (StatusBytes *)&tmpstatus;
  uint8_t aHeader[2] = {0};
  uint8_t dummy = 0xFF;

  aHeader[0] = READ_HEADER;
  aHeader[1] = LINEAR_FIFO_ADDRESS;

  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_RESET);
  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&aHeader[0], (uint8_t *)&(tmpstatus), 1, HAL_MAX_DELAY);
  tmpstatus = tmpstatus << 8;
  HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&aHeader[1], (uint8_t *)&tmpstatus, 1, HAL_MAX_DELAY);

  for (int index = 0; index < cNbBytes; index++) {
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)&dummy, (uint8_t *)&(pcBuffer)[index], 1, HAL_MAX_DELAY);
  }
  while (__HAL_SPI_GET_FLAG(&hspi1, SPI_FLAG_BSY) == SET);
  HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_SET);

  return *pStatus;
}


/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
