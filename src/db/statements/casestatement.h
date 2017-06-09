#pragma once

#include "db/statements/gotostatement.h"


class CaseStatement : public GotoStatement
{
	friend class XMLProgParser;

private:
	SWITCH_INFO *pSwitchInfo; // Ptr to struct with info about the switch

public:

	/***************************************************************************/ /**
	 * \fn        CaseStatement::CaseStatement
	 * \brief        Constructor.
	 ******************************************************************************/
	CaseStatement();

	/***************************************************************************/ /**
	 * \fn    CaseStatement::~CaseStatement
	 * \brief Destructor
	 * \note  Don't delete the pSwitchVar; it's always a copy of something else (so don't delete twice)
	 ******************************************************************************/
	virtual ~CaseStatement();

	// Make a deep copy, and make the copy a derived object if needed.

	/***************************************************************************/ /**
	 * \fn      CaseStatement::clone
	 * \brief   Deep copy clone
	 * \returns Pointer to a new Instruction that is a clone of this one
	 ******************************************************************************/
	virtual Instruction *clone() const override;

	// Accept a visitor to this Statememt
	// visit this stmt
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	// Set and return the Exp representing the switch variable

	/***************************************************************************/ /**
	 * \fn      CaseStatement::getSwitchInfo
	 * \brief   Return a pointer to a struct with switch information in it
	 * \returns SWITCH_INFO struct
	 ******************************************************************************/

	SWITCH_INFO *getSwitchInfo();


	/***************************************************************************/ /**
	 * \fn    CaseStatement::setSwitchInfo
	 * \brief Set a pointer to a SWITCH_INFO struct
	 * \param psi Pointer to SWITCH_INFO struct
	 ******************************************************************************/
	void setSwitchInfo(SWITCH_INFO *psi);

	/***************************************************************************/ /**
	 * \fn    CaseStatement::print
	 * \brief Write a text representation to the given stream
	 * \param os - target stream
	 * \param html - produce html encoded representation
	 ******************************************************************************/
	virtual void print(QTextStream& os, bool html = false) const override;

	// Replace all instances of "search" with "replace".

	/***************************************************************************/ /**
	 * \fn    CaseStatement::searchAndReplace
	 * \brief Replace all instances of search with replace.
	 * \param search - a location to search for
	 * \param replace - the expression with which to replace it
	 * \param cc - ignored
	 * \returns             True if any change
	 ******************************************************************************/
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	// Searches for all instances of a given subexpression within this
	// expression and adds them to a given list in reverse nesting order.

	/***************************************************************************/ /**
	 * \fn    CaseStatement::searchAll
	 * \brief Find all instances of the search expression
	 * \param search - a location to search for
	 * \param result - a list which will have any matching exprs appended to it
	 * \returns true if there were any matches
	 ******************************************************************************/
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	/// code generation
	virtual void generateCode(HLLCode *, BasicBlock *, int) override;

	/// dataflow analysis
	virtual bool usesExp(const Exp& e) const override;

public:
	/// simplify all the uses/defs in this Statement
	virtual void simplify() override;
};
