#include "Include.h"
#include "ioctl.tmh"

void prepare_report(GCN_AdaptorData * in, GCN_ControllerReport * out);


VOID GCN_AdaptorUsbIoctlGetInterruptMessage(
	_In_ WDFDEVICE aDevice,
	_In_ NTSTATUS  aReaderStatus)
{
	NTSTATUS status;
	WDFREQUEST request;
	PDEVICE_CONTEXT pDevContext;
	size_t bytesToCopy = 0, bytesReturned = 0;
	GCN_AdaptorData	data;
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
			if (NT_SUCCESS(aReaderStatus))
			{
				prepare_report(&pDevContext->adaptorData, pReport);
				bytesReturned = bytesToCopy;
			}
			else
			{
				bytesReturned = 0;
			}
		}

		WdfRequestCompleteWithInformation(request,
			NT_SUCCESS(status) ? aReaderStatus : status, bytesReturned);
	}
	else if (status != STATUS_NO_MORE_ENTRIES)
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IOCTL, "WdfIoQueueRetrieveNextRequest status %08x\n", status);
	}
}

VOID GCN_AdaptorEvtInternalDeviceControl(
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
		status = GCN_AdaptorGetHidDescriptor(device, aRequest);
		break;

	case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
		//
		//Retrieves a device's attributes in a HID_DEVICE_ATTRIBUTES structure.
		//
		status = GCN_AdaptorGetDeviceAttributes(aRequest);
		break;

	case IOCTL_HID_GET_REPORT_DESCRIPTOR:
		//
		//Obtains the report descriptor for the HID device.
		//
		status = GCN_AdaptorGetReportDescriptor(device, aRequest);
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


	case IOCTL_HID_SET_FEATURE:
		//
		// This sends a HID class feature report to a top-level collection of
		// a HID class device.
		//


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

NTSTATUS GCN_AdaptorGetHidDescriptor(
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

NTSTATUS GCN_AdaptorGetDeviceAttributes(
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

NTSTATUS GCN_AdaptorGetReportDescriptor(
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

//This needs to cycle through the 4 controllers
void prepare_report(GCN_AdaptorData * in, GCN_ControllerReport * out)
{
	static BYTE id = 1;
	out->id = id;
	memcpy(&(out->Buttons1), &(in->Port[id-1].Buttons1), 8);
	out->LeftAxis.Y = ~out->LeftAxis.Y;
	id = (id + 1) % 5;
}
