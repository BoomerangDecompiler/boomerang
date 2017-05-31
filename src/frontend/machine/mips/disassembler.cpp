#define sign_extend(N, SIZE)    (((int)((N) << (sizeof(unsigned) * 8 - (SIZE)))) >> (sizeof(unsigned) * 8 - (SIZE)))
#include <assert.h>

// #line 2 "machine/mips/disassembler.m"

/****************************************************************
 *
 * FILENAME
 *
 *   \file disassembler.cpp
 *
 * PURPOSE
 *
 *   Skeleton for MIPS disassembly
 *
 * AUTHOR
 *
 *   \author Markus Gothe, nietzsche@lysator.liu.se
 *
 * REVISION
 *
 *   $Id$
 *
 *****************************************************************/

#include "global.h"
#include "include/decoder.h"
