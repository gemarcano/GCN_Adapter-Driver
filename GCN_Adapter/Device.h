#pragma once
#ifndef _GCN_ADAPTER_DEVICE_H_
#define _GCN_ADAPTER_DEVICE_H_

#include "Include.h"
#include "GCN_Adapter.h"
#include "GCN_Controller.h"

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

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

/** Initializes the device and creates the software resources it needs.
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

/** Helper that selects USB interface to use.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS SelectInterfaces(_In_ WDFDEVICE aDevice);

/**	Helper that sets the USB device's power behavior. Also sets up pipes.
 *	
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS GCN_AdapterSetPowerPolicy(_In_ WDFDEVICE Device);

/**	Prepares the USB device with PnP related settings.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
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
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
EVT_WDF_DEVICE_SELF_MANAGED_IO_FLUSH GCN_AdapterEvtDeviceSelfManagedIoFlush;

/**	Callback called when ?TODO. Sets up USB device for use. Reads and selects
 *	descriptors.
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

/**	Helper that sets the USB device's calibration values from current values.
 *
 *	@param [in] apDeviceContext Device context used by driver.
 *	@param [in] aIndex Index of port to calibrate [ 0, 3 ]. A -1 indicates to
 *		calibrate all ports.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
NTSTATUS GCN_AdapterFetchCalibrationData(PDEVICE_CONTEXT _In_ pDeviceContext, int aIndex);

#endif//_GCN_ADAPTER_DEVICE_H_
