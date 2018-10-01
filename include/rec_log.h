/*-----------------------------------------------------------------------------

 Filename = rec_log.h

 Defines all the various kinds of logrecords :
 * commit, 
 * update, 
 * abort,
 * CLR , 
 * checkpoint_begin and 
 * checkpoint_end.
 All are compatible with the logrec class of the log manager.

----------------------------------------------------------------------------*/

#ifndef RECLOGH
#define RECLOGH

#include "lsn.h"
#include "log.h"
#include "rec_type.h"
#include "logrecord.h"
#include "new_error.h"
#include "rec_error.h"

const int RecTRUE  = 1;
const int RecFALSE  = 0;
/*
const int TRUE  = 1;
const int FALSE  = 0;
*/

typedef enum {
        INV,              // Invalid !!
	UPD,		  // Update Log Record
	CLR,              // Compensation Log Record
	COM,		  // Commit Log Record
	CKPT_BEGIN,       // Checkpoint Begin Record
	CKPT_END,	  // Checkpoint End Record
	ABRT } LOG_TYPE;  // Abort Log Record



struct UpdateLogData{
	// fields for update log record.
	LOG_TYPE	type;
	lsn_t		prevlsn; 
	int		xid;	 // transaction id
	PageId		page_id; // page that get updated
	int		offset;	 // offset within the page
	int		size;	 // number of byte get change
	char		data[0]; // undo and redo
};

extern LOG_TYPE GetType(logrecord logrec);
extern void PrintLogRec(logrecord *logrec);

//  Class definition of UpdateLogRec
class UpdateLogRec : public logrecord {
public:
	UpdateLogRec(int /*size*/):logrecord() { 
		 ((UpdateLogData *)data_ptr)->type = UPD; 
        }
	~UpdateLogRec() {}

	//LOG_TYPE GetType() { return ((UpdateLogData *)data_ptr) -> type;}
	lsn_t    GetPrevLSN()  { return ((UpdateLogData *)data_ptr) ->prevlsn;}
	int      Getxid() { return ((UpdateLogData *)data_ptr) -> xid; }
	PageId   GetPageID() { return ((UpdateLogData *)data_ptr) -> page_id;}
	int      GetOffset() { return ((UpdateLogData *)data_ptr) -> offset;}
	int      GetSize() { return ((UpdateLogData *)data_ptr) -> size; }
	char*     GetUndo() { return ((UpdateLogData *)data_ptr) -> data; }
	char*     GetRedo()  { return (((UpdateLogData *)data_ptr) -> data) + 
				       ((UpdateLogData *)data_ptr) -> size; }
	Status   SetPrevLSN(lsn_t newlsn) 
			{ ((UpdateLogData *)data_ptr) ->prevlsn = newlsn; 
			  return OK; }
	Status   Setxid(int newxid) 
			{ ((UpdateLogData *)data_ptr) ->xid = newxid; 
			  return OK; }
	Status   SetPageID(int newpgid) 
			{ ((UpdateLogData *)data_ptr) ->page_id = newpgid; 
			  return OK; }
	Status   SetOffset(int newoffset) 
			{ ((UpdateLogData *)data_ptr) ->offset = newoffset; 
			  return OK; }
	Status   SetSize(int newsize) 
			{ ((UpdateLogData *)data_ptr) ->size = newsize; 
			  set_datalength(sizeof(UpdateLogData) + 2 * newsize);
			  return OK; }

};

struct CLRLogData{
	// fields for CLR
	LOG_TYPE	type;
	lsn_t		prevlsn; 
	lsn_t		undonextlsn;
	int		xid;	 // transaction id
	PageId		page_id; // page that get updated
	int		offset;	 // offset within the page
	int		size;	 // number of byte get change
	char		data[0]; // redo
};

//  Class definition of CLRLogRec
class CLRLogRec : public logrecord {
public:
	CLRLogRec(int /*size*/) : logrecord() { 
		 ((CLRLogData *)data_ptr)->type = CLR;
        }
	~CLRLogRec() {}

	//LOG_TYPE GetType() { return ((CLRLogData *)data_ptr) -> type;}
	lsn_t    GetPrevLSN()  { return ((CLRLogData *)data_ptr) ->prevlsn;}
	lsn_t    GetUndoNextLSN() 
			{ return ((CLRLogData *)data_ptr) ->undonextlsn;}
	int      Getxid() { return ((CLRLogData *)data_ptr) -> xid; }
	PageId   GetPageID() { return ((CLRLogData *)data_ptr) -> page_id;}
	int      GetOffset() { return ((CLRLogData *)data_ptr) -> offset;}
	int      GetSize() { return ((CLRLogData *)data_ptr) -> size; }
	char*     GetRedo() { return ((CLRLogData *)data_ptr) -> data; }
	Status   SetPrevLSN(lsn_t newlsn) 
			{ ((CLRLogData *)data_ptr) ->prevlsn = newlsn; 
			  return OK; }
	Status   SetUndoNextLSN(lsn_t newundonextlsn) 
			{ ((CLRLogData *)data_ptr) ->undonextlsn = 
				newundonextlsn; 
				  return OK; }
	Status   Setxid(int newxid) 
			{ ((CLRLogData *)data_ptr) ->xid = newxid; 
			  return OK; }
	Status   SetPageID(int newpgid) 
			{ ((CLRLogData *)data_ptr) ->page_id = newpgid; 
			  return OK; }
	Status   SetOffset(int newoffset) 
			{ ((CLRLogData *)data_ptr) ->offset = newoffset; 
			  return OK; }
	Status   SetSize(int newsize) 
			{ ((CLRLogData *)data_ptr) ->size = newsize; 
			  set_datalength(sizeof(CLRLogData) + newsize);
			  return OK; }

};

// data for both commit and abort

struct EndLogData {
	LOG_TYPE	type;
	lsn_t		prevlsn; 
	int		xid;	 // transaction id
};

//  Class definition of CommitLogRec

class CommitLogRec : public logrecord {
public:
	CommitLogRec() : logrecord() 
			 { 	
				((EndLogData *)data_ptr) -> type = COM;
			        set_datalength(sizeof(EndLogData));
			 }
	~CommitLogRec() {}

	//LOG_TYPE GetType() { return ((EndLogData *)data_ptr) -> type;}
	lsn_t    GetPrevLSN()  { return ((EndLogData *)data_ptr) ->prevlsn;}
	int      Getxid() { return ((EndLogData *)data_ptr) -> xid; }
	Status   SetPrevLSN(lsn_t newlsn) 
			{ ((EndLogData *)data_ptr) ->prevlsn = newlsn; 
			  return OK; }
	Status   Setxid(int newxid) 
			{ ((EndLogData *)data_ptr) ->xid = newxid; 
			  return OK; }
};

//  Class definition of AbortLogRec
class AbortLogRec : public logrecord {
public:
	AbortLogRec() : logrecord() 
			{ 
				((EndLogData *)data_ptr) -> type =ABRT;
			        set_datalength(sizeof(EndLogData));
		        }
	~AbortLogRec() {}

	//LOG_TYPE GetType() { return ((EndLogData *)data_ptr) -> type;}
	lsn_t    GetPrevLSN()  { return ((EndLogData *)data_ptr) ->prevlsn;}
	int      Getxid() { return ((EndLogData *)data_ptr) -> xid; }
	Status   SetPrevLSN(lsn_t newlsn) 
			{ ((EndLogData *)data_ptr) ->prevlsn = newlsn; 
			  return OK; }
	Status   Setxid(int newxid) 
			{ ((EndLogData *)data_ptr) ->xid = newxid; 
			  return OK; }
};

struct CkptLogData {
	LOG_TYPE	type;
	char		data[0]; // the xact table and dirty page table
};

//  Class definition of CkptBeginLogRec

class CkptBeginLogRec : public logrecord {
public:
	CkptBeginLogRec() : logrecord() 
			    { 
		((CkptLogData *)data_ptr) -> type = CKPT_BEGIN;
	        set_datalength(sizeof(CkptLogData));
        } 
	~CkptBeginLogRec() { }

	//LOG_TYPE GetType() { return ((CkptLogData *)data_ptr) -> type;}
};

//  Class definition of CkptEndLogRec

class CkptEndLogRec : public logrecord {
public:
	CkptEndLogRec(int /*size*/) : logrecord() { 
	        ((CkptLogData *)data_ptr) -> type = CKPT_END;
        }
	~CkptEndLogRec() { }

	//LOG_TYPE GetType() { return ((CkptLogData *)data_ptr) -> type;}
	char*     GetTable() { return ((CkptLogData *)data_ptr) -> data; }

	// user should call SetDirtyTable before SetXactTable
	Status    SetDirtyTable(int NumPage,int IdArray[],lsn_t RecLSN[]);
	Status    SetXactTable(int NumEntries,int Xid[],RecInfo LastLSN[]);
	// the caller provide the space for both array.
	int	  GetDirtyTableSize();
	int	  GetXactTableSize();
	Status    GetDirtyTable(int &NumPage,int IdArray[],lsn_t RecLSN[]);
	Status    GetXactTable(int &NumEntries,int Xid[],lsn_t LastLSN[]);

};

#endif
