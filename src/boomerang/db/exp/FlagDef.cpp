#include "FlagDef.h"

#include "boomerang/db/RTL.h"
#include "boomerang/db/Visitor.h"


FlagDef::FlagDef(SharedExp params, SharedRTL _rtl)
    : Unary(opFlagDef, params)
    , rtl(_rtl)
{
}


FlagDef::~FlagDef()
{
    // delete rtl;
}


void FlagDef::appendDotFile(QTextStream& of)
{
    of << "e_" << HostAddress(this) << " [shape=record,label=\"{";
    of << "opFlagDef \\n" << HostAddress(this) << "| ";
    // Display the RTL as "RTL <r1> <r2>..." vertically (curly brackets)
    of << "{ RTL ";
    const size_t n = rtl->size();

    for (size_t i = 0; i < n; i++) {
        of << "| <r" << i << "> ";
    }

    of << "} | <p1> }\"];\n";
    subExp1->appendDotFile(of);
    of << "e_" << HostAddress(this) << ":p1->e_" << HostAddress(subExp1.get()) << ";\n";
}


bool FlagDef::accept(ExpVisitor *v)
{
    bool override, ret = v->visit(shared_from_base<FlagDef>(), override);

    if (override) {
        return ret;
    }

    if (ret) {
        ret = subExp1->accept(v);
    }

    return ret;
}


SharedExp FlagDef::accept(ExpModifier *v)
{
    bool recur;
    auto ret        = v->preVisit(shared_from_base<FlagDef>(), recur);
    auto flgdef_ret = std::dynamic_pointer_cast<FlagDef>(ret);

    if (recur) {
        subExp1 = subExp1->accept(v);
    }

    assert(flgdef_ret);
    return v->postVisit(flgdef_ret);
}
