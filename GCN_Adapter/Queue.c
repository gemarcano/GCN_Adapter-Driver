#include "Include.h"
#include "queue.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, GCN_AdapterQueueInitialize)
#endif

/*++

Routine Description:


The I/O dispatch callbacks for the frameworks device object
are configured in this function.

A single default I/O Queue is configured for parallel request
processing, and a driver context memory allocation is created
to hold our structure QUEUE_CONTEXT.

Arguments:

Device - Handle to a framework device object.

Return Value:

VOID

--*/
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
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&queueConfig, WdfIoQueueDispatchParallel);

	queueConfig.EvtIoInternalDeviceControl = GCN_AdapterEvtInternalDeviceControl;
	queueConfig.EvtIoDeviceControl = GCN_AdapterEvtInternalDeviceControl;
    queueConfig.EvtIoStop = GCN_AdapterEvtIoStop;

    status = WdfIoQueueCreate(
                 aDevice,
                 &queueConfig,
                 WDF_NO_OBJECT_ATTRIBUTES,
                 &queue
                 );

    if( !NT_SUCCESS(status) )
	{
        TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE, "WdfIoQueueCreate failed %!STATUS!", status);
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
	queueConfig.EvtIoStop = GCN_AdapterEvtIoStop;

	status = WdfIoQueueCreate(
		aDevice,
		&queueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&queue // queue handle
		);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE,
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
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE,
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
	queueConfig.EvtIoStop = GCN_AdapterEvtIoStop;

	status = WdfIoQueueCreate(
		aDevice,
		&queueConfig,
		WDF_NO_OBJECT_ATTRIBUTES,
		&queue); // queue handle

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE,
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
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_QUEUE,
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
	//FIXME do something like tracing?
    return status;
}


