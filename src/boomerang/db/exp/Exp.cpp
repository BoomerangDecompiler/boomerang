/*
 * Copyright (C) 2002-2006 Mike Van Emmerik and Trent Waddington
 */

/***************************************************************************/ /**
 * \file       exp.cpp
 * \brief   Implementation of the Exp and related classes.
 ******************************************************************************/

#include "boomerang/db/cfg.h"
#include "boomerang/db/register.h"
#include "boomerang/db/rtl.h" // E.g. class ParamEntry in decideType()
#include "boomerang/db/proc.h"
#include "boomerang/db/signature.h"
#include "boomerang/db/prog.h"
#include "boomerang/db/visitor.h"

#include "boomerang/util/Log.h"
#include "boomerang/util/types.h"
#include "boomerang/util/Util.h"

#include <QRegularExpression>

#include <cassert>
#include <numeric>   // For accumulate
#include <algorithm> // For std::max()
#include <map>       // In decideType()
#include <sstream>   // Need gcc 3.0 or better
#include <cstring>
#include <iomanip>   // For std::setw etc


// This to satisfy the compiler (never gets called!)
SharedExp  dummy;
SharedExp& Exp::refSubExp1()
{
    assert(false);
    return dummy;
}


SharedExp& Exp::refSubExp2()
{
    assert(false);
    return dummy;
}


SharedExp& Exp::refSubExp3()
{
    assert(false);
    return dummy;
}


//    //    //    //
// TypeVal    //
//    //    //    //



char *Exp::prints()
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void Exp::dump()
{
    QTextStream ost(stderr);

    print(ost);
}


void Exp::createDotFile(const char *name)
{
    QFile fl(name);

    if (!fl.open(QFile::WriteOnly)) {
        LOG << "Could not open " << name << " to write dotty file\n";
        return;
    }

    QTextStream of(&fl);
    of << "digraph Exp {\n";
    appendDotFile(of);
    of << "}";
}



bool Exp::isRegOfK()
{
    if (m_oper != opRegOf) {
        return false;
    }

    return ((Unary *)this)->getSubExp1()->getOper() == opIntConst;
}


bool Exp::isRegN(int N) const
{
    if (m_oper != opRegOf) {
        return false;
    }

    SharedConstExp sub = ((const Unary *)this)->getSubExp1();
    return(sub->getOper() == opIntConst && std::static_pointer_cast<const Const>(sub)->getInt() == N);
}


bool Exp::isAfpTerm()
{
    auto cur = shared_from_this();

    if (m_oper == opTypedExp) {
        cur = getSubExp1();
    }

    if (cur->getOper() == opAddrOf) {
        SharedExp p = cur->getSubExp1();
        if (p && p->getOper() == opMemOf) {
            cur = p->getSubExp1();
        }
    }

    OPER curOp = cur->getOper();

    if (curOp == opAFP) {
        return true;
    }

    if ((curOp != opPlus) && (curOp != opMinus)) {
        return false;
    }

    // cur must be a Binary* now
    OPER subOp1 = cur->getSubExp1()->getOper();
    OPER subOp2 = cur->getSubExp2()->getOper();
    return((subOp1 == opAFP) && (subOp2 == opIntConst));
}


int Exp::getVarIndex()
{
    assert(m_oper == opVar);
    SharedExp sub = this->getSubExp1();
    return std::static_pointer_cast<const Const>(sub)->getInt();
}


SharedExp Exp::getGuard()
{
    if (m_oper == opGuard) {
        return getSubExp1();
    }

    return nullptr;
}


SharedExp Exp::match(const SharedConstExp& pattern)
{
    if (*this == *pattern) {
        return Terminal::get(opNil);
    }

    if (pattern->getOper() == opVar) {
        return Binary::get(opList, Binary::get(opEquals, pattern->clone(), this->clone()), Terminal::get(opNil));
    }

    return nullptr;
}


static QRegularExpression variableRegexp("[a-zA-Z0-9]+");


// TODO use regexp ?
#define ISVARIABLE_S(x)    \
    (strspn((x.c_str()), "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789") == (x).length())


bool Exp::match(const QString& pattern, std::map<QString, SharedConstExp>& bindings)
{
    // most obvious
    QString     tgt;
    QTextStream ostr(&tgt);

    print(ostr);

    if (tgt == pattern) {
        return true;
    }

    assert((pattern.lastIndexOf(variableRegexp) == 0) == ISVARIABLE_S(pattern.toStdString()));

    // alright, is pattern an acceptable variable?
    if (pattern.lastIndexOf(variableRegexp) == 0) {
        bindings[pattern] = shared_from_this();
        return true;
    }

    // no, fail
    return false;
}


void Exp::doSearch(const Exp& search, SharedExp& pSrc, std::list<SharedExp *>& li, bool once)
{
    bool compare;

    compare = (search == *pSrc);

    if (compare) {
        li.push_back(&pSrc); // Success

        if (once) {
            return; // No more to do
        }
    }

    // Either want to find all occurrences, or did not match at this level
    // Recurse into children, unless a matching opSubscript
    if (!compare || (pSrc->m_oper != opSubscript)) {
        pSrc->doSearchChildren(search, li, once);
    }
}


void Exp::doSearchChildren(const Exp& search, std::list<SharedExp *>& li, bool once)
{
    Q_UNUSED(search);
    Q_UNUSED(li);
    Q_UNUSED(once);
    // Const and Terminal do not override this
}



SharedExp Exp::searchReplace(const Exp& search, const SharedExp& replace, bool& change)
{
    return searchReplaceAll(search, replace, change, true);
}


SharedExp Exp::searchReplaceAll(const Exp& search, const SharedExp& replace, bool& change, bool once /* = false */)
{
    // TODO: consider working on base object, and only in case when we find the search, use clone call to return the
    // new object ?
    if (this == &search) { // TODO: WAT ?
        change = true;
        return replace->clone();
    }

    std::list<SharedExp *> li;
    SharedExp              top = shared_from_this(); // top may change; that's why we have to return it
    doSearch(search, top, li, false);

    for (auto it = li.begin(); it != li.end(); it++) {
        SharedExp *pp = *it;
        *pp = replace->clone(); // Do the replacement

        if (once) {
            change = true;
            return top;
        }
    }

    change = (li.size() != 0);
    return top;
}


bool Exp::search(const Exp& search, SharedExp& result)
{
    std::list<SharedExp *> li;
    result = nullptr; // In case it fails; don't leave it unassigned
    // The search requires a reference to a pointer to this object.
    // This isn't needed for searches, only for replacements, but we want to re-use the same search routine
    SharedExp top = shared_from_this();
    doSearch(search, top, li, false);

    if (li.size()) {
        result = *li.front();
        return true;
    }

    return false;
}


bool Exp::searchAll(const Exp& search, std::list<SharedExp>& result)
{
    std::list<SharedExp *> li;
    // result.clear();    // No! Useful when searching for more than one thing
    // (add to the same list)
    // The search requires a reference to a pointer to this object.
    // This isn't needed for searches, only for replacements, but we want to re-use the same search routine
    SharedExp pSrc = shared_from_this();
    doSearch(search, pSrc, li, false);

    for (auto it : li) {
        // li is list of pointers to SharedExp ; result is list of SharedExp
        result.push_back(*it);
    }

    return !li.empty();
}


void Exp::partitionTerms(std::list<SharedExp>& positives, std::list<SharedExp>& negatives, std::vector<int>& integers,
                         bool negate)
{
    SharedExp p1, p2;

    switch (m_oper)
    {
    case opPlus:
        p1 = getSubExp1();
        p2 = getSubExp2();
        p1->partitionTerms(positives, negatives, integers, negate);
        p2->partitionTerms(positives, negatives, integers, negate);
        break;

    case opMinus:
        p1 = getSubExp1();
        p2 = getSubExp2();
        p1->partitionTerms(positives, negatives, integers, negate);
        p2->partitionTerms(positives, negatives, integers, !negate);
        break;

    case opTypedExp:
        p1 = getSubExp1();
        p1->partitionTerms(positives, negatives, integers, negate);
        break;

    case opIntConst:
        {
            int k = static_cast<Const *>(this)->getInt();

            if (negate) {
                integers.push_back(-k);
            }
            else {
                integers.push_back(k);
            }

            break;
        }

    default:

        // These can be any other expression tree
        if (negate) {
            negatives.push_back(shared_from_this());
        }
        else {
            positives.push_back(shared_from_this());
        }
    }
}


SharedExp Exp::accumulate(std::list<SharedExp>& exprs)
{
    int n = exprs.size();

    if (n == 0) {
        return Const::get(0);
    }

    if (n == 1) {
        return exprs.front()->clone();
    }

    std::list<SharedExp> cloned_list;

    for (const SharedExp& v : exprs) {
        cloned_list.push_back(v->clone());
    }

    SharedExp last_val = cloned_list.back();
    cloned_list.pop_back();
    auto res = Binary::get(opPlus, cloned_list.back(), last_val);
    cloned_list.pop_back();

    while (!cloned_list.empty()) {
        res = Binary::get(opPlus, cloned_list.back(), res);
        cloned_list.pop_back();
    }

    return res;
}


SharedExp Exp::simplify()
{
#if DEBUG_SIMP
    SharedExp save = clone();
#endif
    bool      bMod = false; // True if simplified at this or lower level
    SharedExp res  = shared_from_this();

    // res = ExpTransformer::applyAllTo(res, bMod);
    // return res;
    do {
        bMod = false;
        // SharedExp before = res->clone();
        res = res->polySimplify(bMod); // Call the polymorphic simplify
    } while (bMod);                    // If modified at this (or a lower) level, redo


    // The below is still important. E.g. want to canonicalise sums, so we know that a + K + b is the same as a + b + K
    // No! This slows everything down, and it's slow enough as it is. Call only where needed:
//    res = res->simplifyArith();

#if DEBUG_SIMP
    if (!(*res == *save)) {
        std::cout << "simplified " << save << "  to  " << res << "\n";
    }
#endif
    return res;
}


SharedExp accessMember(SharedExp parent, const std::shared_ptr<CompoundType>& c, int n)
{
    unsigned   r   = c->getOffsetRemainder(n * 8);
    QString    nam = c->getNameAtOffset(n * 8);
    SharedType t   = c->getTypeAtOffset(n * 8);
    SharedExp  res = Binary::get(opMemberAccess, parent, Const::get(nam));

    assert((r % 8) == 0);

    if (t->resolvesToCompound()) {
        res = accessMember(res, t->as<CompoundType>(), r / 8);
    }
    else if (t->resolvesToPointer() && t->as<PointerType>()->getPointsTo()->resolvesToCompound()) {
        if (r != 0) {
            assert(false);
        }
    }
    else if (t->resolvesToArray()) {
        std::shared_ptr<ArrayType> a = t->as<ArrayType>();
        SharedType                 array_member_type = a->getBaseType();
        int b  = array_member_type->getSize() / 8;
        assert(array_member_type->getSize() % 8);

        res = Binary::get(opArrayIndex, res, Const::get(n / b));

        if (array_member_type->resolvesToCompound()) {
            res = accessMember(res, array_member_type->as<CompoundType>(), n % b);
        }
    }

    return res;
}


SharedExp Exp::convertFromOffsetToCompound(SharedExp parent, std::shared_ptr<CompoundType>& c, unsigned n)
{
    if (n * 8 >= c->getSize()) {
        return nullptr;
    }

    QString nam = c->getNameAtOffset(n * 8);

    if (!nam.isNull() && (nam != "pad")) {
        SharedExp l = Location::memOf(parent);
        return Unary::get(opAddrOf, accessMember(l, c, n));
    }

    return nullptr;
}


const char *Exp::getOperName() const
{
    return operToString(m_oper);
}


QString Exp::toString() const
{
    QString     res;
    QTextStream os(&res);

    this->print(os);
    return res;
}


void Exp::printt(QTextStream& os /*= cout*/) const
{
    print(os);

    if (m_oper != opTypedExp) {
        return;
    }

    SharedType t = ((TypedExp *)this)->getType();
    os << "<" << t->getSize() << ">";
}


void Exp::printAsHL(QTextStream& os /*= cout*/)
{
    QString     tgt;
    QTextStream ost(&tgt);

    ost << this; // Print to the string stream

    if ((tgt.length() >= 4) && (tgt[1] == '[')) {
        // r[nn]; change to rnn
        tgt.remove(1, 1);             // '['
        tgt.remove(tgt.length() - 1); // ']'
    }

    os << tgt;                        // Print to the output stream
}


QTextStream& operator<<(QTextStream& os, const Exp *p)
{
    // Useful for debugging, but can clutter the output
    p->printt(os);
    return os;
}


SharedExp Exp::fixSuccessor()
{
    bool      change;
    SharedExp result;
    UniqExp   search_expression(new Unary(opSuccessor, Location::regOf(Terminal::get(opWild))));

    // Assume only one successor function in any 1 expression
    if (search(*search_expression, result)) {
        // Result has the matching expression, i.e. succ(r[K])
        SharedExp sub1 = result->getSubExp1();
        assert(sub1->getOper() == opRegOf);
        SharedExp sub2 = sub1->getSubExp1();
        assert(sub2->getOper() == opIntConst);
        // result     sub1    sub2
        // succ(      r[   Const K    ])
        // Note: we need to clone the r[K] part, since it will be ;//deleted as
        // part of the searchReplace below
        auto replace = sub1->clone();
        auto c       = replace->access<Const, 1>();
        c->setInt(c->getInt() + 1); // Do the increment
        SharedExp res = searchReplace(*result, replace, change);
        return res;
    }

    return shared_from_this();
}


SharedExp Exp::killFill()
{
    static Ternary srch1(opZfill, Terminal::get(opWild), Terminal::get(opWild), Terminal::get(opWild));
    static Ternary srch2(opSgnEx, Terminal::get(opWild), Terminal::get(opWild), Terminal::get(opWild));
    SharedExp      res = shared_from_this();

    std::list<SharedExp *> result;
    doSearch(srch1, res, result, false);
    doSearch(srch2, res, result, false);

    for (SharedExp *it : result) {
        // Kill the sign extend bits
        *it = (*it)->getSubExp3();
    }

    return res;
}


bool Exp::isTemp() const
{
    if (m_oper == opTemp) {
        return true;
    }

    if (m_oper != opRegOf) {
        return false;
    }

    // Some old code has r[tmpb] instead of just tmpb
    SharedConstExp sub = getSubExp1();
    return sub->m_oper == opTemp;
}


SharedExp Exp::removeSubscripts(bool& allZero)
{
    auto        e = shared_from_this();
    LocationSet locs;

    e->addUsedLocs(locs);
    LocationSet::iterator xx;
    allZero = true;

    for (xx = locs.begin(); xx != locs.end(); xx++) {
        if ((*xx)->getOper() == opSubscript) {
            auto              r1   = std::static_pointer_cast<RefExp>(*xx);
            const Instruction *def = r1->getDef();

            if (!((def == nullptr) || (def->getNumber() == 0))) {
                allZero = false;
            }

            bool change;
            e = e->searchReplaceAll(**xx, r1->getSubExp1() /*->clone()*/,
                                    change); // TODO: what happens when clone is restored here ?
        }
    }

    return e;
}


SharedExp Exp::fromSSAleft(UserProc *proc, Instruction *d)
{
    auto r = RefExp::get(shared_from_this(), d); // "Wrap" in a ref

    return r->accept(new ExpSsaXformer(proc));
}


//    //    //    //    //    //
//    genConstraints    //
//    //    //    //    //    //

SharedExp Exp::genConstraints(SharedExp /*result*/)
{
    // Default case, no constraints -> return true
    return Terminal::get(opTrue);
}







//    //    //    //    //    //    //    //
//                            //
//       V i s i t i n g        //
//                            //
//    //    //    //    //    //    //    //

// The following are similar, but don't have children that have to accept visitors



void Exp::fixLocationProc(UserProc *p)
{
    // All locations are supposed to have a pointer to the enclosing UserProc that they are a location of. Sometimes,
    // you have an arbitrary expression that may not have all its procs set. This function fixes the procs for all
    // Location subexpresssions.
    FixProcVisitor fpv;

    fpv.setProc(p);
    accept(&fpv);
}


// GetProcVisitor class

UserProc *Exp::findProc()
{
    GetProcVisitor gpv;

    accept(&gpv);
    return gpv.getProc();
}


void Exp::setConscripts(int n, bool bClear)
{
    SetConscripts sc(n, bClear);

    accept(&sc);
}


SharedExp Exp::stripSizes()
{
    SizeStripper ss;

    return accept(&ss);
}


QString Exp::getAnyStrConst()
{
    SharedExp e = shared_from_this();

    if (m_oper == opAddrOf) {
        e = getSubExp1();

        if (e->m_oper == opSubscript) {
            e = e->getSubExp1();
        }

        if (e->m_oper == opMemOf) {
            e = e->getSubExp1();
        }
    }

    if (e->m_oper != opStrConst) {
        return QString::null;
    }

    return std::static_pointer_cast<const Const>(e)->getStr();
}


void Exp::addUsedLocs(LocationSet& used, bool memOnly)
{
    UsedLocsFinder ulf(used, memOnly);

    accept(&ulf);
}


SharedExp Exp::expSubscriptVar(const SharedExp& e, Instruction *def)
{
    ExpSubscripter es(e, def);

    return accept(&es);
}


SharedExp Exp::expSubscriptValNull(const SharedExp& e)
{
    return expSubscriptVar(e, nullptr);
}


SharedExp Exp::expSubscriptAllNull(/*Cfg* cfg*/)
{
    return expSubscriptVar(Terminal::get(opWild), nullptr /* was nullptr, nullptr, cfg */);
}

SharedExp Exp::bypass()
{
    CallBypasser cb(nullptr);

    return accept(&cb);
}


void Exp::bypassComp()
{
    if (m_oper != opMemOf) {
        return;
    }

    setSubExp1(getSubExp1()->bypass());
}


int Exp::getComplexityDepth(UserProc *proc)
{
    ComplexityFinder cf(proc);

    accept(&cf);
    return cf.getDepth();
}


int Exp::getMemDepth()
{
    MemDepthFinder mdf;

    accept(&mdf);
    return mdf.getDepth();
}


SharedExp Exp::propagateAll()
{
    ExpPropagator ep;

    return accept(&ep);
}


SharedExp Exp::propagateAllRpt(bool& changed)
{
    ExpPropagator ep;

    changed = false;
    SharedExp ret = shared_from_this();

    while (true) {
        ep.clearChanged(); // Want to know if changed this *last* accept()
        ret = ret->accept(&ep);

        if (ep.isChanged()) {
            changed = true;
        }
        else {
            break;
        }
    }

    return ret;
}


bool Exp::containsFlags()
{
    FlagsFinder ff;

    accept(&ff);
    return ff.isFound();
}


bool Exp::containsBadMemof(UserProc *proc)
{
    BadMemofFinder bmf(proc);

    accept(&bmf);
    return bmf.isFound();
}


bool Exp::containsMemof(UserProc *proc)
{
    ExpHasMemofTester ehmt(proc);

    accept(&ehmt);
    return ehmt.getResult();
}
