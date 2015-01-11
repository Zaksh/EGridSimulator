#pragma once
#include "Node.h"
#include "FileIO.h"
#include <math.h>


/*! A consumer class */
class Consumer:public Node
{
	uint_32 parentId; /*!< Contains Parent ID */
	vector<Consumer*> subConsumer; /*!< Contains subconsumers like Zones, gridstation, and transformers.
								   Parent of these zones, gridstation can be identified by parentId*/
	
	
	double inverterPenetration; 
	/*static const double inverterPenetration=0.60;
	static const double invterEfficiency=0.576;*/
	ConsumerData InitializeConsumer(vector<double> *cdfData);
	ConsumerData InitZoneDistributions(vector<double> *cdfData);
	ConsumerData InitGridStations(vector<double> *cdfData);
	ConsumerData InitFeederes(vector<double> *cdfData);
	ConsumerData InitTransformers(vector<double> *cdfData, bool isElite = false);
	void SortSubConsumers();
	//ConsumerData ReducePower(double powerReduction);
	void LoadShedding(int groupId);
	void DistributedPowerReduction(double stressFactor, int groupId, bool isTargetAashiyana, bool isEmergency);
	void DistributedPowerReduction(double stressFactor, bool isTargetAashiyana, bool isEmergency);
	virtual void DistributedPowerReduction(double stressFactor, bool isEmergency);
	/*bool ValidatePower();
	double GetPower();*/
	void FillWithConsumerData(ConsumerData* data);
	double GetLosses();
	double GetUPSConsumption();
	double GetSoftUPSConsumption();
	void LossesCalculation(EventInfo& eventInformation,int currentHour);
	float GetUserComfort();
	int GetNumberofHomesInPowerState(PowerLevel powerLevel);
	void FillNumberOfHomesInPowerState(Output &data);
	void GenerateDLC(double powerVariationRequired, double overAllDemand);
	void GenerateCentralPowerReduction(double powerVariationRequired, double overAllDemand);
	void CenteralizedPowerReduction(short groupId, double &powerVariationRequired);
	void CenteralizedPowerReduction(double &powerVariationRequired);
	void VaryPowerLevelsOfHomes(vector<Consumer*> *subConsumer, double powerVariationRequired);
	void TurnOffAllNonAashiyana();
	void TurnOffAllNonAashiyana(int groupId);
protected:
	double upsLosses;
	double upsConsumption;
	double softUPSPowerConsumption;
	int loadSheddingCount;
	double NormalDistributionRandom(double mean, double deviation);
	int NormalDistributionRandom(int mean, int deviation);
	virtual void ShutDownConsumer();
	virtual ConsumerData RandomizePower(vector<double> *cdfData);
public:
	int nodeGroup;
	int consumersCount;
	double powerDemand; //Demand of a sector with out UPS and SoftUPS
	bool isEliteConsumer;
	Consumer(uint_32 nodeID,FILE* file);
	Consumer(uint_32 nodeID, FILE* file,int groupID);
	Consumer(const Consumer& orig);
	~Consumer(void);
	virtual bool Init();
	virtual int RandomizePower(EventInfo& eventInformation);
	virtual	bool operator==(const uint_32 id) const { return (this->myId ==id);}; //used in STL Find function
	Consumer& operator= (const Consumer& orig);
	bool ContainsConsumer(const uint_32 id) const;
	virtual void ExecuteEventToBalanceSupplyDemand(EventInfo& eventInformation,Node *nodeGeneration,Node *nodeConsumption);

};
