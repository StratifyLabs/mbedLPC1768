
//Copyright 2021 Stratify Labs, See LICENSE.md for details

#include <sos/arch.h>

#include <mcu/pio.h>

#include "mcu_config.h"
#include "config.h"


void mcu_reset_watchdog_timer() {}

void mcu_set_interrupt_priority(int number, int priority) {
  NVIC_SetPriority((IRQn_Type)number, priority);
}

int mcu_set_pin_function(const mcu_pin_t *pin, int function, int periph_port) {
  //configure the pin function
  //only needed if application needs to change pin function values
  mcu_core_set_pinsel_func(pin, function, periph_port);
  return 0;
}


const lpc_config_t lpc_config = {
    .usb = {.max_packet_zero = 64 },
    .clock_oscillator_freq = 12000000,
    .clock_peripheral_freq = CONFIG_SYSTEM_CLOCK
};

