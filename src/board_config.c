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

#include <stdint.h>
#include <sys/lock.h>
#include <fcntl.h>
#include <errno.h>
#include <mcu/mcu.h>
#include <mcu/periph.h>
#include <mcu/microchip/sst25vf.h>
#include <mcu/sys.h>
#include <mcu/uartfifo.h>
#include <mcu/usbfifo.h>
#include <mcu/fifo.h>
#include <mcu/core.h>
#include <mcu/sys.h>
#include <sos/link.h>
#include <sos/fs/sysfs.h>
#include <sos/fs/appfs.h>
#include <sos/fs/devfs.h>
#include <sos/stratify.h>

#include "localfs.h"
#include "link_transport_usb.h"
#include "link_transport_uart.h"

#define STFY_SYSTEM_CLOCK 120000000
#define STFY_SYSTEM_OSC 12000000
#define STFY_SYSTEM_MEMORY_SIZE (8192*2)

static void board_event_handler(int event, void * args);

const mcu_board_config_t mcu_board_config = {
		.core_osc_freq = STFY_SYSTEM_OSC,
		.core_cpu_freq = STFY_SYSTEM_CLOCK,
		.core_periph_freq = STFY_SYSTEM_CLOCK,
		.usb_max_packet_zero = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,
		.debug_uart_pin_assignment[0] = {0, 2},
		.debug_uart_pin_assignment[1] = {0, 3},
		.usb_pin_assignment[0] = {0, 29},
		.usb_pin_assignment[1] = {0, 30},
		.usb_pin_assignment[2] = {0xff, 0xff},
		.usb_pin_assignment[3] = {0xff, 0xff},
		.o_flags = MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH,
		.event = board_event_handler,
		.led.port = 1, .led.pin = 18
};

void board_event_handler(int event, void * args){
	core_attr_t attr;
	switch(event){
	case MCU_BOARD_CONFIG_EVENT_PRIV_FATAL:
		//start the bootloader on a fatal event
		attr.o_flags = CORE_FLAG_EXEC_INVOKE_BOOTLOADER;
		mcu_core_setattr(0, &attr);
		break;
	case MCU_BOARD_CONFIG_EVENT_START_LINK:
		stratify_led_startup();
		break;
	}
}


#define SCHED_TASK_TOTAL 10

const stratify_board_config_t stratify_board_config = {
		.clk_usecond_tmr = 3,
		.task_total = SCHED_TASK_TOTAL,
		.clk_usec_mult = (uint32_t)(STFY_SYSTEM_CLOCK / 1000000),
		.clk_nsec_div = (uint32_t)((uint64_t)1024 * 1000000000 / STFY_SYSTEM_CLOCK),
		.stdin_dev = "/dev/stdio-in" ,
		.stdout_dev = "/dev/stdio-out",
		.stderr_dev = "/dev/stdio-out",
		.o_sys_flags = SYS_FLAGS_STDIO_FIFO | SYS_FLAGS_NOTIFY,
		.sys_name = "mbedLPC1768",
		.sys_version = "1.2",
		.sys_id = "-KZTKpwml73OFt90YdD8",
		.sys_memory_size = STFY_SYSTEM_MEMORY_SIZE,
		.start = stratify_default_thread,
#if defined __UART
		.start_args = &link_transport_uart,
#else
		.start_args = &link_transport_usb,
#endif
		.start_stack_size = STRATIFY_DEFAULT_START_STACK_SIZE,
		.notify_write = stratify_link_transport_usb_notify
};

volatile sched_task_t stratify_sched_table[SCHED_TASK_TOTAL] MCU_SYS_MEM;
task_t stratify_task_table[SCHED_TASK_TOTAL] MCU_SYS_MEM;


#define USER_ROOT 0
#define GROUP_ROOT 0


#define UART0_DEVFIFO_BUFFER_SIZE 128
char uart0_fifo_buffer[UART0_DEVFIFO_BUFFER_SIZE];
const uartfifo_cfg_t uart0_fifo_cfg = UARTFIFO_DEVICE_CFG(0,
		uart0_fifo_buffer,
		UART0_DEVFIFO_BUFFER_SIZE);
uartfifo_state_t uart0_fifo_state MCU_SYS_MEM;

#define UART1_DEVFIFO_BUFFER_SIZE 64
char uart1_fifo_buffer[UART1_DEVFIFO_BUFFER_SIZE];
const uartfifo_cfg_t uart1_fifo_cfg = UARTFIFO_DEVICE_CFG(1,
		uart1_fifo_buffer,
		UART1_DEVFIFO_BUFFER_SIZE);
uartfifo_state_t uart1_fifo_state MCU_SYS_MEM;

#define UART3_DEVFIFO_BUFFER_SIZE 64
char uart3_fifo_buffer[UART3_DEVFIFO_BUFFER_SIZE];
const uartfifo_cfg_t uart3_fifo_cfg = UARTFIFO_DEVICE_CFG(3,
		uart3_fifo_buffer,
		UART3_DEVFIFO_BUFFER_SIZE);
uartfifo_state_t uart3_fifo_state MCU_SYS_MEM;

#define STDIO_BUFFER_SIZE 128
char stdio_out_buffer[STDIO_BUFFER_SIZE];
char stdio_in_buffer[STDIO_BUFFER_SIZE];

void stdio_out_notify_write(int nbyte){
	link_notify_dev_t notify;
	notify.id = LINK_NOTIFY_ID_DEVICE_WRITE;
	strcpy(notify.name, "stdio-out");
	notify.nbyte = nbyte;
	if( stratify_board_config.notify_write ){
		stratify_board_config.notify_write(&notify, sizeof(link_notify_dev_t));
	}
}


fifo_cfg_t stdio_out_cfg = FIFO_DEVICE_CFG(stdio_out_buffer, STDIO_BUFFER_SIZE, 0, stdio_out_notify_write);
fifo_cfg_t stdio_in_cfg = FIFO_DEVICE_CFG(stdio_in_buffer, STDIO_BUFFER_SIZE, 0, 0);
fifo_state_t stdio_out_state = { .head = 0, .tail = 0, .rop = NULL, .rop_len = 0, .wop = NULL, .wop_len = 0 };
fifo_state_t stdio_in_state = {
		.head = 0, .tail = 0, .rop = NULL, .rop_len = 0, .wop = NULL, .wop_len = 0
};

#define MEM_DEV 0

/* This is the list of devices that will show up in the /dev folder
 * automatically.  By default, the peripheral devices for the MCU are available
 * plus the devices on the microcomputer.
 */
const devfs_device_t devices[] = {
		//mcu peripherals
		DEVFS_HANDLE("mem0", mcu_mem, 0, 0, 0666, USER_ROOT, S_IFBLK),
		DEVFS_HANDLE("core", mcu_core, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("core0", mcu_core, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("adc0", mcu_adc, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("dac0", mcu_dac, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("eint0", mcu_eint, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("eint1", mcu_eint, 1, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("eint2", mcu_eint, 2, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("eint3", mcu_eint, 3, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pio0", mcu_pio, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pio1", mcu_pio, 1, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pio2", mcu_pio, 2, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pio3", mcu_pio, 3, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pio4", mcu_pio, 4, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("i2c0", mcu_i2c, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("i2c1", mcu_i2c, 1, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("i2c2", mcu_i2c, 2, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pwm1", mcu_pwm, 1, 0, 0666, USER_ROOT, S_IFBLK),
		DEVFS_HANDLE("qei0", mcu_qei, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("rtc", mcu_rtc, 0, 0, 0666, USER_ROOT, S_IFCHR),
		//DEVFS_HANDLE("spi0", mcu_ssp, 0, 0, 0666, USER_ROOT, S_IFCHR),
		//DEVFS_HANDLE("spi1", mcu_ssp, 1, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("spi2", mcu_spi, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("tmr0", mcu_tmr, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("tmr1", mcu_tmr, 1, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("tmr2", mcu_tmr, 2, 0, 0666, USER_ROOT, S_IFCHR),
		//UARTFIFO_DEVICE("uart0", &uart0_fifo_cfg, &uart0_fifo_state, 0, 0666, USER_ROOT, GROUP_ROOT),
		//DEVFS_HANDLE("uart0", mcu_uart, 0, 0, 0666, USER_ROOT, S_IFCHR),
		//UARTFIFO_DEVICE("uart1", &uart1_fifo_cfg, &uart1_fifo_state, 0, 0666, USER_ROOT, GROUP_ROOT),
		//DEVFS_HANDLE("uart1", mcu_uart, 1, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("uart2", mcu_uart, 2, 0, 0666, USER_ROOT, S_IFCHR),
		//UARTFIFO_DEVICE("uart3", &uart3_fifo_cfg, &uart3_fifo_state, 0, 0666, USER_ROOT, GROUP_ROOT),
		DEVFS_HANDLE("usb0", mcu_usb, 0, 0, 0666, USER_ROOT, S_IFCHR),


		//FIFO buffers used for std in and std out
		//FIFO_DEVICE("stdio-out", &stdio_out_cfg, &stdio_out_state, 0666, USER_ROOT, GROUP_ROOT),
		//FIFO_DEVICE("stdio-in", &stdio_in_cfg, &stdio_in_state, 0666, USER_ROOT, GROUP_ROOT),

		//system devices
		//USBFIFO_DEVICE("link-phy-usb", &stratify_link_transport_usb_fifo_cfg, &stratify_link_transport_usb_fifo_state, 0666, USER_ROOT, GROUP_ROOT),

		//SYS_DEVICE,

		DEVFS_TERMINATOR
};

const sysfs_t const sysfs_list[] = {
		APPFS_MOUNT("/app", &(devices[MEM_DEV]), SYSFS_ALL_ACCESS), //the folder for ram/flash applications
		DEVFS_MOUNT("/dev", devices, SYSFS_READONLY_ACCESS), //the list of devices
		LOCALFS_MOUNT("/home", 0, SYSFS_ALL_ACCESS), //the list of devices
		SYSFS_MOUNT("/", sysfs_list, SYSFS_READONLY_ACCESS), //the root filesystem (must be last)
		SYSFS_TERMINATOR
};





