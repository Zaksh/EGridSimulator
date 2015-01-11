 #ifndef _SIMTIME_H_
#define _SIMTIME_H_
 
 
#define MAXVALUE    0x7ffffff // no timer present 


 typedef long handle;
 
 typedef  unsigned long timeTicks; ///the basic unit of time increment
 

 class cSimTime{
 public:
	 cSimTime(){};
	 virtual ~cSimTime(){};
 
	 //the current virtual time 
	 virtual void getTime() =0;
	 
	 //return the current time in the cSimTime object
	 virtual cSimTime* returnCurrentTime()=0;
	 
	 //return -1,0,1  for < == >
	 virtual short compareTime( cSimTime* compareWith)=0;
 
	 //add to this time object a specific # of virtual ticks
	 virtual void addTime(timeTicks ticks)=0;
 
	 //set time arbitrarily to the value passed
	 virtual void setTime(timeTicks ticks)=0;
 
	 //subtract from this time object y  and return their value... return value interpreted by implementor of subclass
	 virtual cSimTime* subTime(cSimTime* y)=0 ;

	//assign the time value of one to the other using a pointer variable
	virtual void assign(cSimTime *y)=0;
	 
	 //generic printing funtion
	 virtual void print()=0;
	 
 };
#endif  
