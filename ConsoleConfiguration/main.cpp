#include <initguid.h>
#include <GCN_Adapter.h>
#include <iostream>

#include "DriverUtilities.h"

int main(int argc, char *argv[], char *envp[])
{
	using namespace GCN_Adapter;
	using namespace std;
	std::string driverPath = findDriverPath(GUID_DEVINTERFACE_GCN_ADAPTER);
	if (driverPath.empty())
	{
		// TODO driver was not found, report this to user somehow
		return -1;
	}
	
	cout << driverPath << endl;
	//Open driver
	HANDLE h = CreateFile(driverPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (h == INVALID_HANDLE_VALUE)
	{
		// TODO we were unable to open driver for some reason!!!
		DWORD err = GetLastError();
		return -2;
	}

	//Allocate memory we will use later
	IOCTL_GCN_Adapter_Calibrate_Data cData;
	IOCTL_GCN_Adapter_Deadzone_Data dData;
	IOCTL_GCN_Adapter_Rumble_Data rData;
	
	bool done = false;
	int input;
	int controllerIndex;
	
	int rumble = 0;
	DWORD written = 0;

	while (!done && cin)
	{
		cout << "Input which controller to affect [1, 4], or 0 to exit." << endl;
		cout << "Controller: ";
		cin >> input;
		cout << endl;

		if (input == 0)
		{
			done = true;
			continue;
		}

		if (input >= 1 && input <= 4)
		{
			controllerIndex = input;
		}
		else
		{
			controllerIndex = -1;
			cout << "Bad controller index received. Please input a range between 1 and 4, inclusive." << endl;
			continue;
		}

		cout << "Select the operation by number from the following list:" << endl;
		
		cout << "1. Force re-calibration." << endl;
		cout << "2. Change deadzone policy." << endl;
		cout << "3. Toggle rumble." << endl;
		cout << "0. Cancel." << endl;
		cout << "Option: ";
		cin >> input;
		cout << endl;

		switch (input)
		{
		case 1:
			cData.controllers = 0;
			cData.controllers = 1 << (controllerIndex - 1);
			DeviceIoControl(h, IOCTL_GCN_ADAPTER_CALIBRATE, &cData, sizeof(cData), NULL, 0, &written, NULL);
			break;
		case 2:
			dData.controller = (controllerIndex - 1);
			DeviceIoControl(h, IOCTL_GCN_ADAPTER_GET_DEADZONE, &dData, sizeof(dData), &dData, sizeof(dData), &written, NULL);

			cout << "Mode is one of the following:" << endl;
			cout << "1. None." << endl;
			cout << "2. Linear." << endl;
			cin >> input;
			dData.data.axis[0].mode = input - 1;
			dData.data.axis[1].mode = input - 1;
			dData.data.shoulder[0].mode = input - 1;
			dData.data.shoulder[1].mode = input - 1;
			if (input == 2)
			{
				cout << "Select deadzone value [0, 255]: ";
				cin >> input;
				dData.data.axis[0].deadzone = input;
				dData.data.axis[1].deadzone = input;
				dData.data.shoulder[0].deadzone = input;
				dData.data.shoulder[1].deadzone = input;
			}

			DeviceIoControl(h, IOCTL_GCN_ADAPTER_SET_DEADZONE, &dData, sizeof(dData), &dData, sizeof(dData), &written, NULL);
			break;
		case 3:
			rumble ^= 1 << (controllerIndex - 1);
			rData.controllers = rumble;
			
			DeviceIoControl(h, IOCTL_GCN_ADAPTER_SET_RUMBLE, &rData, sizeof(rData), &rData, sizeof(rData), &written, NULL);

		case 0:
			continue;
		default:
			break;
		}
	}
	
	return 0;

}
