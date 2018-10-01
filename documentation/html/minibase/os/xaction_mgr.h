TRANSACTION MANAGER HEADER
--------------------------

//Filename: xaction_mgr.h

#ifndef _XACT_H
#define _XACT_H


#include <stdlib.h>
#include "new_error.h"
#include "xact_def.h"
#include "os.h"

enum XactErrCodes {X_DELNONEXISTXACT, OS2, OS3, OS4, OS5};

/*************************************************************************

Xaction_manager is a  hash table that will reside in the shared memory.
Hash table has about 200 slots which contain Xaction pointer point to
corresponding Xaction inside the shared memory. 

**************************************************************************/


class Xaction_manager:public SharedRegion
{

public:


	Xaction_manager();	
	/* ctor of Xaction_manager; */

	~Xaction_manager();
	

Status 	begin();	
	/*This is the function which will be called by every xaction 
	at the startup time. It will return a Status to the calling process; */
	

Status 	commit();		
	/* In commit time, this function will be called to do some 
	clean up job. One thing for sure is that it will delete Xaction
	object from Xaction_manager; */


Status 	abort();			
	/* see explanation above*/


Status 	sleep();		
	/*call this function will put the calling process to sleep at 
	xid's semophore.  Note: the caller should know his xid before 
	make sleep call; */
	

Status 	wakeup(int xid);		
	/* Call this function will awake one of the sleeping process
	( if there are any) given they are sleeping on xid's semophore; */

Status 	print();		

int 	get_key(int seq);		/* hash function */
/************************************************************************* 
	From now on, we will put all the "hanger" for other groups who need 
	   access xaction_manager to get some kind of access
**************************************************************************/
	

Status 	GetRecInfo(int xxid,RecInfo &recInfo);

Status 	SetRecInfo(int xxid,RecInfo &recInfo);

Status 	GetAllRecInfo(const char *db_name,int &num_entries,int xid[], 
					RecInfo recInfoList[]);
Status 	get_lock_info(trans_lock_info* &lock_info);
	
	
private:

	int 	_total_xaction;		//total xactin at given time;
	
	int 	_current_seq;		//current xaction sequence number;	
	
	int 	get_cur_seq();	/* get the current xaction sequence number */
	
	int 	get_total_xaction();	/*return  total number of xactions*/
	
	Xaction* freelist;	//hold deleted Xaction object;	
	
	Xaction* _xaction_array[MINIBASE_MAX_TRANSACTIONS];

};




#endif
