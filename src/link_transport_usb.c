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

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <mcu/debug.h>
#include <sos/dev/pio.h>
#include "link_transport_usb.h"


static link_transport_phy_t link_transport_open(const char * name, int baudrate);

link_transport_driver_t link_transport_usb = {
	.handle = -1,
	.open = link_transport_open,
	.read = sos_link_transport_usb_read,
	.write = sos_link_transport_usb_write,
	.close = sos_link_transport_usb_close,
	.wait = sos_link_transport_usb_wait,
	.flush = sos_link_transport_usb_flush,
	.timeout = 500
};

static usbd_control_t m_usb_control;

link_transport_phy_t link_transport_open(const char * name, int baudrate){
	usb_attr_t usb_attr;
	link_transport_phy_t fd;

	//set up the USB attributes
	memset(&(usb_attr.pin_assignment), 0xff, sizeof(usb_pin_assignment_t));
	usb_attr.o_flags = USB_FLAG_SET_DEVICE;
	usb_attr.pin_assignment.dp.port = 0;
	usb_attr.pin_assignment.dp.pin = 29;
	usb_attr.pin_assignment.dm.port = 0;
	usb_attr.pin_assignment.dm.pin = 30;
	usb_attr.freq = mcu_board_config.core_osc_freq;

	fd = sos_link_transport_usb_open(name,
												&m_usb_control,
												&sos_link_transport_usb_constants,
												&usb_attr,
												mcu_pin(2,9),
												0); //USB pin is active low

	return fd;
}
