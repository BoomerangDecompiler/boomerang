######################################################
# File: Makefile
# Desc: Makefile for the transformation (transform) directory
#
######################################################

# $Revision$
# 18 Apr 04 - Trent: Created.

AUTOMAKE_OPTIONS = foreign

# HOST_GNU_LD is true if the linker on this host machine is GNU
HOST_GNU_LD = @HOST_GNU_LD@

pkglib_LTLIBRARIES = libtransform.la

libtransform_la_SOURCES = \
		transformer.cpp \
		rdi.cpp \
		generic.cpp \
		transformation-parser.cpp \
		transformation-scanner.cpp \
		$(NULL)
			
noinst_HEADERS = \
		rdi.h \
		generic.h \
		transformation-parser.h \
		transformation-scanner.h \
		$(NULL)

libtransform_la_LDFLAGS = -release $(VERSION)
