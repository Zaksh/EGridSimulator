#include "Defines.h"
#pragma once
class HouseHoldLoadProfile
{
	HouseHoldDeviceType deviceType;
	void LoadDeviceLoadModel();
public:
	//static HouseHoldLoadProfile CreateHouseHoldLoadProfile();
	HouseHoldLoadProfile();
	void SetHouseHoldLoadType(HouseHoldDeviceType type);
	HouseHoldDeviceType GetHouseHoldDeviceType();
	vector<double> deviceLoadModel[24];
	~HouseHoldLoadProfile();
};

