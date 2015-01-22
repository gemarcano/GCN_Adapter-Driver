#pragma once
#ifndef _GCN_ADAPTOR_GCN_CONTROLLER_H_
#define _GCN_ADAPTOR_GCN_CONTROLLER_H_

#include "Device.h"

typedef struct _GCN_Controller_Port_Buttons
{
	struct {
		BYTE a : 1;
		BYTE b : 1;
		BYTE x : 1;
		BYTE y : 1;
		BYTE d_left : 1;
		BYTE d_right : 1;
		BYTE d_down : 1;
		BYTE d_up : 1;
	} Buttons1;
	struct {
		BYTE start : 1;
		BYTE z : 1;
		BYTE r : 1;
		BYTE l : 1;
		BYTE: 4;
	} Buttons2;
	struct {
		BYTE X, Y;
	} LeftAxis;

	struct {
		BYTE X, Y;
	} RightAxis;

	struct {
		BYTE left, right;
	} ShoulderAxis;
} GCN_Controller_Port_Buttons;

typedef struct _GCN_AdapterData
{
	BYTE Signal;

	struct {
		struct
		{
			BYTE:2;
			BYTE powered : 1;
			BYTE:1;
			BYTE type : 2;
			BYTE:2;
		} Status;
		GCN_Controller_Port_Buttons Buttons;
	}Port[4];
} GCN_AdapterData;

typedef struct _GCN_ControllerReport
{
	BYTE id;
	GCN_Controller_Port_Buttons Buttons;
} GCN_ControllerReport;

typedef struct _GCN_AdapterReport
{
	GCN_ControllerReport Port[4];
} GCN_AdapterReport;

extern GCN_ControllerReport GCN_AdapterControllerZero;

enum GCN_Controller_Null_Control
{
	GCN_Controller_Null_NONE = 0, GCN_Controller_Null_LINEAR = 1
};

enum GCN_Controller_Axis
{
	GCN_Controller_Axis_Left = 0, GCN_Controller_Axis_Right = 1, GCN_Controller_Axis_SIZE = 2
};

enum GCN_Controller_Shoulder
{
	GCN_Controller_Shoulder_Left = 0, GCN_Controller_Shoulder_Right = 1, GCN_Controller_Shoulder_SIZE = 2
};

typedef IOCTL_GCN_Adapter_Deadzone_Controller_Data GCN_Controller_Deadzone_Status;

typedef struct _GCN_Controller_Status
{
	BYTE lastStatus;

	GCN_Controller_Deadzone_Status deadzone;
	BYTE rumble;

	void(*function_axis[2])(BYTE axis[2], BYTE zero[2], double center[2], double sensitivity);
	void(*function_shoulder[2])(BYTE *axis, BYTE zero, double center, double sensitivity);

} GCN_Controller_Status;

void GCN_Controller_Status_Init(GCN_Controller_Status *aControllerStatus);
void GCN_Controller_Status_Update_Deadzone(GCN_Controller_Status *aControllerStatus, GCN_Controller_Deadzone_Status *aNewStatus);

struct _DEVICE_CONTEXT;
typedef struct _DEVICE_CONTEXT DEVICE_CONTEXT;

NTSTATUS GCN_Adapter_Rumble(DEVICE_CONTEXT *apDeviceContext, BYTE aRumble);
NTSTATUS GCN_Controller_Rumble(DEVICE_CONTEXT *apDeviceContext, BYTE aIndex, BYTE aRumble);


void prepare_report(
	DEVICE_CONTEXT *apDeviceContext,
	GCN_AdapterData *in,
	GCN_ControllerReport *out);

#endif//_GCN_ADAPTOR_GCN_CONTROLLER_H_