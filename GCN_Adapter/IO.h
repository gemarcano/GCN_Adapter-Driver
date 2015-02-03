#pragma once
#ifndef _GCN_ADAPTER_IO_H_
#define _GCN_ADAPTER_INCLUDE_H_

/**	@file
 *	This file contains function and variable declarations/definitions regarding
 *	read and write requests.
 *
 */

#include "Include.h"

/**	Handles Read requests. Returns data from adaptor to request.
 *
 *	@param[in] aQueue Queue from which the request came.
 *	@param[in] aRequest Read request.
 *	@param[in] aLength Length of the read buffer. For this driver, this has to
 *		be 37 bytes (full length of the data).
 *
  *	@remark This function runs at IRQL <= DISPATCH_LEVEL by the framework.
 *	@remark This function is not paged.
 *
 *	@post Request is handled asynchronously.
 *
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID GCN_AdapterEvtIoRead(
	_In_ WDFQUEUE aQueue,
	_In_ WDFREQUEST aRequest,
	_In_ size_t aLength);

/**	Handles Write requests. Writes data to device (rumble).
 *
 *	@param[in] aQueue Queue from which the request came.
 *	@param[in] aRequest Read request.
 *	@param[in] aLength Length of the write buffer. For this driver, this has to
 *		be 5 bytes (full length of the rumble data).
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL by the framework.
 * 	@remark This function is not paged.
 *
 *	@post Request is handled asynchronously.
 *
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID GCN_AdapterEvtIoWrite(
	_In_ WDFQUEUE aQueue,
	_In_ WDFREQUEST aRequest,
	_In_ size_t	aLength);

/**	Finishes handling Write requests.
 *
 *	@param[in] aRequest Write request.
 *	@param[in] aTarget WDF IO target object that represents the target that
 *		completed the IO request.
 *	@param[in] aCompletionParams Completion parameters from the request.
 *	@param[in] aContext Device context.
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL by the framework.
 *	@remark This function is not paged.
 *
 *	@post Request is handled.
 *
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
VOID GCN_AdapterEvtRequestWriteCompletionRoutine(
	_In_ WDFREQUEST aRequest,
	_In_ WDFIOTARGET aTarget,
	_In_ PWDF_REQUEST_COMPLETION_PARAMS aCompletionParams,
	_In_ WDFCONTEXT aContext);

#endif//_GCN_ADAPTER_IO_H_
