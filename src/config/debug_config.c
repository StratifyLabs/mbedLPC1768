
// Copyright 2021 Stratify Labs, See LICENSE.md for details

#include <cortexm/cortexm.h>
#include <mcu/uart.h>
#include <sos/debug.h>

#include "debug_config.h"

#define DEBUG_LED_PORT 1
#define DEBUG_LED_PIN 18

static const uart_config_t m_uart_config = {
    .port = 0,
    .attr = {.pin_assignment = {.rx = {0, 2},
                                .tx = {0, 3},
                                .cts = {0xff, 0xff},
                                .rts = {0xff, 0xff}},
             .freq = 115200,
             .o_flags = UART_FLAG_SET_LINE_CODING_DEFAULT,
             .width = 8}};

static const devfs_handle_t m_uart_handle = {
    .port = 0,
    .config = &m_uart_config,
    .state = NULL,
};

void debug_initialize() {
  // initialize the debug LED
  // use blue LED for debug
  const pio_attr_t attr = {.o_flags = PIO_FLAG_SET_OUTPUT,
                           .o_pinmask = (1 << DEBUG_LED_PIN)};
  sos_config.sys.pio_set_attributes(DEBUG_LED_PORT, &attr);

  // initialize the serial debug output
  mcu_uart_open(&m_uart_handle);
  if( mcu_uart_setattr(&m_uart_handle, (void*)&m_uart_config.attr) < 0 ){
    while (1) {
      sos_config.debug.enable_led();
      cortexm_delay_ms(10);
      sos_config.debug.disable_led();
      cortexm_delay_ms(10);
    }
  }

}

void debug_write(const void *buf, int nbyte) {
  // write the serial debut output
  const u8 *u8_buf = buf;
  for (int i = 0; i < nbyte; i++) {
    mcu_uart_put(&m_uart_handle, (void *)((u32)(u8_buf[i])));
  }
}

void debug_enable_led() {
  // turn the debug LED on
  sos_config.sys.pio_write(DEBUG_LED_PORT, 1 << DEBUG_LED_PIN, 1);
}

void debug_disable_led() {
  // turn the debug LED off
  sos_config.sys.pio_write(DEBUG_LED_PORT, 1 << DEBUG_LED_PIN, 0);
}

void debug_trace_event(void *event) { MCU_UNUSED_ARGUMENT(event); }
