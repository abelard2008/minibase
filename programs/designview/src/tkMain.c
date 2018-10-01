/*
 ========================================================================
 Minibase 
 (c) Copyright R. Ramakrishnan, 
 University of Wisconsin at Madison.
 (1995-1996) All Rights Reserved.
 Version 0.0

 See 'COPYRIGHT' file for the full disclaimer and user agreement.
 ========================================================================
*/

/* main.c */
#include <tk.h>
#include "tclFD.h"

main(int argc, char *argv[]) {
	Tk_Main(argc, argv, Tcl_AppInit);
	exit(0);
}

int RaiseTopLevelCmd(ClientData clientData, Tcl_Interp *interp,
		     int argc, char *argv[]);

/*
 * Tcl_AppInit is called from Tcl_Main
 * after the Tcl interpreter has been created,
 * and before the script file
 * or interactive command loop is entered.
 */
int
Tcl_AppInit(Tcl_Interp *interp) {
	/*
	 * Initialize packages
	 * Tcl_Init sets up the Tcl library facility.
	 */
	if (Tcl_Init(interp) == TCL_ERROR) {
		return TCL_ERROR;
	}
	if (Tk_Init(interp) == TCL_ERROR) {
		return TCL_ERROR;
	}
	/*
	 * Define application-specific commands here.
	 */
	Tcl_CreateCommand(interp, "raise_toplevel", RaiseTopLevelCmd,
			(ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

	Tcl_CreateCommand(interp, "FDset", FDsetCmd,
					  (ClientData)NULL, (Tcl_CmdDeleteProc *)NULL);

	/*
	 * Define startup filename. This file is read in
	 * case the program is run interactively.
	 */
	//tcl_RcFileName = "~/.myapp.tcl";
	return TCL_OK;
}

