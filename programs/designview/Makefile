
.PHONY: all install clean

SUBDIRS = ./src

all install:
	@for i in $(SUBDIRS); \
	do      cd  $$i; \
		echo "  "; \
		echo Installing the files in $$i ; \
		$(MAKE) install; \
		cd  .. ; \
	done ;
	cp designview $(MINIBASE_ROOT)/bin

clean:
	@for i in $(SUBDIRS); \
	do      cd  $$i; \
		echo "  "; \
		echo Cleaning up $$i ; \
		$(MAKE) clean; \
		cd  .. ; \
	done ;

