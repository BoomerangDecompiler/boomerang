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


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/db/Prog.h"
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
    static const QMap<char, QString> replacements {
        { '\n', "\\n" },
        { '\t', "\\t" },
        { '\v', "\\v" },
        { '\b', "\\b" },
        { '\r', "\\r" },
        { '\f', "\\f" },
        { '\a', "\\a" },
        { '"', "\\\"" }
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


int getStackOffset(SharedConstExp e, int sp)
{
    int ret = 0;

    if (e->isMemOf()) {
        SharedConstExp sub = e->getSubExp1();
        OPER      op  = sub->getOper();

        if ((op == opPlus) || (op == opMinus)) {
            SharedConstExp op1 = sub->getSubExp1();

            if (op1->isSubscript()) {
                op1 = op1->getSubExp1();
            }

            if (op1->isRegN(sp)) {
                SharedConstExp op2 = sub->getSubExp2();

                if (op2->isIntConst()) {
                    ret = op2->access<const Const>()->getInt();
                }

                if (op == opMinus) {
                    ret = -ret;
                }
            }
        }
    }

    return ret;
}


int getStackRegisterIndex(const Prog *prog)
{
    switch (prog->getMachine())
    {
    case Machine::SPARC:        return REG_SPARC_SP;
    case Machine::PENTIUM:      return REG_PENT_ESP;
    case Machine::PPC:          return  1;
    case Machine::ST20:         return  3;
    case Machine::MIPS:         return 29;
    default:                    return -1;
    }
}


}
