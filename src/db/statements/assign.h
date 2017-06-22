#pragma once

#include "db/statements/assignment.h"

/**
 * Assign an ordinary assignment with left and right sides.
 */
class Assign : public Assignment
{
	friend class XMLProgParser;

private:
	SharedExp m_rhs;
	SharedExp m_guard;

public:
	/// Constructor, subexpressions
	Assign(SharedExp lhs, SharedExp r, SharedExp guard = nullptr);

	/// Constructor, type and subexpressions
	Assign(SharedType ty, SharedExp lhs, SharedExp r, SharedExp guard = nullptr);

	/// Default constructor, for XML parser
	Assign();

	/// Copy constructor
	Assign(Assign& o);

	/// Destructor
	~Assign() {}

	/// Clone
	virtual Instruction *clone() const override;

	/// Get how to replace this statement in a use
	virtual SharedExp getRight() const override { return m_rhs; }
	SharedExp& getRightRef() { return m_rhs; }
	const SharedExp& getRightRef() const { return m_rhs; }

	/// set the rhs to something new
	void setRight(SharedExp e) { m_rhs = e; }

	/// Accept a visitor to this Statement
	/// Visiting from class StmtExpVisitor
	/// Visit all the various expressions in a statement
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;

	/// Visiting from class StmtModifier
	/// Modify all the various expressions in a statement
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	virtual void printCompact(QTextStream& os, bool html = false) const override; // Without statement number

	/// Guard
	void setGuard(SharedExp g) { m_guard = g; }
	SharedExp getGuard() const { return m_guard; }
	bool isGuarded() const { return m_guard != nullptr; }

	virtual bool usesExp(const Exp& e) const override;

	virtual bool isDefinition() const override { return true; }

	/// general search
	virtual bool search(const Exp& search, SharedExp& result) const override;
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	/// general search and replace
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	/// Get memory depth
	int getMemDepth() const;

	/// Generate code
	virtual void generateCode(ICodeGenerator *hll, BasicBlock *Parent, int indLevel) override;

	/// simpify internal expressions
	virtual void simplify() override;

	/// simplify address expressions
	virtual void simplifyAddr() override;

	/// fixSuccessor (succ(r2) -> r3)
	virtual void fixSuccessor() override;

	/// generate Constraints
	virtual void genConstraints(LocationSet& cons) override;

	/// Data flow based type analysis
	void dfaTypeAnalysis(bool& ch) override;

	/// FIXME: I suspect that this was only used by adhoc TA, and can be deleted
	bool match(const char *pattern, std::map<QString, SharedExp>& bindings);
};
