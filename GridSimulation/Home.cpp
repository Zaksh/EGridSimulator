#include "Home.h"
#include <random>
#include <thread>
#include <errno.h>
extern vector<double> gXiData;
extern cVTime globalClock;///Our global clock!
extern bool isSoftUPS;
extern LoadSheddingType gLoadSheddingType;
extern UlilityType ulilityTypeImplementation;
extern PowerReductionType powerReductionType;
HomeConsumer::HomeConsumer(uint_32 nodeID, FILE* file,bool haveAashiyana) :Consumer(nodeID, file)
{
	haveUps = false;
	currentPowerLevel = PowerLevelFull;
	nodeType = nt_home;
	//double rating = Consumer::NormalDistributionRandom(0.2, 0.04); //Random UPS rating from 0.4 kW to 0.01 kW
	//upsRating = rating > 0.8 ? 0.8 : (rating < 0 ? 0 : rating);
	//invterEfficiency = 0.50;
	//40% Simple homes and 60% Aashiyana enabled.
	if (gLoadSheddingType == lt_DistributedBackOff || gLoadSheddingType == lt_Central)
	{
		isAashiyanaEnabled = haveAashiyana;
	}
	else
	{
		isAashiyanaEnabled = false;
	}
	userComfort = 0;
	houseType = (HouseType)GenerateRandom(0, 2);
	GetSetOfAppliances(houseType);
}

HomeConsumer::HomeConsumer(const HomeConsumer& orig) :Consumer(orig)
{
	*(this) = orig;
}

HomeConsumer::~HomeConsumer()
{
}
HomeConsumer& HomeConsumer::operator= (const HomeConsumer& orig)
{
	if (this != &orig) {
		/*this->invterEfficiency = orig.invterEfficiency;
		this->haveUps = orig.haveUps;
		this->upsRating = orig.upsRating;*/		
		houseType = orig.houseType;
		userComfort=orig.userComfort;
		currentPowerLevel=orig.currentPowerLevel;
		isAashiyanaEnabled = orig.isAashiyanaEnabled;
		lastHourLoadShed =orig.lastHourLoadShed;
		isInDLCState = orig.isInDLCState;
		shouldMoveToDLC = orig.shouldMoveToDLC;
		consumptionIndex=orig.consumptionIndex;
		lastStressFactor = orig.lastStressFactor;
		numberOfDLCSignalsReceived = orig.numberOfDLCSignalsReceived;
		lastHour = orig.lastHour;
		for (auto i = orig.houseHoldDevices.begin(); i < orig.houseHoldDevices.end(); i++)
		{
			houseHoldDevices.push_back(*i);
		}
	}
	return *this;
}
void HomeConsumer::GetSetOfAppliances(HouseType typeOfHouse)
{
	vector<HouseHoldDeviceType> const * deviceType;
	switch (typeOfHouse)
	{
	case 0:
		deviceType = GetAppliancesOfTypeA(PowerLevelFull);
		break;

	case 1:
		deviceType = GetAppliancesOfTypeB(PowerLevelFull);
		break;
	case 2:
		deviceType = GetAppliancesOfTypeC(PowerLevelFull);
		break;
	}
	for (auto iterator = (*deviceType).begin(); iterator < (*deviceType).end(); iterator++)
	{
		HouseHoldDeviceType devType = (*iterator);
		HouseholdDevice *hhd = new HouseholdDevice(devType);
		houseHoldDevices.push_back(hhd);
	}
}
vector<HouseHoldDeviceType> const * HomeConsumer::GetAppliancesOfTypeA(PowerLevel powerLevel)
{
	vector<HouseHoldDeviceType> const *appliancesData = NULL;
	switch (powerLevel)
	{
	case PowerLevel1:
	{
						const static vector<HouseHoldDeviceType> data = { LIGHTING, ROUTER };
						appliancesData = &data;
	}
		break;
	case PowerLevel2:
	{
						const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER, TV };
						appliancesData = &data;
	}
		break;
	case PowerLevel3:
	{
						const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER, TV, FRIDGE };
						appliancesData = &data;
	}
		break;
	case PowerLevelFull:
	{
						   const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER, TV, WASHING_MACHINE, ELECTRIC_HEAT, FRIDGE };
						   appliancesData = &data;
	}
		break;
	case PowerLevel0:
	default:
		const static vector<HouseHoldDeviceType> data = {};
		appliancesData = &data;
		break;
	}
	return appliancesData;
}
vector<HouseHoldDeviceType> const * HomeConsumer::GetAppliancesOfTypeB(PowerLevel powerLevel)
{
	vector<HouseHoldDeviceType> const *appliancesData = NULL;
	switch (powerLevel)
	{
	case PowerLevel1:
	{
						const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER };
						appliancesData = &data;
	}
		break;
	case PowerLevel2:
	{
						const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER, TV, FRIDGE, KITCHEN_LIGHTS };
						appliancesData = &data;
	}
		break;
	case PowerLevel3:
	{
						const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER, TV, WASHING_MACHINE, FRIDGE, KITCHEN_LIGHTS };
						appliancesData = &data;
	}
		break;
	case PowerLevelFull:
	{
						   const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER, TV, WASHING_MACHINE, MICROWAVE_REDD, ELECTRIC_HEAT, FRIDGE, KITCHEN_LIGHTS, AIR_CONDITIONING };
						   appliancesData = &data;
	}
		break;
	case PowerLevel0:
	default:
		const static vector<HouseHoldDeviceType> data = {};
		appliancesData = &data;
		break;
	}
	return appliancesData;
}
vector<HouseHoldDeviceType> const * HomeConsumer::GetAppliancesOfTypeC(PowerLevel powerLevel)
{
	vector<HouseHoldDeviceType> const *appliancesData = NULL;
	switch (powerLevel)
	{
	case PowerLevel1:
	{
						const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER, LIVINGROOM_S_LAMP2 };
						appliancesData = &data;
	}
		break;
	case PowerLevel2:
	{
						const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER, TV, FRIDGE, LIVINGROOM_S_LAMP2, OUTLETS_UNKNOWN };
						appliancesData = &data;
	}
		break;
	case PowerLevel3:
	{
						const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER, TV, WASHING_MACHINE, FRIDGE, LIVINGROOM_S_LAMP2, OUTLETS_UNKNOWN, DISHWASER };
						appliancesData = &data;
	}
		break;
	case PowerLevelFull:
	{
						   const static vector<HouseHoldDeviceType> data = { LIGHTING, MODEM, ROUTER, TV, WASHING_MACHINE, MICROWAVE_REDD, ELECTRIC_HEAT, FRIDGE, AIR_CONDITIONING, LIVINGROOM_S_LAMP2, OUTLETS_UNKNOWN, AIR_CONDITIONING, DISHWASER };
						   appliancesData = &data;
	}
		break;
	case PowerLevel0:
	default:
		const static vector<HouseHoldDeviceType> data = {};
		appliancesData = &data;
		break;
	}
	return appliancesData;
}
vector<HouseHoldDeviceType> const * HomeConsumer::GetSetOfAppliances(HouseType typeOfHouse, PowerLevel powerLevel)
{
	if (typeOfHouse == ht_TypeA)
	{
		return GetAppliancesOfTypeA(powerLevel);
	}
	else if (typeOfHouse == ht_TypeB)
	{
		return GetAppliancesOfTypeB(powerLevel);
	}
	else if (typeOfHouse == ht_TypeC)
	{
		return GetAppliancesOfTypeC(powerLevel);
	}
	else
	{
		return NULL;
	}
}
void HomeConsumer::ShutDownConsumer()
{
	currentPowerLevel = PowerLevel0;
	UpdatePower();
}
ConsumerData HomeConsumer::RandomizePower(vector<double> *cdfData)
{
	currentPowerLevel = PowerLevelFull;
	UpdatePower();
	SetConsumptionIndex();
	return ConsumerData(upsLosses, upsConsumption, power, powerDemand, softUPSPowerConsumption, 0);
}
void HomeConsumer::DistributedPowerReduction(double stressFactor, bool isEmergency)
{
	double rand = GenerateRandom(0, 100);
	SetDistributedCurrentPowerLevel(rand, stressFactor, isEmergency);
	this->UpdatePower();
}
void HomeConsumer::SetDistributedCurrentPowerLevel(double random, double stressFactor, bool isEmergency)
{
	short currentHour = GetCurrentHour();
	if (lastHour != currentHour)
	{
		numberOfDLCSignalsReceived++;
		if (numberOfDLCSignalsReceived > 2 && lastHourLoadShed)
		{
			numberOfDLCSignalsReceived = 0;
			lastHourLoadShed = false;
			shouldMoveToDLC = true;
		}
		else if (lastHourLoadShed)
		{
			shouldMoveToDLC = false;
		}
		lastHour = currentHour;
		isInDLCState = false;
	}
	if (shouldMoveToDLC || isEmergency)
	{
		if (!isInDLCState)
		{
			//numberOfDLCSignalsImplemented++;
			if (random <= (stressFactor<5 ? 5 : stressFactor))
			{
				isInDLCState = true;
				lastStressFactor = stressFactor;
				double uperThreshold = 0, lowerThreshold = 0;
				SetPowerLevelThesholdValues(stressFactor, uperThreshold, lowerThreshold);
				if (random > uperThreshold)
				{
					//if (PowerLevelFull == currentPowerLevel)
					currentPowerLevel = PowerLevel3; //PowerLevel3=75% 
					//else if (PowerLevel1 != currentPowerLevel)
					//currentPowerLevel = (PowerLevel)(currentPowerLevel - 1);
				}
				else if (random > lowerThreshold)
				{
					//if (PowerLevelFull == currentPowerLevel)
					currentPowerLevel = PowerLevel2; //PowerLevel2=50%
					/*else if (PowerLevel1 != currentPowerLevel)
						currentPowerLevel = (PowerLevel)(currentPowerLevel - 1);*/
				}
				else
				{
					currentPowerLevel = PowerLevel1; //PowerLevel1=25%
				}
				lastHourLoadShed = true;
			}
		}
		else if ((isEmergency || random <lastStressFactor) && currentPowerLevel != PowerLevel1)
		{
			currentPowerLevel = (PowerLevel)(currentPowerLevel - 1);
		}
	}
}

void HomeConsumer::CenteralizedPowerReduction(PowerLevel powerLevel)
{
	currentPowerLevel = powerLevel;
	UpdatePower();

}
void HomeConsumer::UpdatePower()
{
	int currentHour = GetCurrentHour();
	double houseConsumption = 0;
	vector<HouseHoldDeviceType> const * deviceType = GetSetOfAppliances(houseType, currentPowerLevel);
	for (auto iterator = (*deviceType).begin(); iterator < (*deviceType).end(); iterator++)
	{
		for (auto deviceIterator = houseHoldDevices.begin(); deviceIterator < houseHoldDevices.end(); deviceIterator++)
		{
			if ((*deviceIterator)->GetDeviceType() == (*iterator))
			{
				houseConsumption += (*deviceIterator)->GetPower(currentHour) ;
				break;
			}
		}
	}
	houseConsumption /= 1000.0;
	this->userComfort = GetUserComfortAccording2Level(currentPowerLevel);
	this->powerDemand = houseConsumption;
	this->power = houseConsumption;
	this->upsLosses = 0;
	this->upsConsumption = 0;
	this->softUPSPowerConsumption = 0;
}
void HomeConsumer::SetPowerLevelThesholdValues(double stressFactor, double &uperThreshold, double &lowerThreshold)
{
	switch (powerReductionType)
	{
	case prt_agressive:
		uperThreshold = (stressFactor*0.75);
		lowerThreshold = (stressFactor*0.40);
		break;
	case prt_conservative:
	default:
		uperThreshold = (stressFactor*0.6);
		lowerThreshold = (stressFactor*0.35);
		break;
	}
}
float HomeConsumer::GetUserComfortAccording2Level(PowerLevel level)
{
	float jumpRelief = 0, jumpDiscomfort = 1.00;
	float comfort = 0;
	switch (ulilityTypeImplementation)
	{
	case ut_HighDiscomfort:
		jumpDiscomfort = 0.60;
		jumpRelief = 0.20;
		break;
	case ut_Middle:
		jumpDiscomfort = 0.70;
		jumpRelief = 0.25;
		break;
	case ut_HighRelief:
		jumpDiscomfort = 0.80;
		jumpRelief = 0.30;
		break;
	default:
		break;
	}
	switch (level)
	{
	case PowerLevel0:
		comfort = 0;
		break;
	case PowerLevel1:
		comfort = jumpRelief;
		break;
	case PowerLevel2:
		comfort = (jumpRelief + jumpDiscomfort) / 2;// jumpRelief + ((jumpDiscomfort - jumpRelief) / 2); //= (2jumpRelief+jumpDiscomfort - jumpRelief)/2
		break;
	case PowerLevel3:
		comfort = jumpDiscomfort;
		break;
	case PowerLevelFull:
		comfort = 1;
		break;
	default:
		break;
	}
	return comfort;
}
void HomeConsumer::SetConsumptionIndex()
{
	float maxConsumption = 0;
	switch (houseType)
	{
	case ht_TypeA:
		maxConsumption = 0.5;
		break;
	case ht_TypeB:
		maxConsumption = 0.7;
		break;
	case ht_TypeC:
		maxConsumption = 1;
		break;
	default:
		break;
	}
	consumptionIndex= power / maxConsumption;
}