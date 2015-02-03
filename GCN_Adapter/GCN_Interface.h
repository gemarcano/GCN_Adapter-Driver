#pragma once
#ifndef _GCN_INTERFACE_H_
#define _GCN_INTERFACE_H_

/**	@file
 *	This file contains function and variable declarations/definitions regarding
 *	the device interface the driver exposes.
 *
 */

#include "Include.h"

//TODO Support multiple interfaces!!!
#define GCN_ADAPTER_DEVICE_ID L"USB\\VID_057E&PID_0337&REV_0100\0"

typedef struct _INT_DEVICE_CONTEXT
{
	//Device State
	ULONG id;

	//Driver State
	WDFQUEUE writeQueue, readQueue, ioctlQueue;

} INT_DEVICE_CONTEXT, *PINT_DEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(INT_DEVICE_CONTEXT, IntDeviceGetContext)

/**	Creates a raw physical device interface in order to access the driver
 *		from user-land applications.
 *	
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *	@param [in] aInstanceNo Number of the new device instance.
 *	
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
NTSTATUS GCN_Adapter_CreateRawPdo(
	WDFDEVICE       aDevice,
	ULONG           aInstanceNo);

#endif//_GCN_INTERFACE_H_