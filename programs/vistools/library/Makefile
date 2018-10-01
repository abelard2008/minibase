# Generated automatically from Makefile.in by configure.
# ------------------------------------------------------------------------
# 	Makefile for library files and directories of TREE library
# ------------------------------------------------------------------------

INSTALL      = /bin/install -c
INSTALL_DATA = ${INSTALL} -m 644
TREE_LIBRARY  = @TREE_LIBRARY@
RM           = rm -f
SHELL        = /bin/sh
FILES        = ButtonFrame.tcl Canvas.tcl CanvasPrint.tcl CheckEntry.tcl Choice.tcl \
	       Dialog.tcl Entry.tcl Frame.tcl InputDialog.tcl LabelMessage.tcl Listbox.tcl \
	       ListboxPrint.tcl PrintDialog.tcl Toplevel.tcl Tree.tcl compat.tcl defaults.tcl \
	       list.tcl tclutil.tcl tkutil.tcl udialog.tcl tclIndex
srcdir	     = .

all:

install-top:
	@if test -d $(TREE_LIBRARY) ; then \
	    : ; \
	else \
	    mkdir $(TREE_LIBRARY) ; \
	fi

install-files: install-top
	@set -x; for i in $(srcdir)/$(FILES) ; do \
	    $(INSTALL_DATA) $$i $(TREE_LIBRARY) ; \
	done 

install: install-files

clean:
	$(RM) *\~ "#"*

