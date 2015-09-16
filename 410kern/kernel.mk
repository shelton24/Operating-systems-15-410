###################### LOCAL BUILD TARGETS ############################

# I consume from config.mk:
#   COMMON_OBJS
#   410_GAME_OBJS
#   KERN_GAME_OBJS

410KOBJS = $(410KDIR)/entry.o $(410_GAME_OBJS:%=$(410KDIR)/%)
ALL_410KOBJS += $(410KOBJS)

410KLIBS = $(410KERNEL_LIBS:%=$(410KDIR)/%)
410KCLEANS += $(410KDIR)/partial_kernel.o

$(410KDIR)/partial_kernel.o : $(410KOBJS)
	$(LD) -r $(LDFLAGS) -o $@ $^

########################### LIBRARY INCLUSIONS ################################
ifneq (,$(410KERNEL_LIBS))
include $(patsubst lib%.a,$(410KDIR)/%/kernel.mk,$(410KERNEL_LIBS))
endif


########################## STUDENT KERNEL BUILD ###############################

STUKOBJS = $(COMMON_OBJS:%=$(STUKDIR)/%) $(KERN_GAME_OBJS:%=$(STUKDIR)/%)
ALL_STUKOBJS += $(STUKOBJS)

STUKCLEANS += $(STUKDIR)/partial_kernel.o

ifeq ($(STUKOBJS),)
$(STUKDIR)/partial_kernel.o :
	touch $@
else
$(STUKDIR)/partial_kernel.o : $(STUKOBJS)
	$(LD) -r $(LDFLAGS) -o $@ $^
endif
