#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <tcl.h>
#include "fdset.h"

int FDsetCmd (ClientData clientData, Tcl_Interp *interp, 
			  int argc, char *argv[]);

int FDsetObjectCmd(ClientData clientData, Tcl_Interp *interp,
				   int argc, char *argv[]);

void FDsetDelete(ClientData clientData);
