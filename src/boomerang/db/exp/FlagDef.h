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

/***************************************************************************/ /**
 * FlagDef is a subclass of Unary, and holds a list of parameters (in the subexpression), and a pointer to an RTL
 ******************************************************************************/
class FlagDef : public Unary
{
public:
    FlagDef(SharedExp params, SharedRTL rtl);
    virtual ~FlagDef() override;

    virtual void appendDotFile(QTextStream& of) override;

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual SharedExp accept(ExpModifier *v) override;

private:
    SharedRTL rtl;
};
