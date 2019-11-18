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


#include "boomerang/db/GraphNode.h"
#include "boomerang/ssl/RTL.h"
#include "boomerang/util/Address.h"
#include "boomerang/util/StatementList.h"

#include <list>
#include <memory>
#include <vector>


class RTL;
class Exp;
class ImplicitAssign;
class PhiAssign;

class OStream;


using RTLList   = std::list<std::unique_ptr<RTL>>;
using SharedExp = std::shared_ptr<Exp>;


/// Kinds of basic block nodes
/// reordering these will break the save files - trent
enum class BBType
{
    Invalid  = -1, ///< invalid instruction
    Fall     = 0,  ///< fall-through node
    Oneway   = 1,  ///< unconditional branch (jmp)
    Twoway   = 2,  ///< conditional branch   (jXX)
    Nway     = 3,  ///< case branch          (jmp [off + 4*eax])
    Call     = 4,  ///< procedure call       (call)
    Ret      = 5,  ///< return               (ret)
    CompJump = 6,  ///< computed jump
    CompCall = 7,  ///< computed call        (call [eax + 0x14])
};


// index of the "then" branch of conditional jumps
#define BTHEN 0

// index of the "else" branch of conditional jumps
#define BELSE 1


/**
 * Basic Blocks hold the sematics (RTLs) of a sequential list of instructions
 * ended by a Control Transfer Instruction (CTI).
 * During decompilation, a special RTL with a zero address is prepended;
 * this RTL contains implicit assigns and phi assigns.
 */
class BOOMERANG_API BasicBlock : public GraphNode<BasicBlock>
{
public:
    typedef RTLList::iterator RTLIterator;
    typedef RTLList::reverse_iterator RTLRIterator;

    class BBComparator
    {
    public:
        /// \returns bb1->getLowAddr() < bb2->getLowAddr();
        bool operator()(const BasicBlock *bb1, const BasicBlock *bb2) const;
    };

public:
    /**
     * Creates an incomplete BB.
     * \param function Enclosing function
     */
    BasicBlock(Address lowAddr, Function *function);

    /**
     * Creates a complete BB.
     * \param bbType   type of BasicBlock
     * \param rtls     rtl statements that will be contained in this BasicBlock
     * \param function Function this BasicBlock belongs to.
     */
    BasicBlock(BBType bbType, std::unique_ptr<RTLList> rtls, Function *function);

    BasicBlock(const BasicBlock &other);
    BasicBlock(BasicBlock &&other) = delete;
    ~BasicBlock();

    BasicBlock &operator=(const BasicBlock &other);
    BasicBlock &operator=(BasicBlock &&other) = delete;

public:
    /// \returns the type of the BasicBlock
    inline BBType getType() const { return m_bbType; }
    inline bool isType(BBType type) const { return m_bbType == type; }
    inline void setType(BBType bbType) { m_bbType = bbType; }

    /// \returns enclosing function, nullptr if the BB does not belong to a function.
    inline const Function *getFunction() const { return m_function; }
    inline Function *getFunction() { return m_function; }

    /**
     * \returns the lowest real address associated with this BB.
     * \note although this is usually the address of the first RTL, it is not
     * always so. For example, if the BB contains just a delayed branch,and the delay
     * instruction for the branch does not affect the branch, so the delay instruction
     * is copied in front of the branch instruction. Its address will be
     * UpdateAddress()'ed to 0, since it is "not really there", so the low address
     * for this BB will be the address of the branch.
     * \sa updateBBAddresses
     */
    Address getLowAddr() const;

    /**
     * Get the highest address associated with this BB.
     * This is always the address associated with the last RTL.
     * \sa updateBBAddresses
     */
    Address getHiAddr() const;

    /// \returns true if the instructions of this BB have not been decoded yet.
    inline bool isIncomplete() const { return m_highAddr == Address::INVALID; }


    // RTL and statement related
public:
    /// \returns all RTLs that are part of this BB.
    RTLList *getRTLs();
    const RTLList *getRTLs() const;

    RTL *getLastRTL();
    const RTL *getLastRTL() const;

    void removeRTL(RTL *rtl);

    /**
     * Update the RTL list of this basic block. Takes ownership of the pointer.
     * \param rtls a list of RTLs
     */
    void completeBB(std::unique_ptr<RTLList> rtls);

    /**
     * Get first/next statement this BB
     * Somewhat intricate because of the post call semantics; these funcs save a lot of duplicated,
     * easily-bugged code
     */
    SharedStmt getFirstStmt(RTLIterator &rit, RTL::iterator &sit);
    SharedStmt getNextStmt(RTLIterator &rit, RTL::iterator &sit);
    SharedStmt getLastStmt(RTLRIterator &rit, RTL::reverse_iterator &sit);
    SharedStmt getPrevStmt(RTLRIterator &rit, RTL::reverse_iterator &sit);

    SharedStmt getFirstStmt();
    const SharedConstStmt getFirstStmt() const;
    SharedStmt getLastStmt();
    const SharedConstStmt getLastStmt() const;

    /// Appends all statements in this BB to \p stmts.
    void appendStatementsTo(StatementList &stmts) const;

    ///
    std::shared_ptr<ImplicitAssign> addImplicitAssign(const SharedExp &lhs);

    /// Add a new phi assignment of the form <usedExp> := phi() to the beginning of the BB.
    std::shared_ptr<PhiAssign> addPhi(const SharedExp &usedExp);

    // Remove all refs from phis in this BB
    void clearPhis();

    bool hasStatement(const SharedStmt &stmt) const;

    /// \returns true iff the BB does not contain any statements.
    /// \note This is different from a BB that does not contain
    /// any RTLs, since all RTLs could be empty.
    bool isEmpty() const;

    /// \returns true iff the BB only contains an unconditional jump statement.
    /// \note this disregards the type of the BB (e.g. Oneway)
    bool isEmptyJump() const;

public:
    /// \returns the destination procedure of the call if this is a call BB.
    /// Returns nullptr for all other BB types.
    Function *getCallDestProc() const;

    /*
     * Structuring and code generation.
     *
     * This code is whole heartly based on AST by Doug Simon.
     * Portions may be copyright to him and are available under a BSD style license.
     *
     * Adapted for Boomerang by Trent Waddington, 20 June 2002.
     */

    /**
     * Get the condition of a conditional branch.
     * If the BB does not have a conditional branch statement,
     * this function returns nullptr.
     */
    SharedExp getCond() const;

    /**
     * Set the condition of a conditional branch BB.
     * If the BB is not a branch, nothing happens.
     */
    void setCond(const SharedExp &cond);

    /// Get the destination of the high level jump in this BB, if any
    SharedExp getDest() const;

    /// Simplify all expressions in this BB
    void simplify();

    /// Update the high and low address of this BB if the RTL list has changed.
    void updateBBAddresses();

public:
    /**
     * Print the whole BB to the given stream
     * \param os   stream to output to
     * \param html print in html mode
     */
    void print(OStream &os) const;

    QString toString() const;

protected:
    /// The function this BB is part of, or nullptr if this BB is not part of a function.
    Function *m_function                  = nullptr;
    std::unique_ptr<RTLList> m_listOfRTLs = nullptr; ///< Ptr to list of RTLs

    Address m_lowAddr  = Address::ZERO;
    Address m_highAddr = Address::INVALID;

    BBType m_bbType = BBType::Invalid; ///< type of basic block
};
