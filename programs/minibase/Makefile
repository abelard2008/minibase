MB_ROOT = ../..
include $(MB_ROOT)/makefiles/common.make

INCLUDES = -I$(MB_ROOT)/include
LIBS = -L$(MB_ROOT)/libs -lminibase $(EXTERNAL_LIBS)


MAIN_OBJ = main.o
BIN_INSTALL_DIR = $(MB_ROOT)/bin
LIBRARY = $(MB_ROOT)/libs/libminibase.a


.PHONY: all install clean

all: minibase

install: all
	@if [ ! -d $(BIN_INSTALL_DIR) ]; then mkdir $(BIN_INSTALL_DIR); fi
	/bin/cp minibase $(BIN_INSTALL_DIR)

.C.o:
	$(CC) $(CFLAGS) -c $< -o $@


minibase: $(MAIN_OBJ) $(LIBRARY)
	$(CC) $(CFLAGS) -o minibase $(MAIN_OBJ) $(LIBS)

# Set up for our local site only (only built when specified explicitly)	
minibase.pure: $(MAIN_OBJ) $(LIBRARY)
	/s/purify/bin/purify -best-effort -collector=/s/gcc/lib/gcc-lib/sparc-sun-sunos4.1.3/2.6.3/ld -g++=yes $(CC) $(CFLAGS) -o minibase.pure $(MAIN_OBJ) $(LIBS)


depend:
	$(CC) $(CFLAGS) -M main.C > Makefile.dependencies

clean: 
	/bin/rm -rf minibase minibase.pure Makefile.dependencies BTREEDRIVER LHDRIVER TEST tmp1 minibase-databases core temporary *~ *.o


-include Makefile.dependencies
