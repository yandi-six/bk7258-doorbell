#include "usbd_core.h"
#include "usbd_hid.h"
#include <os/os.h>
#include <os/mem.h>

/*!< hidraw in endpoint */
#define HIDRAW_IN_EP       0x81
#define HIDRAW_IN_SIZE     64
#define HIDRAW_IN_INTERVAL 10

/*!< hidraw out endpoint */
#define HIDRAW_OUT_EP          0x02
#define HIDRAW_OUT_EP_SIZE     64
#define HIDRAW_OUT_EP_INTERVAL 10

#define USBD_VID           0xffff
#define USBD_PID           0xffff
#define USBD_MAX_POWER     500
#define USBD_LANGID_STRING 1033

/*!< config descriptor size */
#define USB_HID_CONFIG_DESC_SIZ (9 + 9 + 9 + 7 + 7)

/*!< custom hid report descriptor size */
#define HID_CUSTOM_REPORT_DESC_SIZE 34

static uint8_t s_usbd_hid_is_init = false;
static void usbd_hid_out_callback(uint8_t ep, uint32_t nbytes);
static void usbd_hid_in_callback(uint8_t ep, uint32_t nbytes);

static void hid_print(uint8_t *buffer, int length)
{
	for (int i = 0; i < length; i++)
	{
		os_printf("%02x ", *(buffer+i));
	}
	os_printf("\r\n");
}
//#define USB_DEVICE_DESCRIPTOR_INIT(bcdUSB, bDeviceClass, bDeviceSubClass, bDeviceProtocol, idVendor, idProduct, bcdDevice, bNumConfigurations) 
//#define USB_CONFIG_DESCRIPTOR_INIT(wTotalLength, bNumInterfaces, bConfigurationValue, bmAttributes, bMaxPower)

static const uint8_t hid_descriptor[] = {
	USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x00, 0x00, 0x00, USBD_VID, USBD_PID, 0x0002, 0x01),
	USB_CONFIG_DESCRIPTOR_INIT(USB_HID_CONFIG_DESC_SIZ, 0x01, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
	/************** Descriptor of Custom interface *****************/
	0x09,                          /* bLength: Interface Descriptor size */
	USB_DESCRIPTOR_TYPE_INTERFACE, /* bDescriptorType: Interface descriptor type */
	0x00,                          /* bInterfaceNumber: Number of Interface */
	0x00,                          /* bAlternateSetting: Alternate setting */
	0x02,                          /* bNumEndpoints */
	0x03,                          /* bInterfaceClass: HID */
	0x00,                          /* bInterfaceSubClass : 1=BOOT, 0=no boot */
	0x00,                          /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
	0,                             /* iInterface: Index of string descriptor */
	/******************** Descriptor of Custom HID ********************/
	0x09,                    /* bLength: HID Descriptor size */
	HID_DESCRIPTOR_TYPE_HID, /* bDescriptorType: HID */
	0x11,                    /* bcdHID: HID Class Spec release number */
	0x01,
	0x00,                        /* bCountryCode: Hardware target country */
	0x01,                        /* bNumDescriptors: Number of HID class descriptors to follow */
	0x22,                        /* bDescriptorType */
	HID_CUSTOM_REPORT_DESC_SIZE, /* wItemLength: Total length of Report descriptor */
	0x00,
	/******************** Descriptor of Custom in endpoint ********************/
	0x07,                         /* bLength: Endpoint Descriptor size */
	USB_DESCRIPTOR_TYPE_ENDPOINT, /* bDescriptorType: */
	HIDRAW_IN_EP,                 /* bEndpointAddress: Endpoint Address (IN) */
	0x03,                         /* bmAttributes: Interrupt endpoint */
	WBVAL(HIDRAW_IN_SIZE),        /* wMaxPacketSize: 4 Byte max */
	HIDRAW_IN_INTERVAL,           /* bInterval: Polling Interval */
	/******************** Descriptor of Custom out endpoint ********************/
	0x07,                         /* bLength: Endpoint Descriptor size */
	USB_DESCRIPTOR_TYPE_ENDPOINT, /* bDescriptorType: */
	HIDRAW_OUT_EP,                /* bEndpointAddress: Endpoint Address (IN) */
	0x03,                         /* bmAttributes: Interrupt endpoint */
	WBVAL(HIDRAW_OUT_EP_SIZE),    /* wMaxPacketSize: 4 Byte max */
	HIDRAW_OUT_EP_INTERVAL,       /* bInterval: Polling Interval */
	/* 73 */
	///////////////////////////////////////
	/// string0 descriptor
	///////////////////////////////////////
	USB_LANGID_INIT(USBD_LANGID_STRING),
	///////////////////////////////////////
	/// string1 descriptor
	///////////////////////////////////////
	0x14,                       /* bLength */
	USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
	'C', 0x00,                  /* wcChar0 */
	'h', 0x00,                  /* wcChar1 */
	'e', 0x00,                  /* wcChar2 */
	'r', 0x00,                  /* wcChar3 */
	'r', 0x00,                  /* wcChar4 */
	'y', 0x00,                  /* wcChar5 */
	'U', 0x00,                  /* wcChar6 */
	'S', 0x00,                  /* wcChar7 */
	'B', 0x00,                  /* wcChar8 */
	///////////////////////////////////////
	/// string2 descriptor
	///////////////////////////////////////
	0x26,                       /* bLength */
	USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
	'C', 0x00,                  /* wcChar0 */
	'h', 0x00,                  /* wcChar1 */
	'e', 0x00,                  /* wcChar2 */
	'r', 0x00,                  /* wcChar3 */
	'r', 0x00,                  /* wcChar4 */
	'y', 0x00,                  /* wcChar5 */
	'U', 0x00,                  /* wcChar6 */
	'S', 0x00,                  /* wcChar7 */
	'B', 0x00,                  /* wcChar8 */
	' ', 0x00,                  /* wcChar9 */
	'H', 0x00,                  /* wcChar10 */
	'I', 0x00,                  /* wcChar11 */
	'D', 0x00,                  /* wcChar12 */
	' ', 0x00,                  /* wcChar13 */
	'D', 0x00,                  /* wcChar14 */
	'E', 0x00,                  /* wcChar15 */
	'M', 0x00,                  /* wcChar16 */
	'O', 0x00,                  /* wcChar17 */
	///////////////////////////////////////
	/// string3 descriptor
	///////////////////////////////////////
	0x16,                       /* bLength */
	USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
	'2', 0x00,                  /* wcChar0 */
	'0', 0x00,                  /* wcChar1 */
	'2', 0x00,                  /* wcChar2 */
	'2', 0x00,                  /* wcChar3 */
	'1', 0x00,                  /* wcChar4 */
	'2', 0x00,                  /* wcChar5 */
	'3', 0x00,                  /* wcChar6 */
	'4', 0x00,                  /* wcChar7 */
	'5', 0x00,                  /* wcChar8 */
	'6', 0x00,                  /* wcChar9 */


	0x00
};

static const uint8_t hid_custom_report_desc[] = {
    /* USER CODE BEGIN 0 */
    0x06, 0x00, 0xff, // USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01,       // USAGE (Vendor Usage 1)
    0xa1, 0x01,       // COLLECTION (Application)
    0x09, 0x01,       //   USAGE (Vendor Usage 1)
    0x15, 0x00,       //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00, //   LOGICAL_MAXIMUM (255)
    0x95, 0x40,       //   REPORT_COUNT (64)
    0x75, 0x08,       //   REPORT_SIZE (8)
    0x81, 0x02,       //   INPUT (Data,Var,Abs)
    /* <___________________________________________________> */
    0x09, 0x01,       //   USAGE (Vendor Usage 1)
    0x15, 0x00,       //   LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00, //   LOGICAL_MAXIMUM (255)
    0x95, 0x40,       //   REPORT_COUNT (64)
    0x75, 0x08,       //   REPORT_SIZE (8)
    0x91, 0x02,       //   OUTPUT (Data,Var,Abs)
    /* USER CODE END 0 */
    0xC0 /*     END_COLLECTION	             */
};

struct usbd_interface hid_intf0;

static struct usbd_endpoint hid_out_ep = {
	.ep_cb = usbd_hid_out_callback,
	.ep_addr = HIDRAW_OUT_EP
};

static struct usbd_endpoint hid_in_ep = {
    .ep_cb = usbd_hid_in_callback,
    .ep_addr = HIDRAW_IN_EP
};

static void usbd_hid_out_callback(uint8_t ep, uint32_t nbytes)
{
	uint8_t read_buffer[64] = {0};
	usbd_ep_start_read(HIDRAW_OUT_EP, read_buffer, nbytes);
	read_buffer[0] = 0xFF;
	read_buffer[1] = 0xFF;
	read_buffer[2] = 0xFF;
	read_buffer[3] = 0xFF;
	usbd_ep_start_write(HIDRAW_IN_EP, read_buffer, nbytes);
}

static void usbd_hid_in_callback(uint8_t ep, uint32_t nbytes)
{
    USB_LOG_RAW("in report len:%d\r\n", nbytes);
}


void usbd_hid_init()
{
	if (s_usbd_hid_is_init) {
		return;
	}

	usbd_desc_register(hid_descriptor);
	usbd_add_interface(usbd_hid_init_intf(&hid_intf0, hid_custom_report_desc, HID_CUSTOM_REPORT_DESC_SIZE));
	usbd_add_endpoint(&hid_out_ep);
	usbd_add_endpoint(&hid_in_ep);

	usbd_initialize();
	s_usbd_hid_is_init = true;
}
