######################################################
# File: Makefile
# Desc: Makefile for boomerang
#       Makes and tests all object files all directories
#
######################################################

# $Revision$
# 20 May 02 - Trent: Created
# 31 May 02 - Mike: Make lib directory if needed

# LIBDIR isn't used at present; problem is it isn't absolute
LIBDIR=./lib
# HOST_GNU_LD is true if the linker on this host machine is GNU
HOST_GNU_LD = yes

# Choose the RUNPATH variable
BOOMDIR="$(shell pwd)/.."
LIBDIR=$(BOOMDIR)/lib
ifeq ($(HOST_GNU_LD), yes)
RUNPATH=-Wl,-rpath -Wl,$(LIBDIR)
else # Assume Solaris
RUNPATH=-R$(LIBDIR)
endif

# Save some typing
MAKE_THERE = ${MAKE} RUNPATH='$(RUNPATH)'

all: lib test

# Make the lib directory if needed
lib:
	mkdir lib

test: lib /dev/null
	cd util     && ${MAKE_THERE} test
	cd loader   && ${MAKE_THERE} RUNPATH test
	cd db       && ${MAKE_THERE} test
	cd frontend && ${MAKE_THERE} test

clean: /dev/null
	cd util     && ${MAKE} clean
	cd loader   && ${MAKE} clean
	cd db       && ${MAKE} clean
	cd frontend && ${MAKE} clean
