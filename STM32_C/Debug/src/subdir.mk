################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Lcd.c \
../src/Ports.c \
../src/Timer.c \
../src/_initialize_hardware.c \
../src/_write.c \
../src/bsp.c \
../src/game.c \
../src/main.c 

OBJS += \
./src/Lcd.o \
./src/Ports.o \
./src/Timer.o \
./src/_initialize_hardware.o \
./src/_write.o \
./src/bsp.o \
./src/game.o \
./src/main.o 

C_DEPS += \
./src/Lcd.d \
./src/Ports.d \
./src/Timer.d \
./src/_initialize_hardware.d \
./src/_write.d \
./src/bsp.d \
./src/game.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=soft -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DTRACE -DOS_USE_TRACE_ITM -DSTM32F407xx -DUSE_HAL_DRIVER -DHSE_VALUE=8000000 -I"../includes" -I"../system/includes" -I"../system/includes/cmsis" -I"../system/includes/stm32f4-hal" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


