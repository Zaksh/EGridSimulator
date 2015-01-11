#include "Consumer.h"
#include <random>
#include <typeinfo>

extern vector<Node*> gNodeVector;///vector that will keep track of all the nodes in the Simulator


/*************** Event Generation *******************/

//This describes the event expiration routine. Each Event contains in itself
// the info needed to drive the Simulator. 
//This should (on its own) handle all the required event generation and addition to event queue
///needed for the correct Simulator

int Event::Expire(){
	//fprintf(stderr,"Timer Expires for node id %d at Global time = %d \n", eInfo_.nodeId_, globalClock.globalTime);

	vector<Node*>::iterator nodePosition;
	//TODO:: wouldnt it be more efficient to once sort these vectors and then simply index into the Vector using the nodeId?
	//If the Simulator is slow, I might want to do this !

	int nodeId=eInfo_.nodeId_;
	nodePosition = find_if(gNodeVector.begin(), gNodeVector.end(),[nodeId](Node *p) { 
		if (*p == nodeId)
		{
			return true;
		}
		else
		{
			string type=typeid(*p).name();
			if(type.find("Consumer")!= std::string::npos)
			{
				return ((Consumer*)p)->ContainsConsumer(nodeId);
			}
			else
				return false;
		}
		//return *p == nodeId;
	});

	if(nodePosition == gNodeVector.end()) { ///we didnt find the node id ..something fishy!

		cerr<<"node id "<< eInfo_.nodeId_<<" Could not be found "<<endl;

		delete this;//NOTE:: Weird, but this cleans up the memory!

		return -1;
	}

	else{ //now we have the right node execute the event
		switch (eInfo_.eType_){
			/****
			The application.traffic layer events for generation of traffic are handled over here
			*****/
		case et_idle:
			///we have to handle reception of
			(*nodePosition)->Dummy(eInfo_);
			return 1;
			break;
		case et_consumption:
		case et_generation:
			//Handle consumption
			////Note for et_consumption: Due to polymorphism, this call will go to Consumer class not Node class 
			///FOR et_generation, this call will go to node class.
			return (*nodePosition)->RandomizePower(eInfo_);
			//return (*nodePosition).Generation(eInfo_);
			break;
		case et_BalanceSupplyDemandGap:
			(*nodePosition)->BalanceSupplyDemandGap(eInfo_);
			return 1;
			break;
		case et_reduceConsumption:
		case et_reduceGeneration:
		case et_increaseGeneration:
			(*nodePosition)->BalanceSupplyDemandGap(eInfo_);
			delete this;//NOTE:: Weird, but this cleans up the memory!
			return -1;
			break;
		default:
			cerr<<"node id "<< eInfo_.nodeId_<<" presented with unknown event type!"<<endl;
			delete this;//NOTE:: Weird, but this cleans up the memory!
			return -1;
			break;

		}
	}
}