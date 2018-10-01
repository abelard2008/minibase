
#SUBDIRS = library programs bin
SUBDIRS = library programs bin

all install:
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) install); done;

depend:
	cd library; $(MAKE) depend
	cd programs/minibase; $(MAKE) depend

purify:
	cd library; $(MAKE) install
	cd programs/minibase; $(MAKE) minibase.pure

clean:
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean); done;

.PHONY: all install depend purify clean

