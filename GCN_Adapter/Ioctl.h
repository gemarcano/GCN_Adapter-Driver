#pragma once
#ifndef _GCN_ADAPTER_IOCTL_H_
#define _GCN_ADAPTER_IOCTL_H_

#include "Include.h"

/**	Handles the completion of pended IOCTL requests (READ_REPORT).
 *
 *	@param [in] aDevice USB Device created by GCN_AdaptorCreateDevice. 
 *	@param [in] aReaderStatus Status of how the read went.
 *	
 */
VOID GCN_AdaptorUsbIoctlGetInterruptMessage(
	_In_ WDFDEVICE aDevice,
	_In_ NTSTATUS  aReaderStatus);

/**	Handles returning the HID Descriptor to the given request.
 *
 *	@param [in] aDevice USB Device created by GCN_AdaptorCreateDevice.
 *	@param [in] aRequest Request received asking for the HID Descriptor.
 *
 *	@post The request is fullfilled if successful.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
NTSTATUS GCN_AdaptorGetHidDescriptor(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest);

/**	Handles returning the requested USB device attributes.
 *
 *	@param [in] aRequest Request received asking for USB device attributes.
 *
 *	@post The request is fullfilled if successful.*
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
NTSTATUS GCN_AdaptorGetDeviceAttributes(
	_In_ WDFREQUEST aRequest);

/**	Handles returning the HID Report Descriptor to the given request.
 *
 *	@param [in] aDevice USB Device created by GCN_AdaptorCreateDevice.
 *	@param [in] aRequest Request received asking for the HID Descriptor.
 *
 *	@post The request is fullfilled if successful.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
NTSTATUS GCN_AdaptorGetReportDescriptor(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest);

/**	Handles serving IOCTL requests.
 *
 *	@param [in] aQueue Queue associated with the I/O request.
 *	@param [in] aRequest Request received with IOCTL information.
 *	@param [in] aOutputBufferLength Length of the output buffer, if available.
 *	@param [in] aInputBufferLength Length of the input buffer, if available.
 *	@param [in] aIoControlCode IOCTL code received with request.
 *
 *	@post The request is fullfilled one way or another.
 *
 */
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL GCN_AdaptorEvtInternalDeviceControl;

#endif//_GCN_ADAPTER_IOCTL_H_
