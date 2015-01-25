#pragma once
#ifndef _GCN_ADAPTER_DEVICE_H_
#define _GCN_ADAPTER_DEVICE_H_

/**	@file
 *	This file contains function and variable declarations/definitions regarding
 *	device specific functionality, such as initialization and teardown.
 *
 */

#include "Include.h"
#include "GCN_Adapter.h"
#include "GCN_Controller.h"

//TODO Support more than one device!

typedef struct _DEVICE_CONTEXT
{
	//USB Level
    WDFUSBDEVICE usbDevice;
	WDFUSBINTERFACE usbInterface;
	WDFMEMORY usbDeviceDescriptor;
	ULONG usbDeviceTraits;
	WDFUSBPIPE interruptReadPipe;
	WDFUSBPIPE interruptWritePipe;
	
	//Driver Level
	WDFQUEUE interruptMsgQueue;
	WDFQUEUE readQueue;
	WDFQUEUE writeQueue;
	WDFQUEUE otherQueue;

	//Device Level
	WDFSPINLOCK dataLock; //for synchronizing device level access
	GCN_AdapterData calibrationData;
	GCN_AdapterData adapterData; //Latest data read in
	GCN_Controller_Status controllerStatus[4]; //Latest controller state
	WDFREQUEST rumbleRequest; //async data to enable rumble
	WDFMEMORY rumbleMemory;
	BYTE rumbleStatus; //Convenient cache of rumble status

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

/** Initializes the device and creates the software resources it needs.
 *
 *	This function initializes a new device. This is called from the registered
 *	function GCN_AdapterEvtDriverDeviceAdd. It initializes the context structure
 *	for the new device, prepares the PNP callback functions, and leaves the
 *	device data structures ready for hardware initialization.
 *
 *	@remark This function runs at PASSIVE_LEVEL since it is called from
 *		GCN_AdapterEvtDriverDeviceAdd.
 *	@remark This function is paged in page PAGE.
 *
 *	@param [in,out] aDeviceInit Pointer to an opaque init structure. Memory for
 *		this structure will be freed by the framework when the WdfDeviceCreate
 *		succeeds. So don't access the structure after that point.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
NTSTATUS GCN_AdapterCreateDevice(
	_Inout_ PWDFDEVICE_INIT aDeviceInit);

/**	Callback called when hardware resources are ready for use. Sets up USB
 *	device for use. Reads and selects descriptors.
 *
 *	This function is called by the PnP system when it has allocated hardware
 *	resources for use by the driver. The PnP system leaves the hardware in an
 *	uninitialized D0 power state. This function readies the USB device by 
 *	sending a special init sequence, and prepares the driver to stream data
 *	from the adapter. USB endpoints are also configured.
 *
 *	@remark This function runs at PASSIVE_LEVEL.
 *	@remark This function is paged in page PAGE.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *	@param [in] aResourceList ?TODO
 *	@param [in] aResourceListTranslated ?TODO
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
EVT_WDF_DEVICE_PREPARE_HARDWARE GCN_AdapterEvtDevicePrepareHardware;

/** Helper that selects USB interface to use. Used by
 *	GCN_AdapterEvtDevicePrepareHardware.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *
 *	@remark This function runs at PASSIVE_LEVEL.
 *	@remark This function is paged in page PAGE.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
NTSTATUS SelectInterfaces(_In_ WDFDEVICE aDevice);

/**	Prepares the USB device with PnP related settings.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *
 *	@remark This function runs at PASSIVE_LEVEL since it is called from
 *		GCN_AdapterCreateDevice.
 *	@remark This function is paged in page PAGE.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
NTSTATUS GCN_AdapterPnPInitialize(_In_ PWDFDEVICE_INIT device);

/**	Callback called when ?TODO. Flushes the activity for the device's
 *	self-managed I/O operations. (FIXME I believe the current implementation is
 *	incomplete. It does not actually flush the contents of the self-managed
 *	io-queue)
 *
 *	@remark This function runs at PASSIVE_LEVEL.
 *	@remark This function is paged in page PAGE.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
EVT_WDF_DEVICE_SELF_MANAGED_IO_FLUSH GCN_AdapterEvtDeviceSelfManagedIoFlush;

/**	Initializes driver's queues.
 *
 *	This function is called by GCN_AdapterCreateDevice to set up all the
 *	queues the driver uses for receiving requests. The driver has one general
 *	queue where it received IOCTL requests (all else is dropped?), a read queue,
 *	a write queue, (anything else?).
 *
 *	@remark This function runs at PASSIVE_LEVEL since it is called by
 *		GCN_AdapterCreateDevice.
 *	@remark This function is paged in page PAGE.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
NTSTATUS GCN_AdapterQueueInitialize(_In_ WDFDEVICE aDevice);

#endif//_GCN_ADAPTER_DEVICE_H_
