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

#include "boomerang/util/Util.h"
#include "boomerang/util/types.h"

#include <QString>
#include <QMap>
#include <QTextStream>
#include <cassert>
#include <string>

namespace Util
{

// Turn things like newline, return, tab into \n, \r, \t etc
// Note: assumes a C or C++ back end...
QString escapeStr(const QString& inp)
{
    static QMap<char, QString> replacements {
        {
            '\n', "\\n"
        }, {
            '\t', "\\t"
        }, {
            '\v', "\\v"
        }, {
            '\b', "\\b"
        }, {
            '\r', "\\r"
        }, {
            '\f', "\\f"
        }, {
            '\a', "\\a"
        },
        {
            '"', "\\\""
        }
    };

    QString res;

    for (char c : inp.toLocal8Bit()) {
        if (isprint(c) && (c != '\"')) {
            res += QChar(c);
            continue;
        }

        if (replacements.contains(c)) {
            res += replacements[c];
        }
        else {
            res += "\\" + QString::number(c, 16);
        }
    }

    return res;
}


SWord normEndian(SWord value, bool srcBigEndian)
{
    return (srcBigEndian == BOOMERANG_BIG_ENDIAN) ? value : swapEndian(value);
}

DWord normEndian(DWord value, bool srcBigEndian)
{
    return (srcBigEndian == BOOMERANG_BIG_ENDIAN) ? value : swapEndian(value);
}

QWord normEndian(QWord value, bool srcBigEndian)
{
    return (srcBigEndian == BOOMERANG_BIG_ENDIAN) ? value : swapEndian(value);
}

Byte readByte(const void* src)
{
    assert(src);
    return *(const Byte*)src;
}

SWord readWord(const void* src, bool srcBigEndian)
{
    assert(src);
    return  normEndian(*(const SWord*)src, srcBigEndian);
}

DWord readDWord(const void* src, bool srcBigEndian)
{
    assert(src);
    return normEndian(*(const DWord*)src, srcBigEndian);
}

QWord readQWord(const void* src, bool srcBigEndian)
{
    assert(src);
    return normEndian(*(const QWord*)src, srcBigEndian);
}


void writeByte(void* dst, Byte value)
{
    assert(dst);
    *(Byte*)dst = value;
}

void writeWord(void* dst, SWord value, bool dstBigEndian)
{
    assert(dst);
    *(SWord*)dst = normEndian(value, dstBigEndian);
}


void writeDWord(void* dst, DWord value, bool dstBigEndian)
{
    assert(dst);
    *(DWord*)dst = normEndian(value, dstBigEndian);
}

void writeQWord(void* dst, DWord value, bool dstBigEndian)
{
    assert(dst);
    *(QWord*)dst = normEndian(value, dstBigEndian);
}


}
