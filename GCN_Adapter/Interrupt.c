#include "Include.h"
#include "interrupt.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, GCN_AdapterConfigContReaderForInterruptEndPoint)
#pragma alloc_text(PAGE, GCN_AdapterEvtUsbInterruptReadersFailed)
#endif

//number obtained from USB descriptor for endpoint
#define GCN_ADAPTER_READ_LENGTH 37

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS GCN_AdapterConfigContReaderForInterruptEndPoint(
	_In_ PDEVICE_CONTEXT apDeviceContext)
{
	WDF_USB_CONTINUOUS_READER_CONFIG contReaderConfig;
	NTSTATUS status;

	PAGED_CODE();

	WDF_USB_CONTINUOUS_READER_CONFIG_INIT(
		&contReaderConfig,
		GCN_AdapterEvtUsbInterruptPipeReadComplete,
		apDeviceContext,    // Context
		sizeof(UCHAR));   // TransferLength

	contReaderConfig.EvtUsbTargetPipeReadersFailed =
		GCN_AdapterEvtUsbInterruptReadersFailed;
	
	
	contReaderConfig.TransferLength = GCN_ADAPTER_READ_LENGTH;
	contReaderConfig.NumPendingReads = 0; //Use default

	//Remember to actually call WdfIoTargetStart elsewhere to start the reader!
	status = WdfUsbTargetPipeConfigContinuousReader(
		apDeviceContext->interruptReadPipe,
		&contReaderConfig);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_INTERRUPT,
			"GCN_AdapterConfigContReaderForInterruptEndPoint failed %x\n",
			status);
		return status;
	}

	return status;
}

VOID GCN_AdapterEvtUsbInterruptPipeReadComplete(
	WDFUSBPIPE aPipe,
	WDFMEMORY aBuffer,
	size_t aNumBytesTransferred,
	WDFCONTEXT aContext)
{
	WDFDEVICE device;
	PDEVICE_CONTEXT pDeviceContext = aContext;
	GCN_AdapterData *pData;
	int i;
	BYTE rumble;

	UNREFERENCED_PARAMETER(aPipe);

	device = WdfObjectContextGetObject(pDeviceContext);

	if (aNumBytesTransferred == 0)
	{
		TraceEvents(TRACE_LEVEL_WARNING, TRACE_INTERRUPT,
			"GCN_AdapterEvtUsbInterruptPipeReadComplete Zero length read "
			"occured on the Interrupt Pipe's Continuous Reader\n"
			);
		return;
	}

	//Number of bytes coming in from the device
	NT_ASSERT(aNumBytesTransferred == GCN_ADAPTER_READ_LENGTH);
	NT_ASSERT(aNumBytesTransferred == sizeof(pDeviceContext->adapterData));

	pData = WdfMemoryGetBuffer(aBuffer, NULL);

	WdfSpinLockAcquire(pDeviceContext->dataLock);
	memcpy(&pDeviceContext->adapterData, pData, aNumBytesTransferred);
	WdfSpinLockRelease(pDeviceContext->dataLock);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_INTERRUPT,
		"GCN_AdapterEvtUsbInterruptPipeReadComplete matched: %x\n",
		pDeviceContext->adapterData.signal == 0x21);

	//Quickly, handle rumble information for controllers if any have gone
	//offline. Turn rumble off if it is enabled and power is removed
	if (!pData->port[0].status.powered)
	{
		GCN_Adapter_Rumble(pDeviceContext, 0);
	}

	//Handle next Interrupt Message IOCTLs, READ_REPORT
	GCN_AdapterIoctlHIDReadReportHandler(device);
}

BOOLEAN GCN_AdapterEvtUsbInterruptReadersFailed(
	_In_ WDFUSBPIPE aPipe,
	_In_ NTSTATUS aStatus,
	_In_ USBD_STATUS aUsbdStatus)
{
	PAGED_CODE();
	WDFDEVICE device = WdfIoTargetGetDevice(WdfUsbTargetPipeGetIoTarget(aPipe));
	PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(device);

	UNREFERENCED_PARAMETER(aUsbdStatus);
	
	GCN_AdapterIoctlHIDReadReportHandler(device);

	return TRUE;
}
