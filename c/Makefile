######################################################
# File: Makefile
# Desc: Makefile for the c (C parser) directory
#       Makes and tests all object files in this directory
#
######################################################

# $Revision$
# 10 Apr 02 - Trent: created.

CC = g++ -g -I../interface -L../bin
CXX = $(CC)

all: testC

testC: ansi-c.o ansi-c.yy.o CTest.o testC.o 
	$(CC) -o $@ $^ -lcppunit

CTest.o: CTest.h

ansi-c.tab.cc: ansi-c.y ansi-c.l
	bison++ -oansi-c.tab.cc -d ansi-c.y

ansi-c.o: ansi-c.tab.cc
	${CC} -c ansi-c.tab.cc -o ansi-c.o

ansi-c.yy.cc: ansi-c.l ansi-c.tab.cc
	flex++ -oansi-c.yy.cc ansi-c.l

ansi-c.yy.o: ansi-c.yy.cc
	${CC} -c ansi-c.yy.cc -o ansi-c.yy.o

clean:
	rm -f testC *.o ansi-c.tab.cc ansi-c.yy.cc ansi-c.tab.h ansi-c.h

test: testC
	./testC

install:
	@echo Nothing to install
