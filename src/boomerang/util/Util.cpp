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


}
