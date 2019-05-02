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
    qDeleteAll(begin(), end()); // deletes all BBs
}


void ProcCFG::clear()
{
    // Don't delete the BBs; this will delete any CaseStatements we want to save for the re-decode.
    // Just let them leak since we do not use a garbage collection any more.
    // A better idea would be to save the CaseStatements explicitly and delete the BBs afterwards.
    // But this has to wait until the decoder redesign.

    m_bbStartMap.clear();
    m_implicitMap.clear();
    m_entryBB    = nullptr;
    m_exitBB     = nullptr;
    m_wellFormed = true;
}


bool ProcCFG::hasBB(const BasicBlock *bb) const
{
    if (bb == nullptr) {
        return false;
    }

    // we have to use linear search here, since the bb might already have been deleted
    // (invoking UB when calling getLowAddr).
    for (const auto &val : m_bbStartMap) {
        if (val.second == bb) {
            return true;
        }
    }

    return false;
}


BasicBlock *ProcCFG::createBB(BBType bbType, std::unique_ptr<RTLList> bbRTLs)
{
    assert(!bbRTLs->empty());

    // First find the native address of the first RTL
    // Can't use BasicBlock::getLowAddr(), since we don't yet have a BB!
    Address startAddr = bbRTLs->front()->getAddress();

    // If this is zero, try the next RTL (only). This may be necessary if e.g. there is a BB with a
    // delayed branch only, with its delay instruction moved in front of it (with 0 address). Note:
    // it is possible to see two RTLs with zero address with SPARC: jmpl %o0, %o1. There will be one
    // for the delay instr (if not a NOP), and one for the side effect of copying %o7 to %o1. Note
    // that orphaned BBs (for which we must compute addr here to to be 0) must not be added to the
    // map, but they have no RTLs with a non zero address.
    if (startAddr.isZero() && (bbRTLs->size() > 1)) {
        RTLList::iterator next = std::next(bbRTLs->begin());
        startAddr              = (*next)->getAddress();
    }

    // If this addr is non zero, check the map to see if we have a (possibly incomplete) BB here
    // already If it is zero, this is a special BB for handling delayed branches or the like
    bool mustCreateBB       = true;
    BBStartMap::iterator mi = m_bbStartMap.end();
    BasicBlock *currentBB   = nullptr;

    if (!startAddr.isZero()) {
        mi = m_bbStartMap.find(startAddr);

        if ((mi != m_bbStartMap.end()) && mi->second) {
            currentBB = mi->second;

            // It should be incomplete, or the BB there should be zero
            // (we have called ensureBBExists() but not yet created the BB for it).
            // Else we have duplicated BBs.
            // Note: this can happen with forward jumps into the middle of a loop,
            // so not error
            if (!currentBB->isIncomplete()) {
                LOG_VERBOSE("Not creating a BB at address %1 because a BB already exists",
                            currentBB->getLowAddr());

                // we automatically destroy bbRTLs
                return nullptr;
            }
            else {
                // Fill in the details, and return it
                currentBB->setRTLs(std::move(bbRTLs));
                currentBB->setType(bbType);
            }

            mustCreateBB = false;
        }
    }

    if (mustCreateBB) {
        currentBB = new BasicBlock(bbType, std::move(bbRTLs), m_myProc);

        // Note that currentBB->getLowAddr() == startAddr
        if (startAddr == Address::INVALID) {
            LOG_FATAL("Cannot add BB with invalid lowAddr %1", startAddr);
        }

        insertBB(currentBB);
        mi = m_bbStartMap.find(startAddr);
    }

    if (!startAddr.isZero() && (mi != m_bbStartMap.end())) {
        //
        //  Existing   New         +---+ "low" part of new
        //            +---+        +---+
        //            |   |          |   Fall through
        //    +---+   |   |   ==>  +---+
        //    |   |   |   |        |   | Existing; rest of new discarded
        //    +---+   +---+        +---+
        //
        // Check for overlap of the just added BB with the next BB (address wise).
        // If there is an overlap, truncate the RTLList for the new BB to not overlap,
        // and make this a fall through BB.
        // We still want to do this even if the new BB overlaps with an incomplete BB,
        // though in this case, splitBB needs to fill in the details for the "high"
        // BB of the split.
        // Also, in this case, we return a pointer to the newly completed BB,
        // so it will get out edges added (if required). In the other case
        // (i.e. we overlap with an existing, completed BB), we want to return 0, since
        // the out edges are already created.
        //
        mi = std::next(mi);

        if (mi != m_bbStartMap.end()) {
            BasicBlock *nextBB    = (*mi).second;
            Address nextAddr      = (*mi).first;
            bool nextIsIncomplete = nextBB->isIncomplete();

            if (nextAddr <= currentBB->getRTLs()->back()->getAddress()) {
                // Need to truncate the current BB. We use splitBB(), but pass it nextBB so it
                // doesn't create a new BB for the "bottom" BB of the split pair
                splitBB(currentBB, nextAddr, nextBB);

                // If the overlapped BB was incomplete, return the "bottom" part of the BB, so
                // adding out edges will work properly.
                if (nextIsIncomplete) {
                    assert(nextBB);
                    return nextBB;
                }

                LOG_VERBOSE("Not creating a BB at address %1 because a BB already exists",
                            currentBB->getLowAddr());
                return nullptr;
            }
        }

        //  Existing    New        +---+ Top of existing
        //    +---+                +---+
        //    |   |    +---+       +---+ Fall through
        //    |   |    |   | =>    |   |
        //    |   |    |   |       |   | New; rest of existing discarded
        //    +---+    +---+       +---+
        //
        // Note: no need to check the other way around, because in this case,
        // we will have called ensureBBExists(), which will have split
        // the existing BB already.
    }

    assert(currentBB);
    return currentBB;
}


BasicBlock *ProcCFG::createIncompleteBB(Address lowAddr)
{
    BasicBlock *newBB = new BasicBlock(lowAddr, m_myProc);
    insertBB(newBB);
    return newBB;
}


bool ProcCFG::ensureBBExists(Address addr, BasicBlock *&currBB)
{
    // check for overlapping incomplete or complete BBs.
    BBStartMap::iterator itExistingBB = m_bbStartMap.lower_bound(addr);

    BasicBlock *overlappingBB = nullptr;
    if (itExistingBB != m_bbStartMap.end() && itExistingBB->second->getLowAddr() == addr) {
        overlappingBB = itExistingBB->second;
    }
    else if (itExistingBB != m_bbStartMap.begin()) {
        --itExistingBB;
        if (itExistingBB->second->getLowAddr() <= addr &&
            itExistingBB->second->getHiAddr() >= addr) {
            overlappingBB = itExistingBB->second;
        }
    }

    if (!overlappingBB) {
        // no BB at addr -> create a new incomplete BB
        createIncompleteBB(addr);
        return false;
    }
    else if (overlappingBB->isIncomplete()) {
        return false;
    }
    else if (overlappingBB && overlappingBB->getLowAddr() < addr) {
        splitBB(overlappingBB, addr);
        BasicBlock *highBB = getBBStartingAt(addr);

        if (currBB == overlappingBB) {
            // This means that the BB that we are expecting to use, usually to add
            // out edges, has changed. We must change this pointer so that the right
            // BB gets the out edges. However, if the new BB is not the BB of
            // interest, we mustn't change currBB
            currBB = highBB;
        }
        return true;
    }
    else {
        // addr is the start of a complete BB
        return true;
    }
}


bool ProcCFG::isStartOfBB(Address addr) const
{
    return getBBStartingAt(addr) != nullptr;
}


bool ProcCFG::isStartOfIncompleteBB(Address addr) const
{
    const BasicBlock *bb = getBBStartingAt(addr);

    return bb && bb->isIncomplete();
}


void ProcCFG::setEntryAndExitBB(BasicBlock *entryBB)
{
    m_entryBB = entryBB;

    for (BasicBlock *bb : *this) {
        if (bb->getType() == BBType::Ret) {
            m_exitBB = bb;
            return;
        }
    }

    // It is possible that there is no exit BB
}


void ProcCFG::removeBB(BasicBlock *bb)
{
    if (bb == nullptr) {
        return;
    }

    BBStartMap::iterator firstIt, lastIt;
    std::tie(firstIt, lastIt) = m_bbStartMap.equal_range(bb->getLowAddr());

    for (auto it = firstIt; it != lastIt; ++it) {
        if (it->second == bb) {
            m_bbStartMap.erase(it);
            delete bb;
            return;
        }
    }

    LOG_WARN("Tried to remove BB at address %1; does not exist in CFG", bb->getLowAddr());
    delete bb;
}


void ProcCFG::addEdge(BasicBlock *sourceBB, BasicBlock *destBB)
{
    if (!sourceBB || !destBB) {
        return;
    }

    // Wire up edges
    sourceBB->addSuccessor(destBB);
    destBB->addPredecessor(sourceBB);

    // special handling for upgrading oneway BBs to twoway BBs
    if ((sourceBB->getType() == BBType::Oneway) && (sourceBB->getNumSuccessors() > 1)) {
        sourceBB->setType(BBType::Twoway);
    }
}


void ProcCFG::addEdge(BasicBlock *sourceBB, Address addr)
{
    // If we already have a BB for this address, add the edge to it.
    // If not, create a new incomplete BB at the destination address.
    BasicBlock *destBB = getBBStartingAt(addr);

    if (!destBB) {
        destBB = createIncompleteBB(addr);
    }

    this->addEdge(sourceBB, destBB);
}


bool ProcCFG::isWellFormed() const
{
    for (const BasicBlock *bb : *this) {
        if (bb->isIncomplete()) {
            m_wellFormed = false;
            LOG_ERROR("CFG is not well formed: BB at address %1 is incomplete", bb->getLowAddr());
            return false;
        }
        else if (bb->getFunction() != m_myProc) {
            m_wellFormed = false;
            LOG_ERROR("CFG is not well formed: BB at address %1 does not belong to proc '%2'",
                      bb->getLowAddr(), m_myProc->getName());
            return false;
        }

        for (const BasicBlock *pred : bb->getPredecessors()) {
            if (!pred->isPredecessorOf(bb)) {
                m_wellFormed = false;
                LOG_ERROR("CFG is not well formed: Edge from BB at %1 to BB at %2 is malformed.",
                          pred->getLowAddr(), bb->getLowAddr());
                return false;
            }
            else if (pred->getFunction() != bb->getFunction()) {
                m_wellFormed = false;
                LOG_ERROR("CFG is not well formed: Interprocedural edge from '%1' to '%2' found",
                          pred->getFunction() ? "<invalid>" : pred->getFunction()->getName(),
                          bb->getFunction()->getName());
                return false;
            }
        }

        for (const BasicBlock *succ : bb->getSuccessors()) {
            if (!succ->isSuccessorOf(bb)) {
                m_wellFormed = false;
                LOG_ERROR("CFG is not well formed: Edge from BB at %1 to BB at %2 is malformed.",
                          bb->getLowAddr(), succ->getLowAddr());
                return false;
            }
            else if (succ->getFunction() != bb->getFunction()) {
                m_wellFormed = false;
                LOG_ERROR("CFG is not well formed: Interprocedural edge from '%1' to '%2' found",
                          bb->getFunction()->getName(),
                          succ->getFunction() ? "<invalid>" : succ->getFunction()->getName());
                return false;
            }
        }
    }

    m_wellFormed = true;
    return true;
}


void ProcCFG::simplify()
{
    LOG_VERBOSE("Simplifying CFG ...");

    for (BasicBlock *bb : *this) {
        bb->simplify();
    }
}


BasicBlock *ProcCFG::findRetNode()
{
    BasicBlock *retNode = nullptr;

    for (BasicBlock *bb : *this) {
        if (bb->getType() == BBType::Ret) {
            return bb;
        }
        else if (bb->getType() == BBType::Call) {
            const Function *callee = bb->getCallDestProc();
            if (callee && !callee->isLib() && callee->isNoReturn()) {
                retNode = bb; // use noreturn calls if the proc does not return
            }
        }
    }

    return retNode;
}


Statement *ProcCFG::findOrCreateImplicitAssign(SharedExp exp)
{
    ExpStatementMap::iterator it = m_implicitMap.find(exp);
    if (it != m_implicitMap.end()) {
        // implicit already present, use it
        assert(it->second);
        return it->second;
    }

    if (!m_entryBB) {
        return nullptr;
    }

    // A use with no explicit definition. Create a new implicit assignment
    exp                 = exp->clone(); // In case the original gets changed
    ImplicitAssign *def = m_entryBB->addImplicitAssign(exp);

    // Remember it for later so we don't insert more than one implicit assignment for any one
    // location We don't clone the copy in the map. So if the location is a m[...], the same type
    // information is available in the definition as at all uses
    m_implicitMap[exp] = def;

    return def;
}


Statement *ProcCFG::findTheImplicitAssign(const SharedConstExp &x) const
{
    // As per the above, but don't create an implicit if it doesn't already exist
    ExpStatementMap::const_iterator it = m_implicitMap.find(std::const_pointer_cast<Exp>(x));
    return (it != m_implicitMap.end()) ? it->second : nullptr;
}


Statement *ProcCFG::findImplicitParamAssign(Parameter *param)
{
    // As per the above, but for parameters (signatures don't get updated with opParams)
    SharedExp paramExp = param->getExp();

    ExpStatementMap::iterator it = std::find_if(
        m_implicitMap.begin(), m_implicitMap.end(),
        [paramExp](const std::pair<const SharedConstExp &, Statement *> &val) {
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
    Statement *ia = it->second;
    m_implicitMap.erase(it);       // Delete the mapping
    m_myProc->removeStatement(ia); // Remove the actual implicit assignment statement as well
}


BasicBlock *ProcCFG::splitBB(BasicBlock *bb, Address splitAddr, BasicBlock *_newBB /* = 0 */)
{
    RTLList::iterator splitIt;

    // First find which RTL has the split address; note that this could fail
    // (e.g. jump into the middle of an instruction, or some weird delay slot effects)
    for (splitIt = bb->getRTLs()->begin(); splitIt != bb->getRTLs()->end(); ++splitIt) {
        if ((*splitIt)->getAddress() == splitAddr) {
            break;
        }
    }

    if (splitIt == bb->getRTLs()->end()) {
        LOG_WARN("Cannot split BB at address %1 at split address %2", bb->getLowAddr(), splitAddr);
        return bb;
    }

    if (_newBB && !_newBB->isIncomplete()) {
        // we already have a BB for the high part. Delete overlapping RTLs and adjust edges.

        while (splitIt != bb->getRTLs()->end()) {
            splitIt = bb->getRTLs()->erase(splitIt); // deletes RTLs
        }

        bb->updateBBAddresses();
        _newBB->updateBBAddresses();

        _newBB->removeAllPredecessors();
        for (BasicBlock *succ : bb->getSuccessors()) {
            succ->removePredecessor(bb);
        }
        bb->removeAllSuccessors();
        addEdge(bb, _newBB);
        bb->setType(BBType::Fall);
        insertBB(_newBB);
        return _newBB;
    }
    else if (!_newBB) {
        // create a new incomplete BasicBlock.
        _newBB = createIncompleteBB(splitAddr);
    }

    // Now we have an incomplete BB at splitAddr;
    // just complete it with the "high" RTLs from the original BB.
    // We don't want to "deep copy" the RTLs themselves,
    // because we want to transfer ownership from the original BB to the "high" part
    std::unique_ptr<RTLList> highRTLs(new RTLList);
    for (RTLList::iterator it = splitIt; it != bb->getRTLs()->end();) {
        highRTLs->push_back(std::move(*it));
        assert(*it == nullptr);
        it = bb->getRTLs()->erase(it);
    }

    _newBB->setRTLs(std::move(highRTLs));
    bb->updateBBAddresses();
    _newBB->updateBBAddresses();

    assert(_newBB->getNumPredecessors() == 0);
    assert(_newBB->getNumSuccessors() == 0);

    const std::vector<BasicBlock *> &successors = bb->getSuccessors();
    for (BasicBlock *succ : successors) {
        succ->removePredecessor(bb);
        succ->addPredecessor(_newBB);
        _newBB->addSuccessor(succ);
    }

    bb->removeAllSuccessors();
    addEdge(bb, _newBB);
    _newBB->setType(bb->getType());
    bb->setType(BBType::Fall);
    return _newBB;
}


void ProcCFG::print(OStream &out) const
{
    out << "Control Flow Graph:\n";

    for (BasicBlock *bb : *this) {
        bb->print(out);
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


void ProcCFG::insertBB(BasicBlock *bb)
{
    assert(bb != nullptr);
    assert(bb->getLowAddr() != Address::INVALID);
    if (bb->getLowAddr() != Address::ZERO) {
        auto it = m_bbStartMap.find(bb->getLowAddr());
        if (it != m_bbStartMap.end()) {
            // replace it
            it->second = bb;
        }
        else {
            // just insert it
            m_bbStartMap.insert({ bb->getLowAddr(), bb });
        }
    }
    else {
        // this is an orpahned BB (e.g. delay slot)
        m_bbStartMap.insert({ Address::ZERO, bb });
    }
}
