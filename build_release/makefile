################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: mbedLPC1768.elf

# Tool invocations
mbedLPC1768.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	arm-none-eabi-gcc -L"/Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/armv7-m/." -L"/Applications/StratifyLabs-SDK/Tools/gcc/lib/gcc/arm-none-eabi/5.4.1/armv7-m/." -Wl,-Map,fw.map,--gc,--defsym=mcu_core_hardware_id=0x00000003 -Ttext=0x40000 -Tldscripts/lpc1759-rom.ld -nostdlib -u symbols_table -o "mbedLPC1768.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) mbedLPC1768.elf
	-@echo ' '

post-build:
	-sh /Applications/StratifyLabs-SDK/Tools/gcc/arm-none-eabi/lib/ldscripts/kernel-post-process.sh /Applications/StratifyLabs-SDK/Tools/gcc arm-none-eabi mbedLPC1768 mbedLPC1768 armv7-m .
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
