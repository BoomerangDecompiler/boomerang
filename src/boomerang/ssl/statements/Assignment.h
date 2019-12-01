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


#include "boomerang/ssl/statements/TypingStatement.h"


/**
 * Assignment is the base class of all statements that assign to a
 * left hand side like ordinary assign statements, phi statements or implicit assignments.
 */
class BOOMERANG_API Assignment : public TypingStatement
{
public:
    Assignment(SharedExp lhs);
    Assignment(SharedType ty, SharedExp lhs);
    Assignment(const Assignment &other) = default;
    Assignment(Assignment &&other)      = default;

    ~Assignment() override;

    Assignment &operator=(const Assignment &other) = default;
    Assignment &operator=(Assignment &&other) = default;

public:
    /// We also want operator< for assignments. For example, we want ReturnStatement
    /// to contain a set of (pointers to) Assignments, so we can automatically
    /// make sure that existing assignments are not duplicated.
    /// Assume that we won't want sets of assignments differing by anything other than LHSs
    bool operator<(const Assignment &o);

    /// \copydoc Assignment::print
    void print(OStream &os) const override;

    /// Like print, but print without statement number
    virtual void printCompact(OStream &os) const = 0;

    /// \copydoc Statement::getTypeForExp
    SharedConstType getTypeForExp(SharedConstExp e) const override;

    /// \copydoc Statement::getTypeForExp
    SharedType getTypeForExp(SharedExp e) override;

    /// \copydoc Statement::setTypeFor
    void setTypeForExp(SharedExp e, SharedType ty) override;

    /// \copydoc Statement::getDefinitions
    void getDefinitions(LocationSet &defs, bool assumeABICompliance) const override;

    /// \copydoc Statement::definesLoc
    bool definesLoc(SharedExp loc) const override;

    /// \returns the expression defining the left hand side of the assignment
    SharedExp getLeft() const;

    /// Update the left hand side of the assignment
    void setLeft(SharedExp e);

    /// Get how to replace this statement in a use
    /// \returns the expression defining the right hand side of the assignment
    virtual SharedExp getRight() const = 0;

    /// \copydoc Statement::simplifyAddr
    void simplifyAddr() override;

protected:
    SharedExp m_lhs; ///< The left hand side of the assignment
};
