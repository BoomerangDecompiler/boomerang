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
    void            setDef(Statement *def)
    {
        m_def = def;
    }

    Statement       *getDef() { return m_def; }
    const Statement *getDef() const { return m_def; }

    SharedExp       e;   ///< The expression for the thing being defined (never subscripted)

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
    typedef std::map<BasicBlock *, PhiInfo>   PhiDefs;
    typedef PhiDefs::iterator                 iterator;
    typedef PhiDefs::const_iterator           const_iterator;

public:
    PhiAssign(SharedExp _lhs)
        : Assignment(_lhs) { m_kind = StmtType::PhiAssign; }
    PhiAssign(SharedType ty, SharedExp _lhs)
        : Assignment(ty, _lhs) { m_kind = StmtType::PhiAssign; }

    virtual ~PhiAssign() override = default;

    /// \copydoc Statement::clone
    virtual Statement *clone() const override;

    /// \copydoc Statement::getRight
    virtual SharedExp getRight() const override { return nullptr; }

    /// \copydoc Statement::accept
    virtual bool accept(StmtVisitor *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtExpVisitor *visitor) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtModifier *modifier) override;

    /// \copydoc Statement::accept
    virtual bool accept(StmtPartModifier *modifier) override;

    /// \copydoc Assignment::printCompact
    virtual void printCompact(QTextStream& os, bool html = false) const override;

    /// \copydoc Statement::search
    virtual bool search(const Exp& search, SharedExp& result) const override;

    /// \copydoc Statement::searchAll
    virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

    /// \copydoc Statement::searchAndReplace
    virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

    /// \copydoc Statement::simplify
    virtual void simplify() override;

    /// \copydoc Statement::genConstraints
    virtual void genConstraints(LocationSet& cons) override;


    //
    //    Phi specific functions
    //

    /// Get statement at index \p idx
    Statement *getStmtAt(BasicBlock *idx);
    PhiInfo& getAt(BasicBlock *idx);

    /// Update the statement at index \p idx
    void putAt(BasicBlock *idx, Statement *d, SharedExp e);

    void simplifyRefs();

    size_t getNumDefs() const { return m_defs.size(); }
    PhiDefs& getDefs() { return m_defs; }
    const PhiDefs& getDefs() const { return m_defs; }

    // A hack. Check MVE
    bool hasGlobalFuncParam();

    PhiInfo& front() { return m_defs.begin()->second; }
    PhiInfo& back() { return m_defs.rbegin()->second; }
    iterator begin() { return m_defs.begin(); }
    iterator end() { return m_defs.end(); }
    const_iterator begin() const { return m_defs.begin(); }
    const_iterator end() const { return m_defs.end(); }
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
