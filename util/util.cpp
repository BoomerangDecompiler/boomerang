/*
 * Copyright (C) 2000-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
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

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200 
#pragma warning(disable:4786)
#endif 

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "prog.h"
#include "util.h"
#include "type.h"

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


/*==============================================================================
 * FUNCTION:      load
 * OVERVIEW:      Load a project at a given location
 * PARAMETERS:    location: string to the project file 
 * RETURNS:       Nothing; the prog is modified as a side effect
 *============================================================================*/
void load(Prog *prog, std::string &location)
{
    char buf[1024];
    int fid;

    assert(location != "");

    std::ifstream inf(location.c_str(), std::ios::in|std::ios::binary);

    // make sure standard headers are there
    inf.read(buf, FID_MAGIC_LEN);
    assert(memcmp(buf, FID_MAGIC, FID_MAGIC_LEN) == 0);
    loadValue(inf, fid, false);
    // panic if compatibility is bad.
    assert(fid == FID_VERSION);

	prog->deserialize(inf);

    inf.close();
}

/*==============================================================================
 * FUNCTION:      save
 * OVERVIEW:      Save a project at a given location
 * PARAMETERS:    location: string to the project file 
 * RETURNS:       Nothing
 *============================================================================*/
void save(Prog *prog, std::string &location)
{
    int fid;

    assert(location != "");

    std::ofstream ouf(location.c_str(), std::ios::out|std::ios::binary);

    // write standard headers
    ouf.write(FID_MAGIC, FID_MAGIC_LEN);
    fid = FID_VERSION;
    saveValue(ouf, fid, false);

	int len;
	prog->serialize(ouf, len);

    ouf.close();
}

// helper functions for load/save
void loadString(std::istream &is, std::string &str)
{
    char buf[1024];
    int len = loadLen(is);
    assert(len < 1024);
    is.read(buf, len);
    buf[len] = 0;
    str = buf;
}

void saveString(std::ostream &os, const std::string &str)
{
    int len = str.length();
    saveLen(os, len);
    os.write(str.c_str(), len);
}

int loadFID(std::istream &is)
{
    int fid;
    if (is.eof() || !is.good()) return -1;
    unsigned char ch;
    is.read((char *)&ch, 1);
    if (ch == 255) {
        unsigned char sh[2];
        is.read((char *)sh, 2);
        fid = sh[0] | (sh[1] << 8);
    } else {
        fid = ch;
    }
    return fid;
}

void saveFID(std::ostream &os, int fid)
{
    if (fid < 255) {
        unsigned char ch = fid;
        os.write((char *)&ch, 1);
    } else {
        unsigned char ch = 255;
        os.write((char *)&ch, 1);
        assert(fid < 65536);
        unsigned char sh[2];
        sh[0] = fid & 255;
        sh[1] = (fid >> 8) & 255;
        os.write((char *)sh, 2);
    }
}

void skipFID(std::istream &is, int fid)
{
    std::cerr << "WARNING: unknown id in save file '" << fid << "' ignoring, data may be lost!" << std::endl;
    int len = loadLen(is);
    is.seekg(is.tellg() + (std::streamoff)len);
}

int loadLen(std::istream &is)
{
    int len;
    if (is.eof() || !is.good()) return -1;
    unsigned char ch;
    is.read((char *)&ch, 1);
    if (ch == 255) {
        unsigned char sh[4];
        is.read((char *)sh, 4);
        len = sh[0] | (sh[1] << 8) | (sh[2] << 16) | (sh[3] << 24);
    } else {
        len = ch;
    }
    return len;
}

void saveLen(std::ostream &os, int len, bool large)
{
    if (large || (unsigned)len >= 255) {
        unsigned char ch = 255;
        os.write((char *)&ch, 1);
        unsigned char sh[4];
        sh[0] = len & 255;
        sh[1] = (len >> 8) & 255;
        sh[2] = (len >> 16) & 255;
        sh[3] = (len >> 24) & 255;
        os.write((char *)sh, 4);
    } else {    
        unsigned char ch = len;
        assert(ch != 255);
        os.write((char *)&ch, 1);
    }
}

void loadValue(std::istream &is, bool &b, bool wantlen)
{
    assert(!is.eof() && is.good());
    if (wantlen) {
        int len = loadLen(is);
        assert(len == 1);
    }
    char ch;
    is.read(&ch, 1);
    b = (ch == 0 ? false : true);
}

void saveValue(std::ostream &os, bool b, bool wantlen)
{
    if (wantlen) {
        saveLen(os, 1);
    }
    char ch = b;
    os.write(&ch, 1);
}

void loadValue(std::istream &is, char &ch, bool wantlen)
{
    assert(!is.eof() && is.good());
    if (wantlen) {
        int len = loadLen(is);
        assert(len == 1);
    }
    is.read(&ch, 1);
}

void saveValue(std::ostream &os, char ch, bool wantlen)
{
    if (wantlen) {
        saveLen(os, 1);
    }
    os.write(&ch, 1);
}

void loadValue(std::istream &is, int &i, bool wantlen)
{
    assert(!is.eof() && is.good());
    if (wantlen) {
        int len = loadLen(is);
        assert(len == 4);
    }
    unsigned char sh[4];
    is.read((char *)sh, 4);
    i = sh[0] | (sh[1] << 8) | (sh[2] << 16) | (sh[3] << 24);
}

void saveValue(std::ostream &os, int i, bool wantlen)
{
    if (wantlen) {
        saveLen(os, 4);
    }
    unsigned char sh[4];
    sh[0] = i & 255;
    sh[1] = (i >> 8) & 255;
    sh[2] = (i >> 16) & 255;
    sh[3] = (i >> 24) & 255;
    os.write((char *)sh, 4);
}

void loadValue(std::istream &is, ADDRESS &a, bool wantlen)
{
    assert(!is.eof() && is.good());
    if (wantlen) {
        int len = loadLen(is);
        assert(len == 4);
    }
    unsigned char sh[4];
    is.read((char *)sh, 4);
    a = sh[0] | (sh[1] << 8) | (sh[2] << 16) | (sh[3] << 24);
}

void saveValue(std::ostream &os, ADDRESS a, bool wantlen)
{
    if (wantlen) {
        saveLen(os, 4);
    }
    unsigned char sh[4];
    sh[0] = a & 255;
    sh[1] = (a >> 8) & 255;
    sh[2] = (a >> 16) & 255;
    sh[3] = (a >> 24) & 255;
    os.write((char *)sh, 4);
}

// TODO: how do I store doubles in machine independant form?
void loadValue(std::istream &is, double &d, bool wantlen)
{
    assert(!is.eof() && is.good());
    if (wantlen) {
        int len = loadLen(is);
        assert(len == sizeof(double));
    }
    is.read((char *)&d, sizeof(double));
}

void saveValue(std::ostream &os, double d, bool wantlen)
{
    if (wantlen) {
        saveLen(os, sizeof(d));
    }
    os.write((char *)&d, sizeof(d));
}
