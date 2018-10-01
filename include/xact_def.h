#ifndef _OUT_H
#define _OUT_H

extern int MINIBASE_CurrXid;

#include "os.h"
#include "rec_type.h"       //for RecInfo definition

//#include "error.h"

class trans_lock_info{
public:
	trans_lock_info(){
		_xaction_id = MINIBASE_CurrXid; //global Xid ..
	}
	~trans_lock_info(){}
private:
	int _xaction_id;

};


class Xaction:public SharedRegion
{
public:
	Xaction();  		//ctor for Xaction object
	
	~Xaction();		//dtor for Xaction object;
				//delete Xaction object;
	int 	get_pid();		//return process id;	
	
	int 	get_xid();		//return private _xid;
	
	void 	sleep();		//put this process to sleep;

	trans_lock_info* get_lock();	//return a lock ptr;	
	RecInfo* get_info();		//return a recinfo ptr;
	Xaction* next;
	Status 	wakeup();		//wake up this process;
	char dbname[12];
		
private:
	int 	_xid;		//xaction id for this object;	
	int 	_pid;	
	trans_lock_info* _lockptr;	//for lock group use only	
	RecInfo* _infoptr;		//for rec info group	
//	Semaphore* xsema;	//the semophore associate with each xaction;
};


#endif

