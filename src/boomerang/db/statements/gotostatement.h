#pragma once

#include "boomerang/db/statements/statement.h"

/*=============================================================================
 * GotoStatement has just one member variable, an expression representing the
 * jump's destination (an integer constant for direct jumps; an expression
 * for register jumps). An instance of this class will never represent a
 * return or computed call as these are distinguised by the decoder and are
 * instantiated as CallStatements and ReturnStatements respecitvely.
 * This class also represents unconditional jumps with a fixed offset
 * (e.g BN, Ba on SPARC).
 *===========================================================================*/
class GotoStatement : public Instruction
{
protected:
	/// Destination of a jump or call. This is the absolute destinatio
	/// for both static and dynamic CTIs.
	SharedExp m_dest;
	bool m_isComputed; ///< True if this is a CTI with a computed destination address.

	/// NOTE: This should be removed, once CaseStatement and HLNwayCall are implemented
	/// properly.
	std::shared_ptr<Const> constDest() { return std::static_pointer_cast<Const>(m_dest); }
	const std::shared_ptr<const Const> constDest() const { return std::static_pointer_cast<const Const>(m_dest); }

public:
	GotoStatement();

	/***************************************************************************/ /**
	 * \brief        Construct a jump to a fixed address
	 * \param        jumpDest native address of destination
	 ******************************************************************************/
	GotoStatement(Address jumpDest);

	/***************************************************************************/ /**
	 * \fn        GotoStatement::~GotoStatement
	 * \brief        Destructor
	 ******************************************************************************/
	virtual ~GotoStatement();

	/***************************************************************************/ /**
	 * \fn        GotoStatement::clone
	 * \brief     Deep copy clone
	 * \returns   Pointer to a new Statement, a clone of this GotoStatement
	 ******************************************************************************/
	virtual Instruction *clone() const override;  ///< Make a deep copy, and make the copy a derived object if needed.

	// Accept a visitor to this Statement
	// visit this Statement in the RTL
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	// Set and return the destination of the jump. The destination is either an Exp, or an ADDRESS that
	// is converted to a Exp.

	/***************************************************************************/ /**
	 * \brief        Set the destination of this jump to be a given fixed address.
	 * \param   addr - the new fixed address
	 ******************************************************************************/
	void setDest(Address addr);
	void setDest(SharedExp pd);

	/***************************************************************************/ /**
	 * \brief        Returns the destination of this CTI.
	 * \returns Pointer to the Exp representing the dest of this jump
	 ******************************************************************************/
	virtual SharedExp getDest();
	virtual const SharedExp getDest() const;

	/***************************************************************************/ /**
	 * \brief Get the fixed destination of this CTI. Assumes destination
	 *        simplication has already been done so that a fixed dest will
	 *        be of the Exp form:
	 *        opIntConst dest
	 * \returns Fixed dest or Address::INVALID if there isn't one, For dynamic CTIs,
	 *          returns Address::INVALID.
	 ******************************************************************************/
	   Address getFixedDest() const;

	/***************************************************************************/ /**
	 * \brief        Adjust the destination of this CTI by a given amount. Causes
	 *                    an error is this destination is not a fixed destination
	 *                    (i.e. a constant offset).
	 * \param   delta - the amount to add to the destination (can be
	 *                  negative)
	 ******************************************************************************/
	void adjustFixedDest(int delta);

	/***************************************************************************/ /**
	 * \fn      GotoStatement::setIsComputed
	 * \brief      Sets the fact that this call is computed.
	 * \note This should really be removed, once CaseStatement and
	 *                    HLNwayCall are implemented properly
	 ******************************************************************************/
	void setIsComputed(bool b = true);

	/***************************************************************************/ /**
	 * \fn      GotoStatement::isComputed
	 * \brief      Returns whether or not this call is computed.
	 * \note          This should really be removed, once CaseStatement and HLNwayCall
	 *                    are implemented properly
	 * \returns           this call is computed
	 ******************************************************************************/
	bool isComputed() const;

	/***************************************************************************/ /**
	 * \fn    GotoStatement::print
	 * \brief Display a text reprentation of this RTL to the given stream
	 * \note  Usually called from RTL::print, in which case the first 9
	 *        chars of the print have already been output to os
	 * \param os - stream to write to
	 * \param html - produce html encoded representation
	 ******************************************************************************/
	virtual void print(QTextStream& os, bool html = false) const override;

	// general search
	virtual bool search(const Exp&, SharedExp&) const override;

	// Replace all instances of "search" with "replace".

	/***************************************************************************/ /**
	 * \fn        GotoStatement::searchAndReplace
	 * \brief        Replace all instances of search with replace.
	 * \param search - a location to search for
	 * \param replace - the expression with which to replace it
	 * \param cc - ignored
	 * \returns True if any change
	 ******************************************************************************/
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;


	/***************************************************************************/ /**
	 * \fn        GotoStatement::searchAll
	 * \brief        Find all instances of the search expression
	 * Searches for all instances of a given subexpression within this
	 * expression and adds them to a given list in reverse nesting order.
	 *
	 * \param search - a location to search for
	 * \param result - a list which will have any matching exprs
	 *                 appended to it
	 * \returns true if there were any matches
	 ******************************************************************************/
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	// code generation
	virtual void generateCode(ICodeGenerator *, BasicBlock *, int) override;

	// simplify all the uses/defs in this Statement
	virtual void simplify() override;

	// Statement virtual functions
	virtual bool isDefinition() const override { return false; }
	virtual bool usesExp(const Exp&) const override;
};
