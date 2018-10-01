SHARED MEMORY HEADER
--------------------

#ifndef _OS_H
#define _OS_H

extern "C" {
#include <stdlib.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

extern int semget( key_t key, int nsems, int semflg );
extern int semop( int semid, struct sembuf* sops, int nsops );
extern int semctl( int semid, int semnum, int cmd, union semun arg );
extern int shmget( key_t key, int size, int shmflg );
extern char *shmat( int shmid, char* shmaddr, int shmflg );
extern int shmdt( char* shmaddr );
extern int shmctl( int shmid, int cmd, struct shmid_ds* buf );
}
#include "new_error.h"


const int MAXSEMN = 500;               //total semaphore num in MiniRel
const unsigned SEM_SHM_SIZE = 2;      //2k for Semaphore class book-keeping
const unsigned TAG_SIZE = 4;          //4 bytes
const unsigned SHM_HEAD_SIZE = 2048;  //2k
const unsigned MUTEX_ALIGN_SIZE = 1024; //at next 1k position of real_shmadd
const unsigned SHM_MGR_HEAD_SIZE = 1024;//1k now;
const unsigned BIT_ALIGN_MASK = 7;    //in binary is 111;

const key_t  DEFAULT_SEM_KEY = 1111;
const key_t  SEM_SHM_KEY = 1114;
const key_t  SHM_KEY = 1115;


class SharedMemory;
class shm_log;
class Xaction_manager;
class BufMgr;

/////////////////////////////////////////////////

// Part 1:  Semaphore class  

/////////////////////////////////////////////////


//this is the class of Semaphore, which will be constructed from UNIX
//system calls, by set a kernal trap to stop a process and a signal to
//wake up a process.
//Every request will be grant from the sem pool, and private pos
//remember which sem in the pool it is. 
//User just have the impression that he gets a new semaphor. 

class Semaphore {

public:
      Semaphore(int init=1, int flag = 0); //constructor
                                           //bin-semaphore if flag=0, 
					   //else general sem
					   //default is bin-sem with init_val=1;
					  
      ~Semaphore();                        //destructor

      void operator delete(void *p);       //this only will be called by
                                           //os group

      int up();                            //semaphore value minus 1, 
					   //for bin-sem, is unlock
      int  down();                         //val + 1,  for bin-sem, is lock
      void remove();                       //release the sem to system when
					   //not needed any more.
private:
      int semid;
      struct sembuf sops1[1];
      struct sembuf sops2[1];
      int flag;
      int pos;                            //the position in the sem array
      int shmid;
      static int first_sem_flag;
      static int sem_count;               //each user-basis, how many sem used
      static char* shmadd;                //start address of the shm 
					  //which array[MAXSEMN] locates
      struct head_info{
          char tag[4];
          int  array[MAXSEMN];            //booking keeping of semaphore pool
      };

      head_info* header;
      
     public:
      static void initialize() {
	first_sem_flag = 1;
	shmadd = 0;
	sem_count = 0;
       }
};


/////////////////////////////////////////////////

// Part 2:  SharedRegion class                  //

/////////////////////////////////////////////////


//SharedRegion base class
//It will be the parent of other classes like BuferPool, locktable, etc. in shm.

class SharedRegion {

public:
     SharedRegion(){}       			//constructor
     ~SharedRegion(){}      			//destructor

     inline void lock(){
                    mutex.down();
                       }         //enter, set semaphore
     inline void unlock(){
                 mutex.up();
                       }         //leave_normally

private:
     Semaphore  mutex;
};






//////////////////////////////////////////////////////////

/* Part3 SharedMemory class                             */

//////////////////////////////////////////////////////////




class SharedMemory{

public:
      //constructor will get a chunk of shared memory using a GLOBALLY
      //know SHM_KEY, and then mapping the shm into the calling process's
      //address space.
      //Since the KEY is globally know, it can be used by all user process,
      //and be used to make sure the shared memory will be mapped to
      //the same location for all calling process.
      //It will also check to see if this is the first calling process,
      //If yes, get shm and mapping, otherwise, only do mapping.

      SharedMemory(int size);       //in kilo_bytes
                                    //HP system has a limit 
				    //use default key

      ~SharedMemory();              //destructor

      char* begin_shmadd();         //the beginngin address of shm

      char* malloc(int size);       //size in bytes
                                    //get bytes  and return the start address

      void lock(){mutex->down();};  //lock shm, needed at startup to avoid
				    //race condition, i.e. only one process
				    //do the initialization
      void unlock(){mutex->up();};



private:

      char* shmadd;                //user see the beginning of shm
      char* real_shmadd;           //real beginning of shm, 
				   //real_shmadd = shmadd+head-size
      int   shmid;
      key_t key;                   //key_t is long
      int   total_size;         

      struct header_info{
	  char tag[4];
	  unsigned long offset;
	  };
      header_info* header;

      Semaphore *mutex; 	   //to ensure mutual exclusion, one example is
                         	   //when two users want to init Miniral 
				   //almost at the same time at startup
				   //use ptr here, since we want to new it in
				   //shm, see lock() and unlock()

      Semaphore *mutex1;           //for malloc() mutex 

};



/////////////////////////////////////////////////////////////////

//  Part4: ShMemMgr class

/////////////////////////////////////////////////////////////////

/*
  This is a class to manage the shared memory, it uses the 
  the Sharedmemory class, and put all the shared stuff, like 
  LockTable, XactionTable, BufMgr(pool), etc in the shared memory
  in ops of its constructor, the user are totally shaded away form the
  details, they only see is a globally available pointer:

       (ShMemMgr*) SM_Mgr

  to get a chunk of shm, do this:

       SM_Mgr->malloc(size);  

  which will give (size)bytes shm, and return the start address of that
  shm.
 */

class ShMemMgr{

public:
       ShMemMgr(int size = 20); //default 20K bytes
       ~ShMemMgr();
       char* malloc(int size);
       int NumUsers();
       Status SetDirectory(BufMgr *BufMgr,
			   Xaction_manager *XactMgr,
			   shm_log *LogInfo,
			   char *DBName,
			   char *LogName);
       Status GetDirectory(BufMgr *&BufMgr,
			   Xaction_manager *&XactMgr,
			   shm_log *&LogInfo,
			   char *&DBName,
			   char *&LogName);

private:
       struct head_info{
         char tag[4];
	 unsigned count;
       };
       head_info* header;

       char* shmadd1;
       SharedMemory* SM;
       struct SharedStuff
       {
           unsigned _buffer_manager;
	   unsigned _xaction_manager;
	   unsigned _log_info;
	   unsigned _dbname;
	   unsigned _logname;
       }; 
       SharedStuff* directory;
};

#endif





