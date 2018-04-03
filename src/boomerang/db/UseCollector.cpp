#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UseCollector.h"


#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/visitor/expmodifier/ExpSSAXformer.h"

#include <QString>
#include <QTextStream>


UseCollector::UseCollector()
    : m_initialised(false)
{
}


bool UseCollector::operator==(const UseCollector& other) const
{
    if (other.m_initialised != m_initialised) {
        return false;
    }

    iterator it1, it2;

    if (other.m_locs.size() != m_locs.size()) {
        return false;
    }

    for (it1 = m_locs.begin(), it2 = other.m_locs.begin(); it1 != m_locs.end(); ++it1, ++it2) {
        if (!(**it1 == **it2)) {
            return false;
        }
    }

    return true;
}


void UseCollector::makeCloneOf(const UseCollector& other)
{
    m_initialised = other.m_initialised;
    m_locs.clear();

    for (auto const& elem : other) {
        m_locs.insert((elem)->clone());
    }
}


void UseCollector::clear()
{
    m_locs.clear();
    m_initialised = false;
}


void UseCollector::insert(SharedExp e)
{
    m_locs.insert(e);
}


void UseCollector::print(QTextStream& os, bool html) const
{
    bool first = true;

    for (auto const& elem : m_locs) {
        if (first) {
            first = false;
        }
        else {
            os << ",  ";
        }

        (elem)->print(os, html);
    }
}


char *UseCollector::prints() const
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void UseCollector::dump() const
{
    QTextStream ost(stderr);
    print(ost);
}


void UseCollector::fromSSAForm(UserProc *proc, Statement *def)
{
    LocationSet   removes, inserts;
    iterator      it;
    ExpSsaXformer esx(proc);

    for (it = m_locs.begin(); it != m_locs.end(); ++it) {
        auto      ref = RefExp::get(*it, def); // Wrap it in a def
        SharedExp ret = ref->accept(&esx);

        // If there is no change, ret will equal *it again (i.e. fromSSAForm just removed the subscript)
        if (ret != *it) { // Pointer comparison
            // There was a change; we want to replace *it with ret
            removes.insert(*it);
            inserts.insert(ret);
        }
    }

    for (it = removes.begin(); it != removes.end(); ++it) {
        m_locs.remove(*it);
    }

    for (it = inserts.begin(); it != inserts.end(); ++it) {
        m_locs.insert(*it);
    }
}

void UseCollector::remove(SharedExp loc)
{
    m_locs.remove(loc);
}


void UseCollector::remove(iterator it)
{
    m_locs.remove(it);
}

