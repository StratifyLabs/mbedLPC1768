// Copyright 2021 Stratify Labs, See LICENSE.md for details

#include <sdk/types.h>
#include <sos/arch.h>
#include <sos/config.h>
#include <sos/fs/devfs.h>

#include <mcu/flash.h>

#include "boot_link_config.h"

const struct __sFILE_fake __sf_fake_stdin;
const struct __sFILE_fake __sf_fake_stdout;
const struct __sFILE_fake __sf_fake_stderr;

#include "../src/config.h"

void boot_event_handler(int event, void *args) {
  MCU_UNUSED_ARGUMENT(event);
  MCU_UNUSED_ARGUMENT(args);
}

int boot_is_bootloader_requested() {

  //check for SW boot request

  // pin 0,16
  // check if boot request pin is active
  const pio_attr_t attr = {.o_flags = PIO_FLAG_SET_INPUT | PIO_FLAG_IS_PULLDOWN,
                           .o_pinmask = (1 << CONFIG_BOOT_REQUEST_PIN)};
  sos_config.sys.pio_set_attributes(CONFIG_BOOT_REQUEST_PORT, &attr);
  const u32 result =
      sos_config.sys.pio_read(CONFIG_BOOT_REQUEST_PORT, attr.o_pinmask);
  const int bool_result = result == attr.o_pinmask;
  return bool_result == CONFIG_BOOT_REQUEST_ACTIVE_HIGH;
}

int boot_flash_erase_page(const devfs_handle_t *handle, void *ctl) {
  return mcu_flash_erasepage(handle, ctl);
}

int boot_flash_write_page(const devfs_handle_t *handle, void *ctl) {
  return mcu_flash_writepage(handle, ctl);
}
