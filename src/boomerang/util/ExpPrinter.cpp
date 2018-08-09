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


bool ExpPrinter::preVisit(const std::shared_ptr<Binary>& exp, bool& visitChildren)
{
    visitChildren = false;

    assert(exp->getSubExp1() && exp->getSubExp2());
    SharedConstExp p1 = exp->getSubExp1();
    SharedConstExp p2 = exp->getSubExp2();

    // Special cases
    switch (exp->getOper())
    {
    case opSize:
        // This can still be seen after decoding and before type analysis after m[...]
        // *size* is printed after the expression, even though it comes from the first subexpression
        p2->printr(*m_os, m_html);
        *m_os << "*";
        p1->printr(*m_os, m_html);
        *m_os << "*";
        return true;

    case opFlagCall:
        // The name of the flag function (e.g. ADDFLAGS) should be enough
        std::static_pointer_cast<const Const>(p1)->printNoQuotes(*m_os);
        *m_os << "( ";
        p2->printr(*m_os, m_html);
        *m_os << " )";
        return true;

    case opExpTable:
    case opNameTable:

        if (exp->getOper() == opExpTable) {
            *m_os << "exptable(";
        }
        else {
            *m_os << "nametable(";
        }

        *m_os << p1 << ", " << p2 << ")";
        return true;

    case opList:
        // Because "," is the lowest precedence operator, we don't need printr here.
        // Also, same as UQBT, so easier to test
        p1->print(*m_os, m_html);

        if (!p2->isNil()) {
            *m_os << ", ";
        }

        p2->print(*m_os, m_html);
        return false;

    case opMemberAccess:
        p1->print(*m_os, m_html);
        *m_os << ".";
        std::static_pointer_cast<const Const>(p2)->printNoQuotes(*m_os);
        return false;

    case opArrayIndex:
        p1->print(*m_os, m_html);
        *m_os << "[";
        p2->print(*m_os, m_html);
        *m_os << "]";
        return false;

    default:
        break;
    }

    // Ordinary infix operators. Emit parens around the binary
    if (p1 == nullptr) {
        *m_os << "<nullptr>";
    }
    else {
        p1->printr(*m_os, m_html);
    }

    switch (exp->getOper())
    {
    case opPlus:        *m_os << " + ";    break;
    case opMinus:       *m_os << " - ";    break;
    case opMult:        *m_os << " * ";    break;
    case opMults:       *m_os << " *! ";   break;
    case opDiv:         *m_os << " / ";    break;
    case opDivs:        *m_os << " /! ";   break;
    case opMod:         *m_os << " % ";    break;
    case opMods:        *m_os << " %! ";   break;
    case opFPlus:       *m_os << " +f ";   break;
    case opFMinus:      *m_os << " -f ";   break;
    case opFMult:       *m_os << " *f ";   break;
    case opFDiv:        *m_os << " /f ";   break;
    case opPow:         *m_os << " pow ";  break;     // Raising to power
    case opAnd:         *m_os << " and ";  break;
    case opOr:          *m_os << " or ";   break;
    case opBitAnd:      *m_os << " & ";    break;
    case opBitOr:       *m_os << " | ";    break;
    case opBitXor:      *m_os << " ^ ";    break;
    case opEquals:      *m_os << " = ";    break;
    case opNotEqual:    *m_os << " ~= ";   break;
    case opLess:        *m_os << (m_html ? " &lt; "  : " < ");  break;
    case opGtr:         *m_os << (m_html ? " &gt; "  : " > ");  break;
    case opLessEq:      *m_os << (m_html ? " &lt;= " : " <= "); break;
    case opGtrEq:       *m_os << (m_html ? " &gt;= " : " >= "); break;
    case opLessUns:     *m_os << (m_html ? " &lt;u " : " <u "); break;
    case opGtrUns:      *m_os << (m_html ? " &gt;u " : " >u "); break;
    case opLessEqUns:   *m_os << (m_html ? " &lt;u " : " <=u "); break;
    case opGtrEqUns:    *m_os << (m_html ? " &gt;=u " : " >=u "); break;
    case opUpper:       *m_os << " GT "; break;
    case opLower:       *m_os << " LT "; break;
    case opShiftL:      *m_os << (m_html ? " &lt;&lt; " : " << "); break;
    case opShiftR:      *m_os << (m_html ? " &gt;&gt; " : " >> "); break;
    case opShiftRA:     *m_os << (m_html ? " &gt;&gt;A " : " >>A "); break;
    case opRotateL:     *m_os << " rl "; break;
    case opRotateR:     *m_os << " rr "; break;
    case opRotateLC:    *m_os << " rlc "; break;
    case opRotateRC:    *m_os << " rrc "; break;
    default:
        LOG_FATAL("Invalid operator %1", operToString(exp->getOper()));
    }

    if (p2 == nullptr) {
        *m_os << "<nullptr>";
    }
    else {
        p2->printr(*m_os, m_html);
    }

    return true;
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
