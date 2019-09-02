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
    clear();
}


void DefCollector::clear()
{
    m_defs.clear();
    m_initialised = false;
}


void DefCollector::updateDefs(std::map<SharedExp, std::deque<SharedStmt>, lessExpStar> &Stacks,
                              UserProc *proc)
{
    for (auto &Stack : Stacks) {
        if (Stack.second.empty()) {
            continue; // This variable's definition doesn't reach here
        }

        // Create an assignment of the form loc := loc{def}
        auto re    = RefExp::get(Stack.first->clone(), Stack.second.back());
        std::shared_ptr<Assign> as(new Assign(Stack.first->clone(), re));
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

    for (auto def : m_defs) {
        QString tgt;
        OStream ost(&tgt);
        def->getLeft()->print(ost);
        ost << "=";
        def->getRight()->print(ost);
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
    for (std::shared_ptr<Assign> def : m_defs) {
        SharedExp lhs = def->getLeft();

        if (*lhs == *e) {
            return def->getRight();
        }
    }

    return nullptr; // Not explicitly defined here
}


void DefCollector::makeCloneOf(const DefCollector &other)
{
    m_initialised = other.m_initialised;
    m_defs.clear();

    for (const auto &elem : other) {
        m_defs.insert(elem->clone()->as<Assign>());
    }
}


void DefCollector::searchReplaceAll(const Exp &from, SharedExp to, bool &changed)
{
    for (auto def : m_defs) {
        changed |= def->searchAndReplace(from, to);
    }
}


void DefCollector::insert(const std::shared_ptr<Assign> &a)
{
    SharedExp l = a->getLeft();

    if (existsOnLeft(l)) {
        return;
    }

    m_defs.insert(a);
}
