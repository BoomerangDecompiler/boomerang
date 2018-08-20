#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ConscriptSetter.h"

#include "boomerang/ssl/exp/Binary.h"
#include "boomerang/ssl/exp/Const.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/exp/Operator.h"


ConscriptSetter::ConscriptSetter(int n, bool clear)
    : m_inLocalGlobal(false)
    , m_clear(clear)
{
    m_curConscript = n;
}


int ConscriptSetter::getLast() const
{
    return m_curConscript;
}


SharedExp ConscriptSetter::postModify(const std::shared_ptr<Const> &exp)
{
    if (!m_inLocalGlobal) {
        if (m_clear) {
            m_modified |= (exp->getConscript() != 0);
            exp->setConscript(0);
        }
        else {
            m_modified = true;
            exp->setConscript(++m_curConscript);
        }
    }

    m_inLocalGlobal = false;
    return exp; // Continue recursion
}


SharedExp ConscriptSetter::preModify(const std::shared_ptr<Location> &exp, bool &visitChildren)
{
    const OPER op = exp->getOper();

    if ((op == opLocal) || (op == opGlobal) || (op == opRegOf) || (op == opParam)) {
        m_inLocalGlobal = true;
    }

    visitChildren = true;
    return exp; // Continue recursion
}


SharedExp ConscriptSetter::preModify(const std::shared_ptr<Binary> &exp, bool &visitChildren)
{
    const OPER op = exp->getOper();

    if (op == opSize) {
        m_inLocalGlobal = true;
    }

    visitChildren = true;
    return exp; // Continue recursion
}
