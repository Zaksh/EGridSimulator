#pragma once
#ifndef _TIMERS_H_
#define _TIMERS_H_

//#include <sys/time.h>
#include <time.h> 
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>
#include <cstdlib>
#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <algorithm>
#include <iomanip>

using namespace std;

#include "simTime.h"
//typedef long handle;
//
//typedef unsigned long timeTicks; ///the basic unit of time increment

#define MAX_TIMER_VALUE    0x7ffffff // no timer present 

/*One abstraction of the virtual time concept in the simulator
*
*Only one instance of this class should be allowed to use *extra* set_gTime() func.. 
* all other instances will just use.
* all others will simply use the standard interface for timer manipulation
*/
class cVTime:public cSimTime{

	public:
	 cVTime(){};
  	 ~cVTime(){};


	//the current virtual time 
	 void getTime();

	//return the current time in the cSimTime object
	 cSimTime* returnCurrentTime();
	
 	//return -1,0,1  for < == >
	 short compareTime(cSimTime* compareWith);

	//add to this time object a specific # of virtual ticks
	 void addTime(timeTicks ticks);

	
	//set time arbitrarily to the value passed
	void setTime(timeTicks ticks) { m_localTime= ticks;};

	//subtract from this time object y	and return their value... return value interpreted by implementor of subclass
	 cSimTime* subTime(cSimTime* y);
	//assign the tv of y to this
	void assign(cSimTime *y);

	//printing for the timeval struct
	void print(){printf(" Virtual Event time = %ld",m_localTime);};

	// the only non-virtual VTime specific funtion and used *only* by a single global instance of this class
	void advance_gTime(timeTicks time) { globalTime += time;};

	timeTicks m_localTime;//the local copy of the object for time keeping purposes
	static timeTicks globalTime; //this is the global virtual time statically initialized to zero 

};

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif // RAND_MAX


/*
 * To make a new timer,
 * subclass TimerCallback and override
 * the Expire() method.
 *
 * If you need to pass parameters to your timer,
 * pass them in the constructor of your subclass.
 *
 * If you allocate data in your callback,
 * free it in the destructor.
 *
 *
 * When the timer fires, Expire() will be called.
 * You can do anything you want there.
 * When you're done, return a value that indicates what happens
 * to the timer:
 *
 * return = 0   re-add timer again to queue with same timeout as last time
 *        > 0   re-add timer to queue with new timeout given by return value
 *        < 0   discard timer (do not re-add it to the queue)
 */
class TimerCallback {
public:
	TimerCallback() {};
  	virtual ~TimerCallback() {};
       	virtual int  Expire() = 0;
};


/*
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

class event {
public:
	cVTime eTime_; //IMPORTANT: change the time type according to the 
					//inherited implementation of cSimTime class you want to use
	int type_;
	void *payload_;
	event *next_;
};

class EventQueue;
/* Creates the Timer Management class. Creates the eventqueue
* The eventqueue is used by the Timer class only. 
* Use the NextTimerTime and ExecuteNextTimer functions to access
* the event queue
*/
class Timers {
public:
	Timers();
	~Timers();

	// Timer API functions:
	
        /* add a timer to the queue, returning the handle
	 * that can be used to cancel it with RemoveTimer
	 * timeout value deifne in ms. 
	 * When the timer fires, Expire() will be called.
	 * You can do anything you want there.
	 * When you're done, return a value that indicates what happens
	 * to the timer:
	 *
	 * return = 0   re-add timer again to queue with same timeout as last time
	 *        > 0   re-add timer to queue with new timeout given by return value
	 *        < 0   discard timer (do not re-add it to the queue)
	 */
  	handle AddTimer(timeTicks tv,TimerCallback *cb);
	
        // remove a timer that's scheduled, returns if it was there.
	bool RemoveTimer(handle hdl);
	
	// returns the timer value at head of the queue
	void NextTimerTime(cSimTime* simTime);
  
	// Executes the timer on the head of the queue
	void ExecuteNextTimer();

	int GetCount(); //added by zaksh for debuggin and learing purpose
	//remove all entries from timer
	void Clear(); 

protected:
	int next_handle_;  // counter of handle ids
	EventQueue *eq_;  // internal list of timers
};

#endif // _TIMERS_H_
