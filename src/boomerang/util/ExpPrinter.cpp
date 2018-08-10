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
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/exp/TypedExp.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/OStream.h"
#include "boomerang/util/log/Log.h"

#include <QMap>


struct FixSyntax
{
    QString m_prefix;
    QString m_infix1; // unused for unary and 0-ary expressions
    QString m_infix2; // unused for non-ternary expressions.
    QString m_postfix;
};


// ordered by OPER value
static const QMap<OPER, FixSyntax> g_syntaxTable = {
    { opPlus,           { "",           " + ",          "",             ""          } },
    { opMinus,          { "",           " - ",          "",             ""          } },
    { opMult,           { "",           " * ",          "",             ""          } },
    { opDiv,            { "",           " / ",          "",             ""          } },
    { opFPlus,          { "",           " +f ",         "",             ""          } },
    { opFMinus,         { "",           " -f ",         "",             ""          } },
    { opFMult,          { "",           " *f ",         "",             ""          } },
    { opFDiv,           { "",           " /f ",         "",             ""          } },
    { opFNeg,           { "",           " -f ",         "",             ""          } },
    { opMults,          { "",           " *! ",         "",             ""          } },
    { opDivs,           { "",           " /! ",         "",             ""          } },
    { opMod,            { "",           " % ",          "",             ""          } },
    { opMods,           { "",           " %! ",         "",             ""          } },
    { opNeg,            { " -",         "",             "",             ""          } },
    { opAnd,            { "",           " and ",        "",             ""          } },
    { opOr,             { "",           " or ",         "",             ""          } },
    { opEquals,         { "",           " = ",          "",             ""          } },
    { opNotEqual,       { "",           " ~= ",         "",             ""          } },
    { opLess,           { "",           " < ",          "",             ""          } },
    { opGtr,            { "",           " > ",          "",             ""          } },
    { opLessEq,         { "",           " <= ",         "",             ""          } },
    { opGtrEq,          { "",           " >= ",         "",             ""          } },
    { opLessUns,        { "",           " <u ",         "",             ""          } },
    { opGtrUns,         { "",           " >u ",         "",             ""          } },
    { opLessEqUns,      { "",           " <=u ",        "",             ""          } },
    { opGtrEqUns,       { "",           " >=u ",        "",             ""          } },
    { opNot,            { " ~",         "",             "",             ""          } },
    { opLNot,           { " L~",        "",             "",             ""          } },
    { opSignExt,        { "",           "",             "",             "! "        } },
    { opBitAnd,         { "",           " & ",          "",             ""          } },
    { opBitOr,          { "",           " | ",          "",             ""          } },
    { opBitXor,         { "",           " ^ ",          "",             ""          } },
    { opShiftL,         { "",           " << ",         "",             ""          } },
    { opShiftR,         { "",           " >> ",         "",             ""          } },
    { opShiftRA,        { "",           " >>A ",        "",             ""          } },
    { opRotateL,        { "",           " rl ",         "",             ""          } },
    { opRotateR,        { "",           " rr ",         "",             ""          } },
    { opRotateLC,       { "",           " rlc ",        "",             ""          } },
    { opRotateRC,       { "",           " rrc ",        "",             ""          } },
    { opExpTable,       { "exptable(",  ", ",           "",             ")"         } },
    { opNameTable,      { "nametable(", ", ",           "",             ")"         } },
    { opOpTable,        { "optable(",   ", ",           ", ",           ")"         } },
    { opSuccessor,      { "succ(",      "",             "",             ")"         } },
    { opTern,           { "",           " ? ",          " : ",          "",         } },
    { opAt,             { "",           "@",            ":",            "",         } },
    { opRegOf,          { "r[",         "",             "",             "]"         } },
    { opMemOf,          { "m[",         "",             "",             "]"         } },
    { opAddrOf,         { "a[",         "",             "",             "]"         } },
    { opWildMemOf,      { "m[wild]",    "",             "",             ""          } },
    { opWildRegOf,      { "r[wild]",    "",             "",             ""          } },
    { opWildAddrOf,     { "a[wild]",    "",             "",             ""          } },
    { opDefineAll,      { "<all>",      "",             "",             ""          } },
    { opVar,            { "v[",         "",             "",             "]"         } },
    { opPhi,            { "phi(",       "",             "",             ")"         } },
    { opArrayIndex,     { "",           "[",            "",             "]"         } },
    { opMachFtr,        { "machine(",   "",             "",             ")"         } },
    { opTruncu,         { "truncu(",    ", ",           ", ",           ")"         } },
    { opTruncs,         { "truncs(",    ", ",           ", ",           ")"         } },
    { opZfill,          { "zfill(",     ", ",           ", ",           ")"         } },
    { opSignExt,        { "sgnex(",     ", ",           ", ",           ")"         } },
    { opFsize,          { "fsize(",     ", ",           ", ",           ")"         } },
    { opItof,           { "itof(",      ", ",           ", ",           ")"         } },
    { opFtoi,           { "ftoi(",      ", ",           ", ",           ")"         } },
    { opFround,         { "fround(",    ", ",           ", ",           ")"         } },
    { opFtrunc,         { "ftrunc(",    ", ",           ", ",           ")"         } },
    { opFabs,           { "fabs(",      "",             "",             ")"         } },
    { opFpush,          { "FPUSH",      "",             "",             ""          } },
    { opFpop,           { "FPOP",       "",             "",             ""          } },
    { opSin,            { "sin(",       "",             "",             ")"         } },
    { opCos,            { "cos(",       "",             "",             ")"         } },
    { opTan,            { "tan(",       "",             "",             ")"         } },
    { opSin,            { "sin(",       "",             "",             ")"         } },
    { opArcTan,         { "arctan(",    "",             "",             ")"         } },
    { opLog2,           { "log2(",      "",             "",             ")"         } },
    { opLog10,          { "log10(",     "",             "",             ")"         } },
    { opLoge,           { "loge(",      "",             "",             ")"         } },
    { opPow,            { "",           " pow ",        "",             ")"         } },
    { opSqrt,           { "sqrt(",      "",             "",             ")"         } },
    { opExecute,        { "execute(",   "",             "",             ")"         } },
    { opWildIntConst,   { "WILDINT",    "",             "",             ""          } },
    { opWildStrConst,   { "WILDSTR",    "",             "",             ""          } },
    { opPC,             { "%pc",        "",             "",             ""          } },
    { opAFP,            { "%afp",       "",             "",             ""          } },
    { opAGP,            { "%agp",       "",             "",             ""          } },
    { opNil,            { "",           "",             "",             ""          } },
    { opFlags,          { "%flags",     "",             "",             ""          } },
    { opFflags,         { "%fflags",    "",             "",             ""          } },
    { opAnull,          { "%anul",      "",             "",             ""          } },
    { opTrue,           { "true",       "",             "",             ""          } },
    { opFalse,          { "false",      "",             "",             ""          } },
    { opTypeOf,         { "T[",         "",             "",             "]"         } },
    { opKindOf,         { "K[",         "",             "",             "]"         } },
    { opInitValueOf,    { "",           "",             "",             "'"         } },
    { opZF,             { "%ZF",        "",             "",             "",         } },
    { opCF,             { "%CF",        "",             "",             "",         } },
    { opNF,             { "%NF",        "",             "",             "",         } },
    { opOF,             { "%OF",        "",             "",             "",         } },
    { opDF,             { "%DF",        "",             "",             "",         } }
};


void ExpPrinter::print(OStream& os, const Exp& exp, bool html) const
{
    if (html) {
        printHTML(os, exp.shared_from_this());
    }
    else {
        print(os, exp.shared_from_this());
    }
}


void ExpPrinter::print(OStream& os, const std::shared_ptr<const Exp>& exp) const
{
    const OPER oper = exp->getOper();

    // operators that need special attention
    switch (oper) {
    case opList:
        print(os, exp->getSubExp1());
        if (!exp->getSubExp2()->isNil()) {
            os << ", ";
        }
        print(os, exp->getSubExp2());
        return;

    case opFlagCall:
        // The name of the flag function (e.g. ADDFLAGS) should be enough
        exp->access<const Const, 1>()->printNoQuotes(os);
        os << "( ";
        print(os, exp->getSubExp2());
        os << " )";
        return;

    case opSize:
        // This can still be seen after decoding and before type analysis after m[...]
        // *size* is printed after the expression, even though it comes from the first subexpression
        print(os, exp->getSubExp2());
        os << "*";
        print(os, exp->getSubExp1());
        os << "*";
        return
        ;
    case opTemp:
        if (exp->getSubExp1()->getOper() == opWildStrConst) {
            assert(exp->getSubExp1()->isTerminal());
            os << "t[";
            exp->access<const Terminal, 1>()->print(os);
            os << "]";
            return;
        }
        // fallthrough

    case opGlobal:
    case opLocal:
    case opParam:
        // Print a more concise form than param["foo"] (just foo)
        exp->access<const Const, 1>()->printNoQuotes(os);
        return;

    case opMemberAccess:
        print(os, exp->getSubExp1());
        os << ".";
        exp->access<const Const, 2>()->printNoQuotes(os);
        return;

    case opIntConst:
        if (exp->access<const Const>()->getInt() < -1000 ||
            exp->access<const Const>()->getInt() > +1000) {
            os << "0x" << QString::number(exp->access<const Const>()->getInt(), 16);
        }
        else {
            os << exp->access<const Const>()->getInt();
        }
        return;

    case opLongConst:
        if ((static_cast<long long>(exp->access<const Const>()->getLong()) < -1000LL) ||
            (static_cast<long long>(exp->access<const Const>()->getLong()) > +1000LL)) {
            os << "0x" << QString::number(exp->access<const Const>()->getLong(), 16) << "LL";
        }
        else {
            os << exp->access<const Const>()->getLong() << "LL";
        }
        break;

    case opFltConst:
        os << QString("%1").arg(exp->access<const Const>()->getFlt()); // respects English locale
        return;

    case opStrConst:
        os << "\"" << exp->access<const Const>()->getStr() << "\"";
        return;

    case opRegOf:
        if (exp->getSubExp1()->isIntConst()) {
            os << "r" << exp->access<const Const, 1>()->getInt();
            return;
        }
        else if (exp->getSubExp1()->isTemp()) {
            // Just print the temp {   // balance }s
            print(os, exp->getSubExp1());
            return;
        }


    case opSubscript:
        print(os, exp->getSubExp1());
        if (exp->access<const RefExp>()->getDef() == STMT_WILD) {
            os << "{WILD}";
        }
        else if (exp->access<const RefExp>()->getDef()) {
            os << "{" << exp->access<const RefExp>()->getDef()->getNumber() << "}";
        }
        else {
            os << "{-}"; // So you can tell the difference with {0}
        }
        return;

    default: break;
    }

    assert(g_syntaxTable.find(oper) != g_syntaxTable.end());
    os << g_syntaxTable[oper].m_prefix;

    if (exp->getArity() >= 1) {
        if (childNeedsParentheses(exp, exp->getSubExp1())) os << "(";
        assert(exp->getSubExp1()); print(os, exp->getSubExp1());
        if (childNeedsParentheses(exp, exp->getSubExp1())) os << ")";

        if (exp->getArity() >= 2) {
            os << g_syntaxTable[oper].m_infix1;

            if (childNeedsParentheses(exp, exp->getSubExp2())) os << "(";
            assert(exp->getSubExp2()); print(os, exp->getSubExp2());
            if (childNeedsParentheses(exp, exp->getSubExp2())) os << ")";

            if (exp->getArity() >= 3) {
                os << g_syntaxTable[oper].m_infix2;

                if (childNeedsParentheses(exp, exp->getSubExp3())) os << "(";
                assert(exp->getSubExp3()); print(os, exp->getSubExp3());
                if (childNeedsParentheses(exp, exp->getSubExp3())) os << ")";
            }
        }
    }

    os << g_syntaxTable[oper].m_postfix;
}


void ExpPrinter::printHTML(OStream& os, const std::shared_ptr<const Exp>& exp) const
{
    Q_UNUSED(os);
    Q_UNUSED(exp);
    assert(false);
}



bool ExpPrinter::childNeedsParentheses(const SharedConstExp& exp, const SharedConstExp& child) const
{
    // never parenthesize things like m[...] or foo{-} or constants
    if (child->getArity() < 2) {
        return false;
    }

    if (exp->getArity() == 3) {
        switch (exp->getOper()) {
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
            return false;
        default:
            return true;
        }
    }
    else if (exp->getArity() == 2) {
        // parenthesize foo == (bar < baz)
        if (exp->isComparison() && child->isComparison()) {
            return true;
        }

        switch (exp->getOper()) {
        case opSize:
        case opList:
            return false;

        default:
            return true;
        }
    }

    return false;
}
