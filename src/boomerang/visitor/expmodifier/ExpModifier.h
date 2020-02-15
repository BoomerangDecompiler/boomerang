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


class Exp;
class Unary;
class Binary;
class Ternary;
class TypedExp;
class RefExp;
class Location;
class Const;
class Terminal;

using SharedExp = std::shared_ptr<class Exp>;


/**
 * The ExpModifier class is used to iterate over all subexpressions in an expression. It contains
 * methods for each kind of subexpression found in an and can be used to eliminate switch
 * statements. It is a little more expensive to use than ExpVisitor, but can make changes to the
 * expression
 */
class BOOMERANG_API ExpModifier
{
public:
    ExpModifier()          = default;
    virtual ~ExpModifier() = default;

public:
    bool isModified() const { return m_modified; }
    void setModified(bool modified = true) { m_modified = modified; }
    void clearModified() { m_modified = false; }

    /**
     * Change the expression before visiting children.
     * The default behaviour is to not modify the expression
     * and to recurse to all child expressions.
     * \note preModify() functions must not change the type of the expression,
     * e.g. from Binary to Const
     *
     * \param[in]  exp           the expression to change
     * \param[out] visitChildren true to continue visiting children.
     * \returns The modified expression (must not be null). Note that this is not necessarily
     * the same expression as \p exp, but an expression of the same type as \p exp.
     */
    virtual SharedExp preModify(const std::shared_ptr<Unary> &exp, bool &visitChildren);

    /// \copydoc ExpModifier::preModify
    virtual SharedExp preModify(const std::shared_ptr<Binary> &exp, bool &visitChildren);

    /// \copydoc ExpModifier::preModify
    virtual SharedExp preModify(const std::shared_ptr<Ternary> &exp, bool &visitChildren);

    /// \copydoc ExpModifier::preModify
    virtual SharedExp preModify(const std::shared_ptr<TypedExp> &exp, bool &visitChildren);

    /// \copydoc ExpModifier::preModify
    virtual SharedExp preModify(const std::shared_ptr<RefExp> &exp, bool &visitChildren);

    /// \copydoc ExpModifier::preModify
    virtual SharedExp preModify(const std::shared_ptr<Location> &exp, bool &visitChildren);

    /**
     * Modify the expression after modifying children.
     * The default behaviour is to not modify the expression.
     * \note \ref postModify functions are allowed to return an expression of a different type,
     * unlike \ref preModify.
     *
     * \param exp the expression to modify.
     * \returns the modified expression.
     */
    virtual SharedExp postModify(const std::shared_ptr<Unary> &exp);

    /// \copydoc ExpModifier::postModify
    virtual SharedExp postModify(const std::shared_ptr<Binary> &exp);

    /// \copydoc ExpModifier::postModify
    virtual SharedExp postModify(const std::shared_ptr<Ternary> &exp);

    /// \copydoc ExpModifier::postModify
    virtual SharedExp postModify(const std::shared_ptr<TypedExp> &exp);

    /// \copydoc ExpModifier::postModify
    virtual SharedExp postModify(const std::shared_ptr<RefExp> &exp);

    /// \copydoc ExpModifier::postModify
    virtual SharedExp postModify(const std::shared_ptr<Location> &exp);

    /// \copydoc ExpModifier::postModify
    virtual SharedExp postModify(const std::shared_ptr<Const> &exp);

    /// \copydoc ExpModifier::postModify
    virtual SharedExp postModify(const std::shared_ptr<Terminal> &exp);

protected:
    bool m_modified = false; ///< Set if there is any change. Don't have to implement
};
