/*
 * Copyright (C) 2000-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:       util.cc
 * OVERVIEW:   This file contains miscellaneous functions that don't belong to
 *             any particular subsystem of UQBT.
 *============================================================================*/

/*
 * $Revision$
 *
 * 05 Sep 00 - Mike: moved getCodeInfo here from translate2c.cc
 * 21 Sep 00 - Mike: getTempType handles tmph, tmpb now
 * 27 Sep 00 - Mike: getCodeInfo() was not setting parameter last correctly,
 *              and was not rejecting addresses outside the code section
 * 04 Dec 00 - Mike: Added #ifndef NOFRILLS for complex functions, so we can
 *              use simple things like str() and string::operator+ in
 *              PAL/parsepatterns without linking in BINARYFILE/?? etc
 * 11 Feb 01 - Nathan: Removed getCodeInfo and getTempType to prog.cc & type.cc
 * 22 Feb 01 - Nathan: Moved changeExt here from driver.cc
 * 25 Feb 01 - Nathan: Removed NOFRILLS (no longer applicable)
 * 16 Apr 01 - Brian: added hasExt() procedure.
 * 14 Aug 01 - Mike: Added upperStr() to replace Doug's use of transform();
 *              gcc v3 no longer allows the use of char* for an iterator
 * 10 Apr 02 - Mike: Mods for boomerang; put expSimplify here
 */

#include <string>
#include <sstream>
#include <stdio.h>
#include "util.h"
#include "type.h"

/*==============================================================================
 * FUNCTION:      str
 * OVERVIEW:      Return the null terminated string for an std::ostringstream object
 *                NOTE: this used to be a WORK AROUND for a perceived BUG in
 *                gcc 2.8.1's ostrstream::str() function
 *                Now it just saves a const cast to char*
 * PARAMETERS:    os: the stringstream object
 * RETURNS:       The null terminated char*
 *============================================================================*/
char* str(std::ostringstream& os) {
    const char* p = os.str().c_str();
	return (char*)p;
}

/*==============================================================================
 * FUNCTION:      string::operator+(string, int)
 * OVERVIEW:      Append an int to a string
 * PARAMETERS:    s: the string to append to
 *				  i: the integer whose ascii representation is to be appended
 * RETURNS:       A copy of the modified string
 *============================================================================*/
std::string operator+(const std::string& s, int i)
{
	static char buf[50];
	std::string ret(s);

	sprintf(buf,"%d",i);
	return ret.append(buf);
}

/*==============================================================================
 * FUNCTION:      initCapital
 * OVERVIEW:      Return a string the same as the input string, but with the
 *					first character capitalised
 * PARAMETERS:    s: the string to capitalise
 * RETURNS:       A copy of the modified string
 *============================================================================*/
std::string initCapital(const std::string& s)
{
	std::string res(s);
    res[0] = toupper(res[0]);
	return res;
}

/*==============================================================================
 * FUNCTION:      hasExt
 * OVERVIEW:      Returns true if the given file name has the given extension
 *                and false otherwise.
 * PARAMETERS:    s: string representing a file name (e.g. string("foo.c"))
 *                e: the extension (e.g. ".o")
 * RETURNS:       Boolean indicating whether the file name has the extension.
 *============================================================================*/
bool hasExt(const std::string& s, const char* ext)
{
    std::string tailStr = std::string(".") + std::string(ext);
    unsigned int i = s.rfind(tailStr);
    if (i == std::string::npos) {
        return false;
    } else {
        unsigned int sLen = s.length();
        unsigned int tailStrLen = tailStr.length();
        return ((i + tailStrLen) == sLen);
    }
}

/*==============================================================================
 * FUNCTION:      changeExt
 * OVERVIEW:      Change the extension of the given file name
 * PARAMETERS:    s: string representing the file name to be modified
 *                  (e.g. string("foo.c"))
 *                e: the new extension (e.g. ".o")
 * RETURNS:       The converted string (e.g. "foo.o")
 *============================================================================*/
std::string changeExt(const std::string& s, const char* ext)
{
    size_t i = s.rfind(".");
    if (i == std::string::npos) {
        return s + ext;
    }
    else {
        return s.substr(0, i) + ext;
    }
}

/*==============================================================================
 * FUNCTION:      searchAndReplace
 * OVERVIEW:      returns a copy of a string will all occurances of match
 *                replaced with rep. (simple version of s/match/rep/g)
 * PARAMETERS:    in: the source string
 *                match: the search string
 *                rep: the string to replace match with.
 * RETURNS:       The updated string.
 *============================================================================*/
std::string searchAndReplace( const std::string &in, const std::string &match,
                              const std::string &rep )
{
    std::string result;
    for( int n = 0; n != -1; ) {
        int l = in.find(match,n);
        result.append( in.substr(n,(l==-1?in.length() : l )-n) );
        if( l != -1 ) {
            result.append( rep );
            l+=match.length();
        }
        n = l;
    }
    return result;
}

/*==============================================================================
 * FUNCTION:      upperStr
 * OVERVIEW:      Uppercase a C string
 * PARAMETERS:    s: the string (char*) to start with
 *                d: the string (char*) to write to (can be the same string)
 * RETURNS:       Nothing; the string is modified as a side effect
 *============================================================================*/
void upperStr(const char* s, char* d)
{
    int len = strlen(s);
    for (int i=0; i < len; i++)
       d[i] = toupper(s[i]);
    d[len] = '\0';
}

