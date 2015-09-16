
##################################################################
#                                                                #
#          Do not edit this file.  Edit config.mk instead.       #
#         Do not edit this file.  Edit config.mk instead.        #
#        Do not edit this file.  Edit config.mk instead.         #
#       Do not edit this file.  Edit config.mk instead.          #
#      Do not edit this file.  Edit config.mk instead.           #
#                                                                #
#    #####        #######       #######       ######       ###   #
#   #     #          #          #     #       #     #      ###   #
#   #                #          #     #       #     #      ###   #
#    #####           #          #     #       ######        #    #
#         #          #          #     #       #                  #
#   #     #          #          #     #       #            ###   #
#    #####           #          #######       #            ###   #
#                                                                #
#                                                                #
#      Do not edit this file.  Edit config.mk instead.           #
#       Do not edit this file.  Edit config.mk instead.          #
#        Do not edit this file.  Edit config.mk instead.         #
#         Do not edit this file.  Edit config.mk instead.        #
#          Do not edit this file.  Edit config.mk instead.       #
#                                                                #
##################################################################

#################### RULE HEADER ####################

.PHONY: update query_update print html_doc
all: query_update bootfd.img

############## VARIABLE DEFINITIONS #################

DOC = doxygen
MAKE = gmake

# If you are on an Andrew machine, these are in 410/bin,
# which should be on your PATH.
# If not, you'll need to arrange for programs with these
# names which work the same way to be on your PATH.  Note
# that different versions of gcc generally do NOT work the
# same way.
CC = 410-gcc
LD = 410-ld
AR = 410-ar
OBJCOPY = 410-objcopy

PROJROOT = $(PWD)
# All paths relative to PROJROOT unless otherwise noted
410KDIR = 410kern
410SDIR = spec
410UDIR = 410user
BOOTDIR = boot
STUKDIR = kern
STUUDIR = user
EXTRADIR ?= 410extra
BUILDDIR = temp

# These need to be known before we run includes, sadly
410ULIBDIR = $(410UDIR)/lib
STUULIBDIR = $(STUUDIR)/lib

# Relative to {410,STU}UDIR
UPROGDIR = progs

# By default, we are offline
UPDATE_METHOD = offline

# Suffix for dependency files
DEP_SUFFIX = dep

###########################################################################
# Libraries!
# These may be overwridden by {,$(STUKDIR)/,$(STUUDIR)/}config.mk
###########################################################################
410KERNEL_LIBS = \
				 libboot.a \
				 liblmm.a \
				 libmalloc.a \
				 libmisc.a \
				 libRNG.a \
				 libsimics.a \
				 libstdio.a \
				 libstdlib.a \
				 libstring.a \
				 libx86.a \

## Can't have thrgrp be a default as we don't ship with the requisite files
## in user/ like we did with P2.
410USER_LIBS_EARLY = # libthrgrp.a
410USER_LIBS_LATE = libRNG.a libsimics.a libstdio.a libstdlib.a libstring.a # libmalloc.a
STUDENT_LIBS_EARLY = libthread.a
STUDENT_LIBS_LATE = libsyscall.a

###########################################################################
# Details of library structure (you do NOT have to read this part!!)
###########################################################################
#  _EARLY means "specified early in the compiler command line" and so
#               can be taken to mean "can depend on all _LATE files"
#  _LATE means "specified late in the compiler command line" and so
#              might be taken as "there be daemons here"; in particular
#              these libraries may not depend on _EARLY libraries
#              or any library in the list before them.
#
# The full command line order is
#   410USER_LIBS_EARLY -- can depend on everything
#   STUDENT_LIBS_EARLY -- can depend only on _LATE libs
#   410USER_LIBS_LATE -- must not depend on _EARLY libs
#   STUDENT_LIBS_LATE -- must not depend on any other lib group

################# DETERMINATION OF INCLUSION BEHAVIORS #####################
ifeq (0,$(words $(filter %clean,$(MAKECMDGOALS))))
ifeq (0,$(words $(filter print,$(MAKECMDGOALS))))
ifeq (0,$(words $(filter sample,$(MAKECMDGOALS))))
ifeq (0,$(words $(filter %update,$(MAKECMDGOALS))))
DO_INCLUDE_DEPS=1
endif
endif
endif
endif


################ CONFIGURATION INCLUSION #################

-include config.mk

# Infrastructure configuration files
# These may look like fun, but they are NOT FOR STUDENT USE without
# instructor permisison.  Almost anything you want to do can be done
# via the top-level config.mk.
-include $(STUKDIR)/config.mk
-include $(STUUDIR)/config.mk
-include $(EXTRADIR)/extra-early.mk

############### TARGET VARIABLES ###############

# FIXME 410ULIBS_LATE is a temporary gross hack; fix later

PROGS = $(410REQPROGS) $(410REQBINPROGS) \
        $(STUDENTREQPROGS) $(410TESTS) $(STUDENTTESTS) \
        $(EXTRA_TESTS) $(EXTRA_410_TESTS)

410ULIBS_EARLY = $(patsubst %,$(410ULIBDIR)/%,$(410USER_LIBS_EARLY))
410ULIBS_LATE = $(patsubst %,$(410ULIBDIR)/%,$(410USER_LIBS_LATE)) \
	$(patsubst %,$(410ULIBDIR)/%,$(410USER_LIBS_LATE))
STUULIBS_EARLY = $(patsubst %,$(STUULIBDIR)/%,$(STUDENT_LIBS_EARLY))
STUULIBS_LATE = $(patsubst %,$(STUULIBDIR)/%,$(STUDENT_LIBS_LATE))

################# MISCELLANEOUS VARAIBLES ##############

CRT0 = $(410UDIR)/$(UPROGDIR)/crt0.o

LDFLAGSENTRY = --entry=_main

########### MAKEFILE FILE INCLUSION ##############

# These are optional so that we can use the same Makefile for P1,2,3
# Again, these files are NOT FOR STUDENT USE, and unlike the above,
# we probably won't give you permission to overwrite these. :)

# Boot loader P4
-include $(BOOTDIR)/boot.mk

# Include 410kern control, responsible for 410kern libraries, objects, interim
-include $(410KDIR)/kernel.mk
# Include 410user control, responsible for 410user libraries and programs
-include $(410UDIR)/user.mk
# Infrastructure
-include $(EXTRADIR)/extra-late.mk

# Top level targets
-include $(410KDIR)/toplevel.mk


############# BUILD PARAMETER VARIABLES ################
# Note that this section DEPENDS UPON THE INCLUDES ABOVE

# When generating includes, we do so in the order we do so that it is always
# possible to match <lib/header.h> from 410kern/lib exactly, or from kern/lib
# if so desired, but that files without directory prefixes will match
# 410kern/inc, kern/, kern/inc, then the 410kern/lib/* entries according to
# 410KERNEL_LIBS, which is under external control.

KCFLAGS = -nostdinc \
	-fno-strict-aliasing -fno-builtin -fno-stack-protector -fno-omit-frame-pointer \
	-fno-aggressive-loop-optimizations \
	-Wall -gdwarf-2 -Werror -O1 -m32
KLDFLAGS = -static -Ttext 100000 --fatal-warnings -melf_i386
KINCLUDES = -I$(410KDIR) -I$(410KDIR)/inc \
			-I$(410SDIR) \
			-I$(STUKDIR) -I$(STUKDIR)/inc \
			$(patsubst lib%.a,-I$(410KDIR)/%,$(410KERNEL_LIBS))

UCFLAGS = -nostdinc \
	-fno-strict-aliasing -fno-builtin -fno-stack-protector -fno-omit-frame-pointer \
	-fno-aggressive-loop-optimizations \
	-Wall -gdwarf-2 -Werror -O1 -m32
ULDFLAGS = -static -Ttext 1000000 --fatal-warnings -melf_i386 $(LDFLAGSENTRY)
UINCLUDES = -I$(410SDIR) -I$(410UDIR)/inc -I$(410ULIBDIR)/inc \
						-I$(STUUDIR)/inc -I$(STUULIBDIR)/inc


TMP_410KOBJS_NOT_DOTO = $(filter-out %.o,$(ALL_410KOBJS))
ifneq (,$(TMP_410KOBJS_NOT_DOTO))
$(error "Feeling nervous about '$(TMP_410KOBJS_NOT_DOTO)'")
endif
ALL_410KOBJS_DEPS = $(ALL_410KOBJS:%.o=%.$(DEP_SUFFIX))

TMP_410UOBJS_NOT_DOTO = $(filter-out %.o,$(ALL_410UOBJS))
ifneq (,$(TMP_410UOBJS_NOT_DOTO))
$(error "Feeling nervous about '$(TMP_410UOBJS_NOT_DOTO)'")
endif
ALL_410UOBJS_DEPS = $(ALL_410UOBJS:%.o=%.$(DEP_SUFFIX))

TMP_STUKOBJS_NOT_DOTO = $(filter-out %.o,$(ALL_STUKOBJS))
ifneq (,$(TMP_STUKOBJS_NOT_DOTO))
$(error "Feeling nervous about '$(TMP_STUKOBJS_NOT_DOTO)'")
endif
ALL_STUKOBJS_DEPS = $(ALL_STUKOBJS:%.o=%.$(DEP_SUFFIX))

TMP_STUUOBJS_NOT_DOTO = $(filter-out %.o,$(ALL_STUUOBJS))
ifneq (,$(TMP_STUUOBJS_NOT_DOTO))
$(error "Feeling nervous about '$(TMP_STUUOBJS_NOT_DOTO)'")
endif
ALL_STUUOBJS_DEPS = $(ALL_STUUOBJS:%.o=%.$(DEP_SUFFIX))

STUUPROGS_DEPS = $(STUUPROGS:%=%.dep)
410UPROGS_DEPS = $(410UPROGS:%=%.dep)

############## UPDATE RULES #####################

update:
	./update.sh $(UPDATE_METHOD)

query_update:
	./update.sh $(UPDATE_METHOD) query

################### GENERIC RULES ######################
%.$(DEP_SUFFIX): %.S
	$(CC) $(CFLAGS) -DASSEMBLER $(INCLUDES) -M -MP -MF $@ -MT $(<:.S=.o) $<

%.$(DEP_SUFFIX): %.s
	@echo "You should use the .S file extension rather than .s"
	@echo ".s does not support precompiler directives (like #include)"
	@false

%.$(DEP_SUFFIX): %.c
	$(CC) $(CFLAGS) $(INCLUDES) -M -MP -MF $@ -MT $(<:.c=.o) $<

%.o: %.S
	$(CC) $(CFLAGS) -DASSEMBLER $(INCLUDES) -c -o $@ $<
	$(OBJCOPY) -R .comment -R .note $@ $@

%.o: %.s
	@echo "You should use the .S file extension rather than .s"
	@echo ".s does not support precompiler directives (like #include)"
	@false

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
	$(OBJCOPY) -R .comment -R .note $@ $@

%.a:
	rm -f $@
	$(AR) rc $@ $^

################ PATTERNED VARIABLE ASSIGNMENTS ##################
$(410KDIR)/%: CFLAGS=$(KCFLAGS)
$(410KDIR)/%: INCLUDES=$(KINCLUDES)
$(410KDIR)/%: LDFLAGS=$(KLDFLAGS)
$(STUKDIR)/%: CFLAGS=$(KCFLAGS)
$(STUKDIR)/%: INCLUDES=$(KINCLUDES)
$(STUKDIR)/%: LDFLAGS=$(KLDFLAGS)
$(410UDIR)/%: CFLAGS=$(UCFLAGS)
$(410UDIR)/%: INCLUDES=$(UINCLUDES)
$(410UDIR)/%: LDFLAGS=$(ULDFLAGS)
$(STUUDIR)/%: CFLAGS=$(UCFLAGS)
$(STUUDIR)/%: INCLUDES=$(UINCLUDES)
$(STUUDIR)/%: LDFLAGS=$(ULDFLAGS)

################# USERLAND PROGRAM UNIFICATION RULES #################

$(410REQPROGS:%=$(BUILDDIR)/%): \
$(BUILDDIR)/%: $(410UDIR)/$(UPROGDIR)/%
	mkdir -p $(BUILDDIR)
	cp $< $@

$(410REQBINPROGS:%=$(BUILDDIR)/%): \
$(BUILDDIR)/%: $(410UDIR)/$(UPROGDIR)/% 
	mkdir -p $(BUILDDIR)
	cp $< $@

$(410TESTS:%=$(BUILDDIR)/%) : \
$(BUILDDIR)/% : $(410UDIR)/$(UPROGDIR)/%
	mkdir -p $(BUILDDIR)
	cp $< $@

$(EXTRA_410_TESTS:%=$(BUILDDIR)/%) : \
$(BUILDDIR)/% : $(410UDIR)/$(UPROGDIR)/%
	mkdir -p $(BUILDDIR)
	cp $< $@

$(STUDENTREQPROGS:%=$(BUILDDIR)/%) : \
$(BUILDDIR)/% : $(STUUDIR)/$(UPROGDIR)/%
	mkdir -p $(BUILDDIR)
	cp $< $@

$(STUDENTTESTS:%=$(BUILDDIR)/%) : \
$(BUILDDIR)/% : $(STUUDIR)/$(UPROGDIR)/%
	mkdir -p $(BUILDDIR)
	cp $< $@

############## MISCELLANEOUS TARGETS #################

html_doc:
	$(DOC) doxygen.conf

PRINTOUT=kernel.ps

print:
	enscript -2rG -fCourier7 -FCourier-Bold10 -p $(PRINTOUT) \
		$(if $(strip $(TABSTOP)),-T $(TABSTOP),) \
			README.dox \
			`find ./kern/ -type f -regex  '.*\.[chS]' | sort`

################# CLEANING RULES #####################
.PHONY: clean veryclean

clean:
	rm -f $(CRT0)
	rm -f $(FINALCLEANS)
	rm -f $(ALL_410KOBJS)
	rm -f $(ALL_410KOBJS_DEPS)
	rm -f $(410KCLEANS)
	rm -f $(ALL_410UOBJS)
	rm -f $(ALL_410UOBJS_DEPS)
	rm -f $(410UCLEANS)
	rm -f $(410UPROGS:%=%.o)
	rm -f $(410UPROGS_DEPS)
	rm -f $(ALL_STUKOBJS)
	rm -f $(ALL_STUKOBJS_DEPS)
	rm -f $(STUKCLEANS)
	rm -f $(ALL_STUUOBJS)
	rm -f $(ALL_STUUOBJS_DEPS)
	rm -f $(STUUCLEANS)
	rm -f $(STUUPROGS:%=%.o)
	rm -f $(STUUPROGS_DEPS)
	rm -f $(BOOT_CLEANS)

veryclean: clean
	rm -rf doc $(PRINTOUT) bootfd.img kernel kernel.log $(BUILDDIR)
	rm -f $(FINALVERYCLEANS)

%clean:
	$(error "Unknown cleaning target")

########### DEPENDENCY FILE INCLUSION ############
ifeq (1,$(DO_INCLUDE_DEPS))
ifneq (,$(ALL_410KOBJS))
-include $(ALL_410KOBJS_DEPS)
endif
ifneq (,$(ALL_STUKOBJS))
-include $(ALL_STUKOBJS_DEPS)
endif
ifneq (,$(ALL_410UOBJS))
-include $(ALL_410UOBJS_DEPS)
endif
ifneq (,$(ALL_STUUOBJS))
-include $(ALL_STUUOBJS_DEPS)
endif
ifneq (,$(410UPROGS_DEPS))
-include $(410UPROGS_DEPS)
endif
ifneq (,$(STUUPROGS_DEPS))
-include $(STUUPROGS_DEPS)
endif
endif

########### MANDATE CLEANING AS ONLY TARGETS ############
ifneq (0,$(words $(filter %clean,$(MAKECMDGOALS))))
  # The target includes a make target
  ifneq (0, $(words $(filter-out %clean,$(MAKECMDGOALS))))
    # There is another target on the list
    $(error "Clean targets must run by themselves")
  endif
endif
