#include "Consumer.h"
#include "Home.h"
#include <random>
#include <map>
#include <thread>
extern	Timers gTimerManager;
extern vector<Node*> gAllNodes;///vector that will keep track of all the nodes in the Simulator
extern Timers gTimerManager;
//extern vector<double> gXiData;
//extern std::map<int, vector<double>> gCdfMap;
//extern std::map<int, vector<double>> gValData;
extern double gTotalUpsLosses;
extern double gTotalSoftUpsConsumption;
extern double gSaving;
extern vector<uint_32> gFeedersTurnedOff;
extern bool isSoftUPS;
extern LoadSheddingType gLoadSheddingType;
extern PowerLevel powerLevel;
extern vector<Output> gOutputData;
extern int gHomeAgents;
extern int gMaxHomeAgents;
extern short gTotalFeederGroups;
extern bool gVariableSupply;
extern float gSupplyDemandVariationRatio;
extern float gAashiyanaPenetrationRatio;

Consumer::Consumer(uint_32 nodeID, FILE* file) :Node(nodeID, file)
{
	//http://stackoverflow.com/questions/8440213/how-to-define-a-const-double-inside-a-classs-header-file
	inverterPenetration = 0; //0-1 ==> 0-100%
	softUPSPowerConsumption = 0;
	consumersCount = 0;
	powerDemand = 0;
	upsLosses = 0;
	upsConsumption = 0;
	loadSheddingCount = 0;
	isEliteConsumer = false;
}
Consumer::Consumer(uint_32 nodeID, FILE* file, int groupID) :Node(nodeID, file)
{
	inverterPenetration = 0; //0-1 ==> 0-100%
	softUPSPowerConsumption = 0;
	consumersCount = 0;
	powerDemand = 0;
	upsLosses = 0;
	upsConsumption = 0;
	loadSheddingCount = 0;
	isEliteConsumer = false;
	nodeGroup = groupID;
}
Consumer::Consumer(const Consumer& orig) :Node(orig)
{
	*(this) = orig;
}
Consumer& Consumer::operator= (const Consumer& orig){
	if (this == &orig) return *this;


	myId = orig.myId;// the unique node id kept for purposes of identification
	this->power = orig.power;
	this->nodeType = orig.nodeType;
	this->outFile = orig.outFile;

	this->parentId = orig.parentId;

	inverterPenetration = orig.inverterPenetration;
	softUPSPowerConsumption = orig.softUPSPowerConsumption;
	consumersCount = orig.consumersCount;
	upsLosses = orig.upsLosses;;
	upsConsumption = orig.upsLosses;
	loadSheddingCount = orig.loadSheddingCount;
	isEliteConsumer = orig.isEliteConsumer;
	nodeGroup = orig.nodeGroup;

	//vector<Consumer>::iterator consumerIterator;
	/*Consumer c=orig.subConsumer.back();
	c.myId;*/
	for (auto consumerIterator = orig.subConsumer.begin(); consumerIterator != orig.subConsumer.end(); consumerIterator++)
	{
		subConsumer.push_back(*consumerIterator);
	}

	return *this;
}
Consumer::~Consumer(void)
{
	for_each(subConsumer.begin(), subConsumer.end(), [](Consumer *consumer){
		delete consumer;
		return true;
	});
}
void Consumer::SortSubConsumers()
{
	sort(this->subConsumer.begin(), this->subConsumer.end(),
		[](const Consumer* left, const Consumer* right){
		return left->power > right->power;
	}); //sort in descending order based on power
}
/**
* Init initializes a consumer at DISCO level.
* it initializes subconsumers like zones in a DISCO, gridstations, feeders and transformers.
*/
bool Consumer::Init(){

	TimerCallback* tcbPtr;///pointer to insert timers for each node
	EventInfo evInfo;


	if (nodeType == nt_Disco)
	{
		evInfo.eType_ = et_consumption;
		this->power = 0;
		this->upsLosses = 0;
		this->softUPSPowerConsumption = 0;;
		//		int maxZones = 10, minZones = 2;
		//int maxZones = 8, minZones = 4;
		int numberOfZones = 5;// GenerateRandom(minZones, maxZones);
		Consumer *zoneDistribution;
		this->parentId = 0;

		//int currentHour = ((globalClock.globalTime) / HOUR_TICK) + 1;
		//auto mapIterator = gCdfMap.find(currentHour);
		//if (mapIterator == gCdfMap.end())
		//{
		//	gCdfMap.clear();
		//	//gValData.clear();
		//	FileIO file;
		//	for (int i = currentHour; i <= currentHour + 1; i++)
		//	{
		//		gCdfMap[i] = file.ReadCdfData(i);
		//		sort(gCdfMap[i].begin(), gCdfMap[i].end());
		//		//gValData[i] = file.ReadValData(i);
		//	}
		//}

		for (int i = 1; i <= numberOfZones; i++)
		{
			zoneDistribution = new Consumer(this->myId * 100 + i, this->outFile);
			zoneDistribution->parentId = this->myId;
			zoneDistribution->nodeType = nt_ZoneDistribution;
			//ConsumerData *subConsumerData = &zoneDistribution->InitializeConsumer(&gCdfMap[currentHour]);
			ConsumerData *subConsumerData = &zoneDistribution->InitializeConsumer(NULL);
			FillWithConsumerData(subConsumerData);
			this->subConsumer.push_back(zoneDistribution);
			gAllNodes.push_back(zoneDistribution);
			//delete zoneDistribution;
		}
		SortSubConsumers();
		//this->power/=1000;
	}
	else
	{
		evInfo.eType_ = et_idle;
	}

	evInfo.nodeId_ = myId;

	evInfo.timeLeft_ = (timeTicks)(INIT_SLOT_SYNC);

	tcbPtr = new Event(evInfo);
	gTimerManager.AddTimer(evInfo.timeLeft_, tcbPtr);

	return true;
}
/**
* InitializeConsumer initializes a consumers at different levels.
* it initializes subconsumers like zones in a DISCO, gridstations, feeders and transformers.
* based on nodeType, if it is Zone level, it will call a method to generate random grid stations.
*/
ConsumerData Consumer::InitializeConsumer(vector<double> *cdfData)
{
	switch (nodeType)
	{
	case nt_ZoneDistribution:
		InitZoneDistributions(cdfData);
		break;
	case nt_GridStation:
		InitGridStations(cdfData);
		break;
	case nt_Feederes:
		InitFeederes(cdfData);
		break;
	case nt_Transformer:
		InitTransformers(cdfData);
		break;
	default:
		OUTCON(this->myId, "InitConsumer", "Wrong consumer type initialization", 1);
		break;
	}
	//this->SortSubConsumers();
	ConsumerData data;
	data.power = this->power;
	data.losses = this->upsLosses;
	data.softUPSPower = this->softUPSPowerConsumption;
	data.consumersCount = this->consumersCount;
	data.demand = this->powerDemand;
	data.upsConsumption = this->upsConsumption;
	return data;
}
/**
* InitZoneDistributions initializes a subconsumers for zones.
* It generate random number of grid stations in given zones
*/
ConsumerData Consumer::InitZoneDistributions(vector<double> *cdfData)
{
	Consumer *consumer;
	//int maxStations = 18, minStations = 5;
	//int maxStations = 10, minStations = 5;
	int numberOfStations = 10;// GenerateRandom(minStations, maxStations);
	this->power = 0;
	this->upsLosses = 0;
	ConsumerData subConsumerData;
	for (int i = 1; i <= numberOfStations; i++)
	{
		consumer = new Consumer(this->myId * 10 + i, this->outFile);
		this->subConsumer.push_back(consumer);
		gAllNodes.push_back(consumer);
		//consumer=(this->subConsumer.back());
		consumer->parentId = this->myId;
		consumer->nodeType = nt_GridStation;
		subConsumerData = consumer->InitializeConsumer(cdfData);
		FillWithConsumerData(&subConsumerData);
		/*this->subConsumer.push_back(*consumer);
		delete consumer;*/
	}
	return subConsumerData;
}
ConsumerData Consumer::InitGridStations(vector<double> *cdfData)
{
	Consumer *consumer;
	//int maxFeeders = 13, minFeeders = 8;
	int numberOfFeeders = 10;// GenerateRandom(minFeeders, maxFeeders);
	this->power = 0;
	this->upsLosses = 0;
	ConsumerData subConsumerData;
	static int groupID = 0;
	for (int i = 1; i <= numberOfFeeders; i++)
	{
		consumer = new Consumer(this->myId * 10 + i, this->outFile, (groupID++ % gTotalFeederGroups));
		this->subConsumer.push_back(consumer);
		gAllNodes.push_back(consumer);
		//consumer=(this->subConsumer.back());
		consumer->parentId = this->myId;
		consumer->nodeType = nt_Feederes;
		subConsumerData = consumer->InitializeConsumer(cdfData);
		FillWithConsumerData(&subConsumerData);
	}
	return subConsumerData;
}
ConsumerData Consumer::InitFeederes(vector<double> *cdfData)
{
	Consumer *consumer;
	//int maxTransformers = 30, minTransformers = 10;
	//int maxTransformers = 15, minTransformers = 10;
	int numberOfTransformers = 10;//GenerateRandom(minTransformers, maxTransformers);
	int numberOfEliteTransformers = 0;// (numberOfTransformers * GenerateRandom(0.0, 0.25));//MAX 35% off feeders as elite who consumer
	//much power;
	this->power = 0;
	this->upsLosses = 0;
	ConsumerData subConsumerData;
	for (int i = 1; i <= numberOfTransformers; i++)
	{
		consumer = new Consumer(this->myId * 10 + i, this->outFile);
		this->subConsumer.push_back(consumer);
		gAllNodes.push_back(consumer);
		consumer->parentId = this->myId;
		consumer->nodeType = nt_Transformer;
		int minConsumer = 7, maxConsumer = 10;
		consumer->consumersCount = GenerateRandom(minConsumer, maxConsumer);
		//subConsumerData = consumer->InitializeConsumer(cdfData);
		if (i <= numberOfEliteTransformers)
		{
			consumer->isEliteConsumer = true;
			subConsumerData = consumer->InitTransformers(cdfData, true);
		}
		else
		{
			subConsumerData = consumer->InitializeConsumer(cdfData);
		}

		FillWithConsumerData(&subConsumerData);
	}
	return subConsumerData;
}
ConsumerData Consumer::InitTransformers(vector<double> *cdfData, bool isElite)
{
	this->power = 0;
	this->powerDemand = 0;
	bool shouldAddHomeConsumers = (this->subConsumer.size() < 1);
	int numberOfUpses = this->inverterPenetration*this->consumersCount;
	this->isEliteConsumer = isElite;
	double upsCons = 0;
	if (!isSoftUPS && !shouldAddHomeConsumers)
	{
		upsCons = this->upsConsumption;
	}
	this->upsLosses = 0;// ((inverterPenetration*consumersCount)*upsRating*(1 - invterEfficiency) / invterEfficiency);
	this->upsConsumption = 0;
	this->softUPSPowerConsumption = 0;
	Consumer *consumer;
	//	vector<std::thread> threadPool;
	for (int i = 0; i < consumersCount; i++)
	{

		if (shouldAddHomeConsumers)
		{
			if (gHomeAgents >= gMaxHomeAgents)
			{
				consumersCount = subConsumer.size();
				break;
			}
			//gHomeAgents++;
			bool isAashiyana = (GenerateRandom(0.0, 1.0) < gAashiyanaPenetrationRatio);
			consumer = new HomeConsumer(++gHomeAgents, outFile, isAashiyana);

			consumer->isEliteConsumer = isElite;
			//std::thread thread_param(consumer->RandomizePower, cdfData);

			/*ConsumerData(Consumer::*functionPtr)(vector<double> *) = NULL;
			functionPtr = Consumer::RandomizePower;*/
			//functionPtr = consumer->RandomizePower;
			/*/////
			//Function Pointer

			ConsumerData(*functionPtr)(vector<double> *cdfData);
			functionPtr = consumer->RandomizePower;

			/////*/

			//std::async(consumer->RandomizePower, cdfData);
			//std::thread t(consumer->RandomizePower(cdfData));
			consumer->RandomizePower(cdfData);
			if (i < numberOfUpses)
			{
				((HomeConsumer*)consumer)->haveUps = true;
			}
			this->subConsumer.push_back(consumer);
			gAllNodes.push_back(consumer);
		}
		else
		{
			this->subConsumer[i]->RandomizePower(cdfData);
		}
		this->powerDemand += this->subConsumer[i]->power;
	}
	//for (auto singleThread = threadPool.begin(); singleThread != threadPool.end(); singleThread++)
	//{
	//	if ((*singleThread).joinable())
	//	{
	//		(*singleThread).join();
	//	}
	//}
	//threadPool.clear();
	this->power = this->powerDemand + upsCons;// +upsLosses + softUPSPowerConsumption;
	ConsumerData data;
	data.power = this->power;
	data.demand = this->powerDemand;
	data.losses = this->upsLosses;
	data.upsConsumption = this->upsConsumption;
	data.softUPSPower = this->softUPSPowerConsumption;
	data.consumersCount = this->consumersCount;
	return data;
}

/**
* RandomizePower it randomly increase or decrease the power consumption of consumer
*/
int Consumer::RandomizePower(EventInfo& eventInformation)
{
	Output dataOut;
	int currentHour = GetCurrentHour();
	/*std::string message;
	std::string output;
	message = "UPS-Losees at " + to_string(currentHour) + " hour";
	double upsLosses = this->GetLosses();
	dataOut.upsLosses = upsLosses;
	output = to_string(upsLosses);
	OUTCON(eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
	OUTFILE(outFile, eventInformation.nodeId_, message.c_str(), output.c_str(), 2);

	message = "UPS-Consumption at " + to_string(currentHour) + " hour";
	double upsCons = this->GetUPSConsumption();
	dataOut.upsConsumption = upsCons;
	output = to_string(upsCons);
	OUTCON(eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
	OUTFILE(outFile, eventInformation.nodeId_, message.c_str(), output.c_str(), 2);

	message = "SoftUPS-Consumption at " + to_string(currentHour) + " hour";
	double softUPSCons = this->GetSoftUPSConsumption();
	dataOut.softUPSConsumption = softUPSCons;
	output = to_string(softUPSCons);
	OUTCON(eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
	OUTFILE(outFile, eventInformation.nodeId_, message.c_str(), output.c_str(), 2);*/

	//this->RandomizePower(&gCdfMap[currentHour]);
	this->RandomizePower(NULL);
	dataOut.userComfort = this->GetUserComfort();
	dataOut.actualPowerDemand = this->power;
	dataOut.powerSupplied = this->power;
	FillNumberOfHomesInPowerState(dataOut);
	gOutputData.push_back(dataOut);
	if (gVariableSupply)
	{
		Node::MAX_POWER_GENERATION = (this->power*gSupplyDemandVariationRatio);
	}
	this->LossesCalculation(eventInformation, currentHour);
	int nextEventTime = HOUR_TICK;
	return nextEventTime;
}
/**
* ContainsConsumer checks if the given id is contained in any subconsumer
*/
bool Consumer::ContainsConsumer(const uint_32 id) const
{
	int digits = (int)floor(log10(id));
	int divisor = (int)pow(10, digits);
	int mainId = id / divisor;
	return (this->myId == mainId);
}
/**
* ExecuteEventToBalanceSupplyDemand will generate an appropriate load reduciton mechanism based on  loadSheddingType
*/
void Consumer::ExecuteEventToBalanceSupplyDemand(EventInfo& eventInformation, Node *nodeGeneration, Node *nodeConsumption)
{
	int oldFeederCount = gFeedersTurnedOff.size();
	if (gLoadSheddingType == lt_loadShedding)
	{
		static int groupId = 0;
		int oldPower = this->power;
		LoadShedding(groupId++ % gTotalFeederGroups);
		int currentPower = this->power;
	}
	//else if (gLoadSheddingType == lt_Central)
	//{

	//}
	else
	{
		double powerVariationRequired = eventInformation.powerVariation;
		double overAllDemand = (*nodeConsumption).power;
		GenerateDLC(powerVariationRequired, overAllDemand);
	}
	auto lastOutputDataInstance = gOutputData.back();
	gOutputData.pop_back();
	lastOutputDataInstance.numberOfFeedersTurnedOff += (gFeedersTurnedOff.size() - oldFeederCount);
	lastOutputDataInstance.powerSupplied = this->power;
	FillNumberOfHomesInPowerState(lastOutputDataInstance);
	lastOutputDataInstance.userComfort = this->GetUserComfort();
	gOutputData.push_back(lastOutputDataInstance);
}
/**
* LoadShedding will shutdown the feeders based on their group ID
*/
void Consumer::LoadShedding(int groupId)
{
	if (this->nodeType == nt_Feederes)
	{
		if (this->nodeGroup == groupId)
		{
			this->ShutDownConsumer();
		}
	}
	else
	{
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			this->power -= (*iterator)->power;
			this->powerDemand -= (*iterator)->powerDemand;
			(*iterator)->LoadShedding(groupId);
			this->power += (*iterator)->power;
			this->powerDemand += (*iterator)->powerDemand;
		}
	}
}
/**
* DistributedPowerReduction will reduce power distributedly based on stressFactor
*/
void Consumer::DistributedPowerReduction(double stressFactor, int groupId, bool isTargetAashiyana, bool isEmergency)
{
	for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
	{
		this->power -= (*iterator)->power;
		this->powerDemand -= (*iterator)->powerDemand;
		if (nodeType == nt_Feederes)
		{
			if (nodeGroup == groupId && !isTargetAashiyana)
			{
				TurnOffAllNonAashiyana();
			}
			else if (isTargetAashiyana)
			{
				(*iterator)->DistributedPowerReduction(stressFactor, isTargetAashiyana, isEmergency);
			}
		}
		else
		{
			(*iterator)->DistributedPowerReduction(stressFactor, groupId, isTargetAashiyana, isEmergency);
		}
		this->power += (*iterator)->power;
		this->powerDemand += (*iterator)->powerDemand;
	}
}
/**
* DistributedPowerReduction will reduce power distributedly based on stressFactor
*/
void Consumer::DistributedPowerReduction(double stressFactor, bool isTargetAashiyana, bool isEmergency)
{
	for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
	{
		this->power -= (*iterator)->power;
		this->powerDemand -= (*iterator)->powerDemand;
		if ((*iterator)->nodeType == nt_home)
		{
			if (gLoadSheddingType == lt_DistributedBackOff)
			{
				HomeConsumer *cons = (HomeConsumer*)(*iterator);
				if (isTargetAashiyana && cons->IsHomeAashiyanaEnabled())
				{
					(*iterator)->DistributedPowerReduction(stressFactor, isEmergency);
				}
				else if (!isTargetAashiyana && !cons->IsHomeAashiyanaEnabled())
				{
					(*iterator)->ShutDownConsumer();
				}
			}
			else if (gLoadSheddingType == lt_SmartMeterDistributedBackOff)
			{
				(*iterator)->ShutDownConsumer();
			}
		}
		else
		{
			(*iterator)->DistributedPowerReduction(stressFactor, isTargetAashiyana, isEmergency);
		}
		this->power += (*iterator)->power;
		this->powerDemand += (*iterator)->powerDemand;
	}
}
void Consumer::DistributedPowerReduction(double stressFactor, bool isEmergency)
{
	DistributedPowerReduction(stressFactor, true, isEmergency);
}
/**
* ReducePower will reduce the power of Consumer.
* powerReduction is a negative integer tell how much to reducde power.
*/
//ConsumerData Consumer::ReducePower(double powerReduction)
//{
//	ConsumerData data;
//	double newPower = this->power - powerReduction;
//	double powerNeedToReduce = powerReduction;
//	if (this->nodeType == NodeType::nt_Feederes)
//	{
//		gFeedersTurnedOff.push_back(this->myId);
//		this->ShutDownConsumer();
//	}
//	else
//	{
//		this->loadSheddingCount++;
//		this->SortSubConsumers();
//		if (loadSheddingType == lt_Fair)
//		{
//			sort(this->subConsumer.begin(), this->subConsumer.end(),
//				[](const Consumer* left, const Consumer* right){
//				return (left->loadSheddingCount < right->loadSheddingCount);
//			}); //sort in descending order based on power
//		}
//		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
//		{
//			double iteratorCurrentPower = (*iterator)->power;
//			double iteratorCurrentUpsLosses = (*iterator)->upsLosses;
//			double iteratorCurrentUpsConsumption = (*iterator)->upsConsumption;
//			double iteratorCurrentSoftUpsConsumtion = (*iterator)->softUPSPowerConsumption;
//			double iteratorPowerDemand = (*iterator)->powerDemand;
//			if (powerNeedToReduce <= 1)
//			{
//				break;
//			}
//			else
//			{
//				this->power -= iteratorCurrentPower;
//				this->upsLosses -= iteratorCurrentUpsLosses;
//				this->upsConsumption -= iteratorCurrentUpsConsumption;
//				this->softUPSPowerConsumption -= iteratorCurrentSoftUpsConsumtion;
//				this->powerDemand -= iteratorPowerDemand;
//				ConsumerData subData;
//				if (iteratorCurrentPower > powerNeedToReduce)
//				{
//					subData = (*iterator)->ReducePower(powerNeedToReduce);
//					powerNeedToReduce -= iteratorCurrentPower;
//				}
//				else
//				{
//					powerNeedToReduce -= iteratorCurrentPower;
//					subData = (*iterator)->ShutDownConsumer();
//					powerNeedToReduce += subData.power;
//				}
//				this->power += subData.power;
//				this->upsLosses += subData.losses;
//				this->upsConsumption += subData.upsConsumption;
//				this->softUPSPowerConsumption += subData.softUPSPower;
//				this->powerDemand += subData.demand;
//			}
//		}
//	}
//	//rev this
//	data.power = this->power;
//	data.losses = this->upsLosses;
//	data.upsConsumption = this->upsConsumption;
//	data.softUPSPower = this->softUPSPowerConsumption;
//	data.demand = this->powerDemand;
//	return data;
//}
/**
* ShutDownConsumer will make the power consumption all subconsumers to zero.
* Therefore the power of that consumer will become zero.
*/
void Consumer::ShutDownConsumer()
{
	this->power = 0;
	this->powerDemand = 0;
	this->upsLosses = 0;
	this->upsConsumption = 0;
	this->softUPSPowerConsumption = 0;
	this->loadSheddingCount++;
	ConsumerData data(0, 0, 0, 0, 0, 0);
	if (this->nodeType == nt_Transformer)
	{
		/*this->softUPSPowerConsumption=consumers*upsRating;
		this->power=this->softUPSPowerConsumption;*/
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			(*iterator)->ShutDownConsumer();
			data.demand += (*iterator)->powerDemand;
			data.power += (*iterator)->power;
		}
	}
	else
	{
		if (this->nodeType == NodeType::nt_Feederes)
		{
			gFeedersTurnedOff.push_back(this->myId);
		}
		for (auto consumer = subConsumer.begin(); consumer != subConsumer.end(); consumer++)
		{
			(*consumer)->ShutDownConsumer();
			/*data.softUPSPower += subConsumerData.softUPSPower;
			data.power += subConsumerData.power;
			data.losses += subConsumerData.losses;
			data.upsConsumption += subConsumerData.upsConsumption;*/
			data.demand += (*consumer)->powerDemand;
		}
	}
	this->power = data.power;
	this->softUPSPowerConsumption = data.softUPSPower;
	this->upsLosses = data.losses;
	this->upsConsumption = data.upsConsumption;
	this->powerDemand = data.demand;
	//	return data;
}
/**
* RandomizePower will randomize the power of nodes.
*/
ConsumerData Consumer::RandomizePower(vector<double> *cdfData)
{
	/*double consumerPower=this->power;
	double inverterLosses=this->upsLosses;*/
	ConsumerData subConsumerData;
	subConsumerData.losses = this->upsLosses;
	subConsumerData.upsConsumption = this->upsConsumption;
	subConsumerData.power = this->power;
	subConsumerData.softUPSPower = this->softUPSPowerConsumption;
	subConsumerData.demand = this->powerDemand;
	int numOfSubConsumer = this->subConsumer.size();
	int nodesToRandomize = numOfSubConsumer;// numOfSubConsumer > 0 ? GenerateRandom(1, numOfSubConsumer) : 0;
	vector<Consumer*>::iterator iterator = this->subConsumer.begin();
	for (int i = 0; i < nodesToRandomize; i++)
	{
		subConsumerData.power -= (*iterator)->power; //subtract last power of this consumer
		subConsumerData.losses -= (*iterator)->upsLosses; //subtract last ups loss of this consumer
		subConsumerData.upsConsumption -= (*iterator)->upsConsumption; //subtract last ups loss of this consumer
		subConsumerData.softUPSPower -= (*iterator)->softUPSPowerConsumption; /** subtract last SoftUPS consumtpion
																			of this consumer*/
		subConsumerData.demand -= (*iterator)->powerDemand; //subtract last powerdemand for this consumer
		ConsumerData data;
		if (this->nodeType == nt_Feederes)
		{
			data = (*iterator)->InitTransformers(cdfData);
		}
		else
		{
			data = (*iterator)->RandomizePower(cdfData);
		}
		subConsumerData.losses += (*iterator)->upsLosses; //add new loss of this consumer
		subConsumerData.upsConsumption += (*iterator)->upsConsumption; //add new loss of this consumer
		subConsumerData.power += (*iterator)->power; //add new  power of this consumer
		subConsumerData.softUPSPower += (*iterator)->softUPSPowerConsumption; /** add new SoftUPS consumtpion
														 of this consumer*/
		subConsumerData.demand += (*iterator)->powerDemand; //add last powerdemand for this consumer
		iterator++;
	}
	this->power = subConsumerData.power;
	this->upsLosses = subConsumerData.losses;
	this->upsConsumption = subConsumerData.upsConsumption;
	this->softUPSPowerConsumption = subConsumerData.softUPSPower;
	this->powerDemand = subConsumerData.demand;
	return subConsumerData;
}
//bool Consumer::ValidatePower()
//{
//	double thisActualPower=this->GetPower();
//	return this->power==thisActualPower;
//}
//double Consumer::GetPower()
//{
//	double sumPower=0;
//	if(this->nodeType==nt_Transformer)
//	{
//		sumPower=this->power;
//	}
//	else
//	{
//		for(auto iterator=subConsumer.begin();  iterator!=subConsumer.end(); iterator++)
//		{
//			sumPower+=(*iterator)->GetPower();
//		}
//	}
//	return sumPower;
//}
double Consumer::GetLosses()
{
	double sumPower = 0;
	if (this->nodeType == nt_Transformer)
	{
		//sumPower = this->upsLosses;
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			sumPower += (*iterator)->upsLosses;;
		}
	}
	else
	{
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			sumPower += (*iterator)->GetLosses();
		}
	}
	return sumPower;
}
double Consumer::GetUPSConsumption()
{
	double sumPower = 0;
	if (this->nodeType == nt_Transformer)
	{
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			sumPower += ((HomeConsumer*)(*iterator))->upsConsumption;;
		}
	}
	else
	{
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			sumPower += (*iterator)->GetUPSConsumption();
		}
	}
	return sumPower;
}
double Consumer::GetSoftUPSConsumption()
{
	double sumPower = 0;
	if (this->nodeType == nt_Transformer)
	{
		sumPower = this->softUPSPowerConsumption;
	}
	else
	{
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			sumPower += (*iterator)->GetSoftUPSConsumption();
		}
	}
	return sumPower;
}
float Consumer::GetUserComfort()
{
	float sumUserComfort = 0;
	if (this->nodeType == nt_Transformer)
	{
		//sumPower = this->upsLosses;
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			HomeConsumer *temp = (HomeConsumer*)(*iterator);
			sumUserComfort += temp->GetHomeComfortLevel();
		}
	}
	else
	{
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			sumUserComfort += (*iterator)->GetUserComfort();
		}
	}
	return sumUserComfort;
}
void Consumer::FillWithConsumerData(ConsumerData *data)
{
	this->powerDemand += data->demand;
	this->power += data->power;
	this->upsLosses += data->losses;
	this->upsConsumption += data->upsConsumption;
	this->softUPSPowerConsumption += data->softUPSPower;
	this->consumersCount += data->consumersCount;
}
void Consumer::LossesCalculation(EventInfo& eventInformation, int currentHour)
{
	std::string message;
	std::string output;
	gTotalUpsLosses += upsLosses;

	message = "Randomize-Consumption " + to_string(GetCurrentHour()) + " hour";
	output = to_string(power);
	OUTCON(eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
	OUTFILE(outFile, eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
	//message = "Consumer_losses";
	//output = "Losses= " + to_string(upsLosses) + " at " + to_string(currentHour) + " hour";
	//OUTCON(eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
	//OUTFILE(outFile, eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
	//double actualPower = this->power - this->upsLosses - this->softUPSPowerConsumption;
	//double totalUpsConsumption = this->upsLosses;//actualPower + this->upsLosses;
	//double totalSoftUpsConsumption = this->softUPSPowerConsumption; //actualPower + this->softUPSPowerConsumption; //!=GetSoftUPSConsumption() 
	//gTotalSoftUpsConsumption += this->softUPSPowerConsumption;
	//gSaving += totalUpsConsumption - totalSoftUpsConsumption;
	//message = "SoftUPS Consumption";
	//output = "Consumption= " + to_string(softUPSPowerConsumption) + " at " + to_string(currentHour) + " hour";
	//OUTCON(eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
	//OUTFILE(outFile, eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
}
int Consumer::GetNumberofHomesInPowerState(PowerLevel powerLevel)
{
	int counter = 0;
	if (this->nodeType == nt_Transformer)
	{
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			HomeConsumer *temp = (HomeConsumer*)(*iterator);
			if (temp->GetCurrentPowerLevel() == powerLevel)
			{
				counter++;
			}
		}
	}
	else
	{
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			counter += (*iterator)->GetNumberofHomesInPowerState(powerLevel);
		}
	}
	return counter;

}
void Consumer::FillNumberOfHomesInPowerState(Output &data)
{
	data.homesInPowerLevel0 = GetNumberofHomesInPowerState(PowerLevel0);
	data.homesInPowerLevel1 = GetNumberofHomesInPowerState(PowerLevel1);
	data.homesInPowerLevel2 = GetNumberofHomesInPowerState(PowerLevel2);
	data.homesInPowerLevel3 = GetNumberofHomesInPowerState(PowerLevel3);
	data.homesInPowerLevelFull = GetNumberofHomesInPowerState(PowerLevelFull);
}
void Consumer::GenerateDLC(double powerVariationRequired, double overAllDemand)
{
	if (gLoadSheddingType == lt_DistributedBackOff || gLoadSheddingType == lt_SmartMeterDistributedBackOff)
	{
		static short lastHour = GetCurrentHour();
		static bool shoudTargetAashiyana = false;
		static int groupId = 0;
		static int counter = 0;
		int currentHour = GetCurrentHour();
		if (lastHour == currentHour)
		{
			//Flip traget.
			shoudTargetAashiyana = !shoudTargetAashiyana;
		}
		else
		{
			lastHour = currentHour;
			//As it is new hour so now traget should be Aashiyana homes at first.
			shoudTargetAashiyana = true;
			counter = 0;
			//groupId++;
		}
		//After generation of emergency signal in the same hour
		//if there is there is still demand supply gap then select new group.
		if (!shoudTargetAashiyana)
		{
			groupId++;
		}
		double stressFactor = powerVariationRequired * 100 / overAllDemand;
		double oldPower = this->power;
		if (gLoadSheddingType == lt_SmartMeterDistributedBackOff)
		{
			DistributedPowerReduction(stressFactor, false);
		}
		else
		{
			//First iteration for Aashiyana homes, 2nd for Non-Aashiyana
			//then if still demand supply gap, it will send signal to Aashiyana homes
			//If after 4 iterations there is still supply demand gap it will generate
			//emergency signal
			DistributedPowerReduction(stressFactor, (groupId % gTotalFeederGroups), shoudTargetAashiyana, (counter > 9));
			counter++;
		}
		double currentPower = this->power;
	}
	else if (gLoadSheddingType == lt_Central)
	{
		GenerateCentralPowerReduction(powerVariationRequired, overAllDemand);
	}
}
void Consumer::GenerateCentralPowerReduction(double powerVariationRequired, double overAllDemand)
{
	/*vector<short> groupElements;
	for (int i = 0; i < gTotalFeederGroups; i++)
	{
	groupElements.push_back(i);
	}
	int currentIndexCounter = groupElements.size();

	for (auto iter = groupElements.rbegin(); iter != groupElements.rend();
	iter++, --currentIndexCounter)
	{
	int randomIndex = GenerateRandom(0, currentIndexCounter - 1);

	if (*iter != groupElements.at(randomIndex))
	{
	std::swap(groupElements.at(randomIndex), *iter);
	}
	}*/
	static int group = 0;
	double powerVaried = 0;
	for (int iteration = 0; iteration <= 4 && powerVariationRequired > 0; iteration++)
	{
		for (int i = 0; i < gTotalFeederGroups; i++)
		{

			int currentGroup = group++%gTotalFeederGroups;
			double oldPower = this->power;
			TurnOffAllNonAashiyana(currentGroup);
			powerVaried += (oldPower - this->power);
			powerVariationRequired -= powerVaried;
			if (powerVariationRequired>0)
			{
				CenteralizedPowerReduction(currentGroup, powerVariationRequired);
			}
			if (powerVariationRequired <= 0)
			{
				break;
			}
		}
	}
}
void Consumer::CenteralizedPowerReduction(short groupId, double &powerVariationRequired)
{
	for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
	{
		this->power -= (*iterator)->power;
		this->powerDemand -= (*iterator)->powerDemand;
		if (nodeType == nt_Feederes)
		{
			if (nodeGroup == groupId)
			{
				(*iterator)->CenteralizedPowerReduction(powerVariationRequired);
			}
		}
		else
		{
			(*iterator)->CenteralizedPowerReduction(groupId, powerVariationRequired);
		}
		this->power += (*iterator)->power;
		this->powerDemand += (*iterator)->powerDemand;
	}
}
void Consumer::CenteralizedPowerReduction(double &powerVariationRequired)
{
	if (nodeType == nt_Transformer)
	{
		double powerVaired = 0;
		//double oldPower = this->power;
		//TurnOffAllNonAashiyana();
		////VaryPowerLevelsOfHomes(&subConsumer, powerVariationRequired);
		//powerVaired += (oldPower - this->power);
		//powerVariationRequired -= powerVaired;
		if (powerVariationRequired > 0)
		{
			double oldPower = this->power;
			//TurnOffAllNonAashiyana();
			VaryPowerLevelsOfHomes(&subConsumer, powerVariationRequired);
			powerVaired += (oldPower - this->power);
			powerVariationRequired -= powerVaired;
		}
	}
	else
	{
		for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
		{
			this->power -= (*iterator)->power;
			this->powerDemand -= (*iterator)->powerDemand;
			(*iterator)->CenteralizedPowerReduction(powerVariationRequired);
			this->power += (*iterator)->power;
			this->powerDemand += (*iterator)->powerDemand;
		}
	}

}
void Consumer::VaryPowerLevelsOfHomes(vector<Consumer*> *subConsumer, double powerVariationRequired)
{
	double powerVaired = 0;
	vector<HomeConsumer*> consumers[3];
	float levelThreshold = 0.75;
	for (auto iterator = (*subConsumer).begin(); iterator != (*subConsumer).end(); iterator++)
	{
		if ((*iterator)->nodeType == nt_home)
		{
			HomeConsumer *cons = (HomeConsumer*)(*iterator);
			if (cons->IsHomeAashiyanaEnabled())
			{
				float consIndex = cons->GetConsumptionIndex();
				if (consIndex > 0.75)
				{
					consumers[0].push_back(cons);
				}
				else if (consIndex > 0.50)
				{
					consumers[1].push_back(cons);
				}
				else if (consIndex > 0.25)
				{
					consumers[2].push_back(cons);
				}
			}
		}
	}
	for (int i = 0; i < 3 && ((powerVariationRequired - powerVaired) > 0); i++)
	{
		PowerLevel currentPowerLevel = (PowerLevel)(PowerLevelFull - i - 1);
		int divider = (i == 0 ? 3 : ((i == 1) ? 2 : 1));
		int consumerLevelChangeThreshold = (consumers[i].size()) / divider;
		int counter = 0;
		for (auto iterator = consumers[i].begin(); iterator != consumers[i].end(); iterator++)
		{
			if (counter > consumerLevelChangeThreshold)
			{
				currentPowerLevel = (PowerLevel)(currentPowerLevel - 1);
				counter = 0;
			}
			double oldPower = (*iterator)->power;
			(*iterator)->CenteralizedPowerReduction(currentPowerLevel);
			powerVaired += (oldPower - (*iterator)->power);
			counter++;
			if ((powerVariationRequired - powerVaired) <= 0)
				break;
		}
	}
	this->power -= powerVaired;
}
void Consumer::TurnOffAllNonAashiyana(int groupId)
{
	for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
	{
		this->power -= (*iterator)->power;
		this->powerDemand -= (*iterator)->powerDemand;
		if (nodeType == nt_Feederes)
		{
			if (nodeGroup == groupId)
			{
				TurnOffAllNonAashiyana();
			}
		}
		else
		{
			(*iterator)->TurnOffAllNonAashiyana(groupId);
		}
		this->power += (*iterator)->power;
		this->powerDemand += (*iterator)->powerDemand;
	}
}
void Consumer::TurnOffAllNonAashiyana()
{
	for (auto iterator = subConsumer.begin(); iterator != subConsumer.end(); iterator++)
	{
		this->power -= (*iterator)->power;
		this->powerDemand -= (*iterator)->powerDemand;
		if ((*iterator)->nodeType == nt_home)
		{
			HomeConsumer *cons = (HomeConsumer*)(*iterator);
			if (!cons->IsHomeAashiyanaEnabled())
			{
				(*iterator)->ShutDownConsumer();
			}
		}
		else
		{
			(*iterator)->TurnOffAllNonAashiyana();
		}
		this->power += (*iterator)->power;
		this->powerDemand += (*iterator)->powerDemand;
	}
}
/**
* NormalDistributionRandom generate a normal distribtion.
* It takes two integers mean and standard deviation
*/
int Consumer::NormalDistributionRandom(int mean, int deviation)
{
	std::random_device rd;
	std::mt19937 e2(rd());
	std::normal_distribution<> dist(mean, deviation);
	return dist(e2);
}
/**
* NormalDistributionRandom generate a normal distribtion.
* It takes two integers mean and standard deviation
*/
double Consumer::NormalDistributionRandom(double mean, double deviation)
{
	std::random_device rd;
	std::mt19937 e2(rd());
	std::normal_distribution<> dist(mean, deviation);
	return dist(e2);
}
