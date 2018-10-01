GLOBAL_DB_DIR := .
include Makefile.preamble

include $(GLOBALDEFS_DIR)/Makefile.preamble
include $(BTREE_DIR)/Makefile.preamble
include $(BUFMGR_DIR)/Makefile.preamble
include $(CATALOG_DIR)/Makefile.preamble
include $(INTERPRETER_DIR)/Makefile.preamble
include $(JOINS_DIR)/Makefile.preamble
include $(OPTIMIZER_DIR)/Makefile.preamble
include $(SPACE_DIR)/Makefile.preamble
include $(TESTS_DIR)/Makefile.preamble

vpath %.o \
$(GLOBALDEFS_OBJ_DIR):\
$(BTREE_OBJ_DIR):\
$(BUFMGR_OBJ_DIR):\
$(CATALOG_OBJ_DIR):\
$(INTERPRETER_OBJ_DIR):\
$(JOINS_OBJ_DIR):\
$(OPTIMIZER_OBJ_DIR):\
$(SPACE_OBJ_DIR):\
$(TESTS_OBJ_DIR)

OBJS := \
	$(GLOBALDEFS_OBJS) \
	$(BTREE_OBJS) \
	$(BUFMGR_OBJS) \
	$(CATALOG_OBJS) \
	$(INTERPRETER_OBJS) \
	$(JOINS_OBJS) \
	$(OPTIMIZER_OBJS) \
	$(SPACE_OBJS) \
	$(TESTS_OBJS)

ALL_SUBDIRS := \
	globaldefs_subdir \
	btree_subdir \
	bufmgr_subdir \
	catalog_subdir \
	interpreter_subdir \
	joins_subdir \
	optimizer_subdir \
	spacemgr_subdir \
	tests_subdir

# The reason for this two-pass approach is that GNU make
# gets confused by VPATH if all .o files do not already
# exist when we go to make libminibase.a.
all: all_subdirs
	@$(MAKE) libminibase.a

.PHONY: all install depend clean all_subdirs install_headers distclean



SUBDIR_TARGET :=

all_subdirs: $(ALL_SUBDIRS)


%_subdir:
	@cd $(@:%_subdir=%); $(MAKE) $(SUBDIR_TARGET)



libminibase.a: $(OBJS)
	ar rc $@ $?
	ranlib $@


INSTALLED_LIB := $(LIB_INSTALL_DIR)/libminibase.a

install: all
	@$(MAKE) install_headers $(LIB_INSTALL_DIR) $(INSTALLED_LIB)

install_headers: $(HDR_INSTALL_DIR)
	@$(MAKE) all_subdirs SUBDIR_TARGET=install

$(HDR_INSTALL_DIR) $(LIB_INSTALL_DIR):
	mkdir $@

$(INSTALLED_LIB): libminibase.a
	/bin/rm -f $@
	/bin/ln -s ../library/libminibase.a $@


debug:
	@$(MAKE) all_subdirs SUBDIR_TARGET=debug


depend:
	@$(MAKE) all_subdirs SUBDIR_TARGET=depend

distclean: clean
	/bin/rm -rf $(HDR_INSTALL_DIR)
	/bin/rm -rf $(LIB_INSTALL_DIR)

clean:
	@$(MAKE) all_subdirs SUBDIR_TARGET=clean
	/bin/rm -rf libminibase.a *~
