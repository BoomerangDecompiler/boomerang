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


/**
 *
 */
class BOOMERANG_API IRFragment : public GraphNode<IRFragment>
{
public:
    typedef RTLList::iterator RTLIterator;
    typedef RTLList::reverse_iterator RTLRIterator;

public:
    IRFragment(BasicBlock *bb, std::unique_ptr<RTLList> rtls);
    IRFragment(const IRFragment &);
    IRFragment(IRFragment &&) = default;
    ~IRFragment()             = default;

    IRFragment &operator=(const IRFragment &);
    IRFragment &operator=(IRFragment &&) = default;

public:
    /// \returns all RTLs that are part of this BB.
    RTLList *getRTLs() { return m_listOfRTLs.get(); }
    const RTLList *getRTLs() const { return m_listOfRTLs.get(); }

    RTL *getLastRTL();
    const RTL *getLastRTL() const;

    void removeRTL(RTL *rtl);

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

public:
    BasicBlock *m_bb;
    std::unique_ptr<RTLList> m_listOfRTLs = nullptr; ///< Ptr to list of RTLs
};
