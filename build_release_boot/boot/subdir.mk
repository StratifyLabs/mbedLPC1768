################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../boot/boot_board_config.c \
../boot/link_transport.c 

OBJS += \
./boot/boot_board_config.o \
./boot/link_transport.o 

C_DEPS += \
./boot/boot_board_config.d \
./boot/link_transport.d 


# Each subdirectory must supply rules for building sources it contributes
boot/%.o: ../boot/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -DHARDWARE_ID=0x00000003 -D__lpc17xx -D__ -Os -Wall -c -fmessage-length=0 -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


