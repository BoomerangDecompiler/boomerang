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

/***************************************************************************/ /**
  * \file       util.cc
  * \brief   This file contains miscellaneous functions that don't belong to
  *             any particular subsystem of UQBT.
  ******************************************************************************/

#include "util.h"

#include <cassert>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <iomanip> // For setw

/***************************************************************************/ /**
  * \name string::operator+(string, int)
  * \brief      Append an int to a string
  * \param s: the string to append to
  * \param i: the integer whose ascii representation is to be appended
  * \returns        A copy of the modified string
  ******************************************************************************/
std::string operator+(const std::string &s, int i) {
    static char buf[50];
    std::string ret(s);

    sprintf(buf, "%d", i);
    return ret.append(buf);
}

/***************************************************************************/ /**
  * \brief      Uppercase a C string
  * Returns nothing; the string is modified as a side effect
  * \param   s the string (char*) to start with
  * \param   d the string (char*) to write to (can be the same string)
  ******************************************************************************/
void upperStr(const char *s, char *d) {
    size_t len = strlen(s);
    for (size_t i = 0; i < len; i++)
        d[i] = toupper(s[i]);
    d[len] = '\0';
}

int lockFileWrite(const char *fname) {
    int fd = open(fname, O_WRONLY); /* get the file descriptor */
    return fd;
}

void unlockFile(int fd) { close(fd); }

void escapeXMLChars(std::string &s) {
    std::string bad = "<>&";
    const char *replace[] = {"&lt;", "&gt;", "&amp;"};
    for (unsigned i = 0; i < s.size(); i++) {
        std::string::size_type n = bad.find(s[i]);
        if (n != std::string::npos) {
            s.replace(i, 1, replace[n]);
        }
    }
}

// Turn things like newline, return, tab into \n, \r, \t etc
// Note: assumes a C or C++ back end...
char *escapeStr(const char *str) {
    std::ostringstream out;
    char unescaped[] = "ntvbrfa\"";
    char escaped[] = "\n\t\v\b\r\f\a\"";
    bool escapedSucessfully;

    // test each character
    for (; *str; str++) {
        if (isprint((unsigned char)*str) && *str != '\"') {
            // it's printable, so just print it
            out << *str;
        } else { // in fact, this shouldn't happen, except for "
            // maybe it's a known escape sequence
            escapedSucessfully = false;
            for (int i = 0; escaped[i] && !escapedSucessfully; i++) {
                if (*str == escaped[i]) {
                    out << "\\" << unescaped[i];
                    escapedSucessfully = true;
                }
            }
            if (!escapedSucessfully) {
                // it isn't so just use the \xhh escape
                out << "\\x" << std::hex << std::setfill('0') << std::setw(2) << (int)*str;
                out << std::setfill(' ');
            }
        }
    }

    char *ret = new char[out.str().size() + 1];
    strcpy(ret, out.str().c_str());
    return ret;
}
#include "types.h"
#include <iomanip>
std::ostream &operator<<(std::ostream &stream, const ADDRESS &addr) {
    stream << std::hex << addr.m_value << std::dec;
    return stream;
}
#include <QtCore/QTextStream>
QTextStream& operator<<(QTextStream& os, const ADDRESS& mdv) {
    //TODO: properly format ADDRESS : 0-fill to max width.
    os << QString::number(mdv.m_value,16);
    return os;
}

#ifdef __MINGW32__
#include <cstdlib>
char *strdup(const char *s) {
    char *res = (char *)malloc(strlen(s)+1);
    strcpy(res, s);
    return res;
}
#endif
