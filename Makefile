.POSIX:    # Parse it an run in POSIX conforming mode
.SUFFIXES: # Delete the default suffixes (inference rules)

CC=gcc
CFLAGS=-g -Wall -I$(IDIR)
ROOTDIR=.
IDIR=$(ROOTDIR)/include
SDIR=$(ROOTDIR)/src
ODIR=$(ROOTDIR)/obj

_DEPS=tcp.h
DEPS=$(addprefix $(IDIR)/,$(_DEPS))

_OBJS_P=tcp.o publisher.o
OBJS_P=$(addprefix $(ODIR)/,$(_OBJS_P))

_OBJS_S=tcp.o subscriber.o
OBJS_S=$(addprefix $(ODIR)/,$(_OBJS_S))

all: publisher subscriber

publisher: $(OBJS_P)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJS_P): | $(ODIR)

subscriber: $(OBJS_S)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJS_S): | $(ODIR)

$(ODIR):
	mkdir $(ODIR)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean 
clean:
	rm -rf $(ODIR) publisher subscriber

