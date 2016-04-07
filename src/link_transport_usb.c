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

#include "link_transport_usb.h"

#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <iface/link.h>
#include <mcu/mcu.h>
#include <iface/dev/usb.h>
#include <iface/dev/pio.h>
#include <iface/dev/usbfifo.h>
#include <stratify/usb_dev.h>
#include <stratify/usb_dev_cdc.h>
#include <mcu/core.h>
#include <mcu/debug.h>
#include <device/sys.h>

#include "led_start.h"

link_transport_driver_t link_transport_usb = {
		.handle = -1,
		.open = link_transport_usb_open,
		.read = link_transport_usb_read,
		.write = link_transport_usb_write,
		.close = link_transport_usb_close,
		.wait = link_transport_usb_wait,
		.flush = link_transport_usb_flush,
		.timeout = 500
};

static int cdc_if_req(void * context, int event);


typedef struct MCU_PACK {
	usb_dev_cdc_header_t header;
	usb_dev_cdc_acm_t acm;
	usb_dev_cdc_uniondescriptor_t union_descriptor;
	usb_dev_cdc_callmanagement_t call_management;
} link_cdc_acm_interface_t;

typedef struct MCU_PACK {
	usb_dev_cdc_header_t header;
	usb_dev_cdc_acm_t acm;
	usb_dev_cdc_callmanagement_t call_management;
} link_cdc_acm_interface_alt_t;

typedef struct MCU_PACK {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bFirstInterface;
	uint8_t bInterfaceCount;
	uint8_t bFunctionClass;
	uint8_t bFunctionSubClass;
	uint8_t bFunctionProtocol;
	uint8_t iFunction;
} usb_dev_interface_assocation_t;

#define USB_PORT 0
#define USB_INTIN 0x81
#define USB_INTIN_ALT 0x84
#define USB_BULKIN (0x80|LINK_USBPHY_BULK_ENDPOINT)
#define USB_BULKOUT (LINK_USBPHY_BULK_ENDPOINT)
#define USB_BULKIN_ALT (0x80|LINK_USBPHY_BULK_ENDPOINT_ALT)
#define USB_BULKOUT_ALT (LINK_USBPHY_BULK_ENDPOINT_ALT)
#define ENDPOINT_SIZE LINK_USBPHY_BULK_ENDPOINT_SIZE

typedef struct MCU_PACK {
	usb_dev_interface_assocation_t if_asso;
	usb_interface_desc_t ifcontrol /* The interface descriptor */;
	link_cdc_acm_interface_t acm /*! The CDC ACM Class descriptor */;
	usb_ep_desc_t control /* Endpoint:  Interrupt out for control packets */;
	usb_interface_desc_t ifdata /* The interface descriptor */;
	usb_ep_desc_t data_out /* Endpoint:  Bulk out */;
	usb_ep_desc_t data_in /* Endpoint:  Bulk in */;
} link_vcp_t;

/* \details This structure defines the USB descriptors.  This
 * value is read over the control channel by the host to configure
 * the device.
 */
typedef struct MCU_PACK {
	usb_cfg_desc_t cfg /* The configuration descriptor */;
	link_vcp_t vcp0;
#ifdef __STDIO_VCP
	link_vcp_t vcp1;
#endif
	uint8_t terminator  /* A null terminator used by the driver (required) */;
} link_cfg_desc_t;




#define LINK_USB_VID 0x20A0

#ifdef __STDIO_VCP
#define LINK_USB_PID 0x413B
#else
#define LINK_USB_PID 0x41D5
#endif

#ifndef LINK_USB_DEV_PORT
#define LINK_USB_DEV_PORT 0
#endif

#define LINK_USB_DESC_MANUFACTURER_SIZE 13
#define LINK_USB_DESC_PRODUCT_SIZE 10
#define LINK_USB_DESC_SERIAL_SIZE 16
#define LINK_USB_DESC_MANUFACTUER_STRING 'S','t','r','a','t','i','f','y',' ','L','a','b','s'
#define LINK_USB_DESC_PRODUCT_STRING 'S','t','r','a','t','i','f','y','O','S'
#define LINK_USB_DESC_SERIAL_STRING '0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'

#define LINK_USB_DESC_VCP_0_SIZE 20
#define LINK_USB_DESC_VCP_1_SIZE 21
#define LINK_USB_DESC_VCP_0 'S','t','r','a','t','i','f','y','O','S',' ','L','i','n','k',' ','P','o','r','t'
#define LINK_USB_DESC_VCP_1 'S','t','r','a','t','i','f','y','O','S',' ','S','t','d','i','o',' ','P','o','r','t'


#define LINK_REQD_CURRENT 500


/*! \brief USB Link String Data
 * \details This structure defines the USB strings structure which includes
 * a string for the manufacturer, product, and serial number.
 */
struct MCU_PACK link_usb_string_t {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wLANGID;
	usb_declare_string(LINK_USB_DESC_MANUFACTURER_SIZE) manufacturer;
	usb_declare_string(LINK_USB_DESC_PRODUCT_SIZE) product;
	usb_declare_string(LINK_USB_DESC_SERIAL_SIZE) serial;
	usb_declare_string(LINK_USB_DESC_VCP_0_SIZE) vcp0;
#ifdef __STDIO_VCP
	usb_declare_string(LINK_USB_DESC_VCP_1_SIZE) vcp1;
#endif
};



/*! \details This variable stores the device descriptor.  It has a weak attribute and can be
 * overridden by using a user specific value
 * in the file \a devices.c.  This allows the user to change the USB vendor and product IDs.
 * All other values should be unchanged.
 *
 */
const usb_dev_desc_t link_dev_desc MCU_WEAK = {
		.bLength = sizeof(usb_dev_desc_t),
		.bDescriptorType = USB_DEVICE_DESCRIPTOR_TYPE,
		.bcdUSB = 0x0200,
#ifdef __STDIO_VCP
		.bDeviceClass = USB_DEVICE_CLASS_MISCELLANEOUS,
		.bDeviceSubClass = 2,
		.bDeviceProtocol = 1,
#else
		.bDeviceClass = USB_DEVICE_CLASS_COMMUNICATIONS,
		.bDeviceSubClass = 0,
		.bDeviceProtocol = 0,
#endif
		.bMaxPacketSize = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,
		.idVendor = LINK_USB_VID,
		.idProduct = LINK_USB_PID,
		.bcdDevice = 0x170,
		.iManufacturer = 1,
		.iProduct = 2,
		.iSerialNumber = 3,
		.bNumConfigurations = 1
};

const link_cfg_desc_t link_cfg_desc MCU_WEAK = {

		.cfg = {
				.bLength = sizeof(usb_cfg_desc_t),
				.bDescriptorType = USB_CONFIGURATION_DESCRIPTOR_TYPE,
				.wTotalLength = sizeof(link_cfg_desc_t)-1, //exclude the zero terminator
#ifdef __STDIO_VCP
				.bNumInterfaces = 0x04,
#else
				.bNumInterfaces = 0x02,
#endif
				.bConfigurationValue = 0x01,
				.iConfiguration = 0x03,
				.bmAttributes = USB_CONFIG_BUS_POWERED,
				.bMaxPower = USB_CONFIG_POWER_MA( LINK_REQD_CURRENT )
		},

		.vcp0 = {
				.if_asso = {
						.bLength = sizeof(usb_dev_interface_assocation_t),
						.bDescriptorType = USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE,
						.bFirstInterface = 0,
						.bInterfaceCount = 2,
						.bFunctionClass = USB_INTERFACE_CLASS_COMMUNICATIONS,
						.bFunctionSubClass = USB_INTERFACE_SUBCLASS_ACM,
						.bFunctionProtocol = USB_INTERFACE_PROTOCOL_V25TER,
						.iFunction = 0x04,
				},

				.ifcontrol = {
						.bLength = sizeof(usb_interface_desc_t),
						.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
						.bInterfaceNumber = 0x00,
						.bAlternateSetting = 0x00,
						.bNumEndpoints = 0x01,
						.bInterfaceClass = USB_INTERFACE_CLASS_COMMUNICATIONS,
						.bInterfaceSubClass = USB_INTERFACE_SUBCLASS_ACM,
						.bInterfaceProtocol = USB_INTERFACE_PROTOCOL_V25TER,
						.iInterface = 0x04
				},

				.acm = {
						.header.bLength = sizeof(usb_dev_cdc_header_t),
						.header.bDescriptorType = 0x24,
						.header.bDescriptorSubType = 0x00,
						.header.bcdCDC = 0x0110,
						.acm.bFunctionLength = sizeof(usb_dev_cdc_acm_t),
						.acm.bDescriptorType = 0x24,
						.acm.bDescriptorSubType = 0x02,
						.acm.bmCapabilities = 0x00,
						.union_descriptor.bFunctionLength = sizeof(usb_dev_cdc_uniondescriptor_t),
						.union_descriptor.bDescriptorType = 0x24,
						.union_descriptor.bDescriptorSubType = 0x06,
						.union_descriptor.bMasterInterface = 0x00,
						.union_descriptor.bSlaveInterface = 0x01,
						.call_management.bFunctionLength = sizeof(usb_dev_cdc_callmanagement_t),
						.call_management.bDescriptorType = 0x24,
						.call_management.bDescriptorSubType = 0x01,
						.call_management.bmCapabilities = 0x00,
						.call_management.bDataInterface = 0x01
				},

				.control = {
						.bLength= sizeof(usb_ep_desc_t),
						.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
						.bEndpointAddress=USB_INTIN,
						.bmAttributes=USB_ENDPOINT_TYPE_INTERRUPT,
						.wMaxPacketSize=LINK_INTERRUPT_ENDPOINT_SIZE,
						.bInterval=1
				},

				.ifdata = {
						.bLength = sizeof(usb_interface_desc_t),
						.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
						.bInterfaceNumber = 0x01,
						.bAlternateSetting = 0x00,
						.bNumEndpoints = 0x02,
						.bInterfaceClass = USB_INTERFACE_CLASS_COMMUNICATIONS_DATA,
						.bInterfaceSubClass = 0x00,
						.bInterfaceProtocol = 0x00,
						.iInterface = 0x04
				},

				.data_out = {
						.bLength= sizeof(usb_ep_desc_t),
						.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
						.bEndpointAddress=USB_BULKOUT,
						.bmAttributes=USB_ENDPOINT_TYPE_BULK,
						.wMaxPacketSize=LINK_BULK_ENDPOINT_SIZE,
						.bInterval=1
				},

				.data_in = {
						.bLength= sizeof(usb_ep_desc_t),
						.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
						.bEndpointAddress=USB_BULKIN,
						.bmAttributes=USB_ENDPOINT_TYPE_BULK,
						.wMaxPacketSize=LINK_BULK_ENDPOINT_SIZE,
						.bInterval=1
				}
		},

#ifdef __STDIO_VCP

		.vcp1 = {
				.if_asso = {
						.bLength = sizeof(usb_dev_interface_assocation_t),
						.bDescriptorType = USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE,
						.bFirstInterface = 2,
						.bInterfaceCount = 2,
						.bFunctionClass = USB_INTERFACE_CLASS_COMMUNICATIONS,
						.bFunctionSubClass = USB_INTERFACE_SUBCLASS_ACM,
						.bFunctionProtocol = USB_INTERFACE_PROTOCOL_V25TER,
						.iFunction = 0x05,
				},

				.ifcontrol = {
						.bLength = sizeof(usb_interface_desc_t),
						.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
						.bInterfaceNumber = 0x02,
						.bAlternateSetting = 0x00,
						.bNumEndpoints = 0x01,
						.bInterfaceClass = USB_INTERFACE_CLASS_COMMUNICATIONS,
						.bInterfaceSubClass = USB_INTERFACE_SUBCLASS_ACM,
						.bInterfaceProtocol = USB_INTERFACE_PROTOCOL_V25TER,
						.iInterface = 0x05
				},

				.acm = {
						.header.bLength = sizeof(usb_dev_cdc_header_t),
						.header.bDescriptorType = 0x24,
						.header.bDescriptorSubType = 0x00,
						.header.bcdCDC = 0x0110,
						.acm.bFunctionLength = sizeof(usb_dev_cdc_acm_t),
						.acm.bDescriptorType = 0x24,
						.acm.bDescriptorSubType = 0x02,
						.acm.bmCapabilities = 0x00,
						.union_descriptor.bFunctionLength = sizeof(usb_dev_cdc_uniondescriptor_t),
						.union_descriptor.bDescriptorType = 0x24,
						.union_descriptor.bDescriptorSubType = 0x06,
						.union_descriptor.bMasterInterface = 0x02,
						.union_descriptor.bSlaveInterface = 0x03,
						.call_management.bFunctionLength = sizeof(usb_dev_cdc_callmanagement_t),
						.call_management.bDescriptorType = 0x24,
						.call_management.bDescriptorSubType = 0x01,
						.call_management.bmCapabilities = 0x00,
						.call_management.bDataInterface = 0x03
				},

				.control = {
						.bLength= sizeof(usb_ep_desc_t),
						.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
						.bEndpointAddress=USB_INTIN_ALT,
						.bmAttributes=USB_ENDPOINT_TYPE_INTERRUPT,
						.wMaxPacketSize=LINK_INTERRUPT_ENDPOINT_SIZE,
						.bInterval=1
				},

				.ifdata = {
						.bLength = sizeof(usb_interface_desc_t),
						.bDescriptorType = USB_INTERFACE_DESCRIPTOR_TYPE,
						.bInterfaceNumber = 0x03,
						.bAlternateSetting = 0x00,
						.bNumEndpoints = 0x02,
						.bInterfaceClass = USB_INTERFACE_CLASS_COMMUNICATIONS_DATA,
						.bInterfaceSubClass = 0x00,
						.bInterfaceProtocol = 0x00,
						.iInterface = 0x05
				},

				.data_out = {
						.bLength= sizeof(usb_ep_desc_t),
						.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
						.bEndpointAddress=USB_BULKOUT_ALT,
						.bmAttributes=USB_ENDPOINT_TYPE_BULK,
						.wMaxPacketSize=LINK_BULK_ENDPOINT_SIZE,
						.bInterval=1
				},

				.data_in = {
						.bLength= sizeof(usb_ep_desc_t),
						.bDescriptorType=USB_ENDPOINT_DESCRIPTOR_TYPE,
						.bEndpointAddress=USB_BULKIN_ALT,
						.bmAttributes=USB_ENDPOINT_TYPE_BULK,
						.wMaxPacketSize=LINK_BULK_ENDPOINT_SIZE,
						.bInterval=1
				}
		},
#endif



		.terminator = 0
};


#define CONNECT_PORT "/dev/pio2"
#define CONNECT_PINMASK (1<<9)

/*! \details This variable stores the USB strings as the defaults listed below:
 * - Manufacturer: "Stratify OS, Inc"
 * - Product: "Stratify OS"
 * - Serial Number: "00000000"
 *
 * This variable has a weak attribute.  It can be overridden by using a user specific value
 * is the file \a devices.c.
 *
 */
const struct link_usb_string_t link_string_desc MCU_WEAK = {
		.bLength = 4,
		.bDescriptorType = USB_STRING_DESCRIPTOR_TYPE,
		.wLANGID = 0x0409, //English
		.manufacturer = usb_assign_string(LINK_USB_DESC_MANUFACTURER_SIZE, LINK_USB_DESC_MANUFACTUER_STRING),
		.product = usb_assign_string(LINK_USB_DESC_PRODUCT_SIZE, LINK_USB_DESC_PRODUCT_STRING),
		.serial = usb_assign_string(LINK_USB_DESC_SERIAL_SIZE, 0)
		, //dynamically load SN based on silicon
		.vcp0 = usb_assign_string(LINK_USB_DESC_VCP_0_SIZE, LINK_USB_DESC_VCP_0),
#ifdef __STDIO_VCP
		.vcp1 = usb_assign_string(LINK_USB_DESC_VCP_1_SIZE, LINK_USB_DESC_VCP_1)
#endif

};

#ifdef __STDIO_VCP
void init_stdio_vcp(){
	int fd;
	fd = open("/dev/stdio", O_RDWR);
	if( fd < 0 ){
		return;
	}

	ioctl(fd, I_FIFO_INIT);
	close(fd);
	return;
}
#endif

const usb_dev_const_t usb_constants = {
		.port = 0,
		.device =  &link_dev_desc,
		.config = &link_cfg_desc,
		.string = &link_string_desc,
		.feature_event = usb_dev_default_event,
		.configure_event = usb_dev_default_event,
		.interface_event = usb_dev_default_event,
		.adc_if_req = usb_dev_default_if_req,
		.msc_if_req = usb_dev_default_if_req,
		.cdc_if_req = cdc_if_req,
		.hid_if_req = usb_dev_default_if_req
};

static usb_dev_context_t usb_context;

link_transport_phy_t link_transport_usb_open(const char * name, int baudrate){
	link_transport_phy_t fd;
	pio_attr_t attr;
	usb_attr_t usb_attr;
	int fd_pio;

	//Deassert the Connect line and enable the output
	fd_pio = open(CONNECT_PORT, O_RDWR);
	if( fd_pio < 0 ){
		return -1;
	}
	attr.mask = (CONNECT_PINMASK);
	ioctl(fd_pio, I_PIO_SETMASK, (void*)attr.mask);
	attr.mode = PIO_MODE_OUTPUT | PIO_MODE_DIRONLY;
	ioctl(fd_pio, I_PIO_SETATTR, &attr);


	//open USB
	mcu_debug("Open link-phy-usb\n");
	fd = open("/dev/link-phy-usb", O_RDWR);
	if( fd < 0 ){
		return LINK_PHY_ERROR;
	}

	//set USB attributes
	mcu_debug("Set USB attr %d\n", fd);

	usb_attr.pin_assign = 0;
	usb_attr.mode = USB_ATTR_MODE_DEVICE;
	usb_attr.crystal_freq = mcu_board_config.core_osc_freq;
	if( ioctl(fd, I_USB_SETATTR, &usb_attr) < 0 ){
		return LINK_PHY_ERROR;
	}

	mcu_debug("USB Dev Init\n");
	//initialize USB device
	//initialize USB device
	memset(&usb_context, 0, sizeof(usb_context));
	usb_context.constants = &usb_constants;
	mcu_core_privcall(usb_dev_priv_init, &usb_context);


	ioctl(fd_pio, I_PIO_CLRMASK, (void*)attr.mask);

#ifdef __SECURE
	int fd_sys;
	fd_sys = open("/dev/sys", O_RDWR);
	if( fd_sys >= 0 ){
		ioctl(fd_sys, I_SYS_SUDO, 0); //exit SUDO mode
		close(fd_sys);
	}
#endif

#ifdef __STDIO_VCP
	init_stdio_vcp();
#endif

	led_start();

	return fd;
}

int link_transport_usb_write(link_transport_phy_t handle, const void * buf, int nbyte){
	int ret;
	ret = write(handle, buf, nbyte);
	return ret;
}

int link_transport_usb_read(link_transport_phy_t handle, void * buf, int nbyte){
	int ret;
	errno = 0;
	ret = read(handle, buf, nbyte);
	return ret;
}

int link_transport_usb_close(link_transport_phy_t handle){
	return close(handle);
}

void link_transport_usb_wait(int msec){
	int i;
	for(i = 0; i < msec; i++){
		usleep(1000);
	}
}

void link_transport_usb_flush(link_transport_phy_t handle){
	ioctl(handle, I_FIFO_FLUSH);
}

int cdc_if_req(void * object, int event){
	u32 rate = 12000000;
	usb_dev_context_t * context = object;

	if ( (context->setup_pkt.wIndex.u8[0] == 0) || (context->setup_pkt.wIndex.u8[0] == 1) ||
			(context->setup_pkt.wIndex.u8[0] == 2) || (context->setup_pkt.wIndex.u8[0] == 3) ) { //! \todo The wIndex should equal the CDC interface number

		if ( (event == USB_SETUP_EVENT) ){
			switch(context->setup_pkt.bRequest){
			case SET_LINE_CODING:
			case SET_COMM_FEATURE:
			case SEND_BREAK:
			case SEND_ENCAPSULATED_COMMAND:
				//need to receive information from the host
				context->ep0_data.dptr = context->ep0_buf;
				return 1;
			case SET_CONTROL_LINE_STATE:
				usb_dev_std_statusin_stage(context);
				return 1;
			case GET_LINE_CODING:
				context->ep0_data.dptr = context->ep0_buf;

				//copy line coding to dev_std_ep0_buf
				context->ep0_buf[0] = (rate >>  0) & 0xFF;
				context->ep0_buf[1] = (rate >>  8) & 0xFF;
				context->ep0_buf[2] = (rate >> 16) & 0xFF;
				context->ep0_buf[3] = (rate >> 24) & 0xFF;  //rate
				context->ep0_buf[4] =  0; //stop bits 1
				context->ep0_buf[5] =  0; //no parity
				context->ep0_buf[6] =  8; //8 data bits

				usb_dev_std_statusin_stage(context);
				return 1;
			case CLEAR_COMM_FEATURE:
				usb_dev_std_statusin_stage(context);
				return 1;
			case GET_COMM_FEATURE:
				context->ep0_data.dptr = context->ep0_buf;
				//copy data to dev_std_ep0_buf
				usb_dev_std_statusin_stage(context);
				return 1;
			case GET_ENCAPSULATED_RESPONSE:
				context->ep0_data.dptr = context->ep0_buf;
				//copy data to dev_std_ep0_buf
				usb_dev_std_statusin_stage(context);
				return 1;
			default:
				return 0;
			}
		} else if ( event == USB_OUT_EVENT ){
			switch(context->setup_pkt.bRequest){
			case SET_LINE_CODING:
			case SET_CONTROL_LINE_STATE:
			case SET_COMM_FEATURE:
			case SEND_ENCAPSULATED_COMMAND:
				//use data in dev_std_ep0_buf to take action
				usb_dev_std_statusin_stage(context);
				return 1;
			default:
				return 0;
			}
		}
	}
	//The request was not handled
	return 0;
}
