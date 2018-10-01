# General include file for subsidiary makefiles.  Contains
# general-purpose commands and targets.  Expects SUBDIR_PREFIX to be
# set; alternatively, you may set SRCS, HDRS and OBJS yourself after
# including this.


SRCS = $($(SUBDIR_PREFIX)_SRCS)
HDRS = $($(SUBDIR_PREFIX)_HDRS)
OBJS = $($(SUBDIR_PREFIX)_OBJS)

$(SUBDIR_PREFIX)_DIR := .

all: $(OBJ_DIR) $(OBJS)

.PHONY: all debug depend clean install

SRC_DIR = src
HDR_DIR = include
vpath %.o $(OBJ_DIR)
vpath %.C $(SRC_DIR)
vpath %.h $(HDR_DIR)


$(OBJ_DIR)/%.o %.o: %.C
	$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/${@F}

$(OBJ_DIR)/%.o %.o: %.c
	$(C) $(CFLAGS) -c $< -o $(OBJ_DIR)/${@F}

$(OBJ_DIR)/%.c %.c: %.y
	$(YACC) $<

$(OBJ_DIR)/%.c %.c: %.l
	$(LEX) $<

$(OBJ_DIR):
	mkdir $@

debug:
	@$(MAKE) all "CFLAGS=$(CFLAGS) -DDEBUG"

depend: $(SRCS)
	@if [ -n "$?" ]; then \
		cmd="$(CC) $(CFLAGS) -M $? > Makefile.dependencies"; \
		echo $$cmd; eval $$cmd; \
	fi



install: $(HDRS) $(addprefix $(HDR_INSTALL_DIR)/,$(HDRS))

$(HDR_INSTALL_DIR)/%: %
	-@chmod -f +w $@
	$(INSTALLHDRS) $< $(HDR_INSTALL_DIR)


GARBAGE := *~ .\#* $(OBJ_DIR) *.bak Makefile.dependencies
SUBDIR_GARBAGE := *~ .\#*

clean::
	/bin/rm -rf $(GARBAGE) $(OTHERGARBAGE)
	-cd $(SRC_DIR); /bin/rm -rf $(SUBDIR_GARBAGE) $(OTHERGARBAGE)
	-cd $(HDR_DIR); /bin/rm -rf $(SUBDIR_GARBAGE) $(OTHERGARBAGE)

-include Makefile.dependencies
