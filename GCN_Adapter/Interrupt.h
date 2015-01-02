#pragma once
#ifndef _GCN_ADAPTOR_INTERRUPT_H_
#define _GCN_ADAPTOR_INTERRUPT_H_

#include "Include.h"

/**	Configures the continuous interrupt pipe reader for getting controller data.
 *
 *	@param [in] apDeviceContext Device context where to store data.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS GCN_AdaptorConfigContReaderForInterruptEndPoint(
	_In_ PDEVICE_CONTEXT apDeviceContext);

/**	Handles storing the data read from a continuous read if successful.
 *
 *	@param [in] aPipe Pipe from which data was read.
 *	@param [in] aBuffer Memory with results of read.
 *	@param [in] aNumBytesTransferred Amount of data read.
 *	@param [in] aContext Device context where to store data.
 *
 *	@post Data read is stored in device context.
 *
 */
VOID GCN_AdaptorEvtUsbInterruptPipeReadComplete(
	WDFUSBPIPE aPipe,
	WDFMEMORY aBuffer,
	size_t aNumBytesTransferred,
	WDFCONTEXT aContext);

/**	Handles storing the data read from a continuous read if successful.
 *
 *	@param [in] aPipe Pipe from which data should have been read.
 *	@param [in] aStatus Indicates why reading failed.
 *	@param [in] aUsbdStatus ?FIXME
 *
 *	@post FIXME TODO Determine what exactly should happen here...
 *
 */
BOOLEAN GCN_AdaptorEvtUsbInterruptReadersFailed(
	_In_ WDFUSBPIPE aPipe,
	_In_ NTSTATUS aStatus,
	_In_ USBD_STATUS aUsbdStatus);

#endif//_GCN_ADAPTOR_INTERRUPT_H_
