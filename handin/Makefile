IDIR       = src/include
ODIR       = src/obj
SRCDIR     = src
TESTDIR    = src/test
TESTBINDIR = test/bin

CC 		= gcc
#CFLAGS  = -I$(IDIR) -Wall -Werror
CFLAGS  = -I$(IDIR) -Wall -DTESTING
LDFLAGS	= 

_HEADERS = proxy.h logger.h run_proxy.h util.h client.h server.h mydns.h \
		   nameserver.h node_map.h
HEADERS = $(patsubst %,$(IDIR)/%,$(_HEADERS))

_OBJS = proxy.o logger.o run_proxy.o util.o client.o server.o mydns.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

_NS_OBJS = nameserver.o logger.o node_map.o
NS_OBJS = $(patsubst %,$(ODIR)/%,$(_NS_OBJS))

_TEST_OBJS = 
TEST_OBJS = $(patsubst %,$(ODIR)/%,$(_TEST_OBJS))

# Binaries
BINS = proxy nameserver

TESTBINS = 

# Explit build and testing targets
all: ${BINS}

test: ${TESTBINS}

proxy: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

nameserver: $(NS_OBJS)
	$(CC) $(CFLAGS) $(NS_OBJS) -o $@ $(LDFLAGS)

$(ODIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(TESTDIR)/%.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	@rm -f $(BINS) $(TESTBIN) \
		$(ODIR)/* $(SRCDIR)/*~ $(IDIR)/*~ $(SRCDIR)/*.orig $(IDIR)/*.orig  $(TESTDIR)/*.~ $(TESTDIR)/*.orig \
		log/*

