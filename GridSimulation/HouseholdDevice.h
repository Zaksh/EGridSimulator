#include "Defines.h"
#pragma once
class HouseholdDevice
{
	int GenerateRandom(int min, int max);
	double  GenerateRandom(double  min, double max);
	HouseHoldDeviceType deviceType;
	vector<double> *deviceLoadModel;
	//void LoadDeviceLoadModel();
public:
	HouseholdDevice(HouseHoldDeviceType);
	~HouseholdDevice();
	double GetPower(int hour);
	inline HouseHoldDeviceType GetDeviceType()
	{
		return deviceType;
	}
};

