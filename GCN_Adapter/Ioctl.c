#include "Include.h"
#include "ioctl.tmh"

VOID GCN_AdapterEvtInternalDeviceControl(
	_In_ WDFQUEUE aQueue,
	_In_ WDFREQUEST aRequest,
	_In_ size_t aOutputBufferLength,
	_In_ size_t aInputBufferLength,
	_In_ ULONG aIoControlCode)
{
	NTSTATUS status = STATUS_SUCCESS;
	WDFDEVICE device;
	PDEVICE_CONTEXT devContext = NULL;
	ULONG bytesReturned = 0;
	GCN_ControllerReport *pController;

	UNREFERENCED_PARAMETER(aOutputBufferLength);
	UNREFERENCED_PARAMETER(aInputBufferLength);

	device = WdfIoQueueGetDevice(aQueue);
	devContext = DeviceGetContext(device);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL,
		"%s, Queue:0x%p, Request:0x%p\n",
		DbgHidInternalIoctlString(aIoControlCode),
		aQueue,
		aRequest);

	//
	// Please note that HIDCLASS provides the buffer in the Irp->UserBuffer
	// field irrespective of the ioctl buffer type. However, framework is very
	// strict about type checking. You cannot get Irp->UserBuffer by using
	// WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
	// internal ioctl. So depending on the ioctl code, we will either
	// use retreive function or escape to WDM to get the UserBuffer.
	//

	switch (aIoControlCode)
	{

	case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
		//
		// Retrieves the device's HID descriptor.
		//
		status = GCN_AdapterGetHidDescriptor(device, aRequest);
		break;

	case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
		//
		//Retrieves a device's attributes in a HID_DEVICE_ATTRIBUTES structure.
		//
		status = GCN_AdapterGetDeviceAttributes(aRequest);
		break;

	case IOCTL_HID_GET_REPORT_DESCRIPTOR:
		//
		//Obtains the report descriptor for the HID device.
		//
		status = GCN_AdapterGetReportDescriptor(device, aRequest);
		break;

	case IOCTL_HID_READ_REPORT:

		//
		// Returns a report from the device into a class driver-supplied buffer.
		// For now queue the request to the manual queue. The request will
		// be retrived and completd when continuous reader reads new data
		// from the device.
		//
		status = WdfRequestForwardToIoQueue(aRequest, devContext->interruptMsgQueue);
		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
				"WdfRequestForwardToIoQueue failed with status: 0x%x\n", status);

			WdfRequestComplete(aRequest, status);
		}

		return;

		//
		// This feature is only supported on WinXp and later. Compiling in W2K 
		// build environment will fail without this conditional preprocessor statement.
		//
	case IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST:

		//
		// Hidclass sends this IOCTL for devices that have opted-in for Selective
		// Suspend feature. This feature is enabled by adding a registry value
		// "SelectiveSuspendEnabled" = 1 in the hardware key through inf file 
		// (see hidusbfx2.inf). Since hidclass is the power policy owner for 
		// this stack, it controls when to send idle notification and when to 
		// cancel it. This IOCTL is passed to USB stack. USB stack pends it. 
		// USB stack completes the request when it determines that the device is
		// idle. Hidclass's idle notification callback get called that requests a 
		// wait-wake Irp and subsequently powers down the device. 
		// The device is powered-up either when a handle is opened for the PDOs 
		// exposed by hidclass, or when usb stack completes wait
		// wake request. In the first case, hidclass cancels the notification 
		// request (pended with usb stack), cancels wait-wake Irp and powers up
		// the device. In the second case, an external wake event triggers completion
		// of wait-wake irp and powering up of device.
		//
		status = GCN_AdapterSendIdleNotification(aRequest);
		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
				"SendIdleNotification failed with status: 0x%x\n", status);

			WdfRequestComplete(aRequest, status);
		}
		return;

	case IOCTL_GCN_ADAPTER_CALIBRATE:
		status = GCN_AdapterCalibrate(device, aRequest);
		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
				"GCN_AdapterCalibrate failed with status: 0x%x\n", status);

			WdfRequestComplete(aRequest, status);
		}
		return;

	case IOCTL_GCN_ADAPTER_SET_DEADZONE:
		status = GCN_AdapterSetDeadzone(device, aRequest);
		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
				"GCN_AdapterSetDeadzone failed with status: 0x%x\n", status);

			WdfRequestComplete(aRequest, status);
		}
		return;

	case IOCTL_GCN_ADAPTER_GET_DEADZONE:
		status = GCN_AdapterGetSensitivity(device, aRequest);
		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
				"GCN_AdapterGetSensitivity failed with status: 0x%x\n", status);

			WdfRequestComplete(aRequest, status);
		}
		return;

	case IOCTL_GCN_ADAPTER_SET_RUMBLE:
		status = GCN_AdapterSetRumble(device, aRequest);
		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
				"GCN_AdapterSetRumble failed with status: 0x%x\n", status);

			WdfRequestComplete(aRequest, status);
		}
		return;

	case IOCTL_HID_SET_FEATURE:
		//
		// This sends a HID class feature report to a top-level collection of
		// a HID class device.
		//
		//We could use this for enabling rumble...


	case IOCTL_HID_GET_FEATURE:
		//
		// Get a HID class feature report from a top-level collection of
		// a HID class device.
		//


	case IOCTL_HID_WRITE_REPORT:
		//
		//Transmits a class driver-supplied report to the device.
		//
	case IOCTL_HID_GET_STRING:
		//
		// Requests that the HID minidriver retrieve a human-readable string
		// for either the manufacturer ID, the product ID, or the serial number
		// from the string descriptor of the device. The minidriver must send
		// a Get String Descriptor request to the device, in order to retrieve
		// the string descriptor, then it must extract the string at the
		// appropriate index from the string descriptor and return it in the
		// output buffer indicated by the IRP. Before sending the Get String
		// Descriptor request, the minidriver must retrieve the appropriate
		// index for the manufacturer ID, the product ID or the serial number
		// from the device extension of a top level collection associated with
		// the device.
		//
	case IOCTL_HID_ACTIVATE_DEVICE:
		//
		// Makes the device ready for I/O operations.
		//
	case IOCTL_HID_DEACTIVATE_DEVICE:
		//
		// Causes the device to cease operations and terminate all outstanding
		// I/O requests.
		//
	default:
		status = STATUS_NOT_SUPPORTED;
		break;
	}

	WdfRequestComplete(aRequest, status);
}

//Helper IOCTL functions follow below:

NTSTATUS GCN_AdapterIoctlHIDReadReportHandler(_In_ WDFDEVICE aDevice)
{
	NTSTATUS status;
	WDFREQUEST request;
	PDEVICE_CONTEXT pDevContext;
	size_t bytesToCopy = 0, bytesReturned = 0;
	GCN_AdapterData	data;
	GCN_ControllerReport *pReport;

	pDevContext = DeviceGetContext(aDevice);

	status = WdfIoQueueRetrieveNextRequest(pDevContext->interruptMsgQueue, &request);

	if (NT_SUCCESS(status))
	{
		//
		// IOCTL_HID_READ_REPORT is METHOD_NEITHER so WdfRequestRetrieveOutputBuffer
		// will correctly retrieve buffer from Irp->UserBuffer. Remember that
		// HIDCLASS provides the buffer in the Irp->UserBuffer field
		// irrespective of the ioctl buffer type. However, framework is very
		// strict about type checking. You cannot get Irp->UserBuffer by using
		// WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
		// internal ioctl.

		bytesToCopy = sizeof(GCN_ControllerReport);
		status = WdfRequestRetrieveOutputBuffer(
			request,
			bytesToCopy,
			&pReport,
			&bytesReturned);// BufferLength

		if (!NT_SUCCESS(status))
		{
			TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
				"WdfRequestRetrieveOutputBuffer failed with status: 0x%x\n", status);
		}
		else
		{
				prepare_report(pDevContext, &pDevContext->adapterData, pReport);
				bytesReturned = bytesToCopy;
		}

		WdfRequestCompleteWithInformation(request, status, bytesReturned);
	}
	else if (status != STATUS_NO_MORE_ENTRIES)
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL, "WdfIoQueueRetrieveNextRequest status %08x\n", status);
	}

	return status;
}

NTSTATUS GCN_AdapterGetHidDescriptor(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest)
{
	NTSTATUS status = STATUS_SUCCESS;
	size_t bytesToCopy = 0;
	WDFMEMORY memory;

	UNREFERENCED_PARAMETER(aDevice);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL,
		"HidFx2GetHidDescriptor Entry\n");

	//
	// This IOCTL is METHOD_NEITHER so WdfRequestRetrieveOutputMemory
	// will correctly retrieve buffer from Irp->UserBuffer. 
	// Remember that HIDCLASS provides the buffer in the Irp->UserBuffer
	// field irrespective of the ioctl buffer type. However, framework is very
	// strict about type checking. You cannot get Irp->UserBuffer by using
	// WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
	// internal ioctl.
	//
	status = WdfRequestRetrieveOutputMemory(aRequest, &memory);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
		return status;
	}

	bytesToCopy = HIDDescriptor.bLength;

	if (bytesToCopy == 0)
	{
		status = STATUS_INVALID_DEVICE_STATE;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"HIDDescriptor is zero, 0x%x\n", status);
		return status;
	}

	status = WdfMemoryCopyFromBuffer(
		memory,
		0, // Offset
		(PVOID)&HIDDescriptor,
		bytesToCopy);
	
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"WdfMemoryCopyFromBuffer failed 0x%x\n", status);
		return status;
	}

	WdfRequestSetInformation(aRequest, bytesToCopy);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL,
		"HidFx2GetHidDescriptor Exit = 0x%x\n", status);

	return status;
}

NTSTATUS GCN_AdapterGetDeviceAttributes(
	_In_ WDFREQUEST aRequest)
{
	NTSTATUS status;
	PHID_DEVICE_ATTRIBUTES deviceAttributes = NULL;
	PUSB_DEVICE_DESCRIPTOR usbDeviceDescriptor = NULL;
	PDEVICE_CONTEXT deviceInfo = NULL;

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL,
		"HidFx2GetDeviceAttributes Entry\n");

	deviceInfo = DeviceGetContext(WdfIoQueueGetDevice(WdfRequestGetIoQueue(aRequest)));

	//
	// This IOCTL is METHOD_NEITHER so WdfRequestRetrieveOutputMemory
	// will correctly retrieve buffer from Irp->UserBuffer. 
	// Remember that HIDCLASS provides the buffer in the Irp->UserBuffer
	// field irrespective of the ioctl buffer type. However, framework is very
	// strict about type checking. You cannot get Irp->UserBuffer by using
	// WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
	// internal ioctl.
	//
	status = WdfRequestRetrieveOutputBuffer(
		aRequest,
		sizeof(HID_DEVICE_ATTRIBUTES),
		&deviceAttributes,
		NULL);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"WdfRequestRetrieveOutputBuffer failed 0x%x\n", status);
		return status;
	}

	usbDeviceDescriptor = WdfMemoryGetBuffer(deviceInfo->usbDeviceDescriptor, NULL);

	deviceAttributes->Size = sizeof(HID_DEVICE_ATTRIBUTES);
	deviceAttributes->VendorID = usbDeviceDescriptor->idVendor;
	deviceAttributes->ProductID = usbDeviceDescriptor->idProduct;;
	deviceAttributes->VersionNumber = usbDeviceDescriptor->bcdDevice;

	WdfRequestSetInformation(aRequest, sizeof(HID_DEVICE_ATTRIBUTES));

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL,
		"HidFx2GetDeviceAttributes Exit = 0x%x\n", status);
	
	return status;
}

NTSTATUS GCN_AdapterGetReportDescriptor(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest)
{
	NTSTATUS status = STATUS_SUCCESS;
	ULONG_PTR bytesToCopy;
	WDFMEMORY memory;

	UNREFERENCED_PARAMETER(aDevice);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL,
		"HidFx2GetReportDescriptor Entry\n");

	//
	// This IOCTL is METHOD_NEITHER so WdfRequestRetrieveOutputMemory
	// will correctly retrieve buffer from Irp->UserBuffer. 
	// Remember that HIDCLASS provides the buffer in the Irp->UserBuffer
	// field irrespective of the ioctl buffer type. However, framework is very
	// strict about type checking. You cannot get Irp->UserBuffer by using
	// WdfRequestRetrieveOutputMemory if the ioctl is not a METHOD_NEITHER
	// internal ioctl.
	//
	status = WdfRequestRetrieveOutputMemory(aRequest, &memory);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"WdfRequestRetrieveOutputMemory failed 0x%x\n", status);
		return status;
	}

	//
	// Use hardcoded Report descriptor
	//
	bytesToCopy = HIDDescriptor.DescriptorList[0].wReportLength;

	if (bytesToCopy == 0)
	{
		status = STATUS_INVALID_DEVICE_STATE;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"HIDDescriptor's reportLenght is zero, 0x%x\n", status);
		return status;
	}

	status = WdfMemoryCopyFromBuffer(
		memory,
		0,
		(PVOID)ReportDescriptor,
		bytesToCopy);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"WdfMemoryCopyFromBuffer failed 0x%x\n", status);
		return status;
	}

	WdfRequestSetInformation(aRequest, bytesToCopy);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL,
		"HidFx2GetReportDescriptor Exit = 0x%x\n", status);
	
	return status;
}

NTSTATUS GCN_AdapterSendIdleNotification(_In_ WDFREQUEST Request)
{
	NTSTATUS                   status = STATUS_SUCCESS;
	BOOLEAN                    sendStatus = FALSE;
	WDF_REQUEST_SEND_OPTIONS   options;
	WDFIOTARGET                nextLowerDriver;
	WDFDEVICE                  device;
	PIO_STACK_LOCATION         currentIrpStack = NULL;
	IO_STACK_LOCATION          nextIrpStack;

	device = WdfIoQueueGetDevice(WdfRequestGetIoQueue(Request));
	currentIrpStack = IoGetCurrentIrpStackLocation(WdfRequestWdmGetIrp(Request));

	//
	// Convert the request to corresponding USB Idle notification request
	//
	if (currentIrpStack->Parameters.DeviceIoControl.InputBufferLength <
		sizeof(HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO))
	{

		status = STATUS_BUFFER_TOO_SMALL;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"DeviceIoControl.InputBufferLength too small, 0x%x\n", status);
		return status;
	}

	ASSERT(sizeof(HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO)
		== sizeof(USB_IDLE_CALLBACK_INFO));

	if (sizeof(HID_SUBMIT_IDLE_NOTIFICATION_CALLBACK_INFO) != sizeof(USB_IDLE_CALLBACK_INFO))
	{

		status = STATUS_INFO_LENGTH_MISMATCH;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"Incorrect DeviceIoControl.InputBufferLength, 0x%x\n", status);
		return status;
	}

	//
	// prepare next stack location
	//
	RtlZeroMemory(&nextIrpStack, sizeof(IO_STACK_LOCATION));

	nextIrpStack.MajorFunction = currentIrpStack->MajorFunction;
	nextIrpStack.Parameters.DeviceIoControl.InputBufferLength =
		currentIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	nextIrpStack.Parameters.DeviceIoControl.Type3InputBuffer =
		currentIrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
	nextIrpStack.Parameters.DeviceIoControl.IoControlCode =
		IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION;
	nextIrpStack.DeviceObject =
		WdfIoTargetWdmGetTargetDeviceObject(WdfDeviceGetIoTarget(device));

	//
	// Format the I/O request for the driver's local I/O target by using the
	// contents of the specified WDM I/O stack location structure.
	//
	WdfRequestWdmFormatUsingStackLocation(Request, &nextIrpStack);

	//
	// Send the request down using Fire and forget option.
	//
	WDF_REQUEST_SEND_OPTIONS_INIT(
		&options,
		WDF_REQUEST_SEND_OPTION_SEND_AND_FORGET);

	nextLowerDriver = WdfDeviceGetIoTarget(device);

	sendStatus = WdfRequestSend(Request, nextLowerDriver, &options);

	if (sendStatus == FALSE)
	{
		status = STATUS_UNSUCCESSFUL;
	}

	return status;
}

NTSTATUS GCN_AdapterCalibrate(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest)
{
	WDF_REQUEST_PARAMETERS params;
	IOCTL_GCN_Adapter_Calibrate_Data *pData;
	PDEVICE_CONTEXT devContext = DeviceGetContext(aDevice);
	NTSTATUS status = STATUS_SUCCESS;
	UCHAR i;

	PAGED_CODE();

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL, "!FUNC! Enter\n");

	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(aRequest, &params);

	// This IOCTL is METHOD_BUFFER, use SystemBuffer to get memory
	if (params.Parameters.DeviceIoControl.InputBufferLength < sizeof(IOCTL_GCN_Adapter_Calibrate_Data)) {
		status = STATUS_BUFFER_TOO_SMALL;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"Userbuffer is small 0x%x\n", status);
		return status;
	}

	pData = WdfRequestWdmGetIrp(aRequest)->AssociatedIrp.SystemBuffer;
	if (pData == NULL) {
		status = STATUS_INVALID_DEVICE_REQUEST;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"Irp->UserBuffer is NULL 0x%x\n", status);
		return status;
	}

	if ((pData->controllers) == 0x0f)
	{
		status = GCN_Controller_Calibrate(devContext, -1);
	}
	else
	{
		status = STATUS_SUCCESS;
		for (i = 0; i < 4 && NT_SUCCESS(status); ++i)
		{
			if ((1 << i) & pData->controllers)
			{
				status = GCN_Controller_Calibrate(devContext, i);
			}
		}
	}
	
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"Calibration failed with status: 0x%x\n", status);
	}
	else
	{
		WdfRequestSetInformation(aRequest, 0);
		WdfRequestComplete(aRequest, status);
	}

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL, "!FUNC! Exit\n");
	return status;
}

NTSTATUS GCN_AdapterSetDeadzone(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest)
{
	WDF_REQUEST_PARAMETERS params;
	IOCTL_GCN_Adapter_Deadzone_Data *pData;
	PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(aDevice);
	NTSTATUS status = STATUS_SUCCESS;
	UCHAR i;

	PAGED_CODE();

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL, "!FUNC! Enter\n");

	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(aRequest, &params);

	// This IOCTL is METHOD_BUFFER, use SystemBuffer to get memory
	if (params.Parameters.DeviceIoControl.InputBufferLength < sizeof(IOCTL_GCN_Adapter_Deadzone_Data)) {
		status = STATUS_BUFFER_TOO_SMALL;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"Userbuffer is small 0x%x\n", status);
		return status;
	}

	pData = WdfRequestWdmGetIrp(aRequest)->AssociatedIrp.SystemBuffer;
	if (pData == NULL) {
		status = STATUS_INVALID_DEVICE_REQUEST;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"Irp->UserBuffer is NULL 0x%x\n", status);
		return status;
	}

	status = STATUS_SUCCESS;
	
	GCN_Controller_Status_Update_Deadzone(&pDeviceContext->controllerStatus[pData->controller], &pData->data);
	
	WdfRequestSetInformation(aRequest, 0);
	WdfRequestComplete(aRequest, status);
	
	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL, "!FUNC! Exit\n");
	return status;
}

NTSTATUS GCN_AdapterSetRumble(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest)
{
	WDF_REQUEST_PARAMETERS params;
	IOCTL_GCN_Adapter_Rumble_Data *pData;
	PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(aDevice);
	GCN_AdapterData adapterData;
	NTSTATUS status;

	PAGED_CODE();

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL, "!FUNC! Enter\n");

	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(aRequest, &params);

	// This IOCTL is METHOD_BUFFER, use SystemBuffer to get memory
	if (params.Parameters.DeviceIoControl.InputBufferLength < sizeof(IOCTL_GCN_Adapter_Rumble_Data)) {
		status = STATUS_BUFFER_TOO_SMALL;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"Userbuffer is small 0x%x\n", status);
		return status;
	}

	pData = WdfRequestWdmGetIrp(aRequest)->AssociatedIrp.SystemBuffer;
	if (pData == NULL) {
		status = STATUS_INVALID_DEVICE_REQUEST;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"Irp->UserBuffer is NULL 0x%x\n", status);
		return status;
	}

	status = GCN_Adapter_Rumble(pDeviceContext, pData->controllers);

	WdfRequestSetInformation(aRequest, 0);
	WdfRequestComplete(aRequest, status);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL, "!FUNC! Exit\n");
	return status;
}

NTSTATUS GCN_AdapterGetSensitivity(
	_In_ WDFDEVICE aDevice,
	_In_ WDFREQUEST aRequest)
{
	WDF_REQUEST_PARAMETERS params;
	IOCTL_GCN_Adapter_Deadzone_Data *pData;
	PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(aDevice);
	NTSTATUS status = STATUS_SUCCESS;
	UCHAR i;

	PAGED_CODE();

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL, "!FUNC! Enter\n");

	WDF_REQUEST_PARAMETERS_INIT(&params);
	WdfRequestGetParameters(aRequest, &params);

	// This IOCTL is METHOD_BUFFER, use SystemBuffer to get memory
	if (params.Parameters.DeviceIoControl.InputBufferLength < sizeof(IOCTL_GCN_Adapter_Deadzone_Data) ||
		params.Parameters.DeviceIoControl.OutputBufferLength < sizeof(IOCTL_GCN_Adapter_Deadzone_Data))
	{
		status = STATUS_BUFFER_TOO_SMALL;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"Userbuffer is small 0x%x\n", status);
		return status;
	}

	pData = WdfRequestWdmGetIrp(aRequest)->AssociatedIrp.SystemBuffer;
	if (pData == NULL) {
		status = STATUS_INVALID_DEVICE_REQUEST;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL,
			"Irp->UserBuffer is NULL 0x%x\n", status);
		return status;
	}


	pData->data = pDeviceContext->controllerStatus[pData->controller].deadzone;


	WdfRequestSetInformation(aRequest, sizeof(IOCTL_GCN_Adapter_Deadzone_Data));
	WdfRequestComplete(aRequest, status);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IOCTL, "!FUNC! Exit\n");
	return status;
}