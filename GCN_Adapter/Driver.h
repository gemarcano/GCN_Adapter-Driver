#pragma once
#ifndef _GCN_ADAPTER_DRIVER_H_
#define _GCN_ADAPTER_DRIVER_H_

/**	@file
 *	This file contains function and variable declarations/definitions regarding
 *	driver specific functionality, such as initialization and teardown.
 *
 */

#define INITGUID

#include "Include.h"

/**	Loads driver. Called when OS loads driver into memory.
 *
 *	This function is called only once, at the moment the operating system
 *	initializes the driver. No device setup is done yet.
 *
 *	@remark This function runs at PASSIVE_LEVEL.
 *	@remark This function is not paged.
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
 *	This function is called every time the framework detects a new PNP device
 *	matching the information for this driver. Device information and state are
 *	initialized here, and anything that needs to happen per new device as well.
 *
 *	@remark This function runs at PASSIVE_LEVEL.
 *	@remark This function is paged in page PAGE.
 *
 *	@param [in] aDriver Driver object made in DriverEntry.
 *	@param [in,out] aDeviceInit Pointer to device initialization structure
 *		provided by the framework.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
EVT_WDF_DRIVER_DEVICE_ADD GCN_AdapterEvtDriverDeviceAdd;

/**	Frees all the resources allocated in DriverEntry. Called when driver is
 *	being unloaded from memory.
 *
 *	This function is called by the framework when the driver is being unloaded
 *	from memory, due to failed initialization or all devices for the driver
 *	having been removed. Unloads driver resources.
 *
 *	@remark This function runs at PASSIVE_LEVEL since the object in question is
 *		a WDFDRIVER object.
 *	@remark This function is paged in page PAGE.
 *
 *	@param [in] aDriver Driver object made in DriverEntry.
 *	@param [in,out] aDeviceInit Pointer to device initialization structure
 *		provided by the framework.
 *
 */
EVT_WDF_OBJECT_CONTEXT_CLEANUP GCN_AdapterEvtDriverContextCleanup;

#endif//_GCN_ADAPTER_DRIVER_H_
