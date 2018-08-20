#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/visitor/expvisitor/ExpVisitor.h"


class UserProc;
class Prog;


/**
 * Name registers and temporaries.
 * The idea here is to map the default of a register to a symbol with the type of that first use.
 * If the register is not involved in any conflicts, it will use this name by default
 */
class ExpRegMapper : public ExpVisitor
{
public:
    ExpRegMapper(UserProc *proc);
    virtual ~ExpRegMapper() = default;

public:
    /// \copydoc ExpVisitor::preVisit
    bool preVisit(const std::shared_ptr<RefExp> &exp, bool &visitChildren) override;

private:
    UserProc *m_proc; ///< Proc object for storing the symbols
    Prog *m_prog;
};
