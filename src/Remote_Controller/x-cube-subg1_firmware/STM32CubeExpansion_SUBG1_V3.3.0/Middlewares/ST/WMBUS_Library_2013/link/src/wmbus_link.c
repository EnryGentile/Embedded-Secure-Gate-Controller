/**
* @file    wmbus_link.c
* @author  SRA - NOIDA / RF application group - AMG
* @version V2.5.1
* @date    14-Sept-2020
* @brief   This file contains routines for LINK LAYER
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "wmbus_link.h"
#include "wmbus_linkcommand.h"
#include "wmbus_tim.h"
#include "wmbus_crc.h"
#include "uart.h"
#include "command.h"
#include "S2LP_Types.h"
#include "user_appli.h"


/** @addtogroup WMBUS_Library
* @{
*/

#ifdef USE_STM32F4XX_NUCLEO
#include "stm32f4xx.h"
#endif

#ifdef USE_STM32L1XX_NUCLEO
#include "stm32l1xx.h"
#endif

#ifdef USE_STM32L0XX_NUCLEO
#include "stm32l0xx.h"
#endif

/** @addtogroup WMBus_LinkLayer
*   @{
*/

/** @addtogroup WMBus_Link
*   @{
*/

/** @defgroup WMBus_Link_Private_Macros    WMBus Link Private Macros
* @{
*/
#define IS_WMBUS_LINK_ATTR(attr)  (((attr) >= LINK_ATTR_ID_START)&&\
((attr) <= LINK_ATTR_ID_END))

#define  CALC_BUFFER_SIZE_FORMAT_A(len)           ((len)+1+(((len)+22)/16)*2)
#define  CALC_BUFFER_SIZE_FORMAT_B(len)           ((len+1)+((len/125)+1)*2)
/**
* @}
*/

/** @defgroup WMBus_Link_Private_Defines    WMBus Link Private Definitions
* @{
*/

#ifdef GCC
#define __weak                          __attribute__((weak))
#endif



#define LINK_USER_MAX_TX_FRAME_BUFFER_SIZE   310/*!<Maximum Txframe size including 1st block+2 block+crc bytes*/
#define LINK_USER_MAX_RX_FRAME_BUFFER_SIZE   310/*!<Maximum Rxframe size including 1st block+2 block+crc bytes*/
#define LINK_MAX_L_FIELD                     255/*!<Maximum L-Field size*/




#define LINK_LAYER_OFFSET                     9/*!<Link layer offset:(C + M + A)*/
#define CI_FIELD_OFFSET                       1/*!<CI-Field offset*/
#define FORMAT_A_SECOND_BLOCK_MAX_LEN         16/*!<2nd block max lenght for Frame format A*/
#define FORMAT_A_OPTIONAL_BLOCK_MAX_LEN       16/*!<Optional block max lenght for Frame format A*/
#define FORMAT_B_SECOND_BLOCK_MAX_LEN         116/*!<2nd block max lenght for Frame format B*/
#define FORMAT_B_OPTIONAL_BLOCK_MAX_DATA_LEN  126/*!<Optional block max lenght for Frame format B*/
#define LINK_LAYER_FIRST_BLOCK_LEN            10/*!<1st block lenght for Frame format A/B*/
#define FORMAT_A_REV_FIRST_BLOCK_LEN          12 /*!<Received 1st block max lenght for Frame format A:(L+C+M+A+2bytes crc)*/
#define FORMAT_B_FIRST_BLOCK_LEN              10 /*!<Received 1st block max lenght for Frame format B:(L+C+M+A)*/


#define CI_DEFAULT_CALLBACK             0x78 /* no header */

#define DELAY_GUARD_TIME_RX2TX                  dt_rx_tx
#define DELAY_GUARD_TIME_TX2RX                  dt_tx_rx


/**
* @}
*/

/** @defgroup  WMBus_Link_Private_Variables  WMBus Link Private Variables
* @{
*/
volatile uint8_t TxCompleted = 0, RxTimeout = 0;

/** The link layer Tx Frame Buffer points to the linear buffer where the
frame with all computed L-field and CRC for each block is contained and is ready
to TX. This buffer is physically stored in the phy layer because, according
to the sub-mode, it can add preamble and sync before the payload. */
static uint8_t* LinkTxFrameBuffer;

/** TX frame buffer size */
static uint16_t LinkTxFrameBufferSize;                                            

/** Switching delay time variables */
uint32_t dt_rx_tx=3,dt_tx_rx=3;

/** RX frame buffer size */
static uint16_t LinkRxFrameBufferSize;                                      

static uint8_t LinkRxFrameBuffer[LINK_USER_MAX_RX_FRAME_BUFFER_SIZE];       

/** Relevant events timestamp */
static uint32_t TxCompletedTimeStamp=0, 
          RxCompletedTimeStamp=0, 
          FacTimestamp=0;/*RxTimeoutTimeStamp*/

/* Link layer frame struct. 2 variables, on e for Rx and one for TX are instanced */
/*static*/ Frame_t RxFrame = {0};                                               
Frame_t TxFrame = {0};

/* The actual state running */
volatile LL_State_t LL_State = LL_SM_IDLE;

/* The actual state running for FAC */
volatile LL_State_t LL_State_FAC = LL_SM_IDLE;

/* Link Layer attributes initialization */
WMBusLinkAttributes_t  LinkAttr=
{
  .DevType = METER,
  .MField = {0,0},
  .AField = {0,0,0,0,0,0},
  .LinkFrameFormat = FRAME_FORMAT_A,
  .LinkFreqAccessCycle = 0,
  .LinkTimeout = 3000,
};

/*Initialization of appli attributes used by link layer */
WMBusLinkAppliParams_t LinkAppliParams =
{
  .LinkEllLength = 0x00,
  .LinkHeaderLength =0x00, 
  .LinkManufrID =0x0000,
  .LinkDeviceID=0x00000000,
  .LinkDeviceVersion=0x00,
  .LinkMeterType=0x00,
  .LinkAccessNumber=0x00, 
  .LinkStatusField=0x00,
  .LinkConfigWord=0x0000,
  .LinkTimeout=0x00000000,
  .LinkFreqAccessCycle = 0,
  .LinkEllType = ELL_0,
  .LinkEllAccessNum =0x00 
};


#ifdef ENABLE_VCOM                                                
uint8_t TempUartDataBuff[600];
extern uint32_t SysTickTimeStamp;
extern fifo_t  TerminalFifo;    
#endif

/**
* @}
*/

/** @defgroup WMBus_Link_Private_FunctionPrototypes   WMBus Link Private Function Prototypes
* @{
*/
void WMBus_LinkCmdReceived_SM_Meter(Frame_t *pRxframe);
void WMBus_LinkCmdReceived_SM_Other(Frame_t *pRxframe);
static void WMBus_LinkLayer_SM_Meter(void);
static void WMBus_LinkLayer_SM_Other(void);
LINK_STATUS_t WMBus_LinkGetRxFrameBuffer(Frame_t *frame);
LINK_STATUS_t WMBus_LinkSetTxFrameBuffer(Frame_t *frame);
LINK_STATUS_t WMBus_LinkTransmitFrameHandlerMode(Frame_t *frame);
void WMBus_LinkTxDataSentStateHandler(uint8_t val);

/**
* @}
*/

/** @defgroup    WMBus_Link_Private_Functions      WMBus Link Private Functions
* @{
*/

/* For configuring different delay depending WMBUS Modes (T,S,C) */
extern CfgParam_Typedef CurrCfg;
/* For configuring different delay depending WMBUS Modes (T,S,C) */

/**
* @brief  This function performs the overhead establishment
* @param  None
* @retval None
*/
void WMBus_LinkOverHeadEst(void)
{
  uint32_t t0;
  /* need to estimate overall overhead (including LL) */
  /* case Other device & T2 mode : 1ms */
  
  /* Configure different delay depending WMBUS Modes (T,S,C) */ 
  uint16_t tmpDelay = 0;
  /*Need to optimize based on mode selected : T2-mode*/  
  if (CurrCfg.WorkingMode == T2_MODE)
    tmpDelay = 15;
  else if (CurrCfg.WorkingMode == S2_MODE)
    /*Need to optimize based on mode selected : S2-mode*/
    tmpDelay = 8;
  else if (CurrCfg.WorkingMode == C2_MODE)
    /*Need to optimize based on mode selected : C2-mode*/
    tmpDelay = 8;
  else
    tmpDelay = 0;
  /* different delay depending WMBUS Modes (T,S,C) */
  
  
  /* we don't want really set the device in RX or TX, just 
  want to estimate the time to prepare RX or TX.
  So tell the Phy layer to not execute cmd */
  WMBus_PhyDeviceCmd(0);
  
  t0 = WMBus_TimGetTimestamp();
  
  /* Estimate Tx overhead with minimum Tx bytes in Fifo - use NUM_BYTE_FIRST_FIFO_WRITE define to be aligned */
  WMBus_PhyTxDataHandleMode (LinkTxFrameBuffer,12, WMBus_LinkTxDataSentStateHandler);
  
  dt_rx_tx= WMBus_TimDiffFromNow(t0) + tmpDelay;
  
  /* Estimate Tx overhead with minimum Tx bytes in Fifo - use NUM_BYTE_FIRST_FIFO_WRITE define to be aligned */
  t0 = WMBus_TimGetTimestamp();
  WMBus_PhySetState (PHY_STATE_RX);
  
  dt_tx_rx= WMBus_TimDiffFromNow(t0);
  
  /*All done, the phy should execute dev cmd again */
  WMBus_PhyDeviceCmd(1);
  
  if(LinkAttr.DevType==METER)
  {
    WMBus_PhySetState (PHY_STATE_READY);
  }
  
  TxCompleted = 0;
  RxTimeout = 0;
  TxCompletedTimeStamp = 0;
  
}


/**
* @brief  This function Initialize the link layer
* @param  a_field: A Field of the Frame
* @param  dev_type: Device type
* @param  frame_format: Frame Fomrat
* @Retval Link Status: The Status of the link layer initialization
*/
LINK_STATUS_t WMBus_LinkInit(uint8_t* m_field,
                             uint8_t* a_field,
                             WMBusDeviceType_t dev_type,
                             FrameFormatType_t frame_format)
{
  /* the real memory for the last received M and A fields*/
  static uint8_t RxAField[6];
  static uint8_t RxMField[2];
  
  LinkAttr.DevType = dev_type,
  memcpy(LinkAttr.MField,m_field,2);
  memcpy(LinkAttr.AField,a_field,6);

  LinkAttr.LinkFrameFormat = frame_format;
  LinkAttr.LinkTimeout = 1000;
  LinkAttr.LinkFreqAccessCycle = 0;
  
  /* RxFrame.m_field point to the RxMField and same for A. Arrays allocated here */
  RxFrame.m_field=RxMField;
  RxFrame.a_field=RxAField;
  
  /* For tx frame they should point to the fields of LinkAttr */
  TxFrame.m_field=LinkAttr.MField;
  TxFrame.a_field=LinkAttr.AField;
  
  /* Point the real phy buffer */
  LinkTxFrameBuffer=GetPhyBuffer();
  
  /* Link layer init, calls also the Timer module init function
  (implemented by the user according to its implementation) */
  WMBus_TimInit();
  
  /* The LL SM will be in idle state */
  LL_State = LL_SM_IDLE;
  
  return LINK_STATUS_SUCCESS;
}

LINK_STATUS_t WMBus_LinkInit_Test_Mode(uint8_t* m_field,
                             uint8_t* a_field,
                             WMBusDeviceType_t dev_type,
                             FrameFormatType_t frame_format)
{
  /* the real memory for the last received M and A fields*/
  static uint8_t RxAField[6];
  static uint8_t RxMField[2];
  
  LinkAttr.DevType = dev_type,
  memcpy(LinkAttr.MField,m_field,2);
  memcpy(LinkAttr.AField,a_field,6);

  LinkAttr.LinkFrameFormat = frame_format;
  LinkAttr.LinkTimeout = 1000;
  LinkAttr.LinkFreqAccessCycle = 0;
  
  /* RxFrame.m_field point to the RxMField and same for A. Arrays allocated here */
  RxFrame.m_field=RxMField;
  RxFrame.a_field=RxAField;
  
  /* For tx frame they should point to the fields of LinkAttr */
  TxFrame.m_field=LinkAttr.MField;
  TxFrame.a_field=LinkAttr.AField;
  
  /* Point the real phy buffer */
  LinkTxFrameBuffer=GetPhyBuffer();
  
  /* Link layer init, calls also the Timer module init function
  (implemented by the user according to its implementation) */
  WMBus_TimInit();
  
  /* The LL SM will be in idle state */
  LL_State = LL_SM_TEST_MODE;
  
  return LINK_STATUS_SUCCESS;
}


/**
* @brief  This function performs the Link RESET
* @param  None
* @Retval Status of the Link RESET
*/
LINK_STATUS_t WMBus_LinkReset(void)
{
  return LINK_STATUS_SUCCESS;
}

/**
* @brief  This function Sets the Radio to POWER ON state 
* @param  None
* @Retval Status of the Power ON
*/
LINK_STATUS_t WMBus_LinkRadioPowerOn(void)
{
  WMBus_PhySetState (PHY_STATE_READY);
  return LINK_STATUS_SUCCESS;
}

/**
* @brief  This function Sets the Radio to POWER OFF state 
* @param  None
* @Retval Status of the Power OFF
*/
LINK_STATUS_t WMBus_LinkRadioPowerOff(void)
{
  if(LINK_STATUS_SUCCESS ==  WMBus_PhySetState (PHY_STATE_SHUTDOWN))
  {
    return LINK_STATUS_SUCCESS;
  }
  else
  {
    return LINK_STATUS_PHY_ERROR;
  }
}

/**
* @brief  This function Sets the Radio to STANDBY MODE  
* @param  None
* @Retval Status of the operation performed
*/
LINK_STATUS_t WMBus_LinkRadioStandby(void)
{
  if(LINK_STATUS_SUCCESS ==  WMBus_PhySetState (PHY_STATE_STANDBY))
  {
    return LINK_STATUS_SUCCESS;
  }
  else
  {
    return LINK_STATUS_PHY_ERROR;
  }
}

/**
* @brief  This function Sets the Tx Frame Buffer 
* @param  None
* @Retval Status of the operation performed
*/
LINK_STATUS_t WMBus_LinkSetTxFrameBuffer(Frame_t *frame)
{
  uint8_t *pStartBlock;/*Points to beginning of each block for CRC calculation*/
  uint8_t *pFrame;
  uint8_t *pData;
  uint8_t dataBytesRemaining;  
  uint8_t blockDataSize; 
  uint8_t l_field;
  
  pFrame                  = LinkTxFrameBuffer;
  pData                   = frame->data_field.payload;
  dataBytesRemaining      = frame->data_field.size;
  LinkTxFrameBufferSize   = 0;
  
  /* Assemble first block */
  pStartBlock = pFrame;
  
  if(FRAME_FORMAT_B == LinkAttr.LinkFrameFormat) /**FRAME FORMAT B**/
  {
    
    /* Format B, L-Field: 
    All subsequent bytes including crc bytes (Excluding L-field)*/
    
    l_field = LINK_LAYER_OFFSET + CI_FIELD_OFFSET + frame->data_field.size; 
    
    l_field += ((l_field>125)?4:2);  /**Add CRC Length**/
    
    /**MAX Frame length in FORMAT B(Including CRC) is 255 (Exluding L-Field)**/
    //if(l_field <= LINK_MAX_L_FIELD) /* commented out because always true */
    {
      
      /************** Assemble 1st block ***************/  
      *pFrame++ = l_field;
      
      /*Copy C, M , A Fields*/
      *pFrame++=frame->c_field;
      memcpy(pFrame,LinkAttr.MField,2);
      pFrame+=2;
      memcpy(pFrame,LinkAttr.AField,6);
      pFrame+=6;
      /******************End of 1st Block*******************/ 
      
      /************** Assemble second block ***************/     
      /* Control information field (CI)  */
      *pFrame++ = frame->ci_field;
      
            
      /* Copy 0-115 bytes of data in the second block */
      blockDataSize = L_MIN(dataBytesRemaining, \
        (FORMAT_B_SECOND_BLOCK_MAX_LEN-CI_FIELD_OFFSET));
      
      if(blockDataSize)
      {
        /* Data-field (Data) - Application payload */
        memcpy(pFrame, pData, blockDataSize);
        pFrame   += blockDataSize;
        pData    += blockDataSize;
      }
      AppendCRC(pStartBlock, pFrame);
      pFrame += 2;
      /******************End of 2nd Block******************/ 
      
      /**************Start of Optional Block(If Any)**************/ 
      
      /* Copied blockDataSize bytes into the data-field */
      dataBytesRemaining -= blockDataSize;
      
      /* Assemble optional blocks */  
      if(dataBytesRemaining)
      {
        /* Point to beginning of the Optional block for CRC*/
        pStartBlock = pFrame;   
        
        /* Copy 0-126 bytes of data into the next block */
        blockDataSize = L_MIN(dataBytesRemaining,
                            FORMAT_B_OPTIONAL_BLOCK_MAX_DATA_LEN);
        
        /* Data-field (Data) - Application payload */
        memcpy(pFrame, pData, blockDataSize);
        pFrame   += blockDataSize;
        AppendCRC(pStartBlock, pFrame);
        pFrame += 2;
      }
      /******************End of Optional Block******************/ 
      
      /* Calculate frame buffer size */
      LinkTxFrameBufferSize = pFrame - LinkTxFrameBuffer;
    }
        
  }
  else  /*******************FRAME FORMAT A********************/
  {
    
    /*Length-field (L):(C + M + A) + (CI + all subsequent bytes
    (excluding crc)*/ 
    l_field = LINK_LAYER_OFFSET  + CI_FIELD_OFFSET + frame->data_field.size ;
    
    /************ Assemble 1st Block *****************/  
    *pFrame++ = l_field;
    
    /*Copy C, M , A Fields*/
    *pFrame++=frame->c_field;
    memcpy(pFrame,LinkAttr.MField,2);
    pFrame+=2;
    memcpy(pFrame,LinkAttr.AField,6);
    pFrame+=6;
    
    /* After adding 2 byte CRC */
    AppendCRC(pStartBlock, pFrame);
    pFrame += 2;
    /****************************************************/
    
    /************** Assemble second block ***************/  
    
    /* Point to beginning of the 2nd block for CRC*/
    pStartBlock = pFrame;
    
    /* Application Control information field (CI)*/
    *pFrame++ = frame->ci_field;
    
    
    /* Copy 0-15 bytes of data in the second block */
    blockDataSize = L_MIN(dataBytesRemaining, \
      (FORMAT_A_SECOND_BLOCK_MAX_LEN-CI_FIELD_OFFSET));
    
    if(blockDataSize)
    {
      /* Data-field (Data) - Application payload */
      memcpy(pFrame, pData, blockDataSize);
      pFrame   += blockDataSize;
      pData    += blockDataSize;
    }
    AppendCRC(pStartBlock, pFrame);
    pFrame += 2;
    
    /* Copied blockDataSize bytes into the data-field */
    dataBytesRemaining -= blockDataSize;
    
    /* Assemble optional blocks */  
    while (dataBytesRemaining)
    {
      /* Point to beginning of the block for CRC*/
      pStartBlock = pFrame; 
      
      /* Copy 0-16 bytes of data into the next block */
      blockDataSize = L_MIN(dataBytesRemaining, FORMAT_A_OPTIONAL_BLOCK_MAX_LEN);
      
      /* Data-field (Data) - Application payload */
      memcpy(pFrame, pData, blockDataSize);
      pFrame   += blockDataSize;
      pData    += blockDataSize;
      dataBytesRemaining -= blockDataSize;
      AppendCRC(pStartBlock, pFrame);
      pFrame += 2;
    }
    
    /**************************************************/  
    
    /* Calculate frame buffer size */
    LinkTxFrameBufferSize = pFrame - LinkTxFrameBuffer;
  }
    
  return LINK_STATUS_SUCCESS;
}

/**
* @brief  This function handles the Frame Transmission 
* @param  None
* @Retval Status of the operation performed
*/
LINK_STATUS_t WMBus_LinkTransmitFrameHandlerMode(Frame_t *frame)
{
  LINK_STATUS_t status = LINK_STATUS_UNKNOWN_ERROR;
  
  /* frame has been already built in LinkTxFrameBuffer by the linkcommand fcns */
  
  status = (LINK_STATUS_t)WMBus_PhyTxDataHandleMode (LinkTxFrameBuffer,\
    LinkTxFrameBufferSize, WMBus_LinkTxDataSentStateHandler);
  
  return status;
}

/**
* @brief  This function is called once the transmission is complete 
* @param  val: Value
* @Retval None
*/
void WMBus_LinkTxDataSentStateHandler(uint8_t val)
{
  
#ifdef ENABLE_VCOM 
  uint16_t counter;
  char timeStampStr[10];
#endif
   
  TxCompletedTimeStamp = WMBus_TimGetTimestamp();
  TxCompleted = 1;
#ifdef ENABLE_VCOM  
      
  fifo_queue(&TerminalFifo, 'T');                                               
  fifo_queue(&TerminalFifo, 'x');
  fifo_queue(&TerminalFifo, ' ');
   
  HexToText(LinkTxFrameBuffer,TempUartDataBuff,LinkTxFrameBufferSize);
  
  for(counter = 0;counter<2*LinkTxFrameBufferSize;counter++)
  {
    fifo_queue(&TerminalFifo, TempUartDataBuff[counter]);
  }
  
  fifo_queue(&TerminalFifo, ' ');
  
  sprintf(timeStampStr,"%lu",SysTickTimeStamp);
  

  for(counter = 0;counter<strlen(timeStampStr);counter++)
  {
    fifo_queue(&TerminalFifo, timeStampStr[counter]);
  }
  
  fifo_queue(&TerminalFifo, '\r');
  fifo_queue(&TerminalFifo, '\n');
  
#endif
 
}

/**
* @brief  This function indicates the timeout condition 
* @param  None
* @Retval None
*/
void WMBus_LinkRxTimeoutHandler(void)
{
  uint8_t tmpFlag = 0;
  if(RxTimeOutRemaingCtr)
  {
    WMBus_S2LPTimerRxTimeoutUs(RxTimeOutRemaingCtr);
    WMBus_PhySetState(PHY_STATE_RX);
    tmpFlag = 1;
  }
  else
    tmpFlag = 0;
  
  
  if((tmpFlag == 0))
  {
    RxTimeout = 1;
  }
}


/**
* @brief  This function gets the Rx Frame Buffer 
* @param  Frame:Pointer to the Frame
* @Retval Status of the operation performed
*/
LINK_STATUS_t WMBus_LinkGetRxFrameBuffer(Frame_t *frame)
{
#ifdef ENABLE_VCOM
  uint16_t counter;
  char timeStampStr[10];
#endif
  uint8_t   *pData; /* Points to the data-field  */ 
  uint8_t    blockSize;       /* Number of bytes in current block  */
  uint8_t    blockDataSize;   /* Number of data-field bytes in current block*/
  uint8_t    bytesRemaining;
  uint8_t    l_field;  
  pData    = frame->data_field.payload;
  uint8_t crc[2];
  
  if(FRAME_FORMAT_B == frame->frame_format)
  {
    uint8_t numCrcBytes=0;
    
    /* Parse first block */
    blockSize = FORMAT_B_FIRST_BLOCK_LEN;
    
    /* Make sure that there is enough data to parse the first block */
    if (LinkRxFrameBufferSize < blockSize)
    {
      return LINK_STATUS_FRAME_ERROR;
    }    
    
    
    /*Length-field (L) - Number of all subsequent user bytes (CRC included)*/ 
    l_field = WMBus_PhyPopBytefromQueue();
    crcAppend(l_field);
    
    /*Calculate crc bytes*/
    numCrcBytes =  ((l_field>127)?4:2);
    
    /* Validate frame size, Make sure length-field is at least the min value*/
    if (l_field < FORMAT_B_FIRST_BLOCK_LEN)
    {
      return LINK_STATUS_PHY_ERROR_RX_INVALID_LENGTH;
    }
    if (LinkRxFrameBufferSize > (LINK_MAX_L_FIELD+1))
    {
      return LINK_STATUS_PHY_ERROR_RX_INVALID_LENGTH;
    } 
    
    /*Copy C-field, M-field and A-field*/
    frame->c_field= WMBus_PhyPopBytefromQueue();
    crcAppend(frame->c_field);
    
    for(uint8_t i=0;i<2;i++)
    {
      frame->m_field[i]=WMBus_PhyPopBytefromQueue();
      crcAppend(frame->m_field[i]);
    }
          
    for(uint8_t i=0;i<6;i++)
    {
      frame->a_field[i]=WMBus_PhyPopBytefromQueue();
      crcAppend(frame->a_field[i]);
    }
    /*Copy Control information field: CI-field*/
    frame->ci_field = WMBus_PhyPopBytefromQueue();
    crcAppend(frame->ci_field);
    
    /*Data-field size = (L - 1st Block(C,M,A)-ell-ci-Header Bytes-crc bytes) */ 
    frame->data_field.size = l_field - LINK_LAYER_OFFSET - CI_FIELD_OFFSET - numCrcBytes;
    
    /* Number of bytes remaining in Data field excluding CI and CRC fields 
    (2nd block)*/
    bytesRemaining = frame->data_field.size;
    
    /* Parse second block  */
    blockDataSize  = L_MIN(bytesRemaining, FORMAT_B_SECOND_BLOCK_MAX_LEN - CI_FIELD_OFFSET);
    
    /* Second block size */
    blockSize      = FORMAT_B_FIRST_BLOCK_LEN + CI_FIELD_OFFSET+ blockDataSize;
    

    for(uint16_t i=0;i<blockDataSize;i++)
    {
      pData[i]=WMBus_PhyPopBytefromQueue();
      crcAppend(pData[i]);
    }
    for(uint16_t i=0;i<2;i++)
    {
      crc[i]=WMBus_PhyPopBytefromQueue();
    }
    
    pData    += blockDataSize;
    bytesRemaining -= blockDataSize;

    
    if (!crcVerify(crc))
    {
      return LINK_STATUS_CRC_ERROR;
    }
    
    /****** Parse Remaining Data Bytes in Optional Block ************/
    if (bytesRemaining!=0)
    {
      blockDataSize  = L_MIN(bytesRemaining,FORMAT_B_OPTIONAL_BLOCK_MAX_DATA_LEN);
      blockSize      = blockDataSize;

      for(uint16_t i=0;i<blockDataSize;i++)
      {
        pData[i]=WMBus_PhyPopBytefromQueue();
        crcAppend(pData[i]);
      }
      for(uint16_t i=0;i<2;i++)
      {
        crc[i]=WMBus_PhyPopBytefromQueue();
      }
      if (!crcVerify(crc))
      {
        return LINK_STATUS_CRC_ERROR;
      }
    } 
    
    for(int16_t i=0;i<LinkRxFrameBufferSize-(frame->data_field.size+1+LINK_LAYER_OFFSET+CI_FIELD_OFFSET+numCrcBytes);i++)
        WMBus_PhyPopBytefromQueue();
  }
  else
  {
    /* Parse first block */
    blockSize = FORMAT_A_REV_FIRST_BLOCK_LEN ;
    
    /* Make sure that there is enough data to parse the first block */
    if (LinkRxFrameBufferSize < blockSize)
    {
      return LINK_STATUS_FRAME_ERROR;
    } 
    
   
    /* Length-field (L) - Number of user data bytes, includes C, M, A, and CI
    excludes CRC bytes  Data-field size  - (L - 4-10) (4 bytes header) */
    l_field                 = WMBus_PhyPopBytefromQueue();
    crcAppend(l_field);
    
    frame->c_field= WMBus_PhyPopBytefromQueue();

    crcAppend(frame->c_field);
    
    for(uint8_t i=0;i<2;i++)
    {
      frame->m_field[i]=WMBus_PhyPopBytefromQueue();
      crcAppend(frame->m_field[i]);
    }
    
    for(uint8_t i=0;i<6;i++)
    {
      frame->a_field[i]=WMBus_PhyPopBytefromQueue();
      crcAppend(frame->a_field[i]);
    }
    for(uint16_t i=0;i<2;i++)
    {
      crc[i]=WMBus_PhyPopBytefromQueue();
    }
    
    if (!crcVerify(crc))
    {
      return LINK_STATUS_CRC_ERROR;
    }
    
    /* Validate frame size */
    /* Make sure length-field is at least the min value*/
    if (l_field < LINK_LAYER_FIRST_BLOCK_LEN)
    {
      return LINK_STATUS_PHY_ERROR_RX_INVALID_LENGTH;
    }
    if (LinkRxFrameBufferSize < CALC_BUFFER_SIZE_FORMAT_A(l_field))
    {
      return LINK_STATUS_FRAME_ERROR;
    }
            
    /* Actually pFrame points to the CI-field */
    frame->ci_field = WMBus_PhyPopBytefromQueue();
    crcAppend(frame->ci_field);
    
    frame->data_field.size  = l_field - LINK_LAYER_OFFSET - CI_FIELD_OFFSET;
    
    /* Number of bytes remaining in Data field excluding CI and CRC fields */
    bytesRemaining = frame->data_field.size;
    
    /* Parse second block  */
    blockDataSize  = L_MIN(bytesRemaining, FORMAT_A_SECOND_BLOCK_MAX_LEN-CI_FIELD_OFFSET);  
    
    /* Second block size */
    /*(header bytes + data + crc)*/
    blockSize = CI_FIELD_OFFSET + blockDataSize + 2;

    for(uint16_t i=0;i<blockDataSize;i++)
    {
      pData[i]=WMBus_PhyPopBytefromQueue();
      crcAppend(pData[i]);
    }
    for(uint16_t i=0;i<2;i++)
    {
      crc[i]=WMBus_PhyPopBytefromQueue();
    }
    if (!crcVerify(crc))
    {
      return LINK_STATUS_CRC_ERROR;
    }
    
    pData    += blockDataSize;
    bytesRemaining -= blockDataSize;

    
    /****** Parse optional blocks ************/
    while (bytesRemaining)
    {
      blockDataSize  = L_MIN(bytesRemaining, FORMAT_A_OPTIONAL_BLOCK_MAX_LEN);
      blockSize      = blockDataSize + 2;

      for(uint16_t i=0;i<blockDataSize;i++)
      {
        pData[i]=WMBus_PhyPopBytefromQueue();
        crcAppend(pData[i]);
      }
      for(uint16_t i=0;i<2;i++)
      {
        crc[i]=WMBus_PhyPopBytefromQueue();
      }
      if (!crcVerify(crc))
      {
        return LINK_STATUS_CRC_ERROR;
      }
      pData    += blockDataSize;
      bytesRemaining -= blockDataSize;
    }  
    
    for(int16_t i=0; i<LinkRxFrameBufferSize - CALC_BUFFER_SIZE_FORMAT_A(l_field) ;i++)
      WMBus_PhyPopBytefromQueue();
  }
   
#ifdef ENABLE_VCOM  

  fifo_queue(&TerminalFifo, 'R');                                               
  fifo_queue(&TerminalFifo, 'x');
  fifo_queue(&TerminalFifo, ' ');
   
  HexToText(LinkRxFrameBuffer,TempUartDataBuff,LinkRxFrameBufferSize);
  
  for(counter = 0;counter<2*LinkRxFrameBufferSize;counter++)
  {
    fifo_queue(&TerminalFifo, TempUartDataBuff[counter]);
  }
  
  fifo_queue(&TerminalFifo, ' ');
  
  sprintf(timeStampStr,"%lu",SysTickTimeStamp);
  

  for(counter = 0;counter<strlen(timeStampStr);counter++)
  {
    fifo_queue(&TerminalFifo, timeStampStr[counter]);
  }
  
  fifo_queue(&TerminalFifo, '\r');
  fifo_queue(&TerminalFifo, '\n');

#endif
  
  return LINK_STATUS_SUCCESS;
}

/**
* @brief  This function handles the Frame received
* @param  None
* @retval The Status of the Frame Parsing
*/
LINK_STATUS_t WMBus_LinkRxFrameHandler(void)
{
  static uint8_t DataBuff[255];
  LINK_STATUS_t status;
  
  RxFrame.data_field.payload = DataBuff;
  
  RxFrame.frame_format = WMBus_PhyGetPacketFormat(); 
  RxFrame.submode = WMBus_PhyGetRecvSubmode();
  LinkRxFrameBufferSize = WMBus_PhyGetPacketLength(); 
  RxFrame.timestamp = WMBus_PhyGetPacketRxTimestamp();
  RxFrame.RSSI_field = WMBus_PhyGetPacketRssi();
  RxCompletedTimeStamp = WMBus_TimGetTimestamp(); /*RxFrame.timestamp; */

  WMBus_PhyCopyBytefromQueue(LinkRxFrameBuffer, LinkRxFrameBufferSize);
  status=WMBus_LinkGetRxFrameBuffer(&RxFrame);
  WMBus_PhyPopDesc();

  return status;
}

/**
* @brief  This function returns the Frame Format
* @param  None
* @retval The Frame Format of the Frame
*/
FrameFormatType_t WMBus_LinkGetFormat(void)
{
  return LinkAttr.LinkFrameFormat;
}

/**
* @brief  This function will run the Link Layer State Machine 
*         for OTHER for received frame 
* @param  LinkRxframe: Received frame
* @retval None
*/
void WMBus_LinkCmdReceived_SM_Other(Frame_t* pRxframe)
{  
  switch(pRxframe->c_field & LINK_C_FIELD_MASK)
  {
  case LL_SND_NR:
    {   
      /* SND-NR is received*/      
      /*Give choice to User between SND-UD and SND-UD2*/
      WMBus_LinkSendNoReplyReceived_CB(pRxframe);
    }
    break;
  case LL_ACC_NR:
    {
      /* ACC-NR is received*/
      WMBus_LinkAccessNoReplyReceived_CB(pRxframe);
    }
    break;
  case LL_SND_IR:
    {
      /* SND-IR is received; Concentrator will send CNF-IR in response*/
      if(WMBus_PhyIsTwoWayMode())
      {
        WMBus_LinkInstallReqReceived_CB(pRxframe);
      }
      /* manage unidirectional mode */
      else
      {
      WMBus_LinkRxCompleted_SNDIR_CB();
      }
    }
    break;
  case LL_ACC_DMD:
    {
      /* ACC-DMD is received; response to this request is confirmed by an ACK*/
      WMBus_LinkAccessDemandReceived_CB(pRxframe);
    }
    break;    
  case LL_RSP_UD:
    {
      /* RSP-UD is received*/ 
      if(WMBus_PhyIsTwoWayMode())
      {
        LL_State=LL_SM_IDLE;
        WMBus_LinkResponseUdReceived_CB(pRxframe);
      }       
    }    
    break;
  case LL_ACK:
    {
      LL_State=LL_SM_IDLE;
      WMBus_LinkAckReceived_CB(pRxframe);
      break;
    }     
  default:
    break;
  }
}

/**
* @brief  This function will run the Link Layer State Machine for OTHER 
* @param  None
* @retval None
*/
static void WMBus_LinkLayer_SM_Other(void)
{ 
  static uint8_t tx_req = 0;
  
  
  switch(LL_State)
  {
  case LL_SM_IDLE:
      if(WMBus_PhyGetNumRxPackets()>0)
      {
        if(WMBus_LinkRxFrameHandler()==LINK_STATUS_SUCCESS)
        {    
          
          /* If we are here we can process the arrived frame that has been parsed 
          and it is now stored in the variable RxFrame */       
          WMBus_LinkCmdReceived_SM_Other(&RxFrame);
          WMBus_PhyResetPreambSyncFlag();
        } 

        WMBus_PhySetState(PHY_STATE_RX); 

      }
    break; 
    
  case LL_SM_SEND_CNF_IR:    
    if(!tx_req)
    {
      if(TimeDelay(RxCompletedTimeStamp,WMBus_PhyGetResponseTimeMin() - DELAY_GUARD_TIME_RX2TX))
      {

        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }
    }
    else if(TxCompleted)
    {
      /* Reset the TxCompleted flag for the next time */
      TxCompleted  = 0;
      /* Reset the tx_req flag for the next time */
      tx_req = 0;
      /* Set the phy state to RX mode*/
      WMBus_PhySetState(PHY_STATE_RX); 
      /* The new state for the LL SM will be idle state */
      LL_State = LL_SM_IDLE; 
      WMBus_LinkTxCompleted_CB(TX_DONE,&TxFrame);
    }
    break;
    
  case LL_SM_SEND_NKE:
    if(!tx_req)
    {
      if(TimeDelay(RxCompletedTimeStamp,WMBus_PhyGetResponseTimeMin() - DELAY_GUARD_TIME_RX2TX))      
      {
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }
    }
    else if(TxCompleted)
    {
      /* Reset the TxCompleted flag for the next time */
      TxCompleted  = 0;
      /* Reset the tx_req flag for the next time */
      tx_req = 0;
      /* Set the phy state to RX mode*/
      WMBus_PhySetState(PHY_STATE_RX); 
      /* The new state for the LL SM will be idle state */
      LL_State = LL_SM_IDLE; 
      WMBus_LinkTxCompleted_CB(TX_DONE,&TxFrame);
    }
    break;
    
  case LL_SM_SEND_ACK:
    if(!tx_req)
    {
      if(TimeDelay(RxCompletedTimeStamp,WMBus_PhyGetResponseTimeMin() - DELAY_GUARD_TIME_RX2TX))      
      {
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        
        /* Transmission is requested */
        tx_req = 1;
      }
    }
    else if(TxCompleted)
    {
      /* Reset the TxCompleted flag for the next time */
      TxCompleted  = 0;
      
      /* Reset the tx_req flag for the next time */
      tx_req = 0;
    
      WMBus_LinkTxCompleted_CB(TX_DONE,&TxFrame);
    
      /* Set the phy state to RX mode*/
      WMBus_PhySetState(PHY_STATE_RX); 
      
      /* The new state for the LL SM will be idle state */
      LL_State = LL_SM_IDLE; 
      } 
    break;
    
  case LL_SM_SEND_SND_UD:
    if(!tx_req)
    {
      if(TimeDelay(RxCompletedTimeStamp,WMBus_PhyGetResponseTimeMin() - DELAY_GUARD_TIME_RX2TX))      
      {
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        
        /* Transmission is requested */
        tx_req = 1;
      }
    }
    else if(TxCompleted)
    {
      /* Reset the TxCompleted flag for the next time */
      TxCompleted  = 0;
      
      /* Reset the tx_req flag for the next time */
      tx_req = 0;
    
      WMBus_LinkTxCompleted_CB(TX_DONE,&TxFrame);
    
      /* Set the phy state to RX mode*/
      WMBus_PhySetState(PHY_STATE_RX); 
      
      /* The new state for the LL SM will be wait for Ack state */
      LL_State = LL_SM_WAIT_ACK; 
    }
    break;
    
  case LL_SM_SEND_SND_UD2:
    if(!tx_req)
    {
      if(TimeDelay(RxCompletedTimeStamp,WMBus_PhyGetResponseTimeMin() - DELAY_GUARD_TIME_RX2TX))      
      {
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }
    }
    else if(TxCompleted)
    {
      /* Reset the TxCompleted flag for the next time */
      TxCompleted  = 0;
      
      /* Reset the tx_req flag for the next time */
      tx_req = 0;
    
      WMBus_LinkTxCompleted_CB(TX_DONE,&TxFrame);
    
      /* Set the phy state to RX mode*/
      WMBus_PhySetState(PHY_STATE_RX); 
      
      /* The new state for the LL SM will be wait for RSP-UD state */
      LL_State = LL_SM_WAIT_RSP_UD;
    }
    break;  
    
  case LL_SM_SEND_REQ_UD1:
    if(!tx_req)
    {
      if(TimeDelay(RxCompletedTimeStamp,WMBus_PhyGetResponseTimeMin() - DELAY_GUARD_TIME_RX2TX))      
      {
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }
    }
    else if(TxCompleted)
    {
      /* Reset the TxCompleted flag for the next time */
      TxCompleted  = 0;
      
      /* Reset the tx_req flag for the next time */
      tx_req = 0;
      
      WMBus_LinkTxCompleted_CB(TX_DONE,&TxFrame);
    
      /* Set the phy state to RX mode*/
      WMBus_PhySetState(PHY_STATE_RX);  
      /* The new state for the LL SM will be idle state */
      LL_State = LL_SM_WAIT_RSP_UD; 
    }
    break; 
    
  case LL_SM_SEND_REQ_UD2:
    if(!tx_req)
    {
      if(TimeDelay(RxCompletedTimeStamp,WMBus_PhyGetResponseTimeMin() - DELAY_GUARD_TIME_RX2TX))      
      {
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }
    }
    else if(TxCompleted)
    {
      /* Reset the TxCompleted flag for the next time */
      TxCompleted  = 0;
      /* Reset the tx_req flag for the next time */
      tx_req = 0;
      
      WMBus_LinkTxCompleted_CB(TX_DONE,&TxFrame);
    
      /* Set the phy state to RX mode*/
      WMBus_PhySetState(PHY_STATE_RX); 
      /* The new state for the LL SM will be idle state */
      LL_State = LL_SM_WAIT_RSP_UD;
    }
    break; 
    
  case LL_SM_WAIT_ACK:
    if(WMBus_TimDiffFromNow(/*RxCompletedTimeStamp*/TxCompletedTimeStamp)< LinkAttr.LinkTimeout)
    {
        /* In this case we can check if the packet has arrived*/
        if(WMBus_PhyGetNumRxPackets()>0) 
        {
          if(WMBus_LinkRxFrameHandler()==LINK_STATUS_SUCCESS )
          {
            WMBus_PhyResetPreambSyncFlag(); 
            WMBus_LinkCmdReceived_SM_Other(&RxFrame);
          }
        } 
    }
    else
    {
      /* Important to be set IDLE state BEFORE the callback because this 
      can ask for a transmission moving the state.*/
      LL_State = LL_SM_IDLE;
      WMBus_LinkAckTimeout_CB();  
    }
    break; 
    
  case LL_SM_WAIT_RSP_UD:
    if(WMBus_TimDiffFromNow(/*RxFrame.timestamp*/TxCompletedTimeStamp)< LinkAttr.LinkTimeout)
    {
        /* In this case we can check if the packet has arrived*/
        if(WMBus_PhyGetNumRxPackets()>0) 
        {
          if(WMBus_LinkRxFrameHandler()==LINK_STATUS_SUCCESS )
          {
            WMBus_LinkCmdReceived_SM_Other(&RxFrame);
            WMBus_PhyResetPreambSyncFlag(); 
          }
        }
    }
    else
    {
      /* Important to be set IDLE state BEFORE the callback because this 
      can ask for a transmission moving the state.*/
      LL_State = LL_SM_IDLE;
      WMBus_LinkRspUdTimeout_CB();  
    }
    break; 
    
        
#ifdef PER_TEST_MODE    
  case LL_SM_TEST_MODE :
    
      /* In this case we can check if the packet has arrived*/
      if(WMBus_PhyGetNumRxPackets()>0) 
      {
        if(WMBus_LinkRxFrameHandler() == LINK_STATUS_SUCCESS )
        {
          
          WMBus_LinkRxPERTestModeFrameReceived_CB(&RxFrame);
          
        }
        
      }
    
    break;
#endif    

  case LL_SM_SEND_SND_IR: case LL_SM_SEND_SND_NR: case LL_SM_SEND_RSP_UD: case LL_SM_SEND_ACC_DMD: case LL_SM_SEND_ACC_NR:
  case LL_SM_WAIT_CNF_IR:  case LL_SM_WAIT_STATE: case LL_SM_VCO_CAL: case LL_INVALID_EVENT:
#ifndef PER_TEST_MODE
  case LL_SM_TEST_MODE: 
#endif    
	  break;

  }  
}

/**
* @brief  This function will run the Link Layer State Machine 
*         for Meter for received frame 
* @param  LinkRxframe: Received frame
* @retval None
*/
void WMBus_LinkCmdReceived_SM_Meter(Frame_t *pRxframe)
{  
  
  switch(pRxframe->c_field & LINK_C_FIELD_MASK)
  {
  case LL_SND_UD: /***  SND-UD and SND-UD2 ******/
    /* SND-UD/SND-UD2 is received; Response is ACK/RSP_UD*/      
    WMBus_LinkSendUdReceived_CB(pRxframe);     
    
    break;
    
  case LL_REQ_UD2:
    {
      WMBus_LinkRequestUd2Received_CB(pRxframe);
    }
    break;
    
  case LL_REQ_UD1:
    /* REQ-UD1 is received; response to REQ-UD1 is ACK/RSP-UD*/      
    WMBus_LinkRequestUd1Received_CB(pRxframe);
    break;  
    
    
  case LL_SND_NKE:
    WMBus_LinkSendNkeReceived_CB(pRxframe);
    break;
    
  default:
    break;
  }
}

/**
* @brief  This function will run the Link Layer State Machine for Meter 
* @param  None
* @retval None
*/
static void WMBus_LinkLayer_SM_Meter(void)
{
  static uint8_t tx_req = 0;
  static uint8_t firstTx = 1;
  uint32_t timeStamp = 0;
  
  switch(LL_State)
  {
  case LL_SM_IDLE:
   
    if(LinkAttr.LinkFreqAccessCycle)
    {
      if(WMBus_TimDiffFromNow(FacTimestamp)<=WMBus_PhyGetFACTimeout())
      {
        if(TimeDelay(TxCompletedTimeStamp,WMBus_PhyGetFACDelay()))
        {
          /* the FAC timeout has expired so we should send the packet again */
          LL_State=LL_State_FAC;
        }
      }
      else
      {
        LinkAttr.LinkFreqAccessCycle=0;

      }
    }
    break;
    
  case LL_SM_SEND_SND_IR:     
    if(!tx_req)
    {
      if(firstTx == 1)
      {
        firstTx++;
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }
      else
      {
        //if(TimeDelay(TxCompletedTimeStamp,30000))//300000))
        {
          /* Call the function to TX */
          WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
          /* Transmission is requested */
          tx_req = 1;
        }
      }
    }
    else if(TxCompleted)
    {
      if(TimeDelay(TxCompletedTimeStamp,(uint32_t)(WMBus_PhyGetResponseTimeMin()- DELAY_GUARD_TIME_TX2RX))) 
      {
        if(LinkAttr.LinkFreqAccessCycle) LL_State_FAC=LL_State;
          
        /* Reset the TxCompleted flag for the next time */
        TxCompleted  = 0;
        /* Reset the tx_req flag for the next time */
        tx_req = 0;
        
        /* Check if the WMBus mode selected is two way mode */
        if(WMBus_PhyIsTwoWayMode())
        {
          /* Set the phy state to RX mode*/
          WMBus_PhySetState(PHY_STATE_RX);             
          /* The new state for the LL SM will be wait for CNF-IR state */
          LL_State = LL_SM_WAIT_CNF_IR;
        }
        else
        {
          /* The new state for the LL SM will be idle state */
          LL_State = LL_SM_IDLE;
        } 
      }
    }           
    break;
    
  case LL_SM_SEND_SND_NR:
    if(!tx_req)
    {
      if(firstTx == 1)
      {
        firstTx++;
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }
      else
      {
        if(TimeDelay(TxCompletedTimeStamp,WMBus_PhyGetFACDelay()))
        {
          /* Call the function to TX */
          WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
          /* Transmission is requested */
          tx_req = 1;
        }
      }      
    }
    else if(TxCompleted)
    {
      if(TimeDelay(TxCompletedTimeStamp,(uint32_t)(WMBus_PhyGetResponseTimeMin()-DELAY_GUARD_TIME_TX2RX)))
      {
        if(LinkAttr.LinkFreqAccessCycle) LL_State_FAC=LL_State;
        /* Reset the TxCompleted flag for the next time */
        TxCompleted  = 0;
        /* Reset the tx_req flag for the next time */
        tx_req = 0;
        
        /* Check if the WMBus mode selected is two way mode */
        if (WMBus_PhyIsTwoWayMode())
        {
          /* Set the phy state to RX mode*/
          WMBus_PhySetState(PHY_STATE_RX);
          /* The new state for the LL SM will be wait for state */
          LL_State = LL_SM_WAIT_STATE;
        }
        else
        {
          /* The new state for the LL SM will be idle state */
          LL_State = LL_SM_IDLE;
        } 
      }
    }     
    break;
    
  case LL_SM_SEND_ACC_DMD:
    if(!tx_req)
    {
      if(firstTx == 1)
      {
        firstTx++;
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }
      else
      {
        if(TimeDelay(TxCompletedTimeStamp,WMBus_PhyGetFACDelay()))
        {
          /* Call the function to TX */
          WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
          /* Transmission is requested */
          tx_req = 1;
        }
      }      
    }
    else if(TxCompleted)
    {
      if(TimeDelay(TxCompletedTimeStamp,(uint32_t)(WMBus_PhyGetResponseTimeMin()-DELAY_GUARD_TIME_TX2RX)))
      {
        if(LinkAttr.LinkFreqAccessCycle) LL_State_FAC=LL_State;
        
        /* Reset the TxCompleted flag for the next time */
        TxCompleted  = 0;
        /* Reset the tx_req flag for the next time */
        tx_req = 0;
        
        /* Check if the WMBus mode selected is two way mode */
        if (WMBus_PhyIsTwoWayMode())
        {
          /* Set the phy state to RX mode*/
          WMBus_PhySetState(PHY_STATE_RX);
          /* The new state for the LL SM will be wait for Ack state */
          LL_State = LL_SM_WAIT_ACK;
        }
        else
        {
          /* The new state for the LL SM will be idle state */
          LL_State = LL_SM_IDLE;
        } 
      }
    }     
    break;
    
  case LL_SM_SEND_ACC_NR:         
    if(!tx_req)
    {
      if(firstTx == 1)
      {
        firstTx++;
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }
      else
      {
        if(TimeDelay(TxCompletedTimeStamp,WMBus_PhyGetFACDelay()))
        {
          /* Call the function to TX */
          WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
          /* Transmission is requested */
          tx_req = 1;
        }
      }      
    }
    else if(TxCompleted)
    {
      if(TimeDelay(TxCompletedTimeStamp,(WMBus_PhyGetResponseTimeMin()-DELAY_GUARD_TIME_TX2RX)))
      {
        if(LinkAttr.LinkFreqAccessCycle) LL_State_FAC=LL_State;
        
        /* Reset the TxCompleted flag for the next time */
        TxCompleted  = 0;
        /* Reset the tx_req flag for the next time */
        tx_req = 0;
        /* Set the phy state to RX mode*/
        WMBus_PhySetState(PHY_STATE_RX);
        /* The new state for the LL SM will be wait state */
        LL_State = LL_SM_WAIT_STATE; 
      }
    }      
    break;
    
  case LL_SM_SEND_ACK:
    if(!tx_req)
    {
      if(TimeDelay(TxCompletedTimeStamp,WMBus_PhyGetFACDelay()))
      {
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }     
    }
    else if(TxCompleted)
    {
      if(TimeDelay(TxCompletedTimeStamp,(uint32_t)(WMBus_PhyGetResponseTimeMin()-DELAY_GUARD_TIME_TX2RX)))
      {
        if(LinkAttr.LinkFreqAccessCycle) LL_State_FAC=LL_State;
        
        /* Reset the TxCompleted flag for the next time */
        TxCompleted  = 0;
        /* Reset the tx_req flag for the next time */
        tx_req = 0;
        /* Set the phy state to RX mode*/
        WMBus_PhySetState(PHY_STATE_RX);
        /* The new state for the LL SM will be idle */
        LL_State=LL_SM_WAIT_STATE;//LL_SM_IDLE;//LL_SM_ACK_FAC_STATE;
      }
    }  
    break;
    
  case LL_SM_SEND_RSP_UD:
    if(!tx_req)
    {
      if(TimeDelay(TxCompletedTimeStamp,WMBus_PhyGetFACDelay()))
      {
        /* Call the function to TX */
        WMBus_LinkTransmitFrameHandlerMode(&TxFrame);
        /* Transmission is requested */
        tx_req = 1;
      }    
    }
    else if(TxCompleted)
    {
      if(TimeDelay(TxCompletedTimeStamp,(uint32_t)(WMBus_PhyGetResponseTimeMin()-DELAY_GUARD_TIME_TX2RX)))
      {
        /* Reset the TxCompleted flag for the next time */
        TxCompleted  = 0;
        /* Reset the tx_req flag for the next time */
        tx_req = 0;
        /* Set the phy state to RX mode*/
        WMBus_PhySetState(PHY_STATE_RX);
        /* The new state for the LL SM will be idle */
        LL_State=LL_SM_WAIT_STATE;
      }
    }    
    break;
    
  case LL_SM_WAIT_ACK:
    
    timeStamp = WMBus_TimDiffFromNow(TxCompletedTimeStamp);       
    if(RxTimeout==0 && timeStamp <= (WMBus_PhyGetResponseTimeMax() + LinkAttr.LinkTimeout))
    {
        /* In this case we can check if the packet has arrived*/
        if(WMBus_PhyGetNumRxPackets()>0) 
        {
          if(WMBus_LinkRxFrameHandler()==LINK_STATUS_SUCCESS )
          {
            if(LinkAttr.LinkFreqAccessCycle) FacTimestamp=WMBus_TimGetTimestamp();
            
            /* Important to be set IDLE state BEFORE the callback because this 
            can ask for a transmission moving the state.*/
            LL_State = LL_SM_IDLE;
            
            WMBus_LinkAckReceived_CB(&RxFrame);

            WMBus_PhyResetPreambSyncFlag(); 
            
          }
        }   
    }
    else
    {
      /* Important to be set IDLE state BEFORE the callback because this 
      can ask for a transmission moving the state.*/
      RxTimeout=0;
      LL_State = LL_SM_IDLE;
      WMBus_LinkAckTimeout_CB();  
    }
    break;
    
  case LL_SM_WAIT_CNF_IR:
    timeStamp = WMBus_TimDiffFromNow(TxCompletedTimeStamp);
    if(RxTimeout==0 && timeStamp <= (WMBus_PhyGetResponseTimeMax()+LinkAttr.LinkTimeout))
    {
      /* In this case we can check if the packet has arrived*/
      if(WMBus_PhyGetNumRxPackets()>0) 
      {
        if(WMBus_LinkRxFrameHandler() == LINK_STATUS_SUCCESS )
        {
          WMBus_PhyResetPreambSyncFlag(); 
          if(LinkAttr.LinkFreqAccessCycle) FacTimestamp=WMBus_TimGetTimestamp();
          
          if((RxFrame.c_field) == LL_CNF_IR)
          {
            /* Important to be set IDLE state BEFORE the callback because this 
            can ask for a transmission moving the state.*/            
            LL_State = LL_SM_IDLE;
            WMBus_LinkConfirmInstallReqReceived_CB(&RxFrame);
           
          }
          else if((RxFrame.c_field) == LL_SND_NKE)
          {
            /* Important to be set IDLE state BEFORE the callback because this 
            can ask for a transmission moving the state.*/            
            LL_State = LL_SM_IDLE;
           
            WMBus_LinkSendNkeReceived_CB(&RxFrame);
            
          }
        }
      }
    }
    else
    {        
      /* Important to be set IDLE state BEFORE the callback because this 
      can ask for a transmission moving the state.*/ 
      RxTimeout=0;
      LL_State = LL_SM_IDLE;
      WMBus_LinkCnfIRTimeout_CB();
    }
    break;    
    
  case LL_SM_WAIT_STATE:
    
    timeStamp = WMBus_TimDiffFromNow(TxCompletedTimeStamp);
    if(RxTimeout==0 && timeStamp <= (WMBus_PhyGetResponseTimeMax() + LinkAttr.LinkTimeout))
    {
        /* In this case we can check if the packet has arrived*/
        if(WMBus_PhyGetNumRxPackets()>0) 
        {
          if(WMBus_LinkRxFrameHandler()==LINK_STATUS_SUCCESS )
          {          
            if(LinkAttr.LinkFreqAccessCycle) FacTimestamp=WMBus_TimGetTimestamp();
            
            /* Important to be set IDLE state BEFORE the callback because this 
            can ask for a transmission moving the state.*/
            LL_State = LL_SM_IDLE;
            
            WMBus_LinkCmdReceived_SM_Meter(&RxFrame);
            WMBus_PhyResetPreambSyncFlag(); 
          }
        }
    }
    else
    {  
      /* The new state for the LL SM will be idle */
      RxTimeout=0;
      LL_State = LL_SM_IDLE;
      WMBus_LinkRxTimeout_CB();
    }
    break;

#ifdef PER_TEST_MODE    
  case LL_SM_TEST_MODE :
    
      /* In this case we can check if the packet has arrived*/
      if(WMBus_PhyGetNumRxPackets()>0) 
      {
        if(WMBus_LinkRxFrameHandler() == LINK_STATUS_SUCCESS )
        {
          
          WMBus_LinkRxPERTestModeFrameReceived_CB(&RxFrame);
          
        }
        
      }
    
    break;
#endif
    
  default:
    break;
  }
}


/**
* @brief  This function will run the Link Layer State Machine for SNIFFER 
* @param  None
* @retval None
*/
void WMBus_LinkLayer_SM_Sniffer(void)
{
  if(WMBus_PhyGetNumRxPackets()>0)
  {
    WMBus_LinkRxFrameHandler();
    WMBus_PhyResetPreambSyncFlag(); 
  }
}

/**
* @brief  This function will run the Link Layer State Machine 
* @param  None
* @retval None
*/
void WMBus_LinkLayer_SM(void)
{ 
  if(LinkAttr.DevType==OTHER)
  {
    WMBus_LinkLayer_SM_Other();
  }
  else if(LinkAttr.DevType==METER)
  {
    WMBus_LinkLayer_SM_Meter();
  }
  else if (LinkAttr.DevType == SNIFFER)
  {
    WMBus_LinkLayer_SM_Sniffer();
  }
}

/**
* @brief  This function Sets the Link Layer attribute
* @param  attr: Attribute to be set
* @param  value: Value to be set
* @Retval Status of the operation performed
*/
LINK_STATUS_t WMBus_LinkSetAttribute(LINK_ATTR_ID_t attr, uint32_t value)
{
  if(IS_WMBUS_LINK_ATTR(attr))
  {  
    switch(attr)
    {
    case LINK_ATTR_ID_DEV_TYPE:
      LinkAttr.DevType = (WMBusDeviceType_t)value;
      break;
      
    case LINK_ATTR_ID_M_FIELD:
      LinkAttr.MField[0] = (uint8_t)(value&0x00FF);
      LinkAttr.MField[1] = (uint8_t)((value&0xFF00)>>8);
      break;
      
    case LINK_ATTR_ID_FRAME_FORMAT:
      LinkAttr.LinkFrameFormat = ((0x01 == value)? FRAME_FORMAT_B :\
        FRAME_FORMAT_A);
      break;
    case LINK_ATTR_ID_LINK_TIMEOUT:
      LinkAttr.LinkTimeout=value;
      break;
    case LINK_ATTR_ID_FAC:
      LinkAttr.LinkFreqAccessCycle=value;
      LL_State_FAC = LL_SM_IDLE;
      FacTimestamp=WMBus_TimGetTimestamp();
    default:
      return LINK_STATUS_INVALID_ATTR;
    }
    return LINK_STATUS_SUCCESS;
  }
  else    
  {
    return LINK_STATUS_INVALID_ATTR;
  }  
}

/**
* @brief  This function Gets the Link Layer attribute
* @param  attr: Attribute to be read
* @param  *value: Pointer to the value
* @Retval Status of the operation performed
*/
LINK_STATUS_t WMBus_LinkGetAttribute(LINK_ATTR_ID_t attr, uint8_t* value)
{
  if(IS_WMBUS_LINK_ATTR(attr))
  {  
    switch(attr)
    {     
    case LINK_ATTR_ID_DEV_TYPE:
      *value = LinkAttr.DevType;
      break;
      
    case LINK_ATTR_ID_M_FIELD:
      memcpy(value,LinkAttr.MField,2);
      break;
      
    case LINK_ATTR_ID_A_FIELD:
      memcpy(value,LinkAttr.AField,6);
      break;
      
    case LINK_ATTR_ID_FRAME_FORMAT:
      *value = ((FRAME_FORMAT_B == LinkAttr.LinkFrameFormat)? FRAME_FORMAT_B :\
        FRAME_FORMAT_A);
      break;
    case LINK_ATTR_ID_LINK_TIMEOUT:
      memcpy(value,&LinkAttr.LinkTimeout,4);
      break;
    case LINK_ATTR_ID_FAC:
      (*value)=LinkAttr.LinkFreqAccessCycle;
      break;
    default:
      return LINK_STATUS_INVALID_ATTR;
    }
    return LINK_STATUS_SUCCESS;
  }
  else    
  {
    return LINK_STATUS_INVALID_ATTR;
  }
}

/**
* @brief  This function returns the Link Layer State
* @param  None
* @Retval Value of the Link Layer state
*/
LL_State_t WMBus_LinkGetState(void)
{
  return LL_State;
}

/**
* @brief  This function Sets the Link Layer attribute
* @param  None
* @Retval RxFrame: The pointer to the RxFrame
*/
Frame_t* WMBus_LinkGetLastRxFrame(void)
{
  return &RxFrame;
}
     
/**
* @brief  This function returns the pointer to the link specific app attribute
*       structure to be updated in the application layer
* @param  none.
* @retval Pointer to WMBusLinkAppliParams_t structure                  
*/
WMBusLinkAppliParams_t* WMBus_Link_GetAppliParameterPointer(void)
{
  return &LinkAppliParams;
}



/**
* @brief  This callback is called when SND-IR is received at other device.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkInstallReqReceived_CB(Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkInstallReqReceived_CB could be implemented in the user file
  */
  WMBus_LinkSendConfirmInstallationRequest(NULL, 0, 0x78);
}

/**
* @brief  This callback is called when SND-NR is received at other device.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkSendNoReplyReceived_CB(Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkSendNoReplyReceived_CB could be implemented in the user file
  */
  WMBus_LinkFrameReceived_CB(frame);
}

/**
* @brief  This callback is called when ACC-NR is received at other device.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkAccessNoReplyReceived_CB(Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkAccessNoReplyReceived_CB could be implemented in the user file
  */
  WMBus_LinkFrameReceived_CB(frame);
}

/**
* @brief  This callback is called when RSP-UD is received at other device.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkResponseUdReceived_CB(Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkResponseUdReceived_CB could be implemented in the user file
  */
  WMBus_LinkFrameReceived_CB(frame);
}

/**
* @brief  RSP-UD Timeout callback.
* @param  None
* @retval Success/Error
*/
__weak void WMBus_LinkRspUdTimeout_CB(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkAckReceived_CB could be implemented in the user file
  */
}

/**
* @brief  This callback is called when ACC_DMD is received at other device.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkAccessDemandReceived_CB(Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkAccessDemandReceived_CB could be implemented in the user file
  */
  WMBus_LinkFrameReceived_CB(frame);
}

/******************************Callbacks from meter****************************/

/**
* @brief  This callback is called when CNF-IR is received at meter.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkConfirmInstallReqReceived_CB(Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkConfirmInstallReqReceived_CB could be implemented in the user file
  */
  WMBus_LinkFrameReceived_CB(frame);
}

/**
* @brief  CNF-IR timeout callback.
* @param  None
* @retval Success/Error
*/
__weak void WMBus_LinkCnfIRTimeout_CB(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkCnfIRTimeout_CB could be implemented in the user file
  */
}

/**
* @brief  This callback is called when SND-UD is received at meter.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkSendUdReceived_CB(Frame_t *frame)
{
  
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkSendUdReceived_CB could be implemented in the user file
  */
  WMBus_LinkFrameReceived_CB(frame);
}



/**
* @brief  This callback is called when REQ-UD2/SND-UD2 is received at meter.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkRequestUd2Received_CB(Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkRequestUd2Received_CB could be implemented in the user file
  */
  WMBus_LinkFrameReceived_CB(frame);  
}

/**
* @brief  This callback is called when REQ-UD1 is received at meter.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkRequestUd1Received_CB( Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkRequestUd1Received_CB could be implemented in the user file
  */
  WMBus_LinkFrameReceived_CB(frame);
}

/**
* @brief  This callback is called when SND-NKE is received at meter.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkSendNkeReceived_CB(Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkSendNkeReceived_CB could be implemented in the user file
  */
  WMBus_LinkFrameReceived_CB(frame);
}

/**
* @brief  This callback is called when ACK is received at meter/Other device.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkAckReceived_CB(Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkAckReceived_CB could be implemented in the user file
  */
  WMBus_LinkFrameReceived_CB(frame);
}

/**
* @brief  ACK timeout callback.
* @param  None
* @retval Success/Error
*/
__weak void WMBus_LinkAckTimeout_CB(void)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkAckReceived_CB could be implemented in the user file
  */
}

/**
* @brief  Send ACK callback.
* @param  *frame: Pointer to received frame
* @retval Success/Error
*/
__weak void WMBus_LinkSendAck_CB(Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkSendAck_CB could be implemented in the user file
  */
}

/**
* @brief  Rx Timeout callback.
* @param  None
* @retval Success/Error
*/
__weak void WMBus_LinkRxTimeout_CB(void)                            
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkRxTimeout_CB could be implemented in the user file
  */
}


/**
* @brief  Tx Completed callback.
* @param  None
* @retval Success/Error
*/
__weak void WMBus_LinkTxCompleted_CB(Tx_Outcome_t tx_outcome, Frame_t *frame)
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkTxCompleted_CB could be implemented in the user file
  */
}

/**
* @brief  Rx Completed callback.
* @param  None
* @retval Success/Error
*/
__weak void WMBus_LinkFrameReceived_CB(Frame_t *frame)
{
  
  
}

/**
* @brief  Rx PER test callback.
* @param  None
* @retval Success/Error
*/
__weak void WMBus_LinkRxPERTestModeFrameReceived(Frame_t *frame)
{
  
  
}


__weak void WMBus_LinkRxCompleted_SNDIR_CB(void)                            
{
  /* NOTE : This function Should not be modified, when the callback is needed,
  the WMBus_LinkRxTimeout_CB could be implemented in the user file
  */
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
