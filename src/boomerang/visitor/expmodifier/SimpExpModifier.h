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


#include "boomerang/visitor/expmodifier/ExpModifier.h"


/**
 * A simplifying expression modifier.
 * It does a simplification on the parent after a child has been modified.
 */
class SimpExpModifier : public ExpModifier
{
public:
    SimpExpModifier();
    virtual ~SimpExpModifier() = default;

public:
    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<Unary> &exp, bool &visitChildren) override;

    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<Binary> &exp, bool &visitChildren) override;

    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<Ternary> &exp, bool &visitChildren) override;

    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<TypedExp> &exp, bool &visitChildren) override;


    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<RefExp> &exp, bool &visitChildren) override;

    /// \copydoc ExpModifier::preModify
    SharedExp preModify(const std::shared_ptr<Location> &exp, bool &visitChildren) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<Unary> &exp) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<Binary> &exp) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<Ternary> &exp) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<TypedExp> &exp) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<RefExp> &exp) override;

    /// \copydoc ExpModifier::postModify
    SharedExp postModify(const std::shared_ptr<Location> &exp) override;

    /// \copydoc ExpModifier::modify
    SharedExp postModify(const std::shared_ptr<Const> &exp) override;

    /// \copydoc ExpModifier::modify
    SharedExp postModify(const std::shared_ptr<Terminal> &exp) override;


    unsigned getUnchanged() { return m_unchanged; }
    bool isTopChanged() { return !(m_unchanged & m_mask); }

protected:
    /**
     * These two provide 31 bits (or sizeof(int)-1) of information about whether the child is
     * unchanged. If the mask overflows, it goes to zero, and from then on the child is reported as
     * always changing. (That's why it's an "unchanged" set of flags, instead of a "changed" set).
     * This is used to avoid calling simplify in most cases where it is not necessary.
     */
    unsigned int m_mask;
    unsigned int m_unchanged;
};
