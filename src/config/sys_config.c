// Copyright 2021 Stratify Labs, See LICENSE.md for details

#include <jansson/jansson_api.h>
#include <mcu/pio.h>
#include <sgfx/sgfx.h>

extern int mcu_core_initclock(int div);
extern void mcu_core_getserialno(mcu_sn_t *serial_number);

#include "sys_config.h"

void sys_initialize() {
  // initialize the board
  // this is one of the first things called
  // it should initialize the CLOCK/PLL
  // run in ROOT mode before the scheduler is started
  mcu_core_initclock(1);
}

void sys_get_serial_number(mcu_sn_t *serial_number) {
  // populate serial_number
  mcu_core_getserialno(serial_number);
}

int sys_kernel_request(int request, void *arg) {
  MCU_UNUSED_ARGUMENT(request);
  MCU_UNUSED_ARGUMENT(arg);
  // these are custom requests to allow applications
  // to execute kernel functions
  return -1;
}

const void *sys_kernel_request_api(u32 request) {
  MCU_UNUSED_ARGUMENT(request);
  return NULL;
}

void sys_pio_set_attributes(int port, const pio_attr_t *attr) {
  // convenience function to configuring IO
  // always called in ROOT mode
  const devfs_handle_t handle = {port, 0, 0};
  mcu_pio_setattr(&handle, (void *)attr);
}

void sys_pio_write(int port, u32 mask, int value) {
  // convenience function to writing IO
  // always called in ROOT mode
  const devfs_handle_t handle = {port, 0, 0};
  if (value) {
    mcu_pio_setmask(&handle, (void *)mask);
  } else {
    mcu_pio_clrmask(&handle, (void *)mask);
  }
}

u32 sys_pio_read(int port, u32 mask) {
  // convenience function to reading IO
  // always called in ROOT mode
  const devfs_handle_t handle = {port, 0, 0};
  u32 result = 0;
  mcu_pio_get(&handle, &result);
  return result & mask;
}
