#include "Include.h"
#include "power.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, GCN_AdapterEvtDeviceD0Exit)
#endif

_IRQL_requires_(PASSIVE_LEVEL)
PCHAR DbgDevicePowerString(_In_ WDF_POWER_DEVICE_STATE Type)
{
	switch (Type)
	{
	case WdfPowerDeviceInvalid:
		return "WdfPowerDeviceInvalid";
	case WdfPowerDeviceD0:
		return "WdfPowerDeviceD0";
	case WdfPowerDeviceD1:
		return "WdfPowerDeviceD1";
	case WdfPowerDeviceD2:
		return "WdfPowerDeviceD2";
	case WdfPowerDeviceD3:
		return "WdfPowerDeviceD3";
	case WdfPowerDeviceD3Final:
		return "WdfPowerDeviceD3Final";
	case WdfPowerDevicePrepareForHibernation:
		return "WdfPowerDevicePrepareForHibernation";
	case WdfPowerDeviceMaximum:
		return "WdfPowerDeviceMaximum";
	default:
		return "UnKnown Device Power State";
	}
}

NTSTATUS GCN_AdapterEvtDeviceD0Entry(
	WDFDEVICE aDevice, WDF_POWER_DEVICE_STATE aPreviousState)
{
	PDEVICE_CONTEXT pDeviceContext;
	NTSTATUS status;
	BOOLEAN isTargetStarted;
	
	pDeviceContext = DeviceGetContext(aDevice);
	isTargetStarted = FALSE;

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_POWER,
		"-->GCN_AdapterEvtEvtDeviceD0Entry - coming from %s\n",
		DbgDevicePowerString(aPreviousState));

	//Start the continuous reader here...
	status = WdfIoTargetStart(
		WdfUsbTargetPipeGetIoTarget(pDeviceContext->interruptReadPipe));
	status = STATUS_SUCCESS;
	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_POWER,
			"Failed to start interrupt pipe %!STATUS!\n", status);
		goto End;
	}

	isTargetStarted = TRUE;

End:

	if (!NT_SUCCESS(status))
	{
		// Kill the continuous reader since if this operation failed, the driver is being unloaded...
		if (isTargetStarted)
		{
			WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(
				pDeviceContext->interruptReadPipe), WdfIoTargetCancelSentIo);
		}
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_POWER,
		"<--GCN_AdapterEvtEvtDeviceD0Entry\n");

	return status;
}

NTSTATUS GCN_AdapterEvtDeviceD0Exit(WDFDEVICE aDevice, WDF_POWER_DEVICE_STATE aTargetState)
{
	PDEVICE_CONTEXT pDeviceContext;

	PAGED_CODE();

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_POWER,
		"-->GCN_AdapterEvtDeviceD0Exit - moving to %s\n",
		DbgDevicePowerString(aTargetState));

	pDeviceContext = DeviceGetContext(aDevice);

	WdfIoTargetStop(WdfUsbTargetPipeGetIoTarget(
		pDeviceContext->interruptReadPipe), WdfIoTargetCancelSentIo);

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_POWER,
		"<--GCN_AdapterEvtDeviceD0Exit\n");

	return STATUS_SUCCESS;
}
