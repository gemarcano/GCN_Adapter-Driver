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

#include "Device.h"
#include "Driver.h"
#include "Queue.h"
#include "Trace.h"
#include "IO.h"
#include "Interrupt.h"
#include "Ioctl.h"
#include "Power.h"
#include "Queue.h"
#include "HID.h"

#endif//_GCN_ADAPTER_INCLUDE_H_
