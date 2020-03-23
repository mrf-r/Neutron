################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../neutr_src/autotune.c \
../neutr_src/cr.c \
../neutr_src/hal_init.c \
../neutr_src/input_proc.c \
../neutr_src/knobs.c \
../neutr_src/menu.c \
../neutr_src/menu_page_def.c \
../neutr_src/menu_page_fltmode.c \
../neutr_src/menu_page_lfosync.c \
../neutr_src/menu_page_o1range.c \
../neutr_src/menu_page_o2range.c \
../neutr_src/menu_page_osync.c \
../neutr_src/menu_page_paraph.c \
../neutr_src/midi.c \
../neutr_src/neutr_exticalibration.c \
../neutr_src/neutr_lfo_13ex.c \
../neutr_src/neutr_lfo_1classic.c \
../neutr_src/neutr_lfo_2AndyHornBlower.c \
../neutr_src/neutr_lfo_3mono.c \
../neutr_src/neutr_lfo_4poly.c \
../neutr_src/neutr_lfo_5_mathtest.c \
../neutr_src/neutr_lfo_common.c \
../neutr_src/nvm.c \
../neutr_src/proc_pitch.c \
../neutr_src/sr.c \
../neutr_src/tables.c \
../neutr_src/voice_alloc.c 

OBJS += \
./neutr_src/autotune.o \
./neutr_src/cr.o \
./neutr_src/hal_init.o \
./neutr_src/input_proc.o \
./neutr_src/knobs.o \
./neutr_src/menu.o \
./neutr_src/menu_page_def.o \
./neutr_src/menu_page_fltmode.o \
./neutr_src/menu_page_lfosync.o \
./neutr_src/menu_page_o1range.o \
./neutr_src/menu_page_o2range.o \
./neutr_src/menu_page_osync.o \
./neutr_src/menu_page_paraph.o \
./neutr_src/midi.o \
./neutr_src/neutr_exticalibration.o \
./neutr_src/neutr_lfo_13ex.o \
./neutr_src/neutr_lfo_1classic.o \
./neutr_src/neutr_lfo_2AndyHornBlower.o \
./neutr_src/neutr_lfo_3mono.o \
./neutr_src/neutr_lfo_4poly.o \
./neutr_src/neutr_lfo_5_mathtest.o \
./neutr_src/neutr_lfo_common.o \
./neutr_src/nvm.o \
./neutr_src/proc_pitch.o \
./neutr_src/sr.o \
./neutr_src/tables.o \
./neutr_src/voice_alloc.o 

C_DEPS += \
./neutr_src/autotune.d \
./neutr_src/cr.d \
./neutr_src/hal_init.d \
./neutr_src/input_proc.d \
./neutr_src/knobs.d \
./neutr_src/menu.d \
./neutr_src/menu_page_def.d \
./neutr_src/menu_page_fltmode.d \
./neutr_src/menu_page_lfosync.d \
./neutr_src/menu_page_o1range.d \
./neutr_src/menu_page_o2range.d \
./neutr_src/menu_page_osync.d \
./neutr_src/menu_page_paraph.d \
./neutr_src/midi.d \
./neutr_src/neutr_exticalibration.d \
./neutr_src/neutr_lfo_13ex.d \
./neutr_src/neutr_lfo_1classic.d \
./neutr_src/neutr_lfo_2AndyHornBlower.d \
./neutr_src/neutr_lfo_3mono.d \
./neutr_src/neutr_lfo_4poly.d \
./neutr_src/neutr_lfo_5_mathtest.d \
./neutr_src/neutr_lfo_common.d \
./neutr_src/nvm.d \
./neutr_src/proc_pitch.d \
./neutr_src/sr.d \
./neutr_src/tables.d \
./neutr_src/voice_alloc.d 


# Each subdirectory must supply rules for building sources it contributes
neutr_src/%.o: ../neutr_src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F072xB -I../USB_DEVICE/App -I../USB_DEVICE/Target -I"F:/GIT/neutron/neutr_cube/Core/Inc" -I"F:/GIT/neutron/neutr_cube/Drivers/STM32F0xx_HAL_Driver/Inc" -I"F:/GIT/neutron/neutr_cube/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy" -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/AUDIO/Inc -I"F:/GIT/neutron/neutr_cube/Drivers/CMSIS/Device/ST/STM32F0xx/Include" -I"F:/GIT/neutron/neutr_cube/Drivers/CMSIS/Include" -I"F:/GIT/neutron/neutr_cube/neutr_src"  -O3 -pedantic -Wall -Wextra -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


