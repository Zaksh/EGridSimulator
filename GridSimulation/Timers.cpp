/*
* timers.cc       : Timer Management Class
* authors         : John Heidemann, Fabio Silva and Alefiya Hussain 
*
* Copyright (C) 2000-2004 by the Unversity of Southern California
* $Id: timers.cc,v 1.2 2004/10/01 21:59:10 johnh Exp $
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*
* A complete copy of the LGPL is at http://www.gnu.org/copyleft/lesser.txt
*/

#include <stdlib.h>
#include <stdio.h>

#include "Timers.h"
timeTicks cVTime::globalTime = 0;


class EventQueue {

	/*
	*  Methods
	*
	*  Eq_Add        inserts an event into the queue
	*  Eq_AddAfter   creates a new event and inserts it after specified number of ticks
	*  Eq_Pop        extracts the first event and returns it
	*  Eq_NextTimer  returns the time of expiration for the next event
	*  Eq_TopInPast  returns true if the head of the timer queue is in the past
	*  Eq_Remove     remove an event from the queue
	*/

public:
	EventQueue(){
		head_ = NULL;
	};
	~EventQueue(){
		/* Empty destructor */
	};

	void Eq_Add(event *n);
	void Eq_AddAfter(int type, void *payload,timeTicks delay_ticks);
	event * Eq_Pop();
	event * Eq_FindEvent(int type);
	event * Eq_FindNextEvent(int type, event *e);
	void Eq_NextTimer(cSimTime* simTime);
	short Eq_TopInPast(cSimTime* simTime);
	void Eq_Print();
	int Eq_Remove(event *e);

	/*
	*  Event methods
	*
	*  Event_SetDelay   sets the expiration time to a delay (in current time ticks) after present time
	*  RandDelay        computes a randomized delay, measured in milliseconds.
	*                   note that most OS timers have granularities on order of
	*                   5-15 ms. timer[2] = { base_timer, variance }
	*/
	event *head_;
private:

	void Event_SetDelay(event *e, timeTicks delay_ticks);
	int RandDelay(int timer[2]);


};

/*
* This class is used to define a timer in the event queue. The timeout provided
* should be in time ticks. cb specifies the function that will be
* called. 
*/
class TimerEntry {
public:
	handle         hdl_;
	timeTicks	    timeout_;
	TimerCallback  *cb_;

	TimerEntry(handle hdl, timeTicks timeout,TimerCallback *cb) : 
	hdl_(hdl), timeout_(timeout), cb_(cb) {};
	~TimerEntry() {};
};


/*
*  cVTime implementation: linear virtual time abstraction 
*/
//return what is the global virtual time 

void cVTime::getTime()
{
	this->m_localTime =  globalTime;
}


cSimTime* cVTime::returnCurrentTime()
{
	static cVTime currentTime;
	currentTime.getTime();
	return (cSimTime*) &currentTime;
}

/*  returns -1, 0, 1  for < == > */
//assume polymorphic y as cVTime object
short cVTime::compareTime( cSimTime* y)
{
	cVTime* z = (cVTime*) y;

	if (this->m_localTime > z->m_localTime)
		return  1;
	else if  (this->m_localTime == z->m_localTime)
		return 0;
	else 
		return -1;
}

/* Returns a pointer to a static structure. returns 0 as m_localtime if we out.time < y.time.*/
//assume polymorphic y as cVTime object
cSimTime* cVTime::subTime(cSimTime* y)
{
	static cVTime retTimeVal;
	cVTime* z = (cVTime*) y;

	if (compareTime(y) < 0) 
		retTimeVal.m_localTime=0;
	else{
		retTimeVal.m_localTime = this->m_localTime - z->m_localTime; //simply subtract the two values. 
	}
	return &retTimeVal;
}

//add to this time object a specific # of virtual ticks
/*  adds ticks to m_localTime */
void cVTime::addTime(timeTicks ticks)
{
	this->m_localTime +=ticks;
}


///do a copy via a base class pointer
void cVTime::assign(cSimTime *y)
{
	cVTime* z = (cVTime*) y;

	this->m_localTime = z->m_localTime;
}

//////////////// cVTime finished ////////////////////




/*
*  Event utility routines. These are used internally by Timers class
*
*
* event queues are used INTERNALY to timer.cc
* to keep track of multiple timers.
*
* These are NOT for external use...
* the right way is to subclass TimerCallback.
* See examples in test-app.{cc,hh}.
*/

/*
*  An event specifies an expiration time, a type, and a payload.
*  An eventQueue is a list of events, kept in sorted order by 
*  expiration time.  
*/

/*  compares times of two events, returns -1, 0, 1 for < == > */
int Event_Cmp(event *x, event *y)
{
	return x->eTime_.compareTime(&y->eTime_);
}

/*
*  EventQueue Methods
*/

void EventQueue::Eq_Add(event *n)
{
	event *ptr = head_;
	event *last = ptr;
	while (ptr && (Event_Cmp(n,ptr) > 0)){
		last = ptr; 
		ptr = ptr->next_;
	}
	if (last == ptr){
		n->next_ = head_;
		head_ = n;
	}
	else{
		n->next_ = ptr;
		last->next_ = n;
	}
}

event * EventQueue::Eq_Pop()
{
	event *e = head_;
	if (e){
		head_ = head_->next_;
		e->next_ = NULL;
	}
	return e;
}

event * EventQueue::Eq_FindEvent(int type)
{
	return Eq_FindNextEvent(type, head_);
}

event * EventQueue::Eq_FindNextEvent(int type, event *e)
{
	while (e){

		if (e->type_ == type)
			return e;

		e = e->next_;
	}

	return e;
}

void EventQueue::Eq_AddAfter(int type, void *payload, timeTicks delay_ticks)
{

	event *e = new event;
	e->type_ = type;
	e->payload_ = payload;
	Event_SetDelay(e,delay_ticks);
	Eq_Add(e);
}

void EventQueue::Event_SetDelay(event *e, timeTicks delay_ticks)
{
	e->eTime_.getTime();//get the current virtual time 

	e->eTime_.addTime(delay_ticks);///now add after the specified delay
	//e->eTime_.print();
}

/* returns pointer to the next timer value  */
void EventQueue::Eq_NextTimer(cSimTime * simTime)
{
	if (head_){

		cSimTime* currentTime, *temp;
		//we can use the above pointer b/c we know the call below will return a pointer to subclass 
		currentTime =  simTime->returnCurrentTime();
		temp=(head_->eTime_.subTime(currentTime));
		simTime->assign(temp);
	}
	else {
		// nothing, so pick a time far in the future
		simTime->setTime(MAX_TIMER_VALUE);
	};
}

///this func needs to be passed the pointer to the currently used time class 
//for correct functioning 
short EventQueue::Eq_TopInPast(cSimTime * simTime)
{
	if (head_){
		cSimTime* currentTime;
		//we can use the above pointer b/c we know the call below will return a pointer to subclass 
		currentTime =  simTime->returnCurrentTime();

		return  ( simTime->compareTime(currentTime) <= 0);
	}
	return 0;
}

void EventQueue::Eq_Print()
{
	event *e = head_;
	for (; (e); e = e->next_){
		e->eTime_.print();		
	}
}

int EventQueue::Eq_Remove(event *e)
{
	event *ce, *pe;

	if (e){
		if (head_ == e){
			head_ = e->next_;
			return 0;
		}

		pe = head_;
		ce = head_->next_;

		while (ce){
			if (ce == e){
				pe->next_ = e->next_;
				return 0;
			}
			else{
				pe = ce;
				ce = ce->next_;
			}
		}
	}

	return -1;
}

/* computes a randomized delay, measured in milliseconds.
* note that most OS timers have granularities on order of 5-15 ms.
* timer[2] = { base_timer, variance }
*/
int EventQueue::RandDelay(int timer[2])
{
	return (int) (timer[0] + ((((float) rand()) /
		((float) RAND_MAX)) - 0.5) * timer[1]);
}


/* Creates the Timer Management class. Creates the eventqueue
* The eventqueue is used by the Timer class only. 
* Use the nextTimer and ExecuteNextTimer functions to access
* the event queue
*/
Timers::Timers()
{
	//	struct timeval tv;

	/*	// Initialize basic stuff
	next_handle_ = 1;
	gettimeofday(&tv,NULL);
	setSeed(&tv);
	*/

	// Initialize event queue
	eq_ = new EventQueue;
}

Timers::~Timers()
{
	Clear();
}
void Timers::Clear()
{
	event *temp;
	TimerEntry *entry;
	//first extract all events from the event queue
	while((temp=eq_->Eq_Pop()) != eq_->head_){
		entry = (TimerEntry *) temp->payload_;
		delete (entry);
		free(temp);
	}
	if(temp !=NULL){
		entry = (TimerEntry *) temp->payload_;
		delete (entry);
		free(temp);
	}

	delete eq_;
}
/*
* This function adds a timer to the event queue. The timeout provided
* should be in timeticks. cb specifies the function that will be
* called. 
*/
handle Timers::AddTimer(timeTicks timeout, TimerCallback *cb)
{
	TimerEntry *entry;

	entry = new TimerEntry(next_handle_, timeout, cb);
	next_handle_++;
	eq_->Eq_AddAfter(1, entry, timeout);

	return entry->hdl_;
}

/*
* Applications can use this function to remove from the eventqueue
* a previously scheduled timer (the handle should be the one returned
* by the AddTimer function)
*/
bool Timers::RemoveTimer(handle hdl)
{
	event *e;
	TimerEntry *entry;
	bool found = false;

	// Find the timer in the queue
	e = eq_->Eq_FindEvent(1);
	while (e){
		entry = (TimerEntry *) e->payload_;
		if (entry->hdl_ == hdl){
			found = true;
			break;
		}

		e = eq_->Eq_FindNextEvent(1, e->next_);
	}

	// If timer found, remove it from the queue
	if (e){
		if (eq_->Eq_Remove(e) != 0){
			fprintf(stderr, "Error: Can't remove event from queue !\n");
			exit(-1);
		}

		// Call the application provided delete function

		delete entry;
		free(e);
	}

	return found;
}

// Returns the expiration value, in abstract notion,  for the first timer on the queue
void Timers::NextTimerTime(cSimTime* simTime)
{
	eq_->Eq_NextTimer(simTime);
}

//GetCount method is added by zaksh for debuggin and learing purpose
int Timers::GetCount()
{
	event *ptr=eq_->head_ ;
	int count =0;
	if(ptr)
	{
		count++;
		while(ptr->next_)
		{
			ptr=ptr->next_;
			count++;
		}
	}
	//cout<<"Your count "<<count<<", ";
	return count;
}

// Executes the first timer callback in the queue 
void Timers::ExecuteNextTimer()
{

	event *e = eq_->Eq_Pop();
	TimerEntry *entry = (TimerEntry *) e->payload_;
	// run it
  	int new_timeout = entry->cb_->Expire();

	if (new_timeout >= 0){
		if (new_timeout > 0){
			// Change the timer's timeout
			entry->timeout_ = (timeTicks)new_timeout;
		}
		eq_->Eq_AddAfter(1, (TimerEntry *) entry, entry->timeout_);
	}
	else{
		delete entry;
	}
	free(e);
}










