/*
 * Copyright (C) 2002-2006, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file       chllcode.cpp
  * \brief   Concrete backend class for the "C" high level language
  *               This class is provides methods which are specific for the C language binding.
  *               I guess this will be the most popular output language unless we do C++.
  ******************************************************************************/

#include "cfg.h"
#include "statement.h"
#include "exp.h"
#include "proc.h"
#include "prog.h"
#include "hllcode.h"
#include "chllcode.h"
#include "signature.h"
#include "boomerang.h"
#include "type.h"
#include "util.h"
#include "log.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <cassert>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <memory>
using namespace std;
static int codegen_progress = 0;
static bool isBareMemof(const Exp &e, UserProc *proc);
// extern char *operStrings[];

/// Empty constructor, calls HLLCode()
CHLLCode::CHLLCode() : HLLCode() {}

/// Empty constructor, calls HLLCode(p)
CHLLCode::CHLLCode(UserProc *p) : HLLCode(p) {}

/// Empty destructor
CHLLCode::~CHLLCode() {}

/// Output 4 * \a indLevel spaces to \a str
void CHLLCode::indent(QTextStream &str, int indLevel) {
    // Can probably do more efficiently
    for (int i = 0; i < indLevel; i++)
        str << "    ";
}
/**
 * Append code for the given expression \a exp to stream \a str.
 *
 * \param str        The stream to output to.
 * \param exp        The expresson to output.
 * \param curPrec     The current operator precedence. Add parens around this expression if necessary.
 * \param uns         If true, cast operands to unsigned if necessary.
 *
 * \todo This function is 800+ lines, and should possibly be split up.
 */
void CHLLCode::appendExp(QTextStream &str, const Exp &exp, PREC curPrec, bool uns /* = false */) {
    if (++codegen_progress > 500) {
        LOG_STREAM() << 'g';
        LOG_STREAM().flush();
        codegen_progress = 0;
    }

    OPER op = exp.getOper();

#ifdef SYMS_IN_BACK_END // Should no longer be any unmapped symbols by the back end
    // Check if it's mapped to a symbol
    if (m_proc && !exp->isTypedExp()) { // Beware: lookupSym will match (cast)r24 to local0, stripping the cast!
        const char *sym = m_proc->lookupSym(exp);
        if (sym) {
            str << sym;
            return;
        }
    }
#endif

    const Const &c(static_cast<const Const &>(exp));
    const Unary &u(static_cast<const Unary &>(exp));
    const Binary &b(static_cast<const Binary &>(exp));
    const Ternary &t(static_cast<const Ternary &>(exp));

    switch (op) {
    case opIntConst: {
        int K = c.getInt();
        if (uns && K < 0) {
            // An unsigned constant. Use some heuristics
            unsigned rem = (unsigned)K % 100;
            if (rem == 0 || rem == 99 || K > -128) {
                // A multiple of 100, or one less; use 4000000000U style
                char num[16];
                sprintf(num, "%u", K);
                str << num << "U";
            } else {
                // Output it in 0xF0000000 style
                str << "0x" << QString::number(uint32_t(K),16);
            }
        } else {
            if (c.getType() && c.getType()->isChar()) {
                switch (K) {

                case '\a':
                    str << "'\\a'";
                    break;
                case '\b':
                    str << "'\\b'";
                    break;
                case '\f':
                    str << "'\\f'";
                    break;
                case '\n':
                    str << "'\\n'";
                    break;
                case '\r':
                    str << "'\\r'";
                    break;
                case '\t':
                    str << "'\\t'";
                    break;
                case '\v':
                    str << "'\\v'";
                    break;
                case '\\':
                    str << "'\\\\'";
                    break;
                case '\?':
                    str << "'\\?'";
                    break;
                case '\'':
                    str << "'\\''";
                    break;
                case '\"':
                    str << "'\\\"'";
                    break;
                default:
                    str << "'" << (char)K << "'";
                }
            } else {
                // More heuristics
                if (-2048 < K && K < 2048)
                    str << K; // Just a plain vanilla int
                else
                    str << "0x" << QString::number(uint32_t(K),16); // 0x2000 style
            }
        }
        break;
    }
    case opLongConst:
        // str << std::dec << c->getLong() << "LL"; break;
        if ((long long)c.getLong() < -1000LL || (long long)c.getLong() > 1000LL)
            str << "0x" << QString::number(c.getLong(),16) << "LL";
        else
            str << c.getLong() << "LL";
        break;
    case opFltConst: {
        // str.precision(4);     // What to do with precision here? Would be nice to avoid 1.00000 or 0.99999
        QString flt_val = QString::number(c.getFlt(),'g',8);
        if(!flt_val.contains('.'))
            flt_val+='.';
        str << flt_val;
        break;
    }
    case opStrConst:
        // escape string:
        str << "\"" << escapeStr(c.getStr()) << "\"";
        break;
    case opFuncConst:
        str << c.getFuncName();
        break;
    case opAddrOf: {
        const Exp *sub = u.getSubExp1();
        if (sub->isGlobal()) {
            Prog *prog = m_proc->getProg();

            auto con = static_cast<const Const *>(sub->getSubExp1());
            SharedType gt = prog->getGlobalType(con->getStr());
            if (gt && (gt->isArray() || (gt->isPointer() && gt->asPointer()->getPointsTo()->isChar()))) {
                // Special C requirement: don't emit "&" for address of an array or char*
                appendExp(str, *sub, curPrec);
                break;
            }
        }
#ifdef SYMS_IN_BACK_END
        if (sub->isMemOf() && m_proc->lookupSym(sub) == nullptr) { // }
#else
        if (sub->isMemOf()) {
#endif

            // Avoid &*(type*)sub, just emit sub
            appendExp(str, *sub->getSubExp1(), PREC_UNARY);
        } else {
            openParen(str, curPrec, PREC_UNARY);
            str << "&";
            appendExp(str, *sub, PREC_UNARY);
            closeParen(str, curPrec, PREC_UNARY);
        }
        break;
    }
    case opParam:
    case opGlobal:
    case opLocal: {
        auto c1 = dynamic_cast<const Const *>(u.getSubExp1());
        assert(c1 && c1->getOper() == opStrConst);
        str << c1->getStr();
    } break;
    case opEquals: {
        openParen(str, curPrec, PREC_EQUAL);
        appendExp(str, *b.getSubExp1(), PREC_EQUAL);
        str << " == ";
#if 0 // Suspect only for ADHOC TA
            SharedType ty = b.getSubExp1()->getType();
            if (ty && ty->isPointer() && b.getSubExp2()->isIntConst() && ((Const*)b.getSubExp2())->getInt() == 0)
                str << "nullptr";
            else
#endif
        appendExp(str, *b.getSubExp2(), PREC_EQUAL);
        closeParen(str, curPrec, PREC_EQUAL);
    } break;
    case opNotEqual: {
        openParen(str, curPrec, PREC_EQUAL);
        appendExp(str, *b.getSubExp1(), PREC_EQUAL);
        str << " != ";
#if 0 // Suspect only for ADHOC_TA
            SharedType ty = b.getSubExp1()->getType();
            if (ty && ty->isPointer() && b.getSubExp2()->isIntConst() && ((Const*)b.getSubExp2())->getInt() == 0)
                str << "nullptr";
            else
#endif
        appendExp(str, *b.getSubExp2(), PREC_EQUAL);
        closeParen(str, curPrec, PREC_EQUAL);
    } break;
    case opLess:
    case opLessUns:
        openParen(str, curPrec, PREC_REL);
        appendExp(str, *b.getSubExp1(), PREC_REL, op == opLessUns);
        str << " < ";
        appendExp(str, *b.getSubExp2(), PREC_REL, op == opLessUns);
        closeParen(str, curPrec, PREC_REL);
        break;
    case opGtr:
    case opGtrUns:
        openParen(str, curPrec, PREC_REL);
        appendExp(str, *b.getSubExp1(), PREC_REL, op == opGtrUns);
        str << " > ";
        appendExp(str, *b.getSubExp2(), PREC_REL, op == opGtrUns);
        closeParen(str, curPrec, PREC_REL);
        break;
    case opLessEq:
    case opLessEqUns:
        openParen(str, curPrec, PREC_REL);
        appendExp(str, *b.getSubExp1(), PREC_REL, op == opLessEqUns);
        str << " <= ";
        appendExp(str, *b.getSubExp2(), PREC_REL, op == opLessEqUns);
        closeParen(str, curPrec, PREC_REL);
        break;
    case opGtrEq:
    case opGtrEqUns:
        openParen(str, curPrec, PREC_REL);
        appendExp(str, *b.getSubExp1(), PREC_REL, op == opGtrEqUns);
        str << " >= ";
        appendExp(str, *b.getSubExp2(), PREC_REL, op == opGtrEqUns);
        closeParen(str, curPrec, PREC_REL);
        break;
    case opAnd:
        openParen(str, curPrec, PREC_LOG_AND);
        appendExp(str, *b.getSubExp1(), PREC_LOG_AND);
        str << " && ";
        appendExp(str, *b.getSubExp2(), PREC_LOG_AND);
        closeParen(str, curPrec, PREC_LOG_AND);
        break;
    case opOr:
        openParen(str, curPrec, PREC_LOG_OR);
        appendExp(str, *b.getSubExp1(), PREC_LOG_OR);
        str << " || ";
        appendExp(str, *b.getSubExp2(), PREC_LOG_OR);
        closeParen(str, curPrec, PREC_LOG_OR);
        break;
    case opBitAnd:
        openParen(str, curPrec, PREC_BIT_AND);
        appendExp(str, *b.getSubExp1(), PREC_BIT_AND);
        str << " & ";
        if (b.getSubExp2()->getOper() == opIntConst) {
            // print it 0x2000 style
            uint32_t val = uint32_t(((Const *)b.getSubExp2())->getInt());
            QString vanilla = QString("0x")+QString::number(val,16);
            QString negated = QString("~0x")+QString::number(~val,16);
            if(negated.size()<vanilla.size())
                str << negated;
            else
                str << vanilla;
        } else {
            appendExp(str, *b.getSubExp2(), PREC_BIT_AND);
        }
        closeParen(str, curPrec, PREC_BIT_AND);
        break;
    case opBitOr:
        openParen(str, curPrec, PREC_BIT_IOR);
        appendExp(str, *b.getSubExp1(), PREC_BIT_IOR);
        str << " | ";
        appendExp(str, *b.getSubExp2(), PREC_BIT_IOR);
        closeParen(str, curPrec, PREC_BIT_IOR);
        break;
    case opBitXor:
        openParen(str, curPrec, PREC_BIT_XOR);
        appendExp(str, *b.getSubExp1(), PREC_BIT_XOR);
        str << " ^ ";
        appendExp(str, *b.getSubExp2(), PREC_BIT_XOR);
        closeParen(str, curPrec, PREC_BIT_XOR);
        break;
    case opNot:
        openParen(str, curPrec, PREC_UNARY);
        str << " ~";
        appendExp(str, *u.getSubExp1(), PREC_UNARY);
        closeParen(str, curPrec, PREC_UNARY);
        break;
    case opLNot:
        openParen(str, curPrec, PREC_UNARY);
        str << " !";
        appendExp(str, *u.getSubExp1(), PREC_UNARY);
        closeParen(str, curPrec, PREC_UNARY);
        break;
    case opNeg:
    case opFNeg:
        openParen(str, curPrec, PREC_UNARY);
        str << " -";
        appendExp(str, *u.getSubExp1(), PREC_UNARY);
        closeParen(str, curPrec, PREC_UNARY);
        break;
    case opAt: {
        // I guess that most people will find this easier to read
        // s1 >> last & 0xMASK
        openParen(str, curPrec, PREC_BIT_AND);
        appendExp(str, *t.getSubExp1(), PREC_BIT_SHIFT);
        auto first = static_cast<const Const *>(t.getSubExp2());
        auto last = static_cast<const Const *>(t.getSubExp3());
        str << " >> ";
        appendExp(str, *last, PREC_BIT_SHIFT);
        str << " & ";

        unsigned int mask = (1 << (first->getInt() - last->getInt() + 1)) - 1;
        // print 0x3 as 3
        if (mask < 10)
            str << mask;
        else {
            str << "0x" << QString::number(mask,16);
        }
        closeParen(str, curPrec, PREC_BIT_AND);
        break;
    }
    case opPlus:
        openParen(str, curPrec, PREC_ADD);
        appendExp(str, *b.getSubExp1(), PREC_ADD);
        str << " + ";
        appendExp(str, *b.getSubExp2(), PREC_ADD);
        closeParen(str, curPrec, PREC_ADD);
        break;
    case opMinus:
        openParen(str, curPrec, PREC_ADD);
        appendExp(str, *b.getSubExp1(), PREC_ADD);
        str << " - ";
        appendExp(str, *b.getSubExp2(), PREC_ADD);
        closeParen(str, curPrec, PREC_ADD);
        break;
    case opMemOf:
        if (Boomerang::get()->noDecompile) {
            str << "MEMOF(";
            appendExp(str, *u.getSubExp1(), PREC_NONE);
            str << ")";
            break;
        }
        openParen(str, curPrec, PREC_UNARY);
        // annotateMemofs should have added a cast if it was needed
        str << "*";
        appendExp(str, *u.getSubExp1(), PREC_UNARY);
        closeParen(str, curPrec, PREC_UNARY);
        break;
    case opRegOf: {
        // MVE: this can likely go
        LOG_VERBOSE(1) << "WARNING: CHLLCode::appendExp: case opRegOf is deprecated\n";
        if (u.getSubExp1()->getOper() == opTemp) {
            // The great debate: r[tmpb] vs tmpb
            str << "tmp";
            break;
        }
        assert(u.getSubExp1()->getOper() == opIntConst);
        QString n(m_proc->getProg()->getRegName(((Const *)u.getSubExp1())->getInt()));
        if (n.isEmpty()) {
            if (n[0] == '%')
                str << n + 1;
            else
                str << n;
        } else {
            // What is this doing in the back end???
            str << "r[";
            appendExp(str, *u.getSubExp1(), PREC_NONE);
            str << "]";
        }
    } break;
    case opTemp:
        // Should never see this; temps should be mapped to locals now so that they get declared
        LOG_VERBOSE(1) << "WARNING: CHLLCode::appendExp: case opTemp is deprecated\n";
        // Emit the temp name, e.g. "tmp1"
        str << ((Const *)u.getSubExp1())->getStr();
        break;
    case opItof:
        // TODO: MVE: needs work: float/double/long double.
        str << "(float)";
        openParen(str, curPrec, PREC_UNARY);
        appendExp(str, *t.getSubExp3(), PREC_UNARY);
        closeParen(str, curPrec, PREC_UNARY);
        break;
    case opFsize:
        // TODO: needs work!
        if(t.getSubExp3()->isMemOf()) {
            assert(t.getSubExp1()->isIntConst());
            int float_bits =  ((Const *)t.getSubExp1())->getInt();
            if (Boomerang::get()->noDecompile) {
                assert(t.getSubExp1()->isIntConst());
                if (float_bits == 32)
                    str << "FLOAT_MEMOF(";
                else
                    str << "DOUBLE_MEMOF(";
                appendExp(str, *t.getSubExp3()->getSubExp1(), PREC_NONE);
                str << ")";
            } else {
               switch (float_bits) {
                   case 32: str << "*((float *)&"; break;
                   case 64: str << "*((double *)&"; break;
                   case 80: str << "*((long double*)&"; break;
               }
               openParen(str, curPrec, curPrec);
               appendExp(str, *t.getSubExp3(), curPrec);
               closeParen(str, curPrec, curPrec);
               str << ")";
            }
        } 
        else
            appendExp(str, *t.getSubExp3(), curPrec);
        break;
    case opMult:
    case opMults: // FIXME: check types
        openParen(str, curPrec, PREC_MULT);
        appendExp(str, *b.getSubExp1(), PREC_MULT);
        str << " * ";
        appendExp(str, *b.getSubExp2(), PREC_MULT);
        closeParen(str, curPrec, PREC_MULT);
        break;
    case opDiv:
    case opDivs: // FIXME: check types
        openParen(str, curPrec, PREC_MULT);
        appendExp(str, *b.getSubExp1(), PREC_MULT);
        str << " / ";
        appendExp(str, *b.getSubExp2(), PREC_MULT);
        closeParen(str, curPrec, PREC_MULT);
        break;
    case opMod:
    case opMods: // Fixme: check types
        openParen(str, curPrec, PREC_MULT);
        appendExp(str, *b.getSubExp1(), PREC_MULT);
        str << " % ";
        appendExp(str, *b.getSubExp2(), PREC_MULT);
        closeParen(str, curPrec, PREC_MULT);
        break;
    case opShiftL:
        openParen(str, curPrec, PREC_BIT_SHIFT);
        appendExp(str, *b.getSubExp1(), PREC_BIT_SHIFT);
        str << " << ";
        appendExp(str, *b.getSubExp2(), PREC_BIT_SHIFT);
        closeParen(str, curPrec, PREC_BIT_SHIFT);
        break;
    case opShiftR:
    case opShiftRA:
        openParen(str, curPrec, PREC_BIT_SHIFT);
        appendExp(str, *b.getSubExp1(), PREC_BIT_SHIFT);
        str << " >> ";
        appendExp(str, *b.getSubExp2(), PREC_BIT_SHIFT);
        closeParen(str, curPrec, PREC_BIT_SHIFT);
        break;
    case opTern:
        openParen(str, curPrec, PREC_COND);
        str << " (";
        appendExp(str, *t.getSubExp1(), PREC_NONE);
        str << ") ? ";
        appendExp(str, *t.getSubExp2(), PREC_COND);
        str << " : ";
        appendExp(str, *t.getSubExp3(), PREC_COND);
        closeParen(str, curPrec, PREC_COND);
        break;
    case opFPlus:
    case opFPlusd:
    case opFPlusq:
        openParen(str, curPrec, PREC_ADD);
        appendExp(str, *b.getSubExp1(), PREC_ADD);
        str << " + ";
        appendExp(str, *b.getSubExp2(), PREC_ADD);
        closeParen(str, curPrec, PREC_ADD);
        break;
    case opFMinus:
    case opFMinusd:
    case opFMinusq:
        openParen(str, curPrec, PREC_ADD);
        appendExp(str, *b.getSubExp1(), PREC_ADD);
        str << " - ";
        appendExp(str, *b.getSubExp2(), PREC_ADD);
        closeParen(str, curPrec, PREC_ADD);
        break;
    case opFMult:
    case opFMultd:
    case opFMultq:
        openParen(str, curPrec, PREC_MULT);
        appendExp(str, *b.getSubExp1(), PREC_MULT);
        str << " * ";
        appendExp(str, *b.getSubExp2(), PREC_MULT);
        closeParen(str, curPrec, PREC_MULT);
        break;
    case opFDiv:
    case opFDivd:
    case opFDivq:
        openParen(str, curPrec, PREC_MULT);
        appendExp(str, *b.getSubExp1(), PREC_MULT);
        str << " / ";
        appendExp(str, *b.getSubExp2(), PREC_MULT);
        closeParen(str, curPrec, PREC_MULT);
        break;
    case opFround:
        // Note: we need roundf or roundl depending on size of operands
        str << "round("; // Note: math.h required
        appendExp(str, *u.getSubExp1(), PREC_NONE);
        str << ")";
        break;
    case opFtrunc:
        // Note: we need truncf or truncl depending on size of operands
        str << "trunc("; // Note: math.h required
        appendExp(str, *u.getSubExp1(), PREC_NONE);
        str << ")";
        break;
    case opFabs:
        str << "fabs(";
        appendExp(str, *u.getSubExp1(), PREC_NONE);
        str << ")";
        break;
    case opFtoi:
        // Should check size!
        str << "(int)";
        appendExp(str, *u.getSubExp3(), PREC_UNARY);
        break;
    case opRotateL:
        str << "ROTL(";
        appendExp(str, *u.getSubExp1(), PREC_UNARY);
        str << ")";
        break;
    case opRotateR:
        str << "ROTR(";
        appendExp(str, *u.getSubExp1(), PREC_UNARY);
        str << ")";
        break;
    case opRotateLC:
        str << "ROTLC(";
        appendExp(str, *u.getSubExp1(), PREC_UNARY);
        str << ")";
        break;
    case opRotateRC:
        str << "ROTRC(";
        appendExp(str, *u.getSubExp1(), PREC_UNARY);
        str << ")";
        break;
    case opSize: {
        /*SharedType ty = new IntegerType(((Const*)b.getSubExp1())->getInt(), 1);
                            str << "*(" << ty->getCtype(true) << " *)";
                            appendExp(str, new Unary(opAddrOf, b.getSubExp2()), PREC_UNARY);*/
        appendExp(str, *b.getSubExp2(), PREC_UNARY);
    } break;
    case opFMultsd:
    case opFMultdq:
    case opSQRTs:
    case opSQRTd:
    case opSQRTq:
    case opSignExt:
    case opTargetInst:
    case opNamedExp:
    case opGuard:
    case opVar:
    case opArg:
    case opExpand:
    case opCastIntStar:
    case opPostVar:
    case opForceInt:
    case opForceFlt:
    case opFpush:
    case opFpop:
    case opLoge:
    case opExecute:
    case opAFP:
    case opAGP:
        // not implemented
        LOG << "WARNING: CHLLCode::appendExp: case " << exp.getOperName() << " not implemented\n";
        // assert(false);
        break;
    case opFlagCall: {
        assert(b.getSubExp1()->getOper() == opStrConst);
        str << ((Const *)b.getSubExp1())->getStr();
        str << "(";
        auto l = b.getSubExp2();
        for (; l && l->getOper() == opList; l = l->getSubExp2()) {
            appendExp(str, *l->getSubExp1(), PREC_NONE);
            if (l->getSubExp2()->getOper() == opList)
                str << ", ";
        }
        str << ")";
    } break;
    case opList: {
        int elems_on_line = 0; // try to limit line lengths
        const Exp *b2 = &b;
        const Exp *e2 = b.getSubExp2();
        str << "{ ";
        if (b.getSubExp1()->getOper() == opList)
            str << "\n ";
        while (e2->getOper() == opList) {
            appendExp(str, *b2->getSubExp1(), PREC_NONE, uns);
            ++elems_on_line;
            if (b2->getSubExp1()->getOper() == opList ||
                elems_on_line >= 16 /* completely arbitrary, but better than nothing*/) {
                str << ",\n ";
                elems_on_line = 0;
            } else {
                str << ", ";
            }
            b2 = e2;
            e2 = b2->getSubExp2();
        }
        appendExp(str, *b2->getSubExp1(), PREC_NONE, uns);
        str << " }";
    } break;
    case opFlags:
        str << "flags";
        break;
    case opPC:
        str << "pc";
        break;
    case opZfill:
        // MVE: this is a temporary hack... needs cast?
        // sprintf(s, "/* zfill %d->%d */ ",
        //    ((Const*)t.getSubExp1())->getInt(),
        //    ((Const*)t.getSubExp2())->getInt());
        // strcat(str, s); */
        if (t.getSubExp3()->isMemOf() && t.getSubExp1()->isIntConst() && t.getSubExp2()->isIntConst() &&
            ((Const *)t.getSubExp2())->getInt() == 32) {
            unsigned sz = (unsigned)((Const *)t.getSubExp1())->getInt();
            if (sz == 8 || sz == 16) {
                bool close = false;
                str << "*";
#if 0 // Suspect ADHOC TA only
                    SharedType ty = t.getSubExp3()->getSubExp1()->getType();
                    if (ty == nullptr || !ty->isPointer() ||
                            !ty->asPointer()->getPointsTo()->isInteger() ||
                            ty->asPointer()->getPointsTo()->asInteger()->getSize() != sz) {
#endif
                str << "(unsigned ";
                if (sz == 8)
                    str << "char";
                else
                    str << "short";
                str << "*)";
                openParen(str, curPrec, PREC_UNARY);
                close = true;
#if 0 // ADHOC TA as above
                    }
#endif
                appendExp(str, *t.getSubExp3()->getSubExp1(), PREC_UNARY);
                if (close)
                    closeParen(str, curPrec, PREC_UNARY);
                break;
            }
        }
        LOG_VERBOSE(1) << "WARNING: CHLLCode::appendExp: case opZfill is deprecated\n";
        str << "(";
        appendExp(str, *t.getSubExp3(), PREC_NONE);
        str << ")";
        break;

    case opTypedExp: {
#ifdef SYMS_IN_BACK_END
        Exp *b = u.getSubExp1();                  // Base expression
        const char *sym = m_proc->lookupSym(exp); // Check for (cast)sym
        if (sym) {
            str << "(";
            appendType(str, ((TypedExp *)u)->getType());
            str << ")" << sym;
            break;
        }
#endif
        if (u.getSubExp1()->getOper() == opTypedExp &&
            *((const TypedExp &)u).getType() == *((const TypedExp *)u.getSubExp1())->getType()) {
            // We have (type)(type)x: recurse with type(x)
            appendExp(str, *u.getSubExp1(), curPrec);
        } else if (u.getSubExp1()->getOper() == opMemOf) {
// We have (tt)m[x]
#if 0 // ADHOC TA
                PointerType *pty = dynamic_cast<PointerType*>(u.getSubExp1()->getSubExp1()->getType());
#else
            PointerType *pty = nullptr;
#endif
            // pty = T(x)
            SharedType tt = ((TypedExp &)u).getType();
            if (pty != nullptr &&
                (*pty->getPointsTo() == *tt || (tt->isSize() && pty->getPointsTo()->getSize() == tt->getSize())))
                str << "*";
            else {
                if (Boomerang::get()->noDecompile) {
                    if (tt && tt->isFloat()) {
                        if (tt->asFloat()->getSize() == 32)
                            str << "FLOAT_MEMOF";
                        else
                            str << "DOUBLE_MEMOF";
                    } else
                        str << "MEMOF";
                } else {
                    str << "*(";
                    appendType(str, tt);
                    str << "*)";
                }
            }
            openParen(str, curPrec, PREC_UNARY);
            // Emit x
            // was : ((Location*)((TypedExp&)u).getSubExp1())->getSubExp1()
            appendExp(str, *u.getSubExp1()->getSubExp1(), PREC_UNARY);
            closeParen(str, curPrec, PREC_UNARY);
        } else {
            // Check for (tt)b where tt is a pointer; could be &local
            SharedType tt = ((TypedExp &)u).getType();
            if (dynamic_pointer_cast<PointerType>(tt)) {
#ifdef SYMS_IN_BACK_END
                const char *sym = m_proc->lookupSym(Location::memOf(b));
                if (sym) {
                    openParen(str, curPrec, PREC_UNARY);
                    str << "&" << sym;
                    closeParen(str, curPrec, PREC_UNARY);
                    break;
                }
#endif
            }
            // Otherwise, fall back to (tt)b
            str << "(";
            appendType(str, tt);
            str << ")";
            openParen(str, curPrec, PREC_UNARY);
            appendExp(str, *u.getSubExp1(), PREC_UNARY);
            closeParen(str, curPrec, PREC_UNARY);
        }
        break;
    }
    case opSgnEx:
    case opTruncs: {
        const Exp *s = t.getSubExp3();
        int toSize = static_cast<const Const *>(t.getSubExp2())->getInt();
        switch (toSize) {
        case 8:
            str << "(char) ";
            break;
        case 16:
            str << "(short) ";
            break;
        case 64:
            str << "(long long) ";
            break;
        default:
            str << "(int) ";
            break;
        }
        appendExp(str, *s, curPrec);
        break;
    }
    case opTruncu: {
        const Exp *s = t.getSubExp3();
        int toSize = ((const Const *)t.getSubExp2())->getInt();
        switch (toSize) {
        case 8:
            str << "(unsigned char) ";
            break;
        case 16:
            str << "(unsigned short) ";
            break;
        case 64:
            str << "(unsigned long long) ";
            break;
        default:
            str << "(unsigned int) ";
            break;
        }
        appendExp(str, *s, curPrec);
        break;
    }
    case opMachFtr: {
        str << "/* machine specific */ (int) ";
        const Exp *sub = u.getSubExp1();
        assert(sub->isStrConst());
        QString s = ((const Const *)sub)->getStr();
        if (s[0] == '%')  // e.g. %Y
            str << s.mid(1); // Just use Y
        else
            str << s;
        break;
    }
    case opFflags:
        str << "/* Fflags() */ ";
        break;
    case opPow:
        str << "pow(";
        appendExp(str, *b.getSubExp1(), PREC_COMMA);
        str << ", ";
        appendExp(str, *b.getSubExp2(), PREC_COMMA);
        str << ")";
        break;
    case opLog2:
        str << "log2(";
        appendExp(str, *u.getSubExp1(), PREC_NONE);
        str << ")";
        break;
    case opLog10:
        str << "log10(";
        appendExp(str, *u.getSubExp1(), PREC_NONE);
        str << ")";
        break;
    case opSin:
        str << "sin(";
        appendExp(str, *u.getSubExp1(), PREC_NONE);
        str << ")";
        break;
    case opCos:
        str << "cos(";
        appendExp(str, *u.getSubExp1(), PREC_NONE);
        str << ")";
        break;
    case opSqrt:
        str << "sqrt(";
        appendExp(str, *u.getSubExp1(), PREC_NONE);
        str << ")";
        break;
    case opTan:
        str << "tan(";
        appendExp(str, *u.getSubExp1(), PREC_NONE);
        str << ")";
        break;
    case opArcTan:
        str << "atan(";
        appendExp(str, *u.getSubExp1(), PREC_NONE);
        str << ")";
        break;
    case opSubscript:
        appendExp(str, *u.getSubExp1(), curPrec);
        if (VERBOSE)
            LOG << "ERROR: CHLLCode::appendExp: subscript in code generation of proc " << m_proc->getName()
                << " exp (without subscript): " << str.readAll() << "\n";
        // assert(false);
        break;
    case opMemberAccess: {
#if 0 // ADHOC TA
            SharedType ty = b.getSubExp1()->getType();
#else
        SharedType ty = nullptr;
#endif
        if (ty == nullptr) {
            LOG << "type failure: no type for subexp1 of " << &b << "\n";
            // ty = b.getSubExp1()->getType();
            // No idea why this is hitting! - trentw
            // str << "/* type failure */ ";
            // break;
        }
        // Trent: what were you thinking here? Fails for things like
        // local11.lhHeight (where local11 is a register)
        // Mike: it shouldn't!  local11 should have a compound type
        // assert(ty->resolvesToCompound());
        if (b.getSubExp1()->getOper() == opMemOf) {
            appendExp(str, *b.getSubExp1()->getSubExp1(), PREC_PRIM);
            str << "->";
        } else {
            appendExp(str, *b.getSubExp1(), PREC_PRIM);
            str << ".";
        }
        str << ((const Const *)b.getSubExp2())->getStr();
    } break;
    case opArrayIndex:
        openParen(str, curPrec, PREC_PRIM);
        if (b.getSubExp1()->isMemOf()) {
#if 0 // ADHOC TA
                SharedType ty = b.getSubExp1()->getSubExp1()->getType();
#else
            SharedType ty = nullptr;
#endif
            if (ty && ty->resolvesToPointer() && ty->asPointer()->getPointsTo()->resolvesToArray()) {
                // a pointer to an array is automatically dereferenced in C
                appendExp(str, *b.getSubExp1()->getSubExp1(), PREC_PRIM);
            } else
                appendExp(str, *b.getSubExp1(), PREC_PRIM);
        } else
            appendExp(str, *b.getSubExp1(), PREC_PRIM);
        closeParen(str, curPrec, PREC_PRIM);
        str << "[";
        appendExp(str, *b.getSubExp2(), PREC_PRIM);
        str << "]";
        break;
    case opDefineAll:
        str << "<all>";
        if (VERBOSE)
            LOG << "ERROR: should not see opDefineAll in codegen\n";
        break;
    default:
        // others
        OPER other_op = exp.getOper();
        if (other_op >= opZF) {
            // Machine flags; can occasionally be manipulated individually
            // Chop off the "op" part
            str << exp.getOperName() + 2;
            break;
        }
        LOG << "ERROR: CHLLCode::appendExp: case " << exp.getOperName() << " not implemented\n";
        // assert(false);
    }
}

/// Print the type represented by \a typ to \a str.
void CHLLCode::appendType(QTextStream &str, SharedType typ) {
    if (!typ) {
        str << "int"; // Default type for C
        return;
    }
    if (typ->resolvesToPointer() && typ->asPointer()->getPointsTo()->resolvesToArray()) {
        // C programmers prefer to see pointers to arrays as pointers
        // to the first element of the array.  They then use syntactic
        // sugar to access a pointer as if it were an array.
        typ = PointerType::get(typ->asPointer()->getPointsTo()->asArray()->getBaseType());
    }
    str << typ->getCtype(true);
}

/**
 * Print the indented type to \a str.
 */
void CHLLCode::appendTypeIdent(QTextStream &str, SharedType typ, QString ident) {
    if (typ == nullptr)
        return;
    if (typ->isPointer() && typ->asPointer()->getPointsTo()->isArray()) {
        appendType(str, typ->asPointer()->getPointsTo()->asArray()->getBaseType());
        str << " *" << ident;
    } else if (typ->isPointer()) {
        appendType(str, typ);
        str << ident;
    } else if (typ->isArray()) {
        auto a = typ->asArray();
        appendTypeIdent(str, a->getBaseType(), ident);
        str << "[";
        if (!a->isUnbounded())
            str << a->getLength();
        str << "]";
    } else if (typ->isVoid()) {
// Can happen in e.g. twoproc, where really need global parameter and return analysis
#if 1 // TMN: Stop crashes by this workaround
        if (ident.isEmpty()) {
            ident = "unknownVoidType";
        }
#endif
        LOG << "WARNING: CHLLCode::appendTypeIdent: declaring type void as int for " << ident << "\n";
        str << "int " << ident;
    } else {
        appendType(str, typ);
        str << " " << (!ident.isEmpty() ? ident : "<null>");
    }
}

/// Remove all generated code.
void CHLLCode::reset() { lines.clear(); }

/// Adds: while( \a cond) {
void CHLLCode::AddPretestedLoopHeader(int indLevel, Exp *cond) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "while (";
    appendExp(s, *cond, PREC_NONE);
    s << ") {";
    appendLine(tgt);
}

/// Adds: }
void CHLLCode::AddPretestedLoopEnd(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "}";
    appendLine(tgt);
}

/// Adds: for(;;) {
void CHLLCode::AddEndlessLoopHeader(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "for(;;) {";
    appendLine(tgt);
}

/// Adds: }
void CHLLCode::AddEndlessLoopEnd(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "}";
    appendLine(tgt);
}

/// Adds: do {
void CHLLCode::AddPosttestedLoopHeader(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "do {";
    appendLine(tgt);
}

/// Adds: } while (\a cond);
void CHLLCode::AddPosttestedLoopEnd(int indLevel, Exp *cond) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "} while (";
    appendExp(s, *cond, PREC_NONE);
    s << ");";
    appendLine(tgt);
}

/// Adds: switch(\a cond) {
void CHLLCode::AddCaseCondHeader(int indLevel, Exp *cond) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "switch(";
    appendExp(s, *cond, PREC_NONE);
    s << ") {";
    appendLine(tgt);
}

/// Adds: case \a opt :
void CHLLCode::AddCaseCondOption(int indLevel, Exp &opt) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "case ";
    appendExp(s, opt, PREC_NONE);
    s << ":";
    appendLine(tgt);
}

/// Adds: break;
void CHLLCode::AddCaseCondOptionEnd(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "break;";
    appendLine(tgt);
}

/// Adds: default:
void CHLLCode::AddCaseCondElse(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "default:";
    appendLine(tgt);
}

/// Adds: }
void CHLLCode::AddCaseCondEnd(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "}";
    appendLine(tgt);
}

/// Adds: if(\a cond) {
void CHLLCode::AddIfCondHeader(int indLevel, Exp *cond) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "if (";
    appendExp(s, *cond, PREC_NONE);
    s << ") {";
    appendLine(tgt);
}

/// Adds: }
void CHLLCode::AddIfCondEnd(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "}";
    appendLine(tgt);
}

/// Adds: if(\a cond) {
void CHLLCode::AddIfElseCondHeader(int indLevel, Exp *cond) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "if (";
    appendExp(s, *cond, PREC_NONE);
    s << ") {";
    appendLine(tgt);
}

/// Adds: } else {
void CHLLCode::AddIfElseCondOption(int indLevel) {
    QString tgt;

    QTextStream s(&tgt);

    indent(s, indLevel);
    s << "} else {";
    appendLine(tgt);
}

/// Adds: }
void CHLLCode::AddIfElseCondEnd(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "}";
    appendLine(tgt);
}

/// Adds: goto L \em ord
void CHLLCode::AddGoto(int indLevel, int ord) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "goto L" << ord << ";";
    appendLine(tgt);
    usedLabels.insert(ord);
}

/**
 * Removes labels from the code which are not in usedLabels.
 * maxOrd UNUSED
 */
void CHLLCode::RemoveUnusedLabels(int /*maxOrd*/) {
    for (QStringList::iterator it = lines.begin(); it != lines.end();) {
        if (it->startsWith('L') && it->contains(':')) {
            QStringRef sxr = it->midRef(1,it->indexOf(':')-1);
            int n = sxr.toInt();
            if (usedLabels.find(n) == usedLabels.end()) {
                it = lines.erase(it);
                continue;
            }
        }
        it++;
    }
}

/// Adds: continue;
void CHLLCode::AddContinue(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "continue;";
    appendLine(tgt);
}

/// Adds: break;
void CHLLCode::AddBreak(int indLevel) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "break;";
    appendLine(tgt);
}

/// Adds: L \a ord :
void CHLLCode::AddLabel(int /*indLevel*/, int ord) {
    QString tgt;
    QTextStream s(&tgt);
    s << "L" << ord << ":";
    appendLine(tgt);
}

/// Search for the label L \a ord and remove it from the generated code.
void CHLLCode::RemoveLabel(int ord) {
    QString tgt;
    QTextStream s(&tgt);
    s << "L" << ord << ":";
    lines.removeAll(tgt);
}

bool isBareMemof(const Exp &e, UserProc * /*proc*/) {
    if (!e.isMemOf())
        return false;
#ifdef SYMS_IN_BACK_END
    // Check if it maps to a symbol
    const char *sym = proc->lookupSym(e);
    if (sym == nullptr)
        sym = proc->lookupSym(e->getSubExp1());
    return sym == nullptr; // Only a bare memof if it is not a symbol
#else
    return true;
#endif
}

/// Prints an assignment expression.
void CHLLCode::AddAssignmentStatement(int indLevel, Assign *asgn) {
    // Gerard: shouldn't these  3 types of statements be removed earlier?
    if (asgn->getLeft()->getOper() == opPC)
        return; // Never want to see assignments to %PC
    Exp *result;
    if (asgn->getRight()->search(Terminal(opPC), result)) // Gerard: what's this?
        return;
    // ok I want this now
    // if (asgn->getLeft()->isFlags())
    //    return;

    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    SharedType asgnType = asgn->getType();
    Exp *lhs = asgn->getLeft();
    Exp *rhs = asgn->getRight();
    UserProc *proc = asgn->getProc();

    if (*lhs == *rhs)
        return; // never want to see a = a;

    if (Boomerang::get()->noDecompile && isBareMemof(*rhs, proc) && lhs->getOper() == opRegOf &&
        m_proc->getProg()->getFrontEndId() == PLAT_SPARC) {
        // add some fsize hints to rhs
        if (((Const *)lhs->getSubExp1())->getInt() >= 32 && ((Const *)lhs->getSubExp1())->getInt() <= 63)
            rhs = new Ternary(opFsize, new Const(32), new Const(32), rhs);
        else if (((Const *)lhs->getSubExp1())->getInt() >= 64 && ((Const *)lhs->getSubExp1())->getInt() <= 87)
            rhs = new Ternary(opFsize, new Const(64), new Const(64), rhs);
    }

    if (Boomerang::get()->noDecompile && isBareMemof(*lhs, proc)) {
        if (asgnType && asgnType->isFloat()) {
            if (asgnType->asFloat()->getSize() == 32)
                s << "FLOAT_";
            else
                s << "DOUBLE_";
        } else if (rhs->getOper() == opFsize) {
            if (((Const *)rhs->getSubExp2())->getInt() == 32)
                s << "FLOAT_";
            else
                s << "DOUBLE_";
        } else if (rhs->getOper() == opRegOf && m_proc->getProg()->getFrontEndId() == PLAT_SPARC) {
            // yes, this is a hack
            if (((Const *)rhs->getSubExp1())->getInt() >= 32 && ((Const *)rhs->getSubExp1())->getInt() <= 63)
                s << "FLOAT_";
            else if (((Const *)rhs->getSubExp1())->getInt() >= 64 && ((Const *)rhs->getSubExp1())->getInt() <= 87)
                s << "DOUBLE_";
        }

        s << "MEMASSIGN(";
        appendExp(s, *lhs->getSubExp1(), PREC_UNARY);
        s << ", ";
        appendExp(s, *rhs, PREC_UNARY);
        s << ");";
        appendLine(tgt);
        return;
    }

    if (isBareMemof(*lhs, proc) && asgnType && !asgnType->isVoid())
        appendExp(s, TypedExp(asgnType, lhs), PREC_ASSIGN);
    else if (lhs->getOper() == opGlobal && asgn->getType()->isArray())
        appendExp(s, Binary(opArrayIndex, lhs, new Const(0)), PREC_ASSIGN);
    else if (lhs->getOper() == opAt && ((Ternary *)lhs)->getSubExp2()->isIntConst() &&
             ((Ternary *)lhs)->getSubExp3()->isIntConst()) {
        // exp1@[n:m] := rhs -> exp1 = exp1 & mask | rhs << m  where mask = ~((1 << m-n+1)-1)
        Exp *exp1 = ((Ternary *)lhs)->getSubExp1();
        int n = ((Const *)((Ternary *)lhs)->getSubExp2())->getInt();
        int m = ((Const *)((Ternary *)lhs)->getSubExp3())->getInt();
        appendExp(s, *exp1, PREC_ASSIGN);
        s << " = ";
        int mask = ~(((1 << (m - n + 1)) - 1) << m); // MSVC winges without most of these parentheses
        rhs = Binary::get(opBitAnd, exp1,
                          Binary::get(opBitOr, new Const(mask), Binary::get(opShiftL, rhs, new Const(m))));
        rhs = rhs->simplify();
        appendExp(s, *rhs, PREC_ASSIGN);
        s << ";";
        appendLine(tgt);
        return;
    } else
        appendExp(s, *lhs, PREC_ASSIGN); // Ordinary LHS
    if (rhs->getOper() == opPlus && *rhs->getSubExp1() == *lhs) {
        // C has special syntax for this, eg += and ++
        // however it's not always acceptable for assigns to m[] (?)
        if (rhs->getSubExp2()->isIntConst() &&
            (((Const *)rhs->getSubExp2())->getInt() == 1 || (asgn->getType()->isPointer() &&
                                                             asgn->getType()->asPointer()->getPointsTo()->getSize() ==
                                                                 (unsigned)((Const *)rhs->getSubExp2())->getInt() * 8)))
            s << "++";
        else {
            s << " += ";
            appendExp(s, *rhs->getSubExp2(), PREC_ASSIGN);
        }
    } else {
        s << " = ";
        appendExp(s, *rhs, PREC_ASSIGN);
    }
    s << ";";
    appendLine(tgt);
}

/**
 * Adds a call to \a proc.
 *
 * \param indLevel        A string containing spaces to the indentation level.
 * \param proc            The Proc the call is to.
 * \param name            The name the Proc has.
 * \param args            The arguments to the call.
 * \param results        The variable that will receive the return value of the function.
 *
 * \todo                Remove the \a name parameter and use Proc::getName()
 * \todo                Add assingment for when the function returns a struct.
 */
void CHLLCode::AddCallStatement(int indLevel, Function *proc, const QString &name, StatementList &args,
                                StatementList *results) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    if (not results->empty()) {
        // FIXME: Needs changing if more than one real result (return a struct)
        Exp *firstRet = ((Assignment *)*results->begin())->getLeft();
        appendExp(s, *firstRet, PREC_ASSIGN);
        s << " = ";
    }
    s << name << "(";
    StatementList::iterator ss;
    bool first = true;
    int n = 0;
    for (ss = args.begin(); ss != args.end(); ++ss, ++n) {
        if (first)
            first = false;
        else
            s << ", ";
        Assignment *arg_assign = dynamic_cast<Assignment *>(*ss);
        assert(arg_assign!=nullptr);
        SharedType t = arg_assign->getType();
        auto as_arg = arg_assign->getRight();
        auto const_arg = dynamic_cast<const Const *>(as_arg);
        bool ok = true;
        if (t && t->isPointer() && std::static_pointer_cast<PointerType>(t)->getPointsTo()->isFunc() && const_arg->isIntConst()) {
            Function *p = proc->getProg()->findProc(const_arg->getAddr());
            if (p) {
                s << p->getName();
                ok = false;
            }
        }
        if (ok) {
            bool needclose = false;
            if (Boomerang::get()->noDecompile && proc->getSignature()->getParamType(n) &&
                proc->getSignature()->getParamType(n)->isPointer()) {
                s << "ADDR(";
                needclose = true;
            }
            appendExp(s, *as_arg, PREC_COMMA);
            if (needclose)
                s << ")";
        }
    }
    s << ");";
    if (results->size() > 1) {
        first = true;
        s << " /* Warning: also results in ";
        for (ss = ++results->begin(); ss != results->end(); ++ss) {
            if (first)
                first = false;
            else
                s << ", ";
            appendExp(s, *((Assignment *)*ss)->getLeft(), PREC_COMMA);
        }
        s << " */";
    }

    appendLine(tgt);
}

/**
 * Adds an indirect call to \a exp.
 * \see AddCallStatement
 * \param indLevel
 * \param exp
 * \param args
 * \param results UNUSED
 * \todo Add the use of \a results like AddCallStatement.
 */
// Ugh - almost the same as the above, but it needs to take an expression, // not a Proc*
void CHLLCode::AddIndCallStatement(int indLevel, Exp *exp, StatementList &args, StatementList * results) {
    Q_UNUSED(results);
    //    FIXME: Need to use 'results', since we can infer some defines...
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "(*";
    appendExp(s, *exp, PREC_NONE);
    s << ")(";
    QStringList arg_strings;
    QString arg_tgt;
    for (Instruction * ss : args) {
        QTextStream arg_str(&arg_tgt);
        Exp *arg = ((Assign *)ss)->getRight();
        appendExp(arg_str, *arg, PREC_COMMA);
        arg_strings<<arg_tgt;
        arg_tgt.clear();
    }
    s << arg_strings.join(", ") << ");";
    appendLine(tgt);
}

/**
 * Adds a return statement and returns the first expression in \a rets.
 * \todo This should be returning a struct if more than one real return value.
 */
void CHLLCode::AddReturnStatement(int indLevel, StatementList *rets) {
    // FIXME: should be returning a struct of more than one real return */
    // The stack pointer is wanted as a define in calls, and so appears in returns, but needs to be removed here
    StatementList::iterator rr;
    QString tgt;
    QTextStream s(&tgt);
    indent(s, indLevel);
    s << "return";
    size_t n = rets->size();

    if (n == 0 && Boomerang::get()->noDecompile && m_proc->getSignature()->getNumReturns() > 0)
        s << " eax";

    if (n >= 1) {
        s << " ";
        appendExp(s, *((Assign *)*rets->begin())->getRight(), PREC_NONE);
    }
    s << ";";

    if (n > 0) {
        if (n > 1)
            s << " /* WARNING: Also returning: ";
        bool first = true;
        for (rr = ++rets->begin(); rr != rets->end(); ++rr) {
            if (first)
                first = false;
            else
                s << ", ";
            appendExp(s, *((Assign *)*rr)->getLeft(), PREC_NONE);
            s << " := ";
            appendExp(s, *((Assign *)*rr)->getRight(), PREC_NONE);
        }
        if (n > 1)
            s << " */";
    }
    appendLine(tgt);
}

/**
 * Print the start of a function, and also as a comment its address.
 */
void CHLLCode::AddProcStart(UserProc *proc) {
    QString tgt;
    QTextStream s(&tgt);
    s << "// address: 0x" << proc->getNativeAddress();
    appendLine(tgt);
    AddProcDec(proc, true);
}

/// Add a prototype (for forward declaration)
void CHLLCode::AddPrototype(UserProc *proc) { AddProcDec(proc, false); }

/**
 * Print the declaration of a function.
 * \param proc to print
 * \param open False if this is just a prototype and ";" should be printed instead of "{"
 */
void CHLLCode::AddProcDec(UserProc *proc, bool open) {
    QString tgt;
    QTextStream s(&tgt);
    ReturnStatement *returns = proc->getTheReturnStatement();
    SharedType retType;
    if (proc->getSignature()->isForced()) {
        if (proc->getSignature()->getNumReturns() == 0)
            s << "void ";
        else {
            unsigned int n = 0;
            Exp *e = proc->getSignature()->getReturnExp(0);
            if (e->isRegN(Signature::getStackRegister(proc->getProg())))
                n = 1;
            if (n < proc->getSignature()->getNumReturns())
                retType = proc->getSignature()->getReturnType(n);
            if (retType == nullptr)
                s << "void ";
        }
    } else if (returns == nullptr || returns->getNumReturns() == 0) {
        s << "void ";
    } else {
        Assign *firstRet = (Assign *)*returns->begin();
        retType = firstRet->getType();
        if (retType == nullptr || retType->isVoid())
            // There is a real return; make it integer (Remove with AD HOC type analysis)
            retType = IntegerType::get(STD_SIZE, 0);
    }
    if (retType) {
        appendType(s, retType);
        if (!retType->isPointer()) // NOTE: assumes type *proc( style
            s << " ";
    }
    s << proc->getName() << "(";
    StatementList &parameters = proc->getParameters();
    StatementList::iterator pp;

    if (parameters.size() > 10 && open) {
        LOG << "Warning: CHLLCode::AddProcDec: Proc " << proc->getName() << " has " << (int)parameters.size()
            << " parameters\n";
    }

    bool first = true;
    for (pp = parameters.begin(); pp != parameters.end(); ++pp) {
        if (first)
            first = false;
        else
            s << ", ";
        Assignment *as = (Assignment *)*pp;
        Exp *left = as->getLeft();
        SharedType ty = as->getType();
        if (ty == nullptr) {
            if (VERBOSE)
                LOG << "ERROR in CHLLCode::AddProcDec: no type for parameter " << left << "!\n";
            ty = IntegerType::get(STD_SIZE, 0);
        }
        QString name;
        if (left->isParam())
            name = ((Const *)((Location *)left)->getSubExp1())->getStr();
        else {
            LOG << "ERROR: parameter " << left << " is not opParam!\n";
            name = "??";
        }
        if (ty->isPointer() && std::static_pointer_cast<PointerType>(ty)->getPointsTo()->isArray()) {
            // C does this by default when you pass an array, i.e. you pass &array meaning array
            // Replace all m[param] with foo, param with foo, then foo with param
            ty = std::static_pointer_cast<PointerType>(ty)->getPointsTo();
            Exp *foo = Const::get("foo123412341234");
            m_proc->searchAndReplace(*Location::memOf(left, nullptr), foo);
            m_proc->searchAndReplace(*left, foo);
            m_proc->searchAndReplace(*foo, left);
        }
        appendTypeIdent(s, ty, name);
    }
    s << ")";
    if (open)
        s << " {";
    else
        s << ";\n";
    appendLine(tgt);
}

/// Adds: }
void CHLLCode::AddProcEnd() {
    appendLine("}");
    appendLine("");
}

/**
 * Declare a local variable.
 * \param name given to the new local
 * \param type of this local variable
 * \param last true if an empty line should be added.
 */
void CHLLCode::AddLocal(const QString &name, SharedType type, bool last) {
    QString tgt;
    QTextStream s(&tgt);
    indent(s, 1);
    appendTypeIdent(s, type, name);
    const Exp *e = m_proc->expFromSymbol(name);
    if (e) {
        // ? Should never see subscripts in the back end!
        if (e->getOper() == opSubscript && ((RefExp *)e)->isImplicitDef() &&
            (e->getSubExp1()->getOper() == opParam || e->getSubExp1()->getOper() == opGlobal)) {
            s << " = ";
            appendExp(s, *e->getSubExp1(), PREC_NONE);
            s << ";";
        } else {
            s << "; \t\t// ";
            e->print(s);
        }
    } else
        s << ";";
    appendLine(tgt);
    locals[name] = type->clone();
    if (last)
        appendLine("");
}

/**
 * Add the declaration for a global.
 * \param name given name for the global
 * \param type The type of the global
 * \param init The initial value of the global.
 */
void CHLLCode::AddGlobal(const QString &name, SharedType type, Exp *init) {
    QString tgt;
    QTextStream s(&tgt);
    // Check for array types. These are declared differently in C than
    // they are printed
    if (type->isArray()) {
        // Get the component type
        SharedType base = std::static_pointer_cast<ArrayType>(type)->getBaseType();
        appendType(s, base);
        s << " " << name << "[" << std::static_pointer_cast<ArrayType>(type)->getLength() << "]";
    } else if (type->isPointer() && std::static_pointer_cast<PointerType>(type)->getPointsTo()->resolvesToFunc()) {
        // These are even more different to declare than to print. Example:
        // void (void)* global0 = foo__1B;     ->
        // void (*global0)(void) = foo__1B;
        auto pt = std::static_pointer_cast<PointerType>(type);
        std::shared_ptr<FuncType> ft = std::static_pointer_cast<FuncType>(pt->getPointsTo());
        QString ret, param;
        ft->getReturnAndParam(ret, param);
        s << ret << "(*" << name << ")" << param;
    } else {
        appendType(s, type);
        s << " " << name;
    }
    if (init && !init->isNil()) {
        s << " = ";
        SharedType base_type = type->isArray() ? type->asArray()->getBaseType() : type;
        appendExp(s, *init, PREC_ASSIGN, base_type->isInteger() ? !base_type->asInteger()->isSigned() : false);
    }
    s << ";";
    if (type->isSize())
        s << "// " << type->getSize() / 8 << " bytes";
    appendLine(tgt);
}

/// Dump all generated code to \a os.
void CHLLCode::print(QTextStream &os) {
    os << lines.join('\n');
    if (m_proc == nullptr)
        os << '\n';
}

/// Adds one line of comment to the code.
void CHLLCode::AddLineComment(const QString &cmt) {
    appendLine(QString("/* %1*/").arg(cmt));
}

// Private helper functions, to reduce redundant code, and
// have a single place to put a breakpoint on.
void CHLLCode::appendLine(const QString &s) { lines.push_back(s); }
