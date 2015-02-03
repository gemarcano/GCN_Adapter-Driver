#include "Include.h"
#include "gcn_interface.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, GCN_Adapter_CreateRawPdo)
#endif

_IRQL_requires_(PASSIVE_LEVEL)
EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL GCN_Adapter_EvtIoDeviceControlForRawPdo;

_Use_decl_annotations_
VOID GCN_Adapter_EvtIoDeviceControlForRawPdo(
	WDFQUEUE      Queue,
	WDFREQUEST    Request,
	size_t        OutputBufferLength,
	size_t        InputBufferLength,
	ULONG         IoControlCode)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	WDFDEVICE parent = WdfIoQueueGetDevice(Queue);
	PINT_DEVICE_CONTEXT pdoData;
	WDF_REQUEST_FORWARD_OPTIONS forwardOptions;

	pdoData = IntDeviceGetContext(parent);

	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_GCN_INTERFACE, "%!FUNC! Entry");

	//
	// Process the ioctl and complete it when you are done.
	// Since the queue is configured for serial dispatch, you will
	// not receive another ioctl request until you complete this one.
	//

	switch (IoControlCode) {
	case IOCTL_GCN_ADAPTER_CALIBRATE:
	case IOCTL_GCN_ADAPTER_SET_DEADZONE:
	case IOCTL_GCN_ADAPTER_SET_RUMBLE:
	case IOCTL_GCN_ADAPTER_GET_DEADZONE:
		WDF_REQUEST_FORWARD_OPTIONS_INIT(&forwardOptions);
		status = WdfRequestForwardToParentDeviceIoQueue(
			Request, pdoData->ioctlQueue, &forwardOptions);
		if (NT_SUCCESS(status)) {
			break;
		}
	
		//If it failed, fall through and let this function complete the request
	default:
		WdfRequestComplete(Request, status);
		break;
	}

	return;
}

#define MAX_ID_LEN 128

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS GCN_Adapter_CreateRawPdo(
	_In_ WDFDEVICE aDevice,
	_In_ ULONG aInstanceNo)
{
	NTSTATUS status;
	PWDFDEVICE_INIT pDeviceInit = NULL;
	PINT_DEVICE_CONTEXT pdoData = NULL;
	WDFDEVICE hChild = NULL;
	WDF_OBJECT_ATTRIBUTES pdoAttributes;
	WDF_DEVICE_PNP_CAPABILITIES pnpCaps;
	WDF_IO_QUEUE_CONFIG ioQueueConfig;
	WDFQUEUE queue;
	WDF_DEVICE_STATE deviceState;
	PDEVICE_CONTEXT devExt;
	DECLARE_CONST_UNICODE_STRING(deviceId, GCN_ADAPTER_DEVICE_ID);
	DECLARE_CONST_UNICODE_STRING(hardwareId, GCN_ADAPTER_DEVICE_ID);
	DECLARE_CONST_UNICODE_STRING(deviceLocation, L"GCN_Adapter\0");
	DECLARE_UNICODE_STRING_SIZE(buffer, MAX_ID_LEN);

	PAGED_CODE();

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_GCN_INTERFACE, "%!FUNC! Entry");

	//
	// Allocate a WDFDEVICE_INIT structure and set the properties
	// so that we can create a device object for the child.
	//
	pDeviceInit = WdfPdoInitAllocate(aDevice);

	if (pDeviceInit == NULL) {
		status = STATUS_INSUFFICIENT_RESOURCES;
		goto Cleanup;
	}

	//
	// Mark the device RAW so that the child device can be started
	// and accessed without requiring a function driver. Since we are
	// creating a RAW PDO, we must provide a class guid.
	//
	status = WdfPdoInitAssignRawDevice(pDeviceInit, &GUID_DEVCLASS_UNKNOWN);
	if (!NT_SUCCESS(status)) {
		goto Cleanup;
	}

	//
	// Since keyboard is secure device, we must protect ourselves from random
	// users sending ioctls and creating trouble.
	//
	/*status = WdfDeviceInitAssignSDDLString(pDeviceInit,
		&SDDL_DEVOBJ_SYS_ALL_ADM_ALL);
	if (!NT_SUCCESS(status)) {
		goto Cleanup;
	}*/

	//
	// Assign DeviceID -
	//	This will be reported to IRP_MN_QUERY_ID/BusQueryDeviceID
	//
	status = WdfPdoInitAssignDeviceID(pDeviceInit, &deviceId);
	if (!NT_SUCCESS(status)) {
		goto Cleanup;
	}

	//
	// For RAW PDO, there is no need to provide BusQueryHardwareIDs
	// and BusQueryCompatibleIDs IDs unless we are running on
	// Windows 2000.
	//
	if (!RtlIsNtDdiVersionAvailable(NTDDI_WINXP)) {
		//
		// On Win2K, we must provide a HWID for the device to get enumerated.
		// Since we are providing a HWID, we will have to provide a NULL inf
		// to avoid the "found new device" popup and get the device installed
		// silently.
		//
		status = WdfPdoInitAddHardwareID(pDeviceInit, &hardwareId);
		if (!NT_SUCCESS(status)) {
			goto Cleanup;
		}
	}

	//
	// We could be enumerating more than one children if the filter attaches
	// to multiple instances of keyboard, so we must provide a
	// BusQueryInstanceID. If we don't, system will throw CA bugcheck.
	//
	status = RtlUnicodeStringPrintf(&buffer, L"%02d", aInstanceNo);
	if (!NT_SUCCESS(status)) {
		goto Cleanup;
	}

	status = WdfPdoInitAssignInstanceID(pDeviceInit, &buffer);
	if (!NT_SUCCESS(status)) {
		goto Cleanup;
	}

	//
	// Provide a description about the device. This text is usually read from
	// the device. In the case of USB device, this text comes from the string
	// descriptor. This text is displayed momentarily by the PnP manager while
	// it's looking for a matching INF. If it finds one, it uses the Device
	// Description from the INF file to display in the device manager.
	// Since our device is raw device and we don't provide any hardware ID
	// to match with an INF, this text will be displayed in the device manager.
	//
	status = RtlUnicodeStringPrintf(&buffer, L"GCN_Adapter_%02d", aInstanceNo);
	if (!NT_SUCCESS(status)) {
		goto Cleanup;
	}

	//
	// You can call WdfPdoInitAddDeviceText multiple times, adding device
	// text for multiple locales. When the system displays the text, it
	// chooses the text that matches the current locale, if available.
	// Otherwise it will use the string for the default locale.
	// The driver can specify the driver's default locale by calling
	// WdfPdoInitSetDefaultLocale.
	//
	status = WdfPdoInitAddDeviceText(pDeviceInit,
		&buffer,
		&deviceLocation,
		0x409
		);
	if (!NT_SUCCESS(status)) {
		goto Cleanup;
	}

	WdfPdoInitSetDefaultLocale(pDeviceInit, 0x409);

	//
	// Initialize the attributes to specify the size of PDO device extension.
	// All the state information private to the PDO will be tracked here.
	//
	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&pdoAttributes, INT_DEVICE_CONTEXT);

	//
	// Set up our queue to allow forwarding of requests to the parent
	// This is done so that the cached Keyboard Attributes can be retrieved
	//
	WdfPdoInitAllowForwardingRequestToParent(pDeviceInit);

	status = WdfDeviceCreate(&pDeviceInit, &pdoAttributes, &hChild);
	if (!NT_SUCCESS(status))
	{
		hChild = NULL;
		goto Cleanup;
	}

	//
	// Get the device context.
	//
	pdoData = IntDeviceGetContext(hChild);

	pdoData->id = aInstanceNo;

	//
	// Get the parent queue we will be forwarding to
	//
	devExt = DeviceGetContext(aDevice);
	pdoData->ioctlQueue = devExt->otherQueue;

	//
	// Configure the default queue associated with the control device object
	// to be Serial so that request passed to EvtIoDeviceControl are serialized.
	// A default queue gets all the requests that are not
	// configure-fowarded using WdfDeviceConfigureRequestDispatching.
	//

	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,
		WdfIoQueueDispatchSequential);

	ioQueueConfig.EvtIoDeviceControl = GCN_Adapter_EvtIoDeviceControlForRawPdo;

	status = WdfIoQueueCreate(hChild,
		&ioQueueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&queue // pointer to default queue
		);
	if (!NT_SUCCESS(status)) {
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_GCN_INTERFACE,
			"WdfIoQueueCreate failed 0x%x\n", status);
		goto Cleanup;
	}

	//
	// Set some properties for the child device.
	//
	WDF_DEVICE_PNP_CAPABILITIES_INIT(&pnpCaps);

	pnpCaps.Removable = WdfTrue;
	pnpCaps.SurpriseRemovalOK = WdfTrue;
	pnpCaps.NoDisplayInUI = WdfTrue;

	pnpCaps.Address = aInstanceNo;
	pnpCaps.UINumber = aInstanceNo;

	WdfDeviceSetPnpCapabilities(hChild, &pnpCaps);

	//
	// TODO??? It seems the example is incomplete...
	//
	// In addition to setting NoDisplayInUI in DeviceCaps, we
	// have to do the following to hide the device. Following call
	// tells the framework to report the device state in
	// IRP_MN_QUERY_DEVICE_STATE request.
	//
	WDF_DEVICE_STATE_INIT(&deviceState);
	deviceState.DontDisplayInUI = WdfTrue;
	WdfDeviceSetDeviceState(hChild, &deviceState);

	//
	// Tell the Framework that this device will need an interface so that
	// application can find our device and talk to it.
	//
	status = WdfDeviceCreateDeviceInterface(
		hChild,
		&GUID_DEVINTERFACE_GCN_ADAPTER,
		NULL
		);

	if (!NT_SUCCESS(status)) {
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_GCN_INTERFACE,
			"WdfDeviceCreateDeviceInterface failed 0x%x\n", status);
		goto Cleanup;
	}

	//
	// Add this device to the FDO's collection of children.
	// After the child device is added to the static collection successfully,
	// driver must call WdfPdoMarkMissing to get the device deleted. It
	// shouldn't delete the child device directly by calling WdfObjectDelete.
	//
	status = WdfFdoAddStaticChild(aDevice, hChild);
	if (!NT_SUCCESS(status)) {
		goto Cleanup;
	}

	//
	// pDeviceInit will be freed by WDF.
	//
	return STATUS_SUCCESS;

Cleanup:

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_GCN_INTERFACE,
		"%!FUNC! Failed: %!STATUS!", status); 

	//
	// Call WdfDeviceInitFree if you encounter an error while initializing
	// a new framework device object. If you call WdfDeviceInitFree,
	// do not call WdfDeviceCreate.
	//
	if (pDeviceInit != NULL) {
		WdfDeviceInitFree(pDeviceInit);
	}

	if (hChild != NULL) {
		WdfObjectDelete(hChild);
	}

	return status;
}
