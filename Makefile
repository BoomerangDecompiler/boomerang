######################################################
# File: Makefile
# Desc: Makefile for boomerang
#       Makes and tests all object files all directories
#
######################################################

# $Revision$
# 20 May 02 - Trent: Created
# 31 May 02 - Mike: Make lib directory if needed

.PHONY: test clean

all: lib test

# Make the lib directory if needed
lib:
	mkdir lib

test: lib
	cd util     && ${MAKE} test
	cd loader   && ${MAKE} test
	cd db       && ${MAKE} test
	cd frontend && ${MAKE} test

clean:
	cd util     && ${MAKE} clean
	cd loader   && ${MAKE} clean
	cd db       && ${MAKE} clean
	cd frontend && ${MAKE} clean
