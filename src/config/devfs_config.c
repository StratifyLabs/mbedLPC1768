#include <device/ffifo.h>
#include <mcu/periph.h>
#include <sos/fs/devfs.h>
#include <device/sys.h>

#include "config.h"
#include "devfs_config.h"
#include "link_config.h"


#define STDIO_BUFFER_SIZE CONFIG_STDIO_BUFFER_SIZE
FIFO_DECLARE_CONFIG_STATE(stdio_in, STDIO_BUFFER_SIZE);
FIFO_DECLARE_CONFIG_STATE(stdio_out, STDIO_BUFFER_SIZE);

fifo_config_t stdio_in_cfg = {.buffer = stdio_in_buffer,
                              .size = STDIO_BUFFER_SIZE};
fifo_config_t stdio_out_cfg = {.buffer = stdio_out_buffer,
                               .size = STDIO_BUFFER_SIZE};
fifo_state_t stdio_out_state;
fifo_state_t stdio_in_state;

const adc_config_t adc0_config = {
    .attr = {.o_flags = ADC_FLAG_SET_CONVERTER | ADC_FLAG_IS_RIGHT_JUSTIFIED,
             .freq = 0, // use the max frequency
             .pin_assignment = {
                 .channel[0] = {0, 23},
                 .channel[1] = {0, 24},
                 .channel[2] = {0xff, 0xff},
                 .channel[3] = {0xff, 0xff},
             }}};

I2C_DECLARE_CONFIG_MASTER(i2c2, I2C_FLAG_SET_MASTER | I2C_FLAG_IS_PULLUP,
                          100000, 0, 10, // sda
                          0, 11);        // scl

SPI_DECLARE_CONFIG(spi0,
                   SPI_FLAG_SET_MASTER | SPI_FLAG_IS_MODE0 |
                       SPI_FLAG_IS_FORMAT_SPI,
                   1000000, 8, 1, 23, // miso
                   0xff, 0xff,        // mosi
                   1, 20,             // sck,
                   0xff, 0xff);       // cs

PWM_DECLARE_CONFIG(pwm1,
                   PWM_FLAG_SET_TIMER | PWM_FLAG_SET_CHANNELS |
                       PWM_FLAG_IS_ENABLED,
                   1000000, 10000, // freq and period
                   2, 0,           // channel 0
                   2, 1,           // channel 1
                   0xff, 0xff, 0xff, 0xff);

#if 0
const pwm_config_t pwm1_config = {
    .attr = {
        .o_flags = PWM_FLAG_SET_TIMER | PWM_FLAG_SET_CHANNELS | PWM_FLAG_IS_ENABLED,
        .pin_assignment = {
            .channel[0] = {2, 0},
            .channel[1] = {2, 1},
            .channel[2] = {0xff, 0xff},
            .channel[3] = {0xff, 0xff}
        },
        .freq = 1000000,
        .period = 10000,
        .channel = { 0, 0 }
    }
};
#endif

// list of devices for the /dev folder
const devfs_device_t devfs_list[] = {
    // mcu peripherals
#if NOT_BUILDING
    DEVFS_DEVICE("trace", ffifo, 0, &board_trace_config, &board_trace_state,
                 0666, SYSFS_ROOT, S_IFCHR),
#endif
    DEVFS_DEVICE("core", mcu_core, 0, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("core0", mcu_core, 0, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("adc0", mcu_adc, 0, &adc0_config, 0, 0666, SYSFS_ROOT,
                 S_IFCHR),
    DEVFS_DEVICE("dac0", mcu_dac, 0, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("eint1", mcu_eint, 1, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("eint2", mcu_eint, 2, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("eint3", mcu_eint, 3, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("pio0", mcu_pio, 0, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("pio1", mcu_pio, 1, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("pio2", mcu_pio, 2, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("pio3", mcu_pio, 3, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("pio4", mcu_pio, 4, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("i2c2", mcu_i2c, 2, &i2c2_config, 0, 0666, SYSFS_ROOT,
                 S_IFCHR),
    DEVFS_DEVICE("pwm1", mcu_pwm, 1, &pwm1_config, 0, 0666, SYSFS_ROOT,
                 S_IFCHR),
    DEVFS_DEVICE("rtc", mcu_rtc, 0, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("spi0", mcu_ssp, 0, &spi0_config, 0, 0666, SYSFS_ROOT,
                 S_IFCHR),
    DEVFS_DEVICE("tmr0", mcu_tmr, 0, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("tmr1", mcu_tmr, 1, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("tmr2", mcu_tmr, 2, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),
#if NOT_BUILDING
    DEVFS_DEVICE("uart1", uartfifo, 1, &uart1_fifo_config, &uart1_fifo_state,
                 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("uart3", uartfifo, 3, &uart3_fifo_config, &uart3_fifo_state,
                 0666, SYSFS_ROOT, S_IFCHR),
#endif
    DEVFS_DEVICE("usb0", mcu_usb, 0, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),

    // FIFO buffers used for std in and std out
    DEVFS_DEVICE("stdio-out", fifo, 0, &stdio_out_config, &stdio_out_state,
                 0666, SYSFS_ROOT, S_IFCHR),
    DEVFS_DEVICE("stdio-in", fifo, 0, &stdio_in_config, &stdio_in_state, 0666,
                 SYSFS_ROOT, S_IFCHR),

    // system devices
    DEVFS_CHAR_DEVICE("link-phy-usb", device_fifo, &usb_device_fifo_config,
                      &usb_device_fifo_state, 0666, SYSFS_ROOT),

    DEVFS_DEVICE("sys", sys, 0, 0, 0, 0666, SYSFS_ROOT, S_IFCHR),

    DEVFS_TERMINATOR};
