#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "ProcCFG.h"

#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/signature/Parameter.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/ssl/exp/Location.h"
#include "boomerang/ssl/statements/CallStatement.h"
#include "boomerang/ssl/statements/ImplicitAssign.h"
#include "boomerang/util/log/Log.h"

#include <QtAlgorithms>

#include <cassert>


ProcCFG::ProcCFG(UserProc *proc)
    : m_myProc(proc)
{
    assert(m_myProc != nullptr);
}


ProcCFG::~ProcCFG()
{
    clear();
}


void ProcCFG::clear()
{
    m_implicitMap.clear();

    qDeleteAll(begin(), end()); // deletes all fragments
    m_fragmentSet.clear();
}


bool ProcCFG::hasFragment(const IRFragment *frag) const
{
    if (frag == nullptr) {
        return false;
    }

    return findFragment(frag) != m_fragmentSet.end();
}


IRFragment *ProcCFG::createFragment(std::unique_ptr<RTLList> rtls, BasicBlock *bb)
{
    assert(bb != nullptr);

    IRFragment *frag = new IRFragment(bb, std::move(rtls));
    m_fragmentSet.insert(frag);

    frag->setType((FragType)bb->getType());
    frag->updateAddresses();
    return frag;
}


IRFragment *ProcCFG::splitFragment(IRFragment *frag, Address splitAddr)
{
    assert(hasFragment(frag));

    auto it = std::find_if(frag->getRTLs()->begin(), frag->getRTLs()->end(),
                           [splitAddr](const auto &rtl) { return splitAddr == rtl->getAddress(); });

    if (it == frag->getRTLs()->end()) {
        // cannot split
        return frag;
    }
    else if (it == frag->getRTLs()->begin()) {
        // no need to split
        return frag;
    }

    // move RTLs with addr >= splitAddr to new list
    std::unique_ptr<RTLList> newRTLs(new RTLList);
    std::for_each(it, frag->getRTLs()->end(),
                  [&newRTLs](std::unique_ptr<RTL> &rtl) { newRTLs->push_back(std::move(rtl)); });
    frag->getRTLs()->erase(it, frag->getRTLs()->end());

    IRFragment *newFrag = createFragment(std::move(newRTLs), frag->getBB());
    newFrag->setType(frag->getType());

    frag->setType(FragType::Fall);
    addEdge(frag, newFrag);

    frag->updateAddresses();
    newFrag->updateAddresses();

    assert(frag->getHiAddr() < splitAddr);
    return newFrag;
}


void ProcCFG::removeFragment(IRFragment *frag)
{
    assert(frag != nullptr);
    assert(this->hasFragment(frag));

    RTLList::iterator rit;
    StatementList::iterator sit;

    for (SharedStmt s = frag->getFirstStmt(rit, sit); s; s = frag->getNextStmt(rit, sit)) {
        if (s->isCall()) {
            std::shared_ptr<CallStatement> call = s->as<CallStatement>();
            if (call->getDestProc() && !call->getDestProc()->isLib()) {
                UserProc *callee = static_cast<UserProc *>(call->getDestProc());
                callee->removeCaller(call);
            }
        }
    }

    auto it = findFragment(frag);

    if (it == m_fragmentSet.end()) {
        LOG_WARN("Tried to remove fragment at address %1; does not exist in CFG",
                 frag->getLowAddr());

        delete frag;
        return;
    }

    for (IRFragment *pred : frag->getPredecessors()) {
        pred->removeSuccessor(frag);
    }

    for (IRFragment *succ : frag->getSuccessors()) {
        succ->removePredecessor(frag);
    }

    frag->removeAllPredecessors();
    frag->removeAllSuccessors();

    frag->clearPhis();

    assert(*it == frag);
    m_fragmentSet.erase(it);
    delete frag;
}


IRFragment *ProcCFG::getFragmentByAddr(Address addr)
{
    auto it = std::find_if(m_fragmentSet.begin(), m_fragmentSet.end(),
                           [addr](IRFragment *frag) { return frag->getLowAddr() == addr; });

    return it != m_fragmentSet.end() ? *it : nullptr;
}


void ProcCFG::addEdge(IRFragment *sourceFrag, IRFragment *destFrag)
{
    if (!sourceFrag || !destFrag) {
        return;
    }

    // Wire up edges
    sourceFrag->addSuccessor(destFrag);
    destFrag->addPredecessor(sourceFrag);

    // special handling for upgrading oneway BBs to twoway BBs
    if (sourceFrag->isType(FragType::Oneway) && (sourceFrag->getNumSuccessors() > 1)) {
        sourceFrag->setType(FragType::Twoway);
    }
}


bool ProcCFG::isWellFormed() const
{
    for (const IRFragment *frag : *this) {
        if (frag->getProc() != m_myProc) {
            LOG_ERROR("CFG is not well formed: Fragment at address %1 does not belong to proc '%2'",
                      frag->getLowAddr(), m_myProc ? m_myProc->getName() : QString("<Unknown>"));
            return false;
        }

        for (const IRFragment *pred : frag->getPredecessors()) {
            if (!pred->isPredecessorOf(frag)) {
                LOG_ERROR("CFG is not well formed: Edge from fragment at %1 to fragment at %2 "
                          "is malformed.",
                          pred->getLowAddr(), frag->getLowAddr());
                return false;
            }
        }

        for (const IRFragment *succ : frag->getSuccessors()) {
            if (!succ->isSuccessorOf(frag)) {
                LOG_ERROR("CFG is not well formed: Edge from fragment at %1 to fragment at %2 "
                          "is malformed.",
                          frag->getLowAddr(), succ->getLowAddr());
                return false;
            }
        }
    }

    return true;
}


IRFragment *ProcCFG::findRetFragment()
{
    IRFragment *retFrag = nullptr;

    for (IRFragment *frag : *this) {
        if (frag->isType(FragType::Ret)) {
            return frag;
        }
        else if (frag->isType(FragType::Call)) {
            const Function *callee = frag->getCallDestProc();
            if (callee && !callee->isLib() && callee->isNoReturn()) {
                retFrag = frag; // use noreturn calls if the proc does not return
            }
        }
    }

    return retFrag;
}


SharedStmt ProcCFG::findOrCreateImplicitAssign(SharedExp exp)
{
    ExpStatementMap::iterator it = m_implicitMap.find(exp);
    if (it != m_implicitMap.end()) {
        // implicit already present, use it
        assert(it->second);
        return it->second;
    }

    if (!m_entryFrag) {
        return nullptr;
    }

    // In case the original gets changed
    exp = exp->clone();

    // A use with no explicit definition. Create a new implicit assignment
    std::shared_ptr<ImplicitAssign> def = m_entryFrag->addImplicitAssign(exp);

    // Remember it for later so we don't insert more than one implicit assignment for any one
    // location We don't clone the copy in the map. So if the location is a m[...], the same type
    // information is available in the definition as at all uses
    m_implicitMap[exp] = def;

    return def;
}


SharedStmt ProcCFG::findTheImplicitAssign(const SharedConstExp &x) const
{
    // As per the above, but don't create an implicit if it doesn't already exist
    ExpStatementMap::const_iterator it = m_implicitMap.find(std::const_pointer_cast<Exp>(x));
    return (it != m_implicitMap.end()) ? it->second : nullptr;
}


SharedStmt ProcCFG::findImplicitParamAssign(Parameter *param)
{
    // As per the above, but for parameters (signatures don't get updated with opParams)
    SharedExp paramExp = param->getExp();

    ExpStatementMap::iterator it = std::find_if(
        m_implicitMap.begin(), m_implicitMap.end(),
        [paramExp](const std::pair<const SharedConstExp &, SharedStmt> &val) {
            return val.first->equalNoSubscript(*paramExp);
        });

    if (it == m_implicitMap.end()) {
        it = m_implicitMap.find(Location::param(param->getName()));
    }

    return (it != m_implicitMap.end()) ? it->second : nullptr;
}


void ProcCFG::removeImplicitAssign(SharedExp x)
{
    auto it = m_implicitMap.find(x);

    assert(it != m_implicitMap.end());
    SharedStmt ia = it->second;
    m_implicitMap.erase(it);       // Delete the mapping
    m_myProc->removeStatement(ia); // Remove the actual implicit assignment statement as well
}


void ProcCFG::print(OStream &out) const
{
    out << "Control Flow Graph:\n";

    for (IRFragment *frag : *this) {
        frag->print(out);
    }

    out << '\n';
}


QString ProcCFG::toString() const
{
    QString result;
    OStream os(&result);
    print(os);
    return result;
}


void ProcCFG::setEntryAndExitFragment(IRFragment *entryFrag)
{
    m_entryFrag = entryFrag;

    for (IRFragment *frag : *this) {
        if (frag->isType(FragType::Ret)) {
            m_exitFrag = frag;
            return;
        }
    }
}


ProcCFG::FragmentSet::iterator ProcCFG::findFragment(const IRFragment *frag) const
{
    auto [from, to] = m_fragmentSet.equal_range(const_cast<IRFragment *>(frag));

    for (auto it = from; it != to; ++it) {
        if (*it == frag) {
            return it;
        }
    }

    return m_fragmentSet.end();
}
