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


using SharedType = std::shared_ptr<class Type>;


class ExpConstCaster : public ExpModifier
{
public:
    ExpConstCaster(int num, SharedType ty);
    virtual ~ExpConstCaster() = default;

public:
    bool isChanged() const { return m_changed; }

    /// \copydoc ExpModifier::preModify
    SharedExp postModify(const std::shared_ptr<Const>& exp) override;

private:
    int m_num;
    SharedType m_ty;
    bool m_changed;
};
