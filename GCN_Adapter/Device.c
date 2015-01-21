#include "Include.h"
#include "device.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, GCN_AdapterCreateDevice)
#pragma alloc_text (PAGE, GCN_AdapterEvtDevicePrepareHardware)
#pragma alloc_text (PAGE, SelectInterfaces)
#pragma alloc_text (PAGE, GCN_AdapterSetPowerPolicy)
#endif

NTSTATUS GCN_AdapterCreateDevice(
	_Inout_ PWDFDEVICE_INIT aDeviceInit)
{
	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
	WDF_OBJECT_ATTRIBUTES   deviceAttributes;
	PDEVICE_CONTEXT deviceContext;
	WDFDEVICE device;
	NTSTATUS status;
	WDF_DEVICE_PNP_CAPABILITIES pnpCaps;
	UNICODE_STRING deviceInterface;

	PAGED_CODE();

	status = GCN_AdapterPnPInitialize(aDeviceInit);

	if (!NT_SUCCESS(status))
	{
		goto Error;
	}

	//Sets up PnP callback functions
	WdfDeviceInitSetIoType(aDeviceInit, WdfDeviceIoBuffered);

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

	status = WdfDeviceCreate(&aDeviceInit, &deviceAttributes, &device);

	if (!NT_SUCCESS(status))
	{
		goto Error;
	}

	deviceContext = DeviceGetContext(device);

	//It's fine if this device is unplugged at any time
	WDF_DEVICE_PNP_CAPABILITIES_INIT(&pnpCaps);
	pnpCaps.SurpriseRemovalOK = WdfTrue;
	WdfDeviceSetPnpCapabilities(device, &pnpCaps);

	//
	// Create a device interface so that applications can find and talk
	// to us.
	//
	/*status = WdfDeviceCreateDeviceInterface(
		device,
		&GUID_DEVINTERFACE_GCN_ADAPTER,
		NULL); // ReferenceString

	if (!NT_SUCCESS(status))
	{
		goto Error;
	}*/
	
	status = GCN_AdapterQueueInitialize(device);
	if (!NT_SUCCESS(status))
	{
		goto Error;
	}

	return status;

Error:
	//TODO Some tracing perhaps?
	return status;
}

NTSTATUS GCN_AdapterEvtDevicePrepareHardware(
	_In_ WDFDEVICE aDevice,
	_In_ WDFCMRESLIST aResourceList,
	_In_ WDFCMRESLIST aResourceListTranslated)
{
	NTSTATUS status;
	PDEVICE_CONTEXT pDeviceContext;
	WDF_USB_DEVICE_CREATE_CONFIG createParams;
	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;
	WDF_USB_DEVICE_INFORMATION deviceInfo;
	PUSB_DEVICE_DESCRIPTOR usbDeviceDescriptor = NULL;
	ULONG waitWakeEnable;
	WDF_MEMORY_DESCRIPTOR memoryDescriptor;
	WDF_OBJECT_ATTRIBUTES attributes;

	WDF_USB_CONTROL_SETUP_PACKET setup_packet;
	GCN_AdapterData calibrationData;
	WDF_TIMER_CONFIG timerConfig;

	UNREFERENCED_PARAMETER(aResourceList);
	UNREFERENCED_PARAMETER(aResourceListTranslated);

	waitWakeEnable = FALSE;

	PAGED_CODE();

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

	status = STATUS_SUCCESS;
	pDeviceContext = DeviceGetContext(aDevice);

	// Create a USB device handle if it doesn't already exist
	if (pDeviceContext->usbDevice == NULL) {

		//
		// Supposedly version of 602 enables newer capabilities of the USB driver
		// stack in Windows 8 and up? Not sure if we're using these or not, sticking
		// to the example code in this case until there's a reason not to do so.
		WDF_USB_DEVICE_CREATE_CONFIG_INIT(
			&createParams,
			USBD_CLIENT_CONTRACT_VERSION_602);

		status = WdfUsbTargetDeviceCreateWithParameters(
			aDevice,
			&createParams,
			WDF_NO_OBJECT_ATTRIBUTES,
			&pDeviceContext->usbDevice);

		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
						"WdfUsbTargetDeviceCreateWithParameters failed 0x%x", status);
			return status;
		}
	}

	WDF_USB_DEVICE_INFORMATION_INIT(&deviceInfo);

	status = WdfUsbTargetDeviceRetrieveInformation(pDeviceContext->usbDevice, &deviceInfo);

	if (NT_SUCCESS(status))
	{
		waitWakeEnable = deviceInfo.Traits & WDF_USB_DEVICE_TRAIT_REMOTE_WAKE_CAPABLE;
		pDeviceContext->usbDeviceTraits = deviceInfo.Traits;
	}
	else
	{
		pDeviceContext->usbDeviceTraits = 0;
	}

	status = SelectInterfaces(aDevice);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
					"SelectInterfaces failed 0x%x", status);
		return status;
	}

	WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
	attributes.ParentObject = aDevice;
	status = WdfMemoryCreate(&attributes, NonPagedPool, 0,
		sizeof(USB_DEVICE_DESCRIPTOR),
		&pDeviceContext->usbDeviceDescriptor,
		&usbDeviceDescriptor);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfMemoryCreate for Device Descriptor failed %!STATUS!\n",
			status);
		return status;
	}

	WdfUsbTargetDeviceGetDeviceDescriptor(
		pDeviceContext->usbDevice,
		usbDeviceDescriptor);

	//Send Commands to device to initialize it
	WDF_USB_CONTROL_SETUP_PACKET_INIT_CLASS(&setup_packet, BMREQUEST_HOST_TO_DEVICE, BMREQUEST_TO_INTERFACE, 11, 1, 0);
	status = WdfUsbTargetDeviceSendControlTransferSynchronously(pDeviceContext->usbDevice, NULL, NULL, &setup_packet, NULL, NULL);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfUsbTargetDeviceSendControlTransferSynchronously (SET_PROTOCOL) failed  %!STATUS!\n", status);
		return status;
	}

	WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memoryDescriptor, (PVOID)"\x13", 1);
	status = WdfUsbTargetPipeWriteSynchronously(pDeviceContext->interruptWritePipe, NULL, NULL, &memoryDescriptor, NULL);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfUsbTargetPipeWriteSynchronously failed (Init device) %!STATUS!\n", status);
		return status;
	}

	//Initialize spinlock for data synchronization
	WdfSpinLockCreate(&attributes, &pDeviceContext->dataLock); //Attributes is already initialized to use the device as the parent object

	//Fetch calibration data
	GCN_AdapterFetchCalibrationData(pDeviceContext, -1);

	//Initialize controller status
	GCN_Controller_Status_Init(&pDeviceContext->controllerStatus[0]);
	GCN_Controller_Status_Init(&pDeviceContext->controllerStatus[1]);
	GCN_Controller_Status_Init(&pDeviceContext->controllerStatus[2]);
	GCN_Controller_Status_Init(&pDeviceContext->controllerStatus[3]);

	//
	// Enable wait-wake and idle timeout if the device supports it (Filter HID does not support this?)
	//
	/*if (waitWakeEnable) {
		status = GCN_AdapterSetPowerPolicy(Device);
		if (!NT_SUCCESS(status)) {
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
				"GCN_AdapterSetPowerPolicy failed  %!STATUS!\n", status);
			return status;
		}
	}*/

	status = GCN_AdapterConfigContReaderForInterruptEndPoint(pDeviceContext);

	GCN_Adapter_CreateRawPdo(aDevice, 0);

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

	return status;
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS SelectInterfaces(
	_In_ WDFDEVICE aDevice)
{
	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS configParams;
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_CONTEXT pDeviceContext;
	WDFUSBPIPE pipe;
	WDF_USB_PIPE_INFORMATION pipeInfo;
	UCHAR index, numberConfiguredPipes;

	PAGED_CODE();

	pDeviceContext = DeviceGetContext(aDevice);

	WDF_USB_DEVICE_SELECT_CONFIG_PARAMS_INIT_SINGLE_INTERFACE(&configParams);

	status = WdfUsbTargetDeviceSelectConfig(pDeviceContext->usbDevice,
		WDF_NO_OBJECT_ATTRIBUTES,
		&configParams);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfUsbTargetDeviceSelectConfig failed %!STATUS! \n",
			status);
		return status;
	}

	pDeviceContext->usbInterface =
		configParams.Types.SingleInterface.ConfiguredUsbInterface;

	numberConfiguredPipes = configParams.Types.SingleInterface.NumberConfiguredPipes;

	//Get pipes and store them in context
	for (index = 0; index < numberConfiguredPipes; index++)
	{
		WDF_USB_PIPE_INFORMATION_INIT(&pipeInfo);

		pipe = WdfUsbInterfaceGetConfiguredPipe(
			pDeviceContext->usbInterface,
			index, //PipeIndex,
			&pipeInfo
			);

		if (WdfUsbPipeTypeInterrupt == pipeInfo.PipeType
			&& WdfUsbTargetPipeIsInEndpoint(pipe))
		{
			TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE,
				"Interrupt Read Pipe is 0x%p\n", pipe);
			pDeviceContext->interruptReadPipe = pipe;
		}

		if (WdfUsbPipeTypeInterrupt == pipeInfo.PipeType &&
			WdfUsbTargetPipeIsOutEndpoint(pipe))
		{
			TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE,
				"Interrupt Write Pipe is 0x%p\n", pipe);
			pDeviceContext->interruptWritePipe = pipe;
		}
	}

	//
	// If we didn't find all the 2 pipes, fail the start.
	//
	if (!(pDeviceContext->interruptWritePipe
		&& pDeviceContext->interruptReadPipe))
	{
		status = STATUS_INVALID_DEVICE_STATE;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"Device is not configured properly %!STATUS!\n",
			status);

		return status;
	}

	return status;
}

NTSTATUS GCN_AdapterPnPInitialize(
	_In_ PWDFDEVICE_INIT aDeviceInit)
{
	NTSTATUS status = STATUS_SUCCESS;
	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;

	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
	pnpPowerCallbacks.EvtDevicePrepareHardware = GCN_AdapterEvtDevicePrepareHardware;
	
	pnpPowerCallbacks.EvtDeviceD0Entry = GCN_AdapterEvtDeviceD0Entry;
	pnpPowerCallbacks.EvtDeviceD0Exit = GCN_AdapterEvtDeviceD0Exit;
	pnpPowerCallbacks.EvtDeviceSelfManagedIoFlush = GCN_AdapterEvtDeviceSelfManagedIoFlush;

	WdfDeviceInitSetPnpPowerEventCallbacks(aDeviceInit, &pnpPowerCallbacks);

	return status;
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS GCN_AdapterSetPowerPolicy(
	_In_ WDFDEVICE aDevice)
{
	WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS idleSettings;
	WDF_DEVICE_POWER_POLICY_WAKE_SETTINGS wakeSettings;
	NTSTATUS    status = STATUS_SUCCESS;

	PAGED_CODE();

	WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(&idleSettings, IdleUsbSelectiveSuspend);
	idleSettings.IdleTimeout = 10000; //in milliseconds

	status = WdfDeviceAssignS0IdleSettings(aDevice, &idleSettings);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfDeviceSetPowerPolicyS0IdlePolicy failed %x\n", status);
		return status;
	}

	WDF_DEVICE_POWER_POLICY_WAKE_SETTINGS_INIT(&wakeSettings);

	status = WdfDeviceAssignSxWakeSettings(aDevice, &wakeSettings);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfDeviceAssignSxWakeSettings failed %x\n", status);
		return status;
	}

	return status;
}

VOID GCN_AdapterEvtDeviceSelfManagedIoFlush(
	_In_ WDFDEVICE aDevice)
{
	GCN_AdapterUsbIoctlGetInterruptMessage(aDevice, STATUS_DEVICE_REMOVED);
}

NTSTATUS GCN_AdapterFetchCalibrationData(PDEVICE_CONTEXT _In_ apDeviceContext, int _In_ aIndex)
{
	NTSTATUS status;	
	GCN_AdapterData calibrationData;

	WdfSpinLockAcquire(apDeviceContext->dataLock);
	calibrationData = apDeviceContext->adaptorData;
	WdfSpinLockRelease(apDeviceContext->dataLock);

	if (aIndex < 0)
	{
		apDeviceContext->calibrationData = calibrationData;
	}
	else if (aIndex < 4)
	{
		apDeviceContext->calibrationData.Port[aIndex] = calibrationData.Port[aIndex];
	}
	else
	{
		status = STATUS_BUFFER_OVERFLOW;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"!FUNC! bad index received! %!STATUS!\n", status);
	}

	return status;
}
