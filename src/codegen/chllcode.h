/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file       chllcode.h
  * \brief   Concrete class for the "C" high level language
  *               This class provides methods which are specific for the C language binding.
  *               I guess this will be the most popular output language unless we do C++.
  ******************************************************************************/

#ifndef _CHLLCODE_H_
#define _CHLLCODE_H_
#include "hllcode.h"
#include <string>
#include <sstream>

class BasicBlock;
class Exp;
class Function;
class Assign;
class LocationSet;

// Operator precedence
/*
Operator Name                Associativity    Operators
Primary scope resolution    left to right    ::
Primary                        left to right    ()    [ ]     .    -> dynamic_cast typeid
Unary                        right to left    ++    --    +  -  !     ~    &  *
                                                                                        (type_name)     sizeof new
delete
C++ Pointer to Member        left to right    .* ->*
Multiplicative                left to right    *  /  %
Additive                    left to right    +  -
Bitwise Shift                left to right    <<    >>
Relational                    left to right    <  >  <=  >=
Equality                    left to right    ==    !=
Bitwise AND                    left to right    &
Bitwise Exclusive OR        left to right    ^
Bitwise Inclusive OR        left to right    |
Logical AND                    left to right    &&
Logical OR                    left to right    ||
Conditional                    right to left    ? :
Assignment                    right to left    =  +=  -=  *=    /=    <<=     >>=  %=
                                                                                        &=    ^=    |=
Comma                        left to right    ,
*/

/// Operator precedence
enum PREC {
    PREC_NONE = 0,  ///< Outer level (no parens required)
    PREC_COMMA,     ///< Comma
    PREC_ASSIGN,    ///< Assignment
    PREC_COND,      ///< Conditional
    PREC_LOG_OR,    ///< Logical OR
    PREC_LOG_AND,   ///< Logical AND
    PREC_BIT_IOR,   ///< Bitwise Inclusive OR
    PREC_BIT_XOR,   ///< Bitwise Exclusive OR
    PREC_BIT_AND,   ///< Bitwise AND
    PREC_EQUAL,     ///< Equality
    PREC_REL,       ///< Relational
    PREC_BIT_SHIFT, ///< Bitwise Shift
    PREC_ADD,       ///< Additive
    PREC_MULT,      ///< Multiplicative
    PREC_PTR_MEM,   ///< C++ Pointer to Member
    PREC_UNARY,     ///< Unary
    PREC_PRIM,      ///< Primary
    PREC_SCOPE      ///< Primary scope resolution
};

/// Outputs C code.
class CHLLCode : public HLLCode {
  private:
    /// The generated code.
    QStringList lines;

    void indent(QTextStream &str, int indLevel);
    void appendExp(QTextStream &str, const Exp &exp, PREC curPrec, bool uns = false);
    void appendType(QTextStream &str, SharedType typ);
    void appendTypeIdent(QTextStream &str, SharedType typ, QString ident);
    /// Adds: (
    void openParen(QTextStream &str, PREC outer, PREC inner) {
        if (inner < outer)
            str << "(";
    }
    /// Adds: )
    void closeParen(QTextStream &str, PREC outer, PREC inner) {
        if (inner < outer)
            str << ")";
    }

    void appendLine(const QString &s);

    /// All locals in a Proc
    std::map<QString, SharedType > locals;

    /// All used goto labels.
    std::set<int> usedLabels;

  public:
    // constructor
    CHLLCode();
    CHLLCode(UserProc *p);

    // destructor
    virtual ~CHLLCode();

    // clear this class, calls the base
    virtual void reset();

    /*
                 * Functions to add new code
                 */

    // pretested loops (cond is optional because it is in the bb [somewhere])
    virtual void AddPretestedLoopHeader(int indLevel, const SharedExp &cond);
    virtual void AddPretestedLoopEnd(int indLevel);

    // endless loops
    virtual void AddEndlessLoopHeader(int indLevel);
    virtual void AddEndlessLoopEnd(int indLevel);

    // posttested loops
    virtual void AddPosttestedLoopHeader(int indLevel);
    virtual void AddPosttestedLoopEnd(int indLevel, const SharedExp &cond);

    // case conditionals "nways"
    virtual void AddCaseCondHeader(int indLevel, const SharedExp &cond);
    virtual void AddCaseCondOption(int indLevel, Exp &opt);
    virtual void AddCaseCondOptionEnd(int indLevel);
    virtual void AddCaseCondElse(int indLevel);
    virtual void AddCaseCondEnd(int indLevel);

    // if conditions
    virtual void AddIfCondHeader(int indLevel, const SharedExp &cond);
    virtual void AddIfCondEnd(int indLevel);

    // if else conditions
    virtual void AddIfElseCondHeader(int indLevel, const SharedExp &cond);
    virtual void AddIfElseCondOption(int indLevel);
    virtual void AddIfElseCondEnd(int indLevel);

    // goto, break, continue, etc
    virtual void AddGoto(int indLevel, int ord);
    virtual void AddContinue(int indLevel);
    virtual void AddBreak(int indLevel);

    // labels
    virtual void AddLabel(int indLevel, int ord);
    virtual void RemoveLabel(int ord);
    virtual void RemoveUnusedLabels(int maxOrd);

    // sequential statements
    virtual void AddAssignmentStatement(int indLevel, Assign *asgn);
    virtual void AddCallStatement(int indLevel, Function *proc, const QString &name, StatementList &args,
                                  StatementList *results);
    virtual void AddIndCallStatement(int indLevel, const SharedExp &exp, StatementList &args, StatementList *results);
    virtual void AddReturnStatement(int indLevel, StatementList *rets);

    // proc related
    virtual void AddProcStart(UserProc *proc);
    virtual void AddProcEnd();
    virtual void AddLocal(const QString &name, SharedType type, bool last = false);
    virtual void AddGlobal(const QString &name, SharedType type, const SharedExp &init = nullptr);
    virtual void AddPrototype(UserProc *proc);

  private:
    void AddProcDec(UserProc *proc, bool open); // Implement AddProcStart and AddPrototype
  public:
    // comments
    virtual void AddLineComment(const QString &cmt);

    /*
                 * output functions
                 */
    virtual void print(QTextStream &os);
};

#endif
