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

#include "link_transport_uart.h"

#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <iface/link.h>
#include <mcu/mcu.h>
#include <iface/dev/uart.h>
#include <iface/dev/fifo.h>
#include <mcu/core.h>
#include <mcu/debug.h>
#include <stratify/usb_dev_typedefs.h>
#include <stratify/usb_dev_defs.h>
#include <device/sys.h>

#include "link_transport_uart.h"


link_transport_driver_t link_transport_uart = {
		.handle = -1,
		.open = link_transport_uart_open,
		.read = link_transport_uart_read,
		.write = link_transport_uart_write,
		.close = link_transport_uart_close,
		.wait = link_transport_uart_wait,
		.flush = link_transport_uart_flush,
		.timeout = 500
};



link_transport_phy_t link_transport_uart_open(const char * name, int baudrate){
	link_transport_phy_t fd;
	uart_attr_t attr;

	fd = open("/dev/uart0", O_RDWR);
	if( fd <  0){
		return -1;
	}

	attr.baudrate = 460800;
	attr.parity = UART_PARITY_EVEN;
	attr.pin_assign = 0;
	attr.start = UART_ATTR_START_BITS_1;
	attr.stop = UART_ATTR_STOP_BITS_2;
	attr.width = 8;
	if( ioctl(fd, I_UART_SETATTR, &attr) < 0 ){
		return -1;
	}

	return fd;
}

int link_transport_uart_write(link_transport_phy_t handle, const void * buf, int nbyte){
	int ret;
	ret = write(handle, buf, nbyte);
	return ret;
}

int link_transport_uart_read(link_transport_phy_t handle, void * buf, int nbyte){
	int ret;
	errno = 0;
	ret = read(handle, buf, nbyte);
	return ret;
}

int link_transport_uart_close(link_transport_phy_t handle){
	return close(handle);
}

void link_transport_uart_wait(int msec){
	int i;
	for(i = 0; i < msec; i++){
		usleep(1000);
	}
}

void link_transport_uart_flush(link_transport_phy_t handle){
	ioctl(handle, I_FIFO_FLUSH);
}