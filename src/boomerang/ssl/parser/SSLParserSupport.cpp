#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SSLParser.h"


#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/ssl/exp/Ternary.h"
#include "boomerang/ssl/parser/InsNameElem.h"
#include "boomerang/ssl/parser/SSLScanner.h"
#include "boomerang/ssl/parser/Table.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/statements/Assign.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/log/Log.h"
#include "boomerang/util/Util.h"

#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>


class SSLScanner;

/**
 * \param in - the input stream
 * \param trace - whether or not to debug
 *
 */
SSLParser::SSLParser(std::istream& in, bool trace)
    : sslFile("input")
    , floatRegister(false)
{
    m_fin      = nullptr;
    theScanner = new SSLScanner(in, trace);

    if (trace) {
        yydebug = 1;
    }
    else {
        yydebug = 0;
    }
}


/**
 * Parses an assignment from a string.
 * \param  str - the string
 * \returns an Assignment or nullptr.
 */
Statement *SSLParser::parseExp(const char *str, bool verboseOutput)
{
    std::istringstream ss(str);
    SSLParser          p(ss, false); // Second arg true for debugging
    RTLInstDict        d(verboseOutput);
    p.yyparse(d);
    return p.the_asgn;
}


SSLParser::~SSLParser()
{
    std::map<QString, Table *>::iterator loc;

    if (theScanner != nullptr) {
        delete theScanner;
    }

    TableDict.clear();
    delete m_fin;
}


/// Display an error message if the parser encounters an error.
void SSLParser::yyerror(const char *msg)
{
    LOG_ERROR("%1: %2: %3", sslFile, theScanner->theLine, msg);
}


/// \returns the next token
int SSLParser::yylex()
{
    return theScanner->yylex(yylval);
}


/**
 * Convert a string operator (e.g. "+f") to an OPER (opFPlus)
 * \note    An attempt is made to make this moderately efficient,
 *          else we might have a skip chain of string comparisons
 * \note    This is a member of SSLParser so we can call yyerror
 *          and have line number etc printed out
 * \param   s pointer to the operator C string
 * \returns An OPER, or -1 if not found (enum opWild)
 */
OPER SSLParser::strToOper(const QString& s)
{
    static QMap<QString, OPER> opMap {
        {
            "*", opMult
        }, {
            "*!", opMults
        }, {
            "*f", opFMult
        }, {
            "*fsd", opFMultsd
        }, {
            "*fdq", opFMultdq
        },
        {
            "/", opDiv
        }, {
            "/!", opDivs
        }, {
            "/f", opFDiv
        }, {
            "/fs", opFDiv
        }, {
            "/fd", opFDivd
        }, {
            "/fq", opFDivq
        },
        {
            "%", opMod
        }, {
            "%!", opMods
        },                         // no FMod ?
        {
            "+", opPlus
        }, {
            "+f", opFPlus
        }, {
            "+fs", opFPlus
        }, {
            "+fd", opFPlusd
        }, {
            "+fq", opFPlusq
        },
        {
            "-", opMinus
        }, {
            "-f", opFMinus
        }, {
            "-fs", opFMinus
        }, {
            "-fd", opFMinusd
        }, {
            "-fq", opFMinusq
        },
        {
            "<", opLess
        }, {
            "<u", opLessUns
        }, {
            "<=", opLessEq
        }, {
            "<=u", opLessEqUns
        }, {
            "<<", opShiftL
        },
        {
            ">", opGtr
        }, {
            ">u", opGtrUns
        }, {
            ">=", opGtrEq
        }, {
            ">=u", opGtrEqUns
        },
        {
            ">>", opShiftR
        }, {
            ">>A", opShiftRA
        },
        {
            "rlc", opRotateLC
        }, {
            "rrc", opRotateRC
        }, {
            "rl", opRotateL
        }, {
            "rr", opRotateR
        }
    };

    // Could be *, *!, *f, *fsd, *fdq, *f[sdq]
    if (opMap.contains(s)) {
        return opMap[s];
    }

    //
    switch (s[0].toLatin1())
    {
    case 'a':

        // and, arctan, addr
        if (s[1].toLatin1() == 'n') {
            return opAnd;
        }

        if (s[1].toLatin1() == 'r') {
            return opArcTan;
        }

        if (s[1].toLatin1() == 'd') {
            return opAddrOf;
        }

        break;

    case 'c':
        // cos
        return opCos;

    case 'e':
        // execute
        return opExecute;

    case 'f':

        // fsize, ftoi, fround NOTE: ftrunc handled separately because it is a unary
        if (s[1].toLatin1() == 's') {
            return opFsize;
        }

        if (s[1].toLatin1() == 't') {
            return opFtoi;
        }

        if (s[1].toLatin1() == 'r') {
            return opFround;
        }

        break;

    case 'i':
        // itof
        return opItof;

    case 'l':

        // log2, log10, loge
        if (s[3].toLatin1() == '2') {
            return opLog2;
        }

        if (s[3].toLatin1() == '1') {
            return opLog10;
        }

        if (s[3].toLatin1() == 'e') {
            return opLoge;
        }

        break;

    case 'o':
        // or
        return opOr;

    case 'p':
        // pow
        return opPow;

    case 's':

        // sgnex, sin, sqrt
        if (s[1].toLatin1() == 'g') {
            return opSgnEx;
        }

        if (s[1].toLatin1() == 'i') {
            return opSin;
        }

        if (s[1].toLatin1() == 'q') {
            return opSqrt;
        }

        break;

    case 't':

        // truncu, truncs, tan
        // 012345
        if (s[1].toLatin1() == 'a') {
            return opTan;
        }

        if (s[5].toLatin1() == 'u') {
            return opTruncu;
        }

        if (s[5].toLatin1() == 's') {
            return opTruncs;
        }

        break;

    case 'z':
        // zfill
        return opZfill;

    case '=':
        // =
        return opEquals;

    case '!':
        // !
        return opSgnEx;

        break;

    case '~':

        // ~=, ~
        if (s[1].toLatin1() == '=') {
            return opNotEqual;
        }

        return opNot; // Bit inversion

    case '@':
        return opAt;

    case '&':
        return opBitAnd;

    case '|':
        return opBitOr;

    case '^':
        return opBitXor;

    default:
        break;
    }

    yyerror(qPrintable(QString("Unknown operator %1\n").arg(s)));
    return opWild;
}


OPER strToTerm(const QString& s)
{
    static QMap<QString, OPER> mapping =
    {
        { "%pc",     opPC     }, { "%afp", opAFP }, { "%agp", opAGP }, { "%CF", opCF },
        { "%ZF",     opZF     }, { "%OF",  opOF  }, { "%NF",  opNF  }, { "%DF", opDF },{ "%flags", opFlags },
        { "%fflags", opFflags },
    };

    if (mapping.contains(s)) {
        return mapping[s];
    }

    return (OPER)0;
}


/**
 * Convert a list of actual parameters in the form of a STL list of Exps
 * into one expression (using opList)
 * \note The expressions in the list are not cloned;
 *       they are simply copied to the new opList
 *
 * \param le  the list of expressions
 * \returns The opList Expression
 */
SharedExp listExpToExp(std::list<SharedExp> *le)
{
    SharedExp e;
    SharedExp *cur = &e;
    SharedExp end  = Terminal::get(opNil); // Terminate the chain

    for (auto& elem : *le) {
        *cur = Binary::get(opList, elem, end);
        // cur becomes the address of the address of the second subexpression
        // In other words, cur becomes a reference to the second subexp ptr
        // Note that declaring cur as a reference doesn't work (remains a reference to e)
        cur = &(*cur)->refSubExp2();
    }

    return e;
}


/**
 * Convert a list of formal parameters in the form of a STL list of strings
 * into one expression (using opList)
 * \param   ls - the list of strings
 * \returns The opList expression
 */
SharedExp listStrToExp(std::list<QString> *ls)
{
    SharedExp e;
    SharedExp *cur = &e;
    SharedExp end  = Terminal::get(opNil); // Terminate the chain

    for (auto& l : *ls) {
        *cur = Binary::get(opList, Location::get(opParam, Const::get(l), nullptr), end);
        cur  = &(*cur)->refSubExp2();
    }

    *cur = Terminal::get(opNil); // Terminate the chain
    return e;
}


static Binary  srchExpr(opExpTable, Terminal::get(opWild), Terminal::get(opWild));
static Ternary srchOp(opOpTable, Terminal::get(opWild), Terminal::get(opWild), Terminal::get(opWild));


/**
 * Expand tables in an RTL and save to dictionary
 * \note    This may generate many entries
 *
 * \param   iname Parser object representing the instruction name
 * \param   params Parser object representing the instruction params
 * \param   o_rtlist Original rtlist object (before expanding)
 * \param   Dict Ref to the dictionary that will contain the results of the parse
 */
void SSLParser::expandTables(const std::shared_ptr<InsNameElem>& iname, std::list<QString> *params, SharedRTL o_rtlist, RTLInstDict& Dict)
{
    const int m = iname->getNumInstructions();
    iname->reset();

    // Expand the tables (if any) in this instruction
    for (int i = 0; i < m; i++, iname->increment()) {
        QString name = iname->getInstruction();

        // Need to make substitutions to a copy of the RTL
        RTL rtl(*o_rtlist); // deep copy of contents

        for (Statement *s : rtl) {
            std::list<SharedExp> le;
            // Expression tables
            assert(s->getKind() == StmtType::Assign);

            if (((Assign *)s)->searchAll(srchExpr, le)) {
                for (SharedExp e : le) {
                    QString   tbl  = (e)->access<Const, 1>()->getStr();
                    QString   idx  = (e)->access<Const, 2>()->getStr();
                    SharedExp repl = ((ExprTable *)TableDict[tbl].get())->expressions[indexrefmap[idx]->getValue()];
                    s->searchAndReplace(*e, repl);
                }
            }

            // Operator tables
            SharedExp res;

            while (s->search(srchOp, res)) {
                std::shared_ptr<Ternary> t;

                if (res->getOper() == opTypedExp) {
                    t = res->access<Ternary, 1>();
                }
                else {
                    t = res->access<Ternary>();
                }

                assert(t->getOper() == opOpTable);
                // The ternary opOpTable has a table and index name as strings, then a list of 2 expressions
                // (and we want to replace it with e1 OP e2)
                QString tbl = t->access<Const, 1>()->getStr();
                QString idx = t->access<Const, 2>()->getStr();
                // The expressions to operate on are in the list
                auto b = t->access<Binary, 3>();
                assert(b->getOper() == opList);
                SharedExp e1 = b->getSubExp1();
                SharedExp e2 = b->getSubExp2(); // This should be an opList too
                assert(b->getOper() == opList);
                e2 = e2->getSubExp1();
                QString   ops  = ((OpTable *)TableDict[tbl].get())->getRecords()[indexrefmap[idx]->getValue()];
                SharedExp repl = Binary::get(strToOper(ops), e1->clone(), e2->clone()); // FIXME!
                s->searchAndReplace(*res, repl);
            }
        }

        if (Dict.insert(name, *params, rtl) != 0) {
            QString     errmsg;
            OStream o(&errmsg);
            o << "Pattern " << iname->getInsPattern() << " conflicts with an earlier declaration of " << name << ".\n";
            yyerror(qPrintable(errmsg));
        }
    }

    indexrefmap.erase(indexrefmap.begin(), indexrefmap.end());
}


/**
 * Make the successor of the given expression, e.g. given r[2], return succ( r[2] )
 * (using opSuccessor).
 * We can't do the successor operation here, because the parameters
 * are not yet instantiated (still of the form param(rd)).
 * Actual successor done in Exp::fixSuccessor()
 *
 * \note       The given expression should be of the form    r[const]
 * \note       The parameter expresion is copied (not cloned) in the result
 * \param      e  The expression to find the successor of
 * \returns    The modified expression
 */
SharedExp SSLParser::makeSuccessor(SharedExp e)
{
    return Unary::get(opSuccessor, e);
}
