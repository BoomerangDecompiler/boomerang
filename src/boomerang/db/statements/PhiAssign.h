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


#include "boomerang/db/statements/Assignment.h"


/**
 * The below could almost be a RefExp. But we could not
 * at one stage #include exp.h as part of statement,h; that's since changed
 * so it is now possible, and arguably desirable.
 * However, it's convenient to have these members public
 */
struct PhiInfo
{
    void setDef(Statement *def)
    {
        m_def = def;
    }

    Statement *getDef() { return m_def; }
    const Statement *getDef() const { return m_def; }

    SharedExp         e; ///< The expression for the thing being defined (never subscripted)
private:
    Statement       *m_def; ///< The defining statement
};


/**
 * PhiAssign is a subclass of Assignment, having a left hand side, and a StatementVec with the references.
 * For more information about phi functions, see https://en.wikipedia.org/wiki/Static_single_assignment_form.
 *
 * \code
 * m[1000] := phi{3 7 10}    // m[1000] is defined at statements 3, 7, and 10
 * m[r28{3}+4] := phi{2 8}   // the memof is defined at 2 and 8, and the r28 is defined at 3.
 * \endcode
 * The integers are really pointers to statements, printed as the statement number for compactness
 *
 * \note Although the left hand side is nearly always redundant, it is essential in at least one circumstance: when
 * finding locations used by some statement, and the reference is to a CallStatement returning multiple locations.
 */
class PhiAssign : public Assignment
{
public:
    typedef std::map<BasicBlock *, PhiInfo> PhiDefs;
    typedef PhiDefs::iterator             iterator;
    typedef PhiDefs::const_iterator       const_iterator;

public:
    PhiAssign(SharedExp _lhs)
        : Assignment(_lhs) { m_kind = STMT_PHIASSIGN; }
    PhiAssign(SharedType ty, SharedExp _lhs)
        : Assignment(ty, _lhs) { m_kind = STMT_PHIASSIGN; }
    // Copy constructor (not currently used or implemented)
    PhiAssign(Assign& other);
    virtual ~PhiAssign() {}

    // Clone
    virtual Statement *clone() const override;

    // get how to replace this statement in a use
    virtual SharedExp getRight() const override { return nullptr; }

    // Accept a visitor to this Statement
    virtual bool accept(StmtVisitor *visitor) override;
    virtual bool accept(StmtExpVisitor *visitor) override;

    // Visiting from class StmtPartModifier
    // Modify all the various expressions in a statement, except for the top level of the LHS of assignments
    virtual bool accept(StmtModifier *visitor) override;
    virtual bool accept(StmtPartModifier *visitor) override;

    virtual void printCompact(QTextStream& os, bool html = false) const override;

    // general search
    virtual bool search(const Exp& search, SharedExp& result) const override;

    /// FIXME: is this the right semantics for searching a phi statement, disregarding the RHS?
    virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

    // general search and replace
    virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

    // simplify all the uses/defs in this Statement
    virtual void simplify() override;

    // Generate constraints
    virtual void genConstraints(LocationSet& cons) override;

    // Data flow based type analysis
    // For x0 := phi(x1, x2, ...) want
    // Tx0 := Tx0 meet (Tx1 meet Tx2 meet ...)
    // Tx1 := Tx1 meet Tx0
    // Tx2 := Tx2 meet Tx0
    // ...
    void dfaTypeAnalysis(bool& ch) override;

    //
    //    Phi specific functions
    //

    // Get or put the statement at index idx
    Statement *getStmtAt(BasicBlock *idx)
    {
        if (m_defs.find(idx) == m_defs.end()) {
            return nullptr;
        }

        return m_defs[idx].getDef();
    }

    PhiInfo& getAt(BasicBlock *idx);
    void putAt(BasicBlock *idx, Statement *d, SharedExp e);
    void simplifyRefs();

    virtual size_t getNumDefs() const { return m_defs.size(); }
    PhiDefs& getDefs() { return m_defs; }

    // A hack. Check MVE
    bool hasGlobalFuncParam();

    PhiInfo& front() { return m_defs.begin()->second; }
    PhiInfo& back() { return m_defs.rbegin()->second; }
    iterator begin() { return m_defs.begin(); }
    iterator end() { return m_defs.end(); }
    const_iterator cbegin() const { return m_defs.begin(); }
    const_iterator cend() const { return m_defs.end(); }
    iterator erase(iterator it) { return m_defs.erase(it); }

    // Convert this phi assignment to an ordinary assignment

    /// Convert this PhiAssignment to an ordinary Assignment.
    /// Hopefully, this is the only place that Statements change from
    /// one class to another.  All throughout the code, we assume that the addresses of Statement objects do not change,
    /// so we need this slight hack to overwrite one object with another
    void convertToAssign(SharedExp rhs);

    // Generate a list of references for the parameters
    void enumerateParams(std::list<SharedExp>& le);

private:
    PhiDefs m_defs; ///< A vector of information about definitions
};
