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
#ifndef LINK_TRANSPORT_USB_H_
#define LINK_TRANSPORT_USB_H_

//#define __STDIO_VCP

#include <iface/link_transport.h>

#define LINK_USBPHY_BULK_ENDPOINT (0x2)
#define LINK_USBPHY_BULK_ENDPOINT_ALT (0x5)
#define LINK_USBPHY_BULK_ENDPOINT_SIZE (64)

extern link_transport_driver_t link_transport_usb;

link_transport_phy_t link_transport_usb_open(const char * name, int);
int link_transport_usb_read(link_transport_phy_t, void * buf, int nbyte);
int link_transport_usb_write(link_transport_phy_t, const void * buf, int nbyte);
int link_transport_usb_close(link_transport_phy_t handle);
void link_transport_usb_wait(int msec);
void link_transport_usb_flush(link_transport_phy_t handle);

#endif /* LINK_TRANSPORT_USB_H_ */
