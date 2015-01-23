#include <Windows.h>
#include <SetupAPI.h>
#pragma comment(lib, "setupapi.lib")

#include <stdio.h>
#include <string>
#include <iostream>

namespace GCN_Adapter
{
	std::string findDriverPath(const GUID& aGUID)
	{
		HDEVINFO hDevInfo;
		std::string result;

		hDevInfo = SetupDiGetClassDevs(&aGUID,
			0, // Enumerator
			0,
			DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

		if (hDevInfo == INVALID_HANDLE_VALUE)
		{
			// Insert error handling here.
			return result;
		}

		// Enumerate through all devices in Set.

		SP_DEVICE_INTERFACE_DATA DeviceIntData;
		DeviceIntData.cbSize = sizeof(SP_DEVINFO_DATA);

		for (DWORD i = 0;
			SetupDiEnumDeviceInterfaces(
				hDevInfo, NULL, &aGUID, i,&DeviceIntData);
			++i)
		{
			DWORD DataT;
			PSP_DEVICE_INTERFACE_DETAIL_DATA buffer = NULL;
			DWORD buffersize = 0;

			while (!SetupDiGetDeviceInterfaceDetail(
				hDevInfo,
				&DeviceIntData,
				buffer,
				buffersize,
				&buffersize, NULL))
			{
				if ((DataT = GetLastError()) ==
					ERROR_INSUFFICIENT_BUFFER)
				{
					// Change the buffer size.
					if (buffer) delete[](buffer);
					// Double the size to avoid problems on 
					// W2k MBCS systems per KB 888609. 
					buffer = (PSP_DEVICE_INTERFACE_DETAIL_DATA)new char[buffersize * 2];
					buffer->cbSize = sizeof(*buffer);
				}
				else
				{
					delete[] buffer;
					buffer = nullptr;
					break;
				}
			}

			result = buffer->DevicePath;
			if (buffer) delete[](buffer);
		}

		SetupDiDestroyDeviceInfoList(hDevInfo);

		if (GetLastError() != NO_ERROR &&
			GetLastError() != ERROR_NO_MORE_ITEMS)
		{
			// Insert error handling here.
		}

		//  Cleanup
		return result;
	}
}
