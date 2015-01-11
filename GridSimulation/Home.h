#pragma once
#include "Consumer.h"
#include "HouseholdDevice.h"
#include <future>

class HomeConsumer :	public Consumer
{
private:
	HouseType houseType;
	float userComfort;
	PowerLevel currentPowerLevel;
	vector<HouseholdDevice*> houseHoldDevices;
	bool isAashiyanaEnabled;
	bool lastHourLoadShed = false;
	bool isInDLCState= false;
	bool shouldMoveToDLC = true;
	float consumptionIndex;
	double lastStressFactor = 0;
	short numberOfDLCSignalsReceived = 0;
	//short numberOfDLCSignalsImplemented = 0;

	short lastHour = 0;

	void GetSetOfAppliances(HouseType typeOfHouse);
	void UpdatePower();
	void SetDistributedCurrentPowerLevel(double random, double stressFactor, bool isEmergency);
	void SetCentralCurrentPowerLevel();
	void SetPowerLevelThesholdValues(double stressFactor, double &uperThreshold, double &lowerThreshold);
	vector<HouseHoldDeviceType> const * GetSetOfAppliances(HouseType typeOfHouse, PowerLevel powerLevel);
	vector<HouseHoldDeviceType> const* GetAppliancesOfTypeA(PowerLevel powerLevel);
	vector<HouseHoldDeviceType> const* GetAppliancesOfTypeB(PowerLevel powerLevel);
	vector<HouseHoldDeviceType> const* GetAppliancesOfTypeC(PowerLevel powerLevel);
	virtual void DistributedPowerReduction(double threshold,bool isEmergency);
	float GetUserComfortAccording2Level(PowerLevel level);
	void SetConsumptionIndex();
protected:
	virtual void ShutDownConsumer();
	virtual ConsumerData RandomizePower(vector<double> *cdfData);
public:
	bool haveUps;
	//double invterEfficiency;
	//float upsRating;// = 0.9;//1;
	inline float GetHomeComfortLevel()
	{
		return userComfort;
	}
	inline HouseType GetHouseType()
	{
		return houseType;
	}
	inline PowerLevel GetCurrentPowerLevel()
	{
		return currentPowerLevel;
	}
	inline bool IsHomeAashiyanaEnabled()
	{
		return isAashiyanaEnabled;
	}
	inline float GetConsumptionIndex()
	{
		return consumptionIndex;
	}
	HomeConsumer(uint_32 nodeID, FILE* file, bool haveAashiyana);
	HomeConsumer(const HomeConsumer& orig);
	HomeConsumer& operator= (const HomeConsumer& orig);
	~HomeConsumer();
	void CenteralizedPowerReduction(PowerLevel powerLevel);
};

