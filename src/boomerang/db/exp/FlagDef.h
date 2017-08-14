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
    virtual ~FlagDef();

    virtual void appendDotFile(QTextStream& of) override;

    // Visitation
    virtual bool accept(ExpVisitor *v) override;
    virtual SharedExp accept(ExpModifier *v) override;

private:
    SharedRTL rtl;
};

