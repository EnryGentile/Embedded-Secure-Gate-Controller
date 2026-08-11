/**
* @file    wmbus_phy.h
* @author  SRA- NOIDA
* @version V2.5.1
* @date    14-Sept-2020
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WMBUS_PHY_H
#define __WMBUS_PHY_H
/* Includes ------------------------------------------------------------------*/
#ifdef USE_SPIRIT1_DEFAULT
#include "SPIRIT_Config.h"
#include <string.h>  
#endif

#ifdef USE_S2_LP_DEFAULT
#include "S2LP_Config.h"
#include "S2LP_Util.h"
#include "S2LP_Radio.h"
#include "S2LP_PktWMbus.h"
#include "S2LP_Qi.h"
#include "S2LP_PktBasic.h"
#include "MCU_Interface.h"
#include <string.h>
#endif


/** @addtogroup WMBUS_Library
 * @{
 */


/** @addtogroup WMBus_PhyLayer      wM-Bus Physical Layer
* @brief Configuration and management of WMBus Physical layer
* @{
*/

/** @addtogroup WMBus_Phy         WMBus Phy
* @brief Configuration and management of WMBus Phy layer.
* @details See the file <i>@ref wmbus_phy.h</i> for more details.
* @{
*/

/** @defgroup WMBus_Phy_Exported_Types           WMBus Phy Exported Types
* @{
*/

/**
* @brief  WMBus Modes typedef enum for WMBus Phy layer
*/
typedef enum {
  S1_MODE = 0x00,/*!<Transmit only meter for stationary receiving readout */
  S1m_MODE,/*!<Transmit only meter for mobile or stationary readout*/
  S2_MODE,/*!<All meter types. Stationary reading*/
  T1_MODE,/*!<Frequent transmission (short frame meters)*/
  T2_MODE,/*!<Frequent transmission (short frame meter with two way capability)*/
  R2_MODE,/*!<Frequent reception (long range)*/
  ALL_MODE,/*!<Multi-mode option */
  N1_MODE,/*!<Long range transmit for stationary readout*/
  N2_MODE,/*!<Long range two-way communication for stationary readout*/ 
  F2_MODE,/*!<Long range two-way communication for stationary readout*/
  F2m_MODE,/*!<Long range two-way communication */
  C1_MODE,/*!<Frequent transmit only meter for mobile or stationary readout*/
  C2_MODE,/*!<Frequent transmit meter for mobile or stationary readout*/
  INVALID_MODE/*!<Invalid Mode*/
}WMBusMode_t;

/**
* @brief  WMBus Channel typedef enum for WMBus Phy layer
*/
typedef enum{
  CHANNEL_1 = (uint8_t)0x0,/*!<Support for Channel 1*/
  CHANNEL_2,/*!<Support for Channel 2*/
  CHANNEL_3,/*!<Support for Channel 3*/
  CHANNEL_4,/*!<Support for Channel 4*/
  CHANNEL_5,/*!<Support for Channel 5*/
  CHANNEL_6,/*!<Support for Channel 6*/
  CHANNEL_7,/*!<Support for Channel 7*/
  CHANNEL_8,/*!<Support for Channel 8*/
  CHANNEL_9,/*!<Support for Channel 9*/             
  CHANNEL_10,/*!<Support for Channel 10*/
  CHANNEL_1a=0,/*!<Support for Channel 1a*/
  CHANNEL_1b,/*!<Support for Channel 1b*/
  CHANNEL_2a,/*!<Support for Channel 2a*/
  CHANNEL_2b,/*!<Support for Channel 2b*/
  CHANNEL_3a,/*!<Support for Channel 3a*/
  CHANNEL_3b,/*!<Support for Channel 3b*/
  Invalid_Channel/*!<Invalid channel*/
}WMBusChannel_t;

/**
* @brief  WMBus Device type typedef enum for WMBus Phy layer
*/
typedef enum {
  OTHER = 0x00,/*!<Device Type is Other/Concentrator*/
  METER = 0x01,/*!<Device Type is Meter*/
  ROUTER = 0x02,/*!<Device Type is Router*/
  SNIFFER = 0x03,/*!<Device Type is Sniffer*/ 
  NOT_CONFIGURED = 0xFF/*!<Device Type not configured*/
}WMBusDeviceType_t;

/**
* @brief  WMBus Device type typedef enum for WMBus Phy layer
*/
typedef enum {
  NORMAL = 0x00,/*!<Device Type is Other/Concentrator*/
  TXCW = 0x01,/*!<Device Type is TxCW test mode */
  TXPN9 = 0x02,/*!<Device Type is Tx PN9 test mode*/
  RXPER = 0x03,/*!<Device Type is in Rx PER mode*/ 
  TX_TEST = 0x04, /*!<Device Type (Other) is in Tx test mode - send telegrams*/  
  NO_OPERATION = 0xFF/*!<Device Type not configured*/
}WMBusDeviceOperation_t;

/**
* @brief  WMBus Frame Format typedef enum for WMBus Phy layer
*/
typedef enum
{
  FRAME_FORMAT_A  =(uint8_t)0x00,/*!<Indicates Frame Format A*/ 
  FRAME_FORMAT_B,/*!<Indicates Frame Format B*/ 
  FRAME_FORMAT_PRESET
}FrameFormatType_t;

/**
* @brief  WMBus Phy state typedef enum for WMBus Phy layer
*/
typedef enum {
  PHY_STATE_READY=0x00,/*!<PHY radio ready state*/ 
  PHY_STATE_TX,/*!<PHY radio tx state*/ 
  PHY_STATE_RX,/*!<PHY radio rx state*/ 
  PHY_STATE_RX_RESTART, /*!<PHY radio rx state - restart*/
  PHY_STATE_SHUTDOWN,/*!<PHY radio shutdown state*/
  PHY_STATE_STANDBY,/*!<PHY radio standby state*/
  PHY_STATE_RX_NO_TIM,/*!<PHY radio rx state without RX timeout*/ 
  PHY_STATE_TXCW, /*!<PHY radio TXCW test mode */
  PHY_STATE_TXPN9, /*!<PHY radio TXPN9 test mode*/
  PHY_STATE_RXPER, /*!<PHY radio Rx PER mode */
  PHY_STATE_INVALID/*!<PHY radio state invalid*/
}WMBusPhyState_t; 

/**
* @brief  WMBus Phy Attribute typedef enum for WMBus Phy layer
*/
typedef enum {
  WMBUS_PHY_MODE =0x00,/*!<PHY Device Operation Mode*/
  WMBUS_PHY_DEVICE_TYPE,/*!<Device Type*/
  WMBUS_PHY_CHANNEL,/*!< For R2 only, otherwise it is 0*/
  WMBUS_PHY_RESPONSE_TIME_MIN,/*!<PHY minimum response time*/
  WMBUS_PHY_RESPONSE_TIME_MAX,/*!<PHY maximum response time*/
  WMBUS_PHY_RESPONSE_TIME_TYP,/*!<PHY typical response time*/
  WMBUS_PHY_FAC_TIME_DELAY,/*!<FAC time delay*/ 
  WMBUS_PHY_FAC_TIMEOUT,/*!<FAC timeout*/
  WMBUS_PHY_NOMINAL_TIME,/*!<Nomianl Time*/
  WMBUS_PHY_PREAMBLE_LEN,/*!<Preamble length*/ 
  WMBUS_PHY_POSTAMBLE_LEN,/*!<Postable length*/
  WMBUS_PHY_OP_POWER,/*!<Operating Power*/
  WMBUS_PHY_RX_RSSI_THRESHOLD,/*!<RSSI threshold for Rx */
  WMBUS_PHY_TX_INTERVAL,/*!<Transmission interval*/
  WMBUS_PHY_RX_TIMEOUT, /*<RX period to seek the SYNC*/
  WMBUS_PHY_DIR,/*!<Direction of Communication*/  
  WMBUS_PHY_RF_OFFSET,/*!<RF offset, used to compensate the XTAL offset*/
  WMBUS_PHY_CUST_DATARATE,/*!<Specify a datarate from the appli level*/
  WMBUS_PHY_CSMA_ENABLED,/*!<CSMA enabled*/
  WMBUS_PHY_CSMA_RSSI_THRESHOLD,/*!<RSSI threshold for CSMA*/
  WMBUS_PHY_CSMA_OVERRIDE_FAIL,/*!<CSMA override fail*/
  WMBUS_PHY_CSMA_CCA_PERIOD,/*!<CSMA time for RSSI measurement  (ref. to SPIRIT1 datasheet)*/
  WMBUS_PHY_CSMA_CCA_LENGTH,/*!<CSMA length for RSSI measurement (ref. to SPIRIT1 datasheet)*/
  WMBUS_PHY_CSMA_MAX_BO,/*!<CSMA max backoff (ref. to SPIRIT1 datasheet)*/
  WMBUS_PHY_CSMA_BU_SEED,/*!<CSMA max backoff (ref. to SPIRIT1 datasheet)*/
  WMBUS_PHY_CSMA_BU_PRESCALER,/*!<CSMA max backoff (ref. to SPIRIT1 datasheet)*/
  WMBUS_PHY_FRAME_FORMAT, /* WMBUS frame format */
}WMBUS_PHY_ATTR_t;


/**
* @brief  PHY_STATUS enum for WMBus Phy layer
*/
enum
{
  PHY_STATUS_SUCCESS = 0x00,/*!<Indicates PHY Radio success */  
  PHY_STATUS_ERROR,/*!<Indicates PHY Radio error */                   
  PHY_STATUS_ERROR_TIMEOUT,/*!<Indicates PHY  timeout error*/           
  PHY_STATUS_ERROR_INVALID_DEVICE_TYPE,/*!<PHY error for Unknown Device type*/
  PHY_STATUS_ERROR_INVALID_ADDRESS, /*!<PHY error for Invalid device Address*/
  PHY_STATUS_ERROR_INVALID_VALUE,/*!<PHY error for invalid value*/
  PHY_STATUS_ERROR_INVALID_STATE,/*!<PHY error for invalid state*/
  PHY_STATUS_ERROR_INVALID_LENGTH,/*!<PHY error for invalid length*/
  PHY_STATUS_ERROR_NOTHING_RECEIVED,/*!<PHY error nothing received*/
  PHY_STATUS_ERROR_DECODING,/*!<PHY error in decoding*/
  PHY_STATUS_ERROR_XTAL_TIMEOUT,/*!<PHY error crystal timeout occured*/
  PHY_STATUS_ERROR_TX_TIMEOUT,/*!<PHY error for transmission timeout*/
  PHY_STATUS_ERROR_RX_INVALID_LENGTH,/*!<PHY error for received invalid length*/
  PHY_STATUS_ERROR_RX_PACKET_INCOMPLETE,/*!<PHY error for received incomplete packet*/
  PHY_STATUS_ERROR_RX_INVALID_WMBUS_MODE,/*!<PHY error for invalid WMBus mode*/
  PHY_STATUS_INVALID_ATTR/*!<Invalid attribute value*/     
};

#ifdef USE_S2_LP_DEFAULT
/**
 * @brief  Sync length in bytes enumeration.
 */
typedef enum
{
  PKT_SYNC_LENGTH_1BYTE            = 0x00, /*!< Sync length 1 byte*/
  PKT_SYNC_LENGTH_2BYTES           = 0x02, /*!< Sync length 2 bytes*/
  PKT_SYNC_LENGTH_3BYTES           = 0x04, /*!< Sync length 3 bytes */
  PKT_SYNC_LENGTH_4BYTES           = 0x06 , /*!< Sync length 4 bytes */

}PktSyncLength;


/**
 * @brief  Fixed or variable payload length enumeration.
 */
typedef enum
{
  PKT_LENGTH_FIX  = 0x00,    /*!< Fixed payload length     */
  PKT_LENGTH_VAR  = 0x01     /*!< Variable payload length  */

}PktFixVarLength;

#endif


#define IS_PKT_FIX_VAR_LENGTH(LENGTH)   ((LENGTH == PKT_LENGTH_FIX) || \
                                           (LENGTH == PKT_LENGTH_VAR))
/**
* @}
*/

/** @defgroup WMBus_Phy_Exported_Constants        WMBus Phy Exported Constants
* @{
*/



/**
* @}
*/

/** @defgroup WMBus_Phy_Exported_Macros           WMBus Phy Exported Macros
* @{
*/

#define PHY_STATUS              uint8_t
#define  L_MIN(a,b)      ((a>b)?b:a)
#define  L_MAX(a,b)      ((a>b)?a:b)


/**
* @brief  This macro performs the computation of the RSSI threshold over N samples.
* @param  N number of samples to be used.
*/
#define WMBUS_PHY_COMPUTE_RSSI_THR(N)           {int32_t rssi=0;\
                                                  for(uint32_t i=0;i<10;i++){\
                                                    rssi+=(int32_t)WMBus_PhyGetIdleRssi();}\
                                                  WMBus_PhySetAttr(WMBUS_PHY_RX_RSSI_THRESHOLD, rssi/10+10);}
/**
* @}
*/

/** @defgroup WMBus_Phy_Exported_Functions        WMBus Phy Exported Functions
* @{
*/

void WMBus_PhyDeviceCmd(uint8_t en);
uint8_t* GetPhyBuffer(void);

/**
* @brief  This function handles Transmit and Receive
*         interrupt request from the Transceiver.
* @param  None
* @retval None
*/
void WMBus_PhyInterruptHandler(void);



/**
* @brief  This function will configure the SPIRIT Radio registers.This will \
*         abort the current TX and Rx transactions.
* @param  None.
* @retval PHY_STATUS.
*/
#ifdef USE_SPIRIT1_DEFAULT
PHY_STATUS WMBus_PhyInit(WMBusMode_t devMode, WMBusDeviceType_t devType);
#else
PHY_STATUS WMBus_PhyInit(WMBusMode_t devMode, WMBusDeviceType_t devType, uint8_t newMode);
#endif

/**
* @brief  This function will return the current PHY state.
* @param  *phyState: The pointer to WMBusPhyState_t type primitive.
* @retval PHY_STATUS_SUCCESS or PHY_STATUS_ERROR_INVALID_STATE 
*              if the state is a transition one.
*/
PHY_STATUS  WMBus_PhyGetState (WMBusPhyState_t *phyState);

/**
* @brief  This function will set the current PHY state.
* @param  WMBusPhyState_t phyState: The phy state.
* @retval PHY_STATUS: PHY_STATUS_SUCCESS
*/
PHY_STATUS  WMBus_PhySetState(WMBusPhyState_t phyState);

/**
* @brief  This function gets the RSSI when the radio is not receiving any packet.
* @retval RSSI value in dBm. Value 0 is error, meaning that RSSI can't be measured.
*/
int32_t WMBus_PhyGetIdleRssi(void);

/**
* @brief  This function will transmit the buffer.
* @param  *buff: The pointer to the buffer to be transmitted.
* @param  length: The length of the data buffer.
* @param  void(*TxDataSentStateHandler)(uint8_t): This callback function is 
*            invoked after successful transmission.
* @retval PHY_STATUS.
*                       
*/
PHY_STATUS  WMBus_PhyTxDataHandleMode(uint8_t *buff,
                                      uint16_t length,
                                      void(*TxDataSentStateHandler)(uint8_t));

/**
* @brief  This Routine will set the WMBus PHY attributes.
* @param  attr:The WMBus attribute
* @param  ParamVal: The value of the attribute.
* @retval PHY_STATUS: The status of execution.
*/
PHY_STATUS WMBus_PhySetAttr(WMBUS_PHY_ATTR_t attr, int32_t ParamVal);

/**
* @brief  This Routine will return the WMBus PHY attributes.
* @param  attr: The WMBus attribute
*        *ParamVal: The pointer to the variable to set the attribute value
* @retval PHY_STATUS: The status of execution
*/
PHY_STATUS WMBus_PhyGetAttr(WMBUS_PHY_ATTR_t attr, int32_t *ParamVal);


/**
* @brief  This Routine will Check wheather the communication is bidirectional.
* @param  None
* @retval Returns 1 Bidirectional communication ,Otherwise 0.
*/
uint8_t WMBus_PhyIsTwoWayMode(void);

/**
* @brief  This Routine will return the WMBus PHY attributes.
* @param  None.
* @retval Returns the Device Type.
*/
WMBusDeviceType_t WMBus_PhyGetDevType(void);

/**
* @brief  This Routine will return minimum Response time in MicroSecond.
* @param  None.
* @retval Returns Response Time.
*/
uint32_t WMBus_PhyGetResponseTimeMin(void);

/**
* @brief  This Routine will return maximum Response time in MicroSecond.
* @param  None.
* @retval Returns Response Time.
*/
uint32_t WMBus_PhyGetResponseTimeMax(void);

/**
* @brief  This Routine will return typical Response time(Tro)in MicroSecond.
* @param  None.
* @retval Returns Response Time.
*/
uint32_t WMBus_PhyGetResponseTimeTyp(void);

/**
* @brief  This Routine will return FAC delay time(Txd)in MicroSecond.
* @param  None.
* @retval Returns FAC delay time.
*/
uint32_t WMBus_PhyGetFACDelay(void);

/**
* @brief  This Routine will return FAC Timeout time(Tto)in MicroSecond.
* @param  None.
* @retval Returns FAC timeout Time.
*/
uint32_t WMBus_PhyGetFACTimeout(void);

/**
* @brief  This Routine will return Nominal Time(Tnom)in MicroSecond.
* @param  None.
* @retval Returns Nominal Time.
*/
uint32_t WMBus_PhyGetNominalTime(void);

/**
* @brief  This Routine will return number of received packets.
* @param  None.
* @retval Returns the number of received packets.
*/
uint8_t WMBus_PhyGetNumRxPackets(void);

/**
* @brief  This Routine will Pop byte from Queue.
* @param  None.
* @retval Returns  the Byte.
*/
uint8_t WMBus_PhyPopBytefromQueue(void);

/**
* @brief  This Function will return Packet Length.
* @param  None.
* @retval Returns the Packet Length.
*/
uint16_t WMBus_PhyGetPacketLength(void);

/**
* @brief  This Function will return the RSSI Value.
* @param  None.
* @retval Returns the RSSI Value.
*/
int32_t WMBus_PhyGetPacketRssi(void);

/**
* @brief  This Function will return Packet Format.
* @param  None.
* @retval Returns the Packet Format.
*/
FrameFormatType_t WMBus_PhyGetPacketFormat(void);

/**
* @brief  This Function will return the received packet submode.
* @param  None.
* @retval Returns the Packet Format.
*/
WMBusMode_t WMBus_PhyGetRecvSubmode(void);

/**
* @brief  This Function will return Time Stamp.
* @param  None.
* @retval Returns the Time Stamp.
*/
uint32_t WMBus_PhyGetPacketRxTimestamp(void);

/**
* @brief  This Function will Perform Pop Operation from the Phy queue.
* @param  None.
* @retval None.
*/
void WMBus_PhyPopDesc(void);
/**
* @brief  This callback is used to ask to the application to change the SPI speed.
* @param  low_high_speed: a flag that says if the SPI must be reduced or increased.
*            0: reduce
*            1: increase
* @retval None.
*/
void WMBUS_ChangeSpiSpeed_CB(uint8_t low_high_speed);


/**                                                                                          
* @brief  This Routine will copy byte from Queue.
* @param  destBuff: the pointer to the destination buffer.
* @param  len: Number of Byte.
* @retval None.
*/
void WMBus_PhyCopyBytefromQueue(uint8_t *destBuff,uint16_t len);


/**
* @brief  This function indicates the Valid Preamble is received.
* @param  None.
* @retval None.
*/
void WMBus_ValidPreambleRcvd(void);

/**
* @brief  This function indicates the Valid Sync is received.
* @param  None.
* @retval None.
*/
void WMBus_ValidSyncRcvd(void);

/**
* @brief  This function indicates the Valid Sync is status.
* @param  1:if valid Sync is detected, 0 : No valid Sync is deteced.
* @retval None.
*/
uint8_t WMBus_PhyCheckSyncStatus(void);

/**
* @brief  This function Resets the Flag to zero.
* @param  None.
* @retval None.
*/
void WMBus_PhyResetPreambSyncFlag(void);

/**
* @brief  This function sets the Rx Timeout in case of METER.
* @param  None
* @retval None
*/
void WMBus_PhySetRxTimeout(void);

/**
* @brief  This Function will Perform Pop Operation from the Phy queue.
* @param  None.
* @retval None.
*/
void WMBus_PhyPopDesc(void);
PHY_STATUS WMBus_PhyInit_TM(WMBusMode_t devMode, WMBusDeviceType_t devType, WMBusDeviceOperation_t devOperation );
/**
* @}
*/

/**
*@}
*/

/**
* @}
*/

/**
* @}
*/

#endif /* __WMBUS_PHY_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
