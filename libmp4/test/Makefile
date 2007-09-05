
TOP_PATH=..

MP4_TOOL_PATH=$(TOP_PATH)/test

LIB_INC=$(TOP_PATH)/include

CFLAGS = -I$(LIB_INC) -O2 -Wall

DEBUGBUILD=yes
DMALLOC=no

ifeq ($(DEBUGBUILD), yes)
CFLAGS+=-g
endif

ifeq ($(GPROFBUILD), yes)
CFLAGS+=-pg
endif

ifeq ($(DMALLOC), yes)
CFLAGS+=-DDMALLOC 
LDFLAGS += -ldmallocth
endif


CC = gcc

PROG_SRCS = mp4_tool.c 
PROG_OBJS := $(patsubst %.c,%.o,$(PROG_SRCS))

PROG=mp4_tool

all: $(PROG)

$(PROG): $(PROG_OBJS) $(TOP_PATH)/lib/libmp4_static.a
	gcc $(CFLAGS) $(LDFLAGS) -o $@ $^

depend:
	$(CC) -MM $(CFLAGS) $(LIB_SRCS) $(PROG_SRCS) 1>.depend

clean: 
	rm -f $(PROG_OBJS) $(LIB) $(PROG)

distclean: clean
	rm -f Makefile.bak .depend

indent:
	find $(LIB_PATH) -name "*.[ch]" |xargs indent -npro -kr -i4 -ts4 -sob -l80 -ss-ncs
	find $(LIB_PATH) -name "*~" -print | xargs rm -rf



# include dependency files if they exist
#
ifneq ($(wildcard .depend),)
include .depend
endif
