#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SSL2ParserDriver.h"


#include "SSL2Parser.hpp"

#include "boomerang/ssl/RTLInstDict.h"
#include "boomerang/ssl/exp/Terminal.h"
#include "boomerang/util/log/Log.h"

#include <QMap>

#include <sstream>


SSL2ParserDriver::SSL2ParserDriver(RTLInstDict *dict)
    : m_dict(dict)
#if defined(DEBUG_SSLPARSER) && DEBUG_SSLPARSER != 0
    , trace_parsing(true)
    , trace_scanning(true)
#else
    , trace_parsing(false)
    , trace_scanning(false)
#endif
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
        { "fsize",   opFsize     },
        { "itof",    opItof      },
        { "ftoi",    opFtoi      },
        { "fround",  opFround    },
        { "truncu",  opTruncu    },
        { "truncs",  opTruncs    },
        { "zfill",   opZfill     },
        { "sgnex",   opSgnEx     },
        { "sin",     opSin       },
        { "cos",     opCos       },
        { "tan",     opTan       },
        { "arctan",  opArcTan    },
        { "log2",    opLog2      },
        { "loge",    opLoge      },
        { "log10",   opLog10     },
        { "execute", opExecute   },
        { "sqrt",    opSqrt      }
    };
    // clang-format on

    if (opMap.contains(s)) {
        return opMap[s];
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
