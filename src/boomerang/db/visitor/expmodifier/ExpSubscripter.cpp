#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ExpSubscripter.h"


#include "boomerang/db/exp/Binary.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"


ExpSubscripter::ExpSubscripter(const SharedExp& s, Statement* def)
    : m_search(s)
    , m_def(def)
{
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<Location>& exp, bool& visitChildren)
{
    if (*exp == *m_search) {
        visitChildren = exp->isMemOf();         // Don't double subscript unless m[...]
        return RefExp::get(exp, m_def); // Was replaced by postVisit below
    }

    visitChildren = true;
    return exp;
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<Binary>& exp, bool& visitChildren)
{
    // array[index] is like m[addrexp]: requires a subscript
    if (exp->isArrayIndex() && (*exp == *m_search)) {
        visitChildren = true;                 // Check the index expression
        return RefExp::get(exp, m_def); // Was replaced by postVisit below
    }

    visitChildren = true;
    return exp;
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<Terminal>& exp)
{
    if (*exp == *m_search) {
        return RefExp::get(exp, m_def);
    }

    return exp;
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<RefExp>& exp, bool& visitChildren)
{
    // Don't look inside... not sure about this
    visitChildren = false;
    return exp;
}
