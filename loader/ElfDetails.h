/*
 * Copyright (C) 1998, The University of Queensland
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/* File: Details.h
 * Desc: This file contains definitions for various verbose functions
*/

/* $Revision$
 * 3 Feb 98 - Cristina
 *	added RTL dictionary argument to dumpShdr().
 * 2 Jun 98 - Mike
 *	Loader->BinaryFile; renamed from Verbose.cc; integrated into BinaryFile
*/

#ifndef _DETAILS_H_
#define _DETAILS_H_

void dumpElf32 (Elf32_Ehdr *ehdr, const char *fileName, FILE* f);
void dumpPhdr (Elf32_Phdr *phdr, FILE* f);

void dumpPhdr (Elf32_Phdr *phdr);
void dumpShdr (Elf32_Shdr *shdr, int idxElf, FILE* f);

#endif
