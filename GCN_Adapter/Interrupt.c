#include "Include.h"
#include "interrupt.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, GCN_AdapterConfigContReaderForInterruptEndPoint)
#endif

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

	contReaderConfig.EvtUsbTargetPipeReadersFailed = GCN_AdapterEvtUsbInterruptReadersFailed;
	contReaderConfig.TransferLength = 37; //number obtained from USB descriptor for endpoint
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

	NT_ASSERT(aNumBytesTransferred == 37); //Number of bytes coming in from the device
	NT_ASSERT(aNumBytesTransferred == sizeof(pDeviceContext->adaptorData));

	WdfSpinLockAcquire(pDeviceContext->dataLock);
	memcpy(&pDeviceContext->adaptorData, WdfMemoryGetBuffer(aBuffer, NULL), aNumBytesTransferred);
	WdfSpinLockRelease(pDeviceContext->dataLock);

	TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_INTERRUPT,
		"GCN_AdapterEvtUsbInterruptPipeReadComplete matched: %x\n",
		pDeviceContext->adaptorData.Signal == 0x21);

	//Handle next Interrupt Message IOCTLs, READ_REPORT
	//TODO check if this is the only IOCTL message we need to deal with
	GCN_AdapterUsbIoctlGetInterruptMessage(device, STATUS_SUCCESS);
}

BOOLEAN GCN_AdapterEvtUsbInterruptReadersFailed(
	_In_ WDFUSBPIPE aPipe,
	_In_ NTSTATUS aStatus,
	_In_ USBD_STATUS aUsbdStatus)
{
	WDFDEVICE device = WdfIoTargetGetDevice(WdfUsbTargetPipeGetIoTarget(aPipe));
	PDEVICE_CONTEXT pDeviceContext = DeviceGetContext(device);

	UNREFERENCED_PARAMETER(aUsbdStatus);
	
	//FIXME do we want to handle messages even when reading from the device failed?
	//GCN_AdapterUsbIoctlGetInterruptMessage(device, aStatus);

	return TRUE;
}

