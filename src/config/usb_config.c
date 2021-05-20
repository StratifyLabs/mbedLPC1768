//Copyright 2021 Stratify Labs, See LICENSE.md for details

#include <mcu/usb.h>

#include "usb_config.h"

int usb_set_attributes(const devfs_handle_t *handle, void *ctl) {
  //allows the USB stack to set USB attributes
  mcu_usb_setattr(handle, ctl);
  return 0;
}

int usb_set_action(const devfs_handle_t *handle, mcu_action_t *action) {
  //ROOT function to install an action (callback) to the USB driver
  mcu_usb_setaction(handle, action);
  return 0;
}

void usb_write_endpoint(const devfs_handle_t *handle, u32 endpoint_num,
                        const void *src, u32 size) {
    //ROOT function to write a USB endpoint
    mcu_usb_root_write_endpoint(handle, endpoint_num, src, size);

}

int usb_read_endpoint(const devfs_handle_t *handle, u32 endpoint_num,
                      void *dest) {
  return mcu_usb_root_read_endpoint(handle, endpoint_num, dest);
}
