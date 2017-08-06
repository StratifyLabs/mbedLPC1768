################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/board_config.c \
../src/board_trace.c \
../src/link_transport_uart.c \
../src/link_transport_usb.c \
../src/localfs.c \
../src/semihost_api.c \
../src/stratify_symbols.c 

OBJS += \
./src/board_config.o \
./src/board_trace.o \
./src/link_transport_uart.o \
./src/link_transport_usb.o \
./src/localfs.o \
./src/semihost_api.o \
./src/stratify_symbols.o 

C_DEPS += \
./src/board_config.d \
./src/board_trace.d \
./src/link_transport_uart.d \
./src/link_transport_usb.d \
./src/localfs.d \
./src/semihost_api.d \
./src/stratify_symbols.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-none-eabi-gcc -D__StratifyOS__ -D__lpc17xx -D___debug -Os -Wall -c -fmessage-length=0 -fomit-frame-pointer -march=armv7-m -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


