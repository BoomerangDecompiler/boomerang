#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpPrinter.h"


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/OStream.h"
#include "boomerang/util/log/Log.h"


ExpPrinter::ExpPrinter(Exp& exp, bool html)
    : m_exp(exp)
    , m_html(html)
{
}


OStream& operator<<(OStream& lhs, ExpPrinter&& rhs)
{
    rhs.m_os = &lhs;
    rhs.m_exp.acceptVisitor(&rhs);

    return lhs;
}


bool ExpPrinter::preVisit(const std::shared_ptr<RefExp>& exp, bool& visitChildren)
{
    visitChildren = false;

    if (exp->getSubExp1()) {
        exp->getSubExp1()->print(*m_os, m_html);
    }
    else {
        *m_os << "<nullptr>";
    }

    return true;
}


bool ExpPrinter::postVisit(const std::shared_ptr<RefExp>& exp)
{
    if (m_html) {
        *m_os << "<sub>";
    }
    else {
        *m_os << "{";
    }

    if (exp->getDef() == STMT_WILD) {
        *m_os << "WILD";
    }
    else if (exp->getDef()) {
        if (m_html) {
            *m_os << "<a href=\"#stmt" << exp->getDef()->getNumber() << "\">";
        }

        *m_os << exp->getDef()->getNumber();

        if (m_html) {
            *m_os << "</a>";
        }
    }
    else {
        *m_os << "-"; // So you can tell the difference with {0}
    }

    if (m_html) {
        *m_os << "</sub>";
    }
    else {
        *m_os << "}";
    }

    return true;
}



bool ExpPrinter::visit(const std::shared_ptr<Const>& exp)
{
    switch (exp->getOper())
    {
    case opIntConst:
        if (exp->getInt() < -1000 || exp->getInt() > 1000) {
            *m_os << "0x" << QString::number(exp->getInt(), 16);
        }
        else {
            *m_os << exp->getInt();
        }
        break;

    case opLongConst:
        if ((static_cast<long long>(exp->getLong()) < -1000LL) ||
            (static_cast<long long>(exp->getLong()) > 1000LL)) {
                *m_os << "0x" << QString::number(exp->getLong(), 16) << "LL";
        }
        else {
            *m_os << exp->getLong() << "LL";
        }
        break;

    case opFltConst:
        *m_os << QString("%1").arg(exp->getFlt()); // respects English locale
        break;

    case opStrConst:
        *m_os << "\"" << exp->getStr() << "\"";
        break;

    default:
        LOG_FATAL("Invalid operator %1", operToString(exp->getOper()));
    }

    if (exp->getConscript()) {
        *m_os << "\\" << exp->getConscript() << "\\";
    }

#ifdef DUMP_TYPES
    if (exp->getType()) {
        *m_os << "T(" << exp->getType()->prints() << ")";
    }
#endif

    return true;
}


bool ExpPrinter::visit(const std::shared_ptr<Terminal>& exp)
{
    switch (exp->getOper())
    {
    case opPC:          *m_os << "%pc";         break;
    case opFlags:       *m_os << "%flags";      break;
    case opFflags:      *m_os << "%fflags";     break;
    case opCF:          *m_os << "%CF";         break;
    case opZF:          *m_os << "%ZF";         break;
    case opOF:          *m_os << "%OF";         break;
    case opNF:          *m_os << "%NF";         break;
    case opDF:          *m_os << "%DF";         break;
    case opAFP:         *m_os << "%afp";        break;
    case opAGP:         *m_os << "%agp";        break;
    case opWild:        *m_os << "WILD";        break;
    case opAnull:       *m_os << "%anul";       break;
    case opFpush:       *m_os << "FPUSH";       break;
    case opFpop:        *m_os << "FPOP";        break;
    case opWildMemOf:   *m_os << "m[WILD]";     break;
    case opWildRegOf:   *m_os << "r[WILD]";     break;
    case opWildAddrOf:  *m_os << "a[WILD]";     break;
    case opWildIntConst:*m_os << "WILDINT";     break;
    case opWildStrConst:*m_os << "WILDSTR";     break;
    case opNil:                                 break;
    case opTrue:        *m_os << "true";        break;
    case opFalse:       *m_os << "false";       break;
    case opDefineAll:   *m_os << "<all>";       break;
    default:
        LOG_FATAL("Invalid operator %1", operToString(exp->getOper()));
    }

    return true;
}
