/*
 * link_transport.c
 *
 *  Created on: May 23, 2016
 *      Author: tgil
 */

#include <fcntl.h>
#include <unistd.h>
#include <iface/dev/pio.h>

#include "link_transport.h"

static link_transport_phy_t link_transport_open(const char * name, int baudrate);

link_transport_driver_t link_transport = {
		.handle = -1,
		.open = link_transport_open,
		.read = boot_stratify_link_transport_usb_read,
		.write = boot_stratify_link_transport_usb_write,
		.close = boot_stratify_link_transport_usb_close,
		.wait = boot_stratify_link_transport_usb_wait,
		.flush = boot_stratify_link_transport_usb_flush,
		.timeout = 500
};

#define USBDEV_CONNECT_PORT 2
#define USBDEV_CONNECT_PINMASK (1<<9)

link_transport_phy_t link_transport_open(const char * name, int baudrate){
	pio_attr_t attr;
	link_transport_phy_t fd;
	//Deassert the Connect line and enable the output
	mcu_pio_setmask(USBDEV_CONNECT_PORT, (void*)(USBDEV_CONNECT_PINMASK));

	attr.mask = (USBDEV_CONNECT_PINMASK);
	attr.mode = PIO_MODE_OUTPUT | PIO_MODE_DIRONLY;
	mcu_pio_setattr(USBDEV_CONNECT_PORT, &attr);

	fd = boot_stratify_link_transport_usb_open(name, baudrate);

	mcu_pio_clrmask(USBDEV_CONNECT_PORT, (void*)(USBDEV_CONNECT_PINMASK));

	return fd;
}
