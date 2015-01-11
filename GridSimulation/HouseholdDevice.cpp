#include "HouseholdDevice.h"
#include "HouseHoldLoadProfile.h"
#include "FileIO.h"
#include <functional>   // std::greater_equal, std::bind2nd

extern HouseHoldLoadProfile *gHouseHoldLoadProfiles;

HouseholdDevice::HouseholdDevice(HouseHoldDeviceType houseHoldDeviceType)
{
	deviceType = houseHoldDeviceType;
	int totalNumberOfDevices = HouseHoldDeviceNames.size();
	for (int i = 0; i < totalNumberOfDevices; i++)
	{
		if (gHouseHoldLoadProfiles[i].GetHouseHoldDeviceType() == deviceType)
		{
			deviceLoadModel = gHouseHoldLoadProfiles[i].deviceLoadModel;
			break;
		}
	}
//	LoadDeviceLoadModel();
}


HouseholdDevice::~HouseholdDevice()
{
}


/**
* GenerateRandom is a Mersenne twister to generate a random number.
* It takes two integers minimun and maximum and return an integer between these two integers
*/
int HouseholdDevice::GenerateRandom(int min, int max)
{
	std::random_device rseed;
	std::mt19937 rgen(rseed()); // mersenne_twister
	std::uniform_int_distribution<int> idist(min, max); // [0,100]
	return idist(rgen);
}
/**
* GenerateRandom is a Mersenne twister to generate a random number.
* It takes two integers minimun and maximum and return an float between these two floats
*/
double HouseholdDevice::GenerateRandom(double min, double max)
{
	std::random_device rseed;
	std::mt19937 rgen(rseed()); // mersenne_twister
	std::uniform_real_distribution<double> distrbution(min, max); //[min,max]
	double randNumber = distrbution(rgen);
	return randNumber;
}
//void HouseholdDevice::LoadDeviceLoadModel()
//{
//	string deviceName = HouseHoldDeviceNames[(int)deviceType];
//	string filePath = FileIO::DiretoryPath + "/" + deviceName;
//	FileIO fileIo;
//	for (int hour = 0; hour < 24; hour++)
//	{
//		string fileName = filePath + "/fr" + std::to_string(hour+1) + ".txt";
//		deviceLoadModel[hour] = fileIo.ReadFile(fileName);
//	}
//}

double HouseholdDevice::GetPower(int hour)
{
	double value = 0;
	//hour-1 because hours are 24 and it should be 0 to 23
	vector<double> *cdfData = &(deviceLoadModel[hour-1]);
	std::_Vector_iterator<std::_Vector_val<std::_Simple_types<double>>> iterator;
	do{
		double rand = GenerateRandom(0.0, 1.0);
		iterator = find_if((*cdfData).begin(), (*cdfData).end(), std::bind2nd(std::greater_equal<double>(), rand));
	} while (iterator == (*cdfData).end());
	if (iterator != (*cdfData).end())
	{
		int pos = iterator - (*cdfData).begin();
		value=pos/10.0; //value is power consumption in Watts;
	}
	return value;
}