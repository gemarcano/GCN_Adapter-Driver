#pragma once
#ifndef _GCN_ADAPTER_IO_H_
#define _GCN_ADAPTER_INCLUDE_H_

#include "Include.h"

/**	Handles Read requests. Returns data from adaptor to request.
 *
 *	@param[in] aQueue Queue from which the request came.
 *	@param[in] aRequest Read request.
 *	@param[in] aLength Length of the read buffer. For this driver, this has to
 *		be 37 bytes (full length of the data).
 *
 *	@post Request is handled asynchronously.
 *
 */
VOID GCN_AdaptorEvtIoRead(
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
*	@post Request is handled asynchronously.
*
*/
VOID GCN_AdaptorEvtIoWrite(
	_In_ WDFQUEUE aQueue,
	_In_ WDFREQUEST aRequest,
	_In_ size_t	aLength);

/**	Finishes handling Write requests.
*
*	@param[in] aRequest Write request.
*	@param[in] aTarget ?FIXME
*	@param[in] aCompletionParams Completion parameters from the request.
*	@param[in] aContext Device context.
*
*	@post Request is handled.
*
*/
VOID EvtRequestWriteCompletionRoutine(
	_In_ WDFREQUEST aRequest,
	_In_ WDFIOTARGET aTarget,
	_In_ PWDF_REQUEST_COMPLETION_PARAMS aCompletionParams,
	_In_ WDFCONTEXT aContext);

/**	Handles stopping I/O requests on a queue about to change power states.
 *
 *	@param [in] aQueue Queue to handle.
 *	@param [in] aRequest ?FIXME
 *	@param [in] aActionFlags Some WDF_REQUEST_STOP_ACTION_FLAGS (bitwise ORed)
 *		that explain why this callback is being called.
 *
 *	@post ?FIXME
 *
 */
VOID GCN_AdaptorEvtIoStop(
	_In_ WDFQUEUE aQueue,
	_In_ WDFREQUEST aRequest,
	_In_ ULONG aActionFlags);

#endif//_GCN_ADAPTER_IO_H_
