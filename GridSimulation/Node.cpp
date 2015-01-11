#include "Node.h"
//using namespace std;
extern	Timers gTimerManager;

extern cVTime globalClock;///Our global clock!
extern vector<Node*> gNodeVector;///vector that will keep track of all the nodes in the Simulator
extern Timers gTimerManager;
//extern long MAX_SIMTIME;

#define NodeConsumptionID 3
#define NodeGenerationID 2
Node::Node(uint_32 nodeID, FILE* file)
{
	myId = nodeID;
	nodeType = nt_dummy;
	outFile = file;
	power = 0;
}

Node::~Node(void)
{
	outFile = NULL;
}
Node::Node(const Node& orig){ //the copy constructor
	*(this) = orig;
}
Node& Node::operator= (const Node& orig){
	if (this == &orig) return *this;


	myId = orig.myId;// the unique node id kept for purposes of identification
	this->power = orig.power;
	this->nodeType = orig.nodeType;
	this->outFile = orig.outFile;

	return *this;
}
bool Node::Init(){

	TimerCallback* tcbPtr;///pointer to insert timers for each node
	EventInfo evInfo;


	if (nodeType == nt_Disco)
		evInfo.eType_ = et_consumption;
	/*else if(nodeType==nt_ZoneDistribution || nodeType==nt_GridStation || nodeType==nt_Transformer)
	evInfo.eType_=et_consumption;*/
	else if (nodeType == nt_Genco)
		evInfo.eType_ = et_generation;
	else if (nodeType == nt_CentralBody)
		evInfo.eType_ = et_BalanceSupplyDemandGap;
	else
		evInfo.eType_ = et_idle;

	evInfo.nodeId_ = myId;

	evInfo.timeLeft_ = (timeTicks)(INIT_SLOT_SYNC);

	tcbPtr = new Event(evInfo);
	gTimerManager.AddTimer(evInfo.timeLeft_, tcbPtr);

	return true;
}
//To increase or decrease consumption stochastically
//int Node::Consumption(EventInfo& eventInformation)
//{
//	int Max=200, Min=-200, range=(Max-Min)+1; 
//	//if(nodeType==nt_ZoneDistribution || nodeType==nt_GridStation || nodeType==nt_Transformer)
//	srand (time(NULL)+globalClock.globalTime+myId);
//	srand(time(NULL)+rand());
//	int diff=((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
//	int oldPower=power;
//	if((int)power+diff>0)
//	{
//		power += diff;
//	}
//	//rand() is not good to generate a random number check 
//	//http://stackoverflow.com/questions/19665818/best-way-to-generate-random-numbers-using-c11-random-library
//
//	//std::random_device rseed;
//	//std::mt19937 rgen(rseed()); // mersenne_twister
//	//std::uniform_int_distribution<int> idist(Min,Max); // [0,100]
//	//power+=dist(mt);
//
//	std::string message="Consumption";
//	if(power<=10)
//	{
//		power=100;
//	}
//	char buffer[100] = {0};
//	int number_base = 10;
//	std::string output = itoa(power, buffer, number_base);
//	OUTCON(eventInformation.nodeId_,message.c_str(),output.c_str(),2);
//	OUTFILE(outFile,eventInformation.nodeId_,message.c_str(),output.c_str(),2);
//	return 10;//(rand()+globalClock.globalTime)%1000;
//
//}
////To increase or decrease Generation stochastically with in limit of MAX_POWER_GENERATION
//int Node::Generation(EventInfo& eventInformation)
//{
//	int Max=200, Min=-200, range=(Max-Min)+1; 
//	srand (time(NULL));
//	srand(time(NULL)+globalClock.globalTime+myId+rand()+rand());
//	int diff=((float(rand()) / float(RAND_MAX)) * (Max - Min)) + Min;
//	//rand() is not good to generate a random number check 
//	//http://stackoverflow.com/questions/19665818/best-way-to-generate-random-numbers-using-c11-random-library
//
//	if((int)power+diff>0)
//	{
//		power += diff;
//	}
//	if(power>MAX_POWER_GENERATION)
//	{
//		power=MAX_POWER_GENERATION;
//	}
//	else if(power<100)
//	{
//		power=100;
//	}
//	char buffer[100] = {0};
//	int number_base = 10;
//	std::string output = itoa(power, buffer, number_base);
//	OUTCON(eventInformation.nodeId_,"Generation",output.c_str(),2);
//	OUTFILE(outFile,eventInformation.nodeId_,"Generation",output.c_str(),2);
//	return 10;//(rand()+globalClock.globalTime)%1000;
//}
/**
* GenerateRandom is a Mersenne twister to generate a random number.
* It takes two integers minimun and maximum and return an integer between these two integers
*/
int Node::GenerateRandom(int min, int max)
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
double Node::GenerateRandom(double min, double max)
{
	std::random_device rseed;
	std::mt19937 rgen(rseed()); // mersenne_twister
	std::uniform_real_distribution<double> distrbution(min, max); //[min,max]
	double randNumber = distrbution(rgen);
	return randNumber;
}
int Node::RandomizePower(EventInfo& eventInformation)
{
	int Max = 200, Min = -200;
	int diff = GenerateRandom(Min, Max);
	if ((int)power + diff > 0)
	{
		power += diff;
	}
	if (power > MAX_POWER_GENERATION)
	{
		power = MAX_POWER_GENERATION;
	}
	else if (power < 100)
	{
		power = 100;
	}
	std::string message = "Randomize-Generation";
	std::string output = to_string(power);
	OUTCON(eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
	OUTFILE(outFile, eventInformation.nodeId_, message.c_str(), output.c_str(), 2);
	return HOUR_TICK;
}
void Node::Dummy(EventInfo& eventInformation)
{
	OUTCON(eventInformation.nodeId_, "Idle", "Called Dummy", 3);
	OUTFILE(outFile, eventInformation.nodeId_, "Idle", "Called Dummy", 3);
}


/**
* GenerateEventToBalanceSupplyDemand checks if the grid is balanced or not,
* if not generate an event to increase generation or decrease generation or
* consumption based on type of imbalance.
*/

void Node::GenerateEventToBalanceSupplyDemand(EventInfo& eventInformation, Node *nodeGeneration, Node *nodeConsumption)
{
	TimerCallback* tcbPtr;///pointer to insert timers for each node
	EventInfo evInfo;
	evInfo.timeLeft_ = (timeTicks)(INIT_SLOT_SYNC);
	double generation = (*nodeGeneration).power;
	//double generationUpperBound= (*nodeGeneration).power+((*nodeGeneration).power*0.1); //+10%
	//double generationLowerBound= (*nodeGeneration).power-((*nodeGeneration).power*0.1); //-10%
	double consumption = (*nodeConsumption).power;
	uint_32 varConsumption = (uint_32)consumption;
	uint_32 varGeneration = (uint_32)generation;
	if (varGeneration > MAX_POWER_GENERATION)
	{
		evInfo.powerVariation = generation - (double)MAX_POWER_GENERATION;
		evInfo.eType_ = et_reduceGeneration;
		evInfo.nodeId_ = (*nodeGeneration).myId;
		tcbPtr = new Event(evInfo);
		gTimerManager.AddTimer(evInfo.timeLeft_, tcbPtr);
		string message = "Generation: " + to_string(generation) + " - Consumption: " + to_string(consumption);
		OUTCON(eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to reduce power generation", 3);
		OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", message.c_str(), 3);
		OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to reduce power generation", 3);
	}

	if (consumption > generation) /** check if consumption is greater than generation*/
	{
		if (varConsumption >= MAX_POWER_GENERATION) /** check if grid is stressed */
		{
			//add increase gen here
			if ((uint_32)generation < MAX_POWER_GENERATION)
			{
				evInfo.powerVariation = (double)MAX_POWER_GENERATION - generation;
				evInfo.eType_ = et_increaseGeneration;
				evInfo.nodeId_ = (*nodeGeneration).myId;
				tcbPtr = new Event(evInfo);
				gTimerManager.AddTimer(evInfo.timeLeft_, tcbPtr);
				string message = "Generation: " + to_string(generation) + " - Consumption: " + to_string(consumption);
				OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", message.c_str(), 3);
				OUTCON(eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to increase power generation", 3);
				OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to increase power generation", 3);
			}
			//** Reduce consumption grid is stressed */
			evInfo.powerVariation = consumption - (double)MAX_POWER_GENERATION;
			//Node::powerReduction=(*nodeGeneration).power-(*nodeConsumption).power;
			evInfo.eType_ = et_reduceConsumption;
			evInfo.nodeId_ = (*nodeConsumption).myId;
			tcbPtr = new Event(evInfo);
			gTimerManager.AddTimer(evInfo.timeLeft_, tcbPtr);
			string message = "Generation: " + to_string(generation) + " - Consumption: " + to_string(consumption);
			OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", message.c_str(), 3);
			OUTCON(eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to reduce power consumption", 3);
			OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to reduce power consumption", 3);
		}
		else /** Grid is not stressed, we have any spare generation capacity we can increase generation */
		{
			evInfo.powerVariation = consumption - generation;
			evInfo.eType_ = et_increaseGeneration;
			evInfo.nodeId_ = (*nodeGeneration).myId;
			tcbPtr = new Event(evInfo);
			gTimerManager.AddTimer(evInfo.timeLeft_, tcbPtr);
			string message = "Generation: " + to_string(generation) + " - Consumption: " + to_string(consumption);
			OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", message.c_str(), 3);
			OUTCON(eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to increase power generation", 3);
			OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to increase power generation", 3);
		}
		/*if (MAX_POWER_GENERATION < 10000)
		{
			MAX_POWER_GENERATION += 1000;
		}*/

	}
	else if (consumption < generation)
	{
		/** consumption is lower than generation,
		* generate event to reduce power generation
		*/
		evInfo.powerVariation = generation - consumption;
		evInfo.eType_ = et_reduceGeneration;
		evInfo.nodeId_ = (*nodeGeneration).myId;
		tcbPtr = new Event(evInfo);
		gTimerManager.AddTimer(evInfo.timeLeft_, tcbPtr);
		string message = "Generation: " + to_string(generation) + " - Consumption: " + to_string(consumption);
		OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", message.c_str(), 3);
		OUTCON(eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to reduce power generation", 3);
		OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to reduce power generation", 3);
	}
	else // grid is balanced
	{
		string message = "Generation: " + to_string(generation) + " - Consumption: " + to_string(consumption);
		OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", message.c_str(), 3);
		OUTCON(eventInformation.nodeId_, "BalanceSupplyDemandGap", "Found demand supply balanced", 4);
		OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", "Found demand supply balanced", 4);
	}
}

void Node::ExecuteEventToBalanceSupplyDemand(EventInfo& eventInformation, Node *nodeGeneration, Node *nodeConsumption)
{
	TimerCallback* tcbPtr;///pointer to insert timers for each node
	EventInfo evInfo;
	evInfo.timeLeft_ = (timeTicks)(INIT_SLOT_SYNC);
	if (eventInformation.eType_ == et_increaseGeneration)
	{
		this->power += eventInformation.powerVariation;
		if (this->power > MAX_POWER_GENERATION)
		{
			evInfo.powerVariation = (*nodeConsumption).power - (double)MAX_POWER_GENERATION;
			this->power = MAX_POWER_GENERATION;
			evInfo.eType_ = et_reduceConsumption;
			evInfo.nodeId_ = NodeConsumptionID;
			tcbPtr = new Event(evInfo);
			gTimerManager.AddTimer(evInfo.timeLeft_, tcbPtr);
			OUTCON(eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to reduce power consumption", 3);
			OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap", "Requested to reduce power consumption", 3);
		}
	}
	else
	{
		this->power -= eventInformation.powerVariation;
	}
	std::string output = to_string(power);
	if (this->nodeType == nt_Genco)
	{
		OUTCON(eventInformation.nodeId_, "BalanceSupplyDemandGap-Genco", output.c_str(), 3);
		OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap-Genco", output.c_str(), 3);
	}
	else if (this->nodeType == nt_Disco)
	{
		OUTCON(eventInformation.nodeId_, "BalanceSupplyDemandGap-Disco", output.c_str(), 3);
		OUTFILE(outFile, eventInformation.nodeId_, "BalanceSupplyDemandGap-Disco", output.c_str(), 3);
	}
}

/*
//Balance supply demand equation. If generation is in excess with respect to
//consumption it should drop generation, if consumption is more than generation
//and generation has not reached it's max limit then it should increase
//generation else reduce consumption
*/
void Node::BalanceSupplyDemandGap(EventInfo& eventInformation)
{
	vector<Node*>::iterator nodeGeneration, nodeConsumption;
	//TODO:: wouldnt it be more efficient to once sort these vectors and then simply index into the Vector using the nodeId?
	//If the Simulator is slow, I might want to do this !

	int nodeGenerationId = NodeGenerationID, nodeConsumptionID = NodeConsumptionID;
	nodeGeneration = find_if(gNodeVector.begin(), gNodeVector.end(), [nodeGenerationId](Node *p) { return p->nodeType == nt_Genco; });
	nodeConsumption = find_if(gNodeVector.begin(), gNodeVector.end(), [nodeConsumptionID](Node *p) { return p->nodeType == nt_Disco; }); //consumption node ID=3

	if (nodeGeneration == gNodeVector.end()) { ///we didnt find the node id ..something fishy!

		cerr << "Genco not found" << endl;
		return;
	}
	else if (nodeConsumption == gNodeVector.end()) { ///we didnt find the node id ..something fishy!

		cerr << "Disco not found" << endl;
		return;
	}
	if (this->nodeType == nt_CentralBody) //if event called by central body check for balance in grid
	{
		GenerateEventToBalanceSupplyDemand(eventInformation, (*nodeGeneration), (*nodeConsumption));
	}
	else //called by consumer or generator
	{
		ExecuteEventToBalanceSupplyDemand(eventInformation, (*nodeGeneration), (*nodeConsumption));
	}
}
