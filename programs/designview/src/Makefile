
include $(MINIBASE_ROOT)/CONFIG

# Because of it's extensive use of templates 
# the designview tool needs to be compiled with gcc 2.7.2.
# Set the CC variable to your local version

# The Tcl/Tk code was compiled with Tcl version 7.4 and
# Tk version 4.0.  Set the lib variables accordingly.

TCL_TOP  = $(MINIBASE_ROOT)/packages/tcl/$(PLATFORM)

TCL_LIBS = $(TCL_TOP)/lib/libtcl7.4.a -lm
TK_LIBS  = $(TCL_TOP)/lib/libtk4.0.a $(X_LIBS) -lX11 $(SOCKLIB)
INCS = -I$(TCL_TOP)/include $(X_INCLUDES)

DEBUG_FLAG =

#======= You should not need to change anything below =======

CFLAGS = $(DEBUG_FLAG) $(INCS)

.cc.o:
	${CC} ${CFLAGS} -c $<

default: fdwish

install: fdwish
	mv fdwish ../bin/fdwish

TK_OBJS = tkMain.o tclFD.o fdset.o fd.o String.SLList.o raise_toplevel.o \
	int.SLList.o
fdwish: $(TK_OBJS)
	$(CC) -o fdwish $(TK_OBJS) $(TK_LIBS) $(TCL_LIBS)

clean:
	rm -f *.o

