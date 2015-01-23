#include "Include.h"
#include "gcn_controller.tmh"

static void null_handle_shoulder(BYTE *axis, BYTE zero, double center, double sensitivity) {}
static void null_handle_axis(BYTE axis[2], BYTE zero[2], double center[2], double sensitivity) {}
static void linear_handle_shoulder(BYTE *axis, BYTE zero, double center, double sensitivity);
static void linear_handle_axis(BYTE axis[2], BYTE zero[2], double center[2], double sensitivity);

#define DEADZONE 35

GCN_Controller_Status GCN_Controller_Status_Zero =
{
	.lastControllerState = 0,
	.rumble = 0,
	.deadzone.axis = { { .mode = GCN_Controller_Deadzone_LINEAR, .deadzone = DEADZONE }, { .mode = (BYTE)GCN_Controller_Deadzone_LINEAR, .deadzone = DEADZONE } },
	.deadzone.shoulder = { { .mode = GCN_Controller_Deadzone_LINEAR, .deadzone = DEADZONE }, { .mode = (BYTE)GCN_Controller_Deadzone_LINEAR, .deadzone = DEADZONE } },
	.function_axis = { linear_handle_axis, linear_handle_axis },
	.function_shoulder = { linear_handle_shoulder, linear_handle_shoulder}
};

void GCN_Controller_Status_Init(GCN_Controller_Status *aControllerStatus)
{
	*aControllerStatus = GCN_Controller_Status_Zero;
}

void GCN_Controller_Status_Update_Deadzone(GCN_Controller_Status *apControllerStatus, GCN_Controller_Deadzone_Status *apNewStatus)
{
	int i = 0;
	apControllerStatus->deadzone = *apNewStatus;

	for (; i < 2; ++i)
	{
		switch (apControllerStatus->deadzone.axis[i].mode)
		{
		case GCN_Controller_Deadzone_NONE:
			apControllerStatus->function_axis[i] = null_handle_axis;
			break;
		case GCN_Controller_Deadzone_LINEAR:
			apControllerStatus->function_axis[i] = linear_handle_axis;
			break;

		default:
			break;
		}

		switch (apControllerStatus->deadzone.shoulder[i].mode)
		{
		case GCN_Controller_Deadzone_NONE:
			apControllerStatus->function_shoulder[i] = null_handle_shoulder;
			break;
		case GCN_Controller_Deadzone_LINEAR:
			apControllerStatus->function_shoulder[i] = linear_handle_shoulder;
			break;

		default:
			break;
		}
	}
}

static GCN_ControllerReport GCN_AdapterControllerZero = {
	.id = 0,
	.input.buttons = { 0, 0 },
	.input.axis = { { 127, 127 }, { 127, 127 } },
	.input.shoulder = { 0, 0 }
};

void GCN_Adapter_Rumble_Completion(
	_In_  WDFREQUEST aRequest,
	_In_  WDFIOTARGET aTarget,
	_In_  PWDF_REQUEST_COMPLETION_PARAMS apParams,
	_In_  WDFCONTEXT aContext)
{
	NTSTATUS status = WdfRequestGetStatus(aRequest);
	//TODO do something, tracing, error checking?
	//How about updating rumble status?
}

NTSTATUS GCN_Adapter_Rumble(PDEVICE_CONTEXT apDeviceContext, BYTE aRumble)
{
	NTSTATUS status;
	GCN_AdapterData adapterData;
	WDF_REQUEST_REUSE_PARAMS params;
	BYTE *data = WdfMemoryGetBuffer(apDeviceContext->rumbleMemory, NULL);
	BYTE i, newStatus = 0;

	WdfSpinLockAcquire(apDeviceContext->dataLock);
	adapterData = apDeviceContext->adapterData;
	WdfSpinLockRelease(apDeviceContext->dataLock);

	if ((adapterData.port[0].status.powered && (apDeviceContext->rumbleStatus != aRumble)) || (apDeviceContext->rumbleStatus && !adapterData.port[0].status.powered))
	{
		if (adapterData.port[0].status.powered)
		{
			newStatus = aRumble;
		}
		else
		{
			newStatus = 0;
		}

		for (i = 0; i < 4; ++i)
		{
			data[i + 1] = apDeviceContext->controllerStatus[i].rumble = (newStatus >> i) & 0x01;
		}

		WDF_REQUEST_REUSE_PARAMS_INIT(&params, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
		WdfRequestReuse(apDeviceContext->rumbleRequest, &params);

		status = WdfUsbTargetPipeFormatRequestForWrite(apDeviceContext->interruptWritePipe, apDeviceContext->rumbleRequest, apDeviceContext->rumbleMemory, NULL);
		if (!NT_SUCCESS(status))
		{
			goto Exit;
		}
		WdfRequestSetCompletionRoutine(apDeviceContext->rumbleRequest, GCN_Adapter_Rumble_Completion, NULL);

		WdfRequestSend(apDeviceContext->rumbleRequest, WdfUsbTargetPipeGetIoTarget(apDeviceContext->interruptWritePipe), NULL);
		apDeviceContext->rumbleStatus = newStatus; //Should it be protected?
	}
	else
	{
		status = STATUS_SUCCESS;
	}

Exit:
	//FIXME add tracing?
	return status;
}

NTSTATUS GCN_Controller_Rumble(PDEVICE_CONTEXT _In_ apDeviceContext, int _In_ aIndex, BOOLEAN _In_ aRumble)
{
	NTSTATUS status = STATUS_BAD_DATA;
	if (aIndex < 4)
	{
		BYTE rumble = apDeviceContext->rumbleStatus;
		
		if (aIndex < 0)
		{
			rumble = aRumble ? 0xf : 0;
		}
		else if (aRumble)
		{
			rumble |= 1 << aIndex;
		}
		else
		{
			rumble &= ~(1 << aIndex);
		}

		status = GCN_Adapter_Rumble(apDeviceContext, rumble);
	}
	return status;
}

NTSTATUS GCN_Controller_Calibrate(PDEVICE_CONTEXT _In_ apDeviceContext, int _In_ aIndex)
{
	NTSTATUS status = STATUS_SUCCESS;
	GCN_AdapterData calibrationData;

	WdfSpinLockAcquire(apDeviceContext->dataLock);
	calibrationData = apDeviceContext->adapterData;
	WdfSpinLockRelease(apDeviceContext->dataLock);

	if (aIndex < 0)
	{
		apDeviceContext->calibrationData = calibrationData;
	}
	else if (aIndex < 4)
	{
		apDeviceContext->calibrationData.port[aIndex] = calibrationData.port[aIndex];
	}
	else
	{
		status = STATUS_BUFFER_OVERFLOW;
		TraceEvents(TRACE_LEVEL_ERROR, TRACE_GCN_CONTROLLER,
			"!FUNC! bad index received! %!STATUS!\n", status);
	}

	return status;
}

static __inline double dist_2d(double vector1[2], double vector2[2])
{
	//sqrt is an intrinsic function of the compiler, how convenient
	return sqrt((vector1[0] - vector2[0])*(vector1[0] - vector2[0]) + (vector1[1] - vector2[1])*(vector1[1] - vector2[1]));
}

static __inline double scaled_value(double distance, double sensitivity)
{
	return (distance/255. - sensitivity) / (1 - sensitivity);
}

static void linear_handle_axis(BYTE axis[2], BYTE zero[2], double center[2], double sensitivity)
{
	double newAxis[2] = { axis[0], axis[1] };
	double distance = dist_2d( newAxis, center);
	if (abs(distance) < sensitivity * 255.) //abs is a compiler intrinsic function
	{
		distance = 0;
		memcpy(axis, zero, 2);
	}
	else
	{
		double newMagnitude = scaled_value(distance, sensitivity);
		axis[0] = (BYTE)(255 * newMagnitude * (newAxis[0] - center[0]) / distance + zero[0]);
		axis[1] = (BYTE)(255 * newMagnitude * (newAxis[1] - center[1]) / distance + zero[1]);
	}
}

static void linear_handle_shoulder(BYTE *axis, BYTE zero, double center, double sensitivity)
{
	double distance = *axis - center;
	if (distance < sensitivity * 255)
	{
		distance = 0;
		*axis = zero;
	}
	else
	{
		double newMagnitude = scaled_value(distance, sensitivity);
		*axis = (BYTE)(255 * newMagnitude * (*axis - center) / distance + zero);
	}
}

static void handle_null_zones(GCN_Controller_Input _In_ *apCal, GCN_Controller_Status _In_ *apStatus, GCN_Controller_Input _Inout_ *apOutput)
{
	double center[3][2] =
	{
		{ apCal->axis[0].X, apCal->axis[0].Y },
		{ apCal->axis[1].X, apCal->axis[1].Y },
		{ apCal->shoulder[0], apCal->shoulder[1] }
	};
	//Handle null zones
	apStatus->function_axis[0]((BYTE*)&apOutput->axis[0], (BYTE*)&GCN_AdapterControllerZero.input.axis[0], center[0], apStatus->deadzone.axis[0].deadzone / 255.);
	apStatus->function_axis[1]((BYTE*)&apOutput->axis[1], (BYTE*)&GCN_AdapterControllerZero.input.axis[1], center[1], apStatus->deadzone.axis[1].deadzone / 255.);
	apStatus->function_shoulder[0](&apOutput->shoulder[0], GCN_AdapterControllerZero.input.shoulder[0], center[2][0], apStatus->deadzone.shoulder[0].deadzone / 255.);
	apStatus->function_shoulder[1](&apOutput->shoulder[1], GCN_AdapterControllerZero.input.shoulder[1], center[2][1], apStatus->deadzone.shoulder[1].deadzone / 255.);
}

//This needs to cycle through the 4 controllers
void prepare_report(
	PDEVICE_CONTEXT apDeviceContext,
	GCN_AdapterData *in,
	GCN_ControllerReport *out)
{
	//ID should loop from 1 to 4, inclusive ( [1, 4] )
	static BYTE id = 1;
	GCN_Controller_Status status = apDeviceContext->controllerStatus[id - 1];
	GCN_AdapterData *cal = &apDeviceContext->calibrationData, data;
	WDFSPINLOCK *lock = &apDeviceContext->dataLock;
	
	if (!in->port[id - 1].status.type)
	{
		memcpy(out, &GCN_AdapterControllerZero, sizeof(*out));
		if (apDeviceContext->controllerStatus[id - 1].lastControllerState)
		{
			apDeviceContext->controllerStatus[id - 1].lastControllerState = 0;
		}
	}
	else
	{
		data = *in;
		memcpy(&(out->input), &(data.port[id - 1].input), sizeof(out->input));

		out->input.axis[0].Y = ~out->input.axis[0].Y;

		handle_null_zones(&cal->port[id-1].input, &status, &out->input);

		//Detect if the device was previously turned off. If so, calibrate
		if (!apDeviceContext->controllerStatus[id - 1].lastControllerState)
		{
			GCN_Controller_Calibrate(apDeviceContext, id - 1);
			apDeviceContext->controllerStatus[id - 1].lastControllerState = 1;
		}
	}

	out->id = id;
	id = (id % 4) + 1;
}
