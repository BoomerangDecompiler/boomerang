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


#define DEFCOL_COLS 120


DefCollector::~DefCollector()
{
}


void DefCollector::makeCloneOf(const DefCollector &other)
{
    m_defs.clear();

    for (const auto &elem : other) {
        m_defs.insert(elem->clone()->as<Assign>());
    }
}


void DefCollector::clear()
{
    m_defs.clear();
}


void DefCollector::collectDef(const std::shared_ptr<Assign> &a)
{
    if (hasDefOf(a->getLeft())) {
        return;
    }

    m_defs.insert(a);
}


bool DefCollector::hasDefOf(const SharedExp &e) const
{
    return m_defs.definesLoc(e);
}


SharedExp DefCollector::findDefFor(const SharedExp &e) const
{
    assert(e != nullptr);

    for (std::shared_ptr<Assign> def : m_defs) {
        if (*def->getLeft() == *e) {
            return def->getRight();
        }
    }

    return nullptr; // Not explicitly defined here
}


void DefCollector::updateDefs(std::map<SharedExp, std::stack<SharedStmt>, lessExpStar> &Stacks,
                              UserProc *proc)
{
    for (auto &Stack : Stacks) {
        if (Stack.second.empty()) {
            continue; // This variable's definition doesn't reach here
        }

        // Create an assignment of the form loc := loc{def}
        auto re = RefExp::get(Stack.first->clone(), Stack.second.top());
        std::shared_ptr<Assign> as(new Assign(Stack.first->clone(), re));
        as->setProc(proc); // Simplify sometimes needs this
        collectDef(as);
    }
}


void DefCollector::searchReplaceAll(const Exp &from, SharedExp to, bool &changed)
{
    for (auto def : m_defs) {
        changed |= def->searchAndReplace(from, to);
    }
}


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
