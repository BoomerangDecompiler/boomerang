/*
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2001, The University of Queensland
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   rtl.h
 * OVERVIEW:   Definition of the classes that describe an RTL, a low-level
 *			   register transfer list. Higher-level RTLs (instance
 *			   of class HLJump, HLCall, etc.) represent information about
 *			   a control transfer instruction (CTI) in the source program.
 *			   analysis code adds information to existing higher-level
 *			   RTLs and sometimes creates new higher-level RTLs (e.g., for
 *			   switch statements). 
 *============================================================================*/

/*
 * $Revision$	// 1.51.2.1
 *
 * 08 Apr 02 - Mike: Mods for boomerang
 * 13 May 02 - Mike: expList is no longer a pointer
 * 25 Jul 03 - Mike: RTL now a list of Statements
 */

#ifndef __RTL_H__
#define __RTL_H__

#include <list>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include "exp.h"
#include "register.h"

class BasicBlock;
class HLLCode;
typedef BasicBlock* PBB;
class Exp;
class TypedExp;
class DefSet;
class UseSet;
class Type;
class Register;
class Proc;
class XMLProgParser;
class StmtVisitor;


/*==============================================================================
 * Class RTL: describes low level register transfer lists (actually lists of statements).
 * NOTE: when time permits, this class could be removed, replaced with new Statements that mark the current native
 * address
 *============================================================================*/
class RTL {
		ADDRESS		nativeAddr;							// RTL's source program instruction address
		std::list<Statement*> stmtList;					// List of expressions in this RTL.
public:
					RTL();
					RTL(ADDRESS instNativeAddr, std::list<Statement*>* listStmt = NULL);
					RTL(const RTL& other);					// Makes deep copy of "other"
virtual				~RTL();

typedef	std::list<Statement*>::iterator iterator;
typedef	std::list<Statement*>::reverse_iterator reverse_iterator;

		// Return a deep copy, including a deep copy of the list of Statements
virtual RTL* clone();

		// Assignment copy: set this RTL to a deep copy of "other".
		RTL& operator=(RTL &other);

		// Accept a visitor to this RTL
virtual bool		accept(StmtVisitor* visitor);

		// Common enquiry methods
		ADDRESS		getAddress();							// Return RTL's native address
		Type*		getType();								// Return type of first Assign.
		bool		areFlagsAffected();						// True if flags are affected

		// Statement list enquiry methods
		int			getNumStmt();							// Return the number of Stmts in RTL.
		Statement*	elementAt(unsigned i);					// Return the i'th element in RTL.
		
		// Statement list editing methods
		void		appendStmt(Statement *s);				// Add s to end of RTL.
		void		prependStmt(Statement *s);				// Add s to start of RTL.
		void		insertStmt(Statement *s, unsigned i);	// Insert s before expression at position i
		void		insertStmt(Statement *s, iterator it);	// Insert s before iterator it
		void		updateStmt(Statement *s, unsigned i);	// Change stmt at position i.
		void		deleteStmt(unsigned int);				// Delete expression at position i.
		void		deleteLastStmt();						// Delete the last statement
		void		replaceLastStmt(Statement* repl);		// Replace the last Statement
		void		clear();								// Remove all statements from this RTL.
		// Append list of exps to end.
		void		appendListStmt(std::list<Statement*>& le); 
		void		appendRTL(RTL& rtl);					// Append Statements from other RTL to end
		// Make a deep copy of the list of Exp*
		void		deepCopyList(std::list<Statement*>& dest);
		// direct access to the list of expressions
		std::list<Statement*> &getList() { return stmtList; }

		 // Print RTL to a stream.
virtual void		print(std::ostream& os = std::cout, bool html = false);

		void		dump();

		// Set the RTL's source address
		void		updateAddress(ADDRESS addr);

		// Is this RTL a compare instruction? If so, the passed register and compared value (a semantic string) are set.
		bool		isCompare(int& iReg, Exp*& pTerm);

		// Return true if RTL loads the high half of an immediate constant into anything. If so, loads the already
		// shifted high value into the parameter.
		bool		isHiImmedLoad(ADDRESS& uHiHalf);

		// As above for low half. Extra parameters are required for SPARC, where bits are potentially transferred from
		// one register to another.
		bool		isLoImmedLoad(ADDRESS& uLoHalf, bool& bTrans, int& iSrc);

		// Do a machine dependent, and a standard simplification of the RTL.
		void		allSimplify();

		// Perform forward substitutions of temps, if possible. Called from the above
		void		forwardSubs();

		// Insert an assignment into this RTL
		//	 ssLhs: ptr to Exp to place on LHS
		//	 ssRhs: ptr to Exp to place on RHS
		//	 prep:	true if prepend (else append)
		//	 type:	type of the transfer, or NULL
		void		insertAssign(Exp* ssLhs, Exp* ssRhs, bool prep, Type* type = NULL);

		// Insert an assignment into this RTL, after temps have been defined
		//	 ssLhs: ptr to Exp to place on LHS
		//	 ssRhs: ptr to Exp to place on RHS
		//	 type:	type of the transfer, or NULL
		void		insertAfterTemps(Exp* ssLhs, Exp* ssRhs, Type* type = NULL);

		// Replace all instances of "search" with "replace".
virtual bool		searchAndReplace(Exp* search, Exp* replace);
		
		// Searches for all instances of "search" and adds them to "result" in reverse nesting order. The search is
		// optionally type sensitive.
virtual bool		searchAll(Exp* search, std::list<Exp*> &result);

		// code generation
virtual void		generateCode(HLLCode *hll, BasicBlock *pbb, int indLevel);

		// simplify all the uses/defs in this RTL
virtual void		simplify();

		// True if this RTL ends in a GotoStatement
		bool		isGoto();

		// Is this RTL a call instruction?
		bool		isCall();

		// Is this RTL a branch instruction?
		bool		isBranch();

		// Get the "special" (High Level) Statement this RTL (else NULL)
		Statement*	getHlStmt();

		// Print to a string (mainly for debugging)
		char*		prints();

		// Set or clear all the "constant subscripts" (conscripts) in this RTL
		int			setConscripts(int n, bool bClear);
protected:

		friend class XMLProgParser;
};



/*==============================================================================
 * The TableEntry class represents a single instruction - a string/RTL pair.
 *
 * This class plus ParamEntry and RTLInstDict should be moved to a separate
 * header file...
 *============================================================================*/
class TableEntry {
public:
	TableEntry();
	TableEntry(std::list<std::string>& p, RTL& rtl);

	const TableEntry& operator=(const TableEntry& other);

	void setParam(std::list<std::string>& p);
	void setRTL(RTL& rtl);
	
	// non-zero return indicates failure
	int appendRTL(std::list<std::string>& p, RTL& rtl);
	
public:
	std::list<std::string> params;
	RTL rtl;

#define TEF_NEXTPC 1		
	int flags;					// aka required capabilities. Init. to 0 
};


/*==============================================================================
 * The ParamEntry class represents the details of a single parameter.
 *============================================================================*/
typedef enum {PARAM_SIMPLE, PARAM_ASGN, PARAM_LAMBDA, PARAM_VARIANT} ParamKind;

class ParamEntry {
public:
		ParamEntry() {
			asgn = NULL;
			kind = PARAM_SIMPLE;
			type = NULL;
			regType = NULL;
			lhs = false;
			mark = 0;
		}
		~ParamEntry() {
			if (type) delete type;
			if (regType) delete regType;
		}
		
		std::list<std::string> params;		/* PARAM_VARIANT & PARAM_ASGN only */
		std::list<std::string> funcParams;	/* PARAM_LAMBDA - late bound params */
		Statement*	asgn;					/* PARAM_ASGN only */
		bool		lhs;					/* True if this param ever appears on the LHS of an expression */
		ParamKind	kind;
		Type*		type;
		Type*		regType;				/* Type of r[this], if any (void otherwise) */
		std::set<int> regIdx;				/* Values this param can take as an r[param] */
		int			mark;					/* Traversal mark. (free temporary use, basically) */
};


/*==============================================================================
 * The RTLInstDict represents a dictionary that maps instruction names to the
 * parameters they take and a template for the Exp list describing their
 * semantics. It handles both the parsing of the SSL file that fills in
 * the dictionary entries as well as instantiation of an Exp list for a given
 * instruction name and list of actual parameters. 
 *============================================================================*/

class RTLInstDict {
public:
		RTLInstDict();
		~RTLInstDict();

		// Parse a file containing a list of instructions definitions in SSL format and build the contents of this
		// dictionary.
		bool	readSSLFile(const std::string& SSLFileName);

		// Reset the object to "undo" a readSSLFile()
		void	reset();

		// Return the signature of the given instruction.
		std::pair<std::string,unsigned> getSignature(const char* name);

		// Appends an RTL to an idict entry, or Adds it to idict if an entry does not already exist. A non-zero return
		// indicates failure.
		int appendToDict(std::string &n, std::list<std::string>& p, RTL& rtl);
		
		// Given an instruction name and list of actual parameters, return an instantiated RTL for the corresponding
		// instruction entry.
		std::list<Statement*>* instantiateRTL(std::string& name, ADDRESS natPC, std::vector<Exp*>& actuals);
		// As above, but takes an RTL & param list directly rather than doing a table lookup by name.
		std::list<Statement*>* instantiateRTL(RTL& rtls, ADDRESS natPC, std::list<std::string> &params,
			std::vector<Exp*>& actuals);

		// Transform the given list into another list which doesn't have post-variables, by either adding temporaries or
		// just removing them where possible. Modifies the list passed, and also returns a pointer to it. Second
		// parameter indicates whether the routine should attempt to optimize the resulting output, ie to minimize the
		// number of temporaries. This is recommended for fully expanded expressions (ie within uqbt), but unsafe
		// otherwise.
		std::list<Statement*>* transformPostVars(std::list<Statement*> *rts, bool optimise);

		void		print(std::ostream& os = std::cout);
		
		// Add a new register to the machine
		void		addRegister(const char *name, int id, int size, bool flt);

		// If the top level operator of the given expression indicates any kind of type, update ty to match.
		bool		partialType(Exp* exp, Type& ty);

	// Go through the params and fixup any lambda functions
	void			fixupParams();

public:
	// A map from the symbolic representation of a register (e.g. "%g0") to its index within an array of registers.
	std::map<std::string, int, std::less<std::string> > RegMap;

	// Similar to r_map but stores more info about a register such as its size, its addresss etc (see register.h).
	std::map<int, Register, std::less<int> > DetRegMap;

	// A map from symbolic representation of a special (non-addressable) register to a Register object
	std::map<std::string, Register, std::less<std::string> > SpecialRegMap;

	// A set of parameter names, to make sure they are declared (?).
	// Was map from string to SemTable index
	std::set<std::string> ParamSet;

	// Parameter (instruction operand, more like addressing mode) details (where given)
	std::map<std::string, ParamEntry> DetParamMap;

	// The maps which summarise the semantics (.ssl) file
	std::map<std::string, Exp*> FlagFuncs;
	std::map<std::string, std::pair<int, void*>*> DefMap;
	std::map<int, Exp*> AliasMap;

	// Map from ordinary instruction to fast pseudo instruction, for use with -f (fast but not as exact) switch
	std::map<std::string, std::string> fastMap;

	bool bigEndian;				   // True if this source is big endian

	// The actual dictionary.
	std::map<std::string, TableEntry, std::less<std::string> > idict;

	// An RTL describing the machine's basic fetch-execute cycle
	RTL *fetchExecCycle;

	void fixupParamsSub(std::string s, std::list<std::string>& funcParams, bool& haveCount, int mark);
};

#endif /*__RTL_H__*/

