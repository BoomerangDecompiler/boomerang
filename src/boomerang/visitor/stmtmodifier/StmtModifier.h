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


#include "boomerang/core/BoomerangAPI.h"

#include <memory>


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


/**
 * StmtModifier is a class that for all expressions in this statement, makes a modification.
 * The modification is as a result of an ExpModifier; there is a pointer to such an ExpModifier in a
 * StmtModifier. Even the top level of the LHS of assignments are changed. This is useful e.g. when
 * modifiying locations to locals as a result of converting from SSA form, e.g. eax := ebx -> local1
 * := local2 Classes that derive from StmtModifier inherit the code (in the accept member functions)
 * to modify all the expressions in the various types of statement. Because there is nothing
 * specialised about a StmtModifier, it is not an abstract class (can be instantiated).
 *
 * \note This class' visitor functions don't return anything. Maybe we'll need return values at a
 * later stage.
 */
class BOOMERANG_API StmtModifier
{
public:
    StmtModifier(ExpModifier *em, bool ignoreCollector = false);
    virtual ~StmtModifier() = default;

public:
    bool ignoreCollector() const { return m_ignoreCol; }

    /// Modify a statement.
    /// \param[in] stmt Statement to modify
    /// \param[out] visitChildren set to true to visit children of this statement
    virtual void visit(const std::shared_ptr<Assign> &stmt, bool &visitChildren);

    /// \copydoc StmtModifier::visit
    virtual void visit(const std::shared_ptr<PhiAssign> &stmt, bool &visitChildren);

    /// \copydoc StmtModifier::visit
    virtual void visit(const std::shared_ptr<ImplicitAssign> &stmt, bool &visitChildren);

    /// \copydoc StmtModifier::visit
    virtual void visit(const std::shared_ptr<BoolAssign> &stmt, bool &visitChildren);

    /// \copydoc StmtModifier::visit
    virtual void visit(const std::shared_ptr<GotoStatement> &stmt, bool &visitChildren);

    /// \copydoc StmtModifier::visit
    virtual void visit(const std::shared_ptr<BranchStatement> &stmt, bool &visitChildren);

    /// \copydoc StmtModifier::visit
    virtual void visit(const std::shared_ptr<CaseStatement> &stmt, bool &visitChildren);

    /// \copydoc StmtModifier::visit
    virtual void visit(const std::shared_ptr<CallStatement> &stmt, bool &visitChildren);

    /// \copydoc StmtModifier::visit
    virtual void visit(const std::shared_ptr<ReturnStatement> &stmt, bool &visitChildren);

public:
    ExpModifier *m_mod; ///< The expression modifier object

protected:
    bool m_ignoreCol;
};
