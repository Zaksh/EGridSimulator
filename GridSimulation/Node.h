#pragma once

#include "Defines.h"
#define INIT_SLOT_SYNC 0 ///start the slotTime events triggering after 10 ticks (arbitrary)
extern cVTime globalClock;///Our global clock!
class Node
{
protected:
	int GenerateRandom(int min, int max);
	double GenerateRandom(double min, double max);
public:
	uint_32 myId;
	//uint_32 power; //If NodeType is Genco then power is in sense of generation else it is consumption
	double power; //If NodeType is Genco then power is in sense of generation else it is consumption
	NodeType nodeType;
	FILE* outFile;
	//static int powerReduction;

	static uint_32 MAX_POWER_GENERATION;//30000;
	inline int GetCurrentHour()
	{
		return ((globalClock.globalTime) / HOUR_TICK) + 1;
	}
	Node(uint_32 nodeID,FILE* file);
	virtual ~Node(void);
	Node(const Node& orig);
	Node& operator= (const Node& orig);
	virtual bool Init();
	virtual int RandomizePower(EventInfo& eventInformation); //Randomly increase or decrease power
	int Consumption(EventInfo& eventInformation);
	int Generation(EventInfo& eventInformation);
	void GenerateEventToBalanceSupplyDemand(EventInfo& eventInformation,Node *nodeGeneration,Node *nodeConsumption);
	virtual void ExecuteEventToBalanceSupplyDemand(EventInfo& eventInformation,Node *nodeGeneration,Node *nodeConsumption);
	void BalanceSupplyDemandGap(EventInfo& eventInformation);
	void Dummy(EventInfo& eventInformation);
	virtual bool operator==(const uint_32 id) const { return (this->myId ==id);}; //used in STL Find function
};
