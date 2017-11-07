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


class ExpModifier;
class Assign;
class PhiAssign;
class ImplicitAssign;
class BoolAssign;
class GotoStatement;
class BranchStatement;
class CaseStatement;
class CallStatement;
class ReturnStatement;
class ImpRefStatement;


/**
 * StmtModifier is a class that for all expressions in this statement, makes a modification.
 * The modification is as a result of an ExpModifier; there is a pointer to such an ExpModifier in a StmtModifier.
 * Even the top level of the LHS of assignments are changed. This is useful e.g. when modifiying locations to locals
 * as a result of converting from SSA form, e.g. eax := ebx -> local1 := local2
 * Classes that derive from StmtModifier inherit the code (in the accept member functions) to modify all the expressions
 * in the various types of statement.
 * Because there is nothing specialised about a StmtModifier, it is not an abstract class (can be instantiated).
 */
class StmtModifier
{
public:
    StmtModifier(ExpModifier *em, bool ic = false);

    virtual ~StmtModifier() = default;

    bool ignoreCollector() const { return m_ignoreCol; }

    // This class' visitor functions don't return anything. Maybe we'll need return values at a later stage.
    virtual void visit(Assign * /*stmt*/, bool& recur);
    virtual void visit(PhiAssign * /*stmt*/, bool& recur);
    virtual void visit(ImplicitAssign * /*stmt*/, bool& recur);
    virtual void visit(BoolAssign * /*stmt*/, bool& recur);
    virtual void visit(GotoStatement * /*stmt*/, bool& recur);
    virtual void visit(BranchStatement * /*stmt*/, bool& recur);
    virtual void visit(CaseStatement * /*stmt*/, bool& recur);
    virtual void visit(CallStatement * /*stmt*/, bool& recur);
    virtual void visit(ReturnStatement * /*stmt*/, bool& recur);
    virtual void visit(ImpRefStatement * /*stmt*/, bool& recur);

public:
    ExpModifier *m_mod;  ///< The expression modifier object

protected:
    bool m_ignoreCol;
};


