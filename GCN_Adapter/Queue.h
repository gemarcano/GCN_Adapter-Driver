#pragma once
#ifndef _GCN_ADAPTER_QUEUE_H_
#define _GCN_ADAPTER_QUEUE_H_
#include "Include.h"

/**	Initializes driver's queues. (TODO how?)
 *
 *	@param [in] aDevice USB Device created by GCN_AdapterCreateDevice.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
NTSTATUS GCN_AdapterQueueInitialize(_In_ WDFDEVICE aDevice);

#endif//_GCN_ADAPTER_QUEUE_H_
