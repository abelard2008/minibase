# Makefile for B+ tree visualization exercises.
# You use this by preparing your C++ file, then running the command:
#
#   make NAME=your-file-name
#
# (where your C++ file is called "your-file-name.C").

#ROOT=/u/l/u/luke/public/cs564
ROOT=/u/w/h/whuang/minibase

include $(ROOT)/makefiles/common.make

all: $(NAME)
	@if [ -z "$(NAME)" ]; then \
		/bin/echo 'Type "'$(MAKE)' NAME=your-file-name" to compile your-file-name.C' ; \
		exit 1; \
	fi

$(NAME): $(NAME).C $(ROOT)/library/btree/src/btfile.C
	$(CC) $(CFLAGS) $(NAME).C -g -o $(NAME) -I$(ROOT)/include -L$(ROOT)/libs -lminibase -lm
