#pragma once
#ifndef _GCN_ADAPTER_POWER_H_
#define _GCN_ADAPTER_POWER_H_

/**	@file
*	This file contains function and variable declarations/definitions regarding
*	device power state management.
*
*/

#include "Include.h"

/**	Handles entering powered up mode of operation. This function is nonpaged.
 *	Called every time hardware needs to be reinitialized.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *	@param [in] aPreviousState Previous power state.
 *
 *	@remark This function runs at IRQL == PASSIVE_LEVEL by the framework.
 * 	@remark This function is not paged by requirement from the framework.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *
 */
EVT_WDF_DEVICE_D0_ENTRY GCN_AdapterEvtDeviceD0Entry;

/**	Handles leaving the powered up mode of operation.
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *	@param [in] aTargetState State of power transitioning to.
 *
 *	@remark This function runs at IRQL == PASSIVE_LEVEL by the framework.
 * 	@remark This function is paged in page PAGE.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 *		Success implies the device is useable. Any other state will cause for
 *		the device to be torn down.
 *
 */
EVT_WDF_DEVICE_D0_EXIT GCN_AdapterEvtDeviceD0Exit;

#endif//_GCN_ADAPTER_POWER_H_