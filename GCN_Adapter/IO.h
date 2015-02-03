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
EVT_WDF_IO_QUEUE_IO_READ GCN_AdapterEvtIoRead;

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
EVT_WDF_IO_QUEUE_IO_WRITE GCN_AdapterEvtIoWrite;

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
EVT_WDF_REQUEST_COMPLETION_ROUTINE GCN_AdapterEvtRequestWriteCompletionRoutine;

#endif//_GCN_ADAPTER_IO_H_
