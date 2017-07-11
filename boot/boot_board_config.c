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



#include <mcu/types.h>
#include <mcu/core.h>
#include <sos/dev/bootloader.h>

#include "link_transport.h"
#include "board_config.h"

//Can't do CRP or the JTAG shuts down on the MBED
//const u32 _mcu_crp_value __attribute__ ((section(".crp_section"))) = 0x87654321;

#define STFY_SYSTEM_CLOCK 72000000
#define STFY_SYSTEM_OSC 12000000

const mcu_board_config_t mcu_board_config = {
		.core_osc_freq = STFY_SYSTEM_OSC,
		.core_cpu_freq = STFY_SYSTEM_CLOCK,
		.core_periph_freq = STFY_SYSTEM_CLOCK,
		.usb_max_packet_zero = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,
		.debug_uart_pin_assignment[0] = {0, 2},
		.debug_uart_pin_assignment[1] = {0, 3},
		.usb_pin_assignment[0] = {0, 29},
		.usb_pin_assignment[1] = {0, 30},
		.usb_pin_assignment[2] = {1, 30},
		.usb_pin_assignment[3] = {0xff, 0xff},
		.o_flags = MCU_BOARD_CONFIG_FLAG_LED_ACTIVE_HIGH,
		.led.port = 1, .led.pin = 18,
		.event = 0
};

const bootloader_board_config_t boot_board_config = {
		.sw_req_loc = 0x10002000,
		.sw_req_value = 0x55AA55AA,
		.program_start_addr = 0x40000,
		.hw_req.port = 0, .hw_req.pin = 17,
		.flags = 0,
		.link_transport_driver = &link_transport,
		.id = HARDWARE_ID
};


extern void boot_main();

void _main(){
	boot_main();
}
