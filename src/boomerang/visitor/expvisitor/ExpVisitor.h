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


class Unary;
class Binary;
class Ternary;
class TypedExp;
class RefExp;
class Location;
class Const;
class Terminal;


/**
 * The ExpVisitor class is used to iterate over all subexpressions in an expression.
 */
class BOOMERANG_API ExpVisitor
{
public:
    ExpVisitor()          = default;
    virtual ~ExpVisitor() = default;

public:
    /**
     * Visit the expression before all subespressions.
     * The default behaviour is to visit all expressions and all children.
     *
     * \param exp the expression to evaluate
     * \param visitChildren if true, also visit the children of \p exp.
     *
     * \retval true  to continue visiting
     * \retval false to abort visitation immediately.
     */
    virtual bool preVisit(const std::shared_ptr<Unary> &exp, bool &visitChildren);
    /// \copydoc ExpVisitor::preVisit
    virtual bool preVisit(const std::shared_ptr<Binary> &exp, bool &visitChildren);
    /// \copydoc ExpVisitor::preVisit
    virtual bool preVisit(const std::shared_ptr<Ternary> &exp, bool &visitChildren);
    /// \copydoc ExpVisitor::preVisit
    virtual bool preVisit(const std::shared_ptr<TypedExp> &exp, bool &visitChildren);
    /// \copydoc ExpVisitor::preVisit
    virtual bool preVisit(const std::shared_ptr<RefExp> &exp, bool &visitChildren);
    /// \copydoc ExpVisitor::preVisit
    virtual bool preVisit(const std::shared_ptr<Location> &exp, bool &visitChildren);

    /**
     * Visit the expression after all subexpressions.
     * \param exp expression to evaluate
     * \retval true  to continue visiting
     * \retval false to abort visitation immedately.
     */
    virtual bool postVisit(const std::shared_ptr<Unary> &exp);
    /// \copydoc ExpVisitor::postVisit
    virtual bool postVisit(const std::shared_ptr<Binary> &exp);
    /// \copydoc ExpVisitor::postVisit
    virtual bool postVisit(const std::shared_ptr<Ternary> &exp);
    /// \copydoc ExpVisitor::postVisit
    virtual bool postVisit(const std::shared_ptr<TypedExp> &exp);
    /// \copydoc ExpVisitor::postVisit
    virtual bool postVisit(const std::shared_ptr<RefExp> &exp);
    /// \copydoc ExpVisitor::postVisit
    virtual bool postVisit(const std::shared_ptr<Location> &exp);

    /// Visit this Const.
    /// \retval true  to continue visiting parent and sibling expressions.
    /// \retval false to abort visitation immediately.
    virtual bool visit(const std::shared_ptr<Const> &exp);

    /// Visit this Terminal expression.
    /// \retval true  to continue visiting parent and sibling expressions.
    /// \retval false to abort visitation immediately.
    virtual bool visit(const std::shared_ptr<Terminal> &exp);
};
