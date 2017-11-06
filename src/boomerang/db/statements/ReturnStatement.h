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


#include "boomerang/db/statements/Statement.h"


/**
 * Represents an ordinary high level return.
 */
class ReturnStatement : public Statement
{
public:
    typedef StatementList::iterator         iterator;
    typedef StatementList::const_iterator   const_iterator;

public:
    ReturnStatement();
    virtual ~ReturnStatement() override;

    iterator begin() { return m_returns.begin(); }
    iterator end()   { return m_returns.end(); }

    const_iterator begin() const { return m_returns.begin(); }
    const_iterator end()   const { return m_returns.end(); }

    iterator erase(iterator it) { return m_returns.erase(it); }

    StatementList& getModifieds() { return m_modifieds; }
    StatementList& getReturns() { return m_returns; }

    size_t getNumReturns() const { return m_returns.size(); }

    // Update the modifieds, in case the signature and hence ordering and filtering has changed, or the locations in the
    // collector have changed. Does NOT remove preserveds (deferred until updating returns).
    void updateModifieds(); // Update modifieds from the collector

    // Update the returns, in case the signature and hence ordering
    // and filtering has changed, or the locations in the modifieds list
    void updateReturns();   // Update returns from the modifieds

    virtual void print(QTextStream& os, bool html = false) const override;

    // general search
    virtual bool search(const Exp&, SharedExp&) const override;

    // Replace all instances of "search" with "replace".
    virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

    // Searches for all instances of a given subexpression within this statement and adds them to a given list
    virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

    // returns true if this statement uses the given expression
    virtual bool usesExp(const Exp& e) const override;

    virtual void getDefinitions(LocationSet& defs) const override;

    void removeModified(SharedExp loc); // Remove from modifieds AND from returns

    // Remove the return (if any) related to loc. Loc may or may not be subscripted
    void removeReturn(SharedExp loc);   // Remove from returns only
    void addReturn(Assignment *a);

    /// Scan the returns for e. If found, return the type associated with that return
    SharedType getTypeFor(SharedExp e) const override;
    void setTypeFor(SharedExp e, SharedType ty) override;

    // simplify all the uses/defs in this Statement
    virtual void simplify() override;

    virtual bool isDefinition() const override { return true; }

    // Get a subscripted version of e from the collector
    SharedExp subscriptWithDef(SharedExp e);

    // Make a deep copy, and make the copy a derived object if needed.

    /***************************************************************************/ /**
     * \brief        Deep copy clone
     * \returns             Pointer to a new Statement, a clone of this ReturnStatement
     ******************************************************************************/
    virtual Statement *clone() const override;

    // Accept a visitor to this Statement
    // visit this stmt
    virtual bool accept(StmtVisitor *visitor) override;
    virtual bool accept(StmtExpVisitor *visitor) override;
    virtual bool accept(StmtModifier *visitor) override;
    virtual bool accept(StmtPartModifier *visitor) override;

    virtual bool definesLoc(SharedExp loc) const override; // True if this Statement defines loc

    // code generation
    virtual void generateCode(ICodeGenerator *gen, const BasicBlock *parentBB) override;

    // Exp        *getReturnExp(int n) { return returns[n]; }
    // void        setReturnExp(int n, SharedExp e) { returns[n] = e; }
    // void        setSigArguments();                     // Set returns based on signature
    DefCollector *getCollector() { return &m_col; } // Return pointer to the collector object

    // Get and set the native address for the first and only return statement
    Address getRetAddr() { return m_retAddr; }
    void setRetAddr(Address r) {
        m_retAddr = r; }

    // Find definition for e (in the collector)
    SharedExp findDefFor(SharedExp e) { return m_col.findDefFor(e); }

    void dfaTypeAnalysis(bool& ch) override;

    // Remove the stack pointer and return a statement list
    StatementList *getCleanReturns();

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
