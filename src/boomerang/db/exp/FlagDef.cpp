#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "FlagDef.h"


#include "boomerang/db/RTL.h"
#include "boomerang/db/visitor/expmodifier/ExpModifier.h"
#include "boomerang/db/visitor/expvisitor/ExpVisitor.h"


FlagDef::FlagDef(const SharedExp& params, const SharedRTL& _rtl)
    : Unary(opFlagDef, params)
    , m_rtl(_rtl)
{
}


FlagDef::~FlagDef()
{
}


void FlagDef::appendDotFile(QTextStream& of)
{
    of << "e_" << HostAddress(this) << " [shape=record,label=\"{";
    of << "opFlagDef \\n" << HostAddress(this) << "| ";
    // Display the RTL as "RTL <r1> <r2>..." vertically (curly brackets)
    of << "{ RTL ";
    const size_t n = m_rtl->size();

    for (size_t i = 0; i < n; i++) {
        of << "| <r" << i << "> ";
    }

    of << "} | <p1> }\"];\n";
    subExp1->appendDotFile(of);
    of << "e_" << HostAddress(this) << ":p1->e_" << HostAddress(subExp1.get()) << ";\n";
}


bool FlagDef::accept(ExpVisitor *v)
{
    bool visitChildren = true;
    bool ret = v->preVisit(shared_from_base<FlagDef>(), visitChildren);

    if (!visitChildren) {
        return ret;
    }

    if (ret) {
        ret = subExp1->accept(v);
    }

    return ret;
}


SharedExp FlagDef::accept(ExpModifier *v)
{
    bool visitChildren = true;
    SharedExp ret        = v->preModify(shared_from_base<FlagDef>(), visitChildren);
    std::shared_ptr<FlagDef> flgdef_ret = std::dynamic_pointer_cast<FlagDef>(ret);

    if (visitChildren) {
        subExp1 = subExp1->accept(v);
    }

    assert(flgdef_ret);
    return v->postModify(flgdef_ret);
}
