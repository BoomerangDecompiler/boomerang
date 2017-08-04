#pragma once

#include "boomerang/db/statements/TypingStatement.h"

/***************************************************************************/ /**
 * Assignment is an abstract subclass of TypingStatement, holding a location
 ****************************************************************************/
class Assignment : public TypingStatement
{
public:
    /// Constructor, subexpression
    Assignment(SharedExp lhs);
    /// Constructor, type, and subexpression
    Assignment(SharedType ty, SharedExp lhs);
    /// Destructor
    virtual ~Assignment();

    /// Clone
    virtual Statement *clone() const override = 0;

    /// We also want operator< for assignments. For example, we want ReturnStatement
    /// to contain a set of (pointers to) Assignments, so we can automatically
    /// make sure that existing assignments are not duplicated.
    /// Assume that we won't want sets of assignments differing by anything other than LHSs
    bool operator<(const Assignment& o) { return m_lhs < o.m_lhs; }

    /// Accept a visitor to this Statement
    virtual bool accept(StmtVisitor *visitor)      override = 0;
    virtual bool accept(StmtExpVisitor *visitor)   override = 0;
    virtual bool accept(StmtModifier *visitor)     override = 0;
    virtual bool accept(StmtPartModifier *visitor) override = 0;

    virtual void print(QTextStream& os, bool html = false) const override;
    virtual void printCompact(QTextStream& os, bool html = false) const = 0; // Without statement number

    virtual SharedType getTypeFor(SharedExp e) const override;               ///< Get the type for this assignment. It should define e
    virtual void setTypeFor(SharedExp e, SharedType ty) override;            ///< Set the type for this assignment. It should define e

    virtual bool usesExp(const Exp& e) const override;                       ///< PhiAssign and ImplicitAssign don't override

    virtual bool isDefinition() const override { return true; }
    virtual void getDefinitions(LocationSet& defs) const override;
    virtual bool definesLoc(SharedExp loc) const override; ///< True if this Statement defines loc

    /// get how to access this lvalue
    /// Note: now only defined for Assignments, not all Statements
    virtual SharedExp getLeft();
    virtual const SharedExp& getLeft() const;

    virtual SharedExp getRight() const = 0;

    /// set the lhs to something new
    void setLeft(SharedExp e);

    /// memory depth
    int getMemDepth();

    /// general search
    virtual bool search(const Exp& search, SharedExp& result) const override = 0;
    virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override = 0;

    /// general search and replace
    virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override = 0;

    void generateCode(ICodeGenerator *, BasicBlock *, int /*indLevel*/) override {}

    /// simpify internal expressions
    virtual void simplify() override = 0;

    /// simplify address expressions
    virtual void simplifyAddr() override;

    /// generate Constraints
    virtual void genConstraints(LocationSet& cons) override;

    /// Data flow based type analysis
    void dfaTypeAnalysis(bool& ch) override;

protected:
    SharedExp m_lhs; ///< The left hand side
};
