
.PHONY: setup all clean install

SUBDIRS =  minibase

all install:
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) all); done;
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) install); done;

setup:

clean:
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean); done;
