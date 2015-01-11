#include "Simulator.h"
extern vector<Node*> gNodeVector;///vector that will keep track of all the nodes in the Simulator
extern vector<Node*> gAllNodes;
extern Timers gTimerManager;
extern cVTime globalClock;
extern double gTotalUpsLosses;
extern double gTotalSoftUpsConsumption;
extern double gSaving;
Simulator::Simulator(FILE *fptr,int numberOfNodes)
{	
	Node *myNode;
	this->fptr=fptr;
	myNode=new Node(1,this->fptr);
	myNode->nodeType=nt_CentralBody;
	myNode->power=0;
	gNodeVector.push_back(myNode);
	gAllNodes.push_back(myNode);
	gNodeVector.back()->Init();

	myNode=new Node(2,this->fptr);
	myNode->nodeType=nt_Genco;
	myNode->power=7000;
	gNodeVector.push_back(myNode);
	gAllNodes.push_back(myNode);
	gNodeVector.back()->Init();
	
	/*Consumer* consumer=new Consumer(3,this->fptr);
	consumer->nodeType=nt_Disco;
	consumer->power=3000;
	gNodeVector.push_back(*consumer);
	delete consumer;*/
	
	myNode=new Consumer(3,this->fptr);
	myNode->nodeType=nt_Disco;
	//myNode->power=8000;
	gNodeVector.push_back(myNode);
	gAllNodes.push_back(myNode);
	gNodeVector.back()->Init();	

	/*for(short i=1; i<=numberOfNodes; i++)
	{
		Node *node =new Node(i,this->fptr);
		srand (time(NULL));
		node->nodeType=(NodeType)((rand()%3)+3);
		gNodeVector.push_back(*node);
		delete node;
		gNodeVector.back().init();
	}*/
}

Simulator::~Simulator(void)
{
	/*if(fptr!=NULL)
	{
		delete fptr;
	}*/
	for_each(gNodeVector.begin(),gNodeVector.end(),[](Node *p){ 
		delete p;
		return true;
	});
}
void Simulator::SimStart(){
	//int count=gNodeVector.size();
	EventLoop();
}
void Simulator::EventLoop(){

	cVTime  nextEventTime;

	//	fprintf(stderr,"Starting Event loop at global time =%d\n", globalClock.globalTime);


	while (globalClock.globalTime <= MAX_SIMTIME) {//there will always be an event in the queue due to the way traffic is generated
//		int count=gTimerManager.GetCount();
		gTimerManager.NextTimerTime(&nextEventTime);//this will return the time left till the next event 
		//fprintf(stderr,"Next Event Time Left = %d\n", nextEventTime.m_localTime);

		if (0 == nextEventTime.m_localTime ) { //this is a cVTime specific interpretation of return val by subTime()
			// Time to execute the event/timer at the head of the queue 
			gTimerManager.ExecuteNextTimer();
			continue;
		}
		if (nextEventTime.m_localTime == MAXVALUE){
			// There are no timers in the event queue 

			//			fprintf(stderr,"Simulator ends at global time %d\n", globalClock.m_localTime);
			break;
		}

		//if not we should advance the global time to the value of the next event and execute it
		globalClock.advance_gTime(nextEventTime.m_localTime);

		// We have reached the point where we have to execute the event at head of queue
		gTimerManager.ExecuteNextTimer();
		// Execute all timers that have expired.
		gTimerManager.NextTimerTime(&nextEventTime);
		while(0 == nextEventTime.m_localTime){
			//Timer at the head of the queue has expired 
			gTimerManager.ExecuteNextTimer();
			gTimerManager.NextTimerTime(&nextEventTime);
		}

	}

	std::string message="Simulator";
	std::string output ="Total UPS losses= "+to_string(gTotalUpsLosses);
	OUTCON(0,message.c_str(),output.c_str(),2);
	OUTFILE(fptr,-1,message.c_str(),output.c_str(),2);
	output ="Total SoftUPS Consumption= "+to_string(gTotalSoftUpsConsumption);
	OUTCON(0,message.c_str(),output.c_str(),2);
	OUTFILE(fptr,-1,message.c_str(),output.c_str(),2);
	output ="Total saving= "+to_string(gSaving);
	OUTCON(0,message.c_str(),output.c_str(),2);
	OUTFILE(fptr,-1,message.c_str(),output.c_str(),2);
	Consumer *consumer = (Consumer*)gNodeVector[2];
	/*output = "Total Demand " + to_string(consumer->powerDemand);
	OUTCON(0, message.c_str(), output.c_str(), 2);
	OUTFILE(fptr, -1, message.c_str(), output.c_str(), 2);*/
	output = "Total power consumption " + to_string(consumer->power);
	OUTCON(0, message.c_str(), output.c_str(), 2);
	OUTFILE(fptr, -1, message.c_str(), output.c_str(), 2);
	output = "Total Consumers " + to_string(consumer->consumersCount);
	OUTCON(0, message.c_str(), output.c_str(), 2);
	OUTFILE(fptr, -1, message.c_str(), output.c_str(), 2);
	return;
}