################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Library/Src/Network.c \
../Library/Src/SIM800C.c 

OBJS += \
./Library/Src/Network.o \
./Library/Src/SIM800C.o 

C_DEPS += \
./Library/Src/Network.d \
./Library/Src/SIM800C.d 


# Each subdirectory must supply rules for building sources it contributes
Library/Src/%.o Library/Src/%.su Library/Src/%.cyclo: ../Library/Src/%.c Library/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"../Library/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Library-2f-Src

clean-Library-2f-Src:
	-$(RM) ./Library/Src/Network.cyclo ./Library/Src/Network.d ./Library/Src/Network.o ./Library/Src/Network.su ./Library/Src/SIM800C.cyclo ./Library/Src/SIM800C.d ./Library/Src/SIM800C.o ./Library/Src/SIM800C.su

.PHONY: clean-Library-2f-Src

