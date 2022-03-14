
//Copyright 2021 Stratify Labs, See LICENSE.md for details

#include <lpc_arch.h>

#include "clock_config.h"

const devfs_handle_t tmr_handle = {

};


void clock_initialize(
    int (*handle_match_channel0)(void *context, const mcu_event_t *data),
    int (*handle_match_channel1)(void *context, const mcu_event_t *data),
    int (*handle_overflow)(void *context, const mcu_event_t *data)) {
      //initialize the clock
      lpc_clock_initialize(handle_match_channel0, handle_match_channel1, handle_overflow);
}

void clock_enable() {
  //start the clock
  lpc_clock_enable();
}

u32 clock_disable() {
  return lpc_clock_disable();
}

void clock_set_channel(const mcu_channel_t *channel) {
  //set the value of the specified channel
  lpc_clock_set_channel(channel);
}

void clock_get_channel(mcu_channel_t *channel) {
  lpc_clock_get_channel(channel);
}

u32 clock_microseconds() { 
  //return the value of the clock in microseconds
  return lpc_clock_microseconds();
}

u32 clock_nanoseconds() { 
  //return the number of nanseconds from 0 to 1000 (as a sub unit of clock_microseconds())
  //this can just return zero if nanosecond precision is not needed
  return lpc_clock_nanoseconds();
}
