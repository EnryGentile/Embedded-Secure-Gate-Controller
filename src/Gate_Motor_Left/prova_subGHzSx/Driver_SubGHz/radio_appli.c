/**
******************************************************************************
* @file    radio_appli.c
* @author  CLAB
* @version V1.0.0
* @date    03-March-2017
* @brief   user file to configure S2LP transceiver.
*         
@verbatim
===============================================================================
##### How to use this driver #####
===============================================================================
[..]
This file is generated automatically by STM32CubeMX and eventually modified 
by the user

@endverbatim
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

/* Includes ------------------------------------------------------------------*/
#include "radio_appli.h"
#include "cube_hal.h"
#include "MCU_Interface.h" 
#include "p2p_lib.h" 
#include "radio_gpio.h" 
#include <string.h>
#include "sha256.h"


#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
#include "SPIRIT1_Util.h"
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
#include "S2LP_Util.h"
#include "S2LP_Timer_ex.h"
#endif


/** @addtogroup USER
* @{
*/

/** @defgroup S2LP_APPLI
* @brief User file to configure S2LP transceiver for desired frequency and 
* @feature.
* @{
*/

/* Private typedef -----------------------------------------------------------*/
const uint8_t CHIAVE_SEGRETA[32] = {
    0x8F, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x70,
    0x81, 0x92, 0xA3, 0xB4, 0xC5, 0xD6, 0xE7, 0xF8,
    0x09, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F, 0x70,
    0x81, 0x92, 0xA3, 0xB4, 0xC5, 0xD6, 0xE7, 0xF8
};

#define MAX_TELECOMANDI 10

static Telecomando_valido Lista_Telecomandi[MAX_TELECOMANDI] = {
    { .id_telecomando = 1, .contatore = 0 }, // telecomando di test
    { .id_telecomando = 42,  .contatore = 0 }, // ipotetico telecomando per fare una prova in più
    { .id_telecomando = 123,   .contatore = 0 }   // uguale al due

    // Gli altri 7 slot dell'array verranno inizializzati automaticamente a 0
};

//AGGIUNTA-------------------------------------
extern volatile uint8_t flag_radio_irq;
extern messaggio_scartato;

static uint8_t num_telecomandi_registrati = 3;

extern flag_cancello;
uint8_t messaggio_ricevuto=0;
/**
* @brief RadioDriver_t structure fitting
*/

RadioDriver_t radio_cb =
{
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  .Init = RadioInterfaceInit, 
  .GpioIrq = RadioGpioIrqInit,
  .RadioInit = RadioRadioInit,
  .SetRadioPower = RadioSetPower,
#endif
  .PacketConfig = RadioPacketConfig,
  .SetPayloadLen = RadioSetPayloadlength,
  .SetDestinationAddress = RadioSetDestinationAddress,
  .EnableTxIrq = RadioEnableTxIrq,
  .EnableRxIrq = RadioEnableRxIrq,
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5) 
  .DisableIrq = RadioDisableIrq,
#endif
  .SetRxTimeout = RadioSetRxTimeout,
  .EnableSQI = RadioEnableSQI,
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  .SetRssiThreshold = RadioSetRssiTH,
  .ClearIrqStatus = RadioClearIRQ,
#endif
  .StartRx = RadioStartRx,
  .StartTx = RadioStartTx,
  .GetRxPacket = RadioGetRxPacket
};





/**
* @brief MCULowPowerMode_t structure fitting
*/
MCULowPowerMode_t MCU_LPM_cb =
{
  .McuStopMode = MCU_Enter_StopMode,
  .McuStandbyMode = MCU_Enter_StandbyMode,
  .McuSleepMode = MCU_Enter_SleepMode
}; 

/**
* @brief RadioLowPowerMode_t structure fitting
*/
RadioLowPowerMode_t Radio_LPM_cb =
{
  .RadioShutDown = RadioPowerOFF,
  .RadioStandBy = RadioStandBy,
  .RadioSleep = RadioSleep,
  .RadioPowerON = RadioPowerON
};

/**
* @brief GPIO structure fitting
*/
SGpioInit xGpioIRQ={
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  SPIRIT_GPIO_IRQ,
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  S2LP_GPIO_3,
#endif
  
  RADIO_GPIO_MODE_DIGITAL_OUTPUT_LP,
  RADIO_GPIO_DIG_OUT_IRQ
    
};

/**
* @brief Radio structure fitting
*/
SRadioInit xRadioInit = {
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  XTAL_OFFSET_PPM,
#endif
  BASE_FREQUENCY,
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5) 
  CHANNEL_SPACE,
  CHANNEL_NUMBER,
#endif  
  MODULATION_SELECT,
  DATARATE,
  FREQ_DEVIATION,
  BANDWIDTH
};



#if defined(USE_STack_PROTOCOL)
/**
* @brief Packet Basic structure fitting
*/
PktStackInit xStackInit={
  PREAMBLE_LENGTH,
  SYNC_LENGTH,
  SYNC_WORD,
  LENGTH_TYPE,
  LENGTH_WIDTH,
  CRC_MODE,
  CONTROL_LENGTH,
  EN_FEC,
  EN_WHITENING
};

/* LLP structure fitting */
PktStackLlpInit xStackLLPInit ={
  EN_AUTOACK,
  EN_PIGGYBACKING,
  MAX_RETRANSMISSIONS
};

/**
* @brief Address structure fitting
*/
PktStackAddressesInit xAddressInit={
  EN_FILT_MY_ADDRESS,
  MY_ADDRESS,
  EN_FILT_MULTICAST_ADDRESS,
  MULTICAST_ADDRESS,
  EN_FILT_BROADCAST_ADDRESS,
  BROADCAST_ADDRESS
};

#elif defined(USE_BASIC_PROTOCOL)

/**
* @brief Packet Basic structure fitting
*/

PktBasicInit xBasicInit={
  
  PREAMBLE_LENGTH,
  SYNC_LENGTH,
  SYNC_WORD,
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  LENGTH_TYPE,
  LENGTH_WIDTH,
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  VARIABLE_LENGTH,
  EXTENDED_LENGTH_FIELD,
#endif
  CRC_MODE,
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  CONTROL_LENGTH,
#endif
  EN_ADDRESS,
  EN_FEC,
  EN_WHITENING
};

/**
* @brief Address structure fitting
*/
PktBasicAddressesInit xAddressInit={
  EN_FILT_MY_ADDRESS,
  MY_ADDRESS,
  EN_FILT_MULTICAST_ADDRESS,
  MULTICAST_ADDRESS,
  EN_FILT_BROADCAST_ADDRESS,
  BROADCAST_ADDRESS
};

#endif

#ifdef CSMA_ENABLE
/**
* @brief CSMA structure fitting
*/
RadioCsmaInit xCsmaInit={
  
  PERSISTENT_MODE_EN, /* CCA may optionally be persistent, i.e., rather than 
  entering backoff when the channel is found busy, CCA continues until the 
  channel becomes idle or until the MCU stops it.
  The thinking behind using this option is to give the MCU the possibility of 
  managing the CCA by itself, for instance, with the allocation of a 
  transmission timer: this timer would start when MCU finishes sending out 
  data to be transmitted, and would end when MCU expects that its transmission
  takes place, which would occur after a period of CCA.
  The choice of making CCA persistent should come from trading off 
  transmission latency,under the direct control of the MCU, and power 
  consumption, which would be greater due to a busy wait in reception mode.
  */
  CS_PERIOD,
  CS_TIMEOUT,
  MAX_NB,
  BU_COUNTER_SEED,
  CU_PRESCALER
};
#endif 

/* Private define ------------------------------------------------------------*/
#define TIME_UP                                         0x01

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
RadioDriver_t *pRadioDriver;
MCULowPowerMode_t *pMCU_LPM_Comm;
RadioLowPowerMode_t  *pRadio_LPM_Comm;
/*Flags declarations*/
volatile FlagStatus xRxDoneFlag = RESET, xTxDoneFlag=RESET, cmdFlag=RESET;
volatile FlagStatus xStartRx=RESET, rx_timeout=RESET, exitTime=RESET;
volatile FlagStatus PushButtonStatusWakeup=RESET;
volatile FlagStatus PushButtonStatusData=RESET;
/*IRQ status struct declaration*/
RadioIrqs xIrqStatus;



__IO uint32_t KEYStatusData = 0x00;
static AppliFrame_t xTxFrame, xRxFrame;
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
uint8_t DestinationAddr;
#endif
uint8_t TxFrameBuff[MAX_BUFFER_LEN] = {0x00};
uint16_t exitCounter = 0;
uint16_t txCounter = 0;
uint16_t wakeupCounter = 0;
uint16_t dataSendCounter = 0x00;

/* Private function prototypes -----------------------------------------------*/

#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)

void HAL_S2LP_Init(void);
void Enter_LP_mode(void);
void Exit_LP_mode(void);
void MCU_Enter_StopMode(void);
void MCU_Enter_StandbyMode(void);
void MCU_Enter_SleepMode(void);
void RadioPowerON(void);
void RadioPowerOFF(void);
void RadioStandBy(void);
void RadioSleep(void);
void AppliSendBuff(AppliFrame_t *xTxFrame, uint8_t cTxlen);
void AppliReceiveBuff(uint8_t *RxFrameBuff, uint8_t cRxlen);
void P2P_Init(void);
void STackProtocolInit(void);
void BasicProtocolInit(void);
void P2PInterruptHandler(void);
void Set_KeyStatus(FlagStatus val);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void HAL_SYSTICK_Callback(void);
#endif

/* Private functions ---------------------------------------------------------*/

/** @defgroup S2LP_APPLI_Private_Functions
* @{
*/

/**
* @brief  Initializes RF Transceiver's HAL.
* @param  None
* @retval None.
*/
void HAL_Radio_Init(void)
{
  pRadioDriver = &radio_cb;
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  pRadioDriver->Init( ); 
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  S2LPInterfaceInit(); 
#endif
}



SM_State_t SM_State = SM_STATE_START_RX;/* The actual state running */

/**
* @brief  S2LP P2P Process State machine
* @param  uint8_t *pTxBuff = Pointer to aTransmitBuffer 
*         uint8_t cTxlen = length of aTransmitBuffer 
*         uint8_t* pRxBuff = Pointer to aReceiveBuffer
*         uint8_t cRxlen= length of aReceiveBuffer
* @retval None.
*/
void P2P_Process(uint8_t *pTxBuff, uint8_t cTxlen, uint8_t* pRxBuff, uint8_t cRxlen)
{

  uint8_t xIndex = 0;
  uint8_t ledToggleCtr = 0;
  uint8_t  dest_addr;
  /*float rRSSIValue = 0;*/
  
  //AGGIUNTA-------------------------------------
  if(flag_radio_irq == 1)
      {
          flag_radio_irq = 0;      // Abbassa subito la bandierina
          P2PInterruptHandler();   // Ora l'SPI comunicherà senza NESSUN conflitto
      }

  switch(SM_State)
  {
  case SM_STATE_START_RX:
    {      
    	//AGGIUNTO
        xRxDoneFlag = RESET;
        rx_timeout = RESET;
    	SpiritCmdStrobeSabort();
    	SpiritCmdStrobeFlushRxFifo();

      AppliReceiveBuff(pRxBuff, cRxlen);
      /* wait for data received or timeout period occured */
      SM_State = SM_STATE_WAIT_FOR_RX_DONE;    
    }
    break;
    
  case SM_STATE_WAIT_FOR_RX_DONE:

	    if(RESET != xRxDoneFlag)
	    {
	        xRxDoneFlag = RESET;
	        SM_State = SM_STATE_DATA_RECEIVED;
	    }
	    else if((RESET != rx_timeout) || (exitTime == RESET))
	        {
	            rx_timeout = RESET;
	            messaggio_ricevuto=0;
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
        HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);
#endif
	            SM_State = SM_STATE_START_RX;
	        }
	        break;
    
  case SM_STATE_DATA_RECEIVED:
    {
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
      pRadioDriver = &radio_cb; 
#endif
      
      pRadioDriver->GetRxPacket(pRxBuff,&cRxlen); 


      xRxFrame.Cmd = pRxBuff[0];
      xRxFrame.CmdLen = pRxBuff[1];
      xRxFrame.Cmdtag = pRxBuff[2];
      xRxFrame.CmdType = pRxBuff[3];
      xRxFrame.DataLen = pRxBuff[4];
      
      if(messaggio_ricevuto==1 && pRxBuff[0]==ACK_OK){
    	  messaggio_ricevuto=0;

    	  for(uint8_t i = 0; i < 6; i++)
    	      {
    	          HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_14); // Lampeggia il LED Arancione
    	          HAL_Delay(50);                          // Aspetta 50ms (300ms totali di blocco)
    	      }
    	  SM_State=SM_STATE_SEND_ACK;
    	  return;
      }

      static uint8_t memoria_payload_rx[96]; // Creiamo un array reale grande abbastanza
      xRxFrame.DataBuff = memoria_payload_rx; // Diciamo al puntatore di puntare qui!

      for (xIndex = 5; xIndex < cRxlen; xIndex++)
      {
        xRxFrame.DataBuff[xIndex] = pRxBuff[xIndex];
      }

      //variabili del telecomando ricevuto
      uint32_t valori_ricevuti[2];
      uint32_t id_ricevuto;
      uint32_t contatore_ricevuto;
      uint8_t firma_calcolata[32];

      int indice_telecomando = -1;


      //Estrazione dei dati in chiaro (ID e Contatore)
      memcpy(&valori_ricevuti, &pRxBuff[5 + 0], 8);
      //memcpy(&contatore_ricevuto, &pRxBuff[5 + 4], 4);


      for (int i = 0; i < num_telecomandi_registrati; i++) {
              if (Lista_Telecomandi[i].id_telecomando == valori_ricevuti[0]) {
                  indice_telecomando = i;

                  break;
              }
      }

      //attacco nemico: Indice non autorizzato
      if (indice_telecomando == -1){
    	  messaggio_scartato=1;
    	  SM_State = SM_STATE_START_RX;
    	  break;
      }

      //attacco nemico: reply
      if (Lista_Telecomandi[indice_telecomando].contatore >= valori_ricevuti[1]){
    	  messaggio_scartato=1;
    	  SM_State = SM_STATE_START_RX;
    	  break;
      }

      hmac_sha256(CHIAVE_SEGRETA, 32,valori_ricevuti, 8, firma_calcolata);
      if (memcmp(firma_calcolata, &pRxBuff[8 + 5], 32) == 0) {

		  Lista_Telecomandi[indice_telecomando].contatore = valori_ricevuti[1];
		  messaggio_ricevuto=1;
	      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_10);


	  } else {
		  messaggio_scartato=1;
		  SM_State = SM_STATE_START_RX;
		  break;

	  }

      if(xRxFrame.Cmd == LED_TOGGLE)
      {
        SM_State = SM_STATE_TOGGLE_LED; 
      } 
      if(xRxFrame.Cmd == ACK_OK)
      {
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
        HAL_Delay(DELAY_TX_LED_GLOW); 
#endif
        SM_State = SM_STATE_ACK_RECEIVED;
      }
    }
    break;
    
  case SM_STATE_SEND_ACK:
    {

      xTxFrame.Cmd = ACK_OK;
      xTxFrame.CmdLen = 0x01;
      xTxFrame.Cmdtag = xRxFrame.Cmdtag;
      xTxFrame.CmdType = APPLI_CMD;
      xTxFrame.DataBuff = pTxBuff;
      xTxFrame.DataLen = cTxlen;
      
      HAL_Delay(DELAY_TX_LED_GLOW);        
      AppliSendBuff(&xTxFrame, xTxFrame.DataLen);   
      SM_State = SM_STATE_WAIT_FOR_TX_DONE;
    }
    break;   
    
  case SM_STATE_SEND_DATA:
    {

      xTxFrame.Cmd = LED_TOGGLE;
      xTxFrame.CmdLen = 0x01;
      xTxFrame.Cmdtag = txCounter++;
      xTxFrame.CmdType = APPLI_CMD;
      xTxFrame.DataBuff = pTxBuff;
      xTxFrame.DataLen = cTxlen;
      AppliSendBuff(&xTxFrame, xTxFrame.DataLen);
      SM_State = SM_STATE_WAIT_FOR_TX_DONE;
    }
    break;
    
  case SM_STATE_WAIT_FOR_TX_DONE:
    /* wait for TX done */    
    if(xTxDoneFlag)
    {
      xTxDoneFlag = RESET;
      
      if(xTxFrame.Cmd == LED_TOGGLE)
      {
        SM_State = SM_STATE_START_RX;
      }
      else if(xTxFrame.Cmd == ACK_OK)
      {
        SM_State = SM_STATE_IDLE;
        HAL_Delay(400);
        flag_cancello=1;

      }
    }
    break;
    
  case SM_STATE_ACK_RECEIVED:
    for(; ledToggleCtr<5; ledToggleCtr++)
    {
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
    	RadioShieldLedToggle(RADIO_SHIELD_LED);
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
      //RadioShieldLedToggle(RADIO_SHIELD_LED);
      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_14);
#endif
      
      HAL_Delay(DELAY_RX_LED_TOGGLE);
    }
    SM_State = SM_STATE_IDLE;   
    break;
    
  case SM_STATE_TOGGLE_LED:
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
    RadioShieldLedOn(RADIO_SHIELD_LED);
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
    //RadioShieldLedOn(RADIO_SHIELD_LED);
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_14);
#endif
    dest_addr = RadioGetReceivedDestinationAddress();
    
    if ((dest_addr == MULTICAST_ADDRESS) || (dest_addr == BROADCAST_ADDRESS))
    {
      /* in that case do not send ACK to avoid RF collisions between several RF ACK messages */
      HAL_Delay(DELAY_TX_LED_GLOW);
      SM_State = SM_STATE_IDLE;
      //SM_State = SM_STATE_SEND_ACK;
    }
    else
    {
      SM_State = SM_STATE_SEND_ACK;
      //SM_State = SM_STATE_IDLE;
    }
    break;
    
  case SM_STATE_IDLE:
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
    RadioShieldLedOff(RADIO_SHIELD_LED);
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
    //RadioShieldLedoff(RADIO_SHIELD_LED);
#endif
    //BSP_LED_Off(LED2);
    SM_State = SM_STATE_START_RX;
    
#if defined(USE_LOW_POWER_MODE)
    Enter_LP_mode();
#endif     
    break;
  }
}

/**
* @brief  This function handles the point-to-point packet transmission
* @param  AppliFrame_t *xTxFrame = Pointer to AppliFrame_t structure 
*         uint8_t cTxlen = Length of aTransmitBuffer
* @retval None
*/
void AppliSendBuff(AppliFrame_t *xTxFrame, uint8_t cTxlen)
{
  uint8_t xIndex = 0;
  uint8_t trxLength = 0;
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  pRadioDriver = &radio_cb; 
#endif
  
#ifdef USE_STack_PROTOCOL
  
  PktStackAddressesInit xAddressInit=
  {
    .xFilterOnMyAddress = S_ENABLE,
    .cMyAddress = MY_ADDRESS,
    .xFilterOnMulticastAddress = S_DISABLE,
    .cMulticastAddress = MULTICAST_ADDRESS,
    .xFilterOnBroadcastAddress = S_ENABLE,
    .cBroadcastAddress = BROADCAST_ADDRESS
  };
  Radio_PktStackAddressesInit(&xAddressInit);
  
  
  
#ifdef USE_STack_LLP
  
  /* LLP structure fitting */
  PktStackLlpInit xStackLLPInit=
  {
    .xAutoAck = S_DISABLE,                
    .xPiggybacking = S_DISABLE,              
    .xNMaxRetx = PKT_N_RETX_2
  }; 
  
#else
  
  /* LLP structure fitting */
  PktStackLlpInit xStackLLPInit=
  {
    .xAutoAck = S_DISABLE,                
    .xPiggybacking = S_DISABLE,              
    .xNMaxRetx = PKT_DISABLE_RETX
  }; 
#endif
  
  Radio_PktStackLlpInit(&xStackLLPInit);
  
  
#endif
  
#ifdef USE_BASIC_PROTOCOL
  RadioPktBasicAddressesInit(&xAddressInit);
#endif  
  
  TxFrameBuff[0] = xTxFrame->Cmd;
  TxFrameBuff[1] = xTxFrame->CmdLen;
  TxFrameBuff[2] = xTxFrame->Cmdtag;
  TxFrameBuff[3] = xTxFrame->CmdType;
  TxFrameBuff[4] = xTxFrame->DataLen;
  // #if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  for(; xIndex < xTxFrame->DataLen; xIndex++)
    //#endif
    //#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
    //  for(; xIndex < cTxlen; xIndex++)
    //#endif
    
  {
    TxFrameBuff[xIndex+5] =  xTxFrame->DataBuff[xIndex];
  }
  
  
  trxLength = (xIndex+5);
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  /* Spirit IRQs disable */
  pRadioDriver->DisableIrq();
  /* Spirit IRQs enable */
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  S2LPGpioIrqDeInit(NULL);
#endif

  pRadioDriver->EnableTxIrq();
  /* payload length config */
  pRadioDriver->SetPayloadLen(trxLength);  
  /* rx timeout config */
  pRadioDriver->SetRxTimeout(RECEIVE_TIMEOUT);
  /* IRQ registers blanking */
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  pRadioDriver->ClearIrqStatus();
  /* destination address */
  pRadioDriver->SetDestinationAddress(DestinationAddr);
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  S2LPGpioIrqClearStatus();
  /* destination address */
  pRadioDriver->SetDestinationAddress(DESTINATION_ADDRESS);
#endif  
  /* send the TX command */
  pRadioDriver->StartTx(TxFrameBuff, trxLength);
}


/**
* @brief  This function handles the point-to-point packet reception
* @param  uint8_t *RxFrameBuff = Pointer to ReceiveBuffer
*         uint8_t cRxlen = length of ReceiveBuffer
* @retval None
*/
void AppliReceiveBuff(uint8_t *RxFrameBuff, uint8_t cRxlen)
{
  /*float rRSSIValue = 0;*/
  exitTime = SET;
  exitCounter = TIME_TO_EXIT_RX;
  
#ifdef USE_STack_PROTOCOL
  
#ifdef USE_STack_LLP
  /* LLP structure fitting */
  PktStackLlpInit xStackLLPInit=
  {
    .xAutoAck = S_ENABLE,                
    .xPiggybacking = S_ENABLE,              
    .xNMaxRetx = PKT_DISABLE_RETX
  }; 
#else
  /* LLP structure fitting */
  PktStackLlpInit xStackLLPInit=
  {
    .xAutoAck = S_DISABLE,                
    .xPiggybacking = S_DISABLE,              
    .xNMaxRetx = PKT_DISABLE_RETX
  }; 
#endif
  
  Radio_PktStackLlpInit(&xStackLLPInit);
  
  PktStackAddressesInit xAddressInit=
  {
    .xFilterOnMyAddress = S_ENABLE,
    .cMyAddress = MY_ADDRESS,
    .xFilterOnMulticastAddress = S_DISABLE,
    .cMulticastAddress = MULTICAST_ADDRESS,
    .xFilterOnBroadcastAddress = S_ENABLE,
    .cBroadcastAddress = BROADCAST_ADDRESS
  };
  
  Radio_PktStackAddressesInit(&xAddressInit);
  if(EN_FILT_SOURCE_ADDRESS)
  {
    Radio_PktStackFilterOnSourceAddress(S_ENABLE);
    Radio_PktStackSetRxSourceMask(SOURCE_ADDR_MASK);
    Radio_PktStackSetSourceReferenceAddress (SOURCE_ADDR_REF);
    
    
    
  }
  else
  {
    RadioPktStackFilterOnSourceAddress(S_DISABLE);    
  }
#endif
  
  RadioPktBasicAddressesInit(&xAddressInit);
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  pRadioDriver->DisableIrq();
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  pRadioDriver = &radio_cb;  
  /* S2LP IRQs disable */
  S2LPGpioIrqDeInit(NULL);
  /* S2LP IRQs enable */
#endif
  pRadioDriver->EnableRxIrq();
  /* payload length config */
  pRadioDriver->SetPayloadLen(PAYLOAD_LEN);
  /* rx timeout config */
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  pRadioDriver->SetRxTimeout(RECEIVE_TIMEOUT);
#endif
  
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  S2LPTimerSetRxTimerMs(700.0);
  SET_INFINITE_RX_TIMEOUT();  
#endif
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  /* destination address */
  pRadioDriver->SetDestinationAddress(DestinationAddr);  
  /* IRQ registers blanking */
  pRadioDriver->ClearIrqStatus();
  /* RX command */
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  /* destination address */
  pRadioDriver->SetDestinationAddress(DESTINATION_ADDRESS);
  /* IRQ registers blanking */
  S2LPGpioIrqClearStatus(); 
#endif
  /* RX command */ 
  pRadioDriver->StartRx();
}

/**
* @brief  This function initializes the protocol for point-to-point 
* communication
* @param  None
* @retval None
*/
void P2P_Init(void)
{
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
	//riga aggiunta per problemi
	pRadioDriver = &radio_cb;

  DestinationAddr=DESTINATION_ADDRESS;
  pRadioDriver->GpioIrq(&xGpioIRQ);
  pRadioDriver->RadioInit(&xRadioInit);
  pRadioDriver->SetRadioPower(POWER_INDEX, POWER_DBM);  
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  pRadioDriver = &radio_cb;
  
  /* S2LP IRQ config */
  S2LPGpioInit(&xGpioIRQ);
  
  /* S2LP Radio config */    
  S2LPRadioInit(&xRadioInit);
  
  
  /* S2LP Radio set power */
  S2LPRadioSetMaxPALevel(S_DISABLE);  
  
  if(!S2LPManagementGetRangeExtender())
  {
    /* if we haven't an external PA, use the library function */
    S2LPRadioSetPALeveldBm(POWER_INDEX,POWER_DBM);
  }
  else
  { 
    /* in case we are using the PA board, the S2LPRadioSetPALeveldBm will be not functioning because
    the output power is affected by the amplification of this external component.
    Set the raw register. */
    uint8_t paLevelValue=0x25; /* for example, this value will give 23dBm about */
    S2LPSpiWriteRegisters(PA_POWER8_ADDR, 1, &paLevelValue);
  }
  S2LPRadioSetPALevelMaxIndex(POWER_INDEX); 
#endif
  
  
  /* S2LP Packet config */  
  pRadioDriver->PacketConfig();
  
  pRadioDriver->EnableSQI();
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  pRadioDriver->SetRssiThreshold(RSSI_THRESHOLD);
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  S2LPRadioSetRssiThreshdBm(RSSI_THRESHOLD);
#endif
  
}

/**
* @brief  This function initializes the STack Packet handler of S2LP
* @param  None
* @retval None
*/
void STackProtocolInit(void)
{ 
  
  
  PktStackInit xStackInit=
  {
    .xPreambleLength = PREAMBLE_LENGTH,
    .xSyncLength = SYNC_LENGTH,
    .lSyncWords = SYNC_WORD,
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
    .xFixVarLength = LENGTH_TYPE,
    .cPktLengthWidth = 8, 
#endif   
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
    .xFixVarLength = S_ENABLE,
    .cExtendedPktLenField = S_DISABLE,
#endif
    .xCrcMode = CRC_MODE,
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
    .xControlLength = CONTROL_LENGTH,
#endif   
    .xFec = EN_FEC,
    .xDataWhitening = EN_WHITENING
  };  
  /* Radio Packet config */
  RadioPktStackInit (&xStackInit);
}  


/*xDataWhitening;  *
* @brief  This function initializes the BASIC Packet handler of S2LP
* @param  None
* @retval None
*/
void BasicProtocolInit(void)
{
  /* RAdio Packet config */
  RadioPktBasicInit(&xBasicInit);
}

/**
* @brief  This routine will put the radio and mcu in LPM
* @param  None
* @retval None
*/
void Enter_LP_mode(void)
{  
  pMCU_LPM_Comm = &MCU_LPM_cb;
  pRadio_LPM_Comm = &Radio_LPM_cb;
  
#if defined(MCU_SLEEP_MODE)&&defined(RF_SHUTDOWN) 
  {
    pRadio_LPM_Comm->RadioShutDown(); 
    pMCU_LPM_Comm->McuSleepMode();
  }
#elif defined(MCU_SLEEP_MODE)&&defined(RF_STANDBY) 
  {
    pRadio_LPM_Comm->RadioStandBy(); 
    pMCU_LPM_Comm->McuSleepMode();
  }
#elif defined(MCU_SLEEP_MODE)&&defined(RF_SLEEP) 
  {
    pRadio_LPM_Comm->RadioSleep();
    pMCU_LPM_Comm->McuSleepMode();
  }
#elif defined(MCU_STOP_MODE)&&defined(RF_SHUTDOWN) 
  {
    pRadio_LPM_Comm->RadioShutDown(); 
    pMCU_LPM_Comm->McuStopMode();
  }
#elif defined(MCU_STOP_MODE)&&defined(RF_STANDBY) 
  {
    pRadio_LPM_Comm->RadioStandBy(); 
    pMCU_LPM_Comm->McuStopMode();
  }
#elif defined(MCU_STOP_MODE)&&defined(RF_SLEEP) 
  {
    pRadio_LPM_Comm->RadioSleep();
    pMCU_LPM_Comm->McuStopMode();
  }
#else
  pMCU_LPM_Comm->McuSleepMode();
#endif
}

/**
* @brief  This routine wake-up the mcu and radio from LPM
* @param  None
* @retval None
*/
void Exit_LP_mode(void)
{
  pRadio_LPM_Comm = &Radio_LPM_cb;      
  pRadio_LPM_Comm->RadioPowerON();  
}

/**
* @brief  This routine puts the MCU in stop mode
* @param  None
* @retval None
*/
void MCU_Enter_StopMode(void)
{
  APPLI_EnterSTOPMode();
}

/**
* @brief  This routine puts the MCU in standby mode
* @param  None
* @retval None
*/
void MCU_Enter_StandbyMode(void)
{
  APPLI_EnterSTANDBYMode();
}

/**
* @brief  This routine puts the MCU in sleep mode
* @param  None
* @retval None
*/
void MCU_Enter_SleepMode(void)
{
  /*Suspend Tick increment to prevent wakeup by Systick interrupt. 
  Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base)*/
  
  APPLI_EnterSLEEPMode();
}

/**
* @brief  This function will turn on the radio and waits till it enters the Ready state.
* @param  Param:None. 
* @retval None
*                       
*/
void RadioPowerON(void)
{
  RadioCmdStrobeReady();   
  
  do{
    /* Delay for state transition */
    for(volatile uint8_t i=0; i!=0xFF; i++);
    
    /* Reads the MC_STATUS register */
    
    RadioRefreshStatus();
    
    
  }
  while(g_xStatus.MC_STATE!=MC_STATE_READY);
}


/**
* @brief  This function will Shut Down the radio.
* @param  Param:None. 
* @retval None
*                       
*/
void RadioPowerOFF(void)
{
  RadioEnterShutdown();
  
  
}


/**
* @brief  This function will put the radio in standby state.
* @param  None. 
* @retval None
*                       
*/
void RadioStandBy(void)
{
  RadioCmdStrobeStandby();  
}

/**
* @brief  This function will put the radio in sleep state.
* @param  None. 
* @retval None
*                       
*/
void RadioSleep(void)
{
  RadioCmdStrobeSleep(); 
}

/**
* @brief  This routine updates the respective status for key press.
* @param  None
* @retval None
*/
void Set_KeyStatus(FlagStatus val)
{
  if(val==SET)
  {
    SM_State = SM_STATE_SEND_DATA;
  }
}

/**
* @brief  This function handles External interrupt request. In this application it is used
*         to manage the S2LP IRQ configured to be notified on the S2LP GPIO_3.
* @param  None
* @retval None
*/
void P2PInterruptHandler(void)
{
  RadioGpioIrqGetStatus(&xIrqStatus);
  
  /* Check the S2LP TX_DATA_SENT IRQ flag */
  if(
     (xIrqStatus.IRQ_TX_DATA_SENT) 
       
#ifdef CSMA_ENABLE
       ||(xIrqStatus.IRQ_MAX_BO_CCA_REACH)
#endif
         )
  {
#ifdef CSMA_ENABLE
    RadioCsma(S_DISABLE);  
    RadioRadioPersistenRx(S_ENABLE);
    RadioRadioCsBlanking(S_ENABLE);
    
    if(xIrqStatus.IRQ_MAX_BO_CCA_REACH)
    {
      RadioCmdStrobeSabort();
      
      
    }
    RadioQiSetRssiThresholddBm(RSSI_THRESHOLD);
    
    
#endif
    
    xTxDoneFlag = SET;
  }
  
  /* Check the S2LP RX_DATA_READY IRQ flag */
  if((xIrqStatus.IRQ_RX_DATA_READY))
  {
    xRxDoneFlag = SET;   
  }
  
  /* Restart receive after receive timeout*/
  if (xIrqStatus.IRQ_RX_TIMEOUT)
  {
    rx_timeout = SET;
    RadioCmdStrobeRx();

  }

  /* Check the S2LP RX_DATA_DISC IRQ flag */
  if(xIrqStatus.IRQ_RX_DATA_DISC)
  {
    /* RX command - to ensure the device will be ready for the next reception */
    RadioCmdStrobeRx();

  }
}

/**
* @brief  SYSTICK callback.
* @param  None
* @retval None
*/
void HAL_SYSTICK_Callback(void)
{
  if(exitTime)
  {
    /*Decreament the counter to check when 3 seconds has been elapsed*/  
    exitCounter--;
    /*3 seconds has been elapsed*/
    if(exitCounter <= TIME_UP)
    {
      exitTime = RESET;
    }
  }
#if defined(RF_STANDBY)
  /*Check if Push Button pressed for wakeup or to send data*/
  if(PushButtonStatusWakeup)
  {
    /*Decreament the counter to check when 5 seconds has been elapsed*/  
    wakeupCounter--;
    
    /*5seconds has been elapsed*/
    if(wakeupCounter<=TIME_UP)
    {
      /*Perform wakeup opeartion*/      
      Exit_LP_mode();
      PushButtonStatusWakeup = RESET;
      PushButtonStatusData = SET;
    }
  }
  else if(PushButtonStatusData)
  {
    dataSendCounter--;
    if(dataSendCounter<=TIME_UP)
    {
      Set_KeyStatus(SET);
      PushButtonStatusWakeup = RESET;
      PushButtonStatusData = RESET;
    }
  }
#endif
}
/**
* @}
*/

/**
* @brief GPIO EXTI callback
* @param uint16_t GPIO_Pin
* @retval None
*/

#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
extern uint16_t M2S_GPIO_PIN_IRQ;
#endif

/*
 void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  
#if defined(RF_STANDBY)/*if S2LP is in standby
#if defined(MCU_STOP_MODE)/*if MCU is in stop mode
  
  /* Clear Wake Up Flag
  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  
  /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
  PLL as system clock source (HSE and PLL are disabled in STOP mode)
  SystemClockConfig_ExitSTOPMode(); 
#endif
#if defined(MCU_SLEEP_MODE) 
  /* Resume Tick interrupt if disabled prior to sleep mode entry
  HAL_ResumeTick();
#endif 
  
  /* Initialize LEDs
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  RadioShieldLedInit(RADIO_SHIELD_LED);
#endif
#if defined(X_NUCLEO_S2868A1) || defined(X_NUCLEO_S2915A1)
  // RadioShieldLedInit(RADIO_SHIELD_LED);
#endif
  BSP_LED_Init(LED2);
  
  PushButtonStatusWakeup = SET;
  PushButtonStatusData = RESET;
  wakeupCounter = LPM_WAKEUP_TIME; 
  dataSendCounter = DATA_SEND_TIME;
  dataSendCounter++;
#endif
  if(GPIO_Pin==GPIO_PIN_0)
  {
    Set_KeyStatus(SET);
  } 
#if defined(X_NUCLEO_IDS01A4) || defined(X_NUCLEO_IDS01A5)
  else 
    if(GPIO_Pin==RADIO_GPIO_3_EXTI_LINE)
    {
      P2PInterruptHandler();
    }
  
#endif
  
}
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
