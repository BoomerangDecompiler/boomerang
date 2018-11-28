

#include "SSL2ParserDriver.h"

#include "SSL2Parser.hpp"

#include "boomerang/ssl/RTLInstDict.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/util/log/Log.h"

#include <QMap>

#include <sstream>


SSL2ParserDriver::SSL2ParserDriver(RTLInstDict *dict)
    : m_dict(dict)
    , trace_parsing(false)
    , trace_scanning(false)
{
}


int SSL2ParserDriver::parse(const std::string &f)
{
    file = f;
    location.initialize(&file);

    if (!scanBegin()) {
        return false;
    }

    yy::parser parser(*this);
    parser.set_debug_level(trace_parsing);

    const int res = parser.parse();
    scanEnd();
    return res;
}


OPER SSL2ParserDriver::strToOper(const QString &s)
{
    // clang-format off
    static QMap<QString, OPER> opMap{
        { "*",      opMult      },
        { "*!",     opMults     },
        { "*f",     opFMult     },
        { "*fsd",   opFMultsd   },
        { "*fdq",   opFMultdq   },
        { "/",      opDiv       },
        { "/!",     opDivs      },
        { "/f",     opFDiv      },
        { "/fs",    opFDiv      },
        { "/fd",    opFDivd     },
        { "/fq",    opFDivq     },
        { "%",      opMod       },
        { "%!",     opMods      }, // no FMod ?
        { "+",      opPlus      },
        { "+f",     opFPlus     },
        { "+fs",    opFPlus     },
        { "+fd",    opFPlusd    },
        { "+fq",    opFPlusq    },
        { "-",      opMinus     },
        { "-f",     opFMinus    },
        { "-fs",    opFMinus    },
        { "-fd",    opFMinusd   },
        { "-fq",    opFMinusq   },
        { "<",      opLess      },
        { "<u",     opLessUns   },
        { "<=",     opLessEq    },
        { "<=u",    opLessEqUns },
        { "<<",     opShiftL    },
        { ">",      opGtr       },
        { ">u",     opGtrUns    },
        { ">=",     opGtrEq     },
        { ">=u",    opGtrEqUns  },
        { ">>",     opShiftR    },
        { ">>A",    opShiftRA   },
        { "rlc",    opRotateLC  },
        { "rrc",    opRotateRC  },
        { "rl",     opRotateL   },
        { "rr",     opRotateR   }
    };
    // clang-format on

    // Could be *, *!, *f, *fsd, *fdq, *f[sdq]
    if (opMap.contains(s)) {
        return opMap[s];
    }

    //
    switch (s[0].toLatin1()) {
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

    case '@': return opAt;

    case '&': return opBitAnd;

    case '|': return opBitOr;

    case '^': return opBitXor;

    default: break;
    }

    LOG_ERROR("Unknown operator %1", s);
    return opWild;
}


OPER strToTerm(const QString &s)
{
    // clang-format off
    static const QMap<QString, OPER> mapping = {
        { "%pc",        opPC     },
        { "%afp",       opAFP    },
        { "%agp",       opAGP    },
        { "%CF",        opCF     },
        { "%ZF",        opZF     },
        { "%OF",        opOF     },
        { "%NF",        opNF     },
        { "%DF",        opDF     },
        { "%SF",        opNF     },
        { "%flags",     opFlags  },
        { "%fflags",    opFflags },
        { "%C3",        opFZF    },
        { "%C0",        opFLF    }
    };
    // clang-format on

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

    for (auto &elem : *le) {
        *cur = Binary::get(opList, elem, end);
        // cur becomes the address of the address of the second subexpression
        // In other words, cur becomes a reference to the second subexp ptr
        // Note that declaring cur as a reference doesn't work (remains a reference to e)
        cur = &(*cur)->refSubExp2();
    }

    return e;
}


static Binary srchExpr(opExpTable, Terminal::get(opWild), Terminal::get(opWild));


/**
 * Expand tables in an RTL and save to dictionary
 * \note    This may generate many entries
 *
 * \param   iname Parser object representing the instruction name
 * \param   params Parser object representing the instruction params
 * \param   o_rtlist Original rtlist object (before expanding)
 * \param   Dict Ref to the dictionary that will contain the results of the parse
 */
bool SSL2ParserDriver::expandTables(const std::shared_ptr<InsNameElem> &iname,
                                    const std::shared_ptr<std::list<QString>> &params,
                                    SharedRTL o_rtlist, RTLInstDict *dict)
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
                    QString tbl    = (e)->access<Const, 1>()->getStr();
                    QString idx    = (e)->access<Const, 2>()->getStr();
                    SharedExp repl = std::static_pointer_cast<ExprTable>(TableDict[tbl])
                                         ->expressions[indexrefmap[idx]->getValue()];
                    s->searchAndReplace(*e, repl);
                }
            }
        }

        if (dict->insert(name, *params, rtl) != 0) {
            LOG_ERROR("Pattern %1 conflics with an earlier declaration of %2.",
                      iname->getInsPattern(), name);
            return false;
        }
    }

    indexrefmap.erase(indexrefmap.begin(), indexrefmap.end());
    return true;
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
SharedExp SSL2ParserDriver::makeSuccessor(SharedExp e)
{
    return Unary::get(opSuccessor, e);
}
