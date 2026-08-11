################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/enric/Downloads/x-cube-subg1_firmware/STM32CubeExpansion_SUBG1_V3.3.0/Projects/STM32F401RE-NUCLEO/Examples/SPIRIT1/P2P_demo/Src/cube_hal_f4.c \
C:/Users/enric/Downloads/x-cube-subg1_firmware/STM32CubeExpansion_SUBG1_V3.3.0/Projects/STM32F401RE-NUCLEO/Examples/SPIRIT1/P2P_demo/Src/main.c \
C:/Users/enric/Downloads/x-cube-subg1_firmware/STM32CubeExpansion_SUBG1_V3.3.0/Projects/STM32F401RE-NUCLEO/Examples/SPIRIT1/P2P_demo/Src/radio_appli.c \
../Application/USER/sha256.c \
C:/Users/enric/Downloads/x-cube-subg1_firmware/STM32CubeExpansion_SUBG1_V3.3.0/Projects/STM32F401RE-NUCLEO/Examples/SPIRIT1/P2P_demo/Src/stm32f4xx_it.c \
../Application/USER/syscalls.c \
../Application/USER/sysmem.c 

OBJS += \
./Application/USER/cube_hal_f4.o \
./Application/USER/main.o \
./Application/USER/radio_appli.o \
./Application/USER/sha256.o \
./Application/USER/stm32f4xx_it.o \
./Application/USER/syscalls.o \
./Application/USER/sysmem.o 

C_DEPS += \
./Application/USER/cube_hal_f4.d \
./Application/USER/main.d \
./Application/USER/radio_appli.d \
./Application/USER/sha256.d \
./Application/USER/stm32f4xx_it.d \
./Application/USER/syscalls.d \
./Application/USER/sysmem.d 


# Each subdirectory must supply rules for building sources it contributes
Application/USER/cube_hal_f4.o: C:/Users/enric/Downloads/x-cube-subg1_firmware/STM32CubeExpansion_SUBG1_V3.3.0/Projects/STM32F401RE-NUCLEO/Examples/SPIRIT1/P2P_demo/Src/cube_hal_f4.c Application/USER/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_STM32F4XX_NUCLEO -DUSE_HAL_DRIVER -DX_NUCLEO_IDS01A4 -DUSE_SYSTICK_DELAY -DP2P_DEMO -DSPIRIT1_ST_SHIELD -DSTM32F4 -DNO_EEPROM -DSTM32F401xE -DUSE_SPIRIT1_DEFAULT -DUSE_STDPERIPH_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/X-NUCLEO-IDS01Ax/Inc -I../../../../../../../Drivers/BSP/STM32F4xx-Nucleo -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Library/Inc -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Util/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/USER/main.o: C:/Users/enric/Downloads/x-cube-subg1_firmware/STM32CubeExpansion_SUBG1_V3.3.0/Projects/STM32F401RE-NUCLEO/Examples/SPIRIT1/P2P_demo/Src/main.c Application/USER/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_STM32F4XX_NUCLEO -DUSE_HAL_DRIVER -DX_NUCLEO_IDS01A4 -DUSE_SYSTICK_DELAY -DP2P_DEMO -DSPIRIT1_ST_SHIELD -DSTM32F4 -DNO_EEPROM -DSTM32F401xE -DUSE_SPIRIT1_DEFAULT -DUSE_STDPERIPH_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/X-NUCLEO-IDS01Ax/Inc -I../../../../../../../Drivers/BSP/STM32F4xx-Nucleo -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Library/Inc -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Util/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/USER/radio_appli.o: C:/Users/enric/Downloads/x-cube-subg1_firmware/STM32CubeExpansion_SUBG1_V3.3.0/Projects/STM32F401RE-NUCLEO/Examples/SPIRIT1/P2P_demo/Src/radio_appli.c Application/USER/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_STM32F4XX_NUCLEO -DUSE_HAL_DRIVER -DX_NUCLEO_IDS01A4 -DUSE_SYSTICK_DELAY -DP2P_DEMO -DSPIRIT1_ST_SHIELD -DSTM32F4 -DNO_EEPROM -DSTM32F401xE -DUSE_SPIRIT1_DEFAULT -DUSE_STDPERIPH_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/X-NUCLEO-IDS01Ax/Inc -I../../../../../../../Drivers/BSP/STM32F4xx-Nucleo -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Library/Inc -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Util/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/USER/%.o Application/USER/%.su Application/USER/%.cyclo: ../Application/USER/%.c Application/USER/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_STM32F4XX_NUCLEO -DUSE_HAL_DRIVER -DX_NUCLEO_IDS01A4 -DUSE_SYSTICK_DELAY -DP2P_DEMO -DSPIRIT1_ST_SHIELD -DSTM32F4 -DNO_EEPROM -DSTM32F401xE -DUSE_SPIRIT1_DEFAULT -DUSE_STDPERIPH_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/X-NUCLEO-IDS01Ax/Inc -I../../../../../../../Drivers/BSP/STM32F4xx-Nucleo -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Library/Inc -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Util/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"
Application/USER/stm32f4xx_it.o: C:/Users/enric/Downloads/x-cube-subg1_firmware/STM32CubeExpansion_SUBG1_V3.3.0/Projects/STM32F401RE-NUCLEO/Examples/SPIRIT1/P2P_demo/Src/stm32f4xx_it.c Application/USER/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_STM32F4XX_NUCLEO -DUSE_HAL_DRIVER -DX_NUCLEO_IDS01A4 -DUSE_SYSTICK_DELAY -DP2P_DEMO -DSPIRIT1_ST_SHIELD -DSTM32F4 -DNO_EEPROM -DSTM32F401xE -DUSE_SPIRIT1_DEFAULT -DUSE_STDPERIPH_DRIVER -c -I../../Inc -I../../../../../../../Drivers/CMSIS/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../../../../../../../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../../../../../../../Drivers/STM32F4xx_HAL_Driver/Inc -I../../../../../../../Drivers/BSP/X-NUCLEO-IDS01Ax/Inc -I../../../../../../../Drivers/BSP/STM32F4xx-Nucleo -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Library/Inc -I../../../../../../../Drivers/BSP/Components/spirit1/SPIRIT1_Util/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Application-2f-USER

clean-Application-2f-USER:
	-$(RM) ./Application/USER/cube_hal_f4.cyclo ./Application/USER/cube_hal_f4.d ./Application/USER/cube_hal_f4.o ./Application/USER/cube_hal_f4.su ./Application/USER/main.cyclo ./Application/USER/main.d ./Application/USER/main.o ./Application/USER/main.su ./Application/USER/radio_appli.cyclo ./Application/USER/radio_appli.d ./Application/USER/radio_appli.o ./Application/USER/radio_appli.su ./Application/USER/sha256.cyclo ./Application/USER/sha256.d ./Application/USER/sha256.o ./Application/USER/sha256.su ./Application/USER/stm32f4xx_it.cyclo ./Application/USER/stm32f4xx_it.d ./Application/USER/stm32f4xx_it.o ./Application/USER/stm32f4xx_it.su ./Application/USER/syscalls.cyclo ./Application/USER/syscalls.d ./Application/USER/syscalls.o ./Application/USER/syscalls.su ./Application/USER/sysmem.cyclo ./Application/USER/sysmem.d ./Application/USER/sysmem.o ./Application/USER/sysmem.su

.PHONY: clean-Application-2f-USER

