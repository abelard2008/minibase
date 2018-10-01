/* raise_toplevel.c */

#include <stdio.h>
#include <tcl.h>
#include <tk.h>

/*
 * RaiseTopLevelCmd --
 *	The implementation of the "raise_toplevel" Tcl command. This brings
 *      a toplevel window to the top of the stacking order by calling an
 *	X library routine directly. It seems as though tk has to do a lot
 *	of re-drawing (which is very slow) if you allow it bring the window
 *      forward on its own with its raise command.
 */
int
RaiseTopLevelCmd(
    ClientData /*clientData*/,
    Tcl_Interp *interp,
    int argc,
    char *argv[])
{
    int result;
    Tk_Window tkwin;
    Display *d;
    Window w;
    
    if (argc != 2) {
	interp->result = "Usage: raise_toplevel win";
	return TCL_ERROR;
    }
    
    tkwin = Tk_NameToWindow(interp, argv[1], Tk_MainWindow(interp));
    if (tkwin == NULL) {
	return TCL_ERROR;
    }
    
    if (!Tk_IsTopLevel(tkwin)) {
	interp->result = "raise_toplevel error: Not a toplevel window";
	return TCL_ERROR;
    }    
    
    d = Tk_Display(tkwin);
    w = Tk_WindowId(tkwin);
    if (!d || !w) {
	interp->result = "raise_toplevel error: Could not identify X display";
	return TCL_ERROR;
    }
    
    result = XRaiseWindow(d, w);
    sprintf(interp->result, "%d", result);
    return TCL_OK;
}
