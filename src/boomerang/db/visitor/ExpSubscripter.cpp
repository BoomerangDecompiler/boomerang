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


ExpSubscripter::ExpSubscripter(const SharedExp& s, Statement* d)
    : m_search(s)
    , m_def(d)
{
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<Location>& e, bool& recur)
{
    if (*e == *m_search) {
        recur = e->isMemOf();         // Don't double subscript unless m[...]
        return RefExp::get(e, m_def); // Was replaced by postVisit below
    }

    recur = true;
    return e;
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<Binary>& e, bool& recur)
{
    // array[index] is like m[addrexp]: requires a subscript
    if (e->isArrayIndex() && (*e == *m_search)) {
        recur = true;                 // Check the index expression
        return RefExp::get(e, m_def); // Was replaced by postVisit below
    }

    recur = true;
    return e;
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<Terminal>& e)
{
    if (*e == *m_search) {
        return RefExp::get(e, m_def);
    }

    return e;
}


SharedExp ExpSubscripter::preVisit(const std::shared_ptr<RefExp>& e, bool& recur)
{
    recur = false; // Don't look inside... not sure about this
    return e;
}
