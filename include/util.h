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

// This is a workaround for gcc's buggy ostrstream::str()
char* str(std::ostringstream& os);
// Upper case a C string: s is source, d is dest
void upperStr(const char* s, char* d);
// Add string and integer
std::string operator+(const std::string& s, int i);
#endif
