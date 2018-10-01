#ifndef REC_TYPEH
#define REC_TYPEH

#include <stdio.h>
#include "lsn.h"

class RecInfo {
private:
	lsn_t last_lsn;
	lsn_t min_lsn;

friend class RecoveryMgr;
friend class CkptEndLogRec;
public:
	RecInfo():last_lsn(),min_lsn(){}
	~RecInfo() {}
	void RecPrint(){
	last_lsn.print();
	printf("              ");
	min_lsn.print();
	};
};

#endif
