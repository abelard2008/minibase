//lock_manager.h

/* 
Authors: Francis Salmon (Salmon@cs) and Gary Zipfel (Zipfel@cs)
Date: 3-31-95

The lock manager has a very simple external interface. A request
can be made to lock a page, upgrade a lock, unlock a page, and unlock
all pages associated with a particular transaction.  Each running transaction 
has its own instance of the lock manager which it will use to lock and
unlock the pages it needs to access. If possible, a page should be locked
before it is actually requested from the buffer manager. 

A lock manager instance is automatically created for each transaction and is
a member of its transaction table entry. 

*/

// FORWARD DECLARATIONS
#ifndef _LOCK_H
#define _LOCK_H

// class lock_table;
// class trans_lock_info;
// class lock_entry;
// class trans_list_element;
// class lock_list_element;
// class deadlock_detector;

// #include <stdio.h>
// #include <stream.h>
// #include "error.h"
// #include "pf.h"
// #include "os.h"
// #include "deadlock.h"

enum lock_type {Shared,Exclusive,IShared,IExclusive,NoLock};
struct page_def 
{
	PageId page_id;  //The page to be locked in the file
	char db_name[80];  //The name of the file in which the page resides
};

class LockMgr {

public:

    LockMgr (Status & /*status*/){}
    //This needs to connect the lock manager to the lock_table and the 
    //transaction info

    ~LockMgr() {}

    /* 
       The following public functions return OK on success, 
       Restart if this transaction must be restarted.
    */
    
    //Request a Shared/IShared or Exclusive/IExclusive lock on a page.  
    //If it is not possible, the OS will be asked to suspend this
    //process until the page becomes available.
    //If the lock request introduces a dependency cycle into the graph, the
    //process requesting the lock will be restarted.  A return code of
    //Restart indicates the transaction must be restarted.
    Status lock_page(page_def /*page*/,lock_type /*lock*/){return OK;}

    //Upgrade a shared lock to an Exclusive lock.
    //An error code of Restart indicates the transaction must be restarted
    //page is the page to upgrade the lock on.  Lock is the new
    //lock you wish to hold on the page.
    Status upgrade_lock(page_def /*page*/,lock_type /*lock*/){return OK;}

    //Unlock a page
    Status unlock_page(page_def /*page*/){return OK;}

private:

};

#endif










