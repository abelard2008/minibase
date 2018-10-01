#include "tclFD.h"


///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
int FDsetCmd (ClientData /*clientData*/, Tcl_Interp *interp,
	int argc, char **/*argv*/) 
{
	FDset* fdsetPtr;
	static int id =0;

	if (argc !=1) {
		interp->result = "wrong";
		return TCL_ERROR;
	}

	fdsetPtr = new FDset;

	sprintf(interp->result, "fdset%d", id);
	id++;

	Tcl_CreateCommand(interp, interp->result, FDsetObjectCmd,
					  (ClientData) fdsetPtr, FDsetDelete);
	return TCL_OK;
}

///////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////
int FDsetObjectCmd(ClientData clientData, Tcl_Interp *interp,
			  int argc, char *argv[]) {
	
	FDset* fdsetPtr = (FDset *) clientData;
	FD tmp;
	int count;
	char tmpstr[200];

	if (argc < 2) {
		interp->result = "Usage: FDset option ?arg arg...?";
		return TCL_ERROR;
	}

	// parse out commands

	//////////////////////////////////////
	// print command
	if (strcmp(argv[1], "print") ==0 ) {
		if (argc != 2 && argc !=3) {
			interp->result = "Usage: FDset print ?handle?";
			return TCL_ERROR;
		}

		if (argc == 2) {
			sprintf(interp->result, "%d", fdsetPtr->count());
			fdsetPtr->print();
		} 
		else if(argc ==3) {
			fdsetPtr->print(atoi(argv[2]));
		}

	} 
	//////////////////////////////////////
	// add command
	else if (strcmp(argv[1], "add") ==0) {
		if (argc != 4) {
			interp->result = "Usage: FDset add lhs rhs";
			return TCL_ERROR;
		}

		// parse LHS
		char** attrib;
		if ( Tcl_SplitList (interp, argv[2], &count, &attrib) != TCL_OK ) 
			return TCL_ERROR;
		for (int i=0; i<count; i++) {
			String tt = attrib[i];
			tmp.addLHS(tt);
		}

		// parse RHS
		if ( Tcl_SplitList (interp, argv[3], &count, &attrib) != TCL_OK ) 
			return TCL_ERROR;
		for (int i=0; i<count; i++) {
			String tt = attrib[i];
			tmp.addRHS(tt);
		}
		
		count = fdsetPtr->addFD(tmp);
//		sprintf(tmpstr,"%d",count);
		sprintf(interp->result, "%d", count);

	}
	//////////////////////////////////////
	// rem command
	else if (strcmp(argv[1], "rem") ==0) {
		if (argc != 3) {
			interp->result = "Usage: FDset rem handle";
			return TCL_ERROR;
		}
		fdsetPtr->remFD(atoi(argv[2]));

	}
	//////////////////////////////////////
	// size command
	else if (strcmp(argv[1], "size") ==0) {
		if (argc != 2) {
			interp->result = "Usage: FDset size";
			return TCL_ERROR;
		}
		sprintf(interp->result, "%d", fdsetPtr->count());

	}
/*	//////////////////////////////////////
	// lhs command 
	else if (strcmp(argv[1], "lhs") ==0) {
		if (argc != 3) {
			interp->result = "Usage: FDset lhs handle";
			return TCL_ERROR;
		}

		StrSet A = fdsetPtr->getLHS(atoi(argv[2]));
		for (StrSet::iterator i=A.begin(); i!=A.end(); i++) {
			strcpy(tmpstr, *i);
			Tcl_AppendElement(interp,tmpstr);
		}

	}
	//////////////////////////////////////
	// rhs command 
	else if (strcmp(argv[1], "rhs") ==0) {
		if (argc != 3) {
			interp->result = "Usage: FDset rhs handle";
			return TCL_ERROR;
		}

		StrSet A = fdsetPtr->getLHS(atoi(argv[2]));
		for (StrSet::iterator i=A.begin(); i!=A.end(); i++) {
			strcpy(tmpstr, *i);
//		StrSet A = fdsetPtr->getRHS(atoi(argv[2]));
//		for (p = attribs.first(); p!=0; attribs.next(p)) {
//			strcpy(tmpstr, attribs(p).chars() );
			Tcl_AppendElement(interp,tmpstr);
		}

	}
*/
	//////////////////////////////////////
	// set command (change a specific FD)
	else if (strcmp(argv[1], "set") ==0) {
		if (argc != 5) {
			interp->result = "Usage: FDset set [lhs|rhs] list handle";
			return TCL_ERROR;
		}

		// parse LHS
		StrSet A;
		char** attrib;
		if ( Tcl_SplitList (interp, argv[3], &count, &attrib) != TCL_OK ) 
			return TCL_ERROR;
		for (int i=0; i<count; i++) {
			String s = attrib[i];  // for those pesky warnings
//			A.insert(s);
			A.insert(attrib[i]);
		}
		
		if (strcmp(argv[2], "lhs") ==0) {
			fdsetPtr->setLHS(A,atoi(argv[4]));
		}
		if (strcmp(argv[2], "rhs") ==0) {
			fdsetPtr->setRHS(A,atoi(argv[4]));
		}
		// should do error checking

	}
	//////////////////////////////////////
	// isPrime command
	else if (strcmp(argv[1], "isPrime") ==0) {
		if (argc != 4) {
			interp->result = "Usage: FDset isPrime attrib list";
			return TCL_ERROR;
		}

		Attrib A = argv[2];
		// parse list
		char** attrib;
		if ( Tcl_SplitList (interp, argv[3], &count, &attrib) != TCL_OK ) 
			return TCL_ERROR;
		StrSet S;
		for (int i=0; i<count; i++) {
			String s = attrib[i];  // for those pesky warnings
			S.insert(s);
		}
		
		if (fdsetPtr->isPrime(A,S)) {
			Tcl_AppendElement(interp,"TRUE");
		}
		else {
			Tcl_AppendElement(interp,"FALSE");
		}		
			
	}
	//////////////////////////////////////
	// testBCNF command - note RETURNS violations OR TRUE
	else if (strcmp(argv[1], "testBCNF") ==0) {
		if (argc != 3) {
			interp->result = "Usage: FDset testBCNF list";
			return TCL_ERROR;
		}

		// parse list
		char ** attrib;
		if ( Tcl_SplitList (interp, argv[2], &count, &attrib) != TCL_OK ) 
			return TCL_ERROR;
		StrSet S;
		for (int i=0; i<count; i++) {
			String s = attrib[i];  // for those pesky warnings
			S.insert(s);
		}

		list<int> result;
		result = fdsetPtr->testBCNF(S);
		if (result.empty()) {
			Tcl_AppendElement(interp,"TRUE");
		}
		else {
			for (list<int>::iterator i=result.begin(); i!=result.end(); i++) {
				sprintf(tmpstr,"%d",*i);
				Tcl_AppendElement(interp,tmpstr);
			}
		}		
			
	}
	//////////////////////////////////////
	// test3NF command - note RETURNS violations OR TRUE
	else if (strcmp(argv[1], "test3NF") ==0) {
		if (argc != 3) {
			interp->result = "Usage: FDset test3NF list";
			return TCL_ERROR;
		}

		// parse list
		char ** attrib;
		if ( Tcl_SplitList (interp, argv[2], &count, &attrib) != TCL_OK ) 
			return TCL_ERROR;
		StrSet S;
		for (int i=0; i<count; i++) {
			String s = attrib[i];  // for those pesky warnings
			S.insert(s);
		}

		list<int> result;
		result = fdsetPtr->test3NF(S);
		if (result.empty()) {
			Tcl_AppendElement(interp,"TRUE");
		}
		else {
			for (list<int>::iterator i=result.begin(); i!=result.end(); i++) {
				sprintf(tmpstr,"%d",*i);
				Tcl_AppendElement(interp,tmpstr);
			}
		}		
			
	}
	//////////////////////////////////////
	// get command 
	else if (strcmp(argv[1], "get") ==0) {
		if (argc != 3 && argc != 4) {
			interp->result = "Usage: FDset get ?list? ?rhs handle ?lhs handle?";
			return TCL_ERROR;
		}

		if (argc ==3) {
			if (strcmp(argv[2], "list") ==0) {
				count = fdsetPtr->count();
				for (int i=0; i<count; i++) {
					sprintf(tmpstr,"%d ",i);
					Tcl_AppendResult(interp,tmpstr,(char*)NULL);
				}
			} else {
				Tcl_AppendResult(interp,"{",(char*)NULL);
				StrSet A = fdsetPtr->getLHS(atoi(argv[2]));
				for (StrSet::iterator i=A.begin(); i!=A.end(); i++) {
					strcpy(tmpstr, *i);
					Tcl_AppendElement(interp,tmpstr);
				}
				Tcl_AppendResult(interp,"} ",(char*)NULL);
				
				Tcl_AppendResult(interp,"{",(char*)NULL);
				A = fdsetPtr->getRHS(atoi(argv[2]));
				for (StrSet::iterator i=A.begin(); i!=A.end(); i++) {
//				for (p = A.first(); p!=0; A.next(p)) {
					strcpy(tmpstr, *i);
					Tcl_AppendElement(interp,tmpstr);
				}
				Tcl_AppendResult(interp,"}",(char*)NULL);
			}
		}
		else if (argc == 4) {
			if (strcmp(argv[2], "rhs") ==0) {
				StrSet A = fdsetPtr->getRHS(atoi(argv[3]));
				for (StrSet::iterator i=A.begin(); i!=A.end(); i++) {
//				for (p = A.first(); p!=0; A.next(p)) {
					strcpy(tmpstr, *i);
					Tcl_AppendElement(interp,tmpstr);
				}
			}
			else if (strcmp(argv[2], "lhs") ==0) {
				StrSet A = fdsetPtr->getLHS(atoi(argv[3]));
				for (StrSet::iterator i=A.begin(); i!=A.end(); i++) {
//				for (p = A.first(); p!=0; A.next(p)) {
					strcpy(tmpstr, *i);
					Tcl_AppendElement(interp,tmpstr);
				}
			}
		}
	}
	//////////////////////////////////////
	// clear command
	else if (strcmp(argv[1], "clear") ==0) {
		if (argc != 2) {
			interp->result = "Usage: FDset clear";
			return TCL_ERROR;
		}
		fdsetPtr->clear();

	}
	//////////////////////////////////////
	// closure command
	else if (strcmp(argv[1], "attrClosure") ==0) {
		if (argc != 3) {
			interp->result = "Usage: FDset closure ?list?";
			return TCL_ERROR;
		}

		// parse attribs
		char ** attrib;
		StrSet A;
		if ( Tcl_SplitList (interp, argv[2], &count, &attrib) != TCL_OK ) 
			return TCL_ERROR;
		for (int i=0; i<count; i++) {
			String s = attrib[i];
			A.insert(s);
		}

		A = fdsetPtr->attrClosure(A);

		for (StrSet::iterator i=A.begin(); i!=A.end(); i++) {
			strcpy(tmpstr, *i);
			Tcl_AppendElement(interp,tmpstr);
		}
		
	}
	//////////////////////////////////////
	// error in usage
	else {
		Tcl_AppendResult(interp, "bad fdset command \"",
						 argv[1], "\": should be option ?arg arg...?",
						 (char*) NULL);
		return TCL_ERROR;
	}

	return TCL_OK;
}


void FDsetDelete(ClientData clientData) {
	free ((char*) clientData);
}
						 
		  
		  
	  
		  
	   





