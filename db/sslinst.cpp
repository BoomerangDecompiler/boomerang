/*
 * Copyright (C) 1997, Shane Sendall
 * Copyright (C) 1998-1999, David Ung
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   sslinst.cc
 * OVERVIEW:   This file defines the classes used to represent the semantic
 *			   definitions of instructions and given in a .ssl file.
 *============================================================================*/
 
/*
 * $Revision$
 *
 * 27 Apr 02 - Mike: Mods for boomerang
 * 17 Jul 02 - Mike: readSSLFile resets internal state as well
 * 04 Feb 03 - Mike: Fixed a bug with instantiating NOP (could cause bus error?)
 * 22 May 03 - Mike: Fixed a small memory leak (char* opcode)
 * 16 Jul 04 - Mike: Simplify decoded semantics
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif 

#include <algorithm>	// For remove()
#include "types.h"
#include "statement.h"
#include "exp.h"
#include "register.h"
#include "type.h"
#include "rtl.h"
#include "cfg.h"
#include "proc.h"
#include "prog.h"
#include "sslparser.h"
#include "boomerang.h"
// For some reason, MSVC 5.00 complains about use of undefined types a lot
#if defined(_MSC_VER) && _MSC_VER <= 1100
#include "signature.h"		// For MSVC 5.00
#endif


//#define DEBUG_SSLPARSER 1

/*==============================================================================
 * FUNCTION:		TableEntry::TableEntry
 * OVERVIEW:		Constructor
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
TableEntry::TableEntry() { flags = 0; }

/*==============================================================================
 * FUNCTION:		TableEntry::TableEntry
 * OVERVIEW:		Constructor
 * PARAMETERS:		p -
 *					r - reference to a RTL
 * RETURNS:			<nothing>
 *============================================================================*/
TableEntry::TableEntry(std::list<std::string>& p, RTL& r) : rtl(r)
{ 
	for (std::list<std::string>::iterator it = p.begin(); it != p.end(); it++)
		params.push_back(*it);
	flags = 0; 
}

/*==============================================================================
 * FUNCTION:		TableEntry::setParam
 * OVERVIEW:		Set the parameter list.
 * PARAMETERS:		p - a list of strings
 * RETURNS:			<nothing>
 *============================================================================*/
void TableEntry::setParam(std::list<std::string>& p) { params = p; }

/*==============================================================================
 * FUNCTION:		TableEntry::setRTL
 * OVERVIEW:		Set the RTL.
 * PARAMETERS:		r - a RTL
 * RETURNS:			<nothing>
 *============================================================================*/
void TableEntry::setRTL(RTL& r) {
	rtl = r;
}

/*==============================================================================
 * FUNCTION:		TableEntry::operator=
 * OVERVIEW:		Sets the contents of this object with a deepcopy from
 *					another TableEntry object. Note that this is different from
 *					the semantics of operator= for an RTL which only does a
 *					shallow copy!
 * PARAMETERS:		other - the object to copy
 * RETURNS:			a reference to this object
 *============================================================================*/
const TableEntry& TableEntry::operator=(const TableEntry& other) {
	for (std::list<std::string>::const_iterator it = other.params.begin(); 
		 it != other.params.end(); it++)
		params.push_back(*it);
	rtl = *(new RTL(other.rtl));
	return *this;
}

/*==============================================================================
 * FUNCTION:		TableEntry::appendRTL
 * OVERVIEW:		Appends an RTL to an exising TableEntry
 * PARAMETERS:		p: reference to list of formal parameters (as strings)
 *					r: reference to RTL with list of Exps to append
 * RETURNS:			0 for success
 *============================================================================*/
int TableEntry::appendRTL(std::list<std::string>& p, RTL& r) {
	bool match = (p.size() == params.size());
	std::list<std::string>::iterator a, b;
	for (a = params.begin(), b = p.begin();
	  match && (a != params.end()) && (b != p.end());
	  match = (*a == *b), a++, b++)
		;
	if (match) {
		rtl.appendRTL(r);
		return 0;
	}
	return -1;
}

/*==============================================================================
 * FUNCTION:		RTLInstDict::appendToDict
 * OVERVIEW:		Appends one RTL to the dictionary
 * PARAMETERS:		n: name of the instruction to add to
 *					p: list of formal parameters (as strings) for the RTL to add
 *					r: reference to the RTL to add
 * RETURNS:			0 for success
 *============================================================================*/
int RTLInstDict::appendToDict(std::string &n, std::list<std::string>& p, RTL& r)
{
	char *opcode = new char[n.size() + 1];
	strcpy(opcode, n.c_str());
	upperStr(opcode, opcode);
	std::remove(opcode, opcode+strlen(opcode)+1,'.');
	std::string s(opcode);
	//delete [] opcode;
   
	if (idict.find(s) == idict.end()) {
		idict[s] = TableEntry(p, r);
	} else {
		return idict[s].appendRTL(p, r);
	}
	return 0;
}

RTLInstDict::RTLInstDict()
{
}

RTLInstDict::~RTLInstDict()
{
}

/*==============================================================================
 * FUNCTION:		RTLInstDict::readSSLFile
 * OVERVIEW:		Read and parse the SSL file, and initialise the expanded
 *					instruction dictionary (this object). This also reads and
 *					sets up the register map and flag functions.
 * PARAMETERS:		SSLFileName - the name of the file containing the SSL
 *					  specification.
 * RETURNS:			the file was successfully read
 *============================================================================*/
bool RTLInstDict::readSSLFile(const std::string& SSLFileName)
{
	// emptying the rtl dictionary
	idict.erase(idict.begin(),idict.end());
	// Clear all state
	reset();
	
	// Attempt to Parse the SSL file
	SSLParser theParser(SSLFileName,
#ifdef DEBUG_SSLPARSER
	true
#else
	false
#endif
);
	if (theParser.theScanner == NULL)
		return false;
	addRegister( "%CTI", -1, 1, false );
	addRegister( "%NEXT", -1, 32, false );
	
	theParser.yyparse(*this);

	fixupParams();

	if (Boomerang::get()->debugDecoder) {
		std::cout << "\n=======Expanded RTL template dictionary=======\n";
		print();
		std::cout << "\n==============================================\n\n";
	}
	
	return true;
}

/*==============================================================================
 * FUNCTION:		RTLInstDict::addRegister
 * OVERVIEW:		Add a new register definition to the dictionary
 * PARAMETERS:		
 * RETURNS:			<nothing>
 *============================================================================*/
void RTLInstDict::addRegister( const char *name, int id, int size, bool flt )
{
	RegMap[name] = id;
	if( id == -1 ) {
		SpecialRegMap[name].s_name(name);
		SpecialRegMap[name].s_size(size);
		SpecialRegMap[name].s_float(flt);
		SpecialRegMap[name].s_address(NULL);
		SpecialRegMap[name].s_mappedIndex(-1);
		SpecialRegMap[name].s_mappedOffset(-1);
	} else {
		DetRegMap[id].s_name(name);
		DetRegMap[id].s_size(size);
		DetRegMap[id].s_float(flt);
		DetRegMap[id].s_address(NULL);
		DetRegMap[id].s_mappedIndex(-1);
		DetRegMap[id].s_mappedOffset(-1);
	}	 
}


/*==============================================================================
 * FUNCTION:		RTLInstDict::print
 * OVERVIEW:		Print a textual representation of the dictionary.
 * PARAMETERS:		std::cout - stream used for printing
 * RETURNS:			<nothing>
 *============================================================================*/
void RTLInstDict::print(std::ostream& os /*= std::cout*/)
{
	for (std::map<std::string, TableEntry>::iterator p = idict.begin();
	  p != idict.end(); p++) {
		// print the instruction name
		os << (*p).first << "  ";

		// print the parameters
		std::list<std::string>& params = (*p).second.params;
		int i = params.size();
		for (std::list<std::string>::iterator s = params.begin();
		  s != params.end(); s++,i--)
			os << *s << (i != 1 ? "," : "");
		os << "\n";
	
		// print the RTL
		RTL& rtlist = (*p).second.rtl;
		rtlist.print(os);
		os << "\n";
	}

#if 0
	// Detailed register map
	os << "\nDetailed register map\n";
	std::map<int, Register, std::less<int> >::iterator rr;
	for (rr = DetRegMap.begin(); rr != DetRegMap.end(); rr++) {
		int n = rr->first;
		Register* pr = &rr->second;
		os << "number " << n <<
		  " name " << pr->g_name() <<
		  " size " << std::dec << pr->g_size() <<
		  " address 0x" << std::hex << (unsigned)pr->g_address() <<
		  " mappedIndex " << std::dec << pr->g_mappedIndex() <<
		  " mappedOffset " << pr->g_mappedOffset() <<
		  " flt " << pr->isFloat() << "\n";
	}
#endif
}

/*==============================================================================
 * FUNCTION:		 RTLInstDict::fixupParams
 * OVERVIEW:		 Runs after the ssl file is parsed to fix up variant params
 *					 where the arms are lambdas.
 * PARAMETERS:		 None
 * RETURNS:			 Nothing
 *============================================================================*/
void RTLInstDict::fixupParams( )
{
	std::map<std::string,ParamEntry>::iterator param;
	int mark = 1;
	for( param = DetParamMap.begin(); param != DetParamMap.end(); param++ ) {
		param->second.mark = 0;
	}
	for( param = DetParamMap.begin(); param != DetParamMap.end(); param++ ) {
		std::list<std::string> funcParams;
		bool haveCount = false;
		if( param->second.kind == PARAM_VARIANT ) {
			fixupParamsSub( param->first, funcParams, haveCount, mark++ );
		}
	}
}

void RTLInstDict::fixupParamsSub( std::string s, std::list<std::string>& funcParams,
  bool& haveCount, int mark )
{
	ParamEntry &param = DetParamMap[s];

	if( param.params.size() == 0 ) {
		std::cerr << "Error in SSL File: Variant operand "
			 << s << " has no branches. Well that's really useful...\n";
		return;
	}
	if( param.mark == mark )
		return; /* Already seen this round. May indicate a cycle, but may not */
	
	param.mark = mark;
	
	for( std::list<std::string>::iterator it = param.params.begin();
		 it != param.params.end(); it++ ) {
		ParamEntry &sub = DetParamMap[*it];
		if (sub.kind == PARAM_VARIANT ) {
			fixupParamsSub(*it, funcParams, haveCount, mark );
			if (!haveCount) { /* Empty branch? */
				continue;
			}
		} else if (!haveCount ) {
			haveCount = true;
			char buf[10];
			for (unsigned i=1; i <= sub.funcParams.size(); i++ ) {
				sprintf( buf, "__lp%d", i );
				funcParams.push_back(buf);
			}
		}

		if (funcParams.size() != sub.funcParams.size() ) {
			std::cerr << "Error in SSL File: Variant operand " << s
				 << " does not have a fixed number of functional parameters:\n"
				 << "Expected " << funcParams.size() << ", but branch "
				 << *it << " has " << sub.funcParams.size() << ".\n";
		} else if (funcParams != sub.funcParams && sub.asgn != NULL ) {
			/* Rename so all the parameter names match */
			std::list<std::string>::iterator i,j;
			for( i = funcParams.begin(), j = sub.funcParams.begin();
			  i != funcParams.end(); i++, j++ ) {
				Exp* match = Location::param(j->c_str());
				Exp* replace = Location::param(i->c_str());
				sub.asgn->searchAndReplace( match, replace );
			}
			sub.funcParams = funcParams;
		}
	}

//	  if( param.funcParams.size() != funcParams.size() )
//		  theSemTable.setItem( n, cFUNCTION, 0, 0, funcParams.size(),
//							   theSemTable[n].sName.c_str() );
	param.funcParams = funcParams;
}

/*==============================================================================
 * FUNCTION:		 RTLInstDict::getNumOperands
 * OVERVIEW:		 Returns the signature of the given instruction.
 * PARAMETERS:		 name -
 * RETURNS:			 the signature (name + number of operands)
 *============================================================================*/
std::pair<std::string,unsigned> RTLInstDict::getSignature(const char* name) {
	// Take the argument, convert it to upper case and remove any _'s and .'s
	char *opcode = new char[strlen(name) + 1];
	upperStr(name, opcode);
//	std::remove(opcode,opcode+strlen(opcode)+1,'_');
	std::remove(opcode,opcode+strlen(opcode)+1,'.');

	// Look up the dictionary
	std::map<std::string,TableEntry>::iterator it = idict.find(opcode);
	if (it == idict.end()) {
		std::cerr << "Error: no entry for `" << name << "' in RTL dictionary\n";
		it = idict.find("NOP");		// At least, don't cause segfault
	} 

	std::pair<std::string, unsigned> ret;
	ret = std::pair<std::string,unsigned>(opcode,(it->second).params.size());
	//delete [] opcode;
	return ret;
}

/*==============================================================================
 * FUNCTION:		 RTLInstDict::partialType
 * OVERVIEW:		 Scan the Exp* pointed to by exp; if its top level
 *						operator indicates even a partial type, then set
 *						the expression's type, and return true
 * NOTE:			 This version only inspects one expression
 * PARAMETERS:		 exp - points to a Exp* to be scanned
 *					 ty - ref to a Type object to put the partial type into
 * RETURNS:			 True if a partial type is found
 *============================================================================*/
bool RTLInstDict::partialType(Exp* exp, Type& ty)
{
	if (exp->isSizeCast()) {
		ty = IntegerType(((Const*)((Binary*)exp)->getSubExp1())->getInt());
		return true;
	}
	if (exp->isFltConst()) {
		ty = FloatType(64);
		return true;
	}
	return false;
}

/*==============================================================================
 * FUNCTION:		 RTLInstDict::instantiateRTL
 * OVERVIEW:		 Returns an instance of a register transfer list for
 *					 the instruction named 'name' with the actuals
 *					 given as the second parameter.
 * PARAMETERS:		 name - the name of the instruction (must correspond to one
 *					   defined in the SSL file).
 *					 actuals - the actual values
 * RETURNS:			 the instantiated list of Exps
 *============================================================================*/
std::list<Statement*>* RTLInstDict::instantiateRTL(std::string& name,
  ADDRESS natPC, std::vector<Exp*>& actuals) {
	// If -f is in force, use the fast (but not as precise) name instead
	const std::string* lname = &name;
	// FIXME: settings
//	  if (progOptions.fastInstr) {
if (0) {
		std::map<std::string, std::string>::iterator itf = fastMap.find(name);
		if (itf != fastMap.end()) 
			lname = &itf->second;
	}
	// Retrieve the dictionary entry for the named instruction
	if ( idict.find(*lname) == idict.end() ) { /* lname is not in dictionary */
		std::cerr << "ERROR: unknown instruction " << lname << ", ignoring.\n";
		return NULL;
	}
	TableEntry& entry = idict[*lname];

	return instantiateRTL( entry.rtl, natPC, entry.params, actuals );
}

/*==============================================================================
 * FUNCTION:		 RTLInstDict::instantiateRTL
 * OVERVIEW:		 Returns an instance of a register transfer list for
 *					 the parameterized rtlist with the given formals replaced
 *					 with the actuals given as the third parameter.
 * PARAMETERS:		 rtl - a register transfer list
 *					 params - a list of formal parameters
 *					 actuals - the actual parameter values
 * RETURNS:			 the instantiated list of Exps
 *============================================================================*/
std::list<Statement*>* RTLInstDict::instantiateRTL(RTL& rtl, ADDRESS natPC,
		std::list<std::string>& params, std::vector<Exp*>& actuals) {
	assert(params.size() == actuals.size());

	// Get a deep copy of the template RTL
	std::list<Statement*>* newList = new std::list<Statement*>();
	rtl.deepCopyList(*newList);

	// Iterate through each Statement of the new list of stmts
	std::list<Statement*>::iterator ss;
	for (ss = newList->begin(); ss != newList->end(); ss++) {
		// Search for the formals and replace them with the actuals
		std::list<std::string>::iterator param = params.begin();
		std::vector<Exp*>::const_iterator actual = actuals.begin();
		for (; param != params.end(); param++, actual++) {
			/* Simple parameter - just construct the formal to search for */
			Exp* formal = Location::param(param->c_str());
			(*ss)->searchAndReplace(formal, *actual);
			//delete formal;
		}
		(*ss)->fixSuccessor();
		if (Boomerang::get()->debugDecoder)
			std::cout << "			" << *ss << "\n";
	}

	transformPostVars( newList, true );

	// Perform simplifications, e.g. *1 in Pentium addressing modes
	for (ss = newList->begin(); ss != newList->end(); ss++)
		(*ss)->simplify();

	return newList;
}

/* Small struct for transformPostVars */
class transPost {
public:
	bool used;		// If the base expression (e.g. r[0]) is used
					// Important because if not, we don't have to make any
					// substitutions at all
	bool isNew;		// Not sure (MVE)
	Exp* tmp;		// The temp to replace r[0]' with
	Exp* post;		// The whole postvar expression. e.g. r[0]'
	Exp* base;		// The base expression (e.g. r[0])
	Type* type;		// The type of the temporary (needed for the final assign)
};

/*
 * Transform an RTL to eliminate any uses of post-variables. Note that
 * the algorithm used expects to deal with simple expressions as post
 * vars, ie r[22], m[r[1]], generally things which aren't parameterized
 * at a higher level. This is ok for the translator (we do substitution
 * first anyway), but may miss some optimizations for the emulator.
 * For the emulaor, if parameters are detected within a postvar,
 * we just force the temporary, which is always safe to do.
 * (The parameter optimise is set to false for the emulator to achieve this).
 */

std::list<Statement*>* RTLInstDict::transformPostVars(
  std::list<Statement*>* rts, bool optimise ) {
	std::list<Statement*>::iterator rt;

	// Map from var (could be any expression really) to details
	std::map<Exp*,transPost,lessExpStar> vars;
	int tmpcount = 1;		// For making temp names unique
	// Exp* matchParam(1,idParam);	// ? Was never used anyway

#ifdef DEBUG_POSTVAR
	std::cout << "Transforming from:\n";
	for (Exp_CIT p = rts->begin(); p != rts->end(); p++) {
		std::cout << setw(8) << " ";
		(*p)->print(std::cout);
		std::cout << "\n";
	}
#endif
	
	// First pass: Scan for post-variables and usages of their referents
	for( rt = rts->begin(); rt != rts->end(); rt++ ) {
		// ss appears to be a list of expressions to be searched
		// It is either the LHS and RHS of an assignment, 
		// or it's the parameters of a flag call
		Binary* ss;
		if( (*rt)->isAssign()) {
			Exp* lhs = (*rt)->getLeft();
			Exp* rhs = (*rt)->getRight();

			// Look for assignments to post-variables
			if (lhs && lhs->isPostVar()) {
				if( vars.find(lhs) == vars.end() ) {
					// Add a record in the map for this postvar
					transPost& el = vars[lhs];
					el.used = false;
					el.type = ((Assign*)*rt)->getType();
					
					// Constuct a temporary. We should probably be smarter
					// and actually check that it's not otherwise used here.
					std::string tmpname = el.type->getTempName() + (tmpcount++)
					  + "post" ;
					el.tmp = Location::tempOf(new Const(
					  (char*)tmpname.c_str()));

					// Keep a copy of the referrent. For example, if the
					// lhs is r[0]', base is r[0]
					el.base = lhs->getSubExp1();
					el.post = lhs;	   // The whole post-var, e.g. r[0]'
					el.isNew = true;

					// The emulator generator sets optimise false
					// I think this forces always generating the temps (MVE)
					if( !optimise ) {
						el.used = true;
						el.isNew = false;
					}
					
				}
			}
			// For an assignment, the two expressions to search are the
			// left and right hand sides (could just put the whole assignment
			// on, I suppose)
			ss = new Binary(opList, lhs->clone(),
					new Binary(opList, rhs->clone(), new Terminal(opNil)));
		} else if( (*rt)->isFlagAssgn()) {
			// An opFlagCall is assumed to be a Binary with a string and an
			// opList of parameters
			ss = (Binary*) ((Binary*)*rt)->getSubExp2();
		} else
			ss = NULL;

		/* Look for usages of post-variables' referents
		 * Trickier than you'd think, as we need to make sure to skip over
		 * the post-variables themselves. ie match r[0] but not r[0]'
		 * Note: back with SemStrs, we could use a match expression which
		 * was a wildcard prepended to the base expression; this would
		 * match either the base (r[0]) or the post-var (r[0]').
		 * Can't really use this with Exps, so we search twice; once for
		 * the base, and once for the post, and if we get more with the
		 * former, then we have a use of the base (consider r[0] + r[0]')
		 */
		for (std::map<Exp*,transPost,lessExpStar>::iterator sr = vars.begin();
			 sr != vars.end(); sr++ ) {
			if( sr->second.isNew ) {
				// Make sure we don't match a var in its defining statement
				sr->second.isNew = false;
				continue;
			}
			Binary* cur;
			for (cur = ss; !cur->isNil(); cur = (Binary*)cur->getSubExp2()) {
				if( sr->second.used )
					break;		// Don't bother; already know it's used
				Exp* s = cur->getSubExp1();
				if( !s ) continue;
				if( *s == *sr->second.base ) {
					sr->second.used = true;
					break;
				}
				std::list<Exp*> res1, res2;
				s->searchAll( sr->second.base, res1 );
				s->searchAll( sr->second.post, res2 );
				// Each match of a post will also match the base.
				// But if there is a bare (non-post) use of the base, there
				// will be a result in res1 that is not in res2
				if (res1.size() > res2.size())
					sr->second.used = true;
			}
		}
	}

	// Second pass: Replace post-variables with temporaries where needed
	for ( rt = rts->begin(); rt != rts->end(); rt++ ) {
		for (std::map<Exp*,transPost,lessExpStar>::iterator sr = vars.begin();
		  sr != vars.end(); sr++ ) {
			if( sr->second.used ) {
				(*rt)->searchAndReplace(sr->first, sr->second.tmp);
			} else {
				(*rt)->searchAndReplace(sr->first, sr->second.base);
			}
		}
	}

	// Finally: Append assignments where needed from temps to base vars
	// Example: esp' = esp-4; m[esp'] = modrm; FLAG(esp)
	// all the esp' are replaced with say tmp1,
	// you need a "esp = tmp1" at the end to actually make the change
	for( std::map<Exp*,transPost,lessExpStar>::iterator sr = vars.begin();
	  sr != vars.end(); sr++ ) {
		if( sr->second.used ) {
			Assign* te = new Assign(sr->second.type,
					sr->second.base->clone(),
					sr->second.tmp);
			rts->push_back( te );
		} else {
			// The temp is either used (uncloned) in the assignment, or is
			// deleted here
			//delete sr->second.tmp;
		}
	}

#ifdef DEBUG_POSTVAR
	std::cout << "\nTo =>\n";
	for (std::list<Exp*>::iterator p = rts->begin(); p != rts->end(); p++) {
		std::cout << setw(8) << " ";
		(*p)->print(std::cout);
		std::cout << "\n";
	}
	std::cout << "\n";
#endif

	return rts;
}

// Call from test code if (e.g.) want to call readSSLFile() twice
void RTLInstDict::reset() {
	RegMap.clear();
	DetRegMap.clear();
	SpecialRegMap.clear();
	ParamSet.clear();
	DetParamMap.clear();
	FlagFuncs.clear();
	DefMap.clear();
	AliasMap.clear();
	fastMap.clear();
	idict.clear();
	fetchExecCycle = 0;
}
