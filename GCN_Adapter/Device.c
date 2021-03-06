#include "Include.h"
#include "device.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, GCN_AdapterCreateDevice)
#pragma alloc_text (PAGE, GCN_AdapterEvtDevicePrepareHardware)
#pragma alloc_text (PAGE, SelectInterfaces)
#pragma alloc_text (PAGE, GCN_AdapterPnPInitialize)
#pragma alloc_text (PAGE, GCN_AdapterQueueInitialize)
#pragma alloc_text (PAGE, GCN_AdapterEvtDeviceSelfManagedIoFlush)
#endif

 _IRQL_requires_(PASSIVE_LEVEL)
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

	WDF_OBJECT_ATTRIBUTES_INIT(&deviceAttributes);
	deviceAttributes.ParentObject = device;

	//It's fine if this device is unplugged at any time
	WDF_DEVICE_PNP_CAPABILITIES_INIT(&pnpCaps);
	pnpCaps.SurpriseRemovalOK = WdfTrue;
	WdfDeviceSetPnpCapabilities(device, &pnpCaps);
	
	status = GCN_AdapterQueueInitialize(device);
	if (!NT_SUCCESS(status))
	{
		goto Error;
	}

	//Initialize spinlock for data synchronization
	//Attributes are already initialized to use the device as the parent object
	status = WdfSpinLockCreate(&deviceAttributes, &deviceContext->dataLock);
	if (!NT_SUCCESS(status))
	{
		goto Error;
	}

	//Initialize controller status
	GCN_Controller_Status_Init(&deviceContext->controllerStatus[0]);
	GCN_Controller_Status_Init(&deviceContext->controllerStatus[1]);
	GCN_Controller_Status_Init(&deviceContext->controllerStatus[2]);
	GCN_Controller_Status_Init(&deviceContext->controllerStatus[3]);

	//Initialize rumble support data
	status = WdfRequestCreate(
		&deviceAttributes, NULL, &deviceContext->rumbleRequest);

	if (!NT_SUCCESS(status))
	{
		goto Error;
	}

	status = WdfMemoryCreate(
		&deviceAttributes,
		NonPagedPool,
		0,
		5,
		&deviceContext->rumbleMemory,
		NULL);
	
	if (!NT_SUCCESS(status))
	{
		goto Error;
	}

	((BYTE*)(WdfMemoryGetBuffer(deviceContext->rumbleMemory, NULL)))[0] = 0x11;
	return status;

Error:
	TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
		"%!FUNC! failed with status %!STATUS!", status);
	return status;
}

 _Use_decl_annotations_
NTSTATUS GCN_AdapterEvtDevicePrepareHardware(
	WDFDEVICE aDevice,
	WDFCMRESLIST aResourceList,
	WDFCMRESLIST aResourceListTranslated)
{
	NTSTATUS status;
	PDEVICE_CONTEXT pDeviceContext;
	WDF_USB_DEVICE_CREATE_CONFIG createParams;
	WDF_USB_DEVICE_INFORMATION deviceInfo;
	PUSB_DEVICE_DESCRIPTOR usbDeviceDescriptor = NULL;
	WDFMEMORY memory;
	WDF_MEMORY_DESCRIPTOR memoryDescriptor;
	WDF_OBJECT_ATTRIBUTES attributes;

	WDF_USB_CONTROL_SETUP_PACKET setup_packet;
	GCN_AdapterData calibrationData;
	WDF_TIMER_CONFIG timerConfig;

	UNREFERENCED_PARAMETER(aResourceList);
	UNREFERENCED_PARAMETER(aResourceListTranslated);

	PAGED_CODE();

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Entry");

	status = STATUS_SUCCESS;
	pDeviceContext = DeviceGetContext(aDevice);

	// Create a USB device handle if it doesn't already exist
	if (pDeviceContext->usbDevice == NULL) {

		//Supposedly version of 602 enables newer capabilities of the USB driver
		//stack in Windows 8 and up? Not sure if we're using these or not,
		//sticking to the example code in this case until there's a reason not
		//to do so.
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
			TraceEvents(
				TRACE_LEVEL_ERROR,
				TRACE_DEVICE, 
				"WdfUsbTargetDeviceCreateWithParameters failed 0x%x",
				status);

			return status;
		}
	}

	WDF_USB_DEVICE_INFORMATION_INIT(&deviceInfo);

	status = WdfUsbTargetDeviceRetrieveInformation(
		pDeviceContext->usbDevice, &deviceInfo);

	if (NT_SUCCESS(status))
	{
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
	WDF_USB_CONTROL_SETUP_PACKET_INIT_CLASS(
		&setup_packet,
		BMREQUEST_HOST_TO_DEVICE,
		BMREQUEST_TO_INTERFACE,
		11,
		1,
		0);

	status = WdfUsbTargetDeviceSendControlTransferSynchronously(
		pDeviceContext->usbDevice, NULL, NULL, &setup_packet, NULL, NULL);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfUsbTargetDeviceSendControlTransferSynchronously"
			" (SET_PROTOCOL) failed  %!STATUS!\n", status);
		return status;
	}

	status = WdfMemoryCreate(NULL, NonPagedPool, 0, 1, &memory, NULL);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfMemoryCreate for initialization failed %!STATUS!\n",
			status);
		return status;
	}

	/*	I'm using a WDFMEMORY object here instead of just using a normal
		descriptor with a static buffer because it seems that using memory that
		can be paged here can cause problems if it is paged. I experienced some
		problems where USBPORT.SYS called some functions at IRQL DISPATCH, and
		none of the data declared in this function is in non-paged memory,
		which triggered	a page fault. I think this is a Microsoft bug, but I
		don't want to deal with it. Using a non-paged pool memory object works
		without problems.
	 */
	status = WdfMemoryCopyFromBuffer(memory, 0, &"\x13", 1);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfMemoryCopyFromBuffer failed %!STATUS!\n", status);
		return status;
	}

	WDF_MEMORY_DESCRIPTOR_INIT_HANDLE(&memoryDescriptor,
		memory,
		NULL);
	
	status = WdfUsbTargetPipeWriteSynchronously(
		pDeviceContext->interruptWritePipe,
		NULL,
		NULL,
		&memoryDescriptor,
		NULL);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfUsbTargetPipeWriteSynchronously failed %!STATUS!\n", status);
		return status;
	}
	
	//Fetch calibration data
	GCN_Controller_Calibrate(pDeviceContext, -1);

	status = GCN_AdapterConfigContReaderForInterruptEndPoint(pDeviceContext);

	GCN_Adapter_CreateRawPdo(aDevice, GCN_Adapter_deviceCount++);

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DEVICE, "%!FUNC! Exit");

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

	numberConfiguredPipes =
		configParams.Types.SingleInterface.NumberConfiguredPipes;

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

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS GCN_AdapterPnPInitialize(
	_In_ PWDFDEVICE_INIT aDeviceInit)
{
	NTSTATUS status = STATUS_SUCCESS;
	WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
	PAGED_CODE();

	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
	pnpPowerCallbacks.EvtDevicePrepareHardware =
		GCN_AdapterEvtDevicePrepareHardware;
	
	pnpPowerCallbacks.EvtDeviceD0Entry = GCN_AdapterEvtDeviceD0Entry;
	pnpPowerCallbacks.EvtDeviceD0Exit = GCN_AdapterEvtDeviceD0Exit;
	pnpPowerCallbacks.EvtDeviceSelfManagedIoFlush =
		GCN_AdapterEvtDeviceSelfManagedIoFlush;

	WdfDeviceInitSetPnpPowerEventCallbacks(aDeviceInit, &pnpPowerCallbacks);

	return status;
}

_Use_decl_annotations_
VOID GCN_AdapterEvtDeviceSelfManagedIoFlush(
	WDFDEVICE aDevice)
{
	//There is one queue that is self managed that needs to be flushed
	//flush the queue by responding to each currently queued message.
	NTSTATUS status;
	PAGED_CODE();
	do
	{
		status = GCN_AdapterIoctlHIDReadReportHandler(aDevice);
	} while (NT_SUCCESS(status));

	if (status != STATUS_NO_MORE_ENTRIES)
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"Device failed to flush IO queue!!! %!STATUS!\n",
			status);
	}
}

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS GCN_AdapterQueueInitialize(_In_ WDFDEVICE aDevice)
{
	WDFQUEUE queue;
	NTSTATUS status;
	WDF_IO_QUEUE_CONFIG queueConfig;
	PDEVICE_CONTEXT pDevContext;

	PAGED_CODE();

	//
	// Configure a default queue so that requests that are not
	// configure-fowarded using WdfDeviceConfigureRequestDispatching to go to
	// other queues get dispatched here.
	//
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(
		&queueConfig, WdfIoQueueDispatchParallel);

	queueConfig.EvtIoInternalDeviceControl =
		GCN_AdapterEvtInternalDeviceControl;
	queueConfig.EvtIoDeviceControl = GCN_AdapterEvtDeviceControl;

	status = WdfIoQueueCreate(
		aDevice,
		&queueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&queue);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfIoQueueCreate failed %!STATUS!", status);
		goto Error;
	}

	pDevContext = DeviceGetContext(aDevice);
	pDevContext->otherQueue = queue;

	//
	// We will create a separate sequential queue and configure it
	// to receive read requests.  We also need to register a EvtIoStop
	// handler so that we can acknowledge requests that are pending
	// at the target driver.
	//
	WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchSequential);

	queueConfig.EvtIoRead = GCN_AdapterEvtIoRead;

	status = WdfIoQueueCreate(
		aDevice,
		&queueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&queue); // queue handle

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfIoQueueCreate failed 0x%x\n", status);
		goto Error;
	}

	status = WdfDeviceConfigureRequestDispatching(
		aDevice,
		queue,
		WdfRequestTypeRead);

	if (!NT_SUCCESS(status))
	{
		NT_ASSERT(NT_SUCCESS(status));
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfDeviceConfigureRequestDispatching failed 0x%x\n", status);
		goto Error;
	}

	pDevContext->readQueue = queue;

	//
	// We will create another sequential queue and configure it
	// to receive write requests.
	//
	WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchSequential);

	queueConfig.EvtIoWrite = GCN_AdapterEvtIoWrite;

	status = WdfIoQueueCreate(
		aDevice,
		&queueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&queue); // queue handle

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfIoQueueCreate failed 0x%x\n", status);
		goto Error;
	}

	status = WdfDeviceConfigureRequestDispatching(
		aDevice,
		queue,
		WdfRequestTypeWrite);

	if (!NT_SUCCESS(status))
	{
		NT_ASSERT(NT_SUCCESS(status));
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
			"WdfDeviceConfigureRequestDispatching failed 0x%x\n", status);
		goto Error;
	}

	pDevContext->writeQueue = queue;

	//
	// Register a manual I/O queue for handling Interrupt Message Read Requests.
	// This queue will be used for storing Requests that need to wait for an
	// interrupt to occur before they can be completed.
	//
	WDF_IO_QUEUE_CONFIG_INIT(&queueConfig, WdfIoQueueDispatchManual);

	//
	// This queue is used for requests that dont directly access the device. The
	// requests in this queue are serviced only when the device is in a fully
	// powered state and sends an interrupt. So we can use a non-power managed
	// queue to park the requests since we dont care whether the device is idle
	// or fully powered up.
	//
	queueConfig.PowerManaged = WdfFalse;

	status = WdfIoQueueCreate(
		aDevice,
		&queueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&pDevContext->interruptMsgQueue);

	return status;

Error:
	TraceEvents(TRACE_LEVEL_ERROR, TRACE_DEVICE,
		"%!FUNC! failed with status %!STATUS!!", status);

	return status;
}
