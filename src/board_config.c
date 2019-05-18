/*

Copyright 2011-2016 Tyler Gilbert

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	 http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <sys/lock.h>
#include <fcntl.h>
#include <errno.h>
#include <mcu/mcu.h>
#include <mcu/periph.h>
#include <device/microchip/sst25vf.h>
#include <device/sys.h>
#include <device/uartfifo.h>
#include <device/usbfifo.h>
#include <device/fifo.h>
#include <device/led_pwm.h>
#include <sos/link.h>
#include <sos/fs/sysfs.h>
#include <sos/fs/appfs.h>
#include <sos/fs/devfs.h>
#include <sos/sos.h>
#include <sapi/sys/requests.h>
#include <sapi/sg.h>

#include "localfs.h"
#include "board_trace.h"
#include "link_transport_usb.h"
#include "link_transport_uart.h"
#include "sl_config.h"

#define SOS_BOARD_SYSTEM_CLOCK 96000000
#define SOS_BOARD_SYSTEM_OSC 12000000
#define SOS_BOARD_SYSTEM_MEMORY_SIZE (8192*2)
#define SOS_BOARD_TASK_TOTAL 6

static void board_event_handler(int event, void * args);


const mcu_board_config_t mcu_board_config = {
	.core_osc_freq = SOS_BOARD_SYSTEM_OSC,
	.core_cpu_freq = SOS_BOARD_SYSTEM_CLOCK,
	.core_periph_freq = SOS_BOARD_SYSTEM_CLOCK,
	.usb_max_packet_zero = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,
	.debug_uart_port = 0,
	.debug_uart_attr = {
		.pin_assignment =
		{
			.rx = {0, 2},
			.tx = {0, 3},
			.cts = {0xff, 0xff},
			.rts = {0xff, 0xff}
		},
		.freq = 115200,
		.o_flags = UART_FLAG_IS_PARITY_NONE | UART_FLAG_IS_STOP1,
		.width = 8
	},
	.o_flags = MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH,
	.event_handler = board_event_handler,
	.led.port = 1, .led.pin = 18,
	.o_mcu_debug = MCU_DEBUG_INFO | MCU_DEBUG_SYS | MCU_DEBUG_USB | MCU_DEBUG_DEVICE
};

void board_event_handler(int event, void * args){
	core_attr_t attr;
	switch(event){
		case MCU_BOARD_CONFIG_EVENT_ROOT_FATAL:
			//start the bootloader on a fatal event
			attr.o_flags = CORE_FLAG_EXEC_INVOKE_BOOTLOADER;
			mcu_core_setattr(0, &attr);
			break;
		case MCU_BOARD_CONFIG_EVENT_START_LINK:
			sos_led_startup();
			break;
	}
}

const sos_board_config_t sos_board_config = {
	.clk_usecond_tmr = 3,
	.task_total = SOS_BOARD_TASK_TOTAL,
	.stdin_dev = "/dev/stdio-in" ,
	.stdout_dev = "/dev/stdio-out",
	.stderr_dev = "/dev/stdio-out",
	.o_sys_flags = SYS_FLAG_IS_STDIO_FIFO | SYS_FLAG_IS_TRACE,
	.sys_name = SL_CONFIG_NAME,
	.sys_version = SL_CONFIG_VERSION_STRING,
	.sys_id = SL_CONFIG_DOCUMENT_ID,
	.sys_memory_size = SOS_BOARD_SYSTEM_MEMORY_SIZE,
	.start = sos_default_thread,
	#if defined __UART
	.start_args = &link_transport_uart,
	#else
	.start_args = &link_transport_usb,
	#endif
	.start_stack_size = SOS_DEFAULT_START_STACK_SIZE,
	.request = 0,
	.trace_dev = "/dev/trace",
	.trace_event = board_trace_event,
	.git_hash = SOS_GIT_HASH
};


SOS_DECLARE_TASK_TABLE(SOS_BOARD_TASK_TOTAL);

#define SOS_USER_ROOT 0

#define UART0_DEVFIFO_BUFFER_SIZE 128
char uart0_fifo_buffer[UART0_DEVFIFO_BUFFER_SIZE];

const uartfifo_config_t uart0_fifo_cfg = {
	.fifo = { .size = UART0_DEVFIFO_BUFFER_SIZE, .buffer = uart0_fifo_buffer },
	.uart.attr = {
		.pin_assignment =
		{
			.rx = {0, 2},
			.tx = {0, 3},
			.cts = {0xff, 0xff},
			.rts = {0xff, 0xff}
		},
#if defined __UART
		.freq = 230400,
#else
		.freq = 115200,
#endif
		.o_flags = UART_FLAG_IS_PARITY_NONE | UART_FLAG_IS_STOP1,
		.width = 8
	}
};
uartfifo_state_t uart0_fifo_state MCU_SYS_MEM;

#define UART1_DEVFIFO_BUFFER_SIZE 64
char uart1_fifo_buffer[UART1_DEVFIFO_BUFFER_SIZE];
const uartfifo_config_t uart1_fifo_cfg = {
	.fifo = { .size = UART1_DEVFIFO_BUFFER_SIZE, .buffer = uart1_fifo_buffer }
};
uartfifo_state_t uart1_fifo_state MCU_SYS_MEM;

#define UART3_DEVFIFO_BUFFER_SIZE 64
char uart3_fifo_buffer[UART3_DEVFIFO_BUFFER_SIZE];
const uartfifo_config_t uart3_fifo_cfg = {
	.fifo = { .size = UART3_DEVFIFO_BUFFER_SIZE, .buffer = uart3_fifo_buffer }
};
uartfifo_state_t uart3_fifo_state MCU_SYS_MEM;


#define STDIO_BUFFER_SIZE 128

char stdio_out_buffer[STDIO_BUFFER_SIZE];
char stdio_in_buffer[STDIO_BUFFER_SIZE];

fifo_config_t stdio_in_cfg = { .buffer = stdio_in_buffer, .size = STDIO_BUFFER_SIZE };
fifo_config_t stdio_out_cfg = { .buffer = stdio_out_buffer, .size = STDIO_BUFFER_SIZE };
fifo_state_t stdio_out_state;
fifo_state_t stdio_in_state;

const led_pwm_config_t led_pwm0_config = {
	.pwm.attr = {
		.o_flags = PWM_FLAG_IS_ACTIVE_HIGH,
		.freq = 1000000,
		.period = 1000000,
		.pin_assignment.channel[0] = {1,18},
		.pin_assignment.channel[1] = {0xff,0xff},
		.pin_assignment.channel[2] = {0xff,0xff},
		.pin_assignment.channel[3] = {0xff,0xff}
	},
	.loc = 1,
	.o_flags = LED_PWM_CONFIG_FLAG_IS_ACTIVE_HIGH
};

const led_pwm_config_t led_pwm1_config = {
	.pwm.attr = {
		.o_flags = PWM_FLAG_IS_ACTIVE_HIGH,
		.freq = 1000000,
		.period = 1000000,
		.pin_assignment.channel[0] = {1,20},
		.pin_assignment.channel[1] = {0xff,0xff},
		.pin_assignment.channel[2] = {0xff,0xff},
		.pin_assignment.channel[3] = {0xff,0xff}
	},
	.loc = 2,
	.o_flags = LED_PWM_CONFIG_FLAG_IS_ACTIVE_HIGH
};

const led_pwm_config_t led_pwm2_config = {
	.pwm.attr = {
		.o_flags = PWM_FLAG_IS_ACTIVE_HIGH,
		.freq = 1000000,
		.period = 1000000,
		.pin_assignment.channel[0] = {1,21},
		.pin_assignment.channel[1] = {0xff,0xff},
		.pin_assignment.channel[2] = {0xff,0xff},
		.pin_assignment.channel[3] = {0xff,0xff}
	},
	.loc = 3,
	.o_flags = LED_PWM_CONFIG_FLAG_IS_ACTIVE_HIGH
};

const led_pwm_config_t led_pwm3_config = {
	.pwm.attr = {
		.o_flags = PWM_FLAG_IS_ACTIVE_HIGH,
		.freq = 1000000,
		.period = 1000000,
		.pin_assignment.channel[0] = {1,23},
		.pin_assignment.channel[1] = {0xff,0xff},
		.pin_assignment.channel[2] = {0xff,0xff},
		.pin_assignment.channel[3] = {0xff,0xff}
	},
	.loc = 4,
	.o_flags = LED_PWM_CONFIG_FLAG_IS_ACTIVE_HIGH
};

SPI_DECLARE_CONFIG(spi0,
						 SPI_FLAG_SET_MASTER |
						 SPI_FLAG_IS_FORMAT_SPI |
						 SPI_FLAG_IS_MODE0,
						 1000000UL,
						 8,
						 0, 17, //P12 - miso
						 0, 18, //P11 - mosi
						 0, 15, //P13 - sck
						 0xff, 0xff //cs not used
						 );

SPI_DECLARE_CONFIG(spi1,
						 SPI_FLAG_SET_MASTER |
						 SPI_FLAG_IS_FORMAT_SPI |
						 SPI_FLAG_IS_MODE0,
						 1000000UL,
						 8,
						 0, 8, //P6
						 0, 9, //P5
						 0, 7, //P7
						 0xff, 0xff //cs not used
						 );


/* This is the list of devices that will show up in the /dev folder.
 */
const devfs_device_t devfs_list[] = {
	//mcu peripherals
	DEVFS_DEVICE("trace", ffifo, 0, &board_trace_config, &board_trace_state, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("core", mcu_core, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("core0", mcu_core, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("adc0", mcu_adc, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("dac0", mcu_dac, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("eint0", mcu_eint, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("eint1", mcu_eint, 1, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("eint2", mcu_eint, 2, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("eint3", mcu_eint, 3, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("pio0", mcu_pio, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("pio1", mcu_pio, 1, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("pio2", mcu_pio, 2, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("pio3", mcu_pio, 3, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("pio4", mcu_pio, 4, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("i2c0", mcu_i2c, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("i2c1", mcu_i2c, 1, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("i2c2", mcu_i2c, 2, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("pwm1", mcu_pwm, 1, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("qei0", mcu_qei, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("rtc", mcu_rtc, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("spi0", mcu_ssp, 0, &spi0_config, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("spi1", mcu_ssp, 1, &spi1_config, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("tmr0", mcu_tmr, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("tmr1", mcu_tmr, 1, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("tmr2", mcu_tmr, 2, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("uart0", uartfifo, 0, &uart0_fifo_cfg, &uart0_fifo_state, 0666, SOS_USER_ROOT, S_IFCHR),
	//DEVFS_DEVICE("uart0", mcu_uart, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	//DEVFS_DEVICE("uart1", mcu_uart, 1, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("uart2", mcu_uart, 2, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("uart3", mcu_uart, 3, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	//UARTFIFO_DEVICE("uart3", &uart3_fifo_cfg, &uart3_fifo_state, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("usb0", mcu_usb, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),

	DEVFS_DEVICE("led0", led_pwm, 1, &led_pwm0_config, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("led1", led_pwm, 1, &led_pwm1_config, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("led2", led_pwm, 1, &led_pwm2_config, 0, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("led3", led_pwm, 1, &led_pwm3_config, 0, 0666, SOS_USER_ROOT, S_IFCHR),

	//FIFO buffers used for std in and std out
	DEVFS_DEVICE("stdio-out", fifo, 0, &stdio_out_cfg, &stdio_out_state, 0666, SOS_USER_ROOT, S_IFCHR),
	DEVFS_DEVICE("stdio-in", fifo, 0, &stdio_in_cfg, &stdio_in_state, 0666, SOS_USER_ROOT, S_IFCHR),

	//system devices
	DEVFS_DEVICE("link-phy-usb", usbfifo, 0, &sos_link_transport_usb_fifo_cfg, &sos_link_transport_usb_fifo_state, 0666, SOS_USER_ROOT, S_IFCHR),

	DEVFS_DEVICE("sys", sys, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFCHR),

	DEVFS_TERMINATOR
};

const devfs_device_t mem0 = DEVFS_DEVICE("mem0", mcu_mem, 0, 0, 0, 0666, SOS_USER_ROOT, S_IFBLK);


const sysfs_t sysfs_list[] = {
	APPFS_MOUNT("/app", &mem0, SYSFS_ALL_ACCESS), //the folder for ram/flash applications
	DEVFS_MOUNT("/dev", devfs_list, SYSFS_READONLY_ACCESS), //the list of devices
	LOCALFS_MOUNT("/home", 0, SYSFS_ALL_ACCESS), //the list of devices
	SYSFS_MOUNT("/", sysfs_list, SYSFS_READONLY_ACCESS), //the root filesystem (must be last)
	SYSFS_TERMINATOR
};





