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
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/exp/TypedExp.h"
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


bool ExpPrinter::preVisit(const std::shared_ptr<Ternary>& exp, bool& visitChildren)
{
    visitChildren = false;

    SharedExp p1 = exp->getSubExp1();
    SharedExp p2 = exp->getSubExp2();
    SharedExp p3 = exp->getSubExp3();

    switch (exp->getOper())
    {
    // The "function-like" ternaries
    case opTruncu:
    case opTruncs:
    case opZfill:
    case opSgnEx:
    case opFsize:
    case opItof:
    case opFtoi:
    case opFround:
    case opFtrunc:
    case opOpTable:

        switch (exp->getOper())
        {
        case opTruncu:      *m_os << "truncu(";     break;
        case opTruncs:      *m_os << "truncs(";     break;
        case opZfill:       *m_os << "zfill(";      break;
        case opSgnEx:       *m_os << "sgnex(";      break;
        case opFsize:       *m_os << "fsize(";      break;
        case opItof:        *m_os << "itof(";       break;
        case opFtoi:        *m_os << "ftoi(";       break;
        case opFround:      *m_os << "fround(";     break;
        case opFtrunc:      *m_os << "ftrunc(";     break;
        case opOpTable:     *m_os << "optable(";    break;
        default:            break; // For warning
        }

        // Use print not printr here, since , has the lowest precendence of all.
        // Also it makes it the same as UQBT, so it's easier to test
        if (p1) {
            p1->print(*m_os, m_html);
        }
        else {
            *m_os << "<nullptr>";
        }

        *m_os << ",";

        if (p2) {
            p2->print(*m_os, m_html);
        }
        else {
            *m_os << "<nullptr>";
        }

        *m_os << ",";

        if (p3) {
            p3->print(*m_os, m_html);
        }
        else {
            *m_os << "<nullptr>";
        }

        *m_os << ")";
        return true;

    default:
        break;
    }

    // Else must be ?: or @ (traditional ternary operators)
    if (p1) {
        p1->printr(*m_os, m_html);
    }
    else {
        *m_os << "<nullptr>";
    }

    if (exp->getOper() == opTern) {
        *m_os << " ? ";

        if (p2) {
            p2->printr(*m_os, m_html);
        }
        else {
            *m_os << "<nullptr>";
        }

        *m_os << " : "; // Need wide spacing here

        if (p3) {
            p3->print(*m_os, m_html);
        }
        else {
            *m_os << "<nullptr>";
        }
    }
    else if (exp->getOper() == opAt) {
        *m_os << "@";

        if (p2) {
            p2->printr(*m_os, m_html);
        }
        else {
            *m_os << "nullptr>";
        }

        *m_os << ":";

        if (p3) {
            p3->printr(*m_os, m_html);
        }
        else {
            *m_os << "nullptr>";
        }
    }
    else {
        LOG_FATAL("Invalid operator %1", operToString(exp->getOper()));
    }

    return true;
}



bool ExpPrinter::preVisit(const std::shared_ptr<TypedExp>& exp, bool& visitChildren)
{
    if (exp->getType()) {
        *m_os << "*" << "*m_type" << "* ";
    }
    else {
        *m_os << "*v* ";
    }

    visitChildren = true;
    return true;
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
