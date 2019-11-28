#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/frontend/MachineInstruction.h"
#include "boomerang/ssl/exp/ExpHelp.h"
#include "boomerang/util/Address.h"
#include "boomerang/util/MapIterators.h"

#include <list>
#include <map>
#include <memory>


class BasicBlock;
class Prog;

enum class BBType;


/**
 * Contains all the BasicBlock objects for the whole Prog.
 * These BBs contain all the RTLs for the program, so by traversing the CFG,
 * one traverses the whole program.
 * On architetures with delay stots, there may be more than one BasicBlock per address to simulate
 * the effects of the delay slot(s).
 */
class BOOMERANG_API LowLevelCFG
{
public:
    /// \sa getBBStartingAt
    struct BBStart
    {
        BasicBlock *bb    = nullptr;
        BasicBlock *delay = nullptr;
    };

private:
    typedef std::map<Address, BBStart, std::less<Address>> BBStartMap;

public:
    typedef MapValueIterator<BBStartMap> iterator;
    typedef MapValueConstIterator<BBStartMap> const_iterator;
    typedef MapValueReverseIterator<BBStartMap> reverse_iterator;
    typedef MapValueConstReverseIterator<BBStartMap> const_reverse_iterator;

public:
    /// Creates an empty CFG for the function \p proc
    LowLevelCFG();
    LowLevelCFG(const LowLevelCFG &other) = delete;
    LowLevelCFG(LowLevelCFG &&other)      = default;

    ~LowLevelCFG();

    LowLevelCFG &operator=(const LowLevelCFG &other) = delete;
    LowLevelCFG &operator=(LowLevelCFG &&other) = default;

public:
    /// Note: When removing a BB, the iterator(s) pointing to the removed BB are invalidated.
    iterator begin() { return iterator(m_bbStartMap.begin()); }
    iterator end() { return iterator(m_bbStartMap.end()); }
    const_iterator begin() const { return const_iterator(m_bbStartMap.begin()); }
    const_iterator end() const { return const_iterator(m_bbStartMap.end()); }

    reverse_iterator rbegin() { return reverse_iterator(m_bbStartMap.rbegin()); }
    reverse_iterator rend() { return reverse_iterator(m_bbStartMap.rend()); }
    const_reverse_iterator rbegin() const { return const_reverse_iterator(m_bbStartMap.rbegin()); }
    const_reverse_iterator rend() const { return const_reverse_iterator(m_bbStartMap.rend()); }

public:
    /// \returns the number of (complete and incomplete) BBs in this CFG.
    int getNumBBs() const;

    /**
     * Create a new Basic Block for this CFG.
     * If the BB is blocked by a larger complete BB, the existing BB will be split at the first
     * address of \p bbRTLs; in this case this function returns nullptr (since no BB was created).
     * The case of the new BB being blocked by a smaller complete BB is not handled by this method;
     * use \ref ProcCFG::ensureBBExists instead.
     *
     * The new BB might also be blocked by exising incomplete BBs.
     * If this is the case, the new BB will be split at all blocking incomplete BBs,
     * and fallthrough edges will be added between parts of the split BB.
     * In this case, the incomplete BBs will be removed (since we just completed them).
     *
     * \param bbType Type of the new Basic Block
     * \param bbRTLs RTL list with semantics of all instructions contained in this BB.
     *               Must not be empty.
     *
     * \returns the newly created BB, or the exisitng BB if the new BB is the same as
     * another exising complete BB.
     */
    BasicBlock *createBB(BBType bbType, const std::vector<MachineInstruction> &bbInsns);
    BasicBlock *createBB(BBType bbType, const std::list<MachineInstruction> &bbInsns);

    /**
     * Creates a new incomplete BB at address \p startAddr.
     * Creating an incomplete BB will cause the ProcCFG to not be well-fomed until all
     * incomplete BBs are completed by calling \ref createBB.
     */
    BasicBlock *createIncompleteBB(Address startAddr);

    /**
     * Ensures that \p addr is the start of a complete or incomplete BasicBlock.
     *
     * Explicit labels are addresses that have already been tagged as being labels
     * due to transfers of control to that address (i.e. they are the start of a complete Basic
     * Block). Non explicit labels are addresses that are in the middle of a complete Basic Block.
     * In this case, the existing complete BB is split. If \p currBB is the BB that gets split,
     * \p currBB is updated to point to the "high" part of the split BB (address wise).
     *
     * \param  addr   native (source) address to check
     * \param  currBB See above
     * \returns true if the BB starting at \p address is (now) complete, false otherwise.
     */
    bool ensureBBExists(Address addr, BasicBlock *&currBB);

    /**
     * Get a (complete or incomplete) BasicBlock starting at the given address.
     * If there is no such block, return nullptr.
     */
    inline BBStart getBBStartingAt(Address addr)
    {
        BBStartMap::iterator it = m_bbStartMap.find(addr);
        return (it != m_bbStartMap.end()) ? (*it).second : BBStart{};
    }

    inline const BBStart getBBStartingAt(Address addr) const
    {
        BBStartMap::const_iterator it = m_bbStartMap.find(addr);
        return (it != m_bbStartMap.end()) ? (*it).second : BBStart{};
    }

    /// Check if \p addr is the start of a basic block, complete or not
    bool isStartOfBB(Address addr) const;

    /// Check if the given address is the start of a complete basic block.
    bool isStartOfCompleteBB(Address addr) const;

    /// \returns the entry BB of the procedure of this CFG
    BasicBlock *getEntryBB() { return m_entryBB; }
    const BasicBlock *getEntryBB() const { return m_entryBB; }

    /// Set the entry bb to \p entryBB
    void setEntryBB(BasicBlock *entryBB);

    /// Completely removes a single BB from this CFG.
    /// \note \p bb is invalid after this function returns.
    void removeBB(BasicBlock *bb);

    /**
     * Add an edge from \p sourceBB to \p destBB.
     * \param sourceBB the start of the edge.
     * \param destBB the destination of the edge.
     */
    void addEdge(BasicBlock *sourceBB, BasicBlock *destBB);

    /**
     * Add an out edge from \p sourceBB to address \p destAddr.
     * If \p destAddr is the start of a complete BB, add an edge from \p sourceBB to
     * the complete BB.
     * If \p destAddr is in the middle of a complete BB, the BB will be split; the edge
     * will be added to the "high" part of the split BB.
     * Otherwise, an incomplete BB will be created and the edge will be added to it.
     *
     * \param sourceBB the source of the edge
     * \param destAddr the destination of a CTI (jump or call)
     */
    void addEdge(BasicBlock *sourceBB, Address destAddr);

    /**
     * Checks that all BBs are complete, and all out edges are valid.
     * Also checks that the ProcCFG does not contain interprocedural edges.
     * By definition, the empty CFG is well-formed.
     */
    bool isWellFormed() const;

    BasicBlock *findRetNode();

public:
    /// print this CFG, mainly for debugging
    void print(OStream &out) const;

    QString toString() const;

private:
    /**
     * Split \p bb into a "low" and "high" part at the RTL associated with \p splitAddr.
     * The type of the "low" BB becomes fall-through. The type of the "high" part becomes the type
     * of \p bb.
     *
     * \ | /                    \ | /
     * +---+ bb                 +---+ BB1
     * |   |                    +---+
     * |   |         ==>          |    Fallthrough
     * +---+                    +---+
     * / | \                    +---+ BB2
     *                          / | \
     *
     * If \p splitAddr is not in the range [bb->getLowAddr, bb->getHiAddr], the split fails.
     * \param   bb         pointer to the BB to be split
     * \param   splitAddr  address of RTL to become the start of the new BB
     * \param   newBB      if non zero, it remains as the "bottom" part of the BB, and splitBB only
     * modifies the top part to not overlap. If this is the case, the RTLs of the original BB are
     * deleted. \returns If the merge is successful, returns the "high" part of the split BB.
     * Otherwise, returns the original BB.
     */
    BasicBlock *splitBB(BasicBlock *bb, Address splitAddr, BasicBlock *newBB = nullptr);

    void insertBB(BasicBlock *bb);

private:
    BasicBlock *m_entryBB = nullptr; ///< The BB corresponding to the entry point of the program.
    BBStartMap m_bbStartMap;         ///< The Address to BB map
};
