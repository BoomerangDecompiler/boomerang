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


#include "boomerang/db/DefCollector.h"
#include "boomerang/ssl/statements/Statement.h"
#include "boomerang/util/StatementList.h"


/**
 * Represents an ordinary high level return.
 */
class BOOMERANG_API ReturnStatement : public Statement
{
public:
    typedef StatementList::iterator         iterator;
    typedef StatementList::const_iterator   const_iterator;

public:
    ReturnStatement();
    ReturnStatement(const ReturnStatement& other) = default;
    ReturnStatement(ReturnStatement&& other) = default;

    virtual ~ReturnStatement() override;

    ReturnStatement& operator=(const ReturnStatement& other) = default;
    ReturnStatement& operator=(ReturnStatement&& other) = default;

public:
    iterator begin() { return m_returns.begin(); }
    iterator end()   { return m_returns.end(); }

    const_iterator begin() const { return m_returns.begin(); }
    const_iterator end()   const { return m_returns.end(); }

public:
    /// \copydoc Statement::clone
    virtual Statement *clone() const override;

    iterator erase(iterator it);

    const StatementList& getModifieds() { return m_modifieds; }
    const StatementList& getReturns() { return m_returns; }

    size_t getNumReturns() const { return m_returns.size(); }

    /// Update the modifieds, in case the signature and hence ordering and filtering has changed, or the locations in the
    /// collector have changed. Does NOT remove preserveds (deferred until updating returns).
    void updateModifieds(); // Update modifieds from the collector

    /// Update the returns, in case the signature and hence ordering
    /// and filtering has changed, or the locations in the modifieds list
    void updateReturns();

    /// \copydoc Statement::print
    virtual void print(QTextStream& os, bool html = false) const override;

    /// \copydoc Statement::search
    virtual bool search(const Exp&, SharedExp&) const override;

    /// \copydoc Statement::searchAll
    virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

    /// \copydoc Statement::searchAndReplace
    virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

    /// \copydoc Statement::usesExp
    virtual bool usesExp(const Exp& e) const override;

    /// \copydoc Statement::getDefinitions
    virtual void getDefinitions(LocationSet& defs, bool assumeABICompliance) const override;

    /// Remove from modifieds AND from returns
    void removeModified(SharedExp loc);

    /// For testing only
    void addReturn(Assignment *a);

    /// \copydoc Statement::getTypeFor
    virtual SharedConstType getTypeFor(SharedConstExp e) const override;

    /// \copydoc Statement::getTypeFor
    virtual SharedType getTypeFor(SharedExp e) override;

    /// \copydoc Statement::setTypeFor
    virtual void setTypeFor(SharedExp e, SharedType ty) override;

    /// \copydoc Statement::simplify
    virtual void simplify() override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtVisitor *visitor) const override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtExpVisitor *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtModifier *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtPartModifier *visitor) override;

    /// \copydoc Statement::definesLoc
    virtual bool definesLoc(SharedExp loc) const override;

    /// \copydoc Statement::generateCode
    virtual void generateCode(ICodeGenerator *gen, const BasicBlock *parentBB) override;

    /// \returns pointer to the collector object
    DefCollector *getCollector() { return &m_col; }

    /// Get and set the native address for the first and only return statement
    Address getRetAddr() { return m_retAddr; }
    void setRetAddr(Address r) { m_retAddr = r; }

    /// Find definition for e (in the collector)
    SharedExp findDefFor(SharedExp e) { return m_col.findDefFor(e); }

protected:
    /// Native address of the (only) return instruction.
    /// Needed for branching to this only return statement
    Address m_retAddr;

    /**
     * The progression of return information is as follows:
     * First, reaching definitions are collected in the DefCollector col. These are not sorted or filtered.
     * Second, some of those definitions make it to the modifieds list, which is sorted and filtered. These are
     * the locations that are modified by the enclosing procedure. As locations are proved to be preserved (with NO
     * modification, not even sp = sp+4), they are removed from this list. Defines in calls to the enclosing
     * procedure are based on this list.
     * Third, the modifications are initially copied to the returns list (also sorted and filtered, but the returns
     * have RHS where the modifieds don't). Locations not live at any caller are removed from the returns, but not
     * from the modifieds.
     */
    DefCollector m_col;

    /// A list of assignments that represents the locations modified by the enclosing procedure. These assignments
    /// have no RHS?
    /// These transmit type information to callers
    /// Note that these include preserved locations early on (?)
    StatementList m_modifieds;

    /// A list of assignments of locations to expressions.
    /// Initially definitions reaching the exit less preserveds; later has locations unused by any callers removed.
    /// A list is used to facilitate ordering. (A set would be ideal, but the ordering depends at runtime on the
    /// signature)
    StatementList m_returns;
};
