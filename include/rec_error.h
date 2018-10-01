#ifndef RECERROR
#define RECERROR

enum recovery_mgr_errors {
	REC_ERR_UNKNOWNLOGREC,
	REC_ERR_NOTBEGINCKPT,
	REC_ERR_TABLEFULL,
	REC_ERR_NOMATCH,
	REC_ERR_FAILEDTOSETLSN,
	REC_ERR_FAILEDTOGETLSN,
	REC_ERR_FAILEDTOGETXACTTBL,
	REC_ERR_FAILEDTOGETDPTBL,
	REC_ERR_FAILEDTOSETXACTTBL,
	REC_ERR_FAILEDTOSETDPTBL,
	REC_ERR_PAGEERR,
	REC_ERR_REDOUPDATEERR,
	REC_ERR_ANALYSISFAILED,
	REC_ERR_REDOFAILED,
	REC_ERR_UNDOFAILED,
	REC_ERR_CHECKPOINT,
	REC_ERR_LOGFAILED,
	REC_ERR_BUFFAILED,
	REC_ERR_XACTMGRERR,
	REC_ERR_LOGRECRDERR
	};

#endif
	