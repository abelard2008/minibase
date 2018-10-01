#include "mr_syserr.h"
#include <ostream>


// error msgs:
char *BufferErrors[13] = {
    "Buffer Mgr: No Error",
    "Buffer Mgr: File not found",
    "Buffer Mgr: System failed to open file",
    "Buffer Mgr: Attempt to modify a file in use",
    "Buffer Mgr: Attempted operation on a file not open",
    "Buffer Mgr: Buffer is full",
    "Buffer Mgr: File table is full",
    "Buffer Mgr: not running",
    "Buffer Mgr: Bad constant value supplied",
    "Buffer Mgr: Disk write failed",
    "Buffer Mgr: Disk read failed",
    "Buffer Mgr: Bad page number",
    "Buffer Mgr: Unknown"
};
    
char *PageFileErrors[13] = {
    "Page File: No errors",
    "Page File: Attempt to open file already opened",
    "Page File: Attempted operation on file not open",
    "Page File: Bad page number",
    "Page File: Page not found",
    "Page File: Request for deleted page",
    "Page File: Page header has been corrupted",
    "Page File: Failed to close file",
    "Page File: File has been corrupted",
    "Page File: File not found",
    "Page File: File System is full", // over quota ? not signalled 
    "Page File: Buffer is full",
    "Page File: Unknown"
};  

char *RecordFileErrors[14] = {
    "Record File: No error",
    "Record File: File not created",
    "Record File: Record type mismatch",
    "Record File: Add record failed",
    "Record File: Delete record failed",
    "Record File: Record not found",
    "Record File: Invalid Record",
    "Record File: Invalid Record Id",
    "Record File: Wrong mode (i.e fixed op on variable record, or vice versa)",
    "Record File: Bad page header",
    "Record File: Bad Scan",
    "Record File: Buffer is full",
    "Record File: File not found",
    "Record File: Unknown"
};

char *RelationErrors[18] = {
    "Relation: No error",
    "Relation: Bad access method",
    "Relation: Record field out of bounds",
    "Relation: Record Scan is finished",
    "Relation: Bad record scan",
    "Relation: Bad Record Field",
    "Relation: Field name not found",
    "Relation: Bad relation header",
    "Relation: Bad page in relation",
    "Relation: Record not found",
    "Relation: Failed to insert record",
    "Relation: Failed to delete record",
    "Relation: Failed to update record",
    "Relation: Buffer full",
    "Relation: File not found",
    "Relation: File not created",
    "Relation: attempt to open Relation already open",
    "Relation: Unknown"
};

char * CatalogErrors[11] = {
    "Catalog: No error",
    "Catalog: Relation not found",
    "Catalog: RecordSpec not found"
    "Catalog: Catalog open failed",
    "Catalog: Catalog is not open",
    "Catalog: Attempt to create a relation that already exists",
    "Catalog: No database",
    "Catalog: No catalog relation",
    "Catalog: No attributes relation",
    "Catalog: No access method relation",
    "Catalog: Unknown"
};

char * ExecutiveErrors[6] = {
    "Executive: No error",
    "Executive: Bad attribute list in project"
    "Executive: Relation not found for attribute",
    "Executive: Bad attribute",
    "Executive: Bad attribute value",
    "Executive: No attribute value in select",
    "Executive: Unknown"
} ;   

char *DataBaseErrors[3] = {
    "DataBase: No error",
    "DataBase: relation not found",
    "DataBase: unknown"
};


char *ParserErrors[] = {
    "No error",
    "Illegal characters",
    "Parsing failed",
    "Neither query nor utility",
    "Set operations not handled",
    "Internal error in ii_ConvertFromParseTree::DoFrom;  Not a from",
    "Internal error in ii_ConvertFromParseTree::DoSelect;  Not a select",
    "Aggregates not handled",
    "Internal error in ii_ConvertFromParseTree::DoWhere;  Not a where",
    "Operator not supported",
    "Internal error in ii_ConvertFromParseTree::DoGroup;  Not a group",
    "Only one attribute in group by allowed",
    "Internal error in ii_ConvertFromParseTree::DoOrder;  Not a order",
    "Order by clause not yet handled",
    "Utility not handled yet",
    "Unknown attribute type",
    "Integer for attribute not handled",
    "Integer overflow",
    "Floating point underflow",
    "Floating point overflow",
    "Unexpected node type in typechk.c",
    "Where clause is empty but from clause has more than one relation",
    "NULL or NOT NULL used for non basic type",
    "Type mismatch",
    "Use of nonexistent attribute",
    "Non-existent relation in From clause",
    "Non unique tuple var",
    "Missing comma or tuple var same as an existing relation name",
    "Need int or float for aggregate",
    "Group by clause not yet handled",
    "Having by clause not yet handled",
    "Internal error in ii_ConvertFromParseTree::DoHaving;  Not a having",
    "Having clause without a group by",
    "Attribute in order clause but not in select"
};


void 
SysErr::ShowErr() {
    if (NoError()) {
	std::cout << "No errors" << endl;
    } else {
	std::cerr << "Error state :" << endl;
	if (BufState != BufOK) {
	    std::cerr << BufferErrors[(int)BufState] << endl;
	};
	if (PFState != PFOK) {
	    std::cerr << PageFileErrors[(int)PFState] << endl;
	};
	if (RFState != RFOK) {
	    std::cerr << RecordFileErrors[(int)RFState] << endl;
	};
	if (RelState != RelOK) {
	    std::cerr << RelationErrors[(int)RelState] << endl;
	};
	if (CatState != CatOK) {
	    std::cerr << CatalogErrors[(int)CatState] << endl;
	};
	if (ExecState != ExecOK) {
	    std::cerr << ExecutiveErrors[(int)ExecState] << endl;
	};
	if (DbaseState != DBOK) {
	    std::cerr << DataBaseErrors[(int)DbaseState] << endl;
	};
	if (ParserState != POK) {
	    std::cerr << ParserErrors[(int)ParserState] << endl;
	};
    }
}
	

SysErr::BufErr  SysErr::BufState  = BufOK;
SysErr::PFErr   SysErr::PFState   = PFOK;
SysErr::RFErr   SysErr::RFState   = RFOK;
SysErr::RelErr  SysErr::RelState  = RelOK;
SysErr::CatErr  SysErr::CatState  = CatOK;
SysErr::ExecErr SysErr::ExecState = ExecOK;
SysErr::DbaseErr SysErr::DbaseState = DBOK;
SysErr::ParserErr SysErr::ParserState = POK;
