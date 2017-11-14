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


#include "boomerang/db/statements/TypingStatement.h"


/**
 * Assignment is an abstract subclass of TypingStatement, holding a location
 */
class Assignment : public TypingStatement
{
public:
    Assignment(SharedExp lhs);
    Assignment(SharedType ty, SharedExp lhs);
    virtual ~Assignment() override;

    /// We also want operator< for assignments. For example, we want ReturnStatement
    /// to contain a set of (pointers to) Assignments, so we can automatically
    /// make sure that existing assignments are not duplicated.
    /// Assume that we won't want sets of assignments differing by anything other than LHSs
    bool operator<(const Assignment& o) { return m_lhs < o.m_lhs; }

    /// \copydoc Assignment::print
    virtual void print(QTextStream& os, bool html = false) const override;

    /// Like print, but print without statement number
    virtual void printCompact(QTextStream& os, bool html = false) const = 0;

    /// \copydoc Statement::getTypeFor
    virtual SharedType getTypeFor(SharedExp e) const override;

    /// \copydoc Statement::setTypeFor
    virtual void setTypeFor(SharedExp e, SharedType ty) override;

    /// \copydoc Statement::usesExp
    /// \internal PhiAssign and ImplicitAssign don't override
    virtual bool usesExp(const Exp& e) const override;

    /// \copydoc Statement::isDefinition
    virtual bool isDefinition() const override { return true; }

    /// \copydoc Statement::getDefinitions
    virtual void getDefinitions(LocationSet& defs) const override;

    /// \copydoc Statement::definesLoc
    virtual bool definesLoc(SharedExp loc) const override;

    /// \returns the expression defining the left hand side of the assignment
    virtual SharedExp getLeft();
    virtual const SharedExp& getLeft() const;

    /// Update the left hand side of the assignment
    void setLeft(SharedExp e);

    // get how to replace this statement in a use
    /// \returns the expression defining the right hand side of the assignment
    virtual SharedExp getRight() const = 0;

    /// \copydoc Statement::generateCode
    virtual void generateCode(ICodeGenerator *, const BasicBlock *) override {}

    /// \copydoc Statement::simplifyAddr
    virtual void simplifyAddr() override;

    /// \copydoc Statement::genConstraints
    virtual void genConstraints(LocationSet& cons) override;


protected:
    SharedExp m_lhs; ///< The left hand side of the assignment
};
