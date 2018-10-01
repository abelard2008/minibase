/*-----------------------------------------------------------------------------

File Name : recovery_manager.h

-----------------------------------------------------------------------------*/
#ifndef RECOVERYMGRH
#define RECOVERYMGRH

#include "rec_log.h"
#include "os.h"
#include "page.h"
#include "buf.h"
#include "xaction_mgr.h"

class RecXactTbl{

	public:
	
	RecXactTbl();
	~RecXactTbl() {}
	Status AddEntry(lsn_t lsn, int xid);
	Status DeleteEntry(int xid);
	Status SetLSN(int xid, lsn_t lsn);
	lsn_t GetLSN(int xid);
	int Exists(int xid);
	lsn_t  GetMax();
        int    GetCount() { return _count; }
	void  print();
        private:
        lsn_t _lsn_array[MINIBASE_MAX_TRANSACTIONS];
        int   _xid_array[MINIBASE_MAX_TRANSACTIONS];
        int _count ;
};

class RecDirtyPgTbl{

	public:
	RecDirtyPgTbl();
	~RecDirtyPgTbl();
	Status AddEntry(lsn_t lsn, int pgid);
	Status DeleteEntry(int pgid);
	Status SetLSN(int pgid, lsn_t lsn);
	lsn_t  GetLSN(int pgid);
	int Exists(int pgid);
	lsn_t GetMin();
	void  print();

	private:
	int   _pgid_array[MINIBASE_MAX_TRANSACTIONS];
        lsn_t _lsn_array[MINIBASE_MAX_TRANSACTIONS];
        int _count ;
};

class RecoveryMgr {
	public:

	// First construcor is used for checkpoints and restarts.
	RecoveryMgr() { _current_xid = -1; 
			_dpt = new RecDirtyPgTbl;
			_mode_of_operation = 2; //default = Normal mode.
        }
	// Used for Rollbacks..
	RecoveryMgr(int tid) { 
			_current_xid = tid; 
		        _dpt = new RecDirtyPgTbl;
			_mode_of_operation = 2; //default = Normal mode.
	
        }

	~RecoveryMgr() { delete _dpt; }

	Status RollBack(); //Here we do not specify the transaction id.
			   //The transaction number is implicit.
	Status Restart(int count = -1);  

        Status Checkpoint();

	Status RecoverEarliestLSN(lsn_t& RecLSN);

	Status GetRecoveryLSN(int pgid, lsn_t& RecoveryLSN);

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

	Status WriteUpdateLog(int size,int pgid,int offset,
		const void *old_data,const void *new_data,Page* pg);

	/******************************************************/
	/* Function : WriteCommitLog does the following ..

	   create a commit log record
	   fill in xact id.
	   fill in prevlsn
	   write it to the log
	   update the xact table for new lsn.
	   returns the lsn    

	*/

	Status WriteCommitLog(lsn_t& lsn);

	Status WriteAbortLog(lsn_t& lsn);

	int _current_xid;

	private:

	// Methods for the Restart Phase..

	Status _restart_analysis(lsn_t master_lsn, RecXactTbl* xact_tbl,
			         lsn_t &redo_lsn);
	Status _restart_redo(lsn_t redo_lsn, RecXactTbl* xact_tbl);
				
	Status _restart_undo(RecXactTbl* xact_tbl);

	Status _redo_update(Page* pg,logrecord* logrec);

	RecDirtyPgTbl *_dpt;

	int    _mode_of_operation; // Specifies the mode of operation
				   // 1 = Recovery Mode, 2 = Normal mode
	Status _getreclsn(lsn_t RecLSN, lsn_t RetLSN[], int& NumDirtyPg);
};

#endif
