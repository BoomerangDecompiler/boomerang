#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Util.h"


#include "boomerang/util/Types.h"

#include <QString>
#include <QMap>
#include <QTextStream>

#include <cassert>
#include <string>


char debug_buffer[DEBUG_BUFSIZE];


namespace Util
{
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


QTextStream& alignStream(QTextStream& str, int align)
{
    str << qSetFieldWidth(align) << " " << qSetFieldWidth(0);
    return str;
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


Byte readByte(const void *src)
{
    assert(src);
    return *reinterpret_cast<const Byte *>(src);
}


SWord readWord(const void *src, bool srcBigEndian)
{
    assert(src);
    return normEndian(*reinterpret_cast<const SWord *>(src), srcBigEndian);
}


DWord readDWord(const void *src, bool srcBigEndian)
{
    assert(src);
    return normEndian(*reinterpret_cast<const DWord *>(src), srcBigEndian);
}


QWord readQWord(const void *src, bool srcBigEndian)
{
    assert(src);
    return normEndian(*reinterpret_cast<const QWord *>(src), srcBigEndian);
}


void writeByte(void *dst, Byte value)
{
    assert(dst);
    *reinterpret_cast<Byte *>(dst) = value;
}


void writeWord(void *dst, SWord value, bool dstBigEndian)
{
    assert(dst);
    *reinterpret_cast<SWord *>(dst) = normEndian(value, dstBigEndian);
}


void writeDWord(void *dst, DWord value, bool dstBigEndian)
{
    assert(dst);
    *reinterpret_cast<DWord *>(dst) = normEndian(value, dstBigEndian);
}


void writeQWord(void *dst, DWord value, bool dstBigEndian)
{
    assert(dst);
    *reinterpret_cast<QWord *>(dst) = normEndian(value, dstBigEndian);
}
}
