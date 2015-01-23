#pragma once
#ifndef _GCN_ADAPTER_DRIVERUTILITIES_H_
#define _GCN_ADAPTER_DRIVERUTILITIES_H_

#include <string>
#include <objbase.h>

namespace GCN_Adapter
{
	std::string findDriverPath(const GUID& aGUID);
}

#endif//_GCN_ADAPTER_DRIVERUTILITIES_H_