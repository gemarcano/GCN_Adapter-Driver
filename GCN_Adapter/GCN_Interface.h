#pragma once
#ifndef _GCN_INTERFACE_H_
#define _GCN_INTERFACE_H_

/**	@file
 *	This file contains function and variable declarations/definitions regarding
 *	the device interface the driver exposes.
 *
 */

#include "Include.h"

#define GCN_ADAPTER_DEVICE_ID L"USB\\VID_057E&PID_0337&REV_0100\0"

typedef struct _INT_DEVICE_CONTEXT
{
	//Device State
	ULONG id;

	//Driver State
	WDFQUEUE writeQueue, readQueue, ioctlQueue;

} INT_DEVICE_CONTEXT, *PINT_DEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(INT_DEVICE_CONTEXT, IntDeviceGetContext)

NTSTATUS GCN_Adapter_CreateRawPdo(
	WDFDEVICE       Device,
	ULONG           InstanceNo);

#endif//_GCN_INTERFACE_H_