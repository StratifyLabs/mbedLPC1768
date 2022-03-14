
#ifndef MBED_LPC1768_CONFIG_H_
#define MBED_LPC1768_CONFIG_H_

#include <sos/arch.h>
#include <sos/debug.h>

#include "config/os_config.h"
#include "sl_config.h"

#define CONFIG_SYSTEM_MEMORY_SIZE (16 * 1024)
#define CONFIG_STDIO_BUFFER_SIZE 64
#define CONFIG_SYSTEM_CLOCK 96000000

#define CONFIG_USB_PULLUP_PORT 2
#define CONFIG_USB_PULLUP_PIN 9
#define CONFIG_USB_PULLUP_ACTIVE_HIGH 0
#define CONFIG_USB_DP_PORT 0
#define CONFIG_USB_DP_PIN 29
#define CONFIG_USB_DM_PORT 0
#define CONFIG_USB_DM_PIN 30

#define CONFIG_DEBUG_LED_PORT 1
#define CONFIG_DEBUG_LED_PIN 18

#define CONFIG_BOOT_REQUEST_PORT 0
#define CONFIG_BOOT_REQUEST_PIN 16
#define CONFIG_BOOT_REQUEST_ACTIVE_HIGH 1


#define usb_state_t u32


#endif /* MBED_LPC1768_CONFIG_H_ */
