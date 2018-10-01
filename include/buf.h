#ifndef BUF_H
#define BUF_H

//#define BM_TRACE
  /* When #defined, BM_TRACE causes a structured trace to be written to the
     std::ostream pointed to by the class variable BufMgr::Trace.  This output is
     used to drive a visualization tool that shows the inner workings of the
     buffer manager during its operations.  You must set BufMgr::Trace to the
     std::ostream of your choice before creating your BufMgr. */

#include "db.h"
#include "page.h"


#define NUMBUF 50     	// Default number of frames, small number for debugging.
#define HTSIZE 5	// Hash Table size





/*******************ALL BELOW are purely local to buffer Manager********/
// class for maintaining information about buffer pool frame
class   BufMgr;  		//forward declaration of BufMgr class

#define REPLACER Clock		// This is the default replacement policy.  You
				//may specify a different policy when you create
				// the buffer manager.

// Create enums for internal errors.
enum bufErrCodes  {HASHTBLERROR, HASHNOTFOUND, BUFFEREXCEEDED, PAGENOTPINNED,
        BADBUFFER, PAGEPINNED, REPLACERERROR, BADBUFFRAMENO,
	VICTIMLISTOVFL, VICTIMDELETEERR, PAGENOTFOUND
        };


class BufDesc {
    	friend class BufMgr;
	friend class BufHashTbl;
private:
    	int   	pageNo; 	// page within file
				// prevframe & nextframe are for BufHashTbl

	int	prevframe;	// -1 stands for this frame is empty
				// otherwise, it either stands for the 
				// frame number of previous frame or 
				// hash table's slot number (actually its
				// value is slot number plus NUMBUF to 
				// distinguish the 2-fold) if it's at the
				// top of the list for that slot.

	int	nextframe;	// -1 stands for no nextframe,end of list

	int	dirty;		// 1(TRUE) stands for this frame is altered
				// 0(FALSE) for clean frames

	int	reading;	// 1(TRUE) stands for a new  page is being
				// brought in this frame. 0(FALSE)otherwise
				// Actually we could also use it to record
				// the old page number by storing 
				// (1+oldpageNo) in it.

	BufDesc(){		// Initialization
		pageNo=prevframe=nextframe=-1;
		dirty=FALSE;
		reading=0;
		}
	~BufDesc(){}
};

// hash table to keep track of pages in the buffer pool
class BufHashTbl
{
        friend  class BufMgr;
    
private:
    	int	ht[HTSIZE]; 	// -1 stands for no page associated
				// otherwise first frameNo is stored
        unsigned        numBuffers;
	BufDesc*        bufTable; // [numBuffers]
				// An array of Descriptors one for a frame

    	int 	hash(int pageNo); 
				// returns value between 0 and HTSIZE-1
public:
    	BufHashTbl( unsigned numBuffers );

    	~BufHashTbl();

    	int 	insert(int pageNo,int frameNo);
				// Put a page(pageNo) into frame(frameNo)

    	int	lookup(int pageNo);  
				// Search a page in Hash Table, return -1
				// if failure, otherwise frame number 

    	int	remove(int frameNo); 
				// Remove the frame from the Hash Table

	void    display();	// Show hash Table contents and frame status
};

class Replacer {
  public:
    virtual int pin( int frameNo );
    virtual int unpin( int frameNo );
    virtual int free( int frameNo );
    virtual int pick_victim() = 0; // Must pin returned frame.
    virtual const char* name() = 0;
    virtual void info();

    unsigned getNumUnpinnedBuffers();

  protected:
    Replacer();
    virtual ~Replacer();

    enum STATE {Available, Referenced, Pinned};

    const BufMgr* mgr;
    friend class BufMgr;
    virtual void setBufferManager( const BufMgr* mgr );

    // These variables are required for the clock algorithm.

    int head;                   // Clock hand.
    unsigned*   pin_count;      // [numBuffers]
    STATE*      state_bit;      // [numBuffers]
};

class Clock : public Replacer
{
  public:

    Clock();
    ~Clock();

    int   pick_victim();
    const char* name() { return "Clock"; }
    void  info();
};


class BufMgr {

private:
    	BufHashTbl 	hashTable;  // hash table only allocated once
    	Page*           bufPool;    // [numBuffers];//physical buffer pool
        unsigned        numBuffers;
	Replacer*       replacer;

        struct victim_data {
	  int frame_num;
	  int page_id;
	};
	/* The victim list keeps track of pages that have been removed 
	from the buffer pool and are dirty and waiting to be written to
	disk */

	unsigned        _valid_victim_entries; //number of valid entries.
	victim_data*    _victim_list; 
	Status _exists_victim_list(int pageid, int& victim_frame);
	Status _add_victim_list(int pageid, int frame_num);
	Status _remove_victim_list(int pageid);

public:

    	BufMgr( int bufsize, Replacer* replacer=0 );
                                // If you provide a replacer, the BufMgr will
                                // free it.

       ~BufMgr();  	
			//flush all valid dirty pages to disk

#ifdef BM_TRACE
    	Status 	pinPage(int PageId_in_a_DB, Page*& page,
			int emptyPage=0, const char* filename=0);
#else
    	Status 	pinPage(int PageId_in_a_DB, Page*& page,int emptyPage=0);
#endif
		// check if this page is in buffer pool, otherwise
		// find a frame for this page , read in and pin it.
		// also write out the old page if it's dirty before reading
		// if emptyPage==TRUE, then actually no read is done to bring
		// the page

#ifdef BM_TRACE
    	Status 	unpinPage(int globalPageId_in_a_DB, int dirty=FALSE, const char* filename=0);
#else
    	Status 	unpinPage(int globalPageId_in_a_DB, int dirty=FALSE);
#endif
		// if pincount>0, decrement it and if it becomes zero,
		// put it in a group of replacement candidates.
		// if pincount=0 before this call, return error.

    	Status 	newPage(int& firstPageId, Page*& firstpage,int howmany=1); 
		
		// call DB object to allocate a run of new pages and 
                // find a frame in the buffer pool for the first page
		// and pin it. If buffer is full, ask DB to deallocate 
		// all these pages and return error

    	Status 	freePage(int globalPageId); 

		// user should call this method if it needs to delete a page
		// this routine will call DB to deallocate the page 

	Status  flushPage(int pageid);
		// Added to flush a particular page of the buffer pool to
		// disk

	Status  flushAllPages();
		// Flushes all pages of the buffer pool to disk


        unsigned getNumBuffers() const { return numBuffers; }
	unsigned getNumUnpinnedBuffers();
	Page* pageInFrame( unsigned frameNo ) { return bufPool + frameNo; }

    	void 	printSelf(void);

#if defined(BM_TRACE)
        static class std::ostream* Trace;   // Set this to your stream to get a trace.
#endif
};

#endif
