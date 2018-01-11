#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CFG.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/IndirectJumpAnalyzer.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/util/Types.h"
#include "boomerang/util/Log.h"
#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/util/Util.h"
#include <algorithm>
#include <cassert>
#include <cstring>


Cfg::Cfg(UserProc *proc)
    : m_myProc(proc)
    , m_wellFormed(true)
    , m_implicitsDone(false)
    , m_entryBB(nullptr)
    , m_exitBB(nullptr)
{
}


Cfg::~Cfg()
{
    for (BasicBlock *bb : m_listBB) {
        delete bb;
    }
}


void Cfg::clear()
{
    // Don't delete the BBs; this will delete any CaseStatements we want to save for the re-decode.
    // Just let them leak since we do not use a garbage collection any more.
    // A better idea would be to save the CaseStatements explicitly and delete the BBs afterwards.
    // But this has to wait until the decoder redesign.

    m_listBB.clear();
    m_bbStartMap.clear();
    m_implicitMap.clear();
    m_entryBB    = nullptr;
    m_exitBB     = nullptr;
    m_wellFormed = true;
}


void Cfg::setEntryAndExitBB(BasicBlock *entryBB)
{
    m_entryBB = entryBB;

    for (BasicBlock *bb : m_listBB) {
        if (bb->getType() == BBType::Ret) {
            m_exitBB = bb;
            return;
        }
    }

    // It is possible that there is no exit BB
}


BasicBlock *Cfg::createBB(BBType bbType, std::unique_ptr<RTLList> bbRTLs)
{
    // First find the native address of the first RTL
    // Can't use BasicBlock::GetLowAddr(), since we don't yet have a BB!
    Address startAddr = bbRTLs->front()->getAddress();

    // If this is zero, try the next RTL (only). This may be necessary if e.g. there is a BB with a delayed branch only,
    // with its delay instruction moved in front of it (with 0 address).
    // Note: it is possible to see two RTLs with zero address with Sparc: jmpl %o0, %o1. There will be one for the delay
    // instr (if not a NOP), and one for the side effect of copying %o7 to %o1.
    // Note that orphaned BBs (for which we must compute addr here to to be 0) must not be added to the map, but they
    // have no RTLs with a non zero address.
    if (startAddr.isZero() && (bbRTLs->size() > 1)) {
        std::list<RTL *>::iterator next = std::next(bbRTLs->begin());
        startAddr = (*next)->getAddress();
    }

    // If this addr is non zero, check the map to see if we have a (possibly incomplete) BB here already
    // If it is zero, this is a special BB for handling delayed branches or the like
    bool bDone = false;
    BBStartMap::iterator mi = m_bbStartMap.end();
    BasicBlock      *currentBB = nullptr;

    if (!startAddr.isZero()) {
        mi = m_bbStartMap.find(startAddr);

        if ((mi != m_bbStartMap.end()) && (*mi).second) {
            currentBB = (*mi).second;

            // It should be incomplete, or the pBB there should be zero (we have called Label but not yet created the BB
            // for it).  Else we have duplicated BBs. Note: this can happen with forward jumps into the middle of a
            // loop, so not error
            if (!currentBB->isIncomplete()) {
                // This list of RTLs is not needed now
                qDeleteAll(*bbRTLs);

                LOG_VERBOSE("Not creating a BB at address %1 because a BB already exists", currentBB->getLowAddr());
                return nullptr;
            }
            else {
                // Fill in the details, and return it
                currentBB->setRTLs(std::move(bbRTLs));
                currentBB->setType(bbType);
            }

            bDone = true;
        }
    }

    if (!bDone) {
        // Else add a new BB to the back of the current list.
        currentBB = new BasicBlock(bbType, std::move(bbRTLs), m_myProc);
        m_listBB.push_back(currentBB);

        // Also add the address to the map from native (source) address to
        // pointer to BB, unless it's zero
        if (!startAddr.isZero()) {
            m_bbStartMap[startAddr] = currentBB; // Insert the mapping
            mi = m_bbStartMap.find(startAddr);
        }
    }

    if (!startAddr.isZero() && (mi != m_bbStartMap.end())) {
        //
        //  Existing   New         +---+ Top of new
        //            +---+        +---+
        //            |   |          \/ Fall through
        //    +---+   |   | =>     +---+
        //    |   |   |   |        |   | Existing; rest of new discarded
        //    +---+   +---+        +---+
        //
        // Check for overlap of the just added BB with the next BB (address wise).  If there is an overlap, truncate the
        // std::list<Exp*> for the new BB to not overlap, and make this a fall through BB.
        // We still want to do this even if the new BB overlaps with an incomplete BB, though in this case,
        // splitBB needs to fill in the details for the "bottom" BB of the split.
        // Also, in this case, we return a pointer to the newly completed BB, so it will get out edges added
        // (if required). In the other case (i.e. we overlap with an existing, completed BB), we want to return 0, since
        // the out edges are already created.
        //
        mi = std::next(mi);

        if (mi != m_bbStartMap.end()) {
            BasicBlock *nextBB          = (*mi).second;
            Address    nextAddr         = (*mi).first;
            bool       nextIsIncomplete = nextBB->isIncomplete();

            if (nextAddr <= currentBB->getRTLs()->back()->getAddress()) {
                // Need to truncate the current BB. We use splitBB(), but pass it pNextBB so it doesn't create a new BB
                // for the "bottom" BB of the split pair
                splitBB(currentBB, nextAddr, nextBB);

                // If the overlapped BB was incomplete, return the "bottom" part of the BB, so adding out edges will
                // work properly.
                if (nextIsIncomplete) {
                    assert(nextBB);
                    return nextBB;
                }

                LOG_VERBOSE("Not creating a BB at address %1 because a BB already exists", currentBB->getLowAddr());
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
        // Note: no need to check the other way around, because in this case, we will have called Cfg::Label(), and it
        // will have split the existing BB already.
    }

    assert(currentBB);
    return currentBB;
}


BasicBlock *Cfg::createIncompleteBB(Address lowAddr)
{
    // Create a new (basically empty) BB
    BasicBlock *bb = new BasicBlock(lowAddr, m_myProc);

    // Add it to the list
    m_listBB.push_back(bb);
    m_bbStartMap[lowAddr] = bb; // Insert the mapping
    return bb;
}


void Cfg::addEdge(BasicBlock *sourceBB, BasicBlock *destBB)
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


void Cfg::addEdge(BasicBlock *sourceBB, Address addr)
{
    // If we already have a BB for this address, add the edge to it.
    // If not, create a new incomplete BB at the destination address.
    BasicBlock *destBB = getBBStartingAt(addr);

    if (!destBB) {
        destBB = createIncompleteBB(addr);
    }

    this->addEdge(sourceBB, destBB);
}


bool Cfg::isStartOfBB(Address addr) const
{
    return getBBStartingAt(addr) != nullptr;
}


BasicBlock *Cfg::splitBB(BasicBlock *bb, Address splitAddr, BasicBlock *_newBB /* = 0 */,
                         bool deleteRTLs /* = false */)
{
    RTLList::iterator ri;

    // First find which RTL has the split address; note that this could fail (e.g. label in the middle of an
    // instruction, or some weird delay slot effects)
    for (ri = bb->getRTLs()->begin(); ri != bb->getRTLs()->end(); ri++) {
        if ((*ri)->getAddress() == splitAddr) {
            break;
        }
    }

    if (ri == bb->getRTLs()->end()) {
        LOG_WARN("Cannot split BB at address %1 at split address %2", bb->getLowAddr(), splitAddr);
        return bb;
    }

    // If necessary, set up a new basic block with information from the original bb
    if (_newBB == nullptr) {
        _newBB = new BasicBlock(*bb);

        // But we don't want the top BB's in edges; our only in-edge should be the out edge from the top BB
        _newBB->removeAllPredecessors();

        // The "bottom" BB now starts at the implicit label, so we create a new list
        // that starts at ri. We need a new list, since it is different from the
        // original BB's list. We don't have to "deep copy" the RTLs themselves,
        // since they will never overlap
        _newBB->setRTLs(Util::makeUnique<RTLList>(ri, bb->getRTLs()->end()));
        m_listBB.push_back(_newBB); // Put it in the graph

        // Put the implicit label into the map. Need to do this before the addOutEdge() below
        m_bbStartMap[splitAddr] = _newBB;
    }
    else if (_newBB->isIncomplete()) {
        // We have an existing BB and a map entry, but no details except for
        // in-edges and m_bHasLabel.
        // First save the in-edges and m_iLabelNum
        std::vector<BasicBlock *> oldPredecessors(_newBB->getPredecessors());

        // Copy over the details now, completing the bottom BB
        *_newBB = *bb;              // Assign the BB, copying fields.

        // Replace the in edges (likely only one)
        for (BasicBlock *pred : oldPredecessors) {
            _newBB->addPredecessor(pred);
        }

        _newBB->setRTLs(Util::makeUnique<RTLList>(ri, bb->getRTLs()->end()));
    }

    // else pNewBB exists and is complete. We don't want to change the complete
    // BB in any way, except to later add one in-edge
    bb->setType(BBType::Fall); // Update original ("top") basic block's info and make it a fall-through

    // Fix the in-edges of pBB's descendants. They are now pNewBB
    // Note: you can't believe m_iNumOutEdges at the time that this function may
    // get called
    for (BasicBlock *succ : bb->getSuccessors()) {
        // Search through the in edges for pBB (old ancestor)
        int k;

        for (k = 0; k < succ->getNumPredecessors(); k++) {
            if (succ->getPredecessor(k) == bb) {
                // Replace with a pointer to the new predecessor
                succ->setPredecessor(k, _newBB);
                break;
            }
        }

        // That pointer should have been found!
        assert(k < succ->getNumPredecessors());
    }

    // The old BB needs to have part of its list of RTLs erased, since the
    // instructions overlap
    if (deleteRTLs) {
        // Delete the list of pointers, and also the RTLs they point to
        qDeleteAll(ri, bb->getRTLs()->end());
    }

    bb->getRTLs()->erase(ri, bb->getRTLs()->end());
    bb->updateBBAddresses();

    // Erase any existing out edges
    bb->removeAllSuccessors();
    addEdge(bb, splitAddr);
    return _newBB;
}




bool Cfg::label(Address uNativeAddr, BasicBlock *& pCurBB)
{
    BBStartMap::iterator mi, newi;

    mi = m_bbStartMap.find(uNativeAddr);     // check if the native address is in the map already (explicit label)

    if (mi == m_bbStartMap.end()) {          // not in the map
                                        // If not an explicit label, temporarily add the address to the map
        m_bbStartMap[uNativeAddr] = nullptr; // no PBB yet
                                        // get an iterator to the new native address and check if the previous
                                        // element in the (sorted) map overlaps this new native address; if so,
                                        // it's a non-explicit label which needs to be made explicit by
                                        // splitting the previous BB.
        mi   = m_bbStartMap.find(uNativeAddr);
        newi = mi;
        bool       bSplit   = false;
        BasicBlock *pPrevBB = nullptr;

        if (newi != m_bbStartMap.begin()) {
            pPrevBB = (*--mi).second;

            if (!pPrevBB->isIncomplete() && (pPrevBB->getLowAddr() < uNativeAddr) &&
                (pPrevBB->getHiAddr() >= uNativeAddr)) {
                bSplit = true;
            }
        }

        if (bSplit) {
            // Non-explicit label. Split the previous BB
            BasicBlock *pNewBB = splitBB(pPrevBB, uNativeAddr);

            if (pCurBB == pPrevBB) {
                // This means that the BB that we are expecting to use, usually to add
                // out edges, has changed. We must change this pointer so that the right
                // BB gets the out edges. However, if the new BB is not the BB of
                // interest, we mustn't change pCurBB
                pCurBB = pNewBB;
            }

            return true;  // wasn't a label, but already parsed
        }
        else {            // not a non-explicit label
                          // We don't have to erase this map entry. Having a null BasicBlock
                          // pointer is coped with in newBB() and addOutEdge(); when eventually
                          // the BB is created, it will replace this entry.  We should be
                          // currently processing this BB. The map will be corrected when newBB is
                          // called with this address.
            return false; // was not already parsed
        }
    }
    else {               // We already have uNativeAddr in the map
        if ((*mi).second && !(*mi).second->isIncomplete()) {
            return true; // There is a complete BB here. Return true.
        }

        // We are finalising an incomplete BB. Still need to check previous map
        // entry to see if there is a complete BB overlapping
        bool       bSplit = false;
        BasicBlock *pPrevBB = nullptr, *pBB = (*mi).second;

        if (mi != m_bbStartMap.begin()) {
            pPrevBB = (*--mi).second;

            if (!pPrevBB->isIncomplete() && (pPrevBB->getLowAddr() < uNativeAddr) &&
                (pPrevBB->getHiAddr() >= uNativeAddr)) {
                bSplit = true;
            }
        }

        if (bSplit) {
            // Pass the third parameter to splitBB, because we already have an
            // (incomplete) BB for the "bottom" BB of the split
            splitBB(pPrevBB, uNativeAddr, pBB); // non-explicit label
            return true;                        // wasn't a label, but already parsed
        }

        // A non overlapping, incomplete entry is in the map.
        return false;
    }
}


bool Cfg::isStartOfIncompleteBB(Address uAddr) const
{
    const BasicBlock *bb = getBBStartingAt(uAddr);

    return bb && bb->isIncomplete();
}


void Cfg::sortByAddress()
{
    m_listBB.sort([] (const BasicBlock *bb1, const BasicBlock *bb2) {
        return bb1->getLowAddr() < bb2->getLowAddr();
    });
}


bool Cfg::isWellFormed() const
{
    for (const BasicBlock *bb : *this) {
        if (bb->isIncomplete()) {
            m_wellFormed = false;
            LOG_VERBOSE("CFG is not well formed: BB at address %1 is incomplete", bb->getLowAddr());
            return false;
        }
        else if (bb->getFunction() != m_myProc) {
            m_wellFormed = false;
            LOG_VERBOSE("CFG is not well formed: BB at address %1 does not belong to proc '%2'",
                        bb->getLowAddr(), m_myProc->getName());
            return false;
        }

        for (const BasicBlock *pred : bb->getPredecessors()) {
            if (!pred->isPredecessorOf(bb)) {
                m_wellFormed = false;
                LOG_VERBOSE("CFG is not well formed: Edge from BB at %1 to BB at %2 is malformed.",
                            pred->getLowAddr(), bb->getLowAddr());
                return false;
            }
            else if (pred->getFunction() != bb->getFunction()) {
                m_wellFormed = false;
                LOG_VERBOSE("CFG is not well formed: Interprocedural edge from '%1' to '%2' found",
                            pred->getFunction() ? "<invalid>" : pred->getFunction()->getName(),
                            bb->getFunction()->getName());
                return false;
            }
        }

        for (const BasicBlock *succ : bb->getSuccessors()) {
            if (!succ->isSuccessorOf(bb)) {
                m_wellFormed = false;
                LOG_VERBOSE("CFG is not well formed: Edge from BB at %1 to BB at %2 is malformed.",
                            bb->getLowAddr(), succ->getLowAddr());
                return false;
            }
            else if (succ->getFunction() != bb->getFunction()) {
                m_wellFormed = false;
                LOG_VERBOSE("CFG is not well formed: Interprocedural edge from '%1' to '%2' found",
                            bb->getFunction()->getName(),
                            succ->getFunction() ? "<invalid>" : succ->getFunction()->getName());
                return false;
            }
        }
    }

    m_wellFormed = true;
    return true;
}


bool Cfg::mergeBBs(BasicBlock *pb1, BasicBlock *pb2)
{
    // Can only merge if pb1 has only one outedge to pb2, and pb2 has only one in-edge, from pb1. This can only be done
    // after the in-edges are done, which can only be done on a well formed CFG.
    if (!m_wellFormed) {
        return false;
    }

    if (pb1->getNumSuccessors() != 1 || pb2->getNumSuccessors() != 1) {
        return false;
    }

    if (pb1->getSuccessor(0) != pb2 || pb2->getPredecessor(0) != pb1) {
        return false;
    }

    // Merge them! We remove pb1 rather than pb2, since this is also what is needed for many optimisations, e.g. jump to
    // jump.
    completeMerge(pb1, pb2, true);
    return true;
}


void Cfg::completeMerge(BasicBlock *bb1, BasicBlock *bb2, bool bDelete)
{
    // First we replace all of pb1's predecessors' out edges that used to point to pb1 (usually only one of these) with
    // pb2
    for (BasicBlock *pPred : bb1->getPredecessors()) {
        for (int i = 0; i < pPred->getNumSuccessors(); i++) {
            if (pPred->getSuccessor(i) == bb1) {
                pPred->setSuccessor(i, bb2);
            }
        }
    }

    // Now we replace pb2's in edges by pb1's inedges
    bb2->removeAllSuccessors();
    for (BasicBlock *bb1Pred : bb1->getPredecessors()) {
        bb2->addSuccessor(bb1Pred);
    }

    if (bDelete) {
        // Finally, we delete bb1 from the CFG.
        removeBB(bb1);
    }
}


bool Cfg::joinBB(BasicBlock *bb1, BasicBlock *bb2)
{
    // Ensure that the fallthrough case for bb1 is bb2
    if (bb1->getNumSuccessors() != 2 || bb1->getSuccessor(BELSE) != bb2) {
        return false;
    }

    // Prepend the RTLs for pb1 to those of pb2.
    // Since they will be pushed to the front of pb2,
    // push them in reverse order
    for (auto it = bb1->getRTLs()->rbegin(); it != bb1->getRTLs()->rend(); it++) {
        bb2->getRTLs()->push_front(*it);
    }
    bb2->updateBBAddresses();

    completeMerge(bb1, bb2); // Mash them together

    // pb1 no longer needed. Remove it from the list of BBs.
    // This will also delete *pb1. It will be a shallow delete,
    // but that's good because we only did shallow copies to *pb2
    removeBB(bb1);
    return true;
}


void Cfg::removeBB(BasicBlock *bb)
{
    if (bb == nullptr) {
        return;
    }

    iterator bbIt = std::find(m_listBB.begin(), m_listBB.end(), bb);
    if (bbIt == m_listBB.end()) {
        // not found in this CFG
        return;
    }

    if (bb->getLowAddr() != Address::ZERO) {
        m_bbStartMap.erase(bb->getLowAddr());
    }

    m_listBB.erase(bbIt);

    // Actually, removed BBs should be deleted; however,
    // doing so deletes the statements of the BB that seem to be still in use.
    // So don't do it for now.
}


BasicBlock *Cfg::findRetNode()
{
    BasicBlock *retNode = nullptr;

    for (BasicBlock *bb : m_listBB) {
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


void Cfg::simplify()
{
    LOG_VERBOSE("Simplifying CFG ...");

    for (BasicBlock *bb : m_listBB) {
        bb->simplify();
    }
}


void Cfg::print(QTextStream& out, bool html)
{
    out << "Control Flow Graph:\n";

    for (BasicBlock *bb : m_listBB) {
        bb->print(out, html);
    }

    out << '\n';
}


void Cfg::dump()
{
    QTextStream q_cerr(stderr);

    print(q_cerr);
}


void Cfg::dumpImplicitMap()
{
    QTextStream q_cerr(stderr);

    for (auto it : m_implicitMap) {
        q_cerr << it.first << " -> " << it.second << "\n";
    }
}


void Cfg::printToLog()
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    LOG_MSG(tgt);
}



void Cfg::undoComputedBB(Statement *stmt)
{
    for (BasicBlock *bb : *this) {
        if (bb->hasStatement(stmt)) {
            LOG_MSG("undoComputedBB for statement %1", stmt);
            bb->setType(BBType::Call);
            break;
        }
    }
}


Statement *Cfg::findImplicitAssign(SharedExp x)
{
    std::map<SharedExp, Statement *, lessExpStar>::iterator it = m_implicitMap.find(x);

    if (it != m_implicitMap.end()) {
        // implicit already present, use it
        assert(it->second);
        return it->second;
    }

    // A use with no explicit definition. Create a new implicit assignment
    x   = x->clone(); // In case the original gets changed
    Statement *def = new ImplicitAssign(x);
    m_entryBB->prependStmt(def, m_myProc);

    // Remember it for later so we don't insert more than one implicit assignment for any one location
    // We don't clone the copy in the map. So if the location is a m[...], the same type information is available in
    // the definition as at all uses
    m_implicitMap[x] = def;

    return def;
}


Statement *Cfg::findTheImplicitAssign(const SharedExp& x)
{
    // As per the above, but don't create an implicit if it doesn't already exist
    auto it = m_implicitMap.find(x);
    return (it != m_implicitMap.end()) ? it->second : nullptr;
}


Statement *Cfg::findImplicitParamAssign(Parameter *param)
{
    // As per the above, but for parameters (signatures don't get updated with opParams)
    SharedExp n = param->getExp();

    ExpStatementMap::iterator it = std::find_if(m_implicitMap.begin(), m_implicitMap.end(),
        [n] (const std::pair<const SharedExp&, Statement *>& val) {
            return *(val.first) *= *n;
        });

    if (it == m_implicitMap.end()) {
        it = m_implicitMap.find(Location::param(param->getName()));
    }

    return (it != m_implicitMap.end()) ? it->second : nullptr;
}


void Cfg::removeImplicitAssign(SharedExp x)
{
    auto it = m_implicitMap.find(x);

    assert(it != m_implicitMap.end());
    Statement *ia = it->second;
    m_implicitMap.erase(it);          // Delete the mapping
    m_myProc->removeStatement(ia);    // Remove the actual implicit assignment statement as well
}
