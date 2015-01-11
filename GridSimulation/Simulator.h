#pragma once
#include "Node.h"
#include "Consumer.h"

class Simulator
{
public:
	Simulator(FILE *fptr,int numberOfNodes);
	~Simulator(void);
	void SimStart();///used to give the stimuli to run the Simulator
	void EventLoop();//run through all possible events
	timeTicks MAX_SIMTIME;
	FILE* fptr;	
};
