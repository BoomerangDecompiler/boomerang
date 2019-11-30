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
#include "boomerang/util/StatementList.h"

#include <list>
#include <memory>


class BasicBlock;
class ImplicitAssign;
class PhiAssign;

using RTLList   = std::list<std::unique_ptr<RTL>>;
using SharedExp = std::shared_ptr<Exp>;


/// Kinds of basic block nodes
/// reordering these will break the save files - trent
enum class FragType
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


/// Holds the IR for at most a single BasicBlock.
/// In some cases, this might be only a part of a single instruction (e.g. x86 bsf/bsr)
class BOOMERANG_API IRFragment : public GraphNode<IRFragment>
{
public:
    typedef RTLList::iterator RTLIterator;
    typedef RTLList::reverse_iterator RTLRIterator;

public:
    IRFragment(BasicBlock *bb, Address lowAddr);
    IRFragment(BasicBlock *bb, std::unique_ptr<RTLList> rtls);
    IRFragment(const IRFragment &);
    IRFragment(IRFragment &&) = default;
    ~IRFragment()             = default;

    IRFragment &operator=(const IRFragment &);
    IRFragment &operator=(IRFragment &&) = default;

    bool operator<(const IRFragment &rhs) const;

public:
    BasicBlock *getBB() { return m_bb; }
    const BasicBlock *getBB() const { return m_bb; }

    /// \returns the type of the BasicBlock
    inline FragType getType() const { return m_fragType; }
    inline bool isType(FragType type) const { return m_fragType == type; }
    inline void setType(FragType type) { m_fragType = type; }

    UserProc *getProc();
    const UserProc *getProc() const;

    /// \returns all RTLs that are part of this fragment.
    RTLList *getRTLs() { return m_listOfRTLs.get(); }
    const RTLList *getRTLs() const { return m_listOfRTLs.get(); }

    RTL *getLastRTL();
    const RTL *getLastRTL() const;

    void removeRTL(RTL *rtl);

public:
    /// \returns the lowest real address associated with this fragement.
    /// \sa updateAddresses
    Address getLowAddr() const;

    /// Get the highest address associated with this fragment.
    /// This is always the address associated with the last RTL.
    /// \sa updateAddresses
    Address getHiAddr() const;

    /// Update the high and low address of this fragment if the RTL list has changed.
    void updateAddresses();

public:
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

    /// Appends all statements in this fragment to \p stmts.
    void appendStatementsTo(StatementList &stmts) const;

    ///
    std::shared_ptr<ImplicitAssign> addImplicitAssign(const SharedExp &lhs);

    /// Add a new phi assignment of the form <usedExp> := phi() to the beginning of the fragment.
    std::shared_ptr<PhiAssign> addPhi(const SharedExp &usedExp);

    // Remove all refs from phis in this fragment
    void clearPhis();

    bool hasStatement(const SharedStmt &stmt) const;

    /// \returns true iff the fragment does not contain any statements.
    /// \note This is different from a fragment that does not contain
    /// any RTLs, since all RTLs could be empty.
    bool isEmpty() const;

    /// \returns true iff the fragment only contains an unconditional jump statement.
    /// \note this disregards the type of the fragment (e.g. Oneway)
    bool isEmptyJump() const;

public:
    /// \returns the destination procedure of the call if this is a call fragment.
    /// Returns nullptr for all other fragment types.
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
     * If the fragment does not have a conditional branch statement,
     * this function returns nullptr.
     */
    SharedExp getCond() const;

    /**
     * Set the condition of a conditional branch fragment.
     * If the BB is not a branch, nothing happens.
     */
    void setCond(const SharedExp &cond);

    /// Get the destination of the high level jump in this fragment, if any
    SharedExp getDest() const;

    /// Simplify all expressions in this fragment
    void simplify();

public:
    void print(OStream &os) const;

    QString toString() const;

public:
    BasicBlock *m_bb;
    std::unique_ptr<RTLList> m_listOfRTLs = nullptr; ///< Ptr to list of RTLs

    Address m_lowAddr  = Address::ZERO;
    Address m_highAddr = Address::INVALID;

    FragType m_fragType = FragType::Invalid;
};
