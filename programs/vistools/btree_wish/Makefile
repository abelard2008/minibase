
.PHONY: all install clean

include $(MINIBASE_ROOT)/CONFIG

PKG_ROOT	= $(MINIBASE_ROOT)/packages
ARCH_OS		= $(PLATFORM)

MB_INC		= -I../../../include
TCL_INC		= -I$(PKG_ROOT)/itcl-2.0/$(ARCH_OS)/include

BLT_LIB		= -L$(PKG_ROOT)/blt/$(ARCH_OS)/lib
ITCL_LIB	= -L$(PKG_ROOT)/itcl-2.0/$(ARCH_OS)/lib
MB_LIB		= -L../../../libs
TCLX_LIB	= -L$(PKG_ROOT)/tclX/$(ARCH_OS)/lib
TCL_LIB		= -L$(PKG_ROOT)/tcl/$(ARCH_OS)/lib
TREE_LIB	= -L$(PKG_ROOT)/tree/$(ARCH_OS)/lib

INC_DIR	= $(X_INCLUDES) $(MB_INC) $(TCL_INC)
LIB_DIR	= $(MB_LIB) $(TREE_LIB) $(BLT_LIB) $(ITCL_LIB) $(TCLX_LIB) \
          $(TCL_LIB) $(X_LIBS)
LIBS = -lminibase -lTkTree -lBLT -litcl -ltclx -ltk -ltcl -lX11 $(SOCKLIB) -lm

CXX		= $(CC)
CFLAGS	= -O -g $(INC_DIR)


install: btree_wish
	mv btree_wish $(MINIBASE_ROOT)/bin/btree_wish
	rm -f btreeInit.o tkAppInit.o

all: btree_wish

btree_wish:	btreeInit.o tkAppInit.o
	$(CXX) btreeInit.o tkAppInit.o $(LIB_DIR) $(LIBS) -o btree_wish

btreeInit.o:	btreeInit.C
	$(CXX) $(CFLAGS) -c btreeInit.C -o btreeInit.o

tkAppInit.o:	tkAppInit.C
	$(CXX) $(CFLAGS) -c tkAppInit.C -o tkAppInit.o

clean:
	rm -f btreeInit.o tkAppInit.o btree_wish a.out core
