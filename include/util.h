/*==============================================================================
 * FILE:       util.h
 * OVERVIEW:   Provides the definition for the miscellaneous bits and pieces
 *               implemented in the util.so library
 *============================================================================*/
/*
 * $Revision$
 *
 * 10 Apr 02 - Mike: Created
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include <sstream>
#include <string>

// was a workaround
#define STR(x) (char *)(x.str().c_str())
// Upper case a C string: s is source, d is dest
void upperStr(const char* s, char* d);
// Add string and integer
std::string operator+(const std::string& s, int i);

int lockFileRead(const char *fname);
int lockFileWrite(const char *fname);
void unlockFile(int n);

#endif
