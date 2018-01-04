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


#include "boomerang/db/exp/ExpHelp.h"
#include "boomerang/util/Address.h"
#include "boomerang/db/LivenessAnalyzer.h"

#include <list>
#include <vector>
#include <set>
#include <map>


class Function;
class Prog;
class UserProc;
class UseSet;
class LocationSet;
class SSACounts;
class BasicBlock;
class ICodeGenerator;
class CallStatement;
class BranchStatement;
class RTL;
class Global;
class Parameter;
class ConnectionGraph;
class Statement;

enum class BBType;

using RTLList   = std::list<RTL *>;

#define BTHEN    0
#define BELSE    1


/**
 * Control Flow Graph class. Contains all the BasicBlock objects for a procedure.
 * These BBs contain all the RTLs for the procedure, so by traversing the Cfg,
 * one traverses the whole procedure.
 */
class Cfg
{
    typedef std::map<SharedExp, Statement *, lessExpStar>          ExpStatementMap;

    // A type for the Address to BB map
    typedef std::map<Address, BasicBlock *, std::less<Address> >   MAPBB;

public:
    typedef std::list<BasicBlock *>::iterator                      iterator;
    typedef std::list<BasicBlock *>::const_iterator                const_iterator;

    class BBAlreadyExistsError : public std::exception
    {
    public:
        BasicBlock *pBB;
        BBAlreadyExistsError(BasicBlock *_pBB)
            : pBB(_pBB) {}
    };

public:
    /// Creates an empty CFG for the function \p proc
    Cfg(UserProc *proc);
    Cfg(const Cfg& other) = delete;
    Cfg(Cfg&& other) = default;

    ~Cfg();

    Cfg& operator=(const Cfg& other) = delete;
    Cfg& operator=(Cfg&& other) = default;

public:
    /// Remove all basic blocks from the CFG
    void clear();

    /// \returns the number of BBs in this CFG.
    size_t getNumBBs() const { return m_listBB.size(); }

    /// Checks if the BB is part of this CFG
    bool hasBB(const BasicBlock *bb) const { return std::find(m_listBB.begin(), m_listBB.end(), bb) != m_listBB.end(); }

    /**
     * Add a new basic block to this cfg.
     *
     * Checks to see if the address associated with pRtls is already in the map as an incomplete BB; if so, it is
     * completed now and a pointer to that BB is returned. Otherwise, allocates memory for a new basic block node,
     * initializes its list of RTLs with pRtls, its type to the given type.
     * The native address associated with the start of the BB is taken from pRtls, and added to the map (unless 0).
     * If the native address of the new BB already belongs to a BB the existing BB is split,
     * and an exception is thrown.
     *
     * \note You cannot assume that the returned BB will have the RTL associated with pStart as its first RTL, since
     * the BB could be split. You can however assume that the returned BB is suitable for adding out edges (i.e. if
     * the BB is split, you get the "bottom" part of the BB, not the "top" (with lower addresses at the "top").
     * Returns nullptr if not successful, or if there already exists a completed BB at this address (this can happen
     * with certain kinds of forward branches).
     *
     * \param pRtls list of pointers to RTLs to initialise the BB with bbType: the type of the BB (e.g. TWOWAY)
     * \param bbType - type of new BasicBlock
     * \returns Pointer to the newly created BB (non-null)
     */
    BasicBlock *createBB(std::unique_ptr<RTLList> pRtls, BBType bbType) noexcept (false);

    /**
     * Allocates space for a new, incomplete BB, and the given address is added to the map.
     * This BB will have to be completed before calling WellFormCfg.
     *
     * Use this function when there are outedges to BBs that are not created yet. Usually used via addOutEdge()
     * This function will commonly be called via addOutEdge()
     */
    BasicBlock *createIncompleteBB(Address addr);

    /**
     * Get a BasicBlock starting at the given address.
     * If there is no such block, return nullptr.
     */
    inline BasicBlock *getBB(Address addr)
    {
        MAPBB::iterator it = m_mapBB.find(addr);
        return (it != m_mapBB.end()) ? (*it).second : nullptr;
    }

    inline const BasicBlock *getBB(Address addr) const
    {
        MAPBB::const_iterator it = m_mapBB.find(addr);
        return (it != m_mapBB.end()) ? (*it).second : nullptr;
    }

    /**
     * Add an out edge to \p sourceBB to address \p destAddr.
     *
     * Adds an out-edge to the basic block \p sourceBB.
     * \note An address is given here; the out edge will be filled in as a pointer to a BB.
     * An incomplete BB will be created if required.
     * If \p labelRequired is true, the destination BB will have its "label required" bit set.
     *
     * \param sourceBB  Source BB (to have the out edge added to)
     * \param destAddr  Destination BB of the out edge
     * \param labelRequired if true, set a label at the destination address. Set true on "true" branches of labels
     */
    void addEdge(BasicBlock *sourceBB, Address destAddr);

    /**
     * Add an edge between \p sourceBB and destBB.
     *
     * Adds an out-edge to the basic block \p sourceBB and an in-edge
     * to the basic block \p destBB
     *
     * \param sourceBB source BB (to have the out edge added to)
     * \param destBB Start address of the BB reached by the out edge
     * \param labelRequired - indicates that label is required in the destination BB
     */
    void addEdge(BasicBlock *sourceBB, BasicBlock *destBB);

    /**
     * Get the first BB of this CFG.
     * Gets a pointer to the first BB this cfg. Also initialises `it' so that calling GetNextBB will return the
     * second BB, etc.  Also, *it is the first BB.  Returns null if there are no BBs this CFG.
     *
     * \param       it set to an value that must be passed to getNextBB
     * \returns     Pointer to the first BB this cfg, or nullptr if none
     */
    BasicBlock *getFirstBB(iterator& it);
    const BasicBlock *getFirstBB(const_iterator& it) const;

    /**
     * Gets a pointer to the next BB this cfg.
     * \p it must be from a call to GetFirstBB(), or from a subsequent call to GetNextBB().
     * Also, *it is the current BB. Returns nullptr if there are no more BBs this CFG.
     *
     * \param   it - iterator from a call to getFirstBB or getNextBB
     * \returns pointer to the BB, or nullptr if no more
     */
    BasicBlock *getNextBB(iterator& it);
    const BasicBlock *getNextBB(const_iterator& it) const;

    /*
     * An alternative to the above is to use begin() and end():
     */
    iterator begin() { return m_listBB.begin(); }
    iterator end()   { return m_listBB.end(); }

    /* Checks whether the given native address is a label (explicit or non explicit) or not.  Explicit labels are
     * addresses that have already been tagged as being labels due to transfers of control to that address.
     * Non explicit labels are those that belong to basic blocks that have already been constructed (i.e. have
     * previously been parsed) and now need to be made explicit labels.     In the case of non explicit labels, the
     * basic block is split into two and types and edges are adjusted accordingly. pNewBB is set to the lower part
     * of the split BB.
     * Returns true if the native address is that of an explicit or non explicit label, false otherwise. */

    /**
     * Checks whether the given native address is a label (explicit or non explicit) or not.
     * Returns false for incomplete BBs.
     * So it returns true iff the address has already been decoded in some BB. If it was not
     * already a label (i.e. the first instruction of some BB), the BB is split so that it becomes a label.
     * Explicit labels are addresses that have already been tagged as being labels due to transfers of control
     * to that address, and are therefore the start of some BB.
     * Non explicit labels are those that belong to basic blocks that have already been constructed
     * (i.e. have previously been parsed) and now need to be made explicit labels.
     * In the case of non explicit labels, the basic block is split into two and types and edges
     * are adjusted accordingly.
     * If \p pNewBB is the BB that gets split, it is changed to point to the
     * address of the new (lower) part of the split BB.
     * If there is an incomplete entry in the table for this address which overlaps with a completed address,
     * the completed BB is split and the BB for this address is completed.
     *
     * \param         addr   native (source) address to check
     * \param         pNewBB See above
     * \returns       True if \p addr is a label, i.e. (now) the start of a BB
     *                Note: \p pNewBB may be modified (as above)
     */
    bool label(Address addr, BasicBlock *& pNewBB);

    /// Check if the given address is the start of an incomplete basic block.
    bool isIncomplete(Address addr) const;

    /**
     * Check if \p addr is the start of a basic block, complete or not
     * \note must ignore entries with a null BB, since these are caused by
     * calls to Label that failed, i.e. the instruction is not decoded yet.
     */
    bool existsBB(Address addr) const;

    /// Check if the BasicBlock is in this graph
    bool existsBB(const BasicBlock *bb) const { return std::find(m_listBB.begin(), m_listBB.end(), bb) != m_listBB.end(); }

    /**
     * Sorts the BBs in the CFG according to the low address of each BB.
     * Useful because it makes printouts easier, if they used iterators
     * to traverse the list of BBs.
     */
    void sortByAddress();

    /**
     * Checks that all BBs are complete, and all out edges are valid;
     * however, Addresses that are interprocedural out edges are not checked or changed.
     */
    bool isWellFormed() const;

    /**
     * Given two basic blocks that belong to a well-formed graph,
     * merges the second block onto the first one and returns the new block.
     * The in and out edges links are updated accordingly.
     * Note that two basic blocks can only be merged if each
     * has a unique out-edge and in-edge respectively,
     * and these edges correspond to each other.
     *
     * \returns true if the blocks were merged.
     */
    bool mergeBBs(BasicBlock *bb1, BasicBlock *bb2);

    /**
     * Given a well-formed cfg, optimizations are performed on the graph
     * to reduce the number of basic blocks and edges.
     *
     * Optimizations performed are:
     *  - Removal of redundant jumps (e.g. remove J in A->J->B if J only contains a jump)
     *
     * \returns true iff successful.
     */
    bool compressCfg();

    /// Check if is a BB at the address given
    /// whose first RTL is an orphan, i.e. getAddress() returns 0.
    bool isOrphan(Address uAddr) const;

    /**
     * Amalgamate the RTLs for \p bb1 and  \p bb2, and place the result into \p bb2
     *
     * This is called where a two-way branch is deleted, thereby joining a two-way BB with it's successor.
     * This happens for example when transforming Intel floating point branches, and a branch on parity is deleted.
     * The joined BB becomes the type of the successor.
     *
     * \note Assumes that fallthrough of *pb1 is *pb2
     *
     * \param   bb1,bb2 pointers to the BBs to join
     * \returns True if successful
     */
    bool joinBB(BasicBlock *bb1, BasicBlock *bb2);

    /// Completely removes a single BB from this CFG.
    void removeBB(BasicBlock *bb);

    /// return a BB given an address
    BasicBlock *bbForAddr(Address addr) { return m_mapBB[addr]; }

    /// Simplify all the expressions in the CFG
    void simplify();

    /// Change the BB enclosing stmt to be CALL, not COMPCALL
    void undoComputedBB(Statement *stmt);

private:
    /**
     * Split the given basic block at the RTL associated with \p splitAddr. The first node's type becomes
     * fall-through and ends at the RTL prior to that associated with \p splitAddr.
     * The second node's type becomes the type of the original basic block (\p bb),
     * and its out-edges are those of the original basic block.
     * In edges of the new BB's descendants are changed.
     *
     * \pre assumes \p splitAddr is an address within the boundaries of the given BB.
     *
     * \param   bb         pointer to the BB to be split
     * \param   splitAddr  address of RTL to become the start of the new BB
     * \param   newBB      if non zero, it remains as the "bottom" part of the BB, and splitBB only modifies the top part
     *                     to not overlap.
     * \param   deleteRTLs if true, deletes the RTLs removed from the existing BB after the split point. Only used if
     *                     there is an overlap with existing instructions
     * \returns A pointer to the "bottom" (new) part of the split BB.
     */
    BasicBlock *splitBB(BasicBlock *bb, Address splitAddr, BasicBlock *newBB = nullptr, bool deleteRTLs = false);

    /**
     * Complete the merge of two BBs by adjusting in and out edges.
     * No checks are made that the merge is valid (hence this is a private function).
     *
     * \param bb1,bb2 pointers to the two BBs to merge
     * \param deleteBB if true, \p bb1 is deleted as well
     */
    void completeMerge(BasicBlock *bb1, BasicBlock *bb2, bool deleteBB = false);

    /**
     * Check if the procedure associated with the BB has an entry BB.
     * \returns false if the procedure has an entry bb, true otherwise
     */
    bool hasNoEntryBB();

public:
    /**
     * Split the given BB at the RTL given, and turn it into the BranchStatement given. Sort out all the in and out
     * edges.
     */

    /*    pBB-> +----+    +----+ <-pBB
     *   Change | A  | to | A  | where A and B could be empty. S is the string
     *          |    |    |    | instruction (with will branch to itself and to the
     *          +----+    +----+ start of the next instruction, i.e. the start of B,
     *          | S  |      |       if B is non empty).
     *          +----+      V
     *          | B  |    +----+ <-skipBB
     *          |    |    +-b1-+              b1 is just a branch for the skip part
     *          +----+      |
     *                      V
     *                    +----+ <-rptBB
     *                    | S' |              S' = S less the skip and repeat parts
     *                    +-b2-+              b2 is a branch for the repeat part
     *                      |
     *                      V
     *                    +----+ <-newBb
     *                    | B  |
     *                    |    |
     *                    +----+
     * S is an RTL with 6 statements representing one string instruction (so this function is highly specialised for the job
     * of replacing the %SKIP and %RPT parts of string instructions)
     */
    BasicBlock *splitForBranch(BasicBlock *pBB, RTL *rtl, BranchStatement *br1, BranchStatement *br2, iterator& it);


    void removeUnneededLabels(ICodeGenerator *gen);
    void generateDotFile(QTextStream& of);

    /// \returns the entry BB of the procedure of this CFG
    BasicBlock *getEntryBB() { return m_entryBB; }
    BasicBlock *getExitBB() { return m_exitBB; }

    /// Set the entry bb to \p entryBB and mark all return BBs as exit BBs.
    void setEntryAndExitBB(BasicBlock *entryBB);
    void setExitBB(BasicBlock *exitBB);

    BasicBlock *findRetNode();

    /// print this cfg, mainly for debugging
    void print(QTextStream& out, bool html = false);
    void printToLog();
    void dump();            ///< Dump to LOG_STREAM()
    void dumpImplicitMap(); ///< Dump the implicit map to LOG_STREAM()

    /// Check for indirect jumps and calls.
    /// If any found, decode the extra code and return true
    bool decodeIndirectJmp(UserProc *proc);

    // Implicit assignments

    /// Find or create an implicit assign for x
    Statement *findImplicitAssign(SharedExp x);

    /// Find the existing implicit assign for x (if any)
    Statement *findTheImplicitAssign(const SharedExp& x);

    /// Find exiting implicit assign for parameter p
    Statement *findImplicitParamAssign(Parameter *p);

    /// Remove an existing implicit assignment for x
    void removeImplicitAssign(SharedExp x);

    bool implicitsDone() const { return m_implicitsDone; }    ///<  True if implicits have been created
    void setImplicitsDone() { m_implicitsDone = true; } ///< Call when implicits have been created
    void findInterferences(ConnectionGraph& ig);
    void appendBBs(std::list<BasicBlock *>& worklist, std::set<BasicBlock *>& workset);

    bool removeOrphanBBs();

private:
    UserProc *m_myProc;                      ///< Pointer to the UserProc object that contains this CFG object
    mutable bool m_wellFormed;
    bool m_implicitsDone;                    ///< True when the implicits are done; they can cause problems (e.g. with ad-hoc global assignment)

    std::list<BasicBlock *> m_listBB;        ///< BasicBlock s contained in this CFG

    MAPBB m_mapBB;                           ///< The Address to BB map
    BasicBlock *m_entryBB;                   ///< The CFG entry BasicBlock.
    BasicBlock *m_exitBB;                    ///< The CFG exit BasicBlock.
    ExpStatementMap m_implicitMap;           ///< Map from expression to implicit assignment. The purpose is to prevent multiple implicit assignments for the same location.

    LivenessAnalyzer m_livenessAna;
};
