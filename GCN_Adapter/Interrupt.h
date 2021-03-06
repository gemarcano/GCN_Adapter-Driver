#pragma once
#ifndef _GCN_ADAPTOR_INTERRUPT_H_
#define _GCN_ADAPTOR_INTERRUPT_H_

/**	@file
 *	This file contains function and variable declarations/definitions regarding
 *	USB interrupt pipe functionality and callbacks.
 *
 */

#include "Include.h"

/**	Configures the continuous interrupt pipe reader for getting controller data.
 *
 *	@param [in] apDeviceContext Device context where to store data.
 *
 *	@remark This function runs at IRQL == PASSIVE_LEVEL since it is called by
 *		GCN_AdapterEvtDevicePrepareHardware.
 *	@remark This function is paged in page PAGE.
 *
 *	@returns NTSTATUS. @See
 *		http://msdn.microsoft.com/en-us/library/cc704588.aspx for details.
 */
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS GCN_AdapterConfigContReaderForInterruptEndPoint(
	_In_ PDEVICE_CONTEXT apDeviceContext);

/**	Handles storing the data read from a continuous read if successful.
 *
 *	@param [in] aPipe Pipe from which data was read.
 *	@param [in] aBuffer Memory with results of read.
 *	@param [in] aNumBytesTransferred Amount of data read.
 *	@param [in] aContext Device context where to store data.
 *
 *	@remark This function runs at IRQL <= DISPATCH_LEVEL by the framework since
 *		it runs at the level the IO operation was completed in.
 *	@remark This function is not paged.
 *
 *	@post Data read is stored in device context. Next HID READ_REPORT request is
 *		serviced with most up-to-date data.
 *
 */
_IRQL_requires_max_(DISPATCH_LEVEL)
EVT_WDF_USB_READER_COMPLETION_ROUTINE GCN_AdapterEvtUsbInterruptPipeReadComplete;

/**	Handles the case when the continuous reader fails to read.
 *
 *	@param [in] aPipe Pipe from which data should have been read.
 *	@param [in] aStatus Indicates why reading failed.
 *	@param [in] aUsbdStatus Failed status reported by the usb continuous reader.
 *
 *	@remark This function runs at IRQL == PASSIVE_LEVEL by the framework.
 *	@remark This function is paged in page PAGE.
 *
 *	@post Next HID READ_REPORT request is serviced with most up-to-date data.
 *
 */
_IRQL_requires_(PASSIVE_LEVEL)
EVT_WDF_USB_READERS_FAILED GCN_AdapterEvtUsbInterruptReadersFailed;

#endif//_GCN_ADAPTOR_INTERRUPT_H_
