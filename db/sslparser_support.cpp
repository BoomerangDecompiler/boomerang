#include "sslparser.h"
#include "sslscanner.h"
#include "config.h"
#ifdef HAVE_LIBGC
#include "gc.h"
#else
#define NO_GARBAGE_COLLECTOR
#endif
#include "types.h"
#include "rtl.h"
#include "table.h"
#include "insnameelem.h"
#include "util.h" // E.g. str()
#include "statement.h"
#include "exp.h"
#include "boomerang.h"

#include <cassert>
#include <sstream>
#include <cstring>

class SSLScanner;

/***************************************************************************/ /**
  * \fn    SSLParser::SSLParser
  * \brief Constructor for an existing stream.
  * \param in - the input stream
  * \param trace - whether or not to debug
  *
  ******************************************************************************/
SSLParser::SSLParser(std::istream &in, bool trace) : sslFile("input"), bFloat(false) {
    theScanner = new SSLScanner(in, trace);
    if (trace)
        yydebug = 1;
    else
        yydebug = 0;
}

/***************************************************************************/ /**
  * \fn     SSLParser::parseExp
  * \brief  Parses an assignment from a string.
  * \param  str - the string
  * \returns an Assignment or nullptr.
  ******************************************************************************/
Instruction *SSLParser::parseExp(const char *str) {
    std::istringstream ss(str);
    SSLParser p(ss, false); // Second arg true for debugging
    RTLInstDict d;
    p.yyparse(d);
    return p.the_asgn;
}

/***************************************************************************/ /**
  * \fn        SSLParser::~SSLParser
  * \brief        Destructor.
  *
  ******************************************************************************/
SSLParser::~SSLParser() {
    std::map<QString, Table *>::iterator loc;
    if (theScanner != nullptr)
        delete theScanner;
    for (loc = TableDict.begin(); loc != TableDict.end(); loc++)
        delete loc->second;
}

/***************************************************************************/ /**
  * \brief        Display an error message and exit.
  * \param        msg - an error message
  *
  ******************************************************************************/
void SSLParser::yyerror(const char *msg) {
    LOG_STREAM() << sslFile << ":" << theScanner->theLine << ": " << msg << '\n';
}

/***************************************************************************/ /**
  * \brief        The scanner driver than returns the next token.
  * \returns             the next token
  ******************************************************************************/
int SSLParser::yylex() {
    int token = theScanner->yylex(yylval);
    return token;
}

/***************************************************************************/ /**
  * \fn      SSLParser::strToOper
  * \brief   Convert a string operator (e.g. "+f") to an OPER (opFPlus)
  * \note    An attempt is made to make this moderately efficient, else we might have a skip chain of string
  *          comparisons
  * \note    This is a member of SSLParser so we can call yyyerror and have line number etc printed out
  * \param   s - pointer to the operator C string
  * \returns An OPER, or -1 if not found (enum opWild)
  ******************************************************************************/
OPER SSLParser::strToOper(const QString &s) {
    static QMap<QString,OPER> opMap {
        {"*",opMult},{"*!",opMults} , {"*f",opFMult},{"*fsd",opFMultsd},{"*fdq",opFMultdq},
        {"/",opDiv},{"/!",opDivs} , {"/f",opFDiv},{"/fs",opFDiv},{"/fd",opFDivd},{"/fq",opFDivq},
        {"%",opMod},{"%!",opMods}, // no FMod ?
        {"+",opPlus},{"+f",opFPlus} , {"+fs",opFPlus},{"+fd",opFPlusd},{"+fq",opFPlusq},
        {"-",opMinus},{"-f",opFMinus} , {"-fs",opFMinus},{"-fd",opFMinusd},{"-fq",opFMinusq},
        {"<",opLess},{"<u",opLessUns} , {"<=",opLessEq},{"<=u",opLessEqUns},{"<<",opShiftL},
        {">",opGtr}, {">u",opGtrUns}, {">=",opGtrEq},{">=u",opGtrEqUns},
        {">>",opShiftR},{">>A",opShiftRA},
        {"rlc",opRotateLC},{"rrc",opRotateRC},{"rl",opRotateL},{"rr",opRotateR}
    };
        // Could be *, *!, *f, *fsd, *fdq, *f[sdq]
    if(opMap.contains(s))
        return opMap[s];
    //
    switch (s[0].toLatin1()) {
    case 'a':
        // and, arctan, addr
        if (s[1].toLatin1() == 'n')
            return opAnd;
        if (s[1].toLatin1() == 'r')
            return opArcTan;
        if (s[1].toLatin1() == 'd')
            return opAddrOf;
        break;
    case 'c':
        // cos
        return opCos;
    case 'e':
        // execute
        return opExecute;
    case 'f':
        // fsize, ftoi, fround NOTE: ftrunc handled separately because it is a unary
        if (s[1].toLatin1() == 's')
            return opFsize;
        if (s[1].toLatin1() == 't')
            return opFtoi;
        if (s[1].toLatin1() == 'r')
            return opFround;
        break;
    case 'i':
        // itof
        return opItof;
    case 'l':
        // log2, log10, loge
        if (s[3].toLatin1() == '2')
            return opLog2;
        if (s[3].toLatin1() == '1')
            return opLog10;
        if (s[3].toLatin1() == 'e')
            return opLoge;
        break;
    case 'o':
        // or
        return opOr;
    case 'p':
        // pow
        return opPow;
    case 's':
        // sgnex, sin, sqrt
        if (s[1].toLatin1() == 'g')
            return opSgnEx;
        if (s[1].toLatin1() == 'i')
            return opSin;
        if (s[1].toLatin1() == 'q')
            return opSqrt;
        break;
    case 't':
        // truncu, truncs, tan
        // 012345
        if (s[1].toLatin1() == 'a')
            return opTan;
        if (s[5].toLatin1() == 'u')
            return opTruncu;
        if (s[5].toLatin1() == 's')
            return opTruncs;
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
        if (s[1].toLatin1() == '=')
            return opNotEqual;
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

OPER strToTerm(const QString &s) {
    static QMap<QString,OPER> mapping = {
        {"%pc",opPC},{"%afp",opAFP},{"%agp",opAGP},{"%CF",opCF},
        {"%ZF",opZF},{"%OF",opOF},{"%NF",opNF},{"%DF",opDF},{"%flags",opFlags},
        {"%fflags",opFflags},
    };
    if(mapping.contains(s))
        return mapping[s];
    return (OPER)0;
}

/***************************************************************************/ /**
  * \brief        Convert a list of actual parameters in the form of a STL list of Exps into one expression
  *                      (using opList)
  * \note The expressions in the list are not cloned; they are simply copied to the new opList
  * \param le  the list of expressions
  * \returns The opList Expression
  ******************************************************************************/
Exp *listExpToExp(std::list<Exp *> *le) {
    Exp *e;
    Exp **cur = &e;
    Exp *end = new Terminal(opNil); // Terminate the chain
    for (auto &elem : *le) {
        *cur = Binary::get(opList, elem, end);
        // cur becomes the address of the address of the second subexpression
        // In other words, cur becomes a reference to the second subexp ptr
        // Note that declaring cur as a reference doesn't work (remains a reference to e)
        cur = &(*cur)->refSubExp2();
    }
    return e;
}

/***************************************************************************/ /**
  *
  * \brief   Convert a list of formal parameters in the form of a STL list of strings into one expression
  *          (using opList)
  * \param   ls - the list of strings
  * \returns The opList expression
  ******************************************************************************/
Exp *listStrToExp(std::list<QString> *ls) {
    Exp *e;
    Exp **cur = &e;
    Exp *end = new Terminal(opNil); // Terminate the chain
    for (auto &l : *ls) {
        *cur = Binary::get(opList, new Location(opParam, Const::get(l), nullptr), end);
        cur = &(*cur)->refSubExp2();
    }
    *cur = new Terminal(opNil); // Terminate the chain
    return e;
}

static Binary srchExpr(opExpTable, Terminal::get(opWild), Terminal::get(opWild));
static Ternary srchOp(opOpTable, Terminal::get(opWild), Terminal::get(opWild), Terminal::get(opWild));
void init_sslparser() {
#ifndef NO_GARBAGE_COLLECTOR
    static Exp **gc_pointers = (Exp **)GC_MALLOC_UNCOLLECTABLE(2 * sizeof(Exp *));
    gc_pointers[0] = srchExpr;
    gc_pointers[1] = srchOp;
#endif
}

/***************************************************************************/ /**
  *
  * \brief   Expand tables in an RTL and save to dictionary
  * \note    This may generate many entries
  * \param   iname Parser object representing the instruction name
  * \param   params Parser object representing the instruction params
  * \param   o_rtlist Original rtlist object (before expanding)
  * \param   Dict Ref to the dictionary that will contain the results of the parse
  ******************************************************************************/
void SSLParser::expandTables(const std::shared_ptr<InsNameElem> &iname, std::list<QString> *params, SharedRTL o_rtlist, RTLInstDict &Dict) {
    int i, m;
    QString nam;
    m = iname->ninstructions();
    // Expand the tables (if any) in this instruction
    for (i = 0, iname->reset(); i < m; i++, iname->increment()) {
        nam = iname->getinstruction();
        // Need to make substitutions to a copy of the RTL
        RTL rtl = *o_rtlist; // deep copy of contents
        for (Instruction *s : rtl) {
            std::list<Exp *> le;
            // Expression tables
            assert(s->getKind() == STMT_ASSIGN);
            if (((Assign *)s)->searchAll(srchExpr, le)) {
                std::list<Exp *>::iterator it;
                for (it = le.begin(); it != le.end(); it++) {
                    QString tbl = ((Const *)(*it)->getSubExp1())->getStr();
                    QString idx = ((Const *)(*it)->getSubExp2())->getStr();
                    Exp *repl = ((ExprTable *)(TableDict[tbl]))->expressions[indexrefmap[idx]->getvalue()];
                    s->searchAndReplace(**it, repl);
                }
            }
            // Operator tables
            Exp *res;
            while (s->search(srchOp, res)) {
                Ternary *t;
                if (res->getOper() == opTypedExp)
                    t = (Ternary *)res->getSubExp1();
                else
                    t = (Ternary *)res;
                assert(t->getOper() == opOpTable);
                // The ternary opOpTable has a table and index name as strings, then a list of 2 expressions
                // (and we want to replace it with e1 OP e2)
                QString tbl = ((Const *)t->getSubExp1())->getStr();
                QString idx = ((Const *)t->getSubExp2())->getStr();
                // The expressions to operate on are in the list
                Binary *b = (Binary *)t->getSubExp3();
                assert(b->getOper() == opList);
                Exp *e1 = b->getSubExp1();
                Exp *e2 = b->getSubExp2(); // This should be an opList too
                assert(b->getOper() == opList);
                e2 = e2->getSubExp1();
                QString ops = ((OpTable *)(TableDict[tbl]))->Records[indexrefmap[idx]->getvalue()];
                Exp *repl = Binary::get(strToOper(ops), e1->clone(), e2->clone()); // FIXME!
                s->searchAndReplace(*res, repl);
            }
        }

        if (Dict.appendToDict(nam, *params, rtl) != 0) {
            QString errmsg;
            QTextStream o(&errmsg);
            o << "Pattern " << iname->getinspattern() << " conflicts with an earlier declaration of " << nam << ".\n";
            yyerror(qPrintable(errmsg));
        }
    }
    indexrefmap.erase(indexrefmap.begin(), indexrefmap.end());
}

/***************************************************************************/ /**
  * \brief        Make the successor of the given expression, e.g. given r[2], return succ( r[2] )
  *              (using opSuccessor)
  *          We can't do the successor operation here, because the parameters are not yet instantiated
  *          (still of the form param(rd)). Actual successor done in Exp::fixSuccessor()
  * \note            The given expression should be of the form    r[const]
  * \note            The parameter expresion is copied (not cloned) in the result
  * \param      e  The expression to find the successor of
  * \returns             The modified expression
  ******************************************************************************/
Exp *SSLParser::makeSuccessor(Exp *e) { return new Unary(opSuccessor, e); }
