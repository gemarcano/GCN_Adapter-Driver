#pragma once
#ifndef _GCN_ADAPTER_INCLUDE_H_
#define _GCN_ADAPTER_INCLUDE_H_

//Master include file, to simplify include dependecies 

#include <ntddk.h>
#include <wdf.h>
#include <usb.h>
#include <usbdlib.h>
#include <wdfusb.h>
#include <hidport.h>
#include <usb.h>
#include <usbioctl.h>
#include <devguid.h>

#define NTSTRSAFE_LIB
#include <ntstrsafe.h>

#include "Device.h"
#include "Driver.h"
#include "Trace.h"
#include "IO.h"
#include "Interrupt.h"
#include "Ioctl.h"
#include "Power.h"
#include "HID.h"
#include "GCN_Interface.h"
#include "GCN_Adapter.h"
#include "GCN_Controller.h"

#endif//_GCN_ADAPTER_INCLUDE_H_
