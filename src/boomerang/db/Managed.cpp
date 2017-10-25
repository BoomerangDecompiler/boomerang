#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "Managed.h"


/**
 * \file       managed.cpp
 * \brief   Implementation of "managed" classes such as InstructionSet, which feature makeUnion etc
 */

#include <sstream>
#include <cstring>

#include "boomerang/core/Boomerang.h"

#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Location.h"

#include "boomerang/util/Log.h"
#include "boomerang/util/Types.h"


QTextStream & operator<<(QTextStream& os, const InstructionSet *ss)
{
    ss->print(os);
    return os;
}


QTextStream& operator<<(QTextStream& os, const AssignSet *as)
{
    as->print(os);
    return os;
}


QTextStream& operator<<(QTextStream& os, const LocationSet *ls)
{
    ls->print(os);
    return os;
}


void InstructionSet::makeUnion(InstructionSet& other)
{
    std::set<Statement *>::iterator it;

    for (it = other.begin(); it != other.end(); it++) {
        insert(*it);
    }
}


void InstructionSet::makeDiff(InstructionSet& other)
{
    std::set<Statement *>::iterator it;

    for (it = other.begin(); it != other.end(); it++) {
        erase(*it);
    }
}


void InstructionSet::makeIsect(InstructionSet& other)
{
    std::set<Statement *>::iterator it, ff;

    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);

        if (ff == other.end()) {
            // Not in both sets
            erase(it);
        }
    }
}


bool InstructionSet::isSubSetOf(InstructionSet& other)
{
    std::set<Statement *>::iterator it, ff;

    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);

        if (ff == other.end()) {
            return false;
        }
    }

    return true;
}


bool InstructionSet::remove(Statement *s)
{
    if (find(s) != end()) {
        erase(s);
        return true;
    }

    return false;
}


bool InstructionSet::exists(Statement *s)
{
    iterator it = find(s);

    return(it != end());
}


bool InstructionSet::definesLoc(SharedExp loc)
{
    for (auto const& elem : *this) {
        if ((elem)->definesLoc(loc)) {
            return true;
        }
    }

    return false;
}


const char *InstructionSet::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    std::set<Statement *>::iterator it;

    for (it = begin(); it != end(); it++) {
        if (it != begin()) {
            ost << ",\t";
        }

        ost << *it;
    }

    ost << "\n";
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void InstructionSet::dump()
{
    QTextStream q_cerr(stderr);

    print(q_cerr);
}


void InstructionSet::print(QTextStream& os) const
{
    std::set<Statement *>::iterator it;

    for (it = begin(); it != end(); it++) {
        if (it != begin()) {
            os << ",\t";
        }

        os << *it;
    }

    os << "\n";
}


void InstructionSet::printNums(QTextStream& os)
{
    for (iterator it = begin(); it != end();) {
        if (*it) {
            (*it)->printNum(os);
        }
        else {
            os << "-"; // Special case for nullptr definition
        }

        if (++it != end()) {
            os << " ";
        }
    }
}


bool InstructionSet::operator<(const InstructionSet& o) const
{
    if (size() < o.size()) {
        return true;
    }

    if (size() > o.size()) {
        return false;
    }

    const_iterator it1, it2;

    for (it1 = begin(), it2 = o.begin(); it1 != end(); it1++, it2++) {
        if (*it1 < *it2) {
            return true;
        }

        if (*it1 > *it2) {
            return false;
        }
    }

    return false;
}


void AssignSet::makeUnion(AssignSet& other)
{
    iterator it;

    for (it = other.begin(); it != other.end(); it++) {
        insert(*it);
    }
}


void AssignSet::makeDiff(AssignSet& other)
{
    iterator it;

    for (it = other.begin(); it != other.end(); it++) {
        erase(*it);
    }
}


void AssignSet::makeIsect(AssignSet& other)
{
    iterator it, ff;

    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);

        if (ff == other.end()) {
            // Not in both sets
            erase(it);
        }
    }
}


bool AssignSet::isSubSetOf(AssignSet& other)
{
    iterator it, ff;

    for (it = begin(); it != end(); it++) {
        ff = other.find(*it);

        if (ff == other.end()) {
            return false;
        }
    }

    return true;
}


bool AssignSet::remove(Assign *a)
{
    if (find(a) != end()) {
        erase(a);
        return true;
    }

    return false;
}


bool AssignSet::exists(Assign *a)
{
    iterator it = find(a);

    return(it != end());
}


bool AssignSet::definesLoc(SharedExp loc) const
{
    Assign as(loc, Terminal::get(opWild));

    return find(&as) != end();
}


Assign *AssignSet::lookupLoc(SharedExp loc)
{
    Assign   as(loc, Terminal::get(opWild));
    iterator ff = find(&as);

    if (ff == end()) {
        return nullptr;
    }

    return *ff;
}


char *AssignSet::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);
    iterator    it;

    for (it = begin(); it != end(); it++) {
        if (it != begin()) {
            ost << ",\t";
        }

        ost << *it;
    }

    ost << "\n";
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void AssignSet::dump()
{
    QTextStream q_cerr(stderr);

    print(q_cerr);
}


void AssignSet::print(QTextStream& os) const
{
    iterator it;

    for (it = begin(); it != end(); it++) {
        if (it != begin()) {
            os << ",\t";
        }

        os << *it;
    }

    os << "\n";
}


void AssignSet::printNums(QTextStream& os)
{
    for (iterator it = begin(); it != end();) {
        if (*it) {
            (*it)->printNum(os);
        }
        else {
            os << "-"; // Special case for nullptr definition
        }

        if (++it != end()) {
            os << " ";
        }
    }
}


bool AssignSet::operator<(const AssignSet& o) const
{
    if (size() < o.size()) {
        return true;
    }

    if (size() > o.size()) {
        return false;
    }

    const_iterator it1, it2;

    for (it1 = begin(), it2 = o.begin(); it1 != end(); it1++, it2++) {
        if (*it1 < *it2) {
            return true;
        }

        if (*it1 > *it2) {
            return false;
        }
    }

    return false;
}


LocationSet& LocationSet::operator=(const LocationSet& o)
{
    lset.clear();
    ExpSet::const_iterator it;

    for (it = o.lset.begin(); it != o.lset.end(); it++) {
        lset.insert((*it)->clone());
    }

    return *this;
}


LocationSet::LocationSet(const LocationSet& o)
{
    ExpSet::const_iterator it;

    for (it = o.lset.begin(); it != o.lset.end(); it++) {
        lset.insert((*it)->clone());
    }
}


char *LocationSet::prints() const
{
    QString     tgt;
    QTextStream ost(&tgt);

    ExpSet::iterator it;

    for (it = lset.begin(); it != lset.end(); it++) {
        if (it != lset.begin()) {
            ost << ",\t";
        }

        ost << *it;
    }

    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void LocationSet::dump() const
{
    QTextStream ost(stderr);

    print(ost);
}


void LocationSet::print(QTextStream& os) const
{
    ExpSet::iterator it;

    for (it = lset.begin(); it != lset.end(); it++) {
        if (it != lset.begin()) {
            os << ",\t";
        }

        os << *it;
    }
}


void LocationSet::remove(SharedExp given)
{
    ExpSet::iterator it = lset.find(given);

    if (it == lset.end()) {
        return;
    }

    // NOTE: if the below uncommented, things go crazy. Valgrind says that
    // the deleted value gets used next in LocationSet::operator== ?!
    // delete *it;          // These expressions were cloned when created
    lset.erase(it);
}


void LocationSet::removeIfDefines(InstructionSet& given)
{
    InstructionSet::iterator it;

    for (it = given.begin(); it != given.end(); ++it) {
        Statement   *s = (Statement *)*it;
        LocationSet defs;
        s->getDefinitions(defs);
        LocationSet::iterator dd;

        for (dd = defs.begin(); dd != defs.end(); ++dd) {
            lset.erase(*dd);
        }
    }
}


void LocationSet::makeUnion(LocationSet& other)
{
    iterator it;

    for (it = other.lset.begin(); it != other.lset.end(); it++) {
        lset.insert(*it);
    }
}


void LocationSet::makeDiff(LocationSet& other)
{
    ExpSet::iterator it;

    for (it = other.lset.begin(); it != other.lset.end(); it++) {
        lset.erase(*it);
    }
}


bool LocationSet::operator==(const LocationSet& o) const
{
    // We want to compare the locations, not the pointers
    if (size() != o.size()) {
        return false;
    }

    ExpSet::const_iterator it1, it2;

    for (it1 = lset.begin(), it2 = o.lset.begin(); it1 != lset.end(); it1++, it2++) {
        if (!(**it1 == **it2)) {
            return false;
        }
    }

    return true;
}


bool LocationSet::exists(SharedExp e) const
{
    return lset.find(e) != lset.end();
}


SharedExp LocationSet::findNS(SharedExp e)
{
    // Note: can't search with a wildcard, since it doesn't have the weak ordering required (I think)
    auto r(RefExp::get(e, nullptr));
    // Note: the below assumes that nullptr is less than any other pointer
    iterator it = lset.lower_bound(r);

    if (it == lset.end()) {
        return nullptr;
    }

    if ((*(*it)->getSubExp1() == *e)) {
        return *it;
    }
    else {
        return nullptr;
    }
}


bool LocationSet::existsImplicit(SharedExp e) const
{
    auto     r(RefExp::get(e, nullptr));
    iterator it = lset.lower_bound(r); // First element >= r

    // Note: the below relies on the fact that nullptr is less than any other pointer. Try later entries in the set:
    while (it != lset.end()) {
        if (!(*it)->isSubscript()) {
            return false;                            // Looking for e{something} (could be e.g. %pc)
        }

        if (!(*(*it)->getSubExp1() == *e)) { // Gone past e{anything}?
            return false;                    // Yes, then e{-} or e{0} cannot exist
        }

        if ((*it)->access<RefExp>()->isImplicitDef()) { // Check for e{-} or e{0}
            return true;                                // Found
        }

        ++it;                                           // Else check next entry
    }

    return false;
}


bool LocationSet::findDifferentRef(const std::shared_ptr<RefExp>& e, SharedExp& dr)
{
    assert(e);
    auto             search = RefExp::get(e->getSubExp1()->clone(), (Statement *)-1);
    ExpSet::iterator pos    = lset.find(search);

    if (pos == lset.end()) {
        return false;
    }

    while (pos != lset.end()) {
        assert(*pos);

        // Exit if we've gone to a new base expression
        // E.g. searching for r13{10} and **pos is r14{0}
        // Note: we want a ref-sensitive compare, but with the outer refs stripped off
        // For example: m[r29{10} - 16]{any} is different from m[r29{20} - 16]{any}
        if (!(*(*pos)->getSubExp1() == *e->getSubExp1())) {
            break;
        }

        // Bases are the same; return true if only different ref
        if (!(**pos == *e)) {
            dr = *pos;
            return true;
        }

        ++pos;
    }

    return false;
}


void LocationSet::addSubscript(Statement *d /* , Cfg* cfg */)
{
    ExpSet newSet;

    for (SharedExp it : lset) {
        newSet.insert(it->expSubscriptVar(it, d /* , cfg */));
    }

    lset = newSet; // Replace the old set!
                   // Note: don't delete the old exps; they are copied in the new set
}


void LocationSet::substitute(Assign& a)
{
    auto lhs = a.getLeft();

    if (lhs == nullptr) {
        return;
    }

    SharedExp rhs = a.getRight();

    if (rhs == nullptr) {
        return; // ? Will this ever happen?
    }

    ExpSet::iterator it;
    // Note: it's important not to change the pointer in the set of pointers to expressions, without removing and
    // inserting again. Otherwise, the set becomes out of order, and operations such as set comparison fail!
    // To avoid any funny behaviour when iterating the loop, we use the following two sets
    LocationSet removeSet;       // These will be removed after the loop
    LocationSet removeAndDelete; // These will be removed then deleted
    LocationSet insertSet;       // These will be inserted after the loop
    bool        change;

    for (it = lset.begin(); it != lset.end(); it++) {
        SharedExp loc = *it;
        SharedExp replace;

        if (loc->search(*lhs, replace)) {
            if (rhs->isTerminal()) {
                // This is no longer a location of interest (e.g. %pc)
                removeSet.insert(loc);
                continue;
            }

            loc = loc->clone()->searchReplaceAll(*lhs, rhs, change);

            if (change) {
                loc = loc->simplifyArith();
                loc = loc->simplify();

                // If the result is no longer a register or memory (e.g.
                // r[28]-4), then delete this expression and insert any
                // components it uses (in the example, just r[28])
                if (!loc->isRegOf() && !loc->isMemOf()) {
                    // Note: can't delete the expression yet, because the
                    // act of insertion into the remove set requires silent
                    // calls to the compare function
                    removeAndDelete.insert(*it);
                    loc->addUsedLocs(insertSet);
                    continue;
                }

                // Else we just want to replace it
                // Regardless of whether the top level expression pointer has
                // changed, remove and insert it from the set of pointers
                removeSet.insert(*it); // Note: remove the unmodified ptr
                insertSet.insert(loc);
            }
        }
    }

    makeDiff(removeSet);       // Remove the items to be removed
    makeDiff(removeAndDelete); // These are to be removed as well
    makeUnion(insertSet);      // Insert the items to be added
    // Now delete the expressions that are no longer needed
//    std::set<SharedExp , lessExpStar>::iterator dd;
//    for (dd = removeAndDelete.lset.begin(); dd != removeAndDelete.lset.end(); dd++)
//        delete *dd; // Plug that memory leak
}


bool StatementList::remove(Statement *s)
{
    iterator it;

    for (it = begin(); it != end(); it++) {
        if (*it == s) {
            erase(it);
            return true;
        }
    }

    return false;
}


void StatementList::append(const StatementList& sl)
{
    insert(end(), sl.begin(), sl.end());
}


void StatementList::append(const InstructionSet& ss)
{
    insert(end(), ss.begin(), ss.end());
}


char *StatementList::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    for (auto& elem : *this) {
        ost << elem << ",\t";
    }

    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void StatementVec::putAt(int idx, Statement *s)
{
    if (idx >= (int)svec.size()) {
        svec.resize(idx + 1, nullptr);
    }

    svec[idx] = s;
}


StatementVec::iterator StatementVec::remove(iterator it)
{
    /*
     *  iterator oldoldit = it;
     *  iterator oldit = it;
     *  for (it++; it != svec.end(); it++, oldit++)
     * oldit = *it;
     *  svec.resize(svec.size()-1);
     *  return oldoldit;
     */
    return svec.erase(it);
}


char *StatementVec::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    for (Statement *it : svec) {
        ost << it << ",\t";
    }

    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void StatementVec::printNums(QTextStream& os)
{
    for (iterator it = svec.begin(); it != svec.end();) {
        if (*it) {
            (*it)->printNum(os);
        }
        else {
            os << "-"; // Special case for no definition
        }

        if (++it != svec.end()) {
            os << " ";
        }
    }
}


void StatementList::makeIsect(StatementList& a, LocationSet& b)
{
    clear();

    for (auto& elem : a) {
        Assignment *as = (Assignment *)elem;

        if (b.exists(as->getLeft())) {
            push_back(as);
        }
    }
}


void StatementList::makeCloneOf(StatementList& o)
{
    clear();

    for (auto& elem : o) {
        push_back((elem)->clone());
    }
}


bool StatementList::existsOnLeft(const SharedExp& loc) const
{
    for (auto& elem : *this) {
        if (*((Assignment *)elem)->getLeft() == *loc) {
            return true;
        }
    }

    return false;
}


void StatementList::removeDefOf(SharedExp loc)
{
    for (iterator it = begin(); it != end(); it++) {
        if (*((Assignment *)*it)->getLeft() == *loc) {
            erase(it);
            return;
        }
    }
}


Assignment *StatementList::findOnLeft(SharedExp loc) const
{
    if (empty()) {
        return nullptr;
    }

    for (auto& elem : *this) {
        SharedExp left = ((Assignment *)elem)->getLeft();

        if (*left == *loc) {
            return (Assignment *)elem;
        }

        if (left->isLocal()) {
            auto           l = left->access<Location>();
            SharedConstExp e = l->getProc()->expFromSymbol(l->access<Const, 1>()->getStr());

            if (e && ((*e == *loc) || (e->isSubscript() && (*e->getSubExp1() == *loc)))) {
                return (Assignment *)elem;
            }
        }
    }

    return nullptr;
}


void LocationSet::printDiff(LocationSet *o) const
{
    ExpSet::iterator it;
    bool             printed2not1 = false;

    for (it = o->lset.begin(); it != o->lset.end(); it++) {
        SharedExp oe = *it;

        if (lset.find(oe) == lset.end()) {
            if (!printed2not1) {
                printed2not1 = true;
                LOG_MSG("In set 2 but not set 1:");
            }

            LOG_MSG("  %1", oe);
        }
    }

    bool printed1not2 = false;

    for (it = lset.begin(); it != lset.end(); it++) {
        SharedExp e = *it;

        if (o->lset.find(e) == o->lset.end()) {
            if (!printed1not2) {
                printed1not2 = true;
                LOG_MSG("In set 1 but not set 2:");
            }

            LOG_MSG("  %1", e);
        }
    }
}


void ConnectionGraph::add(SharedExp a, SharedExp b)
{
    iterator ff = emap.find(a);

    while (ff != emap.end() && *ff->first == *a) {
        if (*ff->second == *b) {
            return; // Don't add a second entry
        }

        ++ff;
    }

    std::pair<SharedExp, SharedExp> pr;
    pr.first  = a;
    pr.second = b;
    emap.insert(pr);
}


std::vector<SharedExp> ConnectionGraph::allConnected(SharedExp a)
{
    std::vector<SharedExp> res;
    const_iterator         ff = emap.find(a);

    while (ff != emap.end() && *ff->first == *a) {
        res.push_back(ff->second);
        ++ff;
    }

    return res;
}


void ConnectionGraph::connect(SharedExp a, SharedExp b)
{
    // if a is connected to c,d and e, 'b' should also be connected to c,d and e
    std::vector<SharedExp> a_connections = allConnected(a);
    std::vector<SharedExp> b_connections = allConnected(b);
    add(a, b);

    for (const SharedExp& e : b_connections) {
        add(a, e);
    }

    add(b, a);

    for (SharedExp e : a_connections) {
        add(e, b);
    }
}


int ConnectionGraph::count(SharedExp e) const
{
    const_iterator ff = emap.find(e);
    int            n  = 0;

    while (ff != emap.end() && *ff->first == *e) {
        ++n;
        ++ff;
    }

    return n;
}


bool ConnectionGraph::isConnected(SharedExp a, const Exp& b) const
{
    const_iterator ff = emap.find(a);

    while (ff != emap.end() && *ff->first == *a) {
        if (*ff->second == b) {
            return true; // Found the connection
        }

        ++ff;
    }

    return false;
}


bool ConnectionGraph::allRefsHaveDefs() const
{
    for (auto iter : *this) {
        const SharedExp& fr(iter.first);
        const SharedExp& sc(iter.second);
        assert(std::dynamic_pointer_cast<RefExp>(fr));
        assert(std::dynamic_pointer_cast<RefExp>(sc));

        if (nullptr == std::static_pointer_cast<RefExp>(fr)->getDef()) {
            return false;
        }

        if (nullptr == std::static_pointer_cast<RefExp>(sc)->getDef()) {
            return false;
        }
    }

    return true;
}


void ConnectionGraph::update(SharedExp a, SharedExp b, SharedExp c)
{
    // find a->b
    iterator ff = emap.find(a);

    while (ff != emap.end() && *ff->first == *a) {
        if (*ff->second == *b) {
            ff->second = c; // Now a->c
            break;
        }

        ++ff;
    }

    // find b -> a
    ff = emap.find(b);

    while (ff != emap.end() && *ff->first == *b) {
        if (*ff->second == *a) {
            emap.erase(ff);
            add(c, a); // Now c->a
            break;
        }

        ++ff;
    }
}


ConnectionGraph::iterator ConnectionGraph::remove(iterator aa)
{
    assert(aa != emap.end());
    SharedExp b = aa->second;
    emap.erase(aa++);
    iterator bb = emap.find(b);
    assert(bb != emap.end());

    if (bb == aa) {
        ++aa;
    }

    emap.erase(bb);
    return aa;
}


void ConnectionGraph::dump() const
{
    for (auto iter : *this) {
        LOG_MSG("%1 <-> %2", iter.first, iter.second);
    }
}
