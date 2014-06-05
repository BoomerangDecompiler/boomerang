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

#include <cassert>
#include <sstream>
#include <cstring>

class SSLScanner;

/***************************************************************************/ /**
  * \fn        SSLParser::SSLParser
  * \brief        Constructor for an existing stream.
  * \param        The stream, whether or not to debug
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
  * \fn        SSLParser::parseExp
  * \brief        Parses an assignment from a string.
  * \param        the string
  * \returns             an Assignment or nullptr.
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
    std::map<std::string, Table *>::iterator loc;
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
  * \fn        SSLParser::strToOper
  * \brief        Convert a string operator (e.g. "+f") to an OPER (opFPlus)
  * \note            An attempt is made to make this moderately efficient, else we might have a skip chain of string
  *                    comparisons
  * \note            This is a member of SSLParser so we can call yyyerror and have line number etc printed out
  * \param        s: pointer to the operator C string
  * \returns             An OPER, or -1 if not found (enum opWild)
  ******************************************************************************/
OPER SSLParser::strToOper(const char *s) {
    switch (s[0]) {
    case '*':
        // Could be *, *!, *f, *fsd, *fdq, *f[sdq]
        switch (s[1]) {
        case '\0':
            return opMult;
        case '!':
            return opMults;
        case 'f':
            if ((s[2] == 's') && (s[3] == 'd'))
                return opFMultsd;
            if ((s[2] == 'd') && (s[3] == 'q'))
                return opFMultdq;
            return opFMult;
        default:
            break;
        }
        break;
    case '/':
        // Could be /, /!, /f, /f[sdq]
        switch (s[1]) {
        case '\0':
            return opDiv;
        case '!':
            return opDivs;
        case 'f':
            return opFDiv;
        default:
            break;
        }
        break;
    case '%':
        // Could be %, %!
        switch (s[1]) {
        case '\0':
            return opMod;
        case '!':
            return opMods;
        default:
            break;
        }
        break;
    case '+':
        // Could be +, +f, +f[sdq]
        switch (s[1]) {
        case '\0':
            return opPlus;
        case 'f':
            return opFPlus;
        default:
            break;
        }
        break;
    case '-':
        // Could be -, -f, -f[sdq]
        switch (s[1]) {
        case '\0':
            return opMinus;
        case 'f':
            return opFMinus;
        default:
            break;
        }
        break;
    case 'a':
        // and, arctan, addr
        if (s[1] == 'n')
            return opAnd;
        if (s[1] == 'r')
            return opArcTan;
        if (s[1] == 'd')
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
        if (s[1] == 's')
            return opFsize;
        if (s[1] == 't')
            return opFtoi;
        if (s[1] == 'r')
            return opFround;
        break;
    case 'i':
        // itof
        return opItof;
    case 'l':
        // log2, log10, loge
        if (s[3] == '2')
            return opLog2;
        if (s[3] == '1')
            return opLog10;
        if (s[3] == 'e')
            return opLoge;
        break;
    case 'o':
        // or
        return opOr;
    case 'p':
        // pow
        return opPow;
    case 'r':
        // rlc, rrc, rl, rr
        if (s[1] == 'l') {
            if (s[2] == 'c')
                return opRotateLC;
            return opRotateL;
        } else if (s[1] == 'r') {
            if (s[2] == 'c')
                return opRotateRC;
            return opRotateR;
        }
        break;
    case 's':
        // sgnex, sin, sqrt
        if (s[1] == 'g')
            return opSgnEx;
        if (s[1] == 'i')
            return opSin;
        if (s[1] == 'q')
            return opSqrt;
        break;
    case 't':
        // truncu, truncs, tan
        // 012345
        if (s[1] == 'a')
            return opTan;
        if (s[5] == 'u')
            return opTruncu;
        if (s[5] == 's')
            return opTruncs;
        break;
    case 'z':
        // zfill
        return opZfill;

    case '>':
        // >, >u, >=, >=u, >>, >>A
        switch (s[1]) {
        case '\0':
            return opGtr;
        case 'u':
            return opGtrUns;
        case '=':
            if (s[2] == '\0')
                return opGtrEq;
            return opGtrEqUns;
        case '>':
            if (s[2] == '\0')
                return opShiftR;
            return opShiftRA;
        default:
            break;
        }
        break;
    case '<':
        // <, <u, <=, <=u, <<
        switch (s[1]) {
        case '\0':
            return opLess;
        case 'u':
            return opLessUns;
        case '=':
            if (s[2] == '\0')
                return opLessEq;
            return opLessEqUns;
        case '<':
            return opShiftL;
        default:
            break;
        }
        break;
    case '=':
        // =
        return opEquals;
    case '!':
        // !
        return opSgnEx;
        break;
    case '~':
        // ~=, ~
        if (s[1] == '=')
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
    std::ostringstream ost;
    ost << "Unknown operator " << s << '\n';
    yyerror(STR(ost));
    return opWild;
}

OPER strToTerm(char *s) {
    // s could be %pc, %afp, %agp, %CF, %ZF, %OF, %NF, %DF, %flags, %fflags
    if (s[2] == 'F') {
        if (s[1] <= 'N') {
            if (s[1] == 'C')
                return opCF;
            if (s[1] == 'N')
                return opNF;
            return opDF;
        } else {
            if (s[1] == 'O')
                return opOF;
            return opZF;
        }
    }
    if (s[1] == 'p')
        return opPC;
    if (s[1] == 'a') {
        if (s[2] == 'f')
            return opAFP;
        if (s[2] == 'g')
            return opAGP;
    } else if (s[1] == 'f') {
        if (s[2] == 'l')
            return opFlags;
        if (s[2] == 'f')
            return opFflags;
    }
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
  * \brief        Convert a list of formal parameters in the form of a STL list of strings into one expression
  *                      (using opList)
  * \param        ls - the list of strings
  * \returns             The opList expression
  ******************************************************************************/
Exp *listStrToExp(std::list<std::string> *ls) {
    Exp *e;
    Exp **cur = &e;
    Exp *end = new Terminal(opNil); // Terminate the chain
    for (auto &l : *ls) {
        *cur = Binary::get(opList, new Location(opParam, new Const(strdup((l).c_str())), nullptr), end);
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
void SSLParser::expandTables(InsNameElem *iname, std::list<std::string> *params, RTL *o_rtlist, RTLInstDict &Dict) {
    int i, m;
    std::string nam;
    std::ostringstream o;
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
                    const char *tbl = ((Const *)((Binary *)*it)->getSubExp1())->getStr();
                    const char *idx = ((Const *)((Binary *)*it)->getSubExp2())->getStr();
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
                const char *tbl = ((Const *)t->getSubExp1())->getStr();
                const char *idx = ((Const *)t->getSubExp2())->getStr();
                // The expressions to operate on are in the list
                Binary *b = (Binary *)t->getSubExp3();
                assert(b->getOper() == opList);
                Exp *e1 = b->getSubExp1();
                Exp *e2 = b->getSubExp2(); // This should be an opList too
                assert(b->getOper() == opList);
                e2 = ((Binary *)e2)->getSubExp1();
                const char *ops = ((OpTable *)(TableDict[tbl]))->Records[indexrefmap[idx]->getvalue()].c_str();
                Exp *repl = Binary::get(strToOper(ops), e1->clone(), e2->clone()); // FIXME!
                s->searchAndReplace(*res, repl);
            }
        }

        if (Dict.appendToDict(nam, *params, rtl) != 0) {
            o << "Pattern " << iname->getinspattern() << " conflicts with an earlier declaration of " << nam << ".\n";
            yyerror(STR(o));
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
