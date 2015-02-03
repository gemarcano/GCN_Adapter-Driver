#include "Include.h"
#include "io.tmh"

_Use_decl_annotations_
VOID GCN_AdapterEvtIoRead(
	WDFQUEUE Queue,
	WDFREQUEST Request,
	size_t Length)
{
	WDFUSBPIPE pipe;
	NTSTATUS status;
	WDFMEMORY reqMemory, tmp;
	PDEVICE_CONTEXT pDeviceContext;

	UNREFERENCED_PARAMETER(Queue);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IO, "-->GCN_AdapterEvtIoRead\n");

	//Validate input
	if (Length != 37)
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IO, "Transfer exceeds %d\n",
			37);
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(Queue));

	status = WdfRequestRetrieveOutputMemory(Request, &reqMemory);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IO,
			"WdfRequestRetrieveOutputMemory failed %!STATUS!\n", status);
		goto Exit;
	}

	WdfSpinLockAcquire(pDeviceContext->dataLock);
	memcpy(
		WdfMemoryGetBuffer(reqMemory, NULL),
		&pDeviceContext->adapterData,
		Length);

	WdfSpinLockRelease(pDeviceContext->dataLock);

	WdfRequestCompleteWithInformation(Request, status, Length);

Exit:
	if (!NT_SUCCESS(status))
	{
		WdfRequestCompleteWithInformation(Request, status, 0);
	}
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_IO, "<-- GCN_AdapterEvtIoRead\n");
}

_Use_decl_annotations_
VOID GCN_AdapterEvtIoWrite(
	WDFQUEUE aQueue,
	WDFREQUEST aRequest,
	size_t aLength)
{
	NTSTATUS status;
	WDFUSBPIPE pipe;
	WDFMEMORY reqMemory;
	PDEVICE_CONTEXT pDeviceContext;
	unsigned char buf[5];
	
	UNREFERENCED_PARAMETER(aQueue);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IO, "-->GCN_AdapterEvtIoWrite\n");

	if (aLength != 5)
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IO,
			"Transfer does not equal %d\n", 5);
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(aQueue));

	pipe = pDeviceContext->interruptWritePipe;

	status = WdfRequestRetrieveInputMemory(aRequest, &reqMemory);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IO,
			"WdfRequestRetrieveInputBuffer failed\n");
		goto Exit;
	}

	memcpy(buf, WdfMemoryGetBuffer(reqMemory, NULL), 5);

	status = WdfUsbTargetPipeFormatRequestForWrite(
		pipe,
		aRequest,
		reqMemory,
		NULL); // Offset


	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IO,
			"WdfUsbTargetPipeFormatRequestForWrite failed 0x%x\n", status);
		goto Exit;
	}

	WdfRequestSetCompletionRoutine(
		aRequest,
		GCN_AdapterEvtRequestWriteCompletionRoutine,
		pipe);

	if (WdfRequestSend(
			aRequest, WdfUsbTargetPipeGetIoTarget(pipe), WDF_NO_SEND_OPTIONS)
		== FALSE)
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IO, "WdfRequestSend failed\n");
		status = WdfRequestGetStatus(aRequest);
		goto Exit;
	}

Exit:

	if (!NT_SUCCESS(status))
	{	
		WdfRequestCompleteWithInformation(aRequest, status, 0);
	}

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_IO, "<-- GCN_AdapterEvtIoWrite\n");

	return;
}

_Use_decl_annotations_
VOID GCN_AdapterEvtRequestWriteCompletionRoutine(
	WDFREQUEST aRequest,
	WDFIOTARGET aTarget,
	PWDF_REQUEST_COMPLETION_PARAMS aCompletionParams,
	WDFCONTEXT aContext)
{
	NTSTATUS status;
	size_t bytesWritten = 0;
	PWDF_USB_REQUEST_COMPLETION_PARAMS usbCompletionParams;

	UNREFERENCED_PARAMETER(aTarget);
	UNREFERENCED_PARAMETER(aContext);

	status = aCompletionParams->IoStatus.Status;

	//
	// For usb devices, we should look at the Usb.Completion param.
	//
	usbCompletionParams = aCompletionParams->Parameters.Usb.Completion;

	bytesWritten = usbCompletionParams->Parameters.PipeWrite.Length;

	if (NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_IO,
			"Number of bytes written: %I64d\n", (INT64)bytesWritten);
	}
	else
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IO,
			"Write failed: request Status 0x%x UsbdStatus 0x%x\n",
			status, usbCompletionParams->UsbdStatus);
	}

	WdfRequestCompleteWithInformation(aRequest, status, bytesWritten);
}
