#pragma once

/*
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

#include <vector>
#include <cassert>
#include <memory>

#include "boomerang/db/Managed.h"
#include "boomerang/type/Type.h"
#include "boomerang/util/Types.h"

class BasicBlock;
class Exp;
using SharedExp = std::shared_ptr<Exp>;
class UserProc;
class Function;
class Type;
class Signature;
class Assign;
class LocationSet;
class CallStatement;
class QTextStream;
class QString;
class ReturnStatement;
class Module;
class Prog;

/**
 * Base class for generating high-level code from statements.
 *
 * This class is provides methods which are generic of procedural
 * languages like C, Pascal, Fortran, etc. Included in the base class
 * is the follow and goto sets which are used during code generation.
 * Concrete implementations of this class provide specific language
 * bindings for a single procedure in the program.
 */
class ICodeGenerator
{
public:
    ICodeGenerator() {}
    virtual ~ICodeGenerator() {}

    /// Generate code for a procedure.
    virtual void generateCode(UserProc* proc) = 0;
    virtual void addPrototype(UserProc *proc) = 0;

    virtual void generateCode(const Prog* prog, QTextStream& os) = 0;
    virtual void generateCode(const Prog* prog, Module *cluster = nullptr, UserProc *proc = nullptr, bool intermixRTL = false) = 0;

    virtual int getIndent() const = 0;

    /*
     * Functions to add new code, pure virtual.
     */

    // pretested loops
    virtual void addPretestedLoopHeader(const SharedExp& cond) = 0;
    virtual void addPretestedLoopEnd() = 0;

    // endless loops
    virtual void addEndlessLoopHeader() = 0;
    virtual void addEndlessLoopEnd()    = 0;

    // post-tested loops
    virtual void addPostTestedLoopHeader() = 0;
    virtual void addPostTestedLoopEnd(const SharedExp& cond) = 0;

    // case conditionals "nways"
    virtual void addCaseCondHeader(const SharedExp& cond) = 0;
    virtual void addCaseCondOption(Exp& opt) = 0;
    virtual void addCaseCondOptionEnd()      = 0;
    virtual void addCaseCondElse()           = 0;
    virtual void addCaseCondEnd()            = 0;

    // if conditions
    virtual void addIfCondHeader(const SharedExp& cond) = 0;
    virtual void addIfCondEnd() = 0;

    // if else conditions
    virtual void addIfElseCondHeader(const SharedExp& cond) = 0;
    virtual void addIfElseCondOption() = 0;
    virtual void addIfElseCondEnd()    = 0;

    // goto, break, continue, etc
    virtual void addGoto(int ord) = 0;
    virtual void addBreak()         = 0;
    virtual void addContinue()      = 0;

    // labels
    virtual void addLabel(int ord) = 0;
    virtual void removeLabel(int ord)            = 0;
    virtual void removeUnusedLabels(int maxOrd)  = 0;

    // sequential statements
    virtual void addAssignmentStatement(Assign *s) = 0;
    virtual void addCallStatement(Function *proc, const QString& name, StatementList& args,
                                  StatementList *results) = 0;
    virtual void addIndCallStatement(const SharedExp& exp, StatementList& args, StatementList *results) = 0;
    virtual void addReturnStatement(StatementList *rets) = 0;

    // procedure related
    virtual void addProcStart(UserProc *proc) = 0;
    virtual void addProcEnd() = 0;
    virtual void addLocal(const QString& name, SharedType type, bool last = false) = 0;
    virtual void addGlobal(const QString& name, SharedType type, const SharedExp& init = nullptr) = 0;

    // comments
    virtual void addLineComment(const QString& cmt) = 0;

    /*
     * output functions, pure virtual.
     */
    virtual void print(QTextStream& os) = 0;
};
