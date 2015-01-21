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
	GCN_Controller_Null_NONE, GCN_Controller_Null_LINEAR
};

enum GCN_Controller_Axis
{
	GCN_Controller_Axis_Left = 0, GCN_Controller_Axis_Right = 1, GCN_Controller_Axis_SIZE = 2
};

enum GCN_Controller_Shoulder
{
	GCN_Controller_Shoulder_Left = 0, GCN_Controller_Shoulder_Right = 1, GCN_Controller_Shoulder_SIZE = 2
};

static void null_handle_shoulder(BYTE *axis, BYTE zero, double center) {}
static void null_handle_axis(BYTE axis[2], BYTE zero[2], double center[2]) {}
static void linear_handle_shoulder(BYTE *axis, BYTE zero, double center);
static void linear_handle_axis(BYTE axis[2], BYTE zero[2], double center[2]);

typedef struct _GCN_Controller_Status
{
	BYTE lastStatus;
	double axis_sensitivity[GCN_Controller_Axis_SIZE];
	double shoulder_sensitivity[GCN_Controller_Shoulder_SIZE];

	void(*function_axis[2])(BYTE axis[2], BYTE zero[2], double center[2]);
	void(*function_shoulder[2])(BYTE *axis, BYTE zero, double center);

} GCN_Controller_Status;

void GCN_Controller_Status_Init(GCN_Controller_Status *aControllerStatus);
void GCN_Controller_Change_Null_Control(GCN_Controller_Status *aStatus, enum GCN_Controller_Null_Control aEnum);
void GCN_Controller_Change_Null_Control_Axis(GCN_Controller_Status *aStatus, enum GCN_Controller_Axis aAxis, enum GCN_Controller_Null_Control aEnum);
void GCN_Controller_Change_Null_Control_Shoulder(GCN_Controller_Status *aStatus, enum GCN_Controller_Shoulder aShoulder, enum GCN_Controller_Null_Control aEnum);
void GCN_Controller_Change_Sensitivity_Control(GCN_Controller_Status *aStatus, BYTE aSensitivity);
void GCN_Controller_Change_Sensitivity_Control_Axis(GCN_Controller_Status *aStatus, enum GCN_Controller_Axis aAxis, BYTE aSensitivity);
void GCN_Controller_Change_Sensitivity_Control_Shoulder(GCN_Controller_Status *aStatus, enum GCN_Controller_Shoulder aShoulder, BYTE aSensitivity);

struct _DEVICE_CONTEXT;
typedef struct _DEVICE_CONTEXT DEVICE_CONTEXT;

void prepare_report(
	DEVICE_CONTEXT *apDeviceContext,
	GCN_AdapterData *in,
	GCN_ControllerReport *out);

#endif//_GCN_ADAPTOR_GCN_CONTROLLER_H_