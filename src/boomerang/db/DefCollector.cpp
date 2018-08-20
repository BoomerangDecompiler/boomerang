#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DefCollector.h"

#include "boomerang/ssl/exp/RefExp.h"
#include "boomerang/util/Util.h"

#include <QtAlgorithms>


DefCollector::~DefCollector()
{
    qDeleteAll(m_defs);
}


void DefCollector::clear()
{
    m_defs.clear();
    m_initialised = false;
}


void DefCollector::updateDefs(std::map<SharedExp, std::deque<Statement *>, lessExpStar> &Stacks,
                              UserProc *proc)
{
    for (auto it = Stacks.begin(); it != Stacks.end(); ++it) {
        if (it->second.empty()) {
            continue; // This variable's definition doesn't reach here
        }

        // Create an assignment of the form loc := loc{def}
        auto re    = RefExp::get(it->first->clone(), it->second.back());
        Assign *as = new Assign(it->first->clone(), re);
        as->setProc(proc); // Simplify sometimes needs this
        insert(as);
    }

    m_initialised = true;
}


#define DEFCOL_COLS 120

void DefCollector::print(OStream &os) const
{
    if (m_defs.empty()) {
        os << "<None>";
        return;
    }

    size_t col = 36;
    bool first = true;

    for (const_iterator it = m_defs.begin(); it != m_defs.end(); ++it) {
        QString tgt;
        OStream ost(&tgt);
        (*it)->getLeft()->print(ost);
        ost << "=";
        (*it)->getRight()->print(ost);
        size_t len = tgt.length();

        if (first) {
            first = false;
        }
        else if (col + 4 + len >= DEFCOL_COLS) { // 4 for a comma and three spaces
            if (col != DEFCOL_COLS - 1) {
                os << ","; // Comma at end of line
            }

            os << "\n                ";
            col = 16;
        }
        else {
            os << ",   ";
            col += 4;
        }

        os << tgt;
        col += len;
    }
}


SharedExp DefCollector::findDefFor(SharedExp e) const
{
    for (const_iterator it = m_defs.begin(); it != m_defs.end(); ++it) {
        SharedExp lhs = (*it)->getLeft();

        if (*lhs == *e) {
            return (*it)->getRight();
        }
    }

    return nullptr; // Not explicitly defined here
}


void DefCollector::makeCloneOf(const DefCollector &other)
{
    m_initialised = other.m_initialised;
    qDeleteAll(m_defs);
    m_defs.clear();

    for (const auto &elem : other) {
        m_defs.insert(static_cast<Assign *>(elem->clone()));
    }
}


void DefCollector::searchReplaceAll(const Exp &from, SharedExp to, bool &change)
{
    for (iterator it = m_defs.begin(); it != m_defs.end(); ++it) {
        change |= (*it)->searchAndReplace(from, to);
    }
}


void DefCollector::insert(Assign *a)
{
    SharedExp l = a->getLeft();

    if (existsOnLeft(l)) {
        delete a;
        return;
    }

    m_defs.insert(a);
}
