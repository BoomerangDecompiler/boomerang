######################################################
# File: Makefile
# Desc: Makefile for the db (database) directory
#       Makes and tests all object files in this directory
#
######################################################

# $Revision$
# 05 Apr 02 - Mike: Created. Added "test" target

CC = g++ -g -I../interface
CXX = $(CC)

all: testExp

testExp: testExp.o exp.o ExpTest.o ../bin/libcppunit.so
	$(CC) -o $@ -Wl,-R../bin $^
# The -Wl, above passes -R../bin to the linker. This finds the
# libraries without having to set LD_LIBRARY_PATH

ExpTest.o: ExpTest.h

exp.o: ../interface/exp.h

clean:
	rm testExp *.o

test: testExp
	./testExp

install:
	@echo Nothing to install
