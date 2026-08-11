/**
  @page S2-LP Expansion Board(X-NUCLEO-S2868A1/X-NUCLEO-S2868A2/X-NUCLEO-S2915A1) for STM32 Nucleo(NUCLEO-L152RE), STM32 Nucleo(NUCLEO-L073RZ) and STM32 Nucleo(NUCLEO-F401RE) 
        Boards as wM-Bus Meter/Concentrator Example

  @verbatim
  ******************** (C) COPYRIGHT 2021 STMicroelectronics *******************
  * @file    WMBusExample/readme.txt
  * @author  SRA - NOIDA / RF application group - AMG
  * @version V3.3.0
  * @date    27-Apr-2021
  * @brief   This application is an example to use wM-Bus implementation between 
             a STM32Nucleo(NUCLEO-L152RE) / STM32Nucleo(NUCLEO-L073RZ) / STM32 Nucleo(NUCLEO-F401RE) and S2-LP expansion 
             boards (X-NUCLEO-S2868A1/X-NUCLEO-S2868A1/X-NUCLEO-S2868A2/X-NUCLEO-S2915A1) acting like a meter and a concentrator 
             connected to a PC GUI
  ******************************************************************************
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
  @endverbatim

@par Example Description

This example is a part of the WMBus Library package using STM32Cube firmware. 

This is a demo implementation of the standard WMBUS protocol. 
It takes in account the physical and data link layers defined by the standard.

The example shows how to configure the packets in order to transmit or 
receive WMBUS packets.

CONC   :   NUCLEO-L152RE + X-NUCLEO-S2868A1/X-NUCLEO-S2868A2/X-NUCLEO-S2915A1
METER  :   NUCLEO-L152RE + X-NUCLEO-S2868A1/X-NUCLEO-S2868A2/X-NUCLEO-S2915A1
OR

CONC   :   NUCLEO-L073RZ + X-NUCLEO-S2868A1/X-NUCLEO-S2868A2/X-NUCLEO-S2915A1
METER  :   NUCLEO-L073RZ + X-NUCLEO-S2868A1/X-NUCLEO-S2868A2/X-NUCLEO-S2915A1
OR

CONC   :   NUCLEO-F401RE + X-NUCLEO-S2868A1/X-NUCLEO-S2868A2/X-NUCLEO-S2915A1
METER  :   NUCLEO-F401RE + X-NUCLEO-S2868A1/X-NUCLEO-S2868A2/X-NUCLEO-S2915A1

Upon each push of User button B1 on METER,it sends SND-IR message to the 
Concentrator. The Concentrator sends CNF-IR to the Meter in Response
(Response is sent only in bi-directional Modes).

@par Hardware and Software environment


  
  - User can select desired frequency by defining:
    => DEVICE_BAND_868MHz : Supports S/T/R modes of WMBus(user_config.h)
  
  - Device Type and Examples is selectable by dropdwon in workspace 
  - For GUI connectivity,Program the 1st Board with S2LP-Conc-PC-GUI workspace.
    Program the 2nd board(METER) with S2LP-Meter-PC-GUI workspace, 
    If conutinous Mode is selected(only for workspace "S2LP_METER_PC_GUI") in "user_config.h" file, The meter will start transmissting the packet in every 4 Second.
    If conutinous Mode is not selected in "user_config.h" file,press button B1 to send Message.

    There are other Example of WMBUS in pair(METER/CONC).
    The binary in pair :
    {
     S2LP_WMBUS_CONC_ACC_DMD_XX.bin
     S2LP_WMBUS_METER_ACC_DMD_XX.bin
    }

    {
    S2LP_WMBUS_METER_INSTALLATION_XX.bin
    S2LP_WMBUS_CONC_INSTALLATION_XX.bin
    }

    {
     S2LP_WMBUS_METER_SND_UD2_XX.bin
     S2LP_WMBUS_CONC_SND_UD2_XX.bin
    }

    {
     S2LP_WMBUS_METER_FAC_XX.bin
     S2LP_WMBUS_CONC_FAC_XX.bin
    }

    {
     S2LP_WMBUS_METER_TPL_XX.bin
     S2LP_WMBUS_CONC_TPL_XX.bin
    }

    {
     S2LP_WMBUS_CONC_GUI_XX.bin
    }
 
    Program one board by METER and Other by CONC.After programming press the button on the Meter. The Messages will be visible on the Terminal on both side.
    The baud rate is 115200.
   
  - Desired WMBus features can be selected in user_config.h file by 
    changing below section of code:
  
  /* Wireless Meter Bus Specific Configuration--------------------------------*/
  
/********************WMBUS Sub-Mode Configuration******************************/
#define DEVICE_WMBUS_MODE               S2_MODE                               
#define DEVICE_WMBUS_CHANNEL            CHANNEL_5
#define WMBUS_FRAME_FORMAT              FRAME_FORMAT_A
#define TX_OUTPUT_POWER                 (10)/*Between [-3.0 ,+27.0] if the RF 
                                               board has an external PA. 
                                               Otherwise between[-30.0 ,+13.0]*/

/***********************************User Settings******************************/
//#define ENABLE_CONTINUOUS_TX                /*Define only for workspace "S2LP_METER_PC_GUI" */
              
#ifdef ENABLE_CONTINUOUS_TX
#define CONTINUOUS_TX_TIME_INTERVAL     4000  /*In MiliSecond*/
#endif                                                

#define UART_BAUD_RATE                 115200
  

  /*--------------------------------------------------------------------------*/
    
  - Connect the board to PC through USB - mini connector of nucleo board.

  - Flash the board for desired configuration as explained above.


Known Limitation: STM32CubeIDE is not supported on NUCLEO-L073RZ due to Flash Size Limitation

  
@par How to use it ? 

In order to make the program work, you must do the following:
 - WARNING: before opening the project with any toolchain be sure your folder
   installation path is not too in-depth since the toolchain may report errors
   after building.
 - The tested tool chain and environment is explained in the Release notes
 - Open the suitable toolchain (IAR v8.2) and open the project 
    for the required STM32 Nucleo board
 - Rebuild all files and load your image into target memory.
 - Run the example.
 - Alternatively, you can download the pre-built binaries in "Binary" 
   folder included in the distributed package.

 - IMPORTANT NOTE: To avoid issues with USB connection 
   (mandatory if you have USB 3.0), it is suggested to update the ST-Link/V2
   firmware for STM32 Nucleo boards to the latest version.Please refer to the 
   readme.txt file in the Applications directory for details.



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/