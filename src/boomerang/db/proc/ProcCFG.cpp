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

    return m_fragmentSet.find(const_cast<IRFragment *>(frag)) != m_fragmentSet.end();
}


IRFragment *ProcCFG::createFragment(std::unique_ptr<RTLList> rtls, BasicBlock *bb)
{
    IRFragment *frag = new IRFragment(bb, std::move(rtls));
    m_fragmentSet.insert(frag);

    return frag;
}


IRFragment *ProcCFG::splitFragment(IRFragment *frag, Address splitAddr)
{
    auto it = std::find_if(frag->getRTLs()->begin(), frag->getRTLs()->end(),
                           [splitAddr](const auto &rtl) { return splitAddr == rtl->getAddress(); });

    if (it == frag->getRTLs()->end()) {
        // cannot split
        return nullptr;
    }

    std::unique_ptr<RTLList> newRTLs(new RTLList);
    std::for_each(it, frag->getRTLs()->end(),
                  [&newRTLs](std::unique_ptr<RTL> &rtl) { newRTLs->push_back(std::move(rtl)); });
    frag->getRTLs()->erase(it, frag->getRTLs()->end());

    IRFragment *newFrag = createFragment(std::move(newRTLs), frag->getBB());
    newFrag->setType(frag->getType());
    frag->setType(FragType::Fall);

    return newFrag;
}


void ProcCFG::removeFragment(IRFragment *frag)
{
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

    auto it = m_fragmentSet.find(frag);

    if (it != m_fragmentSet.end()) {
        frag->clearPhis();
        m_fragmentSet.erase(it);
        delete frag;
        return;
    }

    LOG_WARN("Tried to remove fragment at address %1; does not exist in CFG", frag->getLowAddr());
    delete frag;
}


void ProcCFG::addEdge(IRFragment *sourceBB, IRFragment *destBB)
{
    if (!sourceBB || !destBB) {
        return;
    }

    // Wire up edges
    sourceBB->addSuccessor(destBB);
    destBB->addPredecessor(sourceBB);

    // special handling for upgrading oneway BBs to twoway BBs
    if (sourceBB->isType(FragType::Oneway) && (sourceBB->getNumSuccessors() > 1)) {
        sourceBB->setType(FragType::Twoway);
    }
}


bool ProcCFG::isWellFormed() const
{
    for (const IRFragment *bb : *this) {
        if (bb->getFunction() != m_myProc) {
            LOG_ERROR("CFG is not well formed: BB at address %1 does not belong to proc '%2'",
                      bb->getLowAddr(), m_myProc->getName());
            return false;
        }

        for (const IRFragment *pred : bb->getPredecessors()) {
            if (!pred->isPredecessorOf(bb)) {
                LOG_ERROR("CFG is not well formed: Edge from BB at %1 to BB at %2 is malformed.",
                          pred->getLowAddr(), bb->getLowAddr());
                return false;
            }
        }

        for (const IRFragment *succ : bb->getSuccessors()) {
            if (!succ->isSuccessorOf(bb)) {
                LOG_ERROR("CFG is not well formed: Edge from BB at %1 to BB at %2 is malformed.",
                          bb->getLowAddr(), succ->getLowAddr());
                return false;
            }
        }
    }
    return true;
}


IRFragment *ProcCFG::findRetNode()
{
    IRFragment *retNode = nullptr;

    for (IRFragment *bb : *this) {
        if (bb->isType(FragType::Ret)) {
            return bb;
        }
        else if (bb->isType(FragType::Call)) {
            const Function *callee = bb->getCallDestProc();
            if (callee && !callee->isLib() && callee->isNoReturn()) {
                retNode = bb; // use noreturn calls if the proc does not return
            }
        }
    }

    return retNode;
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
    assert(false); // FIXME

    //     for (IRFragment */*bb*/ : *this) {
    //         bb->print(out);
    //     }

    out << '\n';
}


QString ProcCFG::toString() const
{
    QString result;
    OStream os(&result);
    print(os);
    return result;
}
