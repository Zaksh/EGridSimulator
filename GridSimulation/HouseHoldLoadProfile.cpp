#include "HouseHoldLoadProfile.h"
#include "FileIO.h"

HouseHoldLoadProfile::HouseHoldLoadProfile()
{
}


HouseHoldLoadProfile::~HouseHoldLoadProfile()
{
}
void HouseHoldLoadProfile::LoadDeviceLoadModel()
{
	string deviceName = HouseHoldDeviceNames[(int)deviceType];
	string filePath = FileIO::DiretoryPath + "/" + deviceName;
	FileIO fileIo;
	for (int hour = 0; hour < 24; hour++)
	{
		string fileName = filePath + "/fr" + std::to_string(hour + 1) + ".txt";
		deviceLoadModel[hour] = fileIo.ReadFile(fileName);
	}
}
void HouseHoldLoadProfile::SetHouseHoldLoadType(HouseHoldDeviceType type)
{
	deviceType = type;
	LoadDeviceLoadModel();
}
HouseHoldDeviceType HouseHoldLoadProfile::GetHouseHoldDeviceType()
{
	return deviceType;
}