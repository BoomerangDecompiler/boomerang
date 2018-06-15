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


#include "boomerang/db/exp/Unary.h"

class RTL;
typedef std::shared_ptr<RTL> SharedRTL;
typedef std::shared_ptr<const RTL> SharedConstRTL;


/**
 * FlagDef holds a list of parameters (in the subexpression),
 * and a pointer to a RTL
 */
class FlagDef : public Unary
{
public:
    FlagDef(const SharedExp& params, const SharedRTL& rtl);
    FlagDef(const FlagDef& other) = default;
    FlagDef(FlagDef&& other) = default;

    virtual ~FlagDef() override;

    FlagDef& operator=(const FlagDef& other) = default;
    FlagDef& operator=(FlagDef&& other) = default;

    SharedConstRTL getRTL() const { return m_rtl; }

public:
    /// \copydoc Unary::accept
    virtual bool acceptVisitor(ExpVisitor *v) override;

    /// \copydoc Exp::preAccept
    virtual SharedExp preAccept(ExpModifier *mod, bool& visitChildren) override;

    /// \copydoc Exp::postAccept
    virtual SharedExp postAccept(ExpModifier *mod) override;

private:
    SharedRTL m_rtl;
};
