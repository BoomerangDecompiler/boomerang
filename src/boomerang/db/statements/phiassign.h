#pragma once

#include "boomerang/db/statements/assignment.h"

// The below could almost be a RefExp. But we could not at one stage #include exp.h as part of statement,h; that's since
// changed so it is now possible, and arguably desirable.  However, it's convenient to have these members public
struct PhiInfo
{
    // A default constructor is required because CFG changes (?) can cause access to elements of the vector that
    // are beyond the current end, creating gaps which have to be initialised to zeroes so that they can be skipped
    PhiInfo() {} // : def(0), e(0) not initializing to help valgrind find locations of unset vals

    SharedExp         e; // The expression for the thing being defined (never subscripted)
    void              def(Instruction *def) { m_def = def; /*assert(def);*/ }
    Instruction       *def() { return m_def; }
    const Instruction *def() const { return m_def; }

protected:
    Instruction       *m_def; // The defining statement
};


/***************************************************************************/ /**
 * PhiAssign is a subclass of Assignment, having a left hand side, and a StatementVec with the references.
 * \code
 * m[1000] := phi{3 7 10}    // m[1000] is defined at statements 3, 7, and 10
 * m[r28{3}+4] := phi{2 8}   // the memof is defined at 2 and 8, and the r28 is defined at 3.
 * \endcode
 * The integers are really pointers to statements,printed as the statement number for compactness
 *
 * \note Although the left hand side is nearly always redundant, it is essential in at least one circumstance: when
 * finding locations used by some statement, and the reference is to a CallStatement returning multiple locations.
 ******************************************************************************/
class PhiAssign : public Assignment
{
public:
    typedef std::map<BasicBlock *, PhiInfo>   Definitions;
    typedef Definitions::iterator             iterator;
    typedef Definitions::const_iterator       const_iterator;

private:
    Definitions DefVec; // A vector of information about definitions

public:
    PhiAssign(SharedExp _lhs)
        : Assignment(_lhs) { m_kind = STMT_PHIASSIGN; }
    PhiAssign(SharedType ty, SharedExp _lhs)
        : Assignment(ty, _lhs) { m_kind = STMT_PHIASSIGN; }
    // Copy constructor (not currently used or implemented)
    PhiAssign(Assign& o);
    virtual ~PhiAssign() {}

    // Clone
    virtual Instruction *clone() const override;

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
    Instruction *getStmtAt(BasicBlock *idx)
    {
        if (DefVec.find(idx) == DefVec.end()) {
            return nullptr;
        }

        return DefVec[idx].def();
    }

    PhiInfo& getAt(BasicBlock *idx);
    void putAt(BasicBlock *idx, Instruction *d, SharedExp e);
    void simplifyRefs();

    virtual size_t getNumDefs() const { return DefVec.size(); }
    Definitions& getDefs() { return DefVec; }

    // A hack. Check MVE
    bool hasGlobalFuncParam();

    PhiInfo& front() { return DefVec.begin()->second; }
    PhiInfo& back() { return DefVec.rbegin()->second; }
    iterator begin() { return DefVec.begin(); }
    iterator end() { return DefVec.end(); }
    const_iterator cbegin() const { return DefVec.begin(); }
    const_iterator cend() const { return DefVec.end(); }
    iterator erase(iterator it) { return DefVec.erase(it); }

    // Convert this phi assignment to an ordinary assignment

    /// Convert this PhiAssignment to an ordinary Assignment.
    /// Hopefully, this is the only place that Statements change from
    /// one class to another.  All throughout the code, we assume that the addresses of Statement objects do not change,
    /// so we need this slight hack to overwrite one object with another
    void convertToAssign(SharedExp rhs);

    // Generate a list of references for the parameters
    void enumerateParams(std::list<SharedExp>& le);
};
