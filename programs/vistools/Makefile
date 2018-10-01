
.PHONY: all install clean

SUBDIRS = btree_wish bmtool bttool

all install:
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) all); done;
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) install); done;

clean:
	-@for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean); done;

