#pragma once

#include "boomerang/db/statements/assignment.h"

/***************************************************************************/ /**
 * BoolAssign represents "setCC" type instructions, where some destination is set
 * (to 1 or 0) depending on the condition codes.
 * It has a condition Exp, similar to the BranchStatement class.
 ******************************************************************************/
class BoolAssign : public Assignment
{
	BranchType m_jumpType; ///< the condition for setting true
	SharedExp m_cond;      ///< Exp representation of the high level
	// condition: e.g. r[8] == 5
	bool m_isFloat;        ///< True if condition uses floating point CC
	int m_size;            ///< The size of the dest

public:

	/***************************************************************************/ /**
	 * \fn         BoolAssign::BoolAssign
	 * \brief         Constructor.
	 * \param         size - the size of the assignment
	 ******************************************************************************/
	BoolAssign(int size);
	virtual ~BoolAssign();

	// Make a deep copy, and make the copy a derived object if needed.

	/***************************************************************************/ /**
	 * \fn        BoolAssign::clone
	 * \brief     Deep copy clone
	 * \returns   Pointer to a new Statement, a clone of this BoolAssign
	 ******************************************************************************/
	virtual Instruction *clone() const override;

	// Accept a visitor to this Statement
	/// visit this Statement
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	// Set and return the BRANCH_TYPE of this scond as well as whether the
	// floating point condition codes are used.

	/***************************************************************************/ /**
	 * \brief Sets the BRANCH_TYPE of this jcond as well as the flag
	 * indicating whether or not the floating point condition codes
	 * are used.
	 * \param cond - the BRANCH_TYPE
	 * \param usesFloat - this condional jump checks the floating point condition codes
	 ******************************************************************************/
	void setCondType(BranchType cond, bool usesFloat = false);

	BranchType getCond() const { return m_jumpType; }
	bool isFloat() const { return m_isFloat; }
	void setFloat(bool b) { m_isFloat = b; }

	// Set and return the Exp representing the HL condition

	/***************************************************************************/ /**
	 * \brief Return the Exp expression containing the HL condition.
	 * \returns Exp instance
	 ******************************************************************************/
	SharedExp getCondExpr() const;

	/***************************************************************************/ /**
	 * \brief Set the Exp expression containing the HL condition.
	 * \param pss Pointer to semantic string to set
	 ******************************************************************************/
	void setCondExpr(SharedExp pss);

	// As above, no delete (for subscripting)
	void setCondExprND(SharedExp e) { m_cond = e; }
	int getSize() const { return m_size; } // Return the size of the assignment

	/***************************************************************************/ /**
	 * \brief Change this from an unsigned to a signed branch
	 * \note Not sure if this is ever going to be used
	 ******************************************************************************/
	void makeSigned();

	/***************************************************************************/ /**
	 * \fn    BoolAssign::printCompact
	 * \brief Write a text representation to the given stream
	 * \param os: stream
	 * \param html - produce html encoded representation
	 ******************************************************************************/
	virtual void printCompact(QTextStream& os, bool html = false) const override;

	/// code generation
	virtual void generateCode(ICodeGenerator *hll, BasicBlock *, int indLevel) override;

	/// simplify all the uses/defs in this Statement
	virtual void simplify() override;

	// Statement functions
	virtual bool isDefinition() const override { return true; }

	// All the Assignment-derived classes have the same definitions: the lhs
	virtual void getDefinitions(LocationSet& def) const override;

	virtual SharedExp getRight() const override { return getCondExpr(); }

	virtual bool usesExp(const Exp& e) const override;
	virtual bool search(const Exp& search, SharedExp& result) const override;
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	/// a hack for the SETS macro
	/// This is for setting up SETcc instructions; see include/decoder.h macro SETS
	void setLeftFromList(std::list<Instruction *> *stmts);

	virtual void dfaTypeAnalysis(bool& ch) override;

	friend class XMLProgParser;
}; // class BoolAssign
