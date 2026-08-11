/**
* @file    wmbus_phy.c
* @author  SRA - NOIDA / RF application group - AMG
* @version V2.5.1
* @date    14-Sept-2020
* @brief   This file contains routines for PHYSICAL LAYER
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
#include "wmbus_phy.h"
#include "wmbus_link.h"
#include "wmbus_tim.h"

#include "S2LP_Util.h"
#include "S2LP_Radio.h"
#include "S2LP_PktWMbus.h"
#include "S2LP_Qi.h"
#include "S2LP_PktBasic.h"
#include "S2LP_Gpio.h"

#ifdef USE_STM32F4XX_NUCLEO
#include "stm32f4xx.h"
#endif

#ifdef USE_STM32L1XX_NUCLEO
#include "stm32l1xx.h"
#endif

#ifdef USE_STM32L0XX_NUCLEO
#include "stm32l0xx.h"
#endif


/** @addtogroup WMBUS_Library
* @{
*/


/** @addtogroup WMBus_PhyLayer
* @{
*/

/** @addtogroup WMBus_Phy
*   @{
*/

/** @defgroup WMBus_Phy_Private_Defines              WMBus Phy Private Defines
* @{
*/


#define RX_QUEUE_SIZE      512
#define RX_FRAME_DESC_Q_SIZE 10
#define RX_MODE 0
#define TX_MODE 1
#define DIR_O2M 1
#define DIR_M2O 0

#define AF_INITIAL_THR          12
#define DEVICE_FIFO_SIZE        128

/*Number of Byte for Almost Empty Thrshold*/
#define ALMOST_EMPTY_THRESHOLD       10  

/* Number of Byte to be written in FIFO for the 1st time */
/* : reduce to 12 bytes minimum number of Tx bytes in Tx FIFO */
//#define NUM_BYTE_FIRST_FIFO_WRITE    15  
#define NUM_BYTE_FIRST_FIFO_WRITE    12
/* -  : reduce to 12 bytes minimum number of Tx bytes in Tx FIFO */
      
      
/* These defines are used because the three submodes N and F have the last 
sync word byte associated with the frame format. Since N and F have 2 sync 
bytes, to use this variability with S2LP, the hw sync should be 1byte.
This makes the application very sensitive to the RSSI threshold because
if it is set bad the user will have a big number of false sync.
defining the symbol for one of these methods will give this behavior.
If not defined, the receiver will consider only the frame format 
configured by the user and will not deduce it from the sync allowing
S2LP to have 2bytes sync for N and F modes.
C mode too has dynamic mode, but in this case 3 bytes of sync are used.
*/
//#define DYNAMIC_PKT_FORMAT_F
//#define DYNAMIC_PKT_FORMAT_N


#define PHY_CONFIG_DELAY                2
#define PHY_BUFF_MAX_LENGTH             320     /*including crc*/
#define PHY_BUFF_OFFSET_PREAMB_SYNC     20


/*  Radio configuration parameters  */

#define	SUBMODE0_BASE_FREQUENCY         868.3e6/*!<Submode S1, S2(long header, mbus_submode=0) Base frequency*/         
#define	SUBMODE1_BASE_FREQUENCY         868.3e6/*!<Submode S1-m, S2, T2 (other to meter) (mbus_submode=1) Base frequency*/         
#define	SUBMODE3_BASE_FREQUENCY         868.95e6/*!<Submode T1, T2 (meter to other)  (mbus_submode=3) Base frequency*/        
#define	SUBMODE5_BASE_FREQUENCY         868.03e6/*!<Submode R2, short header (mbus_submode=5) Base frequency*/        
#define N_MODE_BASE_FREQUENCY           169406250/*!<Submode N1 and N2 (done by sw)*/
#define C1C2m2o_MODE_BASE_FREQUENCY     868950000/*!<Submode C1 or C2 m2o (done by sw) Base frequency*/       
#define C2o2m_MODE_BASE_FREQUENCY       869525000/*!<Submode C2 o2m (done by sw) Base frequency*/       
#define F_MODE_BASE_FREQUENCY           433.82e6/*!<Submode F2 Base frequency*/ 

#define SMODE_TMODE_CHANNEL_SPACE       100e3/*!<Submode S1/S1m/S2/T1/T2 channel space*/  
#define R2MODE_CHANNEL_SPACE            10e3/*!<Submode R2 channel space*/  
#define N_MODE_CHANNEL_SPACE            12500/*!<Submode N1/N2 channel space*/  

#define SMODE_TMODE_CHANNEL_NUMBER      0/*!<Submode S/T default channel number*/

#define S_T_R_MODE_MODULATION_SCHEME    MOD_2FSK /*FSK*//*!<S/T/R mode modulation scheme*/
#define N_MODE_MODULATION_SCHEME        MOD_2GFSK_BT05 /*GFSK_BT05*//*!<N mode modulation scheme*/
#define C1C2m2o_MODE_MODULATION_SCHEME  MOD_2FSK /*FSK*//*!<Submode C1 or C2 m2o modulation scheme*/    
#define C2o2m_MODE_MODULATION_SCHEME    MOD_2GFSK_BT05 /*GFSK_BT05*/ /*!<Submode C2 o2m modulation scheme*/  
#define F_MODE_MODULATION_SCHEME        MOD_2FSK  /*FSK*//*!<Submode F2 modulation scheme*/  

#define SUBMODE0_FREQ_DEV       50e3/*!<Submode S1, S2(long header, mbus_submode=0) Freq deviation*/
#define SUBMODE1_FREQ_DEV       50e3/*!<Submode S1-m, S2, T2 (other to meter) (mbus_submode=1) Freq deviation*/  
#define SUBMODE3_FREQ_DEV       50e3/*!<Submode T1, T2 (meter to other)  (mbus_submode=3) Freq deviation*/  
#define SUBMODE5_FREQ_DEV       6.0e3/*!<Submode R2, short header (mbus_submode=5) Freq deviation*/ 
#define N_AB_EF_MODE_FREQ_DEV   2400/*!<Submode N1(a/b/e/f) and N2(a/b/e/f) Freq deviation*/ 
#define N_CD_MODE_FREQ_DEV      2400/*!<Submode N1(c/d) and N2(c/d) Freq deviation*/ 
#define C1C2m2o_MODE_FREQ_DEV   45000/*!<Submode C1 or C2 m2o Freq deviation*/ 
#define C2o2m_MODE_FREQ_DEV     25000/*!<Submode C2 o2m Freq deviation*/ 
#define F_MODE_FREQ_DEV         5.5e3/*!<Submode F2 Freq deviation*/ 

#define SUBMODE0_BANDWIDTH      150e3/*!<Submode S1, S2(long header, mbus_submode=0) Bandwidth*/
#define SUBMODE1_BANDWIDTH      216e3/*!<Submode S1-m, S2, T2 (other to meter) (mbus_submode=1) Bandwidth*/ 
#define SUBMODE3_BANDWIDTH      250e3/*!<Submode T1, T2 (meter to other)  (mbus_submode=3) Bandwidth*/
#define SUBMODE5_BANDWIDTH      20.0e3/*!<Submode R2, short header (mbus_submode=5) Bandwidth*/
#define N_AB_EF_MODE_BANDWIDTH  8000/*!<Submode N1(a/b/e/f) and N2(a/b/e/f) Bandwidth*/
#define N_CD_MODE_BANDWIDTH     8000/*!<Submode N1(c/d) and N2(c/d) Bandwidth*/
#define C1C2m2o_MODE_BANDWIDTH  200000/*!<Submode C1 or C2 m2o Bandwidth*/
#define C2o2m_MODE_BANDWIDTH    156000/*!<Submode C2 o2m Bandwidth*/  
#define F_MODE_BANDWIDTH        20000/*!<Submode F2 Bandwidth*/  

#define SUBMODE0_DATARATE       32768/*!<Submode S1, S2(long header, mbus_submode=0) datarate*/
#define SUBMODE1_DATARATE       32768/*!<Submode S1-m, S2, T2 (other to meter) (mbus_submode=1) datarate*/ 
#define SUBMODE3_DATARATE       100000/*!<Submode T1, T2 (meter to other)  (mbus_submode=3) datarate*/  
#define SUBMODE5_DATARATE       4800/*!<Submode R2, short header (mbus_submode=5) datarate*/
#define N_AB_EF_MODE_DATARATE   4800/*!<Submode N1(a/b/e/f) and N2(a/b/e/f) datarate*/
#define N_CD_MODE_DATARATE      2400/*!<Submode N1(c/d) and N2(c/d) datarate*/
#define C1C2m2o_MODE_DATARATE   100000/*!<Submode C1 or C2 m2o datarate*/
#define C2o2m_MODE_DATARATE     50000/*!<Submode C2 o2m datarate*/  
#define F_MODE_DATARATE         2400/*!<Submode F2 datarate*/

#define N_MODE_SYNC_LENGTH            PKT_SYNC_LENGTH_2BYTES/*!<Submode N1 and N2 Synch length*/
#define N_MODE_SYNC_WORD_PACKET_A     0xF68D0000/*!<Submode N1 and N2 Synch word for frame format A*/
#define N_MODE_SYNC_WORD_PACKET_B     0xF6720000/*!<Submode N1 and N2 Synch word for frame format B*/

#define C_MODE_SYNC_LENGTH            PKT_SYNC_LENGTH_3BYTES/*!<Submode C1 or C2 Synch length*/ 
#define C_MODE_SYNC_WORD_PACKET_A     0x543D54CD/*!<Submode C1 or C2 Synch word for frame format A*/
#define C_MODE_SYNC_WORD_PACKET_B     0x543D543D/*!<Submode C1 or C2 Synch word for frame format B*/

#define F_MODE_SYNC_LENGTH            PKT_SYNC_LENGTH_2BYTES/*!<Submode F2 Synch length*/
#define F_MODE_SYNC_WORD_PACKET_A     0xF68D0000/*!<Submode F2 Synch word for frame format A*/
#define F_MODE_SYNC_WORD_PACKET_B     0xF6720000/*!<Submode F2 Synch word for frame format B*/

#define POSTABMLE_MAX_LEN             (uint8_t)8/*!<Maximum postamble length*/
#define S1_PREAMB_CHIPCOMB_LEN_MIN    (uint16_t)288/*!<Submode S1 min chip combination length*/
#define S1m_PREAMB_CHIPCOMB_LEN_MIN   (uint16_t)24/*!<Submode S1m min chip combination length*/
#define S2_PREAMB_CHIPCOMB_LEN_MIN    (uint16_t)24/*!<Submode S2 min chip combination length*/
#define T_PREAMB_CHIPCOMB_LEN_MIN     (uint16_t)24/*!<Submode T1/T2 min chip combination length*/
#define R2_PREAMB_CHIPCOMB_LEN_MIN    (uint16_t)48/*!<Submode R2 min chip combination length*/
#define N_PREAMB_CHIPCOMB_LEN_MIN     (uint16_t)8/*!<Submode N min chip combination length*/
#define N_PREAMB_CHIPCOMB_LEN_MAX     (uint16_t)8/*!<Submode N max chip combination length*/



#define PREAMBLE_LENGTH_DEFAULT      100/*!<Default preamble length*/
#define POSTAMBLE_LENGTH_DEFAULT     4/*!<Default postamble length*/

#define DEFAULT_S1_PREAMBLE_LEN      (uint16_t)279/*!<Submode S1 default preamble length*/
/* +patch   : S-mode postamble to 2 - cut2.1 issue */
//#define DEFAULT_S1_POSTAMBLE_LEN     4/*!<Submode S1 default postamble length*/
#define DEFAULT_S1_POSTAMBLE_LEN     2/*!<Submode S1 default postamble length*/
#define DEFAULT_S1m_PREAMBLE_LEN     (uint16_t)15/*!<Submode S1m default preamble length*/
//#define DEFAULT_S1m_POSTAMBLE_LEN    8/*!<Submode S1m default postamble length*/
#define DEFAULT_S1m_POSTAMBLE_LEN    2/*!<Submode S1m default postamble length*/
#define DEFAULT_S2_PREAMBLE_LEN      (uint16_t)15/*!<Submode S2 default preamble length*/
#define DEFAULT_S2_PREAMBLE_LEN_L    (uint16_t)279/*!<Submode S2 default preamble length (long)*/
//#define DEFAULT_S2_POSTAMBLE_LEN     8/*!<Submode S2 default postamble length*/
#define DEFAULT_S2_POSTAMBLE_LEN     2
/* -patch   : S-mode postamble to 2 - cut2.1 issue */
#define DEFAULT_T_PREAMBLE_LEN       (uint16_t)19/*!<Submode T1/T2 default preamble length*/
#define DEFAULT_T_POSTAMBLE_LEN      4/*!<Submode T1/T2 default postamble length*/
#define DEFAULT_R2_PREAMBLE_LEN      (uint16_t)39/*!<Submode R2 default preamble length*/          
#define DEFAULT_R2_POSTAMBLE_LEN     8/*!<Submode R2 default postamble length*/


#define PKT_PREAMBLE_LENGTH_02BYTES      2
#define PKT_PREAMBLE_LENGTH_04BYTES      4
#define PKT_PREAMBLE_LENGTH_10BYTES      10

#define DEFAULT_N_MODE_PREAMBLE_LENGTH   PKT_PREAMBLE_LENGTH_02BYTES/*!<Submode N1/N2 default preamble length*/  
#define DEFAULT_N_MODE_POSTAMBLE_LENGTH  0/*!<Submode N1/N2 default postamble length*/
#define DEFAULT_C_MODE_PREAMBLE_LENGTH   PKT_PREAMBLE_LENGTH_04BYTES/*!<Submode C1/C2 default preamble length*/
#define DEFAULT_C_MODE_POSTAMBLE_LENGTH  0/*!<Submode C1/C2 default postamble length*/
#define DEFAULT_F2_MODE_PREAMBLE_LEN     PKT_PREAMBLE_LENGTH_10BYTES/*!<Submode F2 default preamble length*/          
#define DEFAULT_F2_MODE_POSTAMBLE_LEN    0/*!<Submode F2 default postamble length*/


#define T_RESPONSE_TIME_MAX_MS          3/*!<Submode T1/T2 max response time(ms)*/
#define T_RESPONSE_TIME_MIN_MS          2/*!<Submode T1/T2 min response time(ms)*/
#define T_RESPONSE_TIME_TYP_MS          2.5/*!<Submode T1/T2 typical response time(ms)*/
/* management of 2 Rx timers for T-mode meter */
/* in T-mode preamble + Sync pattern is 48 bits @ 32,768kbps => 1,46ms */
//#define T_RESPONSE_TIME_FOR_SYNC_PATTERN_US 1500
#define T_RESPONSE_TIME_FOR_SYNC_PATTERN_US 2300
#define RESPONSE_TIME_FOR_SYNC_PATTERN_US_MARGIN 1000 /* margin needed if no fast Rx timer used */

#define S_RESPONSE_TIME_MAX_MS          50/*!<Submode S max response time(ms)*/
#define S_RESPONSE_TIME_MIN_MS          3/*!<Submode S min response time(ms)*/
#define S_RESPONSE_TIME_TYP_MS          5/*!<Submode S typical time(ms)*/

#define R_METER_RESPONSE_TIME_MAX_MS    10000/*!<Submode R2(meter) max response time(ms)*/ 
#define R_METER_RESPONSE_TIME_MIN_MS    10/*!<Submode R2(meter) min response time(ms)*/
#define R_METER_RESPONSE_TIME_TYP_MS    15/*!<Submode R2(meter) typical time(ms)*/

#define R_OTHER_RESPONSE_TIME_MAX_MS    50/*!<Submode R2(other) max response time(ms)*/
#define R_OTHER_RESPONSE_TIME_MIN_MS    3/*!<Submode R2(other) min response time(ms)*/
#define R_OTHER_RESPONSE_TIME_TYP_MS    5/*!<Submode R2(other) typical time(ms)*/

#define N_FAST_RESPONSE_TIME_MAX_MS     100.5/*!<Submode N1/N2 max response time(ms)*/
#define N_FAST_RESPONSE_TIME_MIN_MS     99.5/*!<Submode N1/N2 min response time(ms)*/
#define N_FAST_RESPONSE_TIME_TYP_MS     100/*!<Submode N1/N2 typical response time(ms)*/

//#define C_SLOW_RESPONSE_TIME_MAX_MS     1000.5/*!<Submode C1/C2(slow) max response time(ms)*/
//#define C_SLOW_RESPONSE_TIME_MIN_MS     999.5/*!<Submode C1/C2(slow) min response time(ms)*/
//#define C_SLOW_RESPONSE_TIME_TYP_MS     1000.0/*!<Submode C1/C2(slow) typical response time(ms)*/
#define C_SLOW_RESPONSE_TIME_MAX_MS     100.5/*!<Submode C1/C2(slow) max response time(ms)*/
#define C_SLOW_RESPONSE_TIME_MIN_MS     99.5/*!<Submode C1/C2(slow) min response time(ms)*/
#define C_SLOW_RESPONSE_TIME_TYP_MS     100.0/*!<Submode C1/C2(slow) typical response time(ms)*/

#define C_FAST_RESPONSE_TIME_MAX_MS     100.5/*!<Submode C1/C2(fast) max response time(ms)*/
#define C_FAST_RESPONSE_TIME_MIN_MS     99.5/*!<Submode C1/C2(fast) min response time(ms)*/
#define C_FAST_RESPONSE_TIME_TYP_MS     100.0/*!<Submode C1/C2(fast) typical response time(ms)*/

/* management of 2 Rx timers for C-mode meter */
/* in C-mode preamble + Sync pattern is 64 bits @ 50kbps => 1,28ms */
#define C_RESPONSE_TIME_FOR_SYNC_PATTERN_US 1300

#define F_METER_RESPONSE_TIME_MAX_MS    4000/*!<Submode F meter max response time(ms)*/
#define F_METER_RESPONSE_TIME_MIN_MS    3/*!<Submode F meter min response time(ms)*/
#define F_METER_RESPONSE_TIME_MIN_TYP   50/*!<Submode F meter typical response time(ms)*/ 

#define F_OTHER_SLOW_RESPONSE_TIME_MAX_MS    1000.5/*!<Submode F other (slow) max response time(ms)*/
#define F_OTHER_SLOW_RESPONSE_TIME_MIN_MS    999.5/*!<Submode F other (slow) min response time(ms)*/
#define F_OTHER_SLOW_RESPONSE_TIME_TYP_MS    1000/*!<Submode F other (slow) typical response time(ms)*/

#define F_OTHER_FAST_RESPONSE_TIME_MAX_MS    100.5/*!<Submode F other (fast) max response time(ms)*/
#define F_OTHER_FAST_RESPONSE_TIME_MIN_MS    99.5/*!<Submode F other (slow) min response time(ms)*/
#define F_OTHER_FAST_RESPONSE_TIME_TYP_MS    100/*!<Submode F other (slow) typical response time(ms)*/

#define S2_FAC_N_VALUE    2/*!<S2 mode FAC Value: N = 2, 3 or 5 */
#define T2_FAC_N_VALUE    2/*!<T2 mode FAC Value: N = 2, 3 or 5 */
#define C2_FAC_N_VALUE    2/*!<C2 mode FAC Value: N = 2, 3 or 5 */
#define R2_FAC_N_VALUE    5/*!<R2 mode FAC Value: N = 5, 7 or 13 */
#define N2_FAC_N_VALUE    5/*!<N2 mode FAC Value: N = 5, 7 or 13 */
#define F2_FAC_N_VALUE    2/*!<F2 mode FAC Value: N = 2, 3 or 5 */

#define S2_FAC_TRANSMISSION_DELAY  (1000*S2_FAC_N_VALUE)/*!<S2 mode FAC delay*/
#define S2_FAC_TIME_OUT             30000/*!<S2 mode FAC Timeout: min = 25s and max = 30s*/

#define T2_FAC_TRANSMISSION_DELAY  (1000*T2_FAC_N_VALUE)/*!<T2 mode FAC delay*/
#define T2_FAC_TIME_OUT             30000/*!<T2 mode FAC Timeout: min = 25s and max = 30s*/

#define C2_FAC_TRANSMISSION_DELAY  (1000*C2_FAC_N_VALUE)/*!<C2 mode FAC delay*/
#define C2_FAC_TIME_OUT             30000/*!<C2 mode FAC Timeout: min = 25s and max = 30s*/

#define R2_FAC_TRANSMISSION_DELAY  (1000*R2_FAC_N_VALUE)/*!<R2 mode FAC delay*/
#define R2_FAC_TIME_OUT             30000/*!<R2 mode FAC Timeout: min = 25s and max = 30s*/

#define N2_FAC_TRANSMISSION_DELAY   (1000*N2_FAC_N_VALUE)/*!<N2 mode FAC delay*/
#define N2_FAC_TIME_OUT              30000/*!<N2 mode FAC Timeout: min = 25s and max = 30s*/

#define F2_FAC_TRANSMISSION_DELAY   (1000*F2_FAC_N_VALUE)/*!<F2 mode FAC delay*/
#define F2_FAC_TIME_OUT                30000/*!<F2 mode FAC Timeout: min = 25s and max = 30s*/

#define S_NOMINAL_TRANSMISSION_INTERVAL         720e3/*!<Submode S nominal time*/
#define T_C_NOMINAL_TRANSMISSION_INTERVAL       90e3/*!<Submode T/C nominal time*/
#define R_N_F_H_NOMINAL_TRANSMISSION_INTERVAL   86400e3/*!<Submode R/N/F/H nominal time*/


#define RESPONSE_TIME_GUARD             0.5

/**
* @}
*/

/** @defgroup WMBus_Phy_Private_TypesDefinitions  WMBus Phy Private Types Definitions
* @{
*/

/**
* @brief  Transmission state machine state enum
*/
enum
{
  TX_STATUS_SUCCESS = 0x00,
  TX_STATUS_ERROR,
  TX_STATUS_TIMEOUT,
  TX_STATUS_PACKET_WRITTEN,
  TX_STATUS_FIFO_ALMOST_EMPTY
};

/**
* @brief  Reception state machine state enum
*/
enum
{
  RX_STATUS_SUCCESS = 0x00,
  RX_STATUS_ERROR,
  RX_STATUS_TIMEOUT,
  RX_STATUS_FIFO_ALMOST_FULL
};

/**
* @brief  Reception state machine state typedef
*/
typedef enum{
  SM_RX_FIRST=0,
  SM_RX_INTERM,
  SM_RX_LAST,
} PhyRxState_t;

/**
* @brief  Frame descriptor at phy level
*/
typedef struct{
  uint16_t size;
  FrameFormatType_t format;
  int32_t rssi;
  uint32_t rx_timestamp;
  WMBusMode_t dynamic_submode;
  uint32_t start_index;
} PhyFrameDesc_t;

/**
* @brief  Phy Frame typedef structure
*/
typedef struct
{
  uint8_t* DataBuff;
  uint16_t length;
  uint16_t lenRemain;
}PHY_Frame_t;

/**
* @brief  S2LP registor config typedef structure
*/
typedef struct {
  uint8_t regs_from_05[4];   /* SYNTH[0:3] regs (base frequency) */
  uint8_t regs_from_0E[6];   /* MOD[] regs (modulation scheme, datarate,fdev,ch_filt) */
  uint8_t regs_from_2B[5];   /* packet format*/
  uint8_t regs_from_33[4];   /* sync */
  uint8_t regs_from_38[1];   /* postamble */
  uint8_t regs_from_46[2];   /* RX timeout */
  uint8_t regs_from_31[1];   /* packet type */
} S2LPRegConf_t;

/**
* @brief  WMBus Phy layer Attributes typedef structure for WMBus Phy layer
*/
typedef struct
{
  WMBusMode_t   DevMode;
  WMBusDeviceType_t  DevType;/*!<WMbus Device type*/
  uint8_t  channel;/* For R2 only, otherwise it is 0*/
  float ResponseTimeMin;/*!<response time min*/ 
  float ResponseTimeMax;/*!<response time max*/ 
  float ResponseTimeTyp;/*!<response time typ*/ 
  float FACDelay;/*!<FAC time delay*/ 
  float FACTimeout;/*!<FAC timeout*/ 
  float NominalTime;
  uint16_t  PreambleLength;/*!<Preamble length*/
  uint8_t  PostambleLength;/*!<Postamble length*/ 
  float  RFPower;/*!<RF Power*/
  int32_t RxRSSIThreshold;/*RSSI Threshold for RX */
  uint32_t TxInterval;/*!<Tx Interval*/
  int32_t RFOffset; /* Offset of the RF device */
  float RxTimeout;  /*<RX period to seek the SYNC*/
  uint8_t  Direction;/*!<0--> Meter to other, 1--> other to meter */
  uint32_t CustDatarate;/*!<if it is 0 (default), the datarate associated with the submode will be set, else, the datarate will be this one*/
  uint8_t CsmaEnabled;
  int32_t CsmaRSSIThreshold;/*!<RSSI Threshold for CSMA*/
  uint8_t CsmaOverrideFail;
  uint8_t CsmaCcaPeriod; /*!< RSSI meas cycle = (CsmaCcaPeriod+1)*64*TBIT*/
  uint8_t CsmaCcaLength; /*!<times of CsmaCcaPeriod for ch assesment */
  uint8_t CsmaMaxNb; /*!<Max N backoff */
  uint16_t CsmaBuSeed; /*!<seed for rand in (2^rand)* presc */
  uint8_t CsmaBuPrescaler; /*!<presc for (2^rand)*presc */
  uint8_t FrameFormat;
}WMBus_PhyAttr_t;


/**
* @brief  WMBus Phy layer Rx RSSI Threshold initialization
*/
int16_t RxRSSIThTable[13][2] = {{-100/*OTHER*/,-104/*METER*/},/*S1_MODE  */
                                {-100/*OTHER*/,-104/*METER*/},/*S1m_MODE */
                                {-100/*OTHER*/,-104/*METER*/},/*S2_MODE */
                                {-100/*OTHER*/,-104/*METER*/},/*T1_MODE */
                                {-100/*OTHER*/,-104/*METER*/},/*T2_MODE */
                                {-100/*OTHER*/,-104/*METER*/},/*R2_MODE */
                                {-100/*OTHER*/,-104/*METER*/},/*ALL_MODE */
                                {-100/*OTHER*/,-104/*METER*/},/*N1_MODE */
                                {-100/*OTHER*/,-104/*METER*/},/*N2_MODE */
                                {-100/*OTHER*/,-104/*METER*/},/*F2_MODE */
                                {-100/*OTHER*/,-104/*METER*/},/*F2m_MODE */
                                {-100/*OTHER*/,-104/*METER*/},/*C1_MODE */
                                {-100/*OTHER*/,-104/*METER*/}/*C2_MODE */
                                };                            


/**
* @brief  WMBus Phy layer attributes initialization
*/
WMBus_PhyAttr_t  WMBusPhyAttr =
{
  .DevMode = S2_MODE, 
  .DevType = OTHER,
  .channel = CHANNEL_1,
  .FrameFormat = FRAME_FORMAT_A,
  .ResponseTimeMin = S_RESPONSE_TIME_MIN_MS,
  .ResponseTimeMax = S_RESPONSE_TIME_MAX_MS,
  .ResponseTimeTyp = S_RESPONSE_TIME_TYP_MS,
  .FACDelay = S2_FAC_TRANSMISSION_DELAY,
  .FACTimeout = S2_FAC_TIME_OUT,
  .NominalTime = S_NOMINAL_TRANSMISSION_INTERVAL,
  .PreambleLength = DEFAULT_S2_PREAMBLE_LEN,
  .PostambleLength = DEFAULT_S2_POSTAMBLE_LEN, 
  .RFPower = 5.0,
  .RxRSSIThreshold = -100,
  .RFOffset =0,
  .TxInterval = 3600,
  .Direction=DIR_M2O,
  .CustDatarate=0,
  .CsmaEnabled=0,
  .CsmaRSSIThreshold = -80,
  .CsmaOverrideFail=0,
  .CsmaCcaPeriod=0, 
  .CsmaCcaLength=0,
  .CsmaMaxNb=3,
  .CsmaBuSeed=0xfa21,
  .CsmaBuPrescaler=32
};

static uint8_t PhyDataBuff[PHY_BUFF_MAX_LENGTH+PHY_BUFF_OFFSET_PREAMB_SYNC];
volatile uint8_t ValidPreambleFlag = 0,ValidSyncFlag = 0;              

static volatile SBool RxPER_flag = S_FALSE ;

/**
* @brief  WMBus Phy Tx Frame initialization
*/
PHY_Frame_t  PhyTxFrame = 
{
  .length = 0x00,
  .lenRemain = 0x00
};


/**
* @}
*/

/** @defgroup WMBus_Phy_Private_Macros                 WMBus Phy Private Macros
* @{
*/


#define APPEND_RX_QUEUE_FROM_FIFO(N)    {\
if(RxQueueHeadNew+N>RX_QUEUE_SIZE)\
  {\
    S2LPSpiReadFifo(RX_QUEUE_SIZE-RxQueueHeadNew, \
      &RxBuffQueue[RxQueueHeadNew]);\
        S2LPSpiReadFifo(N-(RX_QUEUE_SIZE-RxQueueHeadNew),\
          &RxBuffQueue[0]);\
            RxQueueHeadNew=N-(RX_QUEUE_SIZE-RxQueueHeadNew);\
  }\
else\
  {\
    S2LPSpiReadFifo(N, &RxBuffQueue[RxQueueHeadNew]);\
      RxQueueHeadNew=(RxQueueHeadNew+N)%RX_QUEUE_SIZE;\
  }\
    RxFrameDescQueue[RxFrameDescHead].size += N;\
                                        }


#define APPEND_RX_QUEUE_BYTE(B)    {\
  RxBuffQueue[RxQueueHeadNew]=B;\
  RxQueueHeadNew=(RxQueueHeadNew+1)%RX_QUEUE_SIZE;\
  RxFrameDescQueue[RxFrameDescHead].size++;\
  }


#define  LSB_U16(NUM)  ((uint8_t)(NUM &0xFF)) 
#define  MSB_U16(NUM)  ((uint8_t)((NUM>>8) &0xFF)) 
#define CEILING(X)     ((X-(uint16_t)(X)) > 0 ? (uint16_t)(X+1) : (uint16_t)(X))

#define TX_STATUS               uint8_t
#define RX_STATUS               uint8_t


#define IS_WMBUS_MODE(MODE) (((MODE) == S1_MODE) || ((MODE) == S1m_MODE)|| \
((MODE) == S2_MODE)|| ((MODE) == T1_MODE) ||((MODE) == T2_MODE)|| \
  ((MODE) == R2_MODE)|| ((MODE) == N1_MODE)|| ((MODE) == N2_MODE)||\
    ((MODE) == C1_MODE)||((MODE) == C2_MODE)||((MODE) == F2m_MODE)||\
      ((MODE) == F2_MODE)||((MODE) == ALL_MODE)|| ((MODE) == INVALID_MODE))

#define IS_WMBUS_DEVICE_TYPE(DEV_TYPE) (((DEV_TYPE) == METER) || \
((DEV_TYPE) == OTHER)|| ((DEV_TYPE) == ROUTER)|| ((DEV_TYPE)\
  == SNIFFER)|| ((DEV_TYPE) == NOT_CONFIGURED))

#define IS_WMBUS_CHANNEL_NUM(CHAN_NUM) (((CHAN_NUM) == CHANNEL_1) ||\
((CHAN_NUM) == CHANNEL_2) || ((CHAN_NUM) == CHANNEL_3) ||\
  ((CHAN_NUM) == CHANNEL_4) || ((CHAN_NUM) == CHANNEL_5) ||\
    ((CHAN_NUM) == CHANNEL_6) || ((CHAN_NUM) == CHANNEL_7) ||\
      ((CHAN_NUM) == CHANNEL_8) || ((CHAN_NUM) == CHANNEL_9) ||\
        ((CHAN_NUM) == CHANNEL_10) || ((CHAN_NUM) == CHANNEL_1a) ||\
          ((CHAN_NUM) == CHANNEL_1b) || ((CHAN_NUM) == CHANNEL_2a) ||\
            ((CHAN_NUM) == CHANNEL_2b) || ((CHAN_NUM) == CHANNEL_3a) ||\
              ((CHAN_NUM) == CHANNEL_3b) || ((CHAN_NUM) == Invalid_Channel))

/**
* @}
*/



/** @defgroup WMBus_Phy_Private_Variables    WMBus Phy Private Variables
* @{
*/

/* Reception state machine state */
static PhyRxState_t rx_state=SM_RX_FIRST;

/* Buffer to Rx from S2LP. It is a circular buffer, 
*   so we need a head, a tail,
* and a vector of frames size in order to identify the different frames queued
* into the buffer */
static uint8_t RxBuffQueue[RX_QUEUE_SIZE];

/* Head of the data packet queue. Tail is no more used because the starting point of the
   packet is now stored in the packet descriptor struct ( @ref PhyFrameDesc_t ).
*/
static uint16_t RxQueueHead=0;/*,RxQueueTail=0;*/

/* This is an auxiliary variable to update the head of the queue during the 
reception SM in the ISR in order to avoid race condition. 
At the end of the reception it will be copied into the RxQueueHead. */
static uint16_t RxQueueHeadNew=0;

/* Queue of packets descriptors */
static PhyFrameDesc_t RxFrameDescQueue[RX_FRAME_DESC_Q_SIZE];

/* Head and tail of desc */
static uint16_t RxFrameDescHead=0,RxFrameDescTail=0;

/* Number of packets in queue */
static uint16_t nRxPackets=0;

static volatile SBool tx_done_flag=S_FALSE, \
  rx_done_flag=S_FALSE, rx_timeOut=S_FALSE;

/* To store the actual status */
static SRadioInit radioInit_p;
static PktWMbusInit MbusInitVal_p;
static float RFPower_p,RxTim_p;
static int32_t RSSIThr_p;

static S2LPRegConf_t configurationRegs[2];
static uint8_t last_s2lp_regs_index;
static uint8_t phy_init_called=0;
static uint8_t ex_cmd=1;

//#ifdef DEVICE_TYPE_METER
static uint8_t WMBusInitRegisters[122];
//#endif
/**
* @}
*/

/** @defgroup WMBus_Phy_Private_FunctionPrototypes   WMBus Phy Private Function Prototypes
* @{
*/
void(*PhyTxDataSentStateHandler)(uint8_t);
void WMBus_LinkRxTimeoutHandler(void);
void ResetActualStatus(void);
void ReInitRxCycle(void);
static void SaveConfRegs(uint8_t index);
static void LoadConfRegs(uint8_t index);
static void StoreS2lpRegsBothDir(void);
static void packetReceptionSM(void);
static void WMBus_PhySetDefaultPars(void);
static uint8_t WMBus_PhyConfigFull(void);
static int RadioCmp(SRadioInit *r1, SRadioInit *r2);
static int WMbusCmp(PktWMbusInit *w1, PktWMbusInit *w2);
static int WMbusCmp(PktWMbusInit *w1, PktWMbusInit *w2);
PHY_STATUS WMBus_PhyConfig(uint8_t trx_mode);
PHY_STATUS  WMBus_PhyReadConfig(WMBus_PhyAttr_t *WMBusCurrPar);
void WMBUS_Init_RxPerformanceSettings(void);
/**
* @}
*/

/** @defgroup WMBus_Phy_Private_Functions         WMBus Phy Private Functions
* @{
*/

uint8_t* GetPhyBuffer(void)
{
  return &PhyDataBuff[PHY_BUFF_OFFSET_PREAMB_SYNC];
}

/**
* @brief  This function Resets actual status.
* @param  None.
* @retval None.
*/
void ResetActualStatus(void)
{
  memset(&radioInit_p,0,sizeof(SRadioInit));
  memset(&MbusInitVal_p,0,sizeof(PktWMbusInit));
  RFPower_p=-1000;
  RSSIThr_p=100; 
  RxTim_p=0;
}

//#ifdef DEVICE_TYPE_METER
static void SaveWMBusInitRegisters()
{
  S2LPSpiReadRegisters(0x00, 0x7A, WMBusInitRegisters);

}

/*static */ void LoadWMBusInitRegisters()
{
  S2LPSpiWriteRegisters(0x00, 0x7A, WMBusInitRegisters);

}
//#endif


/**
* @brief  This function will save Configuration Register.
* @param  index: Index number.
* @retval None.
*/
static void SaveConfRegs(uint8_t index)
{
  S2LPSpiReadRegisters(0x05, 4, configurationRegs[index].regs_from_05);
  S2LPSpiReadRegisters(0x0e, 6, configurationRegs[index].regs_from_0E);
  S2LPSpiReadRegisters(0x2b, 5, configurationRegs[index].regs_from_2B);
  S2LPSpiReadRegisters(0x33, 4, configurationRegs[index].regs_from_33);
  S2LPSpiReadRegisters(0x38, 1, configurationRegs[index].regs_from_38);
  S2LPSpiReadRegisters(0x46, 2, configurationRegs[index].regs_from_46);
  last_s2lp_regs_index=index;
}

/**
* @brief  This function will Load Configuration Register on S2LP.
* @param  index: Index number.
* @retval None.
*/
static void LoadConfRegs(uint8_t index)
{
  /*only for sucmodes that have configs different 
  by direction, update the S2LP regs*/
  if(last_s2lp_regs_index!=index)
  {
    if(memcmp(configurationRegs[index].regs_from_05,\
      configurationRegs[1-index].regs_from_05,4)!=0)
      S2LPSpiWriteRegisters(0x05, 4, configurationRegs[index].regs_from_05);
    
    if(memcmp(configurationRegs[index].regs_from_0E\
      ,configurationRegs[1-index].regs_from_0E,6)!=0)
      S2LPSpiWriteRegisters(0x0e, 6, configurationRegs[index].regs_from_0E);
    
    if(memcmp(configurationRegs[index].regs_from_2B,\
      configurationRegs[1-index].regs_from_2B,5)!=0)
      S2LPSpiWriteRegisters(0x2b, 5, configurationRegs[index].regs_from_2B);
    
    if(memcmp(configurationRegs[index].regs_from_33,\
      configurationRegs[1-index].regs_from_33,4)!=0)
      S2LPSpiWriteRegisters(0x33, 4, configurationRegs[index].regs_from_33);
    
    if(memcmp(configurationRegs[index].regs_from_38,\
      configurationRegs[1-index].regs_from_38,1)!=0)
      S2LPSpiWriteRegisters(0x38, 1, configurationRegs[index].regs_from_38);
    
    if(memcmp(configurationRegs[index].regs_from_46,\
      configurationRegs[1-index].regs_from_46,2)!=0)
      S2LPSpiWriteRegisters(0x46, 2, configurationRegs[index].regs_from_46);
    
    last_s2lp_regs_index=index;
  }
}


/**
* @brief  This function will save/Load Configuration Register on S2LP.
* @param  None.
* @retval None.
*/
static void StoreS2lpRegsBothDir(void)
{
    WMBusPhyState_t phyState;
    
    while(WMBus_PhyGetState(&phyState)==PHY_STATUS_ERROR_INVALID_STATE);
    
    
    WMBus_PhySetState(PHY_STATE_READY);
    
    S2LPCmdStrobeSabort();
      
    if(WMBusPhyAttr.DevMode == T2_MODE || WMBusPhyAttr.DevMode == R2_MODE ||\
      WMBusPhyAttr.DevMode == C2_MODE || WMBusPhyAttr.DevMode == F2_MODE || WMBusPhyAttr.DevMode == F2m_MODE )    
    {
      /* calculate it for the direction opposite to the one passed */
      WMBusPhyAttr.Direction = 1-WMBusPhyAttr.Direction;
      /* anyway, if we are here it means that the 
      submode has radio config different M2O,O2M*/
      /*0-Meter to other,1-other to meter*/
      if(WMBus_PhyConfigFull())
      {
        /* here dump the registers of radio and store it in the 
          1-WMBusPhyAttr.Direction array entry */
        SaveConfRegs(WMBusPhyAttr.Direction);
//#ifdef DEVICE_TYPE_METER
          if (phy_init_called != 1)
          {
          SaveWMBusInitRegisters();
          }
//#endif
      }
      /* calculate for the direction passed */
      /*0-Meter to other,1-other to meter*/
      WMBusPhyAttr.Direction = 1-WMBusPhyAttr.Direction;    
    }
    
    if(WMBus_PhyConfigFull())
    {
      /* here dump the registers of radio and store it in the 
        WMBusPhyAttr.Direction array entry */
      SaveConfRegs(WMBusPhyAttr.Direction);
      /* +patch   : S2-mode is same configuration for Tx and Rx */
//#ifdef DEVICE_TYPE_METER
      /* only in case of S2-mode as same configuration for Tx and Rx */
          //if ((phy_init_called != 1) && (WMBusPhyAttr.DevMode == S2_MODE))
          if (phy_init_called != 1)
          {
          SaveWMBusInitRegisters();
          }
//#endif
      /* -patch   : S2-mode is same configuration for Tx and Rx */
    }
    /* else they are the same and we copy the structures */
    if(!(WMBusPhyAttr.DevMode == T2_MODE || WMBusPhyAttr.DevMode == R2_MODE ||\
      WMBusPhyAttr.DevMode == C2_MODE || WMBusPhyAttr.DevMode == F2_MODE || WMBusPhyAttr.DevMode == F2m_MODE))   
    {
      configurationRegs[1-WMBusPhyAttr.Direction] = \
        configurationRegs[WMBusPhyAttr.Direction];
    }
    
   WMBus_PhySetState(phyState);
}



/**
* @brief  This function will initialize the Transceiver Radio registers.
* @param  devMode:Device Mode
* @param  devType:Device Type
* @retval PHY_STATUS:
*                    PHY_STATUS_SUCCESS
*                    PHY_STATUS_ERROR_INVALID_DEVICE_TYPE.
*/
PHY_STATUS WMBus_PhyInit(WMBusMode_t devMode, WMBusDeviceType_t devType, uint8_t newMode)
{ 
  S2LPExitShutdown();  
  S2LPCmdStrobeSres();
  
  if (newMode == 1)
  {
    /* reset phy_init_called */
    phy_init_called = 0;
  }
  
#ifdef DEVICE_TYPE_METER
  /* only one inizitalization if Meter */
  if (phy_init_called != 1)
  {
#endif
  /* first init not done */
  
  S2LPManagementRangeExtInit();
  
#ifdef S2LP_GPIO_DEBUG
  
  S2LPGpioInit(&(SGpioInit){S2LP_GPIO_2,
               S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
               S2LP_GPIO_DIG_OUT_RX_STATE});
  S2LPGpioInit(&(SGpioInit){S2LP_GPIO_1,
               S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
               S2LP_GPIO_DIG_OUT_TX_STATE});  
  
   S2LPGpioInit(&(SGpioInit){S2LP_GPIO_0,
               S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
               S2LP_GPIO_DIG_OUT_RX_DATA}); 
#endif
  
 /* IRQ GPIO */
 SGpioInit xGpioIRQ={
   S2LP_GPIO_3,
   S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
   S2LP_GPIO_DIG_OUT_IRQ
 };
    
  /* S2LP IRQ config */
 S2LPGpioInit(&xGpioIRQ);
 
  if((IS_WMBUS_DEVICE_TYPE(devType))&&(IS_WMBUS_MODE(devMode)))
  {   
    
    WMBusPhyAttr.DevMode = devMode;
    WMBusPhyAttr.DevType = devType;
    
    WMBus_PhySetDefaultPars();
   
    /* Application setting to be set to improve performance */
    S2LPRadioCsBlanking(S_ENABLE);
    
    S2LPTimerSetRxTimerStopCondition(SQI_ABOVE_THRESHOLD);
    
    SET_INFINITE_RX_TIMEOUT();
    
    /* +patch   : initialize settings for Rx performances tests */
    /* AFC, Clockrecovery and RSSI threshold */
    WMBUS_Init_RxPerformanceSettings();
    /* +patch   : initialize settings for Rx performances tests */
    
    S2LPCsma(S_DISABLE);
    
    ResetActualStatus();
    StoreS2lpRegsBothDir();
    
    phy_init_called=1;
    
    
    
    if(WMBusPhyAttr.DevType!=METER)
    {
      WMBus_PhySetState(PHY_STATE_RX);
    }
    return PHY_STATUS_SUCCESS;
  }
  else
  {
    return PHY_STATUS_ERROR_INVALID_DEVICE_TYPE;
  }

#ifdef DEVICE_TYPE_METER  
  }

  else
  {
  
  if((IS_WMBUS_DEVICE_TYPE(devType))&&(IS_WMBUS_MODE(devMode)))
  {
    
    LoadWMBusInitRegisters();
    
    if(WMBusPhyAttr.DevType!=METER)
    {
      WMBus_PhySetState(PHY_STATE_RX);
    }
    return PHY_STATUS_SUCCESS;
  }
  else
  {
    return PHY_STATUS_ERROR_INVALID_DEVICE_TYPE;
  }
    
    
  }
#endif
}


PHY_STATUS WMBus_PhyInit_TM(WMBusMode_t devMode, WMBusDeviceType_t devType, WMBusDeviceOperation_t devOperation )
{ 
  S2LPExitShutdown();  
  S2LPCmdStrobeSres();
   
  S2LPManagementRangeExtInit();
  
#ifdef S2LP_GPIO_DEBUG
  S2LPGpioInit(&(SGpioInit){S2LP_GPIO_2,
               S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
               S2LP_GPIO_DIG_OUT_RX_STATE});
  S2LPGpioInit(&(SGpioInit){S2LP_GPIO_1,
               S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
               S2LP_GPIO_DIG_OUT_TX_STATE});  
  
   S2LPGpioInit(&(SGpioInit){S2LP_GPIO_0,
               S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
               S2LP_GPIO_DIG_OUT_RX_DATA}); 
#endif
 /* IRQ GPIO */
 SGpioInit xGpioIRQ={
   S2LP_GPIO_3,
   S2LP_GPIO_MODE_DIGITAL_OUTPUT_LP,
   S2LP_GPIO_DIG_OUT_IRQ
 };
    
  /* S2LP IRQ config */
 S2LPGpioInit(&xGpioIRQ);  
  
  if((IS_WMBUS_DEVICE_TYPE(devType))&&(IS_WMBUS_MODE(devMode)))
  {   

    WMBusPhyAttr.DevMode = devMode;
    WMBusPhyAttr.DevType = devType;
    
    WMBus_PhySetDefaultPars();

    
    /* Application setting to be set to improve performance */
    S2LPRadioCsBlanking(S_ENABLE);
    
    S2LPTimerSetRxTimerStopCondition(SQI_ABOVE_THRESHOLD);
    
    SET_INFINITE_RX_TIMEOUT();    

    /* +patch   : initialize settings for Rx performances tests */
    /* AFC, Clockrecovery and RSSI threshold */
    WMBUS_Init_RxPerformanceSettings();
    /* +patch   : initialize settings for Rx performances tests */
    
    
    S2LPCsma(S_DISABLE);
    
    ResetActualStatus();
    StoreS2lpRegsBothDir();
    
    switch(devOperation)
    {
      
    case TXCW :
      {
      WMBus_PhySetState(PHY_STATE_TXCW);
      break;
      }
      
    case TXPN9 : 
      {
      WMBus_PhySetState(PHY_STATE_TXPN9);
      break;
      }
      
    case RXPER :
      {
      RxPER_flag = S_TRUE;
      WMBus_PhySetState(PHY_STATE_RX);
      break;
      } 
      
      case TX_TEST :
      {
        break;
      }
      
    case NORMAL : case NO_OPERATION:
    	break;

    }
    
    return PHY_STATUS_SUCCESS;
  }
  else
  {
    return PHY_STATUS_ERROR_INVALID_DEVICE_TYPE;
  }
}




/**
* @brief  This function will Handle Packet Reception  State Machine.
* @param  None.
* @retval None.
*/
static void packetReceptionSM(void)
{
  static uint16_t Length=0x00,ResidualLength=0x00,nRxIndex=0x00;
  
  switch(rx_state)
  {
  case SM_RX_FIRST:
    {
      uint8_t cThreholdRxFifoAF=0x00;
      
      /* copy the head of the data packets queue 
      in the temporary variable of head */
      RxQueueHeadNew=RxQueueHead;
      
      FrameFormatType_t frame_format=WMBus_LinkGetFormat();      
      RxFrameDescQueue[RxFrameDescHead].dynamic_submode=WMBusPhyAttr.DevMode;
      nRxIndex=AF_INITIAL_THR;
      
      /* zero out the size */
      RxFrameDescQueue[RxFrameDescHead].size=0;
      
      
      
#ifdef DYNAMIC_PKT_FORMAT_F
      if(WMBusPhyAttr.DevMode==F2_MODE || WMBusPhyAttr.DevMode==F2m_MODE)
      {
         uint8_t last_sync_byte=0x00;
         S2LPSpiReadFifo(1, &last_sync_byte);
         
        nRxIndex--;
        
        if(last_sync_byte==0x8D)
        {
          frame_format=FRAME_FORMAT_A;
        }
        else if(last_sync_byte==0x72)
        {
          frame_format=FRAME_FORMAT_B;
        }
        else
        {
          S2LPGpioIrqClearStatus();
           
          S2LPGpioIrqConfig(RX_FIFO_ALMOST_FULL,S_DISABLE);
          S2LPCmdStrobeSabort();
          
          ReInitRxCycle();
          WMBus_PhySetState(PHY_STATE_RX);
          
          return;
        }
      }
#endif
      
      
      RxFrameDescQueue[RxFrameDescHead].format=frame_format;
      
      /* Fill the queue with the AF_INITIAL_THR bytes received */
      APPEND_RX_QUEUE_FROM_FIFO(nRxIndex);
      
      /* Get the L-field in the circular queue.
      It is the first byte in the FIFO at this stage so it is the 
      actual head of the rx queue address 
      (since the macro APPEND_RX_QUEUE_...
      works on the RxQueueHeadNew). */
      uint8_t LField;

      /* read first byte in buffer that is L-field in any case */
      LField=RxBuffQueue[RxQueueHead];
      
      
      /* L should be at least 10 (C-field+M-field+A-field+CI-field), 
      if not give an error */
      if(LField<10)
      {
         S2LPGpioIrqClearStatus();
         
        S2LPGpioIrqConfig(RX_FIFO_ALMOST_FULL,S_DISABLE);
        S2LPCmdStrobeSabort();
        
        ReInitRxCycle();

        /* The SM should not continue! */
        return;
      }

#ifdef FIRSTCRC_CHECK
      /* add 1st CRC check - only format A */
      uint8_t crc[2];
      uint8_t crc_index = 0;
      
      /* only format A */
      if (frame_format == FRAME_FORMAT_A)
      {
      
        /* compute CRC of 10 first bytes - of 1st block */
        for (crc_index = 0; crc_index <(AF_INITIAL_THR-2); crc_index++)
        {
          
          /* take care of RxBuffQueue[] cyclic buffer (RX_QUEUE_SIZE = 512 entries) */
          if (RxQueueHead+crc_index < RX_QUEUE_SIZE)
          {
          crcAppend(RxBuffQueue[RxQueueHead+crc_index]);
          }
          else
          {
          crcAppend(RxBuffQueue[(RxQueueHead+crc_index - RX_QUEUE_SIZE)]);  
          }
        
        }
        
        /* copy Received CRC from Rx buffer */
        if (RxQueueHead+10 < RX_QUEUE_SIZE)
        {
        crc[0]=RxBuffQueue[RxQueueHead+10];
        }
        else
        {
        crc[0]=RxBuffQueue[(RxQueueHead+10-RX_QUEUE_SIZE)];  
        }
        
        if (RxQueueHead+11 < RX_QUEUE_SIZE)
        {
        crc[1]=RxBuffQueue[RxQueueHead+11];
        }
        else
        {
        crc[1]=RxBuffQueue[(RxQueueHead+11-RX_QUEUE_SIZE)];  
        }
        
        /* compare CRCs and if not correct discard packet and reset Rx */
        if (!crcVerify(crc))
        {
        
          S2LPGpioIrqClearStatus();
         
          S2LPGpioIrqConfig(RX_FIFO_ALMOST_FULL,S_DISABLE);
          S2LPCmdStrobeSabort();
        
          ReInitRxCycle();

          /* The SM should not continue! */
          return;

        }

      }
#endif
      
      /* Use the L-field to compute the packet length
      according to the frame format. */
      if(frame_format == FRAME_FORMAT_A)
      { 
        /* In this case we can compute the packet length 
        using the following formula:
        1+L_field+2+2*sup_int((L_field-9)/16) */
        if(LField>9)
        {
          Length = 1+LField+2+2*(CEILING((((float)LField)-9.0)/16.0));
        }
        else
        {
          /* this should never happen :) */
          Length = LField+2+1;
        }
      }
      else if(frame_format == FRAME_FORMAT_B)
      {
        /* in this case L-field encounters all the CRC 
        (that are max 2*16bits in B format)
        So to obtain the length of the entire frame just add 1 
        (L-field size) to L-field value */
        Length = LField+1;
      }
        
      /* 3o6 */
      ResidualLength=Length-nRxIndex;
      
      if(ResidualLength>DEVICE_FIFO_SIZE-2)
      {      
        cThreholdRxFifoAF = 64;  
        rx_state = SM_RX_INTERM;
      }
      else
      {
        cThreholdRxFifoAF = ResidualLength;
        rx_state = SM_RX_LAST;
      }
      S2LPFifoSetAlmostFullThresholdRx(DEVICE_FIFO_SIZE-cThreholdRxFifoAF);
      
    }
    break;
    
  case SM_RX_LAST:
    {      
      /* stop the reception */
      S2LPCmdStrobeSabort();
      
      /* save the timestamp and rssi of the new packet */
      RxFrameDescQueue[RxFrameDescHead].rx_timestamp = WMBus_TimGetTimestamp();
      RxFrameDescQueue[RxFrameDescHead].rssi = (int32_t)S2LPRadioGetRssidBm();
      
      /* unarm the almost full threshold */
      S2LPGpioIrqConfig(RX_FIFO_ALMOST_FULL,S_DISABLE);
      
      /* Read the RX FIFO */
      APPEND_RX_QUEUE_FROM_FIFO(ResidualLength);
      
      /* update the number of received bytes */
      ResidualLength=Length-nRxIndex-ResidualLength;
      
      S2LPCmdStrobeFlushRxFifo();
      
      S2LPFifoSetAlmostFullThresholdRx(DEVICE_FIFO_SIZE-AF_INITIAL_THR);
      
      /* make the new packet available in the queue */
      RxFrameDescQueue[RxFrameDescHead].start_index=RxQueueHead;
      RxQueueHead=RxQueueHeadNew;
      RxFrameDescHead=(RxFrameDescHead+1)%RX_FRAME_DESC_Q_SIZE;
      nRxPackets++;
      
      /* return in the idle state of the reception machine */
      rx_state=SM_RX_FIRST;
      
      if((WMBusPhyAttr.DevType!=METER) || (RxPER_flag == S_TRUE))
      {
        WMBus_PhySetState(PHY_STATE_RX_RESTART); 
      }
    }
    break;
    
  case SM_RX_INTERM:
    {
      /* we read always the same quantity of bytes */
      APPEND_RX_QUEUE_FROM_FIFO(64-1);
      
      nRxIndex+=64-1; 
      ResidualLength=Length-nRxIndex;
      
      if(ResidualLength<DEVICE_FIFO_SIZE-6)
      {
        S2LPFifoSetAlmostFullThresholdRx(DEVICE_FIFO_SIZE-ResidualLength);

        rx_state = SM_RX_LAST;
      }
    }
    break;
  }
}


/**
* @brief  This function handles Transmit and Receive
*         interrupt request from the Transceiver.
* @param  None
* @retval None
*/
void WMBus_PhyInterruptHandler(void)
{
  uint8_t cNElemTxFifo;
  
  S2LPIrqs  irqStatus;
  
  /* Get the IRQ status */
  S2LPGpioIrqGetStatus(&irqStatus); 
  
  if(irqStatus.IRQ_TX_DATA_SENT || (WMBusPhyAttr.CsmaEnabled && irqStatus.IRQ_MAX_BO_CCA_REACH))
  {
    if(WMBusPhyAttr.CsmaEnabled)
    {
      S2LPCsma(S_DISABLE);
      if(irqStatus.IRQ_MAX_BO_CCA_REACH)
      {
        S2LPCmdStrobeSabort();
      }
       S2LPRadioCsBlanking(S_ENABLE);

      S2LPRadioSetRssiThreshdBm(WMBusPhyAttr.RxRSSIThreshold);
      
      if(irqStatus.IRQ_MAX_BO_CCA_REACH)
      {
        if(WMBusPhyAttr.CsmaOverrideFail)
        {
          S2LPCmdStrobeCommand(CMD_TX);
        }
        else
        {
          PhyTxDataSentStateHandler(1);
        }
      }
      else
      {
        PhyTxDataSentStateHandler(0);
      }
    }
    else
    {
      PhyTxDataSentStateHandler(0);
    }
  }
  
  /* Check the S2LP TX_DATA_SENT IRQ flag */
  /* Check the S2LP TX_FIFO_ALMOST_EMPTY IRQ flag */
  else if(irqStatus.IRQ_TX_FIFO_ALMOST_EMPTY)  
  {
    /* these two here are important! We need to disable the IRQ and 
    disarm the AE threshold to be sure that an IRQ 
    close to the one that has been just raised is not triggered again... */
    S2LPGpioIrqConfig(TX_FIFO_ALMOST_EMPTY , S_DISABLE);
//    S2LPFifoSetAlmostEmptyThresholdTx(DEVICE_FIFO_SIZE);        
    
    
    /* read the number of elements in the Tx FIFO */
    cNElemTxFifo = S2LPFifoReadNumberBytesTxFifo();

    
    /* check if the sum of the residual payload to be transmitted
    and the actual bytes in FIFO are higher than DEVICE_FIFO_SIZE */
    if(PhyTxFrame.lenRemain+cNElemTxFifo>DEVICE_FIFO_SIZE)
    {
      S2LPSpiWriteFifo(DEVICE_FIFO_SIZE-cNElemTxFifo-1,                       
                               &PhyTxFrame.DataBuff[PhyTxFrame.length]);
      
      S2LPFifoSetAlmostEmptyThresholdTx(ALMOST_EMPTY_THRESHOLD);
      
      PhyTxFrame.lenRemain -= (DEVICE_FIFO_SIZE-cNElemTxFifo-1);
      PhyTxFrame.length += (DEVICE_FIFO_SIZE-cNElemTxFifo-1);
      S2LPGpioIrqConfig(TX_FIFO_ALMOST_EMPTY , S_ENABLE);
    }
    else
    {
      S2LPSpiWriteFifo(PhyTxFrame.lenRemain,
                               &PhyTxFrame.DataBuff[PhyTxFrame.length]);
        
      PhyTxFrame.lenRemain = 0;
    }
    
    /* do another SPI transaction to put all bytes into FIFO */
    S2LPRefreshStatus(); 
  }
  else if(irqStatus.IRQ_RX_FIFO_ALMOST_FULL)
  {
    packetReceptionSM();
  }
  else if(irqStatus.IRQ_RX_TIMEOUT)
  {
    S2LPGpioIrqClearStatus();
    WMBus_LinkRxTimeoutHandler();
    WMBus_PhyResetPreambSyncFlag(); 
  }
#ifdef FASTRX_TIMER_AND_RXTIMEOUT_SYNC
  else if(irqStatus.IRQ_RX_SNIFF_TIMEOUT)
  {
    S2LPGpioIrqClearStatus();
    WMBus_LinkRxTimeoutHandler();
    WMBus_PhyResetPreambSyncFlag(); 
  }
#endif
  else if(irqStatus.IRQ_VALID_PREAMBLE) 
  {
    WMBus_ValidPreambleRcvd();
    S2LPGpioIrqClearStatus();
  }
  else if(irqStatus.IRQ_VALID_SYNC) 
  {
    WMBus_ValidSyncRcvd();
    S2LPGpioIrqClearStatus(); 
  }
  
  if(irqStatus.IRQ_RX_FIFO_ERROR)
  {
    S2LPGpioIrqClearStatus(); 
    
    S2LPGpioIrqConfig(RX_FIFO_ALMOST_FULL,S_DISABLE);
    S2LPCmdStrobeSabort();
    
    ReInitRxCycle();
    WMBus_PhyResetPreambSyncFlag();
  }

}




/**
* @brief  This function indicates the Valid Preamble is received.
* @param  None.
* @retval None.
*/
void WMBus_ValidPreambleRcvd(void)
{  
  ValidPreambleFlag = 1;
}

/**
* @brief  This function indicates the Valid Sync is received.
* @param  None.
* @retval None.
*/
void WMBus_ValidSyncRcvd(void)                                          
{ 
  if(ValidPreambleFlag)
  {
    ValidSyncFlag = 1;
  }
  else
  {
    ValidSyncFlag = 0;
  }
}

/**
* @brief  This function indicates the Valid Sync is status.
* @param  1:if valid Sync is detected, 0 : No valid Sync is deteced.
* @retval None.
*/
uint8_t WMBus_PhyCheckSyncStatus(void)                                    
{
  return (ValidSyncFlag);
}


/**
* @brief  This function Resets the Flag to zero.
* @param  None.
* @retval None.
*/
void WMBus_PhyResetPreambSyncFlag(void)                                     
{
  ValidSyncFlag = 0;
  ValidPreambleFlag = 0;
}


/**
* @brief  This function will re-init the reception cycle.
* @param  None.
* @retval None.
*/
void ReInitRxCycle(void)
{
  rx_state=SM_RX_FIRST;
  
  S2LPCmdStrobeFlushRxFifo();
  
  S2LPFifoSetAlmostFullThresholdRx(DEVICE_FIFO_SIZE-AF_INITIAL_THR);
 
  S2LPGpioIrqConfig(RX_FIFO_ALMOST_FULL,S_ENABLE);  
  S2LPCmdStrobeRx();
  WMBus_PhyResetPreambSyncFlag();  
}

/**
* @brief  This function initialize all Rx performance settings (AFC, ClockRec, RSSI threshold)
* @param  None.
* @retval None.
*/
void WMBUS_Init_RxPerformanceSettings(void)
{
  uint8_t tmp_reg[3];
  /* initialization to be done for Meter application */
   if(WMBusPhyAttr.DevType==METER)
   {
     /* initialize ClockRec settings */
    tmp_reg[0] = 0x28; 
    tmp_reg[1] = 0x48;

    S2LPSpiWriteRegisters(CLOCKREC1_ADDR,2,tmp_reg); 
    
    
    /* set RSSI threshold to -106dBm / tested with X-Nucleo-S2868A1 */
     
     if ((WMBusPhyAttr.DevMode == T2_MODE) || (WMBusPhyAttr.DevMode == S2_MODE))
     {
       
       /* initialize AFC settings : use AFC mode 1 to cope with bigger frequency offsets */
       tmp_reg[0] = 0xE8; 
       tmp_reg[1] = 0x18;
       tmp_reg[2] = 0x25;
    
       S2LPSpiWriteRegisters(AFC2_ADDR,3,tmp_reg); 
       
     }
     else if (WMBusPhyAttr.DevMode == C2_MODE)
     {
       
       /* initialize AFC settings : use AFC mode 1 to cope with bigger frequency offsets */
       /* change AFC1 & AFC0 register due to narrow frequency deviation in C2 mode */
       tmp_reg[0] = 0xE8; 
       tmp_reg[1] = 0x10;
       tmp_reg[2] = 0xA5;
    
       S2LPSpiWriteRegisters(AFC2_ADDR,3,tmp_reg); 
       
     }
     
   }
     
     
  
}

/**
* @brief  This function will configure the Physical level.
* @param  trx_mode:Transmit Mode(TX_MODE) or Receive Mode(RX_MODE).
* @retval PHY_STATUS: It will return PHY_STATUS_SUCCESS
*                     after successful Configuration.
*/
PHY_STATUS WMBus_PhyConfig(uint8_t trx_mode)
{
  if(!IS_WMBUS_DEVICE_TYPE(WMBusPhyAttr.DevType))
    return PHY_STATUS_ERROR_INVALID_DEVICE_TYPE;
  
  if(WMBusPhyAttr.DevType==METER)	/* we are meter */
  {
    if(trx_mode==TX_MODE)
      WMBusPhyAttr.Direction=DIR_M2O;
    else
      WMBusPhyAttr.Direction=DIR_O2M;
  }
  else	/* we are other */
  {
    if(trx_mode==TX_MODE)
      WMBusPhyAttr.Direction=DIR_O2M;
    else
      WMBusPhyAttr.Direction=DIR_M2O;
  }
  
  LoadConfRegs(WMBusPhyAttr.Direction);

  return PHY_STATUS_SUCCESS;  
}


/**
* @brief  This function will compare Two Radio Init Parameter.
* @param  *r1:Pointer to 1st RadioInit structure.
* @param  *r2:Pointer to 2nd RadioInit structure.
* @retval Returns 1 if same ,otherwise it will return 0.
*/
static int RadioCmp(SRadioInit *r1, SRadioInit *r2)
{
  if(/*r1->cChannelNumber!=r2->cChannelNumber ||\*/
  r1->lBandwidth!=r2->lBandwidth ||\
  r1->lDatarate!=r2->lDatarate ||\
  r1->lFreqDev!=r2->lFreqDev ||\
  r1->lFrequencyBase!=r2->lFrequencyBase ||\
  /*r1->!=r2->nChannelSpace ||\*/
  /*r1->nXtalOffsetPpm!=r2->nXtalOffsetPpm ||\*/
  r1->xModulationSelect!=r2->xModulationSelect) return 1;
  
  return 0;
}

/**
* @brief  This function will compare Two WMBus Packet Init structure.
* @param  *w1:Pointer to 1st WMBus packet Init structure.
* @param  *w2:Pointer to 2nd WMBus packet Init structure.
* @retval Returns 1 if same ,otherwise it will return 0.
*/
static int WMbusCmp(PktWMbusInit *w1,PktWMbusInit *w2)
{
  if(w1->cPostambleLength != w2->cPostambleLength || \
    w1->cPreambleLength != w2->cPreambleLength) return 1;
  
  return 0;
}


/**
* @brief  This function will Set the Physical parameters to Default.
* @param  None.
* @retval None.
*/
static void WMBus_PhySetDefaultPars(void)
{
  
  WMBusPhyAttr.TxInterval = 3600;

  if(WMBusPhyAttr.DevType==OTHER)
  {
    WMBusPhyAttr.Direction = DIR_M2O;
  }
  else
  {
    if((WMBusPhyAttr.DevMode == T2_MODE) ||(WMBusPhyAttr.DevMode == C2_MODE))

    {
      WMBusPhyAttr.Direction = DIR_O2M;
    }
    else
    {
      WMBusPhyAttr.Direction = DIR_M2O;
    }
    
  }
  switch(WMBusPhyAttr.DevMode)
  {
  case S1_MODE:		
    WMBusPhyAttr.TxInterval=10;
    WMBusPhyAttr.ResponseTimeMin = S_RESPONSE_TIME_MIN_MS;
    WMBusPhyAttr.ResponseTimeMax = S_RESPONSE_TIME_MAX_MS;
    WMBusPhyAttr.ResponseTimeTyp = S_RESPONSE_TIME_TYP_MS;
    WMBusPhyAttr.NominalTime = S_NOMINAL_TRANSMISSION_INTERVAL;
    WMBusPhyAttr.PreambleLength = DEFAULT_S1_PREAMBLE_LEN;
    WMBusPhyAttr.PostambleLength = DEFAULT_S1_POSTAMBLE_LEN;
    break;
    
  case S2_MODE:
    WMBusPhyAttr.TxInterval=10;                                           
    WMBusPhyAttr.ResponseTimeMin = S_RESPONSE_TIME_MIN_MS;
    WMBusPhyAttr.ResponseTimeMax = S_RESPONSE_TIME_MAX_MS;
    WMBusPhyAttr.ResponseTimeTyp = S_RESPONSE_TIME_TYP_MS;
    WMBusPhyAttr.FACDelay = S2_FAC_TRANSMISSION_DELAY;
    WMBusPhyAttr.FACTimeout = S2_FAC_TIME_OUT;   
    WMBusPhyAttr.NominalTime = S_NOMINAL_TRANSMISSION_INTERVAL;
    WMBusPhyAttr.PreambleLength = DEFAULT_S2_PREAMBLE_LEN;
    WMBusPhyAttr.PostambleLength = DEFAULT_S2_POSTAMBLE_LEN;
    break;
    
  case N1_MODE:
  case N2_MODE:
    WMBusPhyAttr.channel = CHANNEL_1a;
    WMBusPhyAttr.ResponseTimeMin = N_FAST_RESPONSE_TIME_MIN_MS;
    WMBusPhyAttr.ResponseTimeMax = N_FAST_RESPONSE_TIME_MAX_MS;
    WMBusPhyAttr.ResponseTimeTyp = N_FAST_RESPONSE_TIME_TYP_MS;
    WMBusPhyAttr.FACDelay = N2_FAC_TRANSMISSION_DELAY;
    WMBusPhyAttr.FACTimeout = N2_FAC_TIME_OUT;
    WMBusPhyAttr.NominalTime = R_N_F_H_NOMINAL_TRANSMISSION_INTERVAL;    
    WMBusPhyAttr.PreambleLength = DEFAULT_N_MODE_PREAMBLE_LENGTH;
    WMBusPhyAttr.PostambleLength = DEFAULT_N_MODE_POSTAMBLE_LENGTH;
    break;
    
  case R2_MODE:
    if(WMBusPhyAttr.DevType==METER)
    {
      WMBusPhyAttr.ResponseTimeMin = R_METER_RESPONSE_TIME_MIN_MS;
      WMBusPhyAttr.ResponseTimeMax = R_METER_RESPONSE_TIME_MAX_MS;
      WMBusPhyAttr.ResponseTimeTyp = R_METER_RESPONSE_TIME_TYP_MS;
    }
    else
    {
      WMBusPhyAttr.ResponseTimeMin = R_OTHER_RESPONSE_TIME_MIN_MS;
      WMBusPhyAttr.ResponseTimeMax = R_OTHER_RESPONSE_TIME_MAX_MS; 
      WMBusPhyAttr.ResponseTimeTyp = R_OTHER_RESPONSE_TIME_TYP_MS;
    }
    WMBusPhyAttr.FACDelay = R2_FAC_TRANSMISSION_DELAY;
    WMBusPhyAttr.FACTimeout = R2_FAC_TIME_OUT;
    WMBusPhyAttr.NominalTime = R_N_F_H_NOMINAL_TRANSMISSION_INTERVAL;
    WMBusPhyAttr.PreambleLength = DEFAULT_R2_PREAMBLE_LEN;
    WMBusPhyAttr.PostambleLength = DEFAULT_R2_POSTAMBLE_LEN;
    break;
    
  case C1_MODE:
  case C2_MODE:
    WMBusPhyAttr.ResponseTimeMin = C_SLOW_RESPONSE_TIME_MIN_MS;
    WMBusPhyAttr.ResponseTimeMax = C_SLOW_RESPONSE_TIME_MAX_MS; 
    WMBusPhyAttr.ResponseTimeTyp = C_SLOW_RESPONSE_TIME_TYP_MS;
    WMBusPhyAttr.FACDelay = C2_FAC_TRANSMISSION_DELAY;
    WMBusPhyAttr.FACTimeout = C2_FAC_TIME_OUT;
    WMBusPhyAttr.NominalTime = T_C_NOMINAL_TRANSMISSION_INTERVAL;
    WMBusPhyAttr.PreambleLength = DEFAULT_C_MODE_PREAMBLE_LENGTH;
    WMBusPhyAttr.PostambleLength = 0;
    break;
    
  case T1_MODE:
  case T2_MODE:
    WMBusPhyAttr.ResponseTimeMin = T_RESPONSE_TIME_MIN_MS;
    WMBusPhyAttr.ResponseTimeMax = T_RESPONSE_TIME_MAX_MS; 
    WMBusPhyAttr.ResponseTimeTyp = T_RESPONSE_TIME_TYP_MS;
    WMBusPhyAttr.FACDelay = T2_FAC_TRANSMISSION_DELAY;
    WMBusPhyAttr.FACTimeout = T2_FAC_TIME_OUT; 
    WMBusPhyAttr.NominalTime = T_C_NOMINAL_TRANSMISSION_INTERVAL;
    WMBusPhyAttr.PreambleLength = DEFAULT_T_PREAMBLE_LEN;
    WMBusPhyAttr.PostambleLength = DEFAULT_T_POSTAMBLE_LEN;
    break;
    
  case F2_MODE:
  case F2m_MODE:
    if(WMBusPhyAttr.DevType==METER)
    {
      WMBusPhyAttr.ResponseTimeMin = F_METER_RESPONSE_TIME_MIN_MS;
      WMBusPhyAttr.ResponseTimeMax = F_METER_RESPONSE_TIME_MAX_MS; 
      WMBusPhyAttr.ResponseTimeTyp = F_METER_RESPONSE_TIME_MIN_TYP;
    }
    else
    {
      WMBusPhyAttr.ResponseTimeMin = F_OTHER_SLOW_RESPONSE_TIME_MIN_MS;
      WMBusPhyAttr.ResponseTimeMax = F_OTHER_SLOW_RESPONSE_TIME_MAX_MS; 
      WMBusPhyAttr.ResponseTimeTyp = F_OTHER_SLOW_RESPONSE_TIME_TYP_MS;
    }
    WMBusPhyAttr.FACDelay = F2_FAC_TRANSMISSION_DELAY;
    WMBusPhyAttr.FACTimeout = F2_FAC_TIME_OUT;
    WMBusPhyAttr.NominalTime = R_N_F_H_NOMINAL_TRANSMISSION_INTERVAL;
    WMBusPhyAttr.PreambleLength = DEFAULT_F2_MODE_PREAMBLE_LEN;
    WMBusPhyAttr.PostambleLength = DEFAULT_F2_MODE_POSTAMBLE_LEN;
    break;

  case S1m_MODE : case ALL_MODE: case INVALID_MODE:
	  break;

  } 
}

/**
* @brief  This function will configure the S2LP Radio registers.This will \
*         abort the current TX and Rx transactions.
* @param  None.
* @retval PHY_STATUS.
*/
static uint8_t WMBus_PhyConfigFull(void)
{
  if(!IS_WMBUS_DEVICE_TYPE(WMBusPhyAttr.DevType))
    return 0;
  
  uint32_t safe_multiplier=1;
  uint32_t timerValueUs;
  uint32_t timerValueMs;
  uint8_t changed=0;
  uint8_t tmp;
  
  SRadioInit radioInit;
  PktWMbusInit MbusInitVal;
  
  S2LPRadioSetChannelSpace(20000);
  S2LPRadioSetChannel(0);
  
  switch(WMBusPhyAttr.DevMode)
  {
  case S1_MODE:
    {
      radioInit.xModulationSelect = S_T_R_MODE_MODULATION_SCHEME;    
      radioInit.lFrequencyBase = SUBMODE0_BASE_FREQUENCY+(SMODE_TMODE_CHANNEL_NUMBER*SMODE_TMODE_CHANNEL_SPACE);
      radioInit.lDatarate = SUBMODE0_DATARATE;
      radioInit.lFreqDev = SUBMODE0_FREQ_DEV;
      radioInit.lBandwidth = SUBMODE0_BANDWIDTH;    
      MbusInitVal.xWMbusSubmode = (WMbusSubmode)WMBUS_SUBMODE_S1_S2_LONG_HEADER;
      MbusInitVal.cPreambleLength = (uint8_t)((WMBusPhyAttr.PreambleLength >= \
        S1_PREAMB_CHIPCOMB_LEN_MIN)?(WMBusPhyAttr.PreambleLength - 279):\
          (DEFAULT_S1_PREAMBLE_LEN-279));
      MbusInitVal.cPostambleLength = \
        (uint8_t)(((WMBusPhyAttr.PostambleLength >= 2)&&\
          (WMBusPhyAttr.PostambleLength <= 8))?WMBusPhyAttr.PostambleLength:\
            DEFAULT_S1_POSTAMBLE_LEN); 
      /* S2LP Packet config */
      break;  
    }          
  case S1m_MODE:
    {
      radioInit.xModulationSelect = S_T_R_MODE_MODULATION_SCHEME;    
      radioInit.lFrequencyBase = SUBMODE1_BASE_FREQUENCY+(SMODE_TMODE_CHANNEL_NUMBER*SMODE_TMODE_CHANNEL_SPACE);
      radioInit.lDatarate = SUBMODE1_DATARATE;
      radioInit.lFreqDev = SUBMODE1_FREQ_DEV;
      radioInit.lBandwidth = SUBMODE1_BANDWIDTH;
      MbusInitVal.xWMbusSubmode = (WMbusSubmode)WMBUS_SUBMODE_S1_M_S2_T2_OTHER_TO_METER;
      MbusInitVal.cPreambleLength = \
        (uint8_t)((WMBusPhyAttr.PreambleLength >= \
          S1m_PREAMB_CHIPCOMB_LEN_MIN)?(WMBusPhyAttr.PreambleLength - 15):\
            (DEFAULT_S1m_PREAMBLE_LEN-15));
      MbusInitVal.cPostambleLength = \
        (uint8_t)(((WMBusPhyAttr.PostambleLength >= 2)&&\
          (WMBusPhyAttr.PostambleLength <= 8))?WMBusPhyAttr.PostambleLength:\
            DEFAULT_S1m_POSTAMBLE_LEN);
      /* S2LP Packet config */
      break; 
    }   
  case S2_MODE:
    {
      radioInit.xModulationSelect = S_T_R_MODE_MODULATION_SCHEME;    
      if((WMBusPhyAttr.PreambleLength) >= 279)        /* S2 long header case */
      {
        radioInit.lFrequencyBase = SUBMODE0_BASE_FREQUENCY+(SMODE_TMODE_CHANNEL_NUMBER*SMODE_TMODE_CHANNEL_SPACE);
        radioInit.lDatarate = SUBMODE0_DATARATE;
        radioInit.lFreqDev = SUBMODE0_FREQ_DEV;
        radioInit.lBandwidth = SUBMODE0_BANDWIDTH;    
        MbusInitVal.xWMbusSubmode = (WMbusSubmode)WMBUS_SUBMODE_S1_S2_LONG_HEADER;
        MbusInitVal.cPreambleLength = \
          (uint8_t)((WMBusPhyAttr.PreambleLength<279)?\
            (DEFAULT_S2_PREAMBLE_LEN_L - 279):\
              (WMBusPhyAttr.PreambleLength - 279));      
        MbusInitVal.cPostambleLength = \
          (uint8_t)((0x00 == WMBusPhyAttr.PostambleLength)||\
            (WMBusPhyAttr.PostambleLength > POSTABMLE_MAX_LEN))?\
      DEFAULT_S2_POSTAMBLE_LEN : WMBusPhyAttr.PostambleLength;
      }
      else 
      {
        radioInit.lFrequencyBase = SUBMODE1_BASE_FREQUENCY+(SMODE_TMODE_CHANNEL_NUMBER*SMODE_TMODE_CHANNEL_SPACE);
        radioInit.lDatarate = SUBMODE1_DATARATE;
        radioInit.lFreqDev = SUBMODE1_FREQ_DEV;
        radioInit.lBandwidth = SUBMODE1_BANDWIDTH;
        MbusInitVal.cPreambleLength =\
          (uint8_t)(WMBusPhyAttr.PreambleLength - 15);
        MbusInitVal.xWMbusSubmode= \
          (WMbusSubmode)WMBUS_SUBMODE_S1_M_S2_T2_OTHER_TO_METER;
        MbusInitVal.cPreambleLength = \
          (uint8_t)((WMBusPhyAttr.PreambleLength >=\
            S2_PREAMB_CHIPCOMB_LEN_MIN)?(WMBusPhyAttr.PreambleLength - 15):\
              (DEFAULT_S2_PREAMBLE_LEN-15));
        MbusInitVal.cPostambleLength = \
          (uint8_t)(((WMBusPhyAttr.PostambleLength >= 2)&&\
            (WMBusPhyAttr.PostambleLength <= 8))?WMBusPhyAttr.PostambleLength:\
              DEFAULT_S2_POSTAMBLE_LEN);
      }
      break;
    }
  case T1_MODE:
    {
      radioInit.xModulationSelect = S_T_R_MODE_MODULATION_SCHEME;    
      radioInit.lFrequencyBase = SUBMODE3_BASE_FREQUENCY+(SMODE_TMODE_CHANNEL_NUMBER*SMODE_TMODE_CHANNEL_SPACE);
      radioInit.lDatarate = SUBMODE3_DATARATE;
      radioInit.lFreqDev = SUBMODE3_FREQ_DEV;
      radioInit.lBandwidth = SUBMODE3_BANDWIDTH;
      MbusInitVal.xWMbusSubmode = \
        (WMbusSubmode)WMBUS_SUBMODE_T1_T2_METER_TO_OTHER;
      MbusInitVal.cPreambleLength = (uint8_t)((WMBusPhyAttr.PreambleLength >=\
        T_PREAMB_CHIPCOMB_LEN_MIN)?(WMBusPhyAttr.PreambleLength - 19):\
          (DEFAULT_T_PREAMBLE_LEN-19));
      MbusInitVal.cPostambleLength = \
        (uint8_t)(((WMBusPhyAttr.PostambleLength >= 2)&&\
          (WMBusPhyAttr.PostambleLength <= 8))?WMBusPhyAttr.PostambleLength:\
            DEFAULT_T_POSTAMBLE_LEN);
      break;
    }
  case T2_MODE:
    {
      radioInit.xModulationSelect = S_T_R_MODE_MODULATION_SCHEME;    
      if(WMBusPhyAttr.Direction)/* other to meter  */ 
      {
        radioInit.lFrequencyBase = SUBMODE1_BASE_FREQUENCY+(SMODE_TMODE_CHANNEL_NUMBER*SMODE_TMODE_CHANNEL_SPACE);
        radioInit.lDatarate = SUBMODE1_DATARATE;
        radioInit.lFreqDev = SUBMODE1_FREQ_DEV;
        radioInit.lBandwidth = SUBMODE1_BANDWIDTH;
        MbusInitVal.xWMbusSubmode =(WMbusSubmode)WMBUS_SUBMODE_S1_M_S2_T2_OTHER_TO_METER;
        MbusInitVal.cPreambleLength = 15;
        MbusInitVal.cPostambleLength = 4;
      }
      else/* Meter to other  */ 
      {
        radioInit.lFrequencyBase = SUBMODE3_BASE_FREQUENCY+(SMODE_TMODE_CHANNEL_NUMBER*SMODE_TMODE_CHANNEL_SPACE);
        radioInit.lDatarate = SUBMODE3_DATARATE;
        radioInit.lFreqDev = SUBMODE3_FREQ_DEV;
        radioInit.lBandwidth = SUBMODE3_BANDWIDTH;
        MbusInitVal.xWMbusSubmode = \
          (WMbusSubmode)WMBUS_SUBMODE_T1_T2_METER_TO_OTHER;
        MbusInitVal.cPreambleLength = \
          (uint8_t)((WMBusPhyAttr.PreambleLength >=\
            T_PREAMB_CHIPCOMB_LEN_MIN)?(WMBusPhyAttr.PreambleLength - 19):\
              (DEFAULT_T_PREAMBLE_LEN-19));
        MbusInitVal.cPostambleLength = \
          (((WMBusPhyAttr.PostambleLength >= 2)&&\
            (WMBusPhyAttr.PostambleLength <= 8))?WMBusPhyAttr.PostambleLength:\
              DEFAULT_T_POSTAMBLE_LEN);
      }
      /* S2LP Packet config */
      break;    
    }
  case R2_MODE:
    {
      if(WMBusPhyAttr.Direction)/* other to meter  */ 
      {
        radioInit.lFrequencyBase = SUBMODE0_BASE_FREQUENCY+(SMODE_TMODE_CHANNEL_NUMBER*R2MODE_CHANNEL_SPACE);
      }
      else /* Meter to other  */ 
      {
        radioInit.lFrequencyBase = (uint32_t)(SUBMODE5_BASE_FREQUENCY+(WMBusPhyAttr.channel*R2MODE_CHANNEL_SPACE));
      }
      
      radioInit.xModulationSelect = S_T_R_MODE_MODULATION_SCHEME;    
      radioInit.lDatarate = SUBMODE5_DATARATE;
      radioInit.lFreqDev = SUBMODE5_FREQ_DEV;
      radioInit.lBandwidth = SUBMODE5_BANDWIDTH;
      MbusInitVal.xWMbusSubmode = \
        (WMbusSubmode)WMBUS_SUBMODE_S1_M_S2_T2_OTHER_TO_METER;
      MbusInitVal.cPreambleLength =\
        (uint8_t)((WMBusPhyAttr.PreambleLength >=\
          R2_PREAMB_CHIPCOMB_LEN_MIN)?(WMBusPhyAttr.PreambleLength - 39):\
            (DEFAULT_R2_PREAMBLE_LEN-39));
      MbusInitVal.cPostambleLength =  (((WMBusPhyAttr.PostambleLength >= 2)&&\
        (WMBusPhyAttr.PostambleLength <= 8))?WMBusPhyAttr.PostambleLength:\
          DEFAULT_R2_POSTAMBLE_LEN);
      /* S2LP Packet config */
      break;
    }
  case N1_MODE:
  case N2_MODE:
    switch( WMBusPhyAttr.channel)
    {
      /* we duplicate the cases with 0,1,2,3,4,5 because of an error in the GUI */
    case CHANNEL_1a:
      {
        radioInit.xModulationSelect = N_MODE_MODULATION_SCHEME; 
        radioInit.lFrequencyBase = N_MODE_BASE_FREQUENCY+(CHANNEL_1a*N_MODE_CHANNEL_SPACE);
        radioInit.lFreqDev = N_AB_EF_MODE_FREQ_DEV;
        radioInit.lBandwidth = N_AB_EF_MODE_BANDWIDTH;        
        radioInit.lDatarate = N_AB_EF_MODE_DATARATE;
        break;
      }
    case CHANNEL_1b:
      {
        radioInit.xModulationSelect = N_MODE_MODULATION_SCHEME; 
        radioInit.lFrequencyBase = N_MODE_BASE_FREQUENCY+(CHANNEL_1b*N_MODE_CHANNEL_SPACE);
        radioInit.lFreqDev = N_AB_EF_MODE_FREQ_DEV;
        radioInit.lBandwidth = N_AB_EF_MODE_BANDWIDTH;        
        radioInit.lDatarate = N_AB_EF_MODE_DATARATE;
        break;
      }
    case CHANNEL_2a:
      {
        radioInit.xModulationSelect = N_MODE_MODULATION_SCHEME; 
        radioInit.lFrequencyBase = N_MODE_BASE_FREQUENCY+(CHANNEL_2a*N_MODE_CHANNEL_SPACE);
        radioInit.lFreqDev = N_CD_MODE_FREQ_DEV;
        radioInit.lBandwidth = N_CD_MODE_BANDWIDTH;        
        radioInit.lDatarate = N_CD_MODE_DATARATE;
        break;
      }
    case CHANNEL_2b:
      {
        radioInit.xModulationSelect = N_MODE_MODULATION_SCHEME; 
        radioInit.lFrequencyBase = N_MODE_BASE_FREQUENCY+(CHANNEL_2b*N_MODE_CHANNEL_SPACE);
        radioInit.lFreqDev = N_CD_MODE_FREQ_DEV;
        radioInit.lBandwidth = N_CD_MODE_BANDWIDTH;        
        radioInit.lDatarate = N_CD_MODE_DATARATE;
        break;
      }
    case CHANNEL_3a:
      {
        radioInit.xModulationSelect = N_MODE_MODULATION_SCHEME; 
        radioInit.lFrequencyBase = N_MODE_BASE_FREQUENCY+(CHANNEL_3a*N_MODE_CHANNEL_SPACE);
        radioInit.lFreqDev = N_AB_EF_MODE_FREQ_DEV;
        radioInit.lBandwidth = N_AB_EF_MODE_BANDWIDTH;        
        radioInit.lDatarate = N_AB_EF_MODE_DATARATE;
        break;
      }
    case CHANNEL_3b:
      {
        radioInit.xModulationSelect = N_MODE_MODULATION_SCHEME; 
        radioInit.lFrequencyBase = N_MODE_BASE_FREQUENCY+(CHANNEL_3b*N_MODE_CHANNEL_SPACE);
        radioInit.lFreqDev = N_AB_EF_MODE_FREQ_DEV;
        radioInit.lBandwidth = N_AB_EF_MODE_BANDWIDTH;        
        radioInit.lDatarate = N_AB_EF_MODE_DATARATE;
        break;
      }
    }
    safe_multiplier=2;/* because in case of meter 2 other the overhead produced by the RX-TX breaks the RX timeout... */
    break;
  case C1_MODE:
    {
      radioInit.xModulationSelect = C1C2m2o_MODE_MODULATION_SCHEME;            
      radioInit.lFrequencyBase = C1C2m2o_MODE_BASE_FREQUENCY;
      radioInit.lFreqDev = C1C2m2o_MODE_FREQ_DEV;
      radioInit.lDatarate = C1C2m2o_MODE_DATARATE;
      radioInit.lBandwidth = C1C2m2o_MODE_BANDWIDTH; 
      
      break;
    }
  case C2_MODE:
    {
      if(WMBusPhyAttr.Direction)/* other to meter  */ 
      {
        radioInit.xModulationSelect = C2o2m_MODE_MODULATION_SCHEME; 
        radioInit.lFrequencyBase = C2o2m_MODE_BASE_FREQUENCY;
        radioInit.lFreqDev = C2o2m_MODE_FREQ_DEV;
        radioInit.lDatarate = C2o2m_MODE_DATARATE;
        radioInit.lBandwidth = C2o2m_MODE_BANDWIDTH; 
      }
      else /* meter to other  */ 
      {
        radioInit.xModulationSelect = C1C2m2o_MODE_MODULATION_SCHEME; 
        radioInit.lFrequencyBase = C1C2m2o_MODE_BASE_FREQUENCY;
        radioInit.lFreqDev = C1C2m2o_MODE_FREQ_DEV;
        radioInit.lDatarate = C1C2m2o_MODE_DATARATE;
        radioInit.lBandwidth = C1C2m2o_MODE_BANDWIDTH; 
      }

      break;
    }
  case F2_MODE:
  case F2m_MODE:
    radioInit.xModulationSelect = F_MODE_MODULATION_SCHEME; 
    radioInit.lFrequencyBase = F_MODE_BASE_FREQUENCY;
    radioInit.lFreqDev = F_MODE_FREQ_DEV;
    radioInit.lDatarate = F_MODE_DATARATE;
    radioInit.lBandwidth = F_MODE_BANDWIDTH; 
    break;

  case ALL_MODE: case INVALID_MODE:
	  break;
  }
  
  if(WMBusPhyAttr.CustDatarate)
  {
    radioInit.lDatarate = WMBusPhyAttr.CustDatarate;
  }
  radioInit.lFrequencyBase+=WMBusPhyAttr.RFOffset;
  
  
  if(RadioCmp(&radioInit_p,&radioInit)!=0)
  {
    /* S2LP Radio config */
    S2LPRadioInit(&radioInit);
    radioInit_p=radioInit;  
    changed++;
  } 
  
  if(WMBusPhyAttr.RFPower!=RFPower_p)
  {
    S2LPRadioSetMaxPALevel(S_DISABLE);
    S2LPRadioSetPALeveldBm(0,(int32_t )WMBusPhyAttr.RFPower);
    RFPower_p=WMBusPhyAttr.RFPower;
    S2LPRadioSetPALevelMaxIndex(0);
    
    g_xStatus = S2LPSpiReadRegisters(PM_CONF0_ADDR, 1, &tmp);
    tmp &=(~SET_SMPS_LVL_REGMASK);
    
    if(WMBusPhyAttr.RFPower<=10)/*Low Power Mode 1.2V*/
    {
      tmp |= 0x10;
      S2LPSpiWriteRegisters(PM_CONF0_ADDR, 1, &tmp);
    }
    else if(WMBusPhyAttr.RFPower<=14)/*High Performance Mode 1.5V*/
    {
      tmp |= 0x40;
      S2LPSpiWriteRegisters(PM_CONF0_ADDR, 1, &tmp);
    }
    else if(WMBusPhyAttr.RFPower<=16)/*Boost Mode  1.8V*/
    {
      tmp |= 0x70;
      S2LPSpiWriteRegisters(PM_CONF0_ADDR, 1, &tmp);
    }
  }
  
  if(RxRSSIThTable[WMBusPhyAttr.DevType][WMBusPhyAttr.DevMode]!=RSSIThr_p)
  {
    WMBusPhyAttr.RxRSSIThreshold = RxRSSIThTable[WMBusPhyAttr.DevMode][WMBusPhyAttr.DevType];
    S2LPRadioSetRssiThreshdBm(WMBusPhyAttr.RxRSSIThreshold);
    RSSIThr_p=WMBusPhyAttr.RxRSSIThreshold;
    changed++;
  }
    
  if(WMBusPhyAttr.DevMode==N1_MODE || WMBusPhyAttr.DevMode==N2_MODE)
  {
    PktSyncLength sync_length;
    uint32_t sync_word=0;
    
#ifdef DYNAMIC_PKT_FORMAT_N
    sync_length=PKT_SYNC_LENGTH_1BYTE;
    sync_word=(N_MODE_SYNC_WORD_PACKET_A&0xff000000);
#else
    sync_length=PKT_SYNC_LENGTH_2BYTES;
    sync_word=N_MODE_SYNC_WORD_PACKET_A;
    if(WMBus_LinkGetFormat()==FRAME_FORMAT_B)  
    {
      sync_word=N_MODE_SYNC_WORD_PACKET_B;
    }
#endif
    
    PktBasicInit xBasicInit=
    {
      DEFAULT_N_MODE_PREAMBLE_LENGTH,
      sync_length,
      sync_word,
      (SFunctionalState)PKT_LENGTH_FIX,
      (SFunctionalState)0,
      PKT_NO_CRC,
      S_DISABLE,
      S_DISABLE,
      S_DISABLE
    };
    
    /* S2LP Packet config */
     S2LPPktBasicInit(&xBasicInit);

    
    /* payload length config */
     S2LPPktBasicSetPayloadLength(0xFFF0);

    changed++;
  }
  
  else if((WMBusPhyAttr.DevMode==T1_MODE || WMBusPhyAttr.DevMode==T2_MODE) &&\
  WMBusPhyAttr.Direction==DIR_M2O && WMBusPhyAttr.DevType!=METER)            
  {
      

    PktBasicInit xBasicInit=
    {
      32, // this really does not matter because we are in RX
      24,
      0x55543D00,
      S_DISABLE,
      S_DISABLE,
      PKT_NO_CRC,
      S_DISABLE,
      S_DISABLE,
      S_DISABLE
    }; 
   
    /* S2LP Packet config */
    S2LPPktBasicInit(&xBasicInit);
    
    /* +patch   : activate 3-out-6 decoding for Rx */ 
    S2LPPacketHandler3OutOf6(S_ENABLE); 
    /* -patch   : activate 3-out-6 decoding for Rx */ 
    
    /* payload length config */
    S2LPPktBasicSetPayloadLength(0xFFF0);

    
    MbusInitVal_p=MbusInitVal;
    changed++;
  
  }  
  else if(WMBusPhyAttr.DevMode==C1_MODE || WMBusPhyAttr.DevMode==C2_MODE) 
  {
    uint32_t sync=0;
    uint8_t sync_len;
    

    if((WMBusPhyAttr.Direction==DIR_M2O && WMBusPhyAttr.DevType!=METER)||\
      (WMBusPhyAttr.Direction==DIR_O2M && WMBusPhyAttr.DevType==METER))
    {
      /* RX OTHER */
      /* +patch   : use same C-mode Sync pattern as Tx */
      //sync=0x55543D00;
      //sync_len=(PktSyncLength)24;
      sync=0x543D5400;
      if(WMBus_LinkGetFormat()==FRAME_FORMAT_A)
        sync+=0xCD;
      else if(WMBus_LinkGetFormat()==FRAME_FORMAT_B)
        sync+=0x3D;
      
      sync_len=(PktSyncLength)32;
      /* -patch   : use same C-mode Sync pattern as Tx */      
    }
    else if((WMBusPhyAttr.Direction==DIR_M2O && WMBusPhyAttr.DevType==METER) ||\
      (WMBusPhyAttr.Direction==DIR_O2M && WMBusPhyAttr.DevType!=METER))
    {
      /* TX ANY */
      sync=0x543D5400;
      if(WMBus_LinkGetFormat()==FRAME_FORMAT_A)
        sync+=0xCD;
      else if(WMBus_LinkGetFormat()==FRAME_FORMAT_B)
        sync+=0x3D;
      
      sync_len=(PktSyncLength)32;
    }
   
      
    if(sync)
    {
      PktBasicInit xBasicInit=
      {
        DEFAULT_C_MODE_PREAMBLE_LENGTH*4,
        sync_len,
        sync,
        S_DISABLE,
        S_DISABLE,
        PKT_NO_CRC,
        S_DISABLE,
        S_DISABLE,
        S_DISABLE
      }; 
      
      
      
      /* S2LP Packet config */
      S2LPPktBasicInit(&xBasicInit);
      
      
      /* payload length config */
      S2LPPktBasicSetPayloadLength(0xFFF0);
      
      
      MbusInitVal_p=MbusInitVal;
      changed++;
    }
  }
  
  else if (WMBusPhyAttr.DevMode==F2_MODE || WMBusPhyAttr.DevMode==F2m_MODE)
  {
    PktSyncLength sync_len;
    uint32_t sync = 0;
    
    if((WMBusPhyAttr.Direction==DIR_M2O && WMBusPhyAttr.DevType!=METER)||\
      (WMBusPhyAttr.Direction==DIR_O2M && WMBusPhyAttr.DevType==METER))
    {
      /* RX OTHER or METER */
      sync=0xF6000000;  
      sync_len=(PktSyncLength)8;

    }
    else if((WMBusPhyAttr.Direction==DIR_M2O && WMBusPhyAttr.DevType==METER) ||\
      (WMBusPhyAttr.Direction==DIR_O2M && WMBusPhyAttr.DevType!=METER))
    {
      /* TX ANY */
      sync=0xF6000000;
      if(WMBus_LinkGetFormat()==FRAME_FORMAT_A)
        sync+=0x8D0000;
      else if(WMBus_LinkGetFormat()==FRAME_FORMAT_B)
        sync+=0x720000;
      
      sync_len=(PktSyncLength)16;
    }
    

    if(sync)
    {
      PktBasicInit xBasicInit=
      {
        32,
        sync_len,
        sync,
        S_DISABLE,
        S_DISABLE,
        PKT_NO_CRC,
        S_DISABLE,
        S_DISABLE,
        S_DISABLE
      }; 
      /* S2LP Packet config */
      S2LPPktBasicInit(&xBasicInit);
    }
    
    /* payload length config */
     S2LPPktBasicSetPayloadLength(0xFFF0);

    MbusInitVal_p=MbusInitVal;
      
    changed++;
  }
  
  else if(WMbusCmp(&MbusInitVal,&MbusInitVal_p)!=0)
  {
    S2LPPktWMbusInit(&MbusInitVal);

    /* payload length config */
    S2LPPktWMbusSetPayloadLength(0xFFF0);

    MbusInitVal_p=MbusInitVal;
    changed++;
    
  }  
  
  if(WMBusPhyAttr.DevType==METER && RxTim_p!=(WMBusPhyAttr.ResponseTimeMax-WMBusPhyAttr.ResponseTimeMin+RESPONSE_TIME_GUARD))
  {
    timerValueMs = (uint32_t)(WMBusPhyAttr.ResponseTimeMax-WMBusPhyAttr.ResponseTimeMin+RESPONSE_TIME_GUARD)*safe_multiplier;
    timerValueUs = (uint32_t)(timerValueMs*1000);    
/* only valid for T2 and C2 mode */
#ifdef FASTRX_TIMER_AND_RXTIMEOUT_SYNC
    if (WMBusPhyAttr.DevMode==T2_MODE)
    {
      /* set timer value for Rx timeout (2nd timer) - starting after Fast Rx timer */
    //WMBus_S2LPTimerRxTimeoutUs((timerValueUs + T_RESPONSE_TIME_FOR_SYNC_PATTERN_US));  
    WMBus_S2LPTimerRxTimeoutUs(T_RESPONSE_TIME_FOR_SYNC_PATTERN_US);
    
    /* set FastRx timer to max value to reach 980µs with 216kHz RxBW and 50MHz Xtal */
    S2LpSetTimerFastRxTermTimer(255);
    }
    else if (WMBusPhyAttr.DevMode==C2_MODE)
    {
      /* set timer value for Rx timeout (2nd timer) - starting after Fast Rx timer */
    WMBus_S2LPTimerRxTimeoutUs(C_RESPONSE_TIME_FOR_SYNC_PATTERN_US);
    
    /* set FastRx timer to max value to reach 980µs with 216kHz RxBW and 50MHz Xtal */
    S2LpSetTimerFastRxTermTimer(255);
    
    }
    else if (WMBusPhyAttr.DevMode==S2_MODE)
    {
      /* default Rx timeout setting with 1 timer only */
      WMBus_S2LPTimerRxTimeoutUs(timerValueUs);
    }
    else
    {
      WMBus_S2LPTimerRxTimeoutUs(timerValueUs);
    }
    
#else
    /* only 1 Rx timeout */ 
    /* same management but no FastRx Timer for T2 and C2 mode */
    if (WMBusPhyAttr.DevMode==T2_MODE)
    {
      /* set timer value for Rx timeout (2nd timer) - starting after Fast Rx timer */ 
    WMBus_S2LPTimerRxTimeoutUs((T_RESPONSE_TIME_FOR_SYNC_PATTERN_US+RESPONSE_TIME_FOR_SYNC_PATTERN_US_MARGIN));
    
    }
    else if (WMBusPhyAttr.DevMode==C2_MODE)
    {
      /* set timer value for Rx timeout (2nd timer) - starting after Fast Rx timer */
    WMBus_S2LPTimerRxTimeoutUs((C_RESPONSE_TIME_FOR_SYNC_PATTERN_US+RESPONSE_TIME_FOR_SYNC_PATTERN_US_MARGIN));
 
    
    }
    else if (WMBusPhyAttr.DevMode==S2_MODE)
    {
      /* default Rx timeout setting with 1 timer only */
      WMBus_S2LPTimerRxTimeoutUs(timerValueUs);
    }
    else
    {
      WMBus_S2LPTimerRxTimeoutUs(timerValueUs);
    }


#endif
    
    RxTim_p=WMBusPhyAttr.ResponseTimeMax-WMBusPhyAttr.ResponseTimeMin+RESPONSE_TIME_GUARD;
  }
  return changed;
}

/**
* @brief  This function sets the Rx Timeout in case of METER.
* @param  None
* @retval None
*/
void WMBus_PhySetRxTimeout(void)
{
  uint32_t timerValueMs,timerValueUs;
  uint32_t safe_multiplier=1;
  if(WMBusPhyAttr.DevType==METER)
  {
    timerValueMs = (uint32_t)(WMBusPhyAttr.ResponseTimeMax-WMBusPhyAttr.ResponseTimeMin+RESPONSE_TIME_GUARD)*safe_multiplier;
    timerValueUs = (uint32_t)(timerValueMs*1000);
    if (WMBusPhyAttr.DevMode==T2_MODE || WMBusPhyAttr.DevMode==C2_MODE || WMBusPhyAttr.DevMode==F2_MODE|| WMBusPhyAttr.DevMode==F2m_MODE)/*Timeout value is very less*/
      timerValueUs *=10;
    //WMBus_S2LPTimerRxTimeoutUs(timerValueUs); 
        uint8_t timers_reg[2];
    timers_reg[0] = 20;
    timers_reg[1] = 1;
    S2LPSpiWriteRegisters(TIMERS5_ADDR, 2, timers_reg);
  }   
}


/**
* @brief  This function will return the current PHY state.
* @param  *phyState: The pointer to WMBusPhyState_t type primitive.
* @retval PHY_STATUS_SUCCESS or PHY_STATUS_ERROR_INVALID_STATE 
*              if the state is a transition one.
*/
PHY_STATUS  WMBus_PhyGetState (WMBusPhyState_t *phyState)
{  
  SBool xShutdown = S_FALSE;
    xShutdown = (SBool)S2LPCheckShutdown();
  
  if(xShutdown == S_TRUE)
  {
    *phyState = PHY_STATE_SHUTDOWN;
  }
  else
  {
    S2LPRefreshStatus();

    switch(g_xStatus.MC_STATE)
    {
    case MC_STATE_STANDBY:
      *phyState = PHY_STATE_STANDBY;
      break;
      
    case MC_STATE_READY:
      *phyState = PHY_STATE_READY;
      break;
      
    case MC_STATE_RX:
      *phyState = PHY_STATE_RX;
      break;
      
    case MC_STATE_TX:
      *phyState = PHY_STATE_TX; 
      break;
      
    default:
      *phyState = PHY_STATE_INVALID;
      return PHY_STATUS_ERROR_INVALID_STATE;
    }
  }
  return PHY_STATUS_SUCCESS;
}


void WMBus_PhyDeviceCmd(uint8_t en)
{
  ex_cmd=en;
}

/**
* @brief  This function will set the current PHY state.
* @param  WMBusPhyState_t phyState: The phy state.
* @retval PHY_STATUS: PHY_STATUS_SUCCESS
*/
PHY_STATUS  WMBus_PhySetState(WMBusPhyState_t phyState)
{  
  SBool xShutdown = S_FALSE;
  xShutdown = (SBool)S2LPCheckShutdown();
  
  if(xShutdown == S_FALSE)
  {
    S2LPRefreshStatus();
    
  }
  
  switch(phyState)
  {
  case PHY_STATE_SHUTDOWN:
    {
      if(xShutdown == S_FALSE)
      {
        S2LPEnterShutdown();
        
      }
      phy_init_called=0;
      
      break;
    }
  case PHY_STATE_READY:
    {
      if(xShutdown == S_TRUE)
      {
        S2LPExitShutdown();
        S2LPManagementRangeExtInit();
        
      }
      else if(g_xStatus.MC_STATE == MC_STATE_STANDBY)
      {
        S2LPCmdStrobeReady();
      }
      else if(g_xStatus.MC_STATE == MC_STATE_RX)
      {
        S2LPCmdStrobeSabort();
      }
      break;
    }
  case PHY_STATE_TX:
    {
      if(xShutdown == S_TRUE)
      {
        S2LPExitShutdown();
        S2LPManagementRangeExtInit();
      }
      else if(ex_cmd && g_xStatus.MC_STATE == MC_STATE_STANDBY)
      {
        S2LPCmdStrobeReady();
      }
      else if(ex_cmd && g_xStatus.MC_STATE == MC_STATE_RX)
      {
        S2LPCmdStrobeSabort();
      }
      
      tx_done_flag = S_FALSE;
      WMBus_PhyConfig(TX_MODE);
      
      if(WMBusPhyAttr.CsmaEnabled)
      {
        /* Enable CSMA */
        S2LPRadioCsBlanking(S_DISABLE);
        SCsmaInit /*CsmaInit*/csma_init = {
          .xCsmaPersistentMode=S_DISABLE,
          .xMultiplierTbit=(SCsmaPeriod)WMBusPhyAttr.CsmaCcaPeriod,
          .xCcaLength=(WMBusPhyAttr.CsmaCcaLength<<4),
          .cMaxNb=WMBusPhyAttr.CsmaMaxNb,
          .nBuCounterSeed=WMBusPhyAttr.CsmaBuSeed,
          .cBuPrescaler=WMBusPhyAttr.CsmaBuPrescaler
        };
        
        S2LPCsmaInit(&csma_init);
        S2LPCsma(S_ENABLE);
        WMBusPhyAttr.CsmaRSSIThreshold = RxRSSIThTable[WMBusPhyAttr.DevType][WMBusPhyAttr.DevMode] + 20;
        S2LPRadioSetRssiThreshdBm(WMBusPhyAttr.CsmaRSSIThreshold);
      }
      
      S2LPPktWMbusSetPayloadLength(PhyTxFrame.length);
      S2LPCmdStrobeFlushTxFifo();
      S2LPGpioIrqClearStatus();
      
      S2LPIrqs irq_mask_for_tx;
      uint32_t tempValue = 0x00000000;
      irq_mask_for_tx = (*(S2LPIrqs*)&tempValue);
      irq_mask_for_tx.IRQ_TX_DATA_SENT=S_SET;
      irq_mask_for_tx.IRQ_MAX_BO_CCA_REACH=S_SET;
      S2LPGpioIrqInit(&irq_mask_for_tx);
      
      /* set the almost empty threshold */                                              
      S2LPFifoSetAlmostEmptyThresholdTx(ALMOST_EMPTY_THRESHOLD); 
      S2LPGpioIrqConfig(TX_FIFO_ALMOST_EMPTY , S_ENABLE);   
      
      /* write the linear fifo with the first chunk of bytes of payload */
      S2LPSpiWriteFifo(NUM_BYTE_FIRST_FIFO_WRITE, PhyTxFrame.DataBuff);
      
      /* store the number of transmitted bytes */
      PhyTxFrame.lenRemain = PhyTxFrame.length-NUM_BYTE_FIRST_FIFO_WRITE;
      
      /* USED FROM NOW AS INDEX (in ISR) */
      PhyTxFrame.length = NUM_BYTE_FIRST_FIFO_WRITE;
     
      if(ex_cmd)                                                             
        /* send the TX command */
        S2LPCmdStrobeTx();
      break;
    }
    
  case PHY_STATE_TXCW :
    {
      if(xShutdown == S_TRUE)
      {
        S2LPExitShutdown();
        S2LPManagementRangeExtInit();
      }
      else if(ex_cmd && g_xStatus.MC_STATE == MC_STATE_STANDBY)
      {
        S2LPCmdStrobeReady();
      }
      else if(ex_cmd && g_xStatus.MC_STATE == MC_STATE_RX)
      {

        S2LPCmdStrobeSabort();
      }      
      
      tx_done_flag = S_FALSE;
      WMBus_PhyConfig(TX_MODE);
      
      /* set un-modulated modulation for CW Tx signal */
     S2LPRadioSetModulation(MOD_NO_MOD);
  
     /* test mode configuration */
     S2LPPacketHandlerSetTxMode(PN9_TX_MODE);
      
      if(ex_cmd)
        /* send the TX command */
        S2LPCmdStrobeTx();
      break;
      
    }
    
  case PHY_STATE_TXPN9 :
    {
      if(xShutdown == S_TRUE)
      {
        S2LPExitShutdown();
        S2LPManagementRangeExtInit();
      }
      else if(ex_cmd && g_xStatus.MC_STATE == MC_STATE_STANDBY)
      {
        S2LPCmdStrobeReady();
      }
      else if(ex_cmd && g_xStatus.MC_STATE == MC_STATE_RX)
      {

        S2LPCmdStrobeSabort();
      }      
      
      tx_done_flag = S_FALSE;
      WMBus_PhyConfig(TX_MODE);
      
     /* test mode configuration */
     S2LPPacketHandlerSetTxMode(PN9_TX_MODE);
        
      if(ex_cmd)
        /* send the TX command */
        S2LPCmdStrobeTx();
      break;
      
    }
    
    

  case PHY_STATE_RX:
    {
      if(xShutdown == S_TRUE)
      {
        S2LPExitShutdown();
        S2LPManagementRangeExtInit();
      }
      else if(ex_cmd && g_xStatus.MC_STATE == MC_STATE_STANDBY)
      {
        S2LPCmdStrobeReady();
      }
      else if(ex_cmd && g_xStatus.MC_STATE == MC_STATE_RX)
      {
        return PHY_STATUS_SUCCESS;
      }

      
      rx_done_flag = S_FALSE;
      rx_timeOut = S_FALSE;
      WMBus_PhyConfig(RX_MODE);
      
      S2LPPktWMbusSetPayloadLength(0xfff0);
      
      S2LPFifoSetAlmostFullThresholdRx(DEVICE_FIFO_SIZE-AF_INITIAL_THR);
      S2LPCmdStrobeFlushRxFifo();
      
      /* instead of the single call we can write the 3 IRQ regs involved
      with a single SPI transaction.
      The following is equivalent to
      S2LPIrqDeInit(NULL);
      S2LPIrq(RX_FIFO_ALMOST_FULL,S_ENABLE);
      S2LPIrq(VALID_SYNC,S_ENABLE);*/
      
      S2LPIrqs irq_mask_for_rx;
      uint32_t tempValue = 0x00000000;
      irq_mask_for_rx = (*(S2LPIrqs *)&tempValue);
      irq_mask_for_rx.IRQ_RX_FIFO_ALMOST_FULL=S_SET;
      irq_mask_for_rx.IRQ_RX_FIFO_ERROR=S_SET;

      
      if (RxPER_flag != S_TRUE)
      {
      
        if(WMBusPhyAttr.DevType==METER)
        { 
/* method only valid for T2 and C2 modes */
#ifdef FASTRX_TIMER_AND_RXTIMEOUT_SYNC
          if ((WMBusPhyAttr.DevMode==T2_MODE) || (WMBusPhyAttr.DevMode==C2_MODE))
          {
          /* enable Fast Rx timeout as 1st Timer (RSSI) */
          S2LpTimerFastRxTermTimer(S_ENABLE);
          /* enable Rx timeout (2nd timer) for SQI check */
          S2LPTimerSetRxTimerStopCondition(SQI_ABOVE_THRESHOLD);
          /* enable both Rx timeout IRQs */
          irq_mask_for_rx.IRQ_RX_SNIFF_TIMEOUT=S_SET;
          irq_mask_for_rx.IRQ_RX_TIMEOUT=S_SET;
          }
          else /* default case (S2-mode) : only Rx Timeout with Sync pattern detection ... */
          /* if (WMBusPhyAttr.DevMode==S2_MODE) */  
          {

          S2LPTimerSetRxTimerStopCondition(SQI_ABOVE_THRESHOLD);

          irq_mask_for_rx.IRQ_RX_TIMEOUT=S_SET;
          }
#else
          

          S2LPTimerSetRxTimerStopCondition(SQI_ABOVE_THRESHOLD);
          irq_mask_for_rx.IRQ_RX_TIMEOUT=S_SET;
#endif

        }
        else
        {
          S2LPTimerSetRxTimerStopCondition(SQI_ABOVE_THRESHOLD);
        }
      }
      else
      {
        
        /* force continuous Rx in RxPER test mode */
        SET_INFINITE_RX_TIMEOUT();
        
      }
     
      S2LPGpioIrqInit(&irq_mask_for_rx);
      S2LPGpioIrqClearStatus();
      
      rx_state=SM_RX_FIRST;
      
      if(ex_cmd)
        S2LPCmdStrobeRx(); 
      
      break;
    }

  /* patch   : fast Rx programmation as re-start of Rx */
  case PHY_STATE_RX_RESTART:
    {
      
      rx_done_flag = S_FALSE;
      rx_timeOut = S_FALSE; 
      
      /* only Rx FIFO almost full Irq to enable again */
      S2LPGpioIrqConfig(RX_FIFO_ALMOST_FULL,S_ENABLE);
      //S2LPGpioIrqClearStatus();
      
      rx_state=SM_RX_FIRST;
      
      if(ex_cmd)
        S2LPCmdStrobeRx(); 
      
      break;
    }    
  
  case PHY_STATE_RX_NO_TIM:
    {
      if(xShutdown == S_TRUE)
      {
        S2LPExitShutdown();
        S2LPManagementRangeExtInit();
      }
      else if(ex_cmd && g_xStatus.MC_STATE == MC_STATE_STANDBY)
      {
        S2LPCmdStrobeReady();
      }
      else if(ex_cmd && g_xStatus.MC_STATE == MC_STATE_RX)
      {
        return PHY_STATUS_SUCCESS;
      }
      
      rx_done_flag = S_FALSE;
      rx_timeOut = S_FALSE;
      WMBus_PhyConfig(RX_MODE);
      
      uint8_t tmp=0;
      S2LPSpiWriteRegisters(0x46, 1, &tmp);
      
      S2LPPktWMbusSetPayloadLength(0xfff0);
      
      S2LPFifoSetAlmostFullThresholdRx(DEVICE_FIFO_SIZE-AF_INITIAL_THR);
      S2LPCmdStrobeFlushRxFifo();
      
      /* instead of the single call we can write the 3 IRQ regs involved
      with a single SPI transaction.
      The following is equivalent to
      S2LPIrqDeInit(NULL);
      S2LPIrq(RX_FIFO_ALMOST_FULL,S_ENABLE);
      S2LPIrq(VALID_SYNC,S_ENABLE);*/
      
      S2LPIrqs irq_mask_for_rx;
      uint32_t tempValue = 0x00000000;
      irq_mask_for_rx = (*(S2LPIrqs *)&tempValue);
      irq_mask_for_rx.IRQ_RX_FIFO_ALMOST_FULL=S_SET;
      irq_mask_for_rx.IRQ_RX_FIFO_ERROR=S_SET;
      /* + Patch   : not necessary on gateway side */
      //irq_mask_for_rx.IRQ_VALID_PREAMBLE=S_SET;   
      //irq_mask_for_rx.IRQ_VALID_SYNC=S_SET;  
      /* - Patch   : not necessary on gateway side */
      
      S2LPTimerSetRxTimerStopCondition(SQI_ABOVE_THRESHOLD);
      
      S2LPGpioIrqInit(&irq_mask_for_rx);
      S2LPGpioIrqClearStatus();
      
      rx_state=SM_RX_FIRST;
      
      if(ex_cmd)
        S2LPCmdStrobeRx(); 
      
      break;
    }
  case PHY_STATE_STANDBY:
    {
      if(xShutdown == S_TRUE)
      {
        S2LPExitShutdown();
        S2LPManagementRangeExtInit();
      }
      else if(g_xStatus.MC_STATE == MC_STATE_STANDBY)
      {
        S2LPCmdStrobeReady();
      }
      else if(g_xStatus.MC_STATE == MC_STATE_RX)
      {
        S2LPCmdStrobeSabort();
      }
      S2LPCmdStrobeStandby();
      break;
    }

  case  PHY_STATE_RXPER : case PHY_STATE_INVALID:
	  break;
  }
  return PHY_STATUS_SUCCESS;
}


/**
* @brief  This function gets the RSSI when the radio is not receiving any packet.
* @retval RSSI value in dBm. Value 0 is error, meaning that RSSI can't be measured.
*/
int32_t WMBus_PhyGetIdleRssi(void)
{
  uint32_t t0;
  int32_t rssi=0;
  
  WMBusPhyState_t phyState;
  while(WMBus_PhyGetState(&phyState)==PHY_STATUS_ERROR_INVALID_STATE);
  
  if(rx_state==SM_RX_FIRST)
  {
    S2LPCmdStrobeSabort();
    S2LPCmdStrobeRx();
    
    t0=WMBus_TimGetTimestamp();
    
    /* delay */
    while(!TimeDelay(t0,30));
    
    S2LPCmdStrobeSabort();
    
    rssi=(int32_t)S2LPRadioGetRssidBm();
    
    if(phyState == PHY_STATE_RX)
    {
      ReInitRxCycle();
    }
  }
  
  return rssi;
}


/**
* @brief  This function will transmit the buffer.
* @param  *buff: The pointer to the buffer to be transmitted.
* @param  length: The length of the data buffer.
* @param  void(*TxDataSentStateHandler)(uint8_t): This callback function is 
*            invoked after successful transmission.
* @retval PHY_STATUS.
*                       
*/
PHY_STATUS  WMBus_PhyTxDataHandleMode (uint8_t *buff,
                                       uint16_t length,
                                       void(*TxDataSentStateHandler)(uint8_t))
{
  uint8_t cut;
  
  PhyTxDataSentStateHandler = TxDataSentStateHandler;
  PhyTxFrame.DataBuff = buff;
  PhyTxFrame.length = length;

  S2LPSpiReadRegisters(0xF1, 1, &cut);
  if((cut&0xC0)==0xC0)
  {
    if(buff[length-1]&0x01)
    {
      S2LPPktWMbusSetPostamblePattern(1);
    }
    else
    {
      S2LPPktWMbusSetPostamblePattern(2);
    }
  } 

  
  if(PHY_STATUS_SUCCESS == WMBus_PhySetState (PHY_STATE_TX))
  { 
    return PHY_STATUS_SUCCESS;
  }
  return PHY_STATUS_ERROR_TX_TIMEOUT;
}


/**
* @brief  This function will return the current PHY configuration.
* @param  *WMBusCurrPar: The pointer to WMBus_PhyAttr_t structure.
* @retval PHY_STATUS.
*                       
*/
PHY_STATUS  WMBus_PhyReadConfig (WMBus_PhyAttr_t *WMBusCurrPar)
{
  memcpy(WMBusCurrPar, &WMBusPhyAttr, sizeof(WMBusPhyAttr));
  return PHY_STATUS_SUCCESS;
}

/**
* @brief  This Routine will return the WMBus PHY attributes.
* @param  attr: The WMBus attribute
*        *ParamVal: The pointer to the variable to set the attribute value
* @retval PHY_STATUS: The status of execution
*/
PHY_STATUS WMBus_PhyGetAttr(WMBUS_PHY_ATTR_t attr, int32_t *ParamVal)
{
  switch(attr)
  {
  case WMBUS_PHY_MODE:
    *ParamVal = (uint32_t)WMBusPhyAttr.DevMode;
    break;
    
  case WMBUS_PHY_DEVICE_TYPE:
    *ParamVal = (uint32_t)WMBusPhyAttr.DevType;
    break;
    
  case WMBUS_PHY_CHANNEL:
    *ParamVal = (uint32_t)WMBusPhyAttr.channel;
    break;
    
  case WMBUS_PHY_RESPONSE_TIME_MIN:
    *ParamVal = (uint32_t)(WMBusPhyAttr.ResponseTimeMin/**10*/);
    break;
    
  case WMBUS_PHY_RESPONSE_TIME_MAX:
    *ParamVal = (uint32_t)(WMBusPhyAttr.ResponseTimeMax/**10*/);
    break;
    
  case WMBUS_PHY_RESPONSE_TIME_TYP:
    *ParamVal = (uint32_t)(WMBusPhyAttr.ResponseTimeTyp/**10*/);
    break;
    
  case WMBUS_PHY_FAC_TIME_DELAY:
    *ParamVal = (uint32_t)(WMBusPhyAttr.FACDelay/**10*/);
    break;
    
  case WMBUS_PHY_FAC_TIMEOUT:
    *ParamVal = (uint32_t)(WMBusPhyAttr.FACTimeout/**10*/);
    break;
    
  case WMBUS_PHY_NOMINAL_TIME:
    *ParamVal = (uint32_t)(WMBusPhyAttr.NominalTime/**10*/);
    break;
    
  case WMBUS_PHY_PREAMBLE_LEN:
    *ParamVal = (uint32_t)WMBusPhyAttr.PreambleLength;
    break;
    
  case WMBUS_PHY_POSTAMBLE_LEN:
    *ParamVal = (uint32_t)WMBusPhyAttr.PostambleLength;
    break;
    
  case WMBUS_PHY_TX_INTERVAL:
    *ParamVal = (uint32_t)WMBusPhyAttr.TxInterval;
    break; 
        
  case WMBUS_PHY_OP_POWER:
    *ParamVal = (int32_t)WMBusPhyAttr.RFPower;
    break;
    
  case WMBUS_PHY_RX_RSSI_THRESHOLD:
    *ParamVal = WMBusPhyAttr.RxRSSIThreshold;
    break;
    
  case WMBUS_PHY_DIR:
    *ParamVal = WMBusPhyAttr.Direction;
    break;
    
  case WMBUS_PHY_RF_OFFSET:
    *ParamVal=WMBusPhyAttr.RFOffset;
    break;
    
  case WMBUS_PHY_CUST_DATARATE:
    *ParamVal=WMBusPhyAttr.CustDatarate;
    break;
    
  case WMBUS_PHY_CSMA_ENABLED:
     *ParamVal= WMBusPhyAttr.CsmaEnabled;
    break;
       
  case WMBUS_PHY_CSMA_RSSI_THRESHOLD:
    *ParamVal=  WMBusPhyAttr.CsmaRSSIThreshold;
  
  case WMBUS_PHY_CSMA_OVERRIDE_FAIL:
    *ParamVal= WMBusPhyAttr.CsmaOverrideFail;
    break;
    
  case WMBUS_PHY_CSMA_CCA_PERIOD:
    *ParamVal=WMBusPhyAttr.CsmaCcaPeriod;
    break;
  case WMBUS_PHY_CSMA_CCA_LENGTH:
    *ParamVal=WMBusPhyAttr.CsmaCcaLength;
    break;
  case WMBUS_PHY_CSMA_MAX_BO:
    *ParamVal=WMBusPhyAttr.CsmaMaxNb;
    break;
  case WMBUS_PHY_CSMA_BU_SEED:
    *ParamVal=WMBusPhyAttr.CsmaBuSeed;
    break;
  case WMBUS_PHY_CSMA_BU_PRESCALER:
    *ParamVal=WMBusPhyAttr.CsmaBuPrescaler;
    break;
    
  default:
    return PHY_STATUS_INVALID_ATTR;
  }
  return PHY_STATUS_SUCCESS;
}


/**
* @brief  This Routine will set the WMBus PHY attributes.
* @param  attr:The WMBus attribute
* @param  ParamVal: The value of the attribute.
* @retval PHY_STATUS: The status of execution.
*/
PHY_STATUS WMBus_PhySetAttr(WMBUS_PHY_ATTR_t attr, int32_t ParamVal)
{
  switch(attr)
  {
  case WMBUS_PHY_CHANNEL:
    WMBusPhyAttr.channel = (uint8_t)ParamVal;
    if(phy_init_called) StoreS2lpRegsBothDir();
    break;
    
  case WMBUS_PHY_PREAMBLE_LEN:
    WMBusPhyAttr.PreambleLength = (uint16_t)ParamVal;
    if(phy_init_called) StoreS2lpRegsBothDir();
    break;
    
  case WMBUS_PHY_POSTAMBLE_LEN:
    WMBusPhyAttr.PostambleLength=(uint8_t)ParamVal;
    if(phy_init_called) StoreS2lpRegsBothDir();
    break;
    
  case WMBUS_PHY_TX_INTERVAL:
    WMBusPhyAttr.TxInterval =(uint32_t)ParamVal;
    break;

  case WMBUS_PHY_OP_POWER:
    WMBusPhyAttr.RFPower = (float)ParamVal;
    if(phy_init_called) StoreS2lpRegsBothDir();
    
    break;
  
    
  case WMBUS_PHY_RX_RSSI_THRESHOLD:
    WMBusPhyAttr.RxRSSIThreshold = (int32_t)ParamVal;
    RxRSSIThTable[WMBusPhyAttr.DevType][WMBusPhyAttr.DevMode] = (int32_t)ParamVal;
    if(phy_init_called) StoreS2lpRegsBothDir();
    break;
  
  case WMBUS_PHY_RF_OFFSET:
    WMBusPhyAttr.RFOffset = (int32_t)ParamVal;
    if(phy_init_called) StoreS2lpRegsBothDir();   
    break;
  case WMBUS_PHY_CUST_DATARATE:
    WMBusPhyAttr.CustDatarate = (int32_t)ParamVal;
    if(phy_init_called) StoreS2lpRegsBothDir();   
    break;
    
  case WMBUS_PHY_CSMA_ENABLED:
    WMBusPhyAttr.CsmaEnabled = (uint8_t)ParamVal;
    break;
       
  case WMBUS_PHY_CSMA_RSSI_THRESHOLD:
    RxRSSIThTable[WMBusPhyAttr.DevType][WMBusPhyAttr.DevMode] = (int32_t)ParamVal;
    WMBusPhyAttr.CsmaRSSIThreshold = (int32_t)ParamVal;
    break;
    
  case WMBUS_PHY_CSMA_OVERRIDE_FAIL:
    WMBusPhyAttr.CsmaOverrideFail = (uint8_t)ParamVal;
    break;
  
  case WMBUS_PHY_CSMA_CCA_PERIOD:
    WMBusPhyAttr.CsmaCcaPeriod = (uint8_t)ParamVal;
    break;
  case WMBUS_PHY_CSMA_CCA_LENGTH:
    WMBusPhyAttr.CsmaCcaLength = (uint8_t)ParamVal;
    break;
  case WMBUS_PHY_CSMA_MAX_BO:
    WMBusPhyAttr.CsmaMaxNb = (uint8_t)ParamVal;
    break;
  case WMBUS_PHY_CSMA_BU_SEED:
    WMBusPhyAttr.CsmaBuSeed = (uint16_t)ParamVal;
    break;
  case WMBUS_PHY_CSMA_BU_PRESCALER:
    WMBusPhyAttr.CsmaBuPrescaler = (uint8_t)ParamVal;
    break;
    
   case WMBUS_PHY_FAC_TIME_DELAY:
    WMBusPhyAttr.FACDelay= (uint32_t)ParamVal;
    break;
    
  case WMBUS_PHY_FAC_TIMEOUT:
    WMBusPhyAttr.FACTimeout= (uint32_t)ParamVal;
    break;
  case WMBUS_PHY_FRAME_FORMAT:
    WMBusPhyAttr.FrameFormat = (uint8_t)ParamVal; 
  default:
    return PHY_STATUS_INVALID_ATTR;
  }  
  return PHY_STATUS_SUCCESS;
}


/**
* @brief  This Routine will Check wheather the communication is bidirectional.
* @param  None
* @retval Returns 1 Bidirectional communication ,Otherwise 0.
*/
uint8_t WMBus_PhyIsTwoWayMode(void)
{
  switch(WMBusPhyAttr.DevMode)
  {
  case S2_MODE:
  case R2_MODE:
  case T2_MODE:
  case N2_MODE:
  case C2_MODE:
  case F2_MODE:
  case F2m_MODE:
    return 1;

  case S1_MODE : case S1m_MODE: case T1_MODE: case ALL_MODE: case N1_MODE: case C1_MODE: case INVALID_MODE:
	  break;

  }
  return 0;
}

/**
* @brief  This Routine will return the WMBus PHY attributes.
* @param  None.
* @retval Returns the Device Type.
*/
WMBusDeviceType_t WMBus_PhyGetDevType(void)
{
  return WMBusPhyAttr.DevType;
}

/**
* @brief  This Routine will return minimum Response time in MicroSecond.
* @param  None.
* @retval Returns Response Time.
*/
uint32_t WMBus_PhyGetResponseTimeMin(void)
{
  /* returned as 100us units*/
  return((uint32_t)WMBusPhyAttr.ResponseTimeMin*10);
}

/**
* @brief  This Routine will return maximum Response time in MicroSecond.
* @param  None.
* @retval Returns Response Time.
*/
uint32_t WMBus_PhyGetResponseTimeMax(void)
{
  /* returned as 100us units*/
   return((uint32_t)WMBusPhyAttr.ResponseTimeMax*10);
}

/**
* @brief  This Routine will return typical Response time(Tro)in MicroSecond.
* @param  None.
* @retval Returns Response Time.
*/
uint32_t WMBus_PhyGetResponseTimeTyp(void)
{
  /* returned as 100us units*/
  return((uint32_t)WMBusPhyAttr.ResponseTimeTyp*10);
}

/**
* @brief  This Routine will return FAC delay time(Txd)in MicroSecond.
* @param  None.
* @retval Returns FAC delay time.
*/
uint32_t WMBus_PhyGetFACDelay(void)
{
  /* returned as 100us units*/
    return((uint32_t)WMBusPhyAttr.FACDelay*10);
}      

/**
* @brief  This Routine will return FAC Timeout time(Tto)in MicroSecond.
* @param  None.
* @retval Returns FAC timeout Time.
*/
uint32_t WMBus_PhyGetFACTimeout(void)
{
  /* returned as 100us units*/
  return((uint32_t)WMBusPhyAttr.FACTimeout*10);
} 

/**
* @brief  This Routine will return Nominal Time(Tnom)in MicroSecond.
* @param  None.
* @retval Returns Nominal Time.
*/
uint32_t WMBus_PhyGetNominalTime(void)
{
  /* returned as 100us units*/
  return((uint32_t)WMBusPhyAttr.NominalTime*10);
}



/****************** BEGIN PACKET QUEUE RELATED FUNCTIONS ********************/
/**
* @brief  This Routine will return number of received packets.
* @param  None.
* @retval Returns the number of received packets.
*/
uint8_t WMBus_PhyGetNumRxPackets(void)
{
  return nRxPackets;
}


/**
* @brief  This Routine will Pop byte from Queue.
* @param  None.
* @retval Returns  the Byte.
*/
uint8_t WMBus_PhyPopBytefromQueue(void)
{ 
  uint8_t c = 0;
  c = RxBuffQueue[RxFrameDescQueue[RxFrameDescTail].start_index];
  RxFrameDescQueue[RxFrameDescTail].start_index=(RxFrameDescQueue[RxFrameDescTail].start_index+1)%RX_QUEUE_SIZE;
  return c;
}


/**                                                                                          
* @brief  This Routine will copy byte from Queue.
* @param  destBuff: the pointer to the destination buffer.
* @param  len: Number of Byte.
* @retval None.
*/
void WMBus_PhyCopyBytefromQueue(uint8_t *destBuff,uint16_t len)
{
  /* +patch   : improve management of function in case end of buffer */
  //memcpy(destBuff,(uint8_t*)&RxBuffQueue[RxFrameDescQueue[RxFrameDescTail].start_index],len);
  if ((RxFrameDescQueue[RxFrameDescTail].start_index +len) <= RX_QUEUE_SIZE) {
    memcpy(destBuff,(uint8_t*)&RxBuffQueue[RxFrameDescQueue[RxFrameDescTail].start_index],len);
  } else {
    uint16_t remain = RX_QUEUE_SIZE - RxFrameDescQueue[RxFrameDescTail].start_index;
    memcpy(destBuff,(uint8_t*)&RxBuffQueue[RxFrameDescQueue[RxFrameDescTail].start_index],remain);
    memcpy(destBuff+remain,(uint8_t*)&RxBuffQueue[0],len-remain);
  }
}



/**
* @brief  This Function will return Packet Length.
* @param  None.
* @retval Returns the Packet Length.
*/
uint16_t WMBus_PhyGetPacketLength(void)
{
   
  return RxFrameDescQueue[RxFrameDescTail].size;
}

/**
* @brief  This Function will return the RSSI Value.
* @param  None.
* @retval Returns the RSSI Value.
*/
int32_t WMBus_PhyGetPacketRssi(void)
{
  return RxFrameDescQueue[RxFrameDescTail].rssi;
}

/**
* @brief  This Function will return Packet Format.
* @param  None.
* @retval Returns the Packet Format.
*/
FrameFormatType_t WMBus_PhyGetPacketFormat(void)
{
  return RxFrameDescQueue[RxFrameDescTail].format;
}

/**
* @brief  This Function will return Packet Format.
* @param  None.
* @retval Returns the Packet Format.
*/
WMBusMode_t WMBus_PhyGetRecvSubmode(void)
{
  return RxFrameDescQueue[RxFrameDescTail].dynamic_submode;
}

/**
* @brief  This Function will return Time Stamp.
* @param  None.
* @retval Returns the Time Stamp.
*/
uint32_t WMBus_PhyGetPacketRxTimestamp(void)
{
  return RxFrameDescQueue[RxFrameDescTail].rx_timestamp;
}

/**
* @brief  This Function will Perform Pop Operation from the Phy queue.
* @param  None.
* @retval None.
*/
void WMBus_PhyPopDesc(void)
{
  if(nRxPackets)
  {
    RxFrameDescTail=(RxFrameDescTail+1)%RX_FRAME_DESC_Q_SIZE;
    nRxPackets--;
  }
}

/****************** END PACKET QUEUE RELATED FUNCTIONS ******************/

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
