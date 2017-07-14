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

/***************************************************************************/ /**
 * \file       sslinst.cpp
 * \brief   This file defines the classes used to represent the semantic
 *               definitions of instructions and given in a .ssl file.
 ******************************************************************************/

/***************************************************************************/ /**
 * Dependencies.
 ******************************************************************************/


#include "boomerang/db/exp.h"
#include "boomerang/db/register.h"
#include "boomerang/db/rtl.h"
#include "boomerang/db/cfg.h"
#include "boomerang/db/proc.h"
#include "boomerang/db/prog.h"
#include "boomerang/db/statements/assign.h"
#include "boomerang/db/ssl/sslparser.h"

#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"

#include "boomerang/type/type.h"

#include <cassert>
#include <cstring>
#include <algorithm> // For remove()

// #define DEBUG_SSLPARSER 1

/***************************************************************************/ /**
 * \brief        Constructor
 ******************************************************************************/
TableEntry::TableEntry()
{
	m_flags = 0;
}


/***************************************************************************/ /**
 * \brief Constructor
 * \param p -
 * \param r - reference to a RTL
 *
 ******************************************************************************/
TableEntry::TableEntry(std::list<QString>& p, RTL& r)
	: m_rtl(r)
{
	std::copy(p.begin(), p.end(), std::back_inserter(m_params));
	m_flags = 0;
}


/***************************************************************************/ /**
 * \brief        Set the parameter list.
 * \param        p - a list of strings
 ******************************************************************************/
void TableEntry::setParam(std::list<QString>& p)
{
	m_params = p;
}


/***************************************************************************/ /**
 * \brief        Set the RTL.
 * \param        r - a RTL
 *
 ******************************************************************************/
void TableEntry::setRTL(RTL& r)
{
	m_rtl = r;
}


/***************************************************************************/ /**
 * \brief Sets the contents of this object with a deepcopy from another TableEntry object.  Note that this is
 * different from the semantics of operator= for an RTL which only does a shallow copy!
 * \param other - the object to copy
 * \returns a reference to this object
 ******************************************************************************/
TableEntry& TableEntry::operator=(const TableEntry& other)
{
	m_params = other.m_params;
	m_rtl    = other.m_rtl;
	return *this;
}


/***************************************************************************/ /**
 * \brief        Appends an RTL to an exising TableEntry
 * \param        p reference to list of formal parameters (as strings)
 * \param        r reference to RTL with list of Exps to append
 * \returns             0 for success
 ******************************************************************************/
int TableEntry::appendRTL(std::list<QString>& p, RTL& r)
{
	bool match = (p.size() == m_params.size());

	std::list<QString>::iterator a, b;

	for (a = m_params.begin(), b = p.begin(); match && (a != m_params.end()) && (b != p.end());
		 match = (*a == *b), a++, b++) {
	}

	if (match) {
		m_rtl.appendListStmt(r);
		return 0;
	}

	return -1;
}


// Appends an RTL to an idict entry, or Adds it to idict if an entry does not already exist. A non-zero return
// indicates failure.

/***************************************************************************/ /**
 * \brief        Appends one RTL to the dictionary,or Adds it to idict if an
 * entry does not already exist.
 * \param n name of the instruction to add to
 * \param p list of formal parameters (as strings) for the RTL to add
 * \param r reference to the RTL to add
 * \returns 0 for success
 ******************************************************************************/
int RTLInstDict::appendToDict(const QString& n, std::list<QString>& p, RTL& r)
{
	QString opcode = n.toUpper();

	opcode.remove(".");

	if (idict.find(opcode) == idict.end()) {
		idict[opcode] = TableEntry(p, r);
	}
	else {
		return idict[opcode].appendRTL(p, r);
	}

	return 0;
}


RTLInstDict::RTLInstDict()
{
}


RTLInstDict::~RTLInstDict()
{
}


/***************************************************************************/ /**
 * \brief        Read and parse the SSL file, and initialise the expanded instruction dictionary (this object).
 * This also reads and sets up the register map and flag functions.
 * \param SSLFileName - the name of the file containing the SSL specification.
 * \returns        true if the file was successfully read
 ******************************************************************************/
bool RTLInstDict::readSSLFile(const QString& SSLFileName)
{
	// emptying the rtl dictionary
	idict.erase(idict.begin(), idict.end());
	// Clear all state
	reset();

	// Attempt to Parse the SSL file
	SSLParser theParser(qPrintable(SSLFileName),
#ifdef DEBUG_SSLPARSER
						true
#else
						false
#endif
						);

	if (theParser.theScanner == nullptr) {
		return false;
	}

	addRegister("%CTI", -1, 1, false);
	addRegister("%NEXT", -1, 32, false);

	theParser.yyparse(*this);

	fixupParams();

	if (Boomerang::get()->debugDecoder) {
		QTextStream q_cout(stdout);
		q_cout << "\n=======Expanded RTL template dictionary=======\n";
		print(q_cout);
		q_cout << "\n==============================================\n\n";
	}

	return true;
}


void RTLInstDict::addRegister(const QString& name, int id, int size, bool flt)
{
	RegMap[name] = id;

	if (id == -1) {
		SpecialRegMap[name].setName(name);
		SpecialRegMap[name].setSize(size);
		SpecialRegMap[name].setFloat(flt);
		SpecialRegMap[name].setAddress(nullptr);
		SpecialRegMap[name].setMappedIndex(-1);
		SpecialRegMap[name].setMappedOffset(-1);
	}
	else {
		DetRegMap[id].setName(name);
		DetRegMap[id].setSize(size);
		DetRegMap[id].setFloat(flt);
		DetRegMap[id].setAddress(nullptr);
		DetRegMap[id].setMappedIndex(-1);
		DetRegMap[id].setMappedOffset(-1);
	}
}


/***************************************************************************/ /**
 * \brief        Print a textual representation of the dictionary.
 * \param        os - stream used for printing
 *
 ******************************************************************************/
void RTLInstDict::print(QTextStream& os /*= std::cout*/)
{
	for (auto& elem : idict) {
		// print the instruction name
		os << (elem).first << "  ";

		// print the parameters
		const std::list<QString>& params((elem).second.m_params);
		int i = params.size();

		for (auto s = params.begin(); s != params.end(); s++, i--) {
			os << *s << (i != 1 ? "," : "");
		}

		os << "\n";

		// print the RTL
		RTL& rtlist = (elem).second.m_rtl;
		rtlist.print(os);
		os << "\n";
	}
}


/***************************************************************************/ /**
 * \brief         Runs after the ssl file is parsed to fix up variant params
 *                     where the arms are lambdas.
 * Go through the params and fixup any lambda functions
 ******************************************************************************/
void RTLInstDict::fixupParams()
{
	for (ParamEntry& param : DetParamMap) {
		param.m_mark = 0;
	}

	int mark = 1;

	for (auto iter = DetParamMap.begin(); iter != DetParamMap.end(); ++iter) {
		std::list<QString> funcParams;
		bool               haveCount = false;

		if (iter.value().m_kind == PARAM_VARIANT) {
			fixupParamsSub(iter.key(), funcParams, haveCount, mark++);
		}
	}
}


void RTLInstDict::fixupParamsSub(const QString& s, std::list<QString>& funcParams, bool& haveCount, int mark)
{
	ParamEntry& param = DetParamMap[s];

	if (param.m_params.size() == 0) {
		LOG_STREAM() << "Error in SSL File: Variant operand " << s << " has no branches. Well that's really useful...\n";
		return;
	}

	if (param.m_mark == mark) {
		return; /* Already seen this round. May indicate a cycle, but may not */
	}

	param.m_mark = mark;

	for (const QString& name : param.m_params) {
		ParamEntry& sub = DetParamMap[name];

		if (sub.m_kind == PARAM_VARIANT) {
			fixupParamsSub(name, funcParams, haveCount, mark);

			if (!haveCount) { /* Empty branch? */
				continue;
			}
		}
		else if (!haveCount) {
			haveCount = true;
			char buf[10];

			for (unsigned int i = 1; i <= sub.m_funcParams.size(); i++) {
				sprintf(buf, "__lp%u", i);
				funcParams.push_back(buf);
			}
		}

		if (funcParams.size() != sub.m_funcParams.size()) {
			LOG_STREAM() << "Error in SSL File: Variant operand " << s
						 << " does not have a fixed number of functional parameters:\n"
						 << "Expected " << funcParams.size() << ", but branch " << name
						 << " has " << sub.m_funcParams.size() << ".\n";
		}
		else if ((funcParams != sub.m_funcParams) && (sub.m_asgn != nullptr)) {
			/* Rename so all the parameter names match */
			for (auto i = funcParams.begin(), j = sub.m_funcParams.begin(); i != funcParams.end(); i++, j++) {
				Location  paramLoc(opParam, Const::get(*j), nullptr); // Location::param(j->c_str())
				SharedExp replace = Location::param(*i);
				sub.m_asgn->searchAndReplace(paramLoc, replace);
			}

			sub.m_funcParams = funcParams;
		}
	}

	//      if( param.funcParams.size() != funcParams.size() )
	//          theSemTable.setItem( n, cFUNCTION, 0, 0, funcParams.size(),
	//                               theSemTable[n].sName.c_str() );
	param.m_funcParams = funcParams;
}


/***************************************************************************/ /**
 * \brief         Returns the signature of the given instruction.
 * \param name - instruction name
 * \returns              the signature (name + number of operands)
 ******************************************************************************/
std::pair<QString, unsigned> RTLInstDict::getSignature(const char *name)
{
	// Take the argument, convert it to upper case and remove any _'s and .'s
	QString hlpr(name);

	hlpr = hlpr.replace(".", "").toUpper();
	// Look up the dictionary
	std::map<QString, TableEntry>::iterator it = idict.find(hlpr);

	if (it == idict.end()) {
		LOG_STREAM() << "Error: no entry for `" << name << "' in RTL dictionary\n";
		it = idict.find("NOP"); // At least, don't cause segfault
	}

	return {
			   hlpr, (it->second).m_params.size()
	};
}


/***************************************************************************/ /**
 * \brief         Scan the Exp* pointed to by exp; if its top level operator indicates even a partial type, then set
 *                        the expression's type, and return true
 * \note This version only inspects one expression
 * \param  exp - points to a Exp* to be scanned
 * \param  ty - ref to a Type object to put the partial type into
 * \returns true if a partial type is found
 ******************************************************************************/
bool RTLInstDict::partialType(Exp *exp, Type& ty)
{
	if (exp->isSizeCast()) {
		ty = *IntegerType::get(exp->access<Const, 1>()->getInt());
		return true;
	}

	if (exp->isFltConst()) {
		ty = *FloatType::get(64);
		return true;
	}

	return false;
}


/***************************************************************************/ /**
 * \brief         Returns an instance of a register transfer list for the instruction named 'name' with the actuals
 *                     given as the second parameter.
 * \param name - the name of the instruction (must correspond to one defined in the SSL file).
 * \param natPC - address at which the named instruction is located
 * \param actuals - the actual values
 * \returns   the instantiated list of Exps
 ******************************************************************************/
std::list<Instruction *> *RTLInstDict::instantiateRTL(const QString& name, Address natPC,
													  const std::vector<SharedExp>& actuals)
{
	QTextStream q_cerr(stderr);
	// If -f is in force, use the fast (but not as precise) name instead
	QString lname = name;

	// FIXME: settings
	//      if (progOptions.fastInstr) {
	if (0) {
		auto itf = fastMap.find(name);

		if (itf != fastMap.end()) {
			lname = itf->second;
		}
	}

	// Retrieve the dictionary entry for the named instruction
	auto dict_entry = idict.find(lname);

	if (dict_entry == idict.end()) { /* lname is not in dictionary */
		q_cerr << "ERROR: unknown instruction " << lname << " at " << natPC << ", ignoring.\n";
		return nullptr;
	}

	TableEntry& entry(dict_entry->second);

	return instantiateRTL(entry.m_rtl, natPC, entry.m_params, actuals);
}


/***************************************************************************/ /**
 * \brief         Returns an instance of a register transfer list for the parameterized rtlist with the given formals
 *      replaced with the actuals given as the third parameter.
 * \param   rtl - a register transfer list
 * \param   natPC - address at which the named instruction is located
 * \param   params - a list of formal parameters
 * \param   actuals - the actual parameter values
 * \returns the instantiated list of Exps
 ******************************************************************************/
std::list<Instruction *> *RTLInstDict::instantiateRTL(RTL& rtl, Address natPC, std::list<QString>& params,
													  const std::vector<SharedExp>& actuals)
{
	Q_UNUSED(natPC);
	assert(params.size() == actuals.size());

	// Get a deep copy of the template RTL
	std::list<Instruction *> *newList = new std::list<Instruction *>();
	rtl.deepCopyList(*newList);

	// Iterate through each Statement of the new list of stmts
	for (Instruction *ss : *newList) {
		// Search for the formals and replace them with the actuals
		auto param = params.begin();
		std::vector<SharedExp>::const_iterator actual = actuals.begin();

		for ( ; param != params.end(); param++, actual++) {
			/* Simple parameter - just construct the formal to search for */
			Location formal(opParam, Const::get(*param), nullptr); // Location::param(param->c_str());
			ss->searchAndReplace(formal, *actual);
			// delete formal;
		}

		ss->fixSuccessor();

		if (Boomerang::get()->debugDecoder) {
			QTextStream q_cout(stdout);
			q_cout << "            " << ss << "\n";
		}
	}

	transformPostVars(*newList, true);

	// Perform simplifications, e.g. *1 in Pentium addressing modes
	std::list<Instruction *>::iterator iter;

	for (iter = newList->begin(); iter != newList->end(); iter++) {
		(*iter)->simplify();
	}

	return newList;
}


/* Small struct for transformPostVars */
class transPost
{
public:
	bool used; // If the base expression (e.g. r[0]) is used
	// Important because if not, we don't have to make any
	// substitutions at all
	bool isNew;      // Not sure (MVE)
	SharedExp tmp;   // The temp to replace r[0]' with
	SharedExp post;  // The whole postvar expression. e.g. r[0]'
	SharedExp base;  // The base expression (e.g. r[0])
	SharedType type; // The type of the temporary (needed for the final assign)
};

/***************************************************************************/ /**
 * \brief Transform an RTL to eliminate any uses of post-variables.
 *
 * Note that the algorithm used expects to deal with simple
 * expressions as post vars, ie r[22], m[r[1]], generally things which aren't parameterized at a higher level. This is
 * ok for the translator (we do substitution first anyway), but may miss some optimizations for the emulator.
 * For the emulator, if parameters are detected within a postvar, we just force the temporary, which is always safe to
 * do.  (The parameter optimise is set to false for the emulator to achieve this).
 * Transform the given list into another list which doesn't have post-variables, by either adding temporaries or
 * just removing them where possible. Modifies the list passed, and also returns a pointer to it. Second
 * parameter indicates whether the routine should attempt to optimize the resulting output, ie to minimize the
 * number of temporaries. This is recommended for fully expanded expressions (ie within uqbt), but unsafe
 * otherwise.
 * \param rts the list of statements
 * \param optimise - try to remove temporary registers
 ******************************************************************************/
void RTLInstDict::transformPostVars(std::list<Instruction *>& rts, bool optimise)
{
	// Map from var (could be any expression really) to details
	std::map<SharedExp, transPost, lessExpStar> vars;
	int tmpcount = 1; // For making temp names unique
	                  // Exp* matchParam(1,idParam);    // ? Was never used anyway

#ifdef DEBUG_POSTVAR
	std::cout << "Transforming from:\n";

	for (Exp_CIT p = rts->begin(); p != rts->end(); p++) {
		std::cout << setw(8) << " ";
		(*p)->print(std::cout);
		std::cout << "\n";
	}
#endif

	// First pass: Scan for post-variables and usages of their referents
	for (Instruction *rt : rts) {
		// ss appears to be a list of expressions to be searched
		// It is either the LHS and RHS of an assignment, or it's the parameters of a flag call
		SharedExp ss;

		if (rt->isAssign()) {
			Assign    *rt_asgn((Assign *)rt);
			SharedExp lhs = rt_asgn->getLeft();
			SharedExp rhs = rt_asgn->getRight();

			// Look for assignments to post-variables
			if (lhs && lhs->isPostVar()) {
				if (vars.find(lhs) == vars.end()) {
					// Add a record in the map for this postvar
					transPost& el = vars[lhs];
					el.used = false;
					el.type = rt_asgn->getType();

					// Constuct a temporary. We should probably be smarter and actually check that it's not otherwise
					// used here.
					QString tmpname = QString("%1%2post").arg(el.type->getTempName()).arg((tmpcount++));
					el.tmp = Location::tempOf(Const::get(tmpname));

					// Keep a copy of the referrent. For example, if the lhs is r[0]', base is r[0]
					el.base  = lhs->getSubExp1();
					el.post  = lhs; // The whole post-var, e.g. r[0]'
					el.isNew = true;

					// The emulator generator sets optimise false
					// I think this forces always generating the temps (MVE)
					if (!optimise) {
						el.used  = true;
						el.isNew = false;
					}
				}
			}

			// For an assignment, the two expressions to search are the left and right hand sides (could just put the
			// whole assignment on, I suppose)
			assert(lhs != nullptr);
			assert(rhs != nullptr);
			ss = Binary::get(opList, lhs->clone(), Binary::get(opList, rhs->clone(), Terminal::get(opNil)));
		}
		else if (rt->isFlagAssgn()) {
			Assign *rt_asgn = (Assign *)rt;
			// Exp *lhs = rt_asgn->getLeft();
			auto rhs = rt_asgn->getRight();
			// An opFlagCall is assumed to be a Binary with a string and an opList of parameters
			ss = rhs->getSubExp2();
			assert(false); // was ss = (Binary *)((Binary *)rt)->getSubExp2();
		}

		/* Look for usages of post-variables' referents
		 * Trickier than you'd think, as we need to make sure to skip over the post-variables themselves. ie match
		 * r[0] but not r[0]'
		 * Note: back with SemStrs, we could use a match expression which was a wildcard prepended to the base
		 * expression; this would match either the base (r[0]) or the post-var (r[0]').
		 * Can't really use this with Exps, so we search twice; once for the base, and once for the post, and if we
		 * get more with the former, then we have a use of the base (consider r[0] + r[0]')
		 */
		for (auto& var : vars) {
			if (var.second.isNew) {
				// Make sure we don't match a var in its defining statement
				var.second.isNew = false;
				continue;
			}

			for (auto cur = ss; !cur->isNil(); cur = cur->getSubExp2()) {
				if (var.second.used) {
					break; // Don't bother; already know it's used
				}

				SharedExp s = cur->getSubExp1();

				if (!s) {
					continue;
				}

				if (*s == *var.second.base) {
					var.second.used = true;
					break;
				}

				std::list<SharedExp> res1, res2;
				s->searchAll(*var.second.base, res1);
				s->searchAll(*var.second.post, res2);

				// Each match of a post will also match the base.
				// But if there is a bare (non-post) use of the base, there will be a result in res1 that is not in res2
				if (res1.size() > res2.size()) {
					var.second.used = true;
				}
			}
		}
	}

	// Second pass: Replace post-variables with temporaries where needed
	for (Instruction *rt : rts) {
		for (auto& var : vars) {
			if (var.second.used) {
				rt->searchAndReplace(*var.first, var.second.tmp);
			}
			else {
				rt->searchAndReplace(*var.first, var.second.base);
			}
		}
	}

	// Finally: Append assignments where needed from temps to base vars
	// Example: esp' = esp-4; m[esp'] = modrm; FLAG(esp)
	// all the esp' are replaced with say tmp1, you need a "esp = tmp1" at the end to actually make the change
	for (auto& var : vars) {
		if (var.second.used) {
			Assign *te = new Assign(var.second.type, var.second.base->clone(), var.second.tmp);
			rts.push_back(te);
		}
		else {
			// The temp is either used (uncloned) in the assignment, or is deleted here
			// delete sr->second.tmp;
		}
	}

#ifdef DEBUG_POSTVAR
	std::cout << "\nTo =>\n";

	for (std::list<SharedExp>::iterator p = rts->begin(); p != rts->end(); p++) {
		std::cout << setw(8) << " ";
		(*p)->print(std::cout);
		std::cout << "\n";
	}
	std::cout << "\n";
#endif

	// return rts;
}


/** Reset the object to "undo" a readSSLFile()
 *
 * Called from test code if (e.g.) want to call readSSLFile() twice
 */
void RTLInstDict::reset()
{
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
	fetchExecCycle = nullptr;
}
