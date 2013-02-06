######################################################
# File: Makefile.in
# Desc: Makefile for boomerang
#       Makes and tests all object files all directories
#
######################################################

# $Revision$
# 16 Mar 08 - Markus Gothe: Makefile.am for use with autotools
# 22 Aug 06 - Mike Frysinger: respect values set up by ./configure script
# 12 Apr 06 - Gerard: Restructured, added conditional rules, added generated dependencies
# 13 Mar 06 - Mike: Prevent going to the loader directory 5 or 6 times
# 05 Jul 05 - Mike: Big merge with boomerang.pole branch
# 03 Feb 05 - Mike: Removed analysis/ related files
# 23 Nov 04 - Mike: Added PPC files
# 20 Jul 04 - Mike: Removed BOOMDIR
# 21 Aug 02 - Icer: added codegen/chllcode.cpp to the build process by adding it in STATIC_OBJ
# 20 Aug 02 - Icer: removed test from all, test code is broken. Added more to distclean
# 19 Aug 02 - Icer: changed make distclean delete toplevel makefile; changed all: to read lib static test
# 31 May 02 - Mike: Make lib directory if needed
# 20 May 02 - Trent: Created
# 15 Jul 01 - Mike: Added static target

###############
# Introduction
#
# This Makefile controls the whole build process of the Boomerang decompiler, except for the loaders.
# The loaders have their own makefile (loaders/Makefile).
# The gui's also have to take care of themselves.
#
# Index of this makefile: 
#
# - Variables detected by ./configure
# - Detection of OS specific settings
# - Compiler settings
# - Main rules
# - Loaders
# - Detection of machine types and loaders
# - Lots of object files
# - Conditional rules
# - Rules for compiling
# - Unit testing (broken?)
# - Cleaning up
# - Dependencies
#

AUTOMAKE_OPTIONS = 1.6.0
ACLOCAL_AMFLAGS = -I .

DISTCHECK_CONFIGURE_FLAGS = \
	$(NULL)

STD_DIRS = \
	loader \
	db \
	transform \
	util \
	frontend \
	. \
	$(NULL)

SUBDIRS = $(STD_DIRS)
DIST_SUBDIRS = $(STD_DIRS)

nobase_include_HEADERS = include/*.h

bin_PROGRAMS = boomerang bffDump
noinst_LIBRARIES = libdriver.a
bffDump_sources = loader/bffDump.cpp
boomerang_SOURCES = boomerang.cpp $(STATIC_OBJS) $(GENSSL)
boomerang_LDADD = libdriver.a
libdriver_a_SOURCES= driver.cpp include/boomerang.h

