#pragma once
#ifndef _GCN_ADAPTER_HID_H_
#define _GCN_ADAPTER_HID_H_

#include "Include.h"

typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

extern CONST HID_REPORT_DESCRIPTOR ReportDescriptor[];
extern CONST HID_DESCRIPTOR HIDDescriptor;

/**	Translates an IOCTL code to a string.
 *
 *	@param aIoControlCode IOCTL code.
 *
 *	@returns C string representation of the given IOCTL code.
 *
 */
PCHAR DbgHidInternalIoctlString(_In_ ULONG aIoControlCode);

#endif//_GCN_ADAPTER_HID_H_
