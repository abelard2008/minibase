/* 
 * tkAppInit.C -- 
 *
 * ---------------------------------------------------------------------
 * NOTE: This file was modified by adding the tree widget 
 *       as well as the BLT, Itcl and TclX extensions.
 *       It was also modified to be compiled with a C++ compiler,
 *       although this is not strictly necessary. It is only required
 *       that the actual "main()" be compiled with C++, not the rest
 *       of tkAppInit.c -- Allan
 * ---------------------------------------------------------------------
 *
 * Copyright (c) 1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#ifndef lint
static char sccsid[] = "@(#) tkAppInit.c 1.12 94/12/17 16:30:56";
#endif /* not lint */

#include <stdlib.h>


/* declare command procedures here */
extern "C" {
#include "tk.h"
extern int Blt_Init(Tcl_Interp *interp);
extern int Itcl_Init(Tcl_Interp *interp);
extern int TclXCmd_Init(Tcl_Interp *interp);
extern int Tree_Init(Tcl_Interp *interp);
extern int Dir_Init(Tcl_Interp *interp);
extern int BTree_Init(Tcl_Interp *interp);
}

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

#ifdef NEED_MATHERR
extern "C" int matherr();
int *tclDummyMathPtr = (int *) matherr;
#endif

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	This is the main program for the application.
 *
 * Results:
 *	None: Tk_Main never returns here, so this procedure never
 *	returns either.
 *
 * Side effects:
 *	Whatever the application does.
 *
 *----------------------------------------------------------------------
 */

int
main(int argc, char** argv)
{
    Tk_Main(argc, argv, Tcl_AppInit);
    return 0;			/* Needed only to prevent compiler warning. */
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_AppInit --
 *
 *	This procedure performs application-specific initialization.
 *	Most applications, especially those that incorporate additional
 *	packages, will have their own version of this procedure.
 *
 * Results:
 *	Returns a standard Tcl completion code, and leaves an error
 *	message in interp->result if an error occurs.
 *
 * Side effects:
 *	Depends on the startup script.
 *
 *----------------------------------------------------------------------
 */

extern "C" int
Tcl_AppInit(Tcl_Interp *interp)
{
    Tk_Window main;

    main = Tk_MainWindow(interp);

    if (Tcl_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }
    if (Tk_Init(interp) == TCL_ERROR) {
	return TCL_ERROR;
    }

    /*
     * Call the init procedures for included packages.  Each call should
     * look like this:
     *
     * if (Mod_Init(interp) == TCL_ERROR) {
     *     return TCL_ERROR;
     * }
     *
     * where "Mod" is the name of the module.
     */


    /* --------------------------------------------------
     * Changes for Tree Widget:
     *
     * Note: the C++ tree widget does not need BLT, TclX and Itcl, 
     * but the demos (and the Itcl classes) depend on them.
     */
    if (Itcl_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    if (Blt_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    /* add tclX commands, but not the whole tclX env */
    TclXCmd_Init(interp);


    /* initialize the tree command */
    if (Tree_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    /* initialize the Dir command */
    if (Dir_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    /* initialize the BTree command */
    if (BTree_Init(interp) == TCL_ERROR) {
        return TCL_ERROR;
    }

    /* ------------------------------------------------------------*/


    /*
     * Call Tcl_CreateCommand for application-specific commands, if
     * they weren't already created by the init procedures called above.
     */

    /*
     * Specify a user-specific startup file to invoke if the application
     * is run interactively.  Typically the startup file is "~/.apprc"
     * where "app" is the name of the application.  If this line is deleted
     * then no user-specific startup file will be run under any conditions.
     */

    tcl_RcFileName = "~/.wishrc";
    return TCL_OK;
}
