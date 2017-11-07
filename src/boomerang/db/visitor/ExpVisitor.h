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


#include <memory>


class Unary;
class Binary;
class Ternary;
class TypedExp;
class FlagDef;
class RefExp;
class Location;
class Const;
class Terminal;
class TypeVal;


/**
 * The ExpVisitor class is used to iterate over all subexpressions in an expression.
 */
class ExpVisitor
{
public:
    ExpVisitor() = default;
    virtual ~ExpVisitor() = default;

    /**
     * Visit the expression while iterating through the expression tree.
     * The default behaviour is to visit all expressions and all children.
     *
     * \param exp the expression to evaluate
     * \param dontVisitChildren if true, do not visit the children of \p exp.
     * \returns true to continue visiting
     * return false to abandon iterating through the expression (terminate the search)
     * Set override true to not do the usual recursion into children
     */
    virtual bool visit(const std::shared_ptr<Unary>& exp,    bool& dontVisitChildren);

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Binary>& exp,   bool& dontVisitChildren);

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Ternary>& exp,  bool& dontVisitChildren);

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<TypedExp>& exp, bool& dontVisitChildren);

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<FlagDef>& exp,  bool& dontVisitChildren);

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<RefExp>& exp,   bool& dontVisitChildren);

    /// \copydoc ExpVisitor::visit
    virtual bool visit(const std::shared_ptr<Location>& exp, bool& dontVisitChildren);

    /// \copydoc ExpVisitor::visit
    /// A Const does not have children
    virtual bool visit(const std::shared_ptr<Const>& exp);

    /// \copydoc ExpVisitor::visit
    /// A Terminal does not have children
    virtual bool visit(const std::shared_ptr<Terminal>& exp);

    /// \copydoc ExpVisitor::visit
    /// A TypeVal does not have children
    virtual bool visit(const std::shared_ptr<TypeVal>& exp);
};
