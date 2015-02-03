#pragma once
#ifndef _GCN_ADAPTER_IOCTL_H_
#define _GCN_ADAPTER_IOCTL_H_

/**	@file
*	This file contains function and variable declarations/definitions regarding
*	IOCTL handling.
*
*/

#include "Include.h"

/**	Handles the completion of pended HID READ_REPORT IOCTL requests.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice. 
 *	
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL by the framework.
 * 	@remark This function is not paged.
 *
 *	@returns STATUS_SUCCESS if a message was serviced, STATUS_NO_MORE_ENTRIES if
 *		there is nothing else to process. Any other NTSTATUS represents an
 *		error. @See http://msdn.microsoft.com/en-us/library/cc704588.aspx for
 *		details.
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS GCN_AdapterIoctlHIDReadReportHandler(_In_ WDFDEVICE aDevice);

/**	Handles returning the HID Descriptor to the given request.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *	@param [in] aRequest Request received asking for the HID Descriptor.
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL since it is called by
 *		GCN_AdapterIoctlHIDReadReportHandler.
 * 	@remark This function is not paged.
 *
 *	@post The request is fullfilled if successful.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS GCN_AdapterGetHidDescriptor(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest);

/**	Handles returning the requested USB device attributes.
 *
 *	@param [in] aRequest Request received asking for USB device attributes.
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL since it is called by
 *		GCN_AdapterIoctlHIDReadReportHandler.
 * 	@remark This function is not paged.
 *
 *	@post The request is fullfilled if successful.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS GCN_AdapterGetDeviceAttributes(
	_In_ WDFREQUEST aRequest);

/**	Handles returning the HID Report Descriptor to the given request.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *	@param [in] aRequest Request received asking for the HID Descriptor.
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL since it is called by
 *		GCN_AdapterIoctlHIDReadReportHandler.
 * 	@remark This function is not paged.
 *
 *	@post The request is fullfilled if successful.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS GCN_AdapterGetReportDescriptor(
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
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL since it is called by
 *		GCN_AdapterIoctlHIDReadReportHandler.
 * 	@remark This function is not paged.
 *
 *	@post The request is fullfilled one way or another.
 *
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
EVT_WDF_IO_QUEUE_IO_INTERNAL_DEVICE_CONTROL GCN_AdapterEvtInternalDeviceControl;

/**	Handles serving IOCTL requests.
*
*	@param [in] aQueue Queue associated with the I/O request.
*	@param [in] aRequest Request received with IOCTL information.
*	@param [in] aOutputBufferLength Length of the output buffer, if available.
*	@param [in] aInputBufferLength Length of the input buffer, if available.
*	@param [in] aIoControlCode IOCTL code received with request.
*
*	@remark This function runs at IRQL <= DISPATCH_LEVEL since it is called by
*		GCN_AdapterIoctlHIDReadReportHandler.
* 	@remark This function is not paged.
*
*	@post The request is fullfilled one way or another.
*
*/
_IRQL_requires_max_(DISPATCH_LEVEL)
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL GCN_AdapterEvtDeviceControl;

/**	Passes down Idle notification request to the lower driver.
 *
 *	@param [in] aRequest Request received with IOCTL information.
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL since it is called by
 *		GCN_AdapterIoctlHIDReadReportHandler.
 * 	@remark This function is not paged.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS GCN_AdapterSendIdleNotification(_In_ WDFREQUEST aRequest);


/**	Calibrates the adapter based on the current values read.
 *
 *	@param [in] aDevice Device with calibration data to get.
 *	@param [in] aRequest Request received with IOCTL information.
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL since it is called by
 *		GCN_AdapterIoctlHIDReadReportHandler.
 * 	@remark This function is not paged.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS GCN_AdapterCalibrate(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest);

/**	Sets the deadzone parameters for the device.
 *
 *	@param [in] aDevice Device to modify.
 *	@param [in] aRequest Request received with IOCTL information.
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL since it is called by
 *		GCN_AdapterIoctlHIDReadReportHandler.
 * 	@remark This function is not paged.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS GCN_AdapterSetDeadzone(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest);

/**	Gets the current deadzone sensitivity settings for the device.
 *
 *	@param [in] aDevice Device to query.
 *	@param [in] aRequest Request received with IOCTL information.
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL since it is called by
 *		GCN_AdapterIoctlHIDReadReportHandler.
 * 	@remark This function is not paged.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS GCN_AdapterGetSensitivity(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest);

/**	Sets the rumble status of the controllers attached to the device.
 *
 *	@param [in] aDevice Device to modify.
 *	@param [in] aRequest Request received with IOCTL information.
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL since it is called by
 *		GCN_AdapterIoctlHIDReadReportHandler.
 * 	@remark This function is not paged.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
NTSTATUS GCN_AdapterSetRumble(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest);

#endif//_GCN_ADAPTER_IOCTL_H_
