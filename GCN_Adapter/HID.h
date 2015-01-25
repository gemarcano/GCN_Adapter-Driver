#pragma once
#ifndef _GCN_ADAPTER_HID_H_
#define _GCN_ADAPTER_HID_H_

/**	@file
 *	This file contains function and variable declarations/definitions regarding
 *	HID specific information.
 *
 */

#include "Include.h"

typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

extern CONST HID_REPORT_DESCRIPTOR ReportDescriptor[];
extern CONST HID_DESCRIPTOR HIDDescriptor;

/**	Translates an IOCTL code to a string.
 *
 *	@param aIoControlCode IOCTL code.
 *
 *	@remark This function can run at any IRQL (is this true?).
 *	@remark This function is not paged.
 *
 *	@returns C string representation of the given IOCTL code.
 *
 */
PCHAR DbgHidInternalIoctlString(_In_ ULONG aIoControlCode);

#endif//_GCN_ADAPTER_HID_H_
