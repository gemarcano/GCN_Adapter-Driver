#include "Include.h"
#include "io.tmh"

VOID GCN_AdapterEvtIoRead(
	_In_ WDFQUEUE Queue,
	_In_ WDFREQUEST Request,
	_In_ size_t Length)
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
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IO, "WdfRequestRetrieveOutputMemory failed %!STATUS!\n", status);
		goto Exit;
	}

	//FIXME TODO This probably needs to be synchronized with continuous reader
	memcpy(WdfMemoryGetBuffer(reqMemory, NULL), &pDeviceContext->adaptorData, Length);

	WdfRequestCompleteWithInformation(Request, status, Length);

Exit:
	if (!NT_SUCCESS(status))
	{
		WdfRequestCompleteWithInformation(Request, status, 0);
	}
	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_IO, "<-- GCN_AdapterEvtIoRead\n");
}

VOID GCN_AdapterEvtIoWrite(
	_In_ WDFQUEUE aQueue,
	_In_ WDFREQUEST aRequest,
	_In_ size_t aLength)
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
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IO, "Transfer does not equal %d\n",
			5);
		status = STATUS_INVALID_PARAMETER;
		goto Exit;
	}

	pDeviceContext = DeviceGetContext(WdfIoQueueGetDevice(aQueue));

	pipe = pDeviceContext->interruptWritePipe;

	status = WdfRequestRetrieveInputMemory(aRequest, &reqMemory);
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_IO, "WdfRequestRetrieveInputBuffer failed\n");
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
		EvtRequestWriteCompletionRoutine,
		pipe);

	if (WdfRequestSend(aRequest, WdfUsbTargetPipeGetIoTarget(pipe), WDF_NO_SEND_OPTIONS) == FALSE)
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

VOID EvtRequestWriteCompletionRoutine(
	_In_ WDFREQUEST aRequest,
	_In_ WDFIOTARGET aTarget,
	_In_ PWDF_REQUEST_COMPLETION_PARAMS aCompletionParams,
	_In_ WDFCONTEXT aContext)
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

VOID GCN_AdapterEvtIoStop(
	_In_ WDFQUEUE aQueue,
	_In_ WDFREQUEST aRequest,
	_In_ ULONG aActionFlags)
{
	TraceEvents(TRACE_LEVEL_INFORMATION,
		TRACE_IO,
		"!FUNC! Queue 0x%p, Request 0x%p ActionFlags %d",
		aQueue, aRequest, aActionFlags);

	//
	// In most cases, the EvtIoStop callback function completes, cancels, or postpones
	// further processing of the I/O request.
	//
	// Typically, the driver uses the following rules:
	//
	// - If the driver owns the I/O request, it either postpones further processing
	//   of the request and calls WdfRequestStopAcknowledge, or it calls WdfRequestComplete
	//   with a completion status value of STATUS_SUCCESS or STATUS_CANCELLED.
	//  
	//   The driver must call WdfRequestComplete only once, to either complete or cancel
	//   the request. To ensure that another thread does not call WdfRequestComplete
	//   for the same request, the EvtIoStop callback must synchronize with the driver's
	//   other event callback functions, for instance by using interlocked operations.
	//
	// - If the driver has forwarded the I/O request to an I/O target, it either calls
	//   WdfRequestCancelSentRequest to attempt to cancel the request, or it postpones
	//   further processing of the request and calls WdfRequestStopAcknowledge.
	//
	// A driver might choose to take no action in EvtIoStop for requests that are
	// guaranteed to complete in a small amount of time. For example, the driver might
	// take no action for requests that are completed in one of the driver’s request handlers.
	//

	UNREFERENCED_PARAMETER(aQueue);
	UNREFERENCED_PARAMETER(aActionFlags);

	if (aActionFlags &  WdfRequestStopActionSuspend)
	{
		WdfRequestStopAcknowledge(aRequest, FALSE); // Don't requeue
	}
	else if (aActionFlags &  WdfRequestStopActionPurge)
	{
		WdfRequestCancelSentRequest(aRequest);
	}
}
