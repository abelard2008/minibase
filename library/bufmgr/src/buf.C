#include "buf.h"




/*****************ALL BELOW are purely local to buffer Manager********/
static const char* bufErrMsgs[] = { "hash table error",
				    "hash entry not found",
				    "buffer pool full",
				    "page not pinned",
				    "buffer pool corrupted",
				    "page still pinned",
				    "replacer error",
				    "illegal buffer frame number received by replacer",
				    "victim list overflow",
				    "error deleting from victim list",
		                    "Page not found in the buffer pool"
				   };
static error_string_table bufTable( BUFMGR, bufErrMsgs );

#ifdef DEBUG
int total; //global pincount for the buffer manager - should be a variable
// within the BufMgr class.
#endif
#if defined(BM_TRACE)
class std::ostream* BufMgr::Trace;
#endif


//---------------------------------------------
// The next three methods deal with victim list
// management.
//---------------------------------------------
Status
BufMgr::_exists_victim_list(int pgid, int& victim_frame)
{
 for( unsigned i=0; i < _valid_victim_entries; i++)
   if( _victim_list[i].page_id == pgid )
    {
     victim_frame = _victim_list[i].frame_num;
     return OK;
    }
 victim_frame = -1; //invalid entry
 return BUFMGR;
}

Status
BufMgr::_add_victim_list(int pageid, int frame_num)
{
 if (_valid_victim_entries == numBuffers) 
   return BUFMGR;
 _victim_list[_valid_victim_entries].page_id = pageid;
 _victim_list[_valid_victim_entries].frame_num = frame_num;
 _valid_victim_entries++;
 return OK;
}

Status
BufMgr::_remove_victim_list(int pageid)
{
 unsigned i,index=0,found=0;
 for(i=0; i < _valid_victim_entries; i++)
   if(_victim_list[i].page_id == pageid)
    {
     index = i;
     found = 1;
     break;
    }
 if( i == _valid_victim_entries && !(found) )
   return BUFMGR; // victim_list error..

 // slide things higher up by 1..
 _valid_victim_entries--;
 for(i = index; i < _valid_victim_entries; i++)
  {
   _victim_list[i].page_id = _victim_list[i+1].page_id;
   _victim_list[i].frame_num = _victim_list[i+1].frame_num;
  }
 return OK;
}

//----------------------------------------
// Constructor of the class BufMgr
//----------------------------------------

BufMgr::BufMgr( int bufsize, Replacer* replacerArg )
: hashTable( bufsize )
{

 numBuffers = bufsize;

 bufPool = (Page*) MINIBASE_SHMEM->malloc( numBuffers * sizeof(Page) );

 _valid_victim_entries = 0; 
 _victim_list = (victim_data*)
   MINIBASE_SHMEM->malloc( numBuffers * sizeof(victim_data) );


 for ( unsigned i=0; i < numBuffers; ++i )
  {
   // Construct each buffer frame.
   (void) new(bufPool+i) Page;
   hashTable.bufTable[i].nextframe=i+1;
   _victim_list[i].page_id = 
     _victim_list[i].frame_num = -1; // Invalid entries.
  }

 hashTable.bufTable[numBuffers-1].nextframe=-1;	//end of the free list

 if (replacerArg)
   replacer = replacerArg;
 else {
   char* replacerAddress = MINIBASE_SHMEM->malloc(sizeof(REPLACER));
   replacer = new (replacerAddress) REPLACER;
  }

 replacer->setBufferManager( this );
#if defined(BM_TRACE)
 if ( Trace )
   *Trace << "STARTUP     " << MINIBASE_PAGESIZE << ' ' << numBuffers
     << ' ' << replacer->name() << std::endl;
#endif
}

BufMgr::~BufMgr()
{
#ifdef DEBUG
 std::cout <<"Entering the BM destructor\n"<<std::endl;
 std::cout << "total = numPins - numunPins = " << total << std::endl;
#endif
 // write all the dirty pages to the disk
 unsigned i;
 Status status;

 for (i=0; i< numBuffers; i++)   //write all valid dirty pages to disk
   if ( hashTable.bufTable[i].dirty )
    {
     assert(hashTable.bufTable[i].prevframe>-1);
     PageId pageid = hashTable.bufTable[i].pageNo;
     status=MINIBASE_DB->write_page(pageid, &bufPool[i]);
#if defined(BM_TRACE)
     if ( Trace )
       *Trace << "PAGE_WRITE  " << i << ' ' << pageid << std::endl;
#endif
     if (status!=OK) 
       MINIBASE_CHAIN_ERROR( BUFMGR, status );
     status=(Status)hashTable.remove(i);
     if (status!=OK)
       MINIBASE_FIRST_ERROR( BUFMGR, HASHTBLERROR );
    }	

 delete replacer;
 delete (void*)bufPool;
 delete (void*)_victim_list;
}

Status BufMgr::flushAllPages(){

unsigned int i;
Status status;

 for (i=0; i< numBuffers; i++)   //write all valid dirty pages to disk
   if ( hashTable.bufTable[i].dirty )
    {
     assert(hashTable.bufTable[i].prevframe>-1);
     PageId pageid = hashTable.bufTable[i].pageNo;
     status=MINIBASE_DB->write_page(pageid, &bufPool[i]);
#if defined(BM_TRACE)
     if ( Trace )
       *Trace << "PAGE_WRITE  " << i << ' ' << pageid << std::endl;
#endif
     if (status!=OK) 
       MINIBASE_CHAIN_ERROR( BUFMGR, status );
     status=(Status)hashTable.remove(i);
     if (status!=OK)
       MINIBASE_FIRST_ERROR( BUFMGR, HASHTBLERROR );
    }	
    return OK;
}

Status BufMgr::flushPage(int pageid){

int i;
Status status;

 for (i=0; i< numBuffers; i++)   //write all valid dirty pages to disk
   if( hashTable.bufTable[i].pageNo == pageid) {
       if ( hashTable.bufTable[i].dirty ) {
     	    assert(hashTable.bufTable[i].prevframe>-1);
     PageId pageid = hashTable.bufTable[i].pageNo;
     status=MINIBASE_DB->write_page(pageid, &bufPool[i]);
     if (status!=OK) 
       MINIBASE_CHAIN_ERROR( BUFMGR, status );
     status=(Status)hashTable.remove(i);
     if (status!=OK)
       MINIBASE_FIRST_ERROR( BUFMGR, HASHTBLERROR );
     return OK;
    }	
   }
   return MINIBASE_FIRST_ERROR(BUFMGR,PAGENOTFOUND);
}

#ifdef BM_TRACE
Status BufMgr::pinPage(int pin_pgid, Page*& page,int emptyPage, const char* filename)
#else
    Status BufMgr::pinPage(int pin_pgid, Page*& page,int emptyPage)
#endif
{
#ifdef DEBUG
 total++;
 printf("BufMgr::PinPage,Incremented pin_count to) %d\n",total);
#endif
 int	frameNo;
 int     victim_found=0; //a flag to ind, that the pg is found.
 Page    victim_page;
 Status	status,st;
 int	oldpageNo = -1;
 int	needwrite=0;

 frameNo=hashTable.lookup(pin_pgid);

 if (frameNo<0){				//Not in the buffer pool
   
   // check if the required page is in the victim list

   frameNo=replacer->pick_victim(); // frameNo is pinned
   if (frameNo<0) { 
     page=NULL; 	
     MINIBASE_FIRST_ERROR( BUFMGR, REPLACERERROR );
     return BUFMGR;
    }
   if (hashTable.bufTable[frameNo].prevframe>-1 &&
       hashTable.bufTable[frameNo].dirty==TRUE ){
     needwrite=1;
     oldpageNo=hashTable.bufTable[frameNo].pageNo;

     // put it into the victim_list
     status = _add_victim_list(oldpageNo,frameNo);
     if(status != OK){
       MINIBASE_FIRST_ERROR( BUFMGR, VICTIMLISTOVFL );
       return BUFMGR;
      }
    }

   if (hashTable.bufTable[frameNo].prevframe>-1){
     status=(Status)hashTable.remove(frameNo);
     if (status != OK){
       MINIBASE_FIRST_ERROR( BUFMGR, HASHTBLERROR );
       return	BUFMGR;
      }
    }

   hashTable.bufTable[frameNo].reading=1+oldpageNo;
   //if it's >0 then reading in process
   status=(Status)hashTable.insert(pin_pgid,frameNo);

   if (status != OK){
     MINIBASE_FIRST_ERROR( BUFMGR, HASHTBLERROR );
     return BUFMGR;
    }


   if (needwrite == 1){
     status=MINIBASE_DB->write_page(
				    oldpageNo,
				    &bufPool[frameNo]);
     if (status != OK)
       return MINIBASE_CHAIN_ERROR( BUFMGR, status );
#if defined(BM_TRACE)
     if ( Trace )
       *Trace << "PAGE_WRITE  " << frameNo << ' ' << oldpageNo << std::endl;
#endif

     //remove it from the victim list..
     status = _remove_victim_list(oldpageNo);
     if (status != OK){
       MINIBASE_FIRST_ERROR( BUFMGR, VICTIMDELETEERR );
       return BUFMGR;
      }

    } // end of needwrite..


   //read in the page if not empty
   if ( (emptyPage==FALSE) && (!(victim_found))) {
     status=MINIBASE_DB->read_page(
				   pin_pgid,
				   &bufPool[frameNo]);
#if defined(BM_TRACE)
     if ( Trace )
      {
       *Trace << "PAGE_READ   " << frameNo << ' ' << pin_pgid;
       if (filename) *Trace << ' ' << filename;
       *Trace << std::endl;
      }
#endif
     if (status != OK){
       MINIBASE_CHAIN_ERROR( BUFMGR, status );
       st=(Status)hashTable.remove(frameNo);
       if (st != OK)
	 return MINIBASE_FIRST_ERROR( BUFMGR, HASHTBLERROR );
       st=(Status)replacer->unpin(frameNo);
       if (st != OK)
	 return MINIBASE_FIRST_ERROR( BUFMGR, REPLACERERROR );
       return BUFMGR;
      }
    }


   // set the bufpool[frameNo] to be victim_page
   if(victim_found){
     bufPool[frameNo] = victim_page; //check..
    }

   hashTable.bufTable[frameNo].reading=0;
   page = &bufPool[frameNo];
#if defined(BM_TRACE)
   if ( Trace )
    {
     *Trace << "PAGE_PIN    " << frameNo << ' ' << pin_pgid;
     if (filename) *Trace << ' ' << filename;
     *Trace << std::endl;
    }
#endif
   return OK;
  }else{	// the page is in the buffer pool ( frameNo > 0 )
    page = &bufPool[frameNo];
    if((status=(Status)replacer->pin(frameNo)) != OK){
      MINIBASE_FIRST_ERROR( BUFMGR, REPLACERERROR );
      return  BUFMGR;
     }
#if defined(BM_TRACE)
    if ( Trace )
     {
      *Trace << "PAGE_PIN    " << frameNo << ' ' << pin_pgid;
      if (filename) *Trace << ' ' << filename;
      *Trace << std::endl;
     }
#endif
    return OK;
   }			
}


#ifdef BM_TRACE
Status BufMgr::unpinPage(int PageId_in_a_DB, int dirty, const char* filename)
#else
    Status BufMgr::unpinPage(int PageId_in_a_DB, int dirty)
#endif
{
#ifdef DEBUG
 total--;
 printf("BufMgr::UnPinPage,Decremented pin_count to) %d\n",total);
#endif
 int frameNo;
 if ((frameNo=hashTable.lookup(PageId_in_a_DB))<0){
   MINIBASE_FIRST_ERROR( BUFMGR, HASHNOTFOUND );
   return  BUFMGR;
  }

 if (hashTable.bufTable[frameNo].prevframe<0){
   MINIBASE_FIRST_ERROR( BUFMGR, PAGENOTPINNED );
   return  BUFMGR;
  }
 if ((replacer->unpin(frameNo))!=0){
   MINIBASE_FIRST_ERROR( BUFMGR, REPLACERERROR );
   return  BUFMGR;
  }
 if (dirty == TRUE) hashTable.bufTable[frameNo].dirty = dirty;
#if defined(BM_TRACE)
 if ( Trace )
  {
   if ( dirty )
    {
     *Trace << "PAGE_DIRTY  " << frameNo << ' ' << PageId_in_a_DB;
     if (filename) *Trace << ' ' << filename;
     *Trace << std::endl;
    }
   *Trace << "PAGE_UNPIN  " << frameNo << ' ' << PageId_in_a_DB;
   if (filename) *Trace << ' ' << filename;
   *Trace << std::endl;
  }
#endif
 return OK;
}

//////////////////////////////////////////////////////////////////////////////
// This is called by DB::deallocate_page()
// It frees a page, given  a page Id
//

Status BufMgr::freePage(int globalPageId)
{
 int frameNo;
 Status status;
 frameNo=hashTable.lookup(globalPageId);
 // if globalPageId is not in pool, frameNo<0 then call deallocate
 if (frameNo<0){
   return MINIBASE_DB->deallocate_page(globalPageId);
  }
 if (frameNo>=(int)numBuffers){
   MINIBASE_FIRST_ERROR( BUFMGR, BADBUFFER );
   return  BUFMGR;
  }

 status=(Status)replacer->free(frameNo);
 if (status != OK){
   MINIBASE_FIRST_ERROR( BUFMGR, REPLACERERROR );
   return  BUFMGR;
  }
 status=(Status)hashTable.remove(frameNo);
 if (status != OK){
   MINIBASE_FIRST_ERROR( BUFMGR, HASHTBLERROR );
   return  BUFMGR;
  }
#if defined(BM_TRACE)
 if ( Trace )
   *Trace << "PAGE_FREE   " << frameNo << ' ' << globalPageId << std::endl;
#endif
 status=MINIBASE_DB->deallocate_page(globalPageId);
 if (status != OK){
   MINIBASE_CHAIN_ERROR( BUFMGR, status );
   return  BUFMGR;
  }
 return OK;
}



Status BufMgr::newPage(int& firstPageId, Page*& firstpage,int howmany){
  int	i;
  Status	status,st2;
  if ((status=MINIBASE_DB->allocate_page(firstPageId,howmany))!=OK)
    return MINIBASE_CHAIN_ERROR( BUFMGR, status );
  if ((status=pinPage(firstPageId,firstpage,TRUE))!=OK){
    // rollback because pin failed
    MINIBASE_FIRST_ERROR( BUFMGR, BUFFEREXCEEDED );
    for (i=0;i<howmany;i++){
      st2=MINIBASE_DB->deallocate_page(i+firstPageId);
      if (st2!=OK)
	return MINIBASE_CHAIN_ERROR( BUFMGR, st2 );
     }
    return  BUFMGR;	
   }else
     return	OK;
 }


void	BufMgr::printSelf(void){
#ifdef DEBUG
  std::cout << "total = " << total << std::endl;
#endif
  replacer->info();
 }

unsigned BufMgr::getNumUnpinnedBuffers()
{
 return replacer->getNumUnpinnedBuffers();
}


/*******************************************************
HASHTABLE METHODS
*******************************************************/
int BufHashTbl::hash(int pageNo)
{
 return (pageNo%HTSIZE);
}

BufHashTbl::BufHashTbl( unsigned numBuffersArg )
{
 numBuffers = numBuffersArg;
 for(int i=0; i < HTSIZE; i++)
   ht[i] = -1;

 bufTable = (BufDesc*)
   MINIBASE_SHMEM->malloc( numBuffers * sizeof(BufDesc) );
 for ( unsigned index=0; index < numBuffers; ++index )
   (void) new(bufTable+index) BufDesc;
}

BufHashTbl::~BufHashTbl()
{
 delete (void*)bufTable;
}

int BufHashTbl::insert(int pageNo,int frameNo) 
{				// Insert is not redem
 BufDesc*	tmp=(BufDesc*)&bufTable[frameNo];
 int 	index = hash(pageNo);
 if (tmp->prevframe>-1){
   std::cout<<"WARNING: remove this frame first!!!"<<std::endl;
   exit(1);
  }
 tmp->pageNo=pageNo;
 tmp->dirty=FALSE;
 tmp->prevframe=index+numBuffers; // we use prevframe to record
 // ht index+numBuffers if this frame
 // is at the top of the slot
 tmp->nextframe=ht[index];	// insert this page at the top
 if (ht[index]>-1) bufTable[ht[index]].prevframe=frameNo;	
 // this page(frame) is not the first in the slot
 // let the previous top pointing to the new top	
 ht[index]=frameNo;		// insert current on the top of the slot anyway
 return 0;
}

//-------------------------------------------------------------------	     
// Check if (pageNo) is currently in the buffer pool (ie. in
// the hash table).  If so, return corresponding frameNo. else return 
// HASHNOTFOUND
//-------------------------------------------------------------------
int BufHashTbl::lookup(int pageNo) 
{
 int		frameNo=ht[hash(pageNo)];
 while (frameNo>-1) {
   if (bufTable[frameNo].pageNo == pageNo){
     return frameNo;
    }
   frameNo=bufTable[frameNo].nextframe;	//search for next bucket
  }
 return -1;
}

int	BufHashTbl::remove(int frameNo) 
{				// remove is redem
 BufDesc*	tmp=(BufDesc*)&bufTable[frameNo];
 if (tmp->prevframe>=(int)numBuffers){ // this frame is at the top of the slot
   // so prevframe is numBuffers+index of ht
   ht[tmp->prevframe-numBuffers]=tmp->nextframe;
  }else if (tmp->prevframe>=0){
    // this frame is at the middle of the slot
    // so prevframe is indeed previous frame
    bufTable[tmp->prevframe].nextframe=tmp->nextframe;
   }else{			// this case should not happen
#ifdef DEBUG
     std::cout<<"Warning:This frame is already empty!!!"<<std::endl;
#endif DEBUG
    }
 if (tmp->nextframe>-1) bufTable[tmp->nextframe].prevframe=tmp->prevframe;
 tmp->prevframe=-1;		// This is the indication that the frame is
 tmp->dirty=FALSE;				// empty
 tmp->nextframe=-1;
 tmp->pageNo=-1;
 return 0;
}

void	BufHashTbl::display(){
  int	i;
  int	next;
  std::cout<<"HASH Table contents :FrameNo[PageNo(d)]"<<std::endl;
  for (i=0;i<HTSIZE;i++){
    std::cout<< std::endl << i << "-";
    if (ht[i]==-1) 
      std::cout<<"NONE\t";
    else{
      next=ht[i];
      while(next>-1){
	std::cout<<next<<"["<<bufTable[next].pageNo;
	if (bufTable[next].dirty==TRUE)
	  std::cout<<"(d)";
	std::cout<<"]-";
	next=bufTable[next].nextframe;
       }
      std::cout<<"\t\t";
     }
   }
  std::cout<<std::endl;
 }

Replacer::Replacer()
{
 mgr = NULL;
 pin_count = 0;
 state_bit = 0;
 head = -1;
}

Replacer::~Replacer()
{
 delete [] pin_count;
 delete [] state_bit;
}

void Replacer::setBufferManager( const BufMgr* mgrArg )
{
 delete [] pin_count;
 delete [] state_bit;
 mgr = mgrArg;
 unsigned numBuffers = mgr->getNumBuffers();

 
 char* Sh_PinArr = MINIBASE_SHMEM->malloc((numBuffers * sizeof(int)));
 pin_count = new(Sh_PinArr) unsigned[numBuffers];
 char* Sh_StateArr = MINIBASE_SHMEM->malloc((numBuffers * sizeof(STATE)));
 state_bit = new(Sh_StateArr) STATE[numBuffers];

 for ( unsigned index=0; index < numBuffers; ++index )
  {
   state_bit[index] = Available;
   pin_count[index] = 0;
  }
 head = -1; //maintain the head of the clock.
}

int Replacer::pin( int frameNo )
{
 if( frameNo < 0  ||  frameNo >= (int)mgr->getNumBuffers() )
  {
   MINIBASE_FIRST_ERROR( BUFMGR, BADBUFFRAMENO );
   return -1;
  }
 state_bit[frameNo] = Pinned;
 ++pin_count[frameNo];
 return OK;
}

int Replacer::unpin( int frameNo )
{
 if ( frameNo < 0  || frameNo >= (int)mgr->getNumBuffers() )
  {
   MINIBASE_FIRST_ERROR( BUFMGR, BADBUFFRAMENO );
   return -1;
  }
 if ( pin_count[frameNo] == 0 )
  {
   MINIBASE_FIRST_ERROR( BUFMGR, PAGENOTPINNED );
   return -1;
  }
 --pin_count[frameNo];
 if ( pin_count[frameNo] == 0 )
   state_bit[frameNo] = Referenced;
 return OK;
}

int Replacer::free( int frameNo )
{
#ifdef DEBUG
 total -= pin_count[frameNo];
#endif
 if ( pin_count[frameNo] > 1 )
  {
   MINIBASE_FIRST_ERROR( BUFMGR, PAGEPINNED );
   return -1;
  }
 pin_count[frameNo] = 0;
 state_bit[frameNo] = Available;
 return OK;
}


void Replacer::info()
{
 std::cout << "\nInfo:\nstate_bits:(R)eferenced | (A)vailable | (P)inned"
   << std::endl;

 unsigned numBuffers = mgr->getNumBuffers();

 for( unsigned i = 0; i < numBuffers; ++i )
  {
   if (((i + 1) % 9) == 0)
     std::cout << std::endl;
   std::cout << "(" <<i<<") ";
   switch(state_bit[i]){
    case Referenced: std::cout<<"R\t";break;
		    case Available: std::cout<<"A\t";break;
		    case Pinned: std::cout<<"P\t";break;
		    default: std::cerr<<"ERROR from Replacer.info()"<<std::endl;
		    }
  }
 std::cout << "\npin_counts:\n";
 for ( unsigned i = 0; i < numBuffers; ++i )
  {
   if (((i + 1) % 9) == 0)
     std::cout << std::endl;
   std::cout << "(" <<i<<") "<< pin_count[i]<<"\t";
  }

 std::cout << "\n\n";
}

unsigned Replacer::getNumUnpinnedBuffers()
{
 unsigned numBuffers = mgr->getNumBuffers();
 unsigned answer = 0;

 for ( unsigned index=0; index < numBuffers; ++index )
   if ( pin_count[index] == 0 )
     ++answer;

 return answer;
}



Clock::Clock()
{
}

Clock::~Clock()
{}

int Clock::pick_victim()
{
 unsigned num = 0;
 unsigned numBuffers = mgr->getNumBuffers();

 head = (head+1) % numBuffers;
 while ( state_bit[head] != Available )
  {
   if ( state_bit[head] == Referenced )
     state_bit[head] = Available;

   if ( num == 2*numBuffers )
    {
     MINIBASE_FIRST_ERROR( BUFMGR, BUFFEREXCEEDED );
     return -1;    // no frame available
    }
   ++num;
   head = (head+1) % numBuffers;
  }

 assert( pin_count[head] == 0 );             // Make sure pin_count is 0.
 state_bit[head] = Pinned;                   // Pin this victim so that other
 ++pin_count[head];                          //  process can't pick it as victim (???)

 return head;
}

void Clock::info()
{
 Replacer::info();
 std::cout << "Clock hand:\t" << head;
 std::cout << "\n\n";
}

/*   END OF FILE "buf.C".   */
