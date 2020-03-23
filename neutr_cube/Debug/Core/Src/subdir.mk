################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/main.c \
../Core/Src/stm32f0xx_hal_msp.c \
../Core/Src/stm32f0xx_it.c \
../Core/Src/system_stm32f0xx.c 

OBJS += \
./Core/Src/main.o \
./Core/Src/stm32f0xx_hal_msp.o \
./Core/Src/stm32f0xx_it.o \
./Core/Src/system_stm32f0xx.o 

C_DEPS += \
./Core/Src/main.d \
./Core/Src/stm32f0xx_hal_msp.d \
./Core/Src/stm32f0xx_it.d \
./Core/Src/system_stm32f0xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F072xB -I../USB_DEVICE/App -I../USB_DEVICE/Target -I"F:/GIT/neutron/neutr_cube/Core/Inc" -I"F:/GIT/neutron/neutr_cube/Drivers/STM32F0xx_HAL_Driver/Inc" -I"F:/GIT/neutron/neutr_cube/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy" -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/AUDIO/Inc -I"F:/GIT/neutron/neutr_cube/Drivers/CMSIS/Device/ST/STM32F0xx/Include" -I"F:/GIT/neutron/neutr_cube/Drivers/CMSIS/Include" -I"F:/GIT/neutron/neutr_cube/neutr_src"  -O3 -pedantic -Wall -Wextra -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


