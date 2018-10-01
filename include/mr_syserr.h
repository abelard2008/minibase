/* -*- mode:c++ -*-
 * *
 *
 * minirel system errors.  
 * 
 * class ErrState is a holder for 'global' error
 * states.  There are error codes for each level:
 * 
 * buf - the buffer manager.
 *
 * pf - page file layer.
 *
 * rf - the record page layer
 *
 * rel - the relation layer
 *
 * cat - catalog
 * exec - executor 
 * and needs db - dbase level, when dbase is bigger.
 *
 * parser - parser
 *
 * interface - the parser interface to the optimizer
 *
 * optimizer - optimizer layer
 *
 * */
#ifndef _MR_SYSERR_
#define _MR_SYSERR_

class SysErr {
public:
    enum BufErr {
	BufOK,
	BufFileNotFound,
	BufFileNotOpened,
	BufFileInUse,
	BufFileNotOpen,
	BufFull,
	BufFileTblFull,
	BufMgrNotUp,
	BufBadConst,
	BufWriteError,
	BufReadError,
	BufBadPage,
	BufUnknown // mostly hash table errors.
    };

    enum PFErr {
	PFOK,
	PFFileOpen,
	PFFileNotOpen,
	PFBadPageNum,
	PFPageNotFound,
	PFPageIsFree,
	PFBadHdr,
	PFBadClose,
	PFBadFileState,
	PFFileNotFound,
	PFFileSystemFull,
	PFBufferFull,
	PFUnknown
    };

    enum RFErr {
	RFOK,
	RFFileNotCreated,
	RFTypeMisMatch,
	RFRecNotAdded,
	RFRecNotDeled,
	RFRecNotFound,
	RFRecNotValid,
	RFRecIdNotValid,
	RFWrongMode, // did anything with var records !
	RFBadHdr,
	RFBadScan,
	RFBufferFull,
	RFFileNotFound,
	RFUnknown
    };

    enum RelErr {
	RelOK,
	RelBadAM,
	RelRecBnds, // ref to rec field out of bounds. 
	RelScanClosed,
	RelBadScan,
	RelRSBnds, // ref to field of recordspec out of bnds.
	RelNoSuchField,
	RelBadRelHdr,
	RelBadPage,
	RelRecNotFound,
	RelRecBadInsert,
	RelRecBadDelete,
	RelRecBadUpdate,
	RelBufferFull,
	RelFileNotFound,	
	RelFileNotCreated,
	RelAlreadyOpen,
	RelUnknown
    };
	
    enum CatErr {
	CatOK,
	CatRelNotFound,
	CatRSNotFound,
	CatNotOpened,
	CatClosed,
	CatRelExists,
	CatDBNotFound,
	CatNoCatRel,
	CatNoAttrRel,
	CatNoAMRel,
	CatUnknown
    };
    
    enum ExecErr {
	ExecOK,
	ExecBadAlist,
	ExecAttrNoRel,
	ExecBadAttr,
	ExecBadAttrVal,
	ExecSelNoAV,
	ExecUnknown

    };
    
    enum DbaseErr{
        DBOK,
	DBRelNotFound,
	DBUnknown
    };

    enum ParserErr{
        POK,
        PIllegalChars,
	PParseError,
	PNotQueryNotUtility,
	PSetOp,
	PNotFrom,
	PNotSelect,
	PAggregate,
	PNotWhere,
	PBadOperator,
	PNotGroup,
	PMultiGroup,
	PNotOrder,
	POrder,
	PBadUtility,
	PBadBadAttrType,
	PIntForAttr,
	PIntOverflow,
	PFloatUnderflow,
	PFloatOverflow,
	PBadNode,
	PEmptyWhere,
	PNullUseError,
	PTypeMismatch,
	PBadAttr,
	PBadRel,
	PNonUniqueVar,
	PMissingComma,
	PIntOrFloatForAgg,
	PGroup,
	PHaving,
	PNotHaving,
	PNeedGroupBy,
	PNotSelected
    };
#if 0	
    int SetState(BufErr bfe)  { SysErr::BufState  = bfe; return (bfe == BufOK);};
    int SetState(PFErr pfe)   { SysErr::PFState   = pfe; return (pfe == PFOK); };
    int SetState(RFErr rfe)   { SysErr::RFState   = rfe; return (rfe == RFOK); };
    int SetState(RelErr rle)  { SysErr::RelState  = rle; return (rle == RelOK);};
    int SetState(CatErr cat)  { SysErr::CatState  = cat; return (cat == CatOK);};
    int SetState(ExecErr exe) { SysErr::ExecState = exe; return (exe == ExecOK);};
    int SetState(DbaseErr dbe) { SysErr::DbaseState = dbe; return (dbe == DBOK);};
    int SetState(ParserErr pae) { SysErr::ParserState = pae; return (pae == POK);};

    BufErr   GetBufState()   { return SysErr::BufState; }
    PFErr    GetPFState()    { return SysErr::PFState;  }
    RFErr    GetRFState()    { return SysErr::RFState;  }
    RelErr   GetRelState()   { return SysErr::RelState; }
    CatErr   GetCatState()   { return SysErr::CatState; }
    ExecErr  GetExecState()  { return SysErr::ExecState;}
    DbaseErr GetDBState()    { return SysErr::DbaseState; }	
    ParserErr GetPState()    { return SysErr::ParserState; }

    void ClearState() {
	SysErr::BufState  = BufOK;
	SysErr::PFState   = PFOK;
	SysErr::RFState   = RFOK;
	SysErr::RelState  = RelOK;
	SysErr::CatState  = CatOK;
	SysErr::ExecState = ExecOK;
	SysErr::DbaseState = DBOK;
	SysErr::ParserState = POK;
    };


    int NoError() 
    {
	return ((BufState  == BufOK) &&
		(PFState   == PFOK)  &&
		(RFState   == RFOK)  &&
		(RelState  == RelOK) &&
		(CatState  == CatOK) &&
		(ExecState == ExecOK)&&
		(DbaseState == DBOK) &&
		(ParserState == POK));
    } 

    void ShowErr();
#endif
    static BufErr  BufState;
    static PFErr   PFState;
    static RFErr   RFState;
    static RelErr  RelState;
    static CatErr  CatState;
    static ExecErr ExecState;
    static DbaseErr DbaseState;
    static ParserErr ParserState;

  void ClearState() {

	SysErr::BufState  = BufOK;
	SysErr::PFState   = PFOK;
	SysErr::RFState   = RFOK;
	SysErr::RelState  = RelOK;
	SysErr::CatState  = CatOK;
	SysErr::ExecState = ExecOK;
	SysErr::DbaseState = DBOK;
	SysErr::ParserState = POK;

    };

};


#endif



