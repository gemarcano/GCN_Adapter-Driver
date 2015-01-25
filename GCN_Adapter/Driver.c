#include "Include.h"
#include "driver.tmh"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, GCN_AdapterEvtDriverDeviceAdd)
#pragma alloc_text (PAGE, GCN_AdapterEvtDriverContextCleanup)
#endif

NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT  aDriverObject,
	_In_ PUNICODE_STRING aRegistryPath)
{
	WDF_DRIVER_CONFIG config;
	NTSTATUS status;
	WDF_OBJECT_ATTRIBUTES attributes;

	WPP_INIT_TRACING( aDriverObject, aRegistryPath );

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

	//
	// Register a cleanup callback so that we can call WPP_CLEANUP when
	// the framework driver object is deleted during driver unload.
	//
	WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
	attributes.EvtCleanupCallback = GCN_AdapterEvtDriverContextCleanup;

	WDF_DRIVER_CONFIG_INIT(&config, GCN_AdapterEvtDriverDeviceAdd);
	
	status = WdfDriverCreate(
		aDriverObject,
		aRegistryPath,
		&attributes,
		&config,
		WDF_NO_HANDLE);

	if (!NT_SUCCESS(status))
	{
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "WdfDriverCreate failed %!STATUS!", status);
		WPP_CLEANUP(aDriverObject);
		return status;
	}

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");

	return status;
}

NTSTATUS GCN_AdapterEvtDriverDeviceAdd(
	_In_    WDFDRIVER       aDriver,
	_Inout_ PWDFDEVICE_INIT aDeviceInit)
{
	NTSTATUS status;

	UNREFERENCED_PARAMETER(aDriver);

	PAGED_CODE();

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

	// This is a filter driver for the supplied HID minidriver (Win 7 and up)
	WdfFdoInitSetFilter(aDeviceInit);

	status = GCN_AdapterCreateDevice(aDeviceInit);

	if (status != STATUS_SUCCESS)
	{
		TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
			"GCN_AdapterCreateDevice failed!: %!STATUS! Exit", status);
		goto Exit;
	}

Exit:

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Exit");
	return status;
}

VOID GCN_AdapterEvtDriverContextCleanup(
	_In_ WDFOBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	PAGED_CODE ();

	TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Entry");

	WPP_CLEANUP( WdfDriverWdmGetDriverObject(DriverObject) );
}
