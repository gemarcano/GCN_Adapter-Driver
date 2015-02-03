#pragma once
#ifndef _GCN_ADAPTOR_GCN_CONTROLLER_H_
#define _GCN_ADAPTOR_GCN_CONTROLLER_H_

/**	@file
 *	This file contains function and variable declarations/definitions regarding
 *	controller specific functionality.
 *
 */

#include "Include.h"

typedef struct _DEVICE_CONTEXT DEVICE_CONTEXT;

enum GCN_Controller_Axis
{
	GCN_Controller_Axis_Left = 0,
	GCN_Controller_Axis_Right = 1,
	GCN_Controller_Axis_SIZE = 2
};

enum GCN_Controller_Shoulder
{
	GCN_Controller_Shoulder_Left = 0,
	GCN_Controller_Shoulder_Right = 1,
	GCN_Controller_Shoulder_SIZE = 2
};

//Describes the incoming data for the controller
typedef struct _GCN_Controller_Input
{
	struct
	{
		BYTE a : 1;
		BYTE b : 1;
		BYTE x : 1;
		BYTE y : 1;
		BYTE d_left : 1;
		BYTE d_right : 1;
		BYTE d_down : 1;
		BYTE d_up : 1;
	
		BYTE start : 1;
		BYTE z : 1;
		BYTE r : 1;
		BYTE l : 1;
		BYTE : 4;
	} buttons;

	struct
	{
		BYTE X, Y;
	} axis[GCN_Controller_Axis_SIZE];

	BYTE shoulder[GCN_Controller_Shoulder_SIZE];
} GCN_Controller_Input;

//Start HID section of header --------------------------------------------------

//Describes report coming in from device
typedef struct _GCN_AdapterData
{
	BYTE signal;

	struct
	{
		struct
		{
			BYTE:2;
			BYTE powered : 1;
			BYTE:1;
			BYTE type : 2;
			BYTE:2;
		} status;
		GCN_Controller_Input input;
	} port[4];
} GCN_AdapterData;

//Describes report for a single controller
typedef struct _GCN_ControllerReport
{
	BYTE id;
	GCN_Controller_Input input;
} GCN_ControllerReport;

//Describes report leaving driver to HID minidriver
typedef struct _GCN_AdapterReport
{
	GCN_ControllerReport port[4];
} GCN_AdapterReport;

//HID section end --------------------------------------------------------------

enum GCN_Controller_Deadzone_Control
{
	GCN_Controller_Deadzone_NONE = 0,
	GCN_Controller_Deadzone_LINEAR = 1
};

typedef IOCTL_GCN_Adapter_Deadzone_Controller_Data
	GCN_Controller_Deadzone_Status;

typedef struct _GCN_Controller_Status
{
	BYTE lastControllerState;

	GCN_Controller_Deadzone_Status deadzone;
	BYTE rumble;

	void(*function_axis[2])(
		BYTE axis[2], BYTE zero[2], double center[2], double sensitivity);
	void(*function_shoulder[2])(
		BYTE *axis, BYTE zero, double center, double sensitivity);

} GCN_Controller_Status;

/**	@brief Initializes the data  of a Controller's status variable to default
 *		values.
 *
 *	The default initialization values are as follows:
 *		Rumble is disabled.
 *		Deadzone processsing is set to linear for all axis and shoulders.
 *		Deadzone value is set to 35.
 *
 *	@param [out] aControllerStatus Pointer to a GCN_Controller_Status structure
 *		to be initialized.
 *
 *	@remark This function can run at any IRQL (is this true?). Do make sure that
 *		the data being passed into the function is not paged out if running at
 *		an IRQL > PASSIVE_LEVEL.
 *	@remark This function is not paged.
 *
 *	@post The data structure passed in is initialized to default values.
 *
 */
void GCN_Controller_Status_Init(GCN_Controller_Status *aControllerStatus);

/**	@brief Updates the deadzone parameters (deadzone sensitivity and operating
 *		mode).
 *
 *	@param [in, out] aControllerStatus Pointer to a GCN_Controller_Status
 *		structure of the controller to have its deadzone parameters modified.
 *	@param [in] aNewStatus Pointer to a GCN_Controller_Deadzone_Status
 *		describing the new deadzone state for the specified controller.
 *
 *	@remark This function can run at any IRQL (is this true?). Do make sure that
 *		the data being passed into the function is not paged out if running at
 *		an IRQL > PASSIVE_LEVEL.
 *	@remark This function is not paged.
 *
 */
void GCN_Controller_Status_Update_Deadzone(
	GCN_Controller_Status *aControllerStatus,
	GCN_Controller_Deadzone_Status *aNewStatus);

/**	@brief Uses the lower four bytes of aRumble to determine the current status
 *		of each of the controllers' rumble and updates them accordingly.
 *
 *	@param [in, out] apDeviceContext Pointer to the driver Device context.
 *	@param [in]	aRumble Determines rumble state for the controllers. Bits 0
 *		through 3 control controllers 1 through 4, in that order.
 *
 *	@remark This function can run at any IRQL <= DISPATCH_LEVEL.
 *	@remark This function is not paged.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
NTSTATUS GCN_Adapter_Rumble(DEVICE_CONTEXT *apDeviceContext, BYTE aRumble);

/**	@brief Triggers rumble for the specified controller at index [0, 3].
 *
 *	@param [in, out] apDeviceContext Pointer to the driver Device context.
 *	@param [in]	aIndex Determines which controller to set rumble for. Setting
 *		this variable to -1 causes the given setting to affect all controllers.
 *	@param [in]	aRumble The new state for rumble for the controller/s specified.
 *		Rumble is activated if the variable is true, deactivated otherwise.
 *
 *	@remark This function can run at any IRQL <= DISPATCH_LEVEL.
 *	@remark This function is not paged.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *	@post On returning STATUS_SUCCESS, the controllers rumble setting has been
 *		updated as specified in the aRumble variable.
 *
 */
NTSTATUS GCN_Controller_Rumble(
	DEVICE_CONTEXT *apDeviceContext, int aIndex, BOOLEAN aRumble);

/**	Helper that sets the USB device's calibration values from current values.
 *
 *	@param [in] apDeviceContext Device context used by driver.
 *	@param [in] aIndex Index of port to calibrate [ 0, 3 ]. A -1 indicates to
 *		calibrate all ports.
 *
 *	@remark This function can run at any IRQL <= DISPATCH_LEVEL.
 *	@remark This function is not paged.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *	@post On returning STATUS_SUCCESS, the controller/s rumble setting has been
 *		updated as specified by the aRumble variable.
 *
 */
NTSTATUS GCN_Controller_Calibrate(
	DEVICE_CONTEXT _In_ *pDeviceContext, int _In_ aIndex);

/**	@brief Prepares a controller report to return to the upper HID minidriver.
 *
 *	@param [in] apDeviceContext Device context used by driver.
 *	@param [in] apAdapterData Pointer to data received from the USB adapter.
 *	@param [out] apControllerReport Pointer to a GCN_ControllerReport structure
 *		where updated controller status will be placed. The actual controller
 *		being updated will be indicated by the .id field of the structure upon
 *		return. The driver should cycle through all plugged in controllers on
 *		its own.
 *
 *	@remark This function can run at any IRQL <= DISPATCH_LEVEL.
 *	@remark This function is not paged.
 *
 */
void prepare_report(
	DEVICE_CONTEXT *apDeviceContext,
	GCN_AdapterData *apAdapterData,
	GCN_ControllerReport *apControllerReport);

#endif//_GCN_ADAPTOR_GCN_CONTROLLER_H_
