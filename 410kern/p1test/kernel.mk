################### P1 SPECIFIC BUILD TARGETS ########################
# This is similar to 410kern/kernel.mk and specifies how to build the
# project 1 test program

# I consume from config.mk:
#   COMMON_OBJS
#   410TEST_OBJS

410P1TEST_OBJS = $(410KDIR)/entry.o $(410TEST_OBJS:%=$(410KDIR)/p1test/%)

ALL_410KOBJS += $(410POBJS)
410KCLEANS += $(410KDIR)/partial_tester.o

$(410KDIR)/partial_tester.o : $(410P1TEST_OBJS)
	$(LD) -r $(LDFLAGS) -o $@ $^

STUP1TEST_OBJS = $(COMMON_OBJS:%=$(STUKDIR)/%) $(TESTER_OBJS:%=$(STUKDIR)/%)
ALL_STUKOBJS += $(STUP1TEST_OBJS)
STUKCLEANS += $(STUKDIR)/partial_tester.o

ifeq ($(STUP1TEST_OBJS),)
$(STUKDIR)/partial_tester.o :
	touch $@
else
$(STUKDIR)/partial_tester.o : $(STUP1TEST_OBJS)
	$(LD) -r $(LDFLAGS) -o $@ $^
endif
