#pragma once
#include "Include.h"

//
// Define the tracing flags.
//
// Tracing GUID - 00df223a-e76d-4b5c-9ea6-6300b47ec3c0
//

#define WPP_CONTROL_GUIDS												\
	WPP_DEFINE_CONTROL_GUID(											\
		GCN_AdapterTraceGuid, (00df223a,e76d,4b5c,9ea6,6300b47ec3c0),	\
																		\
		WPP_DEFINE_BIT(MYDRIVER_ALL_INFO)								\
		WPP_DEFINE_BIT(TRACE_DRIVER)									\
		WPP_DEFINE_BIT(TRACE_DEVICE)									\
		WPP_DEFINE_BIT(TRACE_QUEUE)										\
		WPP_DEFINE_BIT(TRACE_INTERRUPT)									\
		WPP_DEFINE_BIT(TRACE_IOCTL)										\
		WPP_DEFINE_BIT(TRACE_IO)										\
		WPP_DEFINE_BIT(TRACE_POWER)										\
		WPP_DEFINE_BIT(TRACE_GCN_INTERFACE)								\
		WPP_DEFINE_BIT(TRACE_GCN_CONTROLLER)							\
		)

#define WPP_FLAG_LEVEL_LOGGER(flag, level)		\
	WPP_LEVEL_LOGGER(flag)

#define WPP_FLAG_LEVEL_ENABLED(flag, level)		\
	(WPP_LEVEL_ENABLED(flag) &&					\
	 WPP_CONTROL(WPP_BIT_ ## flag).Level >= level)

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
		   WPP_LEVEL_LOGGER(flags)
			   
#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
		(WPP_LEVEL_ENABLED(flags) && \
			WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

//
// This comment block is scanned by the trace preprocessor to define our
// Trace function.
//
// begin_wpp config
// FUNC Trace{FLAG=MYDRIVER_ALL_INFO}(LEVEL, MSG, ...);
// FUNC TraceEvents(LEVEL, FLAGS, MSG, ...);
// end_wpp
//
