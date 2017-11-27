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


/**
 * FlagDef holds a list of parameters (in the subexpression),
 * and a pointer to a RTL
 */
class FlagDef : public Unary
{
public:
    FlagDef(SharedExp params, SharedRTL rtl);
    FlagDef(const FlagDef& other) = default;
    FlagDef(FlagDef&& other) = default;

    virtual ~FlagDef() override;

    FlagDef& operator=(const FlagDef& other) = default;
    FlagDef& operator=(FlagDef&& other) = default;

public:
    /// \copydoc Unary::appendDotFile
    virtual void appendDotFile(QTextStream& of) override;

    /// \copydoc Unary::accept
    virtual bool accept(ExpVisitor *v) override;

    /// \copydoc Unary::accept
    virtual SharedExp accept(ExpModifier *v) override;

private:
    SharedRTL m_rtl;
};
