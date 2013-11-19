IDIR       = src/include
ODIR       = src/obj
SRCDIR     = src
TESTDIR    = src/test
TESTBINDIR = test/bin

CC 		= gcc
CFLAGS  = -I$(IDIR) -Wall -Werror
LDFLAGS	= 

_HEADERS = proxy.h logger.h run_proxy.h util.h client.h server.h
HEADERS = $(patsubst %,$(IDIR)/%,$(_HEADERS))

_OBJS = proxy.o logger.o run_proxy.o util.o client.o server.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

_TEST_OBJS   = 
TEST_OBJS = $(patsubst %,$(ODIR)/%,$(_TEST_OBJS))

# Binaries
BINS = proxy

TESTBINS = 

# Explit build and testing targets
all: ${BINS}

test: ${TESTBINS}

proxy: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(ODIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(TESTDIR)/%.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

.PHONY: clean
clean:
	@rm -f $(BINS) $(TESTBIN) \
		$(ODIR)/* $(SRCDIR)/*~ $(IDIR)/*~ $(SRCDIR)/*.orig $(IDIR)/*.orig  $(TESTDIR)/*.~ $(TESTDIR)/*.orig \
		log/*

