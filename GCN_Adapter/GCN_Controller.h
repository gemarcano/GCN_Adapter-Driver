#pragma once
#ifndef _GCN_ADAPTOR_GCN_CONTROLLER_H_
#define _GCN_ADAPTOR_GCN_CONTROLLER_H_

#include <ntddk.h>
#include <wdf.h>

typedef struct _GCN_AdapterData
{
	BYTE Signal;

	struct {
		struct
		{
			BYTE:2;
			BYTE powered : 1;
			BYTE:1;
			BYTE type : 2;
			BYTE:2;
		} Status;
		struct {
			BYTE a : 1;
			BYTE b : 1;
			BYTE x : 1;
			BYTE y : 1;
			BYTE d_left : 1;
			BYTE d_right : 1;
			BYTE d_down : 1;
			BYTE d_up : 1;
		} Buttons1;
		struct {
			BYTE start : 1;
			BYTE z : 1;
			BYTE r : 1;
			BYTE l : 1;
			BYTE: 4;
		} Buttons2;
		struct {
			BYTE X, Y;
		} LeftAxis;

		struct {
			BYTE X, Y;
		} RightAxis;

		struct {
			BYTE left, right;
		} ShoulderAxis;
	}Port[4];
} GCN_AdapterData;

typedef struct _GCN_ControllerReport
{
	BYTE id;
	struct {
		BYTE a : 1;
		BYTE b : 1;
		BYTE x : 1;
		BYTE y : 1;
		BYTE d_left : 1;
		BYTE d_right : 1;
		BYTE d_down : 1;
		BYTE d_up : 1;
	} Buttons1;
	struct {
		BYTE start : 1;
		BYTE z : 1;
		BYTE r : 1;
		BYTE l : 1;
		BYTE: 4;
	} Buttons2;
	struct {
		BYTE X, Y;
	} LeftAxis;

	struct {
		BYTE X, Y;
	} RightAxis;

	struct {
		BYTE left, right;
	} ShoulderAxis;
} GCN_ControllerReport;

typedef struct _GCN_AdapterReport
{
	GCN_ControllerReport Port[4];
} GCN_AdapterReport;

extern GCN_ControllerReport GCN_AdapterControllerZero;

#endif//_GCN_ADAPTOR_GCN_CONTROLLER_H_