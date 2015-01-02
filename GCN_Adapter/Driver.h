#pragma once
#ifndef _GCN_ADAPTER_DRIVER_H_
#define _GCN_ADAPTER_DRIVER_H_

#define INITGUID

#include "Include.h"

/**	Loads driver. Called when OS loads driver into memory.
 *
 *	@param [in] aDriverObject Represents driver being loaded into memory.
 *	@param [in] aRegistryPath Represents the driver's path in the Registry.
 *
 *	@returns STATUS_SUCCESS if successful, STATUS_UNSUCCESSFUL on any failure.
 *
 */
DRIVER_INITIALIZE DriverEntry;

/**	Creates and initializes a Device object. Called when PnP manager requests to
 *	have a new device added. 
 *
 *	@param [in] aDriver Driver object made in DriverEntry.
 *	@param [in,out] aDeviceInit Pointer to device initialization structure
 *		provided by the framework.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
EVT_WDF_DRIVER_DEVICE_ADD GCN_AdaptorEvtDeviceAdd;

/**	Frees all the resources allocated in DriverEntry. Called when driver is
 *	being unloaded from memory.
 *
 *	@param [in] aDriver Driver object made in DriverEntry.
 *	@param [in,out] aDeviceInit Pointer to device initialization structure
 *		provided by the framework.
 *
 */
EVT_WDF_OBJECT_CONTEXT_CLEANUP GCN_AdaptorEvtDriverContextCleanup;

#endif//_GCN_ADAPTER_DRIVER_H_
