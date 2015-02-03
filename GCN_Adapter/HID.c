#include "Include.h"

CONST HID_REPORT_DESCRIPTOR ReportDescriptor[] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,                    // USAGE (Game Pad)
	0xa1, 0x01,                    // COLLECTION (Application)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x85, 0x01,                    //     REPORT_ID (1)
	0x05, 0x09,                    //     USAGE_PAGE (Button)
	0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
	0x29, 0x0c,                    //     USAGE_MAXIMUM (Button 12)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x95, 0x0c,                    //     REPORT_COUNT (12)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x95, 0x04,                    //     REPORT_COUNT (4)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //     USAGE (X)
	0x09, 0x31,                    //     USAGE (Y)
	0x09, 0x32,                    //     USAGE (Z)
	0x09, 0x33,                    //     USAGE (Rx)
	0x09, 0x34,                    //     USAGE (Ry)
	0x09, 0x35,                    //     USAGE (Rz)
	0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
	0x46, 0xff, 0x00,              //     PHYSICAL_MAXIMUM (255)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x06,                    //     REPORT_COUNT (6)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0xc0,                          //   END_COLLECTION
	0xc0,                          // END_COLLECTION
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,                    // USAGE (Game Pad)
	0xa1, 0x01,                    // COLLECTION (Application)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x85, 0x02,                    //     REPORT_ID (2)
	0x05, 0x09,                    //     USAGE_PAGE (Button)
	0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
	0x29, 0x0c,                    //     USAGE_MAXIMUM (Button 12)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x95, 0x0c,                    //     REPORT_COUNT (12)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x95, 0x04,                    //     REPORT_COUNT (4)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //     USAGE (X)
	0x09, 0x31,                    //     USAGE (Y)
	0x09, 0x32,                    //     USAGE (Z)
	0x09, 0x33,                    //     USAGE (Rx)
	0x09, 0x34,                    //     USAGE (Ry)
	0x09, 0x35,                    //     USAGE (Rz)
	0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
	0x46, 0xff, 0x00,              //     PHYSICAL_MAXIMUM (255)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x06,                    //     REPORT_COUNT (6)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0xc0,                          //   END_COLLECTION
	0xc0,                          // END_COLLECTION
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,                    // USAGE (Game Pad)
	0xa1, 0x01,                    // COLLECTION (Application)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x85, 0x03,                    //     REPORT_ID (3)
	0x05, 0x09,                    //     USAGE_PAGE (Button)
	0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
	0x29, 0x0c,                    //     USAGE_MAXIMUM (Button 12)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x95, 0x0c,                    //     REPORT_COUNT (12)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x95, 0x04,                    //     REPORT_COUNT (4)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //     USAGE (X)
	0x09, 0x31,                    //     USAGE (Y)
	0x09, 0x32,                    //     USAGE (Z)
	0x09, 0x33,                    //     USAGE (Rx)
	0x09, 0x34,                    //     USAGE (Ry)
	0x09, 0x35,                    //     USAGE (Rz)
	0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
	0x46, 0xff, 0x00,              //     PHYSICAL_MAXIMUM (255)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x06,                    //     REPORT_COUNT (6)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0xc0,                          //               END_COLLECTION
	0xc0,                          //   END_COLLECTION
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,                    //   USAGE (Game Pad)
	0xa1, 0x01,                    //   COLLECTION (Application)
	0xa1, 0x00,                    //     COLLECTION (Physical)
	0x85, 0x04,                    //     REPORT_ID (4)
	0x05, 0x09,                    //       USAGE_PAGE (Button)
	0x19, 0x01,                    //       USAGE_MINIMUM (Button 1)
	0x29, 0x0c,                    //       USAGE_MAXIMUM (Button 12)
	0x15, 0x00,                    //       LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //       LOGICAL_MAXIMUM (1)
	0x95, 0x0c,                    //       REPORT_COUNT (12)
	0x75, 0x01,                    //       REPORT_SIZE (1)
	0x81, 0x02,                    //       INPUT (Data,Var,Abs)
	0x75, 0x01,                    //       REPORT_SIZE (1)
	0x95, 0x04,                    //       REPORT_COUNT (4)
	0x81, 0x03,                    //       INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //       USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //       USAGE (X)
	0x09, 0x31,                    //       USAGE (Y)
	0x09, 0x32,                    //       USAGE (Z)
	0x09, 0x33,                    //       USAGE (Rx)
	0x09, 0x34,                    //       USAGE (Ry)
	0x09, 0x35,                    //       USAGE (Rz)
	0x35, 0x00,                    //       PHYSICAL_MINIMUM (0)
	0x46, 0xff, 0x00,              //       PHYSICAL_MAXIMUM (255)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
	0x75, 0x08,                    //       REPORT_SIZE (8)
	0x95, 0x06,                    //       REPORT_COUNT (6)
	0x81, 0x02,                    //       INPUT (Data,Var,Abs)
	0xc0,                          //   END_COLLECTION
	0xc0                           // END_COLLECTION
};

CONST HID_DESCRIPTOR HIDDescriptor = {
	0x09,   // length of HID descriptor
	0x21,   // descriptor type == HID  0x21
	0x0110, // hid spec release
	0x00,   // country code == Not Specified
	0x01,   // number of HID class descriptors
	{ 0x22,   // descriptor type 
	sizeof(ReportDescriptor) }  // total length of custom report descriptor
};

_IRQL_requires_min_(PASSIVE_LEVEL)
PCHAR DbgHidInternalIoctlString(
	_In_ ULONG aIoControlCode)
{
	switch (aIoControlCode)
	{
	case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
		return "IOCTL_HID_GET_DEVICE_DESCRIPTOR";
	case IOCTL_HID_GET_REPORT_DESCRIPTOR:
		return "IOCTL_HID_GET_REPORT_DESCRIPTOR";
	case IOCTL_HID_READ_REPORT:
		return "IOCTL_HID_READ_REPORT";
	case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
		return "IOCTL_HID_GET_DEVICE_ATTRIBUTES";
	case IOCTL_HID_WRITE_REPORT:
		return "IOCTL_HID_WRITE_REPORT";
	case IOCTL_HID_SET_FEATURE:
		return "IOCTL_HID_SET_FEATURE";
	case IOCTL_HID_GET_FEATURE:
		return "IOCTL_HID_GET_FEATURE";
	case IOCTL_HID_GET_STRING:
		return "IOCTL_HID_GET_STRING";
	case IOCTL_HID_ACTIVATE_DEVICE:
		return "IOCTL_HID_ACTIVATE_DEVICE";
	case IOCTL_HID_DEACTIVATE_DEVICE:
		return "IOCTL_HID_DEACTIVATE_DEVICE";
	case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:
		return "IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST";
	default:
		return "Unknown IOCTL";
	}
}
