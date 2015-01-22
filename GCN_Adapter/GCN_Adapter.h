//
// Define an Interface Guid so that app can find the device and talk to it.
//
#pragma once 
#ifndef _GCN_ADAPTER_H_
#define _GCN_ADAPTER_H_


#include <guiddef.h>
#ifdef _GCN_ADAPTER_DRIVER_ 
#include <ntdef.h>
#include <wdftypes.h>
#else
#include <Windows.h>
#endif

//According to Microsoft, putting this in an include guard breaks pre-compiled
//headers. Not using that for this project.
DEFINE_GUID(GUID_DEVINTERFACE_GCN_ADAPTER,
	0x902e89e4, 0x5a4c, 0x41e9, 0xb3, 0x27, 0xa3, 0xf1, 0xcb, 0x31, 0x38, 0x2b);
// {902e89e4-5a4c-41e9-b327-a3f1cb31382b}

//Takes in a single byte, with the first four bits identifying which controllers to calibrate
typedef struct _IOCTL_GCN_Adapter_Rumble_Data
{
	BYTE: 4;
	BYTE controllers : 4;
} IOCTL_GCN_Adapter_Calibrate_Data;
#define IOCTL_GCN_ADAPTER_CALIBRATE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_WRITE_DATA)

//Takes in a single byte, with the first four bits identifying which controllers to rumble
typedef IOCTL_GCN_Adapter_Calibrate_Data IOCTL_GCN_Adapter_Rumble_Data;

#define IOCTL_GCN_ADAPTER_SET_RUMBLE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_WRITE_DATA)

//Options for deadzone management
#define IOCTL_GCN_ADAPTER_DEADZONE_IGNORE 0xFF
#define IOCTL_GCN_ADAPTER_DEADZONE_NULL 0x00
#define IOCTL_GCN_ADAPTER_DEADZONE_LINEAR 0x01

typedef struct
{
	struct
	{
		BYTE mode;
		BYTE deadzone;
	} axis[2];

	struct
	{
		BYTE mode;
		BYTE deadzone;
	} shoulder[2];
} IOCTL_GCN_Adapter_Deadzone_Controller_Data;

typedef struct _IOCTL_GCN_Adapter_Deadzone_Data
{
	BYTE controller;
	IOCTL_GCN_Adapter_Deadzone_Controller_Data data;
	
} IOCTL_GCN_Adapter_Deadzone_Data;

#define IOCTL_GCN_ADAPTER_SET_DEADZONE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_WRITE_DATA)

#define IOCTL_GCN_ADAPTER_GET_DEADZONE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_READ_DATA)

#endif//_GCN_ADAPTER_H_
