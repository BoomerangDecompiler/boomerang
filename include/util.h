/***************************************************************************/ /**
  * \file       util.h
  * OVERVIEW:   Provides the definition for the miscellaneous bits and pieces
  *                 implemented in the util.so library
  ******************************************************************************/
#ifndef __UTIL_H__
#define __UTIL_H__

#include <sstream>
#include <string>

// was a workaround
#define STR(x) (char *)(x.str().c_str())
// Add string and integer
std::string operator+(const std::string &s, int i);

void escapeXMLChars(std::string &s);
char *escapeStr(const char *str);

int lockFileWrite(const char *fname);
void unlockFile(int n);
#ifdef __MINGW32__
extern "C" { extern char *strdup(const char *__s); }
#endif
#endif
