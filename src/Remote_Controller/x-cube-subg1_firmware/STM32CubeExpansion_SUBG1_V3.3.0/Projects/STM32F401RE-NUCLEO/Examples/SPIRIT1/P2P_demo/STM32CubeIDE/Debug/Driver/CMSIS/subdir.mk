################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/enric/Downloads/x-cube-subg1_firmware/STM32CubeExpansion_SUBG1_V3.3.0/Projects/STM32F401RE-NUCLEO/Examples/SPIRIT1/P2P_demo/Src/system_stm32f4xx.c 

OBJS += \
./Driver/CMSIS/system_stm32f4xx.o 

C_DEPS += \
./Driver/CMSIS/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Driver/CMSIS/system_stm32f4xx.o: C:/Users/enric/Downloads/x-cube-subg1_firmware/STM32CubeExpansion_SUBG1_V3.3.0/Projects/STM32F401RE-NUCLEO/Examples/SPIRIT1/P2P_demo/Src/system_stm32f4xx.c Driver/CMSIS/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_STM32F4XX_NUCLEO -DUSE_HAL_DRIVER -DX_NUCLEO_IDS01A4 -DUSE_SYSTICK_DELAY -DP2P_DEMO -DSPIRIT1_ST_SHIELD -DSTM32F4 -DNO_EEPROM -DSTM32F401xE -DUSE_SPIRIT1_DEFAULT -DUSE_STDPERIPH_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/X-NUCLEO-IDS01Ax/Inc -I../../../../../../../Drivers/BSP/STM32F4xx-Nucleo -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Library/Inc -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Util/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Driver-2f-CMSIS

clean-Driver-2f-CMSIS:
	-$(RM) ./Driver/CMSIS/system_stm32f4xx.cyclo ./Driver/CMSIS/system_stm32f4xx.d ./Driver/CMSIS/system_stm32f4xx.o ./Driver/CMSIS/system_stm32f4xx.su

.PHONY: clean-Driver-2f-CMSIS

