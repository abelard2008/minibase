GLOBAL_DB_DIR = ..
include ../Makefile.preamble
include Makefile.preamble


.PHONY: all depend clean install $(ALL_SUBDIRS)

ALL_SUBDIRS = optimizer_subdir \
	parserinterface_subdir utils_subdir temporary_subdir


SUBDIR_MAKE = $(MAKE) $(SUBDIR_TARGET) OBJ_DIR=../$(OBJ_DIR)
SUBDIR_TARGET =

all: $(ALL_SUBDIRS)

%_subdir:
	@cd $(@:%_subdir=%); $(SUBDIR_MAKE)


install: $(addprefix $(HDR_INSTALL_DIR)/,$(OPTIMIZER_HDRS))
	@$(MAKE) all SUBDIR_TARGET=install

vpath %.h include
$(HDR_INSTALL_DIR)/%: %
	$(INSTALLHDRS) $< $(HDR_INSTALL_DIR)

depend: 
	@$(MAKE) all SUBDIR_TARGET=depend
	
clean: 
	@$(MAKE) all SUBDIR_TARGET=clean
	/bin/rm -rf *~ *.bak $(OBJ_DIR)


