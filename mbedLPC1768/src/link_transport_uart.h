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
#ifndef LINK_TRANSPORT_UART_H_
#define LINK_TRANSPORT_UART_H_

#include <sos/link/transport.h>

extern link_transport_driver_t link_transport_uart;

link_transport_phy_t link_transport_uart_open(const char * name, const void * options);
int link_transport_uart_read(link_transport_phy_t, void * buf, int nbyte);
int link_transport_uart_write(link_transport_phy_t, const void * buf, int nbyte);
int link_transport_uart_close(link_transport_phy_t * handle);
void link_transport_uart_wait(int msec);
void link_transport_uart_flush(link_transport_phy_t handle);




#endif /* LINK_TRANSPORT_UART_H_ */
