#include "Include.h"
#include "GCN_Controller.h"

static void null_handle_shoulder(BYTE *axis, BYTE zero, double center, double sensitivity) {}
static void null_handle_axis(BYTE axis[2], BYTE zero[2], double center[2], double sensitivity) {}
static void linear_handle_shoulder(BYTE *axis, BYTE zero, double center, double sensitivity);
static void linear_handle_axis(BYTE axis[2], BYTE zero[2], double center[2], double sensitivity);

#define DEADZONE 35

GCN_Controller_Status GCN_Controller_Status_Zero =
{
	.lastStatus = 0,
	.rumble = 0,
	.deadzone =
	{
		.axis = { { .mode = GCN_Controller_Null_LINEAR, .deadzone = DEADZONE }, { .mode = (BYTE)GCN_Controller_Null_LINEAR, .deadzone = DEADZONE } },
		.shoulder = { { .mode = GCN_Controller_Null_LINEAR, .deadzone = DEADZONE }, { .mode = (BYTE)GCN_Controller_Null_LINEAR, .deadzone = DEADZONE } }
	},
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
		case GCN_Controller_Null_NONE:
			apControllerStatus->function_axis[i] = null_handle_axis;
			break;
		case GCN_Controller_Null_LINEAR:
			apControllerStatus->function_axis[i] = linear_handle_axis;
			break;

		default:
			break;
		}
	}

	for (i = 0; i < 2; ++i)
	{
		switch (apControllerStatus->deadzone.shoulder[i].mode)
		{
		case GCN_Controller_Null_NONE:
			apControllerStatus->function_shoulder[i] = null_handle_shoulder;
			break;
		case GCN_Controller_Null_LINEAR:
			apControllerStatus->function_shoulder[i] = linear_handle_shoulder;
			break;

		default:
			break;
		}
	}
}

static GCN_ControllerReport GCN_AdapterControllerZero = {
	0,
	{ { 0 }, { 0 }, { 127, 127 }, { 127, 127 }, { 0, 0 } }
};

void GCN_Adapter_Rumble_Completion(
	_In_  WDFREQUEST Request,
	_In_  WDFIOTARGET Target,
	_In_  PWDF_REQUEST_COMPLETION_PARAMS Params,
	_In_  WDFCONTEXT Context
	)
{
	NTSTATUS status = WdfRequestGetStatus(Request);
	//TODO do something, tracing, error checking?
	//How about updating rumble status?
}

//FIXME 
NTSTATUS GCN_Adapter_Rumble(PDEVICE_CONTEXT apDeviceContext, BYTE aRumble)
{
	NTSTATUS status;
	GCN_AdapterData adapterData;
	DWORD written = 0;
	WDF_REQUEST_REUSE_PARAMS params;
	WDFMEMORY buffer;
	BYTE *data = WdfMemoryGetBuffer(apDeviceContext->rumbleMemory, NULL);

	WdfSpinLockAcquire(apDeviceContext->dataLock);
	adapterData = apDeviceContext->adapterData;
	WdfSpinLockRelease(apDeviceContext->dataLock);

	data[1] = apDeviceContext->controllerStatus[0].rumble = aRumble & 0x1 * adapterData.Port[0].Status.powered;
	data[2] = apDeviceContext->controllerStatus[1].rumble = aRumble & 0x2 * adapterData.Port[1].Status.powered;
	data[3] = apDeviceContext->controllerStatus[2].rumble = aRumble & 0x4 * adapterData.Port[2].Status.powered;
	data[4] = apDeviceContext->controllerStatus[3].rumble = aRumble & 0x8 * adapterData.Port[3].Status.powered;


	WDF_REQUEST_REUSE_PARAMS_INIT(&params, WDF_REQUEST_REUSE_NO_FLAGS, STATUS_SUCCESS);
	WdfRequestReuse(apDeviceContext->rumbleRequest, &params);

	status = WdfUsbTargetPipeFormatRequestForWrite(apDeviceContext->interruptWritePipe, apDeviceContext->rumbleRequest, apDeviceContext->rumbleMemory, NULL);
	if (!NT_SUCCESS(status))
	{
		goto Exit;
	}
	WdfRequestSetCompletionRoutine(apDeviceContext->rumbleRequest, GCN_Adapter_Rumble_Completion, NULL);

	status = WdfRequestSend(apDeviceContext->rumbleRequest, WdfUsbTargetPipeGetIoTarget(apDeviceContext->interruptWritePipe), NULL);

Exit:
	return status;
}

NTSTATUS GCN_Controller_Rumble(PDEVICE_CONTEXT apDeviceContext, BYTE aIndex, BYTE aRumble)
{
	NTSTATUS status = STATUS_BAD_DATA;
	if (aIndex < 4)
	{
		BYTE rumble = 0, i;
		apDeviceContext->controllerStatus[aIndex].rumble = !!aRumble;
		for (i = 0; i < 4; ++i)
		{
			rumble |= apDeviceContext->controllerStatus[i].rumble << i;
		}

		status = GCN_Adapter_Rumble(apDeviceContext, rumble);
	}
	return status;
}

__inline double dist_2d(double vector1[2], double vector2[2])
{
	//sqrt is an intrinsic function of the compiler, how convenient
	return sqrt((vector1[0] - vector2[0])*(vector1[0] - vector2[0]) + (vector1[1] - vector2[1])*(vector1[1] - vector2[1]));
}

__inline double scaled_value(double distance, double sensitivity)
{
	return (distance/255. - sensitivity) / (1 - sensitivity);
}

void linear_handle_axis(BYTE axis[2], BYTE zero[2], double center[2], double sensitivity)
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

void linear_handle_shoulder(BYTE *axis, BYTE zero, double center, double sensitivity)
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

	double center[3][2] =
	{
		{ cal->Port[id - 1].Buttons.LeftAxis.X, cal->Port[id - 1].Buttons.LeftAxis.Y },
		{ cal->Port[id - 1].Buttons.RightAxis.X, cal->Port[id - 1].Buttons.RightAxis.Y },
		{ cal->Port[id - 1].Buttons.ShoulderAxis.left, cal->Port[id - 1].Buttons.ShoulderAxis.right }
	};
	
	if (!in->Port[id - 1].Status.type)
	{
		memcpy(out, &GCN_AdapterControllerZero, sizeof(*out));
		if (apDeviceContext->controllerStatus[id - 1].lastStatus)
		{
			apDeviceContext->controllerStatus[id - 1].lastStatus = 0;
		}
	}
	else
	{//FIXME I should move the spinlock out of here up the stack
		WdfSpinLockAcquire(*lock);
		data = *in;
		WdfSpinLockRelease(*lock);
		memcpy(&(out->Buttons), &(data.Port[id - 1].Buttons), sizeof(out->Buttons));

		out->Buttons.LeftAxis.Y = ~out->Buttons.LeftAxis.Y;

		//Handle null zones
		status.function_axis[0]((BYTE *)&out->Buttons.LeftAxis, (BYTE *)&GCN_AdapterControllerZero.Buttons.LeftAxis, center[0], status.deadzone.axis[0].deadzone/255.);
		status.function_axis[1]((BYTE *)&out->Buttons.RightAxis, (BYTE *)&GCN_AdapterControllerZero.Buttons.RightAxis, center[1], status.deadzone.axis[1].deadzone / 255.);
		status.function_shoulder[0](&out->Buttons.ShoulderAxis.left, GCN_AdapterControllerZero.Buttons.ShoulderAxis.left, center[2][0], status.deadzone.shoulder[0].deadzone / 255.);
		status.function_shoulder[1](&out->Buttons.ShoulderAxis.right, GCN_AdapterControllerZero.Buttons.ShoulderAxis.right, center[2][1], status.deadzone.shoulder[1].deadzone / 255.);

		if (!apDeviceContext->controllerStatus[id - 1].lastStatus)
		{
			GCN_AdapterFetchCalibrationData(apDeviceContext, id - 1);
			apDeviceContext->controllerStatus[id - 1].lastStatus = 1;
		}

		//Turn rumble off if it is enabled and power is removed
		if (apDeviceContext->controllerStatus[id - 1].rumble && !data.Port[id-1].Status.powered)
		{
			GCN_Controller_Rumble(apDeviceContext, id - 1, 0);
		}
	}

	out->id = id;
	id = (id % 4) + 1;
}
