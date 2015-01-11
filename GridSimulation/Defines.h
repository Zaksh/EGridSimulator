#ifndef _DEFINES_H_
#define _DEFINES_H_

#define uint_32 unsigned int 
#define uint_64 unsigned long
#define uint_16 unsigned short
#define uint_8   unsigned char

//#define   TIMETICK_TO_USEC	1 ///how much is microsec equal 1 time tick (atomic unit of our simulator)
//#define   SEC_TO_TIMETICK      ((double)3600/TIMETICK_TO_USEC)//how many time ticks equal 1 second
#define   HR_TO_TIMETICK      (3458.3333)//how many time ticks equal 1 hour
#define HOUR_TICK 3600

#define DEBUG_LEVEL_CON		5 //higher debug values are of less importance
#define DEBUG_LEVEL_FILE	3 //2 zaksh changed it //higher debug values are of less importance
#define OUTFILE(fptr,id,eventStr,detailStr,dbg) if( dbg<= DEBUG_LEVEL_FILE) fprintf(fptr,"%ld\t %d\t %s \t %s\n",globalClock.globalTime,id,eventStr,detailStr);
#define OUTCON(id,eventStr,detailStr,dbg) if (dbg<= DEBUG_LEVEL_CON) printf("%ld\t %d\t %s \t %s\n",globalClock.globalTime,id,eventStr,detailStr);


#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

#include "timers.h"
#include <random>

enum eventType { et_idle, et_generation, et_consumption, et_BalanceSupplyDemandGap, et_reduceConsumption, et_reduceGeneration, et_increaseGeneration };
enum NodeType { nt_dummy, nt_Genco, nt_Disco, nt_CentralBody, nt_ZoneDistribution, nt_GridStation, nt_Feederes, nt_Transformer,nt_home };
enum LoadSheddingType{ lt_loadShedding, lt_DistributedBackOff, lt_SmartMeterDistributedBackOff, lt_Central};
enum PowerLevel { PowerLevel0, PowerLevel1, PowerLevel2, PowerLevel3, PowerLevelFull };
enum HouseType { ht_TypeA, ht_TypeB, ht_TypeC };
enum PowerReductionType { prt_agressive, prt_conservative };
enum UlilityType { ut_HighDiscomfort, ut_Middle, ut_HighRelief };
enum HouseHoldDeviceType{ AIR_CONDITIONING, BEDROOM_DS_LAMP, DISHWASER, ELECTRIC_HEAT, ELECTRONICS, FREEZER, FRIDGE, KITCHEN_LIGHTS, LIGHTING, LIVINGROOM_LAMP_TV, LIVINGROOM_S_LAMP2, MICROWAVE_REDD, MICROWAVE_UK, MISCELLAENEOUS, MODEM, OFFICE_LAMP2, OUTLETS_UNKNOWN, REFRIGERATOR, ROUTER, TV, WASHING_MACHINE };
const std::vector<std::string> HouseHoldDeviceNames = { "air_conditioning", "bedroom_ds_lamp", "dishwaser", "electric_heat", "electronics", "freezer", "fridge", "kitchen_lights", "lighting", "livingroom_lamp_tv", "livingroom_s_lamp2", "microwave-redd", "microwave-uk", "miscellaeneous", "modem", "office_lamp2", "outlets_unknown", "refrigerator", "router", "tv", "washing_machine" };



/////
struct EventInfo {
	short nodeId_; //the id of the node for which this event occured,  
	double powerVariation;
	eventType eType_; /// event Type... reception or transmission.
	timeTicks timeLeft_;//assigned with the time left till the event expiry	
};
///
class Event : public TimerCallback{
private:
	EventInfo eInfo_;

public:
	Event(EventInfo eInfo) : eInfo_(eInfo){};
	~Event(){};
	int Expire();


};

class ConsumerData
{
public:
	double losses;
	double upsConsumption;
	double power;
	double softUPSPower;
	int consumersCount;
	double demand;
	ConsumerData()
	{
		losses = 0;
		power = 0;
		softUPSPower = 0;
		consumersCount = 0;
		demand = 0;
		upsConsumption = 0;
	}
	ConsumerData(double Losses, double UpsConsumption, double Power, double PowerDemand, double SoftUPSPower, int NoOfConsumers)
	{
		losses = Losses;
		power = Power;
		softUPSPower = SoftUPSPower;
		consumersCount = NoOfConsumers;
		demand = PowerDemand;
		upsConsumption = UpsConsumption;
	}
};
class Output
{
public:
	int numberOfFeedersTurnedOff;
	int homesInPowerLevel0;
	int homesInPowerLevel1;
	int homesInPowerLevel2;
	int homesInPowerLevel3;
	int homesInPowerLevelFull;
	double actualPowerDemand;
	double powerSupplied;
	double upsLosses;
	double upsConsumption;
	double softUPSConsumption;
	float userComfort;
	Output()
	{
		numberOfFeedersTurnedOff = 0;
		actualPowerDemand = 0;
		powerSupplied = 0;
		upsLosses = 0;
		upsConsumption = 0;
		softUPSConsumption = 0;
		userComfort = 0;
		homesInPowerLevel0 = 0;
		homesInPowerLevel1 = 0;
		homesInPowerLevel2 = 0;
		homesInPowerLevel3 = 0;
		homesInPowerLevelFull = 0;
	}
};

////

#endif
