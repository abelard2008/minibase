/*-----------------------------------------------------------------------------

File Name : recovery_manager.h

PUBLIC INTERFACE for the RECOVERY MANAGER GROUP..

Ranjani Ramamurthy & Kar Shun Tsoi

-----------------------------------------------------------------------------*/
#ifndef RECOVERYMGRH
#define RECOVERYMGRH

#include "rec_log.h"
#include "os.h"
#include "pagefile.h"
#include "buf.h"

// global variables used..

extern BufMgr *bm;
extern log    *logMgr;
extern Xaction_table* xactmgr;
extern DB*     db;

class RecoveryMgr {
	public:

	RecoveryMgr() { }

	~RecoveryMgr() {}

	RecErr RollBack(); //Here we do not specify the transaction id.
			   //The transaction number is implicit.
	RecErr Restart();  

        RecErr Checkpoint();

	RecErr RecoverEarliestLSN(lsn_t& RecLSN);

	/*****************************************************/
	/* Function : WriteUpdateLog does the following ..

	   create an update log record
	   fill in the field.
	   fill in xact id.
	   fill in prevlsn
	   write it to the log
	   update the xact table for new lsn.
	   update the PageLSN with the new lsn
	   returns the lsn    

	*/

	RecErr WriteUpdateLog(int size,int pgid,int offset,
		char *old_data,char *new_data,Page* pg);

	/******************************************************/
	/* Function : WriteCommitLog does the following ..

	   create a commit log record
	   fill in xact id.
	   fill in prevlsn
	   write it to the log
	   update the xact table for new lsn.
	   returns the lsn    

	*/

	RecErr WriteCommitLog(lsn_t& lsn);

	/*********************************************************

	WriteAbortLog writes an abort log record to the log file

	*********************************************************/

	RecErr WriteAbortLog(lsn_t& lsn);

	private:

	// ARIES Methods :: Methods for the Restart Phase..

	RecErr _restart_analysis(lsn_t master_lsn, RecXactTbl* xact_tbl,
			         RecDirtyPgTbl* dp_tbl, lsn_t &redo_lsn);
	RecErr _restart_redo(lsn_t redo_lsn, RecXactTbl* xact_tbl,
				 RecDirtyPgTbl* dp_tbl);
	RecErr _restart_undo(RecXactTbl* xact_tbl);

	RecErr _redo_update(Page* pg,logrecord* logrec);
};

#endif
