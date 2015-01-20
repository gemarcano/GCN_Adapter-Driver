#include "Include.h"
#include "GCN_Controller.h"

#define DEADZONE 35

GCN_Controller_Status GCN_Controller_Status_Zero =
	{ { DEADZONE / 255., DEADZONE / 255. }, { DEADZONE / 255., DEADZONE / 255. }, { linear_handle_axis, linear_handle_axis }, { linear_handle_shoulder, linear_handle_shoulder } };

void GCN_Controller_Status_Init(GCN_Controller_Status *aControllerStatus)
{
	*aControllerStatus = GCN_Controller_Status_Zero;
}

static GCN_ControllerReport GCN_AdapterControllerZero = {
	0,
	{ { 0 }, { 0 }, { 127, 127 }, { 127, 127 }, { 0, 0 } }
};

void GCN_Controller_Change_Null_Control(GCN_Controller_Status *aStatus, enum GCN_Controller_Null_Control aEnum)
{
	int i = 0;
	for (; i < 2; ++i)
	{
		GCN_Controller_Change_Null_Control_Axis(aStatus, i, aEnum);
		GCN_Controller_Change_Null_Control_Shoulder(aStatus, i, aEnum);
	}
}

void GCN_Controller_Change_Null_Control_Axis(GCN_Controller_Status *aStatus, enum GCN_Controller_Axis aAxis, enum GCN_Controller_Null_Control aEnum)
{
	switch (aEnum)
	{
	case GCN_Controller_Null_LINEAR:
		aStatus->function_axis[aAxis] = linear_handle_axis;
		break;
	case GCN_Controller_Null_NONE:
	default:
		aStatus->function_axis[aAxis] = null_handle_axis;
	}
}

void GCN_Controller_Change_Null_Control_Shoulder(GCN_Controller_Status *aStatus, enum GCN_Controller_Shoulder aShoulder, enum GCN_Controller_Null_Control aEnum)
{
	switch (aEnum)
	{
	case GCN_Controller_Null_LINEAR:
		aStatus->function_shoulder[aShoulder] = linear_handle_shoulder;
		break;
	case GCN_Controller_Null_NONE:
	default:
		aStatus->function_shoulder[aShoulder] = null_handle_shoulder;
	}
}

void GCN_Controller_Change_Sensitivity_Control(GCN_Controller_Status *aStatus, BYTE aSensitivity)
{
	int i = 0;
	for (; i < 2; ++i)
	{
		GCN_Controller_Change_Sensitivity_Control_Axis(aStatus, i, aSensitivity);
		GCN_Controller_Change_Sensitivity_Control_Shoulder(aStatus, i, aSensitivity);
	}
}

void GCN_Controller_Change_Sensitivity_Control_Axis(GCN_Controller_Status *aStatus, enum GCN_Controller_Axis aAxis, BYTE aSensitivity)
{
	int i = 0;
	for (; i < GCN_Controller_Axis_SIZE; ++i)
	{
		aStatus->axis_sensitivity[aAxis] = aSensitivity / 255.;
	}
}

void GCN_Controller_Change_Sensitivity_Control_Shoulder(GCN_Controller_Status *aStatus, enum GCN_Controller_Shoulder aShoulder, BYTE aSensitivity)
{
	int i = 0;
	for (; i < GCN_Controller_Shoulder_SIZE; ++i)
	{
		aStatus->shoulder_sensitivity[aShoulder] = aSensitivity / 255.;
	}
}

__inline double dist_2d(double vector1[2], double vector2[2])
{
	//sqrt is an intrinsic function of the compiler, how convenient
	return sqrt((vector1[0] - vector2[0])*(vector1[0] - vector2[0]) + (vector1[1] - vector2[1])*(vector1[1] - vector2[1]));
}

__inline double scaled_value(double distance)
{
	return (distance - DEADZONE) / (255 - DEADZONE);
}

void linear_handle_axis(BYTE axis[2], BYTE zero[2], double center[2])
{
	double newAxis[2] = { axis[0], axis[1] };
	double distance = dist_2d( newAxis, center);
	if (abs(distance) < DEADZONE) //abs is a compiler intrinsic function
	{
		distance = 0;
		memcpy(axis, zero, 2);
	}
	else
	{
		double newMagnitude = scaled_value(distance);
		axis[0] = (BYTE)(255 * newMagnitude * (newAxis[0] - center[0]) / distance + zero[0]);
		axis[1] = (BYTE)(255 * newMagnitude * (newAxis[1] - center[1]) / distance + zero[1]);
	}
}

void linear_handle_shoulder(BYTE *axis, BYTE zero, double center)
{
	double distance = *axis - center;
	if (distance < DEADZONE)
	{
		distance = 0;
		*axis = zero;
	}
	else
	{
		double newMagnitude = scaled_value(distance);
		*axis = (BYTE)(255 * newMagnitude * (*axis - center) / distance + zero);
	}
}

//This needs to cycle through the 4 controllers
void prepare_report(
	GCN_Controller_Status aStatus[4],
	GCN_AdapterData *cal,
	GCN_AdapterData *in,
	GCN_ControllerReport *out,
	WDFSPINLOCK *lock)
{
	//ID should loop from 1 to 4, inclusive ( [1, 4] )
	static BYTE id = 1;
	GCN_Controller_Status status = aStatus[id - 1];

	double center[3][2] =
	{
		{ cal->Port[id - 1].Buttons.LeftAxis.X, cal->Port[id - 1].Buttons.LeftAxis.Y },
		{ cal->Port[id - 1].Buttons.RightAxis.X, cal->Port[id - 1].Buttons.RightAxis.Y },
		{ cal->Port[id - 1].Buttons.ShoulderAxis.left, cal->Port[id - 1].Buttons.ShoulderAxis.right }
	};
	
	if (!in->Port[id - 1].Status.type)
	{
		memcpy(out, &GCN_AdapterControllerZero, sizeof(*out));
	}
	else
	{
		WdfSpinLockAcquire(*lock);
		memcpy(&(out->Buttons), &(in->Port[id - 1].Buttons), sizeof(out->Buttons));
		WdfSpinLockRelease(*lock);
		out->Buttons.LeftAxis.Y = ~out->Buttons.LeftAxis.Y;

		//Handle null zones
		status.function_axis[0]((BYTE *)&out->Buttons.LeftAxis, (BYTE *)&GCN_AdapterControllerZero.Buttons.LeftAxis, center[0]);
		status.function_axis[1]((BYTE *)&out->Buttons.RightAxis, (BYTE *)&GCN_AdapterControllerZero.Buttons.RightAxis, center[1]);
		status.function_shoulder[0](&out->Buttons.ShoulderAxis.left, GCN_AdapterControllerZero.Buttons.ShoulderAxis.left, center[2][0]);
		status.function_shoulder[1](&out->Buttons.ShoulderAxis.right, GCN_AdapterControllerZero.Buttons.ShoulderAxis.right, center[2][1]);
	}

	out->id = id;
	id = (id % 4) + 1;
}
