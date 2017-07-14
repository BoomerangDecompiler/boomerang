#pragma once

#include "boomerang/db/statements/gotostatement.h"
#include "boomerang/db/statements/assignment.h"
#include "boomerang/include/managed.h"

class ImplicitAssign;

/***************************************************************************/ /**
 * CallStatement: represents a high level call. Information about parameters and the like are stored here.
 ******************************************************************************/
class CallStatement : public GotoStatement
{
private:
	bool m_returnAfterCall; // True if call is effectively followed by a return.

	/// The list of arguments passed by this call, actually a list of Assign statements (location := expr)
	StatementList m_arguments;

	/// The list of defines for this call, a list of ImplicitAssigns (used to be called returns).
	/// Essentially a localised copy of the modifies of the callee, so the callee could be deleted. Stores types and
	/// locations.  Note that not necessarily all of the defines end up being declared as results.
	StatementList m_defines;

	/// Destination of call. In the case of an analysed indirect call, this will be ONE target's return statement.
	/// For an unanalysed indirect call, or a call whose callee is not yet sufficiently decompiled due to recursion,
	/// this will be nullptr
	Function *m_procDest;

	/// The signature for this call. NOTE: this used to be stored in the Proc, but this does not make sense when
	/// the proc happens to have varargs
	std::shared_ptr<Signature> m_signature;

	/// A UseCollector object to collect the live variables at this call.
	/// Used as part of the calculation of results
	UseCollector m_useCol;

	/// A DefCollector object to collect the reaching definitions;
	/// used for bypassAndPropagate/localiseExp etc; also
	/// the basis for arguments if this is an unanlysed indirect call
	DefCollector m_defCol;

	/// Pointer to the callee ReturnStatement. If the callee is unanlysed,
	/// this will be a special ReturnStatement with ImplicitAssigns.
	/// Callee could be unanalysed because of an unanalysed indirect call,
	/// or a "recursion break".
	ReturnStatement *m_calleeReturn;

public:

	/***************************************************************************/ /**
	 * \fn         CallStatement::CallStatement
	 * \brief      Constructor for a call
	 ******************************************************************************/
	CallStatement();

	/***************************************************************************/ /**
	 * \fn      CallStatement::~CallStatement
	 * \brief   Destructor
	 ******************************************************************************/
	virtual ~CallStatement();

	virtual void setNumber(int num) override;

	// Make a deep copy, and make the copy a derived object if needed.

	/***************************************************************************/ /**
	 * \fn        CallStatement::clone
	 * \brief     Deep copy clone
	 * \returns   Pointer to a new Statement, a clone of this CallStatement
	 ******************************************************************************/
	virtual Instruction *clone() const override;

	// Accept a visitor to this stmt
	// visit this stmt
	virtual bool accept(StmtVisitor *visitor) override;
	virtual bool accept(StmtExpVisitor *visitor) override;
	virtual bool accept(StmtModifier *visitor) override;
	virtual bool accept(StmtPartModifier *visitor) override;

	/***************************************************************************/ /**
	 * \fn      CallStatement::setArguments
	 * \brief      Set the arguments of this call.
	 * \param      args - the list of locations to set the arguments to (for testing)
	 ******************************************************************************/
	void setArguments(StatementList& args);

	// Set implicit arguments: so far, for testing only:
	// void setImpArguments(std::vector<Exp*>& arguments);
	// void setReturns(std::vector<Exp*>& returns);// Set call's return locs

	/***************************************************************************/ /**
	 * \fn      CallStatement::setSigArguments
	 * \brief   Set the arguments of this call based on signature info
	 * \note    Should only be called for calls to library functions
	 ******************************************************************************/
	void setSigArguments();                             // Set arguments based on signature

	StatementList& getArguments() { return m_arguments; } // Return call's arguments
	void updateArguments();                             // Update the arguments based on a callee change

	// Exp        *getDefineExp(int i);
	/// Temporarily needed for ad-hoc type analysis
	int findDefine(SharedExp e);        // Still needed temporarily for ad hoc type analysis
	void removeDefine(SharedExp e);
	void addDefine(ImplicitAssign *as); // For testing

	// void        ignoreReturn(SharedExp e);
	// void        ignoreReturn(int n);
	// void        addReturn(SharedExp e, Type* ty = nullptr);

	/// Set the defines to the set of locations modified by the callee,
	/// or if no callee, to all variables live at this call
	void updateDefines();         // Update the defines based on a callee change

	// Calculate results(this) = defines(this) intersect live(this)
	// Note: could use a LocationList for this, but then there is nowhere to store the types (for DFA based TA)
	// So the RHS is just ignored
	StatementList *calcResults(); // Calculate defines(this) isect live(this)

	ReturnStatement *getCalleeReturn() { return m_calleeReturn; }
	void setCalleeReturn(ReturnStatement *ret) { m_calleeReturn = ret; }
	bool isChildless() const;
	SharedExp getProven(SharedExp e);

	std::shared_ptr<Signature> getSignature() { return m_signature; }
	void setSignature(std::shared_ptr<Signature> sig) { m_signature = sig; } ///< Only used by range analysis

	/// Localise the various components of expression e with reaching definitions to this call
	/// Note: can change e so usually need to clone the argument
	/// Was called substituteParams
	///
	/// Substitute the various components of expression e with the appropriate reaching definitions.
	/// Used in e.g. fixCallBypass (via the CallBypasser). Locations defined in this call are replaced with their proven
	/// values, which are in terms of the initial values at the start of the call (reaching definitions at the call)
	SharedExp localiseExp(SharedExp e);

	/// Localise only components of e, i.e. xxx if e is m[xxx]
	void localiseComp(SharedExp e); // Localise only xxx of m[xxx]

	// Do the call bypass logic e.g. r28{20} -> r28{17} + 4 (where 20 is this CallStatement)
	// Set ch if changed (bypassed)
	SharedExp bypassRef(const std::shared_ptr<RefExp>& r, bool& ch);

	void clearUseCollector() { m_useCol.clear(); }
	void addArgument(SharedExp e, UserProc *proc);

	/// Find the reaching definition for expression e.
	/// Find the definition for the given expression, using the embedded Collector object
	/// Was called findArgument(), and used implicit arguments and signature parameters
	/// \note must only operator on unsubscripted locations, otherwise it is invalid
	SharedExp findDefFor(SharedExp e) const;
	SharedExp getArgumentExp(int i) const;
	void setArgumentExp(int i, SharedExp e);
	void setNumArguments(int i);
	int getNumArguments() const;
	void removeArgument(int i);
	SharedType getArgumentType(int i) const;
	void setArgumentType(int i, SharedType ty);
	void truncateArguments();
	void clearLiveEntry();
	void eliminateDuplicateArgs();

	virtual void print(QTextStream& os, bool html = false) const override;

	// general search
	virtual bool search(const Exp& search, SharedExp& result) const override;

	// Replace all instances of "search" with "replace".

	/***************************************************************************/ /**
	 * \fn              CallStatement::searchAndReplace
	 * \brief           Replace all instances of search with replace.
	 * \param search  - a location to search for
	 * \param replace - the expression with which to replace it
	 * \param cc -      true to replace in collectors
	 * \returns         True if any change
	 ******************************************************************************/
	virtual bool searchAndReplace(const Exp& search, SharedExp replace, bool cc = false) override;

	// Searches for all instances of a given subexpression within this
	// expression and adds them to a given list in reverse nesting order.

	/***************************************************************************/ /**
	 * \fn    CallStatement::searchAll
	 * \brief Find all instances of the search expression
	 * \param search - a location to search for
	 * \param result - a list which will have any matching exprs appended to it
	 * \returns true if there were any matches
	 ******************************************************************************/
	virtual bool searchAll(const Exp& search, std::list<SharedExp>& result) const override;

	// Set and return whether the call is effectively followed by a return.
	// E.g. on Sparc, whether there is a restore in the delay slot.

	/***************************************************************************/ /**
	 * \fn    CallStatement::setReturnAfterCall
	 * \brief Sets a bit that says that this call is effectively followed by a return. This happens e.g. on
	 *        Sparc when there is a restore in the delay slot of the call
	 * \param b true if this is to be set; false to clear the bit
	 ******************************************************************************/
	void setReturnAfterCall(bool b);

	/***************************************************************************/ /**
	 * \fn    CallStatement::isReturnAfterCall
	 * \brief Tests a bit that says that this call is effectively followed by a return. This happens e.g. on
	 *        Sparc when there is a restore in the delay slot of the call
	 * \returns True if this call is effectively followed by a return
	 ******************************************************************************/
	bool isReturnAfterCall() const;

	// Set and return the list of Exps that occur *after* the call (the
	// list of exps in the RTL occur before the call). Useful for odd patterns.
	void setPostCallExpList(std::list<SharedExp> *le);

	std::list<SharedExp> *getPostCallExpList();

	// Set and return the destination proc.

	/***************************************************************************/ /**
	 * \brief        Set the destination of this jump to be a given expression.
	 * \param        dest - the new target
	 ******************************************************************************/
	void setDestProc(Function *dest);
	Function *getDestProc();

	// Generate constraints
	virtual void genConstraints(LocationSet& cons) override;

	// Data flow based type analysis
	void dfaTypeAnalysis(bool& ch) override;

	// code generation
	virtual void generateCode(ICodeGenerator *hll, BasicBlock *Parent, int indLevel) override;

	// dataflow analysis
	virtual bool usesExp(const Exp& e) const override;

	// dataflow related functions
	virtual bool isDefinition() const override;
	virtual void getDefinitions(LocationSet& defs) const override;

	/// Does a ReturnStatement define anything? Not really, the locations are already defined earlier in the procedure.
	/// However, nothing comes after the return statement, so it doesn't hurt to pretend it does, and this is a place to
	/// store the return type(s) for example.
	/// FIXME: seems it would be cleaner to say that Return Statements don't define anything.
	virtual bool definesLoc(SharedExp loc) const override; // True if this Statement defines loc

	// get how to replace this statement in a use
	// virtual Exp*        getRight() { return nullptr; }

	// simplify all the uses/defs in this Statement
	virtual void simplify() override;

	//        void        setIgnoreReturnLoc(bool b);

	void decompile();

	// Insert actual arguments to match formal parameters
	// void        insertArguments(InstructionSet& rs);

	virtual SharedType getTypeFor(SharedExp e) const override;     // Get the type defined by this Statement for this location
	virtual void setTypeFor(SharedExp e, SharedType ty) override;  // Set the type for this location, defined in this statement

	DefCollector *getDefCollector() { return &m_defCol; } // Return pointer to the def collector object
	UseCollector *getUseCollector() { return &m_useCol; }    // Return pointer to the use collector object
	void useBeforeDefine(SharedExp x) { m_useCol.insert(x); } // Add x to the UseCollector for this call
	void removeLiveness(SharedExp e) { m_useCol.remove(e); } // Remove e from the UseCollector
	void removeAllLive() { m_useCol.clear(); }               // Remove all livenesses
	//        Exp*        fromCalleeContext(Exp* e);            // Convert e from callee to caller (this) context
	StatementList& getDefines() { return m_defines; } // Get list of locations defined by this call

	/// Process this call for ellipsis parameters. If found, in a printf/scanf call, truncate the number of
	/// parameters if needed, and return true if any signature parameters added
	/// This function has two jobs. One is to truncate the list of arguments based on the format string.
	/// The second is to add parameter types to the signature.
	/// If -Td is used, type analysis will be rerun with these changes.
	bool ellipsisProcessing(Prog *prog);

	/// Attempt to convert this call, if indirect, to a direct call.
	/// NOTE: at present, we igore the possibility that some other statement
	/// will modify the global. This is a serious limitation!!
	bool convertToDirect(); // Internal function: attempt to convert an indirect to a

	// direct call
	void useColFromSsaForm(Instruction *s) { m_useCol.fromSSAform(m_proc, s); }

	bool isCallToMemOffset() const;

private:
	// Private helper functions for the above
	// Helper function for makeArgAssign(?)
	void addSigParam(SharedType ty, bool isScanf);

	/// Make an assign suitable for use as an argument from a callee context expression
	Assign *makeArgAssign(SharedType ty, SharedExp e);
	bool objcSpecificProcessing(const QString& formatStr);

protected:
	void updateDefineWithType(int n);

	void appendArgument(Assignment *as) { m_arguments.append(as); }
};
