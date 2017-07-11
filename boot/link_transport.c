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



#include <fcntl.h>
#include <unistd.h>
#include <mcu/pio.h>
#include <mcu/boot_debug.h>

#include "link_transport.h"

static link_transport_phy_t link_transport_open(const char * name, int baudrate);

link_transport_driver_t link_transport = {
		.handle = -1,
		.open = link_transport_open,
		.read = stratify_link_boot_transport_usb_read,
		.write = stratify_link_boot_transport_usb_write,
		.close = stratify_link_boot_transport_usb_close,
		.wait = stratify_link_boot_transport_usb_wait,
		.flush = stratify_link_boot_transport_usb_flush,
		.timeout = 500
};

#define CONNECT_PORT 2
#define CONNECT_PINMASK (1<<9)

static usbd_control_t m_usb_control;

link_transport_phy_t link_transport_open(const char * name, int baudrate){
	pio_attr_t attr;
	link_transport_phy_t fd;
	//Deassert the Connect line and enable the output
	mcu_pio_setmask(CONNECT_PORT, (void*)(CONNECT_PINMASK));

	attr.o_pinmask = (CONNECT_PINMASK);
	attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
	mcu_pio_setattr(CONNECT_PORT, &attr);

	memset(&m_usb_control, 0, sizeof(m_usb_control));
	m_usb_control.constants = &stratify_link_transport_usb_constants;

	fd = stratify_link_boot_transport_usb_open(name, &m_usb_control);


	mcu_pio_clrmask(CONNECT_PORT, (void*)(CONNECT_PINMASK));

#if 0
	int tmp = 0;
	while(1){
		if( tmp != usbdev_stat ){
			tmp = usbdev_stat;
			usbdev_stat = 0;
			if( tmp ){
				dstr("S:"); dhex(tmp); dstr("\n");
			}
		}

	}
#endif
	return fd;
}
