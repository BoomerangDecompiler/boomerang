/*
 * Copyright (C) 1997-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   proc.cc
 * OVERVIEW:   Implementation of the Proc hierachy (Proc, UserProc, LibProc).
 *			   All aspects of a procedure, apart from the actual code in the
 *			   Cfg, are stored here
 *
 * Copyright (C) 1997-2001, The University of Queensland, BT group
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 *============================================================================*/

/*
 * $Revision$	// 1.238.2.44
 *
 * 14 Mar 02 - Mike: Fixed a problem caused with 16-bit pushes in richards2
 * 20 Apr 02 - Mike: Mods for boomerang
 * 31 Jan 03 - Mike: Tabs and indenting
 * 03 Feb 03 - Mike: removeStatement no longer linear searches for the BB
 * 13 Jul 05 - Mike: Fixed a segfault in copyDecodedICTs with zero lentgth (!) BBs. Also one in the ad-hoc TA
 */

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include "proc.h"
#include <types.h>
#include <sstream>
#include <algorithm>		// For find()
#include "type.h"
#include "cluster.h"
#include "statement.h"
#include "register.h"
#include "rtl.h"
#include "prog.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "util.h"
#include "signature.h"
#include "boomerang.h"
#include "constraint.h"
#include "visitor.h"
#include "log.h"

typedef std::map<Statement*, int> RefCounter;

extern char debug_buffer[];		// Defined in basicblock.cpp, size DEBUG_BUFSIZE

/************************
 * Proc methods.
 ***********************/

Proc::~Proc()
{}

/*==============================================================================
 * FUNCTION:		Proc::Proc
 * OVERVIEW:		Constructor with name, native address.
 * PARAMETERS:		uNative - Native address of entry point of procedure
 * RETURNS:			<nothing>
 *============================================================================*/
Proc::Proc(Prog *prog, ADDRESS uNative, Signature *sig)
	 : prog(prog), signature(sig), address(uNative), m_firstCaller(NULL) 
{cluster = prog->getRootCluster();}

/*==============================================================================
 * FUNCTION:		Proc::getName
 * OVERVIEW:		Returns the name of this procedure
 * PARAMETERS:		<none>
 * RETURNS:			the name of this procedure
 *============================================================================*/
const char* Proc::getName() {
	assert(signature);
	return signature->getName();
}

/*==============================================================================
 * FUNCTION:		Proc::setName
 * OVERVIEW:		Sets the name of this procedure
 * PARAMETERS:		new name
 * RETURNS:			<nothing>
 *============================================================================*/
void Proc::setName(const char *nam) {
	assert(signature);
	signature->setName(nam);
}


/*==============================================================================
 * FUNCTION:		Proc::getNativeAddress
 * OVERVIEW:		Get the native address (entry point).
 * PARAMETERS:		<none>
 * RETURNS:			the native address of this procedure (entry point)
 *============================================================================*/
ADDRESS Proc::getNativeAddress() {
	return address;
}

void Proc::setNativeAddress(ADDRESS a) {
	address = a;
}

/*==============================================================================
 * FUNCTION:	  Proc::containsAddr
 * OVERVIEW:	  Return true if this procedure contains the given address
 * PARAMETERS:	  address
 * RETURNS:		  true if it does
 *============================================================================*/
bool UserProc::containsAddr(ADDRESS uAddr) {
	BB_IT it;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it))
		if (bb->getRTLs() && bb->getLowAddr() <= uAddr && bb->getHiAddr() >= uAddr)
			return true;	
	return false;
}

void Proc::renameParam(const char *oldName, const char *newName) { 
	signature->renameParam(oldName, newName); 
}

void UserProc::renameParam(const char *oldName, const char *newName)
{
	oldName = strdup(oldName);
	Proc::renameParam(oldName, newName);
	//cfg->searchAndReplace(Location::param(strdup(oldName), this), Location::param(strdup(newName), this));
}

void UserProc::setParamType(const char* nam, Type* ty) {
	signature->setParamType(nam, ty);
}

void UserProc::renameLocal(const char *oldName, const char *newName) {
	Type *t = locals[oldName];
	Exp *e = expFromSymbol(oldName);
	locals.erase(oldName);
	//Exp *l = symbolMap[e];
	Exp *n = Location::local(strdup(newName), this);
	symbolMap[e] = n;
	locals[strdup(newName)] = t;
	//cfg->searchAndReplace(l, n);
}

bool UserProc::searchAll(Exp* search, std::list<Exp*> &result)
{
	return cfg->searchAll(search, result);	
}

void Proc::printCallGraphXML(std::ostream &os, int depth, bool recurse)
{
	if (!DUMP_XML)
		return;
	visited = true;
	for (int i = 0; i < depth; i++)
		os << "	  ";
	os << "<proc name=\"" << getName() << "\"/>\n";
}

void UserProc::printCallGraphXML(std::ostream &os, int depth, bool recurse)
{
	if (!DUMP_XML)
		return;
	bool wasVisited = visited;
	visited = true;
	int i;
	for (i = 0; i < depth; i++)
		os << "	  ";
	os << "<proc name=\"" << getName() << "\">\n";
	if (recurse) {
		for (std::list<Proc*>::iterator it = calleeList.begin(); it != calleeList.end(); it++) 
			(*it)->printCallGraphXML(os, depth+1, !wasVisited && !(*it)->isVisited());
	}
	for (i = 0; i < depth; i++)
		os << "	  ";
	os << "</proc>\n";
}

void Proc::printDetailsXML() {
	if (!DUMP_XML)
		return;
	std::ofstream out((Boomerang::get()->getOutputPath() + getName() + "-details.xml").c_str());
	out << "<proc name=\"" << getName() << "\">\n";
	unsigned i;
	for (i = 0; i < signature->getNumParams(); i++)
		out << "   <param name=\"" << signature->getParamName(i) << "\" " << "exp=\"" << signature->getParamExp(i)
			<< "\" " << "type=\"" << signature->getParamType(i)->getCtype() << "\"\n";
	for (i = 0; i < signature->getNumReturns(); i++)
		out << "   <return exp=\"" << signature->getReturnExp(i) << "\" "
			<< "type=\"" << signature->getReturnType(i)->getCtype() << "\"/>\n";
	out << "</proc>\n";
	out.close();
}

void UserProc::printDecodedXML()
{
	if (!DUMP_XML)
		return;
	std::ofstream out((Boomerang::get()->getOutputPath() + getName() + "-decoded.xml").c_str());
	out << "<proc name=\"" << getName() << "\">\n";
	out << "	<decoded>\n";
	std::ostringstream os;
	print(os);
	std::string s = os.str();
	escapeXMLChars(s);
	out << s;
	out << "	</decoded>\n";
	out << "</proc>\n";
	out.close();
}

void UserProc::printAnalysedXML()
{
	if (!DUMP_XML)
		return;
	std::ofstream out((Boomerang::get()->getOutputPath() + getName() + "-analysed.xml").c_str());
	out << "<proc name=\"" << getName() << "\">\n";
	out << "	<analysed>\n";
	std::ostringstream os;
	print(os);
	std::string s = os.str();
	escapeXMLChars(s);
	out << s;
	out << "	</analysed>\n";
	out << "</proc>\n";
	out.close();
}

void UserProc::printSSAXML()
{
	if (!DUMP_XML)
		return;
	std::ofstream out((Boomerang::get()->getOutputPath() + getName() + "-ssa.xml").c_str());
	out << "<proc name=\"" << getName() << "\">\n";
	out << "	<ssa>\n";
	std::ostringstream os;
	print(os);
	std::string s = os.str();
	escapeXMLChars(s);
	out << s;
	out << "	</ssa>\n";
	out << "</proc>\n";
	out.close();
}


void UserProc::printXML()
{
	if (!DUMP_XML)
		return;
	printDetailsXML();
	printSSAXML();
	prog->printCallGraphXML();
	printUseGraph();
}

void UserProc::printUseGraph()
{
	std::ofstream out((Boomerang::get()->getOutputPath() + getName() + "-usegraph.dot").c_str());
	out << "digraph " << getName() << " {\n";
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		if (s->isPhi())
			out << s->getNumber() << " [shape=diamond];\n";
		LocationSet refs;
		s->addUsedLocs(refs);
		LocationSet::iterator rr;
		for (rr = refs.begin(); rr != refs.end(); rr++) {
			if (((Exp*)*rr)->isSubscript()) {
				RefExp *r = (RefExp*)*rr;
				if (r->getDef())
					out << r->getDef()->getNumber() << " -> " << s->getNumber() << ";\n";
			}
		}
	}
	out << "}\n";
	out.close();
}

/*==============================================================================
 * FUNCTION:		operator<<
 * OVERVIEW:		Output operator for a Proc object.
 * PARAMETERS:		os - output stream
 *					proc -
 * RETURNS:			os
 *============================================================================*/
//std::ostream& operator<<(std::ostream& os, Proc& proc) {
//	return proc.put(os);
//}


Proc *Proc::getFirstCaller() { 
	if (m_firstCaller == NULL && m_firstCallerAddr != NO_ADDRESS) {
		m_firstCaller = prog->findProc(m_firstCallerAddr);
		m_firstCallerAddr = NO_ADDRESS;
	}

	return m_firstCaller; 
}

/**********************
 * LibProc methods.
 *********************/

/*==============================================================================
 * FUNCTION:		LibProc::LibProc
 * OVERVIEW:		Constructor with name, native address.
 * PARAMETERS:		name - Name of procedure
 *					uNative - Native address of entry point of procedure
 * RETURNS:			<nothing>
 *============================================================================*/
LibProc::LibProc(Prog *prog, std::string& name, ADDRESS uNative) : Proc(prog, uNative, NULL) {
	Signature* sig = prog->getLibSignature(name.c_str());
	signature = sig;
}

LibProc::~LibProc()
{}

/*==============================================================================
 * FUNCTION:		LibProc::put
 * OVERVIEW:		Display on os.
 * PARAMETERS:		os -
 * RETURNS:			os
 *============================================================================*/
//std::ostream& LibProc::put(std::ostream& os) {
//	os << "library procedure `" << signature->getName() << "' resides at 0x";
//	return os << std::hex << address << std::endl;
//}

Exp *LibProc::getProven(Exp *left) {
	// Just use the signature information (all we have, after all)
	return signature->getProven(left);
}

bool LibProc::isPreserved(Exp* e) {
	return signature->isPreserved(e);
}

/**********************
 * UserProc methods.
 *********************/

/*==============================================================================
 * FUNCTION:		UserProc::UserProc
 * OVERVIEW:		Constructor with name, native address.
 * PARAMETERS:		name - Name of procedure
 *					uNative - Native address of entry point of procedure
 * RETURNS:			<nothing>
 *============================================================================*/
UserProc::UserProc() : Proc(), cfg(NULL), status(PROC_UNDECODED), maxDepth(-1),
		// decoded(false), analysed(false),
		nextLocal(0),	// decompileSeen(false), decompiled(false), isRecursive(false)
		cycleGroup(NULL), theReturnStatement(NULL) {
}
UserProc::UserProc(Prog *prog, std::string& name, ADDRESS uNative) :
		// Not quite ready for the below fix:
		// Proc(prog, uNative, prog->getDefaultSignature(name.c_str())),
		Proc(prog, uNative, new Signature(name.c_str())),
		cfg(new Cfg()), status(PROC_UNDECODED), maxDepth(-1),
		nextLocal(0), // decompileSeen(false), decompiled(false), isRecursive(false),
		cycleGroup(NULL), theReturnStatement(NULL), DFGcount(0)
{
	cfg->setProc(this);				 // Initialise cfg.myProc
}

UserProc::~UserProc() {
	if (cfg)
		delete cfg; 
}

/*==============================================================================
 * FUNCTION:		UserProc::deleteCFG
 * OVERVIEW:		Deletes the whole CFG for this proc object. Also clears the
 *					cfg pointer, to prevent strange errors after this is called
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void UserProc::deleteCFG() {
	delete cfg;
	cfg = NULL;
}

class lessEvaluate : public std::binary_function<SyntaxNode*, SyntaxNode*, bool> {
public:
	bool operator()(const SyntaxNode* x, const SyntaxNode* y) const
	{
		return ((SyntaxNode*)x)->getScore() > ((SyntaxNode*)y)->getScore();
	}
};

SyntaxNode *UserProc::getAST()
{
	int numBBs = 0;
	BlockSyntaxNode *init = new BlockSyntaxNode();
	BB_IT it;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
		BlockSyntaxNode *b = new BlockSyntaxNode();
		b->setBB(bb);
		init->addStatement(b);
		numBBs++;
	}
	
	// perform a best first search for the nicest AST
	std::priority_queue<SyntaxNode*, std::vector<SyntaxNode*>, lessEvaluate > ASTs;
	ASTs.push(init);

	SyntaxNode *best = init;
	int best_score = init->getScore();
	int count = 0;
	while (ASTs.size()) {
		if (best_score < numBBs * 2) {
			LOG << "exit early: " << best_score << "\n";
			break;
		}

		SyntaxNode *top = ASTs.top();
		ASTs.pop();
		int score = top->evaluate(top);

		printAST(top); // debug

		if (score < best_score) {
			if (best && top != best)
				delete best;
			best = top;
			best_score = score;
		}

		count++;
		if (count > 100)
			break;

		// add successors
		std::vector<SyntaxNode*> successors;
		top->addSuccessors(top, successors);
		for (unsigned i = 0; i < successors.size(); i++) {
			//successors[i]->addToScore(top->getScore());	// uncomment for A*
			successors[i]->addToScore(successors[i]->getDepth()); // or this
			ASTs.push(successors[i]);
		}

		if (top != best)
			delete top;
	}

	// clean up memory
	while(ASTs.size()) {
		SyntaxNode *top = ASTs.top();
		ASTs.pop();
		if (top != best)
			delete top;
	}
	
	return best;
}

int count = 1;

void UserProc::printAST(SyntaxNode *a)
{
	char s[1024];
	if (a == NULL)
		a = getAST();
	sprintf(s, "ast%i-%s.dot", count++, getName());
	std::ofstream of(s);
	of << "digraph " << getName() << " {" << std::endl;
	of << "	 label=\"score: " << a->evaluate(a) << "\";" << std::endl;
	a->printAST(a, of);
	of << "}" << std::endl;
	of.close();
}

/*==============================================================================
 * FUNCTION:		UserProc::setDecoded
 * OVERVIEW:		
 * PARAMETERS:		
 * RETURNS:			
 *============================================================================*/
void UserProc::setDecoded() {
	status = PROC_DECODED;
	printDecodedXML();
}

/*==============================================================================
 * FUNCTION:		UserProc::unDecode
 * OVERVIEW:		
 * PARAMETERS:		
 * RETURNS:			
 *============================================================================*/
void UserProc::unDecode() {
	cfg->clear();
	status = PROC_UNDECODED;
}

/*==============================================================================
 * FUNCTION:	UserProc::getEntryBB
 * OVERVIEW:	Get the BB with the entry point address for this procedure
 * PARAMETERS:	
 * RETURNS:		Pointer to the entry point BB, or NULL if not found
 *============================================================================*/
PBB UserProc::getEntryBB() {
	return cfg->getEntryBB();
}

/*==============================================================================
 * FUNCTION:		UserProc::setEntryBB
 * OVERVIEW:		Set the entry BB for this procedure
 * PARAMETERS:		<none>
 * RETURNS:			<nothing>
 *============================================================================*/
void UserProc::setEntryBB() {
	std::list<PBB>::iterator bbit;
	PBB pBB = cfg->getFirstBB(bbit);		// Get an iterator to the first BB
	// Usually, but not always, this will be the first BB, or at least in the first few
	while (pBB && address != pBB->getLowAddr()) {
		pBB = cfg->getNextBB(bbit);
	}
	cfg->setEntryBB(pBB);
}

/*==============================================================================
 * FUNCTION:		UserProc::addCallee
 * OVERVIEW:		Add this callee to the set of callees for this proc
 * PARAMETERS:		A pointer to the Proc object for the callee
 * RETURNS:			<nothing>
 *============================================================================*/
void UserProc::addCallee(Proc* callee) {
    // is it already in? (this is much slower than using a set)
    std::list<Proc*>::iterator cc;
	for (cc = calleeList.begin(); cc != calleeList.end(); cc++)
		if(*cc == callee)
            return; // it's already in

	calleeList.push_back(callee);
}

void UserProc::generateCode(HLLCode *hll) {
	assert(cfg);
	assert(getEntryBB());

	cfg->structure();
	if (!Boomerang::get()->noGlobals)
	    replaceExpressionsWithGlobals();	// FIXME: why here?
	if (!Boomerang::get()->noLocals) {
//		  while (nameStackLocations())
//			  replaceExpressionsWithSymbols();

#if 0		// In back end now...
			while (nameRegisters())
				replaceExpressionsWithSymbols();
#else
			nameRegisters();
#endif
	}
	removeUnusedLocals();

	// Note: don't try to remove unused statements here; that requires the
	// RefExps, which are all gone now (transformed out of SSA form)!

	if (VERBOSE || Boomerang::get()->printRtl)
		printToLog();

	hll->AddProcStart(this);
	
	// Local variables; print everything in the locals map
	std::map<std::string, Type*>::iterator last = locals.end();
	if (locals.size()) last--;
	for (std::map<std::string, Type*>::iterator it = locals.begin(); it != locals.end(); it++) {
		Type* locType = it->second;
		if (locType == NULL || locType->isVoid())
			locType = new IntegerType();
		hll->AddLocal(it->first.c_str(), locType, it == last);
	}

	std::list<PBB> followSet, gotoSet;
	getEntryBB()->generateCode(hll, 1, NULL, followSet, gotoSet);
	
	hll->AddProcEnd();

	if (!Boomerang::get()->noRemoveLabels)
		cfg->removeUnneededLabels(hll);
}

// print this userproc, maining for debugging
void UserProc::print(std::ostream &out) {
	signature->print(out);
	printParams(out);
	cfg->print(out);
	out << "\n";
}

void UserProc::printParams(std::ostream& out) {
	out << "parameters: ";
	bool first = true;
	for (StatementList::iterator pp = parameters.begin(); pp != parameters.end(); ++pp) {
		if (first)
			first = false;
		else
			out << ", ";
		out << ((Assign*)*pp)->getLeft();
	}
	out << "\nend parameters\n";
}

char* UserProc::prints() {
	std::ostringstream ost;
	print(ost);
	strncpy(debug_buffer, ost.str().c_str(), DEBUG_BUFSIZE);
	debug_buffer[DEBUG_BUFSIZE] = '\0';
	return debug_buffer;
}

void UserProc::dump() {
	print(std::cerr);
}

void UserProc::printToLog() {
	signature->printToLog();
	std::ostringstream ost;
	printParams(ost);
	dumpLocals(ost);
	LOG << ost.str().c_str();
	symbolMapToLog();
	LOG << "live variables: ";
	std::ostringstream ost2;
	col.print(ost2);
	LOG << ost2.str().c_str() << "\n";
	LOG << "end live variables\n";
	cfg->printToLog();
	LOG << "\n";
}

void UserProc::printDFG() { 
    char fname[1024];
    sprintf(fname, "%s%s-%i-dfg.dot", Boomerang::get()->getOutputPath().c_str(), getName(), DFGcount);
    DFGcount++;
    if (VERBOSE)
	LOG << "outputing DFG to " << fname << "\n";
    std::ofstream out(fname);
    out << "digraph " << getName() << " {\n";
    StatementList stmts;
    getStatements(stmts);
    StatementList::iterator it;
    for (it = stmts.begin(); it != stmts.end(); it++) {
        Statement *s = *it;
	if (s->isPhi())
	    out << s->getNumber() << " [shape=\"triangle\"];\n";
	if (s->isCall())
	    out << s->getNumber() << " [shape=\"box\"];\n";
	if (s->isBranch())
	    out << s->getNumber() << " [shape=\"diamond\"];\n";
	LocationSet refs;
	s->addUsedLocs(refs);
	LocationSet::iterator rr;
	for (rr = refs.begin(); rr != refs.end(); rr++) {
	    RefExp* r = dynamic_cast<RefExp*>(*rr);
	    if (r) {
		if (r->getDef())
	            out << r->getDef()->getNumber();
		else
		    out << "input";
		out << " -> ";
		if (s->isReturn())
		    out << "output";
		else
		    out << s->getNumber();
		out << ";\n";
	    }
	}
    }
    out << "}\n";
    out.close();
}

// initialise all statements
void UserProc::initStatements() {
	BB_IT it;
	BasicBlock::rtlit rit; StatementList::iterator sit;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
		for (Statement* s = bb->getFirstStmt(rit, sit); s; s = bb->getNextStmt(rit, sit)) {
			s->setProc(this);
			s->setBB(bb);
			CallStatement* call = dynamic_cast<CallStatement*>(s);
			if (call) {
				call->setSigArguments();
			}
		}
	}
}

void UserProc::numberStatements(int& stmtNum) {
	BB_IT it;
	BasicBlock::rtlit rit; StatementList::iterator sit;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
		for (Statement* s = bb->getFirstStmt(rit, sit); s; s = bb->getNextStmt(rit, sit))
			if (!s->isImplicit())		// Don't renumber implicits (remain number 0)
				s->setNumber(++stmtNum);
	}
}

void UserProc::numberPhiStatements(int& stmtNum) {
	BB_IT it;
	BasicBlock::rtlit rit; StatementList::iterator sit;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
		for (Statement* s = bb->getFirstStmt(rit, sit); s; s = bb->getNextStmt(rit, sit))
			if (s->isPhi() && s->getNumber() == 0)
				s->setNumber(++stmtNum);
	}
}


// get all statements
// Get to a statement list, so they come out in a reasonable and consistent order
void UserProc::getStatements(StatementList &stmts) {
	BB_IT it;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
		std::list<RTL*> *rtls = bb->getRTLs();
		if (rtls) {
			for (std::list<RTL*>::iterator rit = rtls->begin(); rit != rtls->end(); rit++) {
				RTL *rtl = *rit;
				for (RTL::iterator it = rtl->getList().begin(); it != rtl->getList().end(); it++) {
					if ((*it)->getBB() == NULL)
						(*it)->setBB(bb);
					if ((*it)->getProc() == NULL)
						(*it)->setProc(this);
					stmts.append(*it);
				}
			}
		}
	}
}

// Remove a statement. This is somewhat inefficient - we have to search the whole BB for the statement.
// Should use iterators or other context to find out how to erase "in place" (without having to linearly search)
void UserProc::removeStatement(Statement *stmt) {
	// remove anything proven about this statement
	for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); it != proven.end(); it++) {
		LocationSet refs;
		(*it)->addUsedLocs(refs);
		LocationSet::iterator rr;
		bool usesIt = false;
		for (rr = refs.begin(); rr != refs.end(); rr++) {
			Exp* r = *rr;
			if (r->isSubscript() && ((RefExp*)r)->getDef() == stmt) {
				usesIt = true;
				break;
			}
		}
		if (usesIt) {
			if (VERBOSE)
				LOG << "removing proven exp " << (*it) << " that uses statement being removed.\n";
			proven.erase(it);
			// it = proven.begin();
			continue;
		}
	}

	// remove from BB/RTL
	PBB bb = stmt->getBB();			// Get our enclosing BB
	std::list<RTL*> *rtls = bb->getRTLs();
	for (std::list<RTL*>::iterator rit = rtls->begin(); rit != rtls->end(); rit++) {
		std::list<Statement*>& stmts = (*rit)->getList();
		for (RTL::iterator it = stmts.begin(); it != stmts.end(); it++) {
			if (*it == stmt) {
				stmts.erase(it);
				return;
			}
		}
	}
}

#if 1
void UserProc::insertAssignAfter(Statement* s, Exp* left, Exp* right) {
	std::list<Statement*>::iterator it;
	std::list<Statement*>* stmts;
	if (s == NULL) {
		// This means right is supposed to be a parameter. We can insert the assignment at the start of the entryBB
		PBB entryBB = cfg->getEntryBB();
		std::list<RTL*> *rtls = entryBB->getRTLs();
		assert(rtls->size());		// Entry BB should have at least 1 RTL
		stmts = &rtls->front()->getList();
		it = stmts->begin();
	} else {
		// An ordinary definition; put the assignment at the end of s's BB
		PBB bb = s->getBB();		 // Get the enclosing BB for s
		std::list<RTL*> *rtls = bb->getRTLs();
		assert(rtls->size());		// If s is defined here, there should be
									// at least 1 RTL
		stmts = &rtls->back()->getList();
		it = stmts->end();			// Insert before the end
	}
	Assign* as = new Assign( left, right);
	stmts->insert(it, as);
	return;
}
#endif

void UserProc::insertStatementAfter(Statement* s, Statement* a) {
	// Note: this procedure is designed for the front end, where enclosing BBs are not set up yet
	// So this is an inefficient linear search!
	BB_IT bb;
	for (bb = cfg->begin(); bb != cfg->end(); bb++) {
		std::list<RTL*>::iterator rr;
		std::list<RTL*>* rtls = (*bb)->getRTLs();
		for (rr = rtls->begin(); rr != rtls->end(); rr++) {
			std::list<Statement*>& stmts = (*rr)->getList();
			std::list<Statement*>::iterator ss;
			for (ss = stmts.begin(); ss != stmts.end(); ss++) {
				if (*ss == s) {
					ss++;		// This is the point to insert before
					stmts.insert(ss, a);
					return;
				}
			}
		}
	}
	assert(false);			// Should have found this statement in this BB
}

/* Cycle detection logic:
 * *********************
 * At the top of partialDecompile, an empty set of known cycles is created. This set represents the procedures involved
 * in the current recursion group (and have to be analysed together). The union of these sets for all child procedures
 * becomes the return set for the current procedure. However, if (after all children have been processed: important!)
 * this set contains the current procedure, we have the maximal set of vertex-disjoint cycles, we can do the recursion
 * group analysis and return an empty set. At the end of the recursion group analysis, the whole group is complete,
 * ready for the global analyses. 
 cycleSet* decompile(CycleList path)	// path initially empty
	ret = new CycleSet
	append this to path
	for each child c called by this proc
		if c has already been visited but not finished
			// have new cycle
			if c is in path
			  insert every proc from c to the end of path into ret
			else
			  find first element f of path that is in c->cycleGroup
			  insert every proc from f to the end of path into ret
			union ret into cycleGroup 
		else
			ret1 = c->decompile(path)
			ret = union(ret, ret1)
			set return statement in call to that of c
	if (ret empty)
		prePresDecompile()
		middleDecompile()
		removeUnusedStatments()		// Not involved in recursion
	else
		prePresDecompile()
		middleDecompile()			// involved in recursion
		find first element f in path that is also in ret
		if (f == this)
			recursionGroupAnalysis(ret)
			empty ret
	remove last element (= this) from path
	return ret
 */
 

// Decompile this UserProc
CycleSet* UserProc::decompile(CycleList* path) {
	std::cout << "decompiling " << getName() << "\n";
	if (VERBOSE)
		LOG << "decompiling " << getName() << "\n";
	// Prevent infinite loops when there are cycles in the call graph
	if (status >= PROC_FINAL) {
		std::cerr << "Error: " << getName() << " already has status PROC_FINAL\n";
		return NULL;		// Already decompiled
	}
	if (status < PROC_DECODED)
		// Can happen e.g. if a callee is visible only after analysing a switch statement
		prog->reDecode(this);					// Actually decoding for the first time, not REdecoding

	status = PROC_VISITED; 						// We have visited this proc "on the way down"
	CycleSet* ret = new CycleSet;
	path->push_back(this);

	/*	*	*	*	*	*	*	*	*	*	*	*
	 *											*
	 *	R e c u r s e   t o   c h i l d r e n	*
	 *											*
	 *	*	*	*	*	*	*	*	*	*	*	*/

	if (!Boomerang::get()->noDecodeChildren) {
		// Recurse to children first, to perform a depth first search
		BB_IT it;
		// Look at each call, to do the DFS
		for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
			if (bb->getType() == CALL) {
				// The call Statement will be in the last RTL in this BB
				CallStatement* call = (CallStatement*)bb->getRTLs()->back()->getHlStmt();
				UserProc* c = (UserProc*)call->getDestProc();
				if (c->isLib()) continue;
				if (c->status == PROC_FINAL) {
					// Already decompiled, but the return statement still needs to be set for this call
					call->setCalleeReturn(c->getTheReturnStatement());
					continue;
				}
				// if c has already been visited but not done (apart from global analyses, i.e. we have a new cycle)
				if (c->status >= PROC_VISITED && c->status < PROC_INITDONE) {
					// if c is in path
					CycleList::iterator pi;
					bool inPath = false;
					for (pi = path->begin(); pi != path->end(); ++pi) {
						if (*pi == c) {
							inPath = true;
							break;
						}
					}
					if (inPath) {
						// Insert every proc from c to the end of path into ret
						do {
							ret->insert(*pi);
							++pi;
						} while (pi != path->end());
					} else {
						// find first element f of path that is in c->cycleGroup
						CycleList::iterator pi;
						Proc* f = NULL;
						assert(c->cycleGroup);
						for (pi = path->begin(); pi != path->end(); ++pi) {
							if (c->cycleGroup->find(*pi) != c->cycleGroup->end()) {
								f = *pi;
								break;
							}
						}
						assert(f);
						// Insert every proc from f to the end of path into ret
						do {
							ret->insert(*pi);
							++pi;
						} while (pi != path->end());
					}
					status = PROC_INCYCLE;	
					// union ret into cycleGroup
					if (cycleGroup == NULL)
						cycleGroup = new CycleSet;
					CycleSet::iterator cg;
					for (cg = ret->begin(); cg != ret->end(); ++cg)
						cycleGroup->insert(*cg);
				} else {
					// Not a new cycle
					if (VERBOSE)
						LOG << "Visiting on the way down child " << c->getName() << " from " << getName() << "\n";
					CycleSet* ret1 = c->decompile(path);
					// Union ret1 into ret
					ret->insert(ret1->begin(), ret1->end());
					// Child has at least done middleDecompile(), possibly more
					call->setCalleeReturn(c->getTheReturnStatement());
					if (ret->size() > 0)
						status = PROC_INCYCLE;
				}
			}
		}
	}

	// if (ret->size() == 0)
		// can now schedule this proc for complete decompilation
	initialiseDecompile();				// Sort the CFG, number statements, etc
	prePresDecompile();					// First part of what was middleDecompile()
	middleDecompile();					// Every proc gets at least this done

	// if ret is empty, i.e. no child involved in recursion
	if (ret->size() == 0) {
		remUnusedStmtEtc();	// Do the whole works
		status = PROC_FINAL;
		Boomerang::get()->alert_end_decompile(this);
	} else {
		// this proc's children, and hence this proc, is/are involved in recursion
		// Note if only the children are involved in recursion, ret is cleared (see below)
		// find first element f in path that is also in ret
		CycleList::iterator f;
		for (f = path->begin(); f != path->end(); ++f)
			if (ret->find(*f) != ret->end())
				break;
		if (*f == this) {
			// When threads are implemented, we can queue this group of procs for group analysis
			recursionGroupAnalysis(ret);// Includes remUnusedStmtEtc on all procs in ret
			ret->clear();
		}
	}

	// Remove last element (= this) from path
	path->erase(--path->end());

	return ret;
}

/*	*	*	*	*	*	*	*	*	*	*	*
 *											*
 *		D e c o m p i l e   p r o p e r		*
 *			( i n i t i a l )				*
 *											*
 *	*	*	*	*	*	*	*	*	*	*	*/

void UserProc::initialiseDecompile() {

	Boomerang::get()->alert_start_decompile(this);
	if (VERBOSE) LOG << "initialise decompile for " << getName() << "\n";

	// Sort by address, so printouts make sense
	cfg->sortByAddress();

	// Initialise statements
	initStatements();

	if (VERBOSE) {
		LOG << "=== Debug Print Before SSA for " << getName() << " ===\n";
		printToLog();
		LOG << "=== End Debug Print Before SSA for " << getName() << " ===\n\n";
	}

	// Compute dominance frontier
	df.dominators(cfg);

	// Number the statements
	stmtNumber = 0;
	numberStatements(stmtNumber); 

	printXML();

	if (Boomerang::get()->noDecompile) {
		status = PROC_FINAL;				// ??!
		return;
	}

	if (VERBOSE) {
		LOG << "--- debug initial print after decoding for " << getName() << " ---\n";
		printToLog();
		LOG << "=== end initial debug print after decoding for " << getName() << " ===\n\n";
	}
}

void UserProc::prePresDecompile() {

	if (VERBOSE) LOG << "initial decompile for " << getName() << "\n";

	status = PROC_PREPRES;					// Status is now pre-preservation

	// Update the defines in the calls. Will redo if involved in recursion
	updateCallDefines();

	// For each memory depth (1). First path is just to do the initial propagation (especially for the stack pointer),
	// place phi functions, number statements, initial propagation
	maxDepth = findMaxDepth() + 1;
	if (Boomerang::get()->maxMemDepth < maxDepth)
		maxDepth = Boomerang::get()->maxMemDepth;
	int depth;
	for (depth = 0; depth <= maxDepth; depth++) {

		if (VERBOSE)
			LOG << "placing phi functions at depth " << depth << "\n";
		// Place the phi functions for this memory depth
		df.placePhiFunctions(depth, this);

		if (VERBOSE)
			LOG << "numbering phi statements at depth " << depth << "\n";
		// Number them
		numberPhiStatements(stmtNumber);

		if (/* status == PROC_INCYCLE && */ depth > 0) {
			// When involved in cycles, there can be phi statements referring to call statements that have not yet been
			// untangled (e.g. because we don't know if the stack pointer is preserved or not).
			// As a result, fib can't prove that r29 is preserved, for example. To avoid this problem, we don't rename
			// or propagate memory locations until recursionGroupAnalysis time.
			if (VERBOSE)
				LOG << "### not renaming variables (1) at depth " << depth <<
					// " since " << getName() << " is involved in a recursion group";
					"\n";
		} else {
			if (VERBOSE)
				LOG << "renaming block variables (1) at depth " << depth << "\n";
			// Rename variables
			doRenameBlockVars(depth, false);
			if (VERBOSE) {
				LOG << "\n--- after rename (1) for " << getName() << " at depth " << depth << ": ---\n";
				printToLog();
				LOG << "\n=== done after rename (1) for " << getName() << " at depth " << depth << ": ===\n\n";
			}

			propagateStatements(depth);
			if (VERBOSE) {
				LOG << "\n--- after propagation (1) for " << getName() << " at depth " << depth << ": ---\n";
				printToLog();
				LOG << "\n=== done after propagation (1) for " << getName() << " at depth " << depth << ": ===\n\n";
			}
		}

		// The call bypass logic should be staged as well. For example, consider m[r1{11}]{11} where 11 is a call.
		// The first stage bypass yields m[r1{2}]{11}, which needs another round of propagation to yield m[r1{-}-32]{11}
		// (which can safely be processed at depth 1).
		// Except that this is inherent in the visitor nature of the latest algorithm.
		fixCallAndPhiRefs();		// Bypass children that are finalised (if any)
		if (status != PROC_INCYCLE || depth == 0)
			propagateStatements(depth);
		if (VERBOSE) {
			LOG << "\n--- after call and phi bypass (1) of " << getName() << " at depth " << depth << ": ---\n";
			printToLog();
			LOG << "\n=== done after call and phi bypass (1) of " << getName() << " at depth " << depth << ": ===\n\n";
		}
	}
	status = PROC_PREPRES;			// Now we are ready for preservation
}

void UserProc::middleDecompile() {
	findSpPreservation();
	// Oops - the idea of splitting the sp from the rest of the preservations was to allow correct naming of locals
	// so you are alias conservative. But of course some locals are ebp based, and so these will never be correct
	// until all the registers have preservation analysis done. So I may as well do them all together here.
	findPreserveds();
	fixCallAndPhiRefs(); 	// Propagate and bypass sp
	if (VERBOSE) {
		LOG << "--- after preservation, bypass and propagation ---\n";
		printToLog();
		LOG << "=== end after preservation, bypass and propagation ===\n";
	}
	status = PROC_PRESERVEDS;		// Preservation done

	if (!Boomerang::get()->noPromote)
		// We want functions other than main to be promoted. Needed before mapExpressionsToLocals
		promoteSignature();
	// The problem with doing locals too early is that the symbol map ends up with some {-} and some {0}
	//mapExpressionsToLocals();
	
	// Now is the time to allow propagate m[...]
	//status = PROC_VISITED;
#if 0
	for (p = cs->begin(); p != cs->end(); ++p) {
		for (int d=0; d < maxDepth; ++d) {
			(*p)->doRenameBlockVars(d, true);
			propagateAtDepth(d);
		}
		if (VERBOSE) {
			LOG << "--- after propagation of m[...]\n";
			(*p)->printToLog();
			LOG << "=== after propagation of m[...]\n";
		}
	}
#endif


	// Update the arguments for calls (mainly for the non recursion affected calls)
	// Ick! We have never run renameBlockVars at depth > 0 at this point, so the collectors in the call statements
	// don't have definitions for m[...], so the findDefFor() in the localiseExp() will fail.
	// We don't want to collect rubbish like m[esp{51}+8] where 51 is a phi statement that will go away with bypass
	// analysis, so it's correct to leave renameBlockVars sensitive to the depth parameter
	updateBlockVars(1);				// Minimum depth 1
	updateArguments();

	// For each memory depth (2)
	int depth;
	for (depth = 0; depth <= maxDepth; depth++) {
		// Redo the renaming process to take into account the arguments
		if (VERBOSE)
			LOG << "renaming block variables (2) at depth " << depth << "\n";
		// Rename variables
		doRenameBlockVars(depth, false);						// E.g. for new arguments

		// Seed the return statement with reaching definitions
		if (theReturnStatement) {
			theReturnStatement->updateModifieds();		// Everything including new arguments reaching the exit
			theReturnStatement->updateReturns();
		}

		if (status != PROC_INCYCLE || depth == 0) {
			// This gets called so much... it would be great to be able to "repair" the names...
			doRenameBlockVars(depth, false);					// E.g. update call use collectors with return uses
		}

		printXML();

		// Print if requested
		if (VERBOSE) {		// was if debugPrintSSA
			LOG << "--- debug print SSA for " << getName() << " at depth " << depth << " (no propagations) ---\n";
			printToLog();
			LOG << "=== end debug print SSA for " << getName() << " at depth " << depth << " ===\n\n";
			printDFG();
		}
		
		Boomerang::get()->alert_decompile_SSADepth(this, depth);

		if (depth == maxDepth) {
			//fixCallAndPhiRefs();			// FIXME: needed?
			if (processConstants()) {
				for (int i = 0; i <= maxDepth; i++) {
					if (status != PROC_INCYCLE || i == 0) {
						doRenameBlockVars(i, true);				// Needed if there was an indirect call to an ellipsis
																// function
					}
				}
			}
			//removeRedundantPhis();
		}
		//fixCallAndPhiRefs();				// FIXME: needed? If so, document why
		// recognising globals early prevents them from becoming parameters
		if (depth == maxDepth)		// Else Sparc problems... MVE FIXME: Exactly why?
			if (!Boomerang::get()->noGlobals)
				replaceExpressionsWithGlobals();
#if 0 	// No: recovering parameters must be late: after removal of unused statements, so don't get phantom parameters
		// from preserveds. Note that some preserveds could be prarameters.
		if (depth > 0 && !Boomerang::get()->noChangeSignatures) {
			findFinalParameters();
			//trimParameters(depth);
		}
#endif

#if 0	// No need to do a whole propagation phase any more; addArguments does the propagation now
		// if we've added new parameters, need to do propagations up to this depth.  it's a recursive function thing.
		if (nparams != signature->getNumParams()) {
			for (int depth_tmp = 0; depth_tmp < depth; depth_tmp++) {
				// Propagate at this memory depth
				for (int td = maxDepth; td >= 0; td--) {
					if (VERBOSE)
						LOG << "parameter propagating at depth " << depth_tmp << " to depth " << td << "\n";
					propagateStatements(depth_tmp, td);
					for (int i = 0; i <= depth_tmp; i++)
						doRenameBlockVars(i, true);
				}
			}
			doRenameBlockVars(depth, true);
			printXML();
			if (VERBOSE) {
				LOG << "=== debug print SSA for " << getName() << " at memory depth " << depth
					<< " (after adding new parameters) ===\n";
				printToLog();
				LOG << "=== end debug print SSA for " << getName() << " at depth " << depth << " ===\n\n";
			}
		}
#endif

		// replacing expressions with Parameters as we go
		if (!Boomerang::get()->noParameterNames) {
			replaceExpressionsWithParameters(depth);
			if (status != PROC_INCYCLE || depth == 0)
				doRenameBlockVars(depth, true);
		}

#if 1	// FIXME: Check if this is needed any more. At least fib seems to need it at present.
		if (!Boomerang::get()->noChangeSignatures) {
			// addNewReturns(depth);
			for (int i=0; i < 3; i++) {		// FIXME: should be iterate until no change
			 	if (VERBOSE)
					LOG << "### update returns loop iteration " << i << " ###\n";
				if (status != PROC_INCYCLE || depth == 0)
					doRenameBlockVars(depth, true);
				findPreserveds();
				updateReturnTypes();
				updateCallDefines();		// Returns have uses which affect call defines (if childless)
				fixCallAndPhiRefs();
				findPreserveds();			// Preserveds subtract from returns
			}
			printXML();
			if (VERBOSE) {
				LOG << "--- debug print SSA for " << getName() << " at memory depth " << depth <<
					" (after updating returns) ---\n";
				printToLog();
				LOG << "=== end debug print SSA for " << getName() << " at depth " << depth << " ===\n\n";
			}
		}
#endif

		printXML();
		// Print if requested
		if (VERBOSE) {		// was if debugPrintSSA
			LOG << "--- debug print SSA for " << getName() << " at memory depth " << depth <<
				" (after trimming return set) ---\n";
			printToLog();
			LOG << "=== end debug print SSA for " << getName() << " at depth " << depth << " ===\n\n";
		}

		Boomerang::get()->alert_decompile_beforePropagate(this, depth);

		// Propagate at this memory depth
		bool convert;			// True when indirect call converted to direct
		do {
			convert = false;
			if (VERBOSE)
				LOG << "propagating at depth " << depth << "\n";
			convert |= propagateStatements(depth);
			if (!convert) 		// Just calling renameBlockVars now can cause problems
				for (int i = 0; i <= depth; i++)
					doRenameBlockVars(i, true);
			// If you have an indirect to direct call conversion, some propagations that were blocked by
			// the indirect call might now succeed, and may be needed to prevent alias problems
			if (convert) {
				if (VERBOSE)
					LOG << "\nabout to restart propagations and dataflow at depth " << depth <<
						" due to conversion of indirect to direct call(s)\n\n";
				depth = 0;			// Start again from depth 0
				doRenameBlockVars(0, true); 			// Initial dataflow level 0
				LOG << "\nafter rename (2) of " << getName() << ":\n";
				printToLog();
				LOG << "\ndone after rename (2) of " << getName() << ":\n\n";
			}
		} while (convert);

		printXML();
		if (VERBOSE) {
			LOG << "--- after propagate for " << getName() << " at memory depth " << depth << " ---\n";
			printToLog();
			LOG << "=== end propagate for " << getName() << " at depth " << depth << " ===\n\n";
		}

		Boomerang::get()->alert_decompile_afterPropagate(this, depth);

	}

	// Check for indirect jumps or calls not already removed by propagation of constants
	if (cfg->decodeIndirectJmp(this)) {
		// There was at least one indirect jump or call found and decoded. That means that most of what has been done
		// to this function so far is invalid. So redo everything. Very expensive!!
		LOG << "=== about to restart decompilation of " << getName() <<
			" because indirect jumps or calls have been analysed\n\n";
		// First copy any new indirect jumps or calls that were decoded this time around. Just copy them all, the map
		// will prevent duplicates
		copyDecodedICTs();
		// Now, decode from scratch
		theReturnStatement = NULL;
		cfg->clear();
		std::ofstream os;
		prog->reDecode(this);
		decompile(new CycleList);			// Restart decompiling this proc
		return;
	}

	findPreserveds();

	// Used to be later...
	if (!Boomerang::get()->noParameterNames) {
		for (int i = maxDepth; i >= 0; i--) {
			//replaceExpressionsWithParameters(df, i);
			//mapExpressionsToLocals(true);
			// doRenameBlockVars(i, true); 			// MVE: is this really needed?
		}
		findPreserveds();		// FIXME: is this necessary here?
		//fixCallBypass();	// FIXME: surely this is not necessary now?
		//trimParameters();	// FIXME: surely there aren't any parameters to trim yet?
		if (VERBOSE) {
			LOG << "--- after replacing expressions, trimming params and returns for " << getName() << " ---\n";
			printToLog();
			LOG << "=== end after replacing expressions, trimming params and returns for " << getName() << " ===\n";
		}
	}

	if (VERBOSE)
		LOG << "===== end initial decompile for " << getName() << " =====\n\n";
	status = PROC_INITDONE;
}

/*	*	*	*	*	*	*	*	*	*	*	*	*	*
 *													*
 *	R e m o v e   u n u s e d   s t a t e m e n t s	*
 *													*
 *	*	*	*	*	*	*	*	*	*	*	*	*	*/

void UserProc::remUnusedStmtEtc() {

	if (VERBOSE)
		LOG << "--- remove unused statements for " << getName() << " ---\n";
	// A temporary hack to remove %CF = %CF{7} when 7 isn't a SUBFLAGS
	if (theReturnStatement)
		theReturnStatement->specialProcessing();

	// Perform type analysis. If we are relying (as we are at present) on TA to perform ellipsis processing,
	// do an initial TA pass now. Ellipsis processing often reveals additional uses (e.g. additional parameters
	// to printf/scanf), and removing unused statements is unsafe without full use information
	typeAnalysis();

	// Only remove unused statements after decompiling as much as possible of the proc
	for (int depth = 0; depth <= maxDepth; depth++) {
		// Remove unused statements
		RefCounter refCounts;			// The map
		// Count the references first
		countRefs(refCounts);
		// Now remove any that have no used
		if (!Boomerang::get()->noRemoveNull)
			remUnusedStmtEtc(refCounts, depth);

		// Remove null statements
		if (!Boomerang::get()->noRemoveNull)
			removeNullStatements();

		printXML();
		if (VERBOSE && !Boomerang::get()->noRemoveNull) {
			LOG << "--- after removing unused and null statements depth " << depth << " for " << getName() << " ---\n";
			printToLog();
			LOG << "=== end after removing unused statements for " << getName() << " ===\n\n";
		}
		Boomerang::get()->alert_decompile_afterRemoveStmts(this, depth);
	}

	// FIXME: not sure where these belong as yet...
	findFinalParameters();
	if (!Boomerang::get()->noParameterNames) {
		for (int i = maxDepth; i >= 0; i--) {
			replaceExpressionsWithParameters(i);
			//doRenameBlockVars(i, true);					// MVE: is this really needed?
		}
		//findPreserveds();		// FIXME: is this necessary here?
		//fixCallAndPhiRef();		// FIXME: surely this is not necessary now?
		//trimParameters();		// FIXME: check
		if (VERBOSE) {
			LOG << "--- after adding new parameters ---\n";
			printToLog();
			LOG << "=== end after adding new parameters ===\n";
		}
	}
	updateCalls();				// Or just updateArguments?
	
	if (VERBOSE) {
		LOG << "--- after remove unused statements etc for " << getName() << "\n";
		printToLog();
		LOG << "=== after remove unused statements etc for " << getName() << "\n";
	}
}

#if 0
/*	*	*	*	*	*	*	*	*	*	*	*
 *											*
 *		D e c o m p i l e   p r o p e r		*
 *				( f i n a l )				*
 *											*
 *	*	*	*	*	*	*	*	*	*	*	*/

void UserProc::finalDecompile() {
	if (VERBOSE)
		LOG << "=== final decompile for " << getName() << " ===\n";

	// Data flow based type analysis
	// Want to be after all propagation, but before converting expressions to locals etc
	if (DFA_TYPE_ANALYSIS) {
		if (VERBOSE || DEBUG_TA)
			LOG << "--- start data flow based type analysis for " << getName() << " ---\n";

		// Now we need to add the implicit assignments. Doing this earlier is extremely problematic, because
		// of all the m[...] that change their sorting order as their arguments get subscripted or propagated into
		addImplicitAssigns();

		bool first = true;
		do {
			if (!first)
				propagateAtDepth(maxDepth);		// HACK: Can sometimes be needed, if call was indirect
												// FIXME: Check if still needed
			first = false;
			dfaTypeAnalysis();
		} while (ellipsisProcessing());
		if (VERBOSE || DEBUG_TA)
			LOG << "=== end type analysis for " << getName() << " ===\n";
	}

#if 0		// Want this earlier, before remove unused statements, so assignments to locals can work
	if (!Boomerang::get()->noParameterNames) {
		for (int i = maxDepth; i >= 0; i--) {
			replaceExpressionsWithParameters(df, i);
			mapExpressionsToLocals(true);
			//doRenameBlockVars(i, true);					// MVE: is this really needed?
		}
		findPreserveds();
		fixCallBypass();
		trimParameters();
		if (VERBOSE) {
			LOG << "=== after replacing expressions, trimming params and returns ===\n";
			printToLog();
			LOG << "=== end after replacing expressions, trimming params and returns ===\n";
			LOG << "===== end after replacing params =====\n\n";
			printDFG();
		}
	}
#endif

	//processConstants();
	//sortParameters();

//	if (DFA_TYPE_ANALYSIS)
		// This has to be done fairly late, e.g. after trimming returns. This seems to be something to do with
		// call statements not defining local variables properly. It is now delayed until code generation.
//		mapRegistersToLocals();

	printXML();

	status = PROC_FINAL;		// Now fully decompiled (apart from one final pass, and transforming out of SSA form)
	if (VERBOSE)
		LOG << "=== end final decompile for " << getName() << "\n";
	Boomerang::get()->alert_end_decompile(this);
}
#endif


void UserProc::recursionGroupAnalysis(CycleSet* cs) {
	/* Overall algorithm:
		mark all calls involved in cs as non-childless
		for each proc in cs
			update parameters and returns, redoing call bypass, until no change
		for each proc in cs
			remove unused statements
		for each proc in cs
			update parameters and returns, redoing call bypass, until no change
	*/
	if (VERBOSE) {
		LOG << "recursion group analysis for ";
		CycleSet::iterator csi;
		for (csi = cs->begin(); csi != cs->end(); ++csi)
			LOG << (*csi)->getName() << ", ";
		LOG << "\n";
	}

	// Mark all the relevant calls as non childless (will harmlessly get done again later)
	CycleSet::iterator it;
	for (it = cs->begin(); it != cs->end(); it++)
		(*it)->markAsNonChildless(cs);

	CycleSet::iterator p;
	// Need to propagate into the initial arguments, since arguments are uses, and we are about to remove unused
	// statements.
	for (p = cs->begin(); p != cs->end(); ++p) {
		(*p)->initialParameters();
		(*p)->updateArguments();
		(*p)->propagateAtDepth(maxDepth);			// Need to propagate into arguments
	}

	// while no change
for (int i=0; i < 2; i++) {
	for (p = cs->begin(); p != cs->end(); ++p) {
		(*p)->remUnusedStmtEtc();				// Also does final parameters and arguments at present
	}
}
	status = PROC_FINAL;		// Now fully decompiled (apart from one final pass, and transforming out of SSA form)
	if (VERBOSE)
		LOG << "=== end recursion group analysis ===\n";
	Boomerang::get()->alert_end_decompile(this);

}

#if 0			// NO! Turns out we don't need the individual cycles, just the maximal edge-disjoint cycle set
void UserProc::findSubCycles(CycleList& path, CycleSet& cs, CycleSetSet& sset) {
	// path is a list of the procs in the path from the top node down to but excluding the current node; initially empty
	// cs is the set of all nodes involved in this recursion group (input)
	// sset is the set of sets of nodes that is the final result (output)
	/* Overall algorithm:
		proc findSubCycles (list path, set cs, set of sets sset)
			if (path contains this)
				insert the list from this to end into sset
			else
				for each child c of this also in cs
					if c == this insert a self cycle
					else
						append this to path
						processChild(path, cs, sset)
						remove this from end of path
			return		
	*/
	if (VERBOSE) {
		LOG << "### finding subcycles for " << getName() << "; cycle set: ";
		CycleSet::iterator cc;
		bool first = true;
		for (cc = cs.begin(); cc != cs.end(); ++cc) {
			if (first)
				first = false;
			else
				LOG << ", ";
			LOG << (*cc)->getName();
		}
		LOG << "\n";
	}

	// if path contains this
	CycleList::iterator pp;
	bool cycleFound = false;
	for (pp = path.begin(); pp != path.end(); ++pp) {
		if (*pp == this) {
			cycleFound = true;
			CycleSet* newCycle = new CycleSet;
			do {
				newCycle->insert(*pp);
				++pp;
			} while (pp != path.end());
			sset.insert(newCycle);
			break;
		}
	}
	if (!cycleFound) {
		BB_IT it;
		// Look at each call, to find children in the call graph
		for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
			if (bb->getType() != CALL) continue;
			// The call Statement will be in the last RTL in this BB
			CallStatement* call = (CallStatement*)bb->getRTLs()->back()->getHlStmt();
			UserProc* c = (UserProc*)call->getDestProc();
			if (cs.find(c) == cs.end())
				continue;			// Only interested in calls to other procs in this cycle set
			if (c == this) {
				// Always seem to need special processing for self recursion (unit) cycles
				CycleSet* selfCycle = new CycleSet;
				selfCycle->insert(this);
				sset.insert(selfCycle);
			} else {
				path.push_back(this);
				c->findSubCycles(path, cs, sset);
				path.erase(--path.end());
			}
		}
	}
	if (VERBOSE) {
		LOG << "### end finding subcycles for " << getName() << "; sub cycles:";
		int i = 1;
		CycleSetSet::iterator ss;
		for (ss = sset.begin(); ss != sset.end(); ++ss, ++i) {
			LOG << " cycle (" << i << "): ";
			CycleSet::iterator cc;
			CycleSet* cs = *ss;
			bool first = true;
			for (cc = cs->begin(); cc != cs->end(); ++cc) {
				if (first)
					first = false;
				else
					LOG << ", ";
				LOG << (*cc)->getName();
			}
			LOG << "\n";
		}
	}
}
#endif

void UserProc::updateCalls() {
	if (VERBOSE)
		LOG << "### updateCalls for " << getName() << " ###\n";
	updateCallDefines();
	updateArguments();
	if (VERBOSE) {
		LOG << "--- after update calls for " << getName() << "\n";
		printToLog();
		LOG << "=== after update calls for " << getName() << "\n";
	}
}

void UserProc::doRenameBlockVars(int depth, bool clearStacks) {
	if (VERBOSE)
		LOG << "### rename block vars for " << getName() << " depth " << depth << ", clear = " << clearStacks <<
			" ###\n";
	df.renameBlockVars(this, 0, depth, clearStacks);
}

void UserProc::updateBlockVars()
{
	clearUses();										// Experimental
	//int depth = findMaxDepth() + 1;					// FIXME: why +1?
	int depth = maxDepth;
	for (int i = 0; i <= depth; i++) {
		if (status != PROC_INCYCLE || i == 0)
			doRenameBlockVars(i, true);
		else
			doRenameBlockVars(0, true);					// FIXME: why repeat this more than once?
	}
}

void UserProc::updateBlockVars(int minDepth) {
	if (VERBOSE)
		LOG << "--- begin update block vars at min depth " << minDepth << " ---\n";
	for (int i = minDepth; i <= maxDepth; i++) {
		doRenameBlockVars(i, true);
		// After a renaming at level n, it's important to do propagation at level n before renaming at level n+1
		// For example: m[m[a+K1]+K2]: you initially have m[m[a+K1]{d1}+K2] and you want to propagate from d1 before
		// deciding that the level 2 expression isn't defined anywhere.
		propagateAtDepth(i);
	}
	if (VERBOSE)
		LOG << "=== end update block vars at min depth " << minDepth << " ===\n";
}

void UserProc::propagateAtDepth(int depth)
{
	if (status == PROC_INCYCLE && depth > 0)
		return;
	propagateStatements(depth);
	for (int i = 0; i <= depth; i++)
		doRenameBlockVars(i, true);
}

int UserProc::findMaxDepth() {
	StatementList stmts;
	getStatements(stmts);
	int maxDepth = 0;
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		// Assume only need to check assignments
		if (s->getKind() == STMT_ASSIGN) {
			int depth = ((Assign*)s)->getMemDepth();
			if (depth > maxDepth) maxDepth = depth;
		}
	}
	return maxDepth;
}

#if 0
void UserProc::removeRedundantPhis() {
	if (VERBOSE || DEBUG_UNUSED)
		LOG << "removing redundant phi statements" << "\n";

	// some phis are just not used, after certain operations e.g. fixCallBypass
	RefCounter refCounts;
	countRefs(refCounts);

	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		if (s->isPhi()) {
			bool unused = false;
			PhiAssign *pa = (PhiAssign*)s;
			if (refCounts[s] == 0)
				unused = true;
			else if (refCounts[s] == 1) {
				/* This looks pretty good, if all the statements in a phi are either NULL or a call to this proc, then 
				 * the phi is redundant.  However, we only remove it if the only use is in the return statement.
				 */
				RefExp *r = new RefExp(pa->getLeft()->clone(), s);
				bool usedInRet = false;
				if (theReturnStatement)
					usedInRet = theReturnStatement->usesExp(r);
				delete r;
				if (usedInRet) {
					bool allZeroOrSelfCall = true;
					PhiAssign::iterator it1;
					for (it1 = pa->begin(); it1 != pa->end(); it1++) {
						Statement* s1 = it1->def;
						if (s1 && (!s1->isCall() || ((CallStatement*)s1)->getDestProc() != this))
							allZeroOrSelfCall = false;
					}
					if (allZeroOrSelfCall) {
						if (VERBOSE || DEBUG_UNUSED)
							LOG << "removing phi using shakey hack:\n";
						unused = true;
						removeReturn(pa->getLeft());
					}
				}
			} else {
#if 0		// NO! This essentially knocks out ALL the phis (I think). MVE
			// More thought required.
				// Check to see if all the refs are to either the same thing
				// or to NULL. If so, they will (would have been?) removed in
				// the fromSSA code. Removing them here allows some pesky
				// statements (e.g. assignments to %pc) to be removed
				LocationSet refs;
				p->addUsedLocs(refs);
				Exp* first = *refs.begin();
				bool same = true;
				LocationSet::iterator rr;
				for (rr = refs.begin(); rr != refs.end(); rr++) {
					if (!(**rr *= *first)) {	   // Ref-insensitive compare
						same = false;
						break;
					}
				}
				if (same) {
					// Is the left of the phi assignment the same base variable
					// as all the operands (or the operand is NULL) ?
					if (*s->getLeft() *= *first) {
						if (DEBUG_UNUSED)
							LOG << "removing phi: left and all refs same or 0: " << s << "\n";
						// Just removing the refs will work, or removing the whole phi
						removeStatement(s);
					}
				}
#endif
			}
			if (unused) {
				if (DEBUG_UNUSED)
					LOG << "removing redundant phi " << s << "\n";
				removeStatement(s);
			}
		}
	}

#if 0
	stmts.clear();
	getStatements(stmts);

	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = (Statement*)*it;
		if (s->isPhi()) {
			if (VERBOSE) {
				LOG << "checking " << s << "\n";
			}
			// if we can prove that all the statements in the phi define equal values then we can replace the phi
			// with any one of the values, but there's not much point if they're all calls
			PhiExp *p = (PhiExp*)s->getRight();
			bool hasphi = false;
			StatementVec::iterator it;
			for (it = p->begin(); it != p->end(); it++)
				if (*it && (*it)->isPhi()) {
					hasphi = true;
				}
			if (hasphi) {
				if (VERBOSE)
					LOG << "contains a ref to a phi statement (skipping)\n";
				// Do we need this?
				//continue;
			}
			bool allsame = true;
			it = p->begin();
			assert(it != p->end());
			Statement* s1 = *it;
			Statement* noncall = s1;
			if (it != p->end())
				for (it++; it != p->end(); it++) {
					Statement* s2 = *it;
					if (noncall && s2 && 
						(noncall->isCall()
								|| s2->getNumber() < noncall->getNumber()) && 
						!s2->isCall() && s2 != s)
						noncall = s2;
					Exp *e = new Binary(opEquals, 
								 new RefExp(s->getLeft()->clone(), s1),
								 new RefExp(s->getLeft()->clone(), s2));
					if (!prove(e)) {
						allsame = false; break;
					}
				}
			if (allsame && (noncall == NULL || !noncall->isCall())) {
				s->searchAndReplace(s->getRight(), 
				   new RefExp(s->getLeft(), noncall));
			}
		}
	}
#endif
}
#endif

void UserProc::findSpPreservation() {
	if (VERBOSE)
		LOG << "finding stack pointer preservation for " << getName() << "\n";

	bool stdsp = false;		// FIXME: are these really used?
	// Note: need this non-virtual version most of the time, since nothing proved yet
	int sp = signature->getStackRegister(prog);

	for (int n = 0; n < 2; n++) {
		// may need to do multiple times due to dependencies FIXME: efficiency! Needed any more?

		// Special case for 32-bit stack-based machines (e.g. Pentium).
		// RISC machines generally preserve the stack pointer (so no special case required)
		for (int p = 0; !stdsp && p < 8; p++) {
			if (DEBUG_PROOF)
				LOG << "attempting to prove sp = sp + " << p*4 << " for " << getName() << "\n";
			stdsp = prove(new Binary(opEquals,
				Location::regOf(sp),
				new Binary(opPlus,
					Location::regOf(sp),
					new Const(p * 4))));
		}
#if 0
		// Prove that pc is set to the return value. FIXME: do we use this? Maybe for signature promotion.
		bool stdret = false;
		if (DEBUG_PROOF)
			LOG << "attempting to prove %pc = m[sp]\n";
		stdret = prove(new Binary(opEquals, new Terminal(opPC), Location::memOf(Location::regOf(sp))), sc);
#endif

	}

	if (DEBUG_PROOF) {
		LOG << "proven for " << getName() << ":\n";
		for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); it != proven.end(); it++)
			LOG << *it << "\n";
	}

}

void UserProc::findPreserveds() {
	std::set<Exp*> removes;

	if (VERBOSE)
		LOG << "finding preserveds for " << getName() << "\n";

	if (theReturnStatement == NULL) {
		if (DEBUG_PROOF)
			LOG << "can't find preservations as there is no return statement!\n";
		return;
	}

	// prove preservation for all modifieds in the return statement
	ReturnStatement::iterator mm;
	StatementList& modifieds = theReturnStatement->getModifieds();
	for (mm = modifieds.begin(); mm != modifieds.end(); ++mm) {
		Exp* lhs = ((Assignment*)*mm)->getLeft();
		Exp* equation = new Binary(opEquals, lhs, lhs);
		if (DEBUG_PROOF)
			LOG << "attempting to prove " << equation << " is preserved by " << getName() << "\n";
		if (prove(equation)) {
			removes.insert(equation);	
		}
	}

	if (DEBUG_PROOF) {
		LOG << "### proven for procedure " << getName() << ":\n";
		for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); it != proven.end(); it++)
			LOG << *it << "\n";
		LOG << "### end proven for procedure " << getName() << "\n\n";
	}

	// Remove the preserved locations from the modifieds and the returns
	for (std::set<Exp*, lessExpStar>::iterator pp = proven.begin(); pp != proven.end(); ++pp) {
		Exp* lhs = ((Binary*)*pp)->getSubExp1();
		Exp* rhs = ((Binary*)*pp)->getSubExp2();
		// Has to be of the form loc = loc, not say loc+4, otherwise the bypass logic won't see the add of 4
		if (!(*lhs == *rhs)) continue;
		theReturnStatement->removeModified(lhs);
	}
}

void UserProc::updateReturnTypes()
{
	if (VERBOSE)
		LOG << "### update return types for " << getName() << " ###\n";
	if (theReturnStatement == NULL || !ADHOC_TYPE_ANALYSIS)		// MVE: check
		return;
	ReturnStatement::iterator rr;
	for (rr = theReturnStatement->begin(); rr != theReturnStatement->end(); ++rr) {
		Exp *e = ((Assignment*)*rr)->getLeft();
		Type *ty = e->getType();
		if (ty && !ty->isVoid()) {
			// UGH! Remove when ad hoc TA is removed!
			int n = signature->getNumReturns();
			for (int i=0; i < n; i++) {
				if (*signature->getReturnExp(i) == *e) {
					signature->setReturnType(n, ty->clone());
					break;
				}
			}
		}
	}
}

/*
 * Find the procs the calls point to.
 * To be called after decoding all procs.
 * was in: analyis.cpp
 */
void UserProc::assignProcsToCalls()
{
	std::list<PBB>::iterator it;
	PBB pBB = cfg->getFirstBB(it);
	while (pBB)
	{
    	std::list<RTL*>* rtls = pBB->getRTLs();
    	if (rtls == NULL)
    		continue;
    	for (std::list<RTL*>::iterator it2 = rtls->begin(); it2 != rtls->end(); it2++) {
    		if (!(*it2)->isCall()) continue;
    		CallStatement* call = (CallStatement*)(*it2)->getList().back();
    		if (call->getDestProc() == NULL && !call->isComputed()) {
    			Proc *p = prog->findProc(call->getFixedDest());
    			if (p == NULL) {
    				std::cerr << "Cannot find proc for dest " << call->getFixedDest() << " in call at "
    					<< (*it2)->getAddress() << "\n";
    				assert(p);
    			}
    			call->setDestProc(p);
    		}
    		// call->setSigArguments();		// But BBs not set yet; will get done in initStatements()
    	}

		pBB = cfg->getNextBB(it);
	}
}

/*
 * Perform final simplifications
 * was in: analyis.cpp
 */
void UserProc::finalSimplify()
{
	std::list<PBB>::iterator it;
	PBB pBB = cfg->getFirstBB(it);
	while (pBB)
	{
    	std::list<RTL*>* pRtls = pBB->getRTLs();
    	if (pRtls == NULL)
    		continue;
    	std::list<RTL*>::iterator rit;
    	for (rit = pRtls->begin(); rit != pRtls->end(); rit++) {
    		for (int i=0; i < (*rit)->getNumStmt(); i++) {
    			Statement* rt = (*rit)->elementAt(i);
    			rt->simplifyAddr();
    			// Also simplify everything; in particular, stack offsets are
    			// often negative, so we at least canonicalise [esp + -8] to [esp-8]
    			rt->simplify();
    		}
    	}
		pBB = cfg->getNextBB(it);
	}
}

#if 0			// Now done by dataflow
void UserProc::addNewReturns(int depth) {

	if (signature->isFullSignature())
		return;

	if (VERBOSE)
		LOG << "adding new returns for " << getName() << "\n";

	StatementList stmts;
	getStatements(stmts);

	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = (Statement*)*it;
		LocationSet defs;
		s->getDefinitions(defs);
		LocationSet::iterator ll;
		for (ll = defs.begin(); ll != defs.end(); ll++) {
			Exp *left = *ll;
			bool allZero = true;
			Exp *e = left->clone()->removeSubscripts(allZero);
			if (allZero && signature->findReturn(e) == -1 &&
				getProven(e) == NULL) {
				if (e->getOper() == opLocal) {
					if (VERBOSE)
						LOG << "ignoring local " << e << "\n";
					continue;
				}
				if (e->getOper() == opGlobal) {
					if (VERBOSE)
						LOG << "ignoring global " << e << "\n";
					continue;
				}
				if (e->getOper() == opRegOf && 
					e->getSubExp1()->getOper() == opTemp) {
					if (VERBOSE)
						LOG << "ignoring temp " << e << "\n";
					continue;
				}
				if (e->getOper() == opFlags) {
					if (VERBOSE)
						LOG << "ignoring flags " << e << "\n";
					continue;
				}
				if (e->getMemDepth() != depth) {
					continue;
				}
				if (VERBOSE)
					LOG << "found new return " << e << "\n";
				addReturn(e);
			}
		}
	}
}
#endif

// m[WILD]{-}
static RefExp *memOfWild = new RefExp(
	Location::memOf(new Terminal(opWild)), NULL);
// r[WILD INT]{-}
static RefExp* regOfWild = new RefExp(
	Location::regOf(new Terminal(opWildIntConst)), NULL);

// Search for expressions without explicit definitions (i.e. WILDCARD{-}), which represent parameters (use before define).
// Note: this identifies saved and restored locations as parameters (e.g. ebp in most Pentium procedures).
// Would like to avoid this if possible. Marking the statements involved in saving and restoring, and ignoring these
// here, does not work in all cases, e.g. this contrived procedure:
// f1: a = a xor b
//	c = a         // Keep a copy of a xor b
//	b = a xor b
//	a = a xor b
//	print a       // Print original value of b
//	print c       // Print (original a) xor (original b)
//	a = ...
//	a = a xor b
//	b = a xor b
//	a = a xor b
//	ret

void UserProc::findFinalParameters() {

	parameters.clear();

	if (signature->isForced()) {
		// Copy from signature
		int n = signature->getNumParams();
		for (int i=0; i < n; ++i) {
			Exp* paramLoc = signature->getParamExp(i)->clone();		// E.g. m[r28 + 4]
			LocationSet components;
			LocationSet::iterator cc;
			paramLoc->addUsedLocs(components);
			for (cc = components.begin(); cc != components.end(); ++cc)
				if (*cc != paramLoc)								// Don't subscript outer level
					paramLoc->expSubscriptVar(*cc, NULL);			// E.g. r28 -> r28{-}
			ImplicitAssign* ia = new ImplicitAssign(signature->getParamType(i), paramLoc);
			parameters.append(ia);
			const char* name = signature->getParamName(i);
			Exp* param = Location::param(name, this);
			symbolMap[paramLoc] = param;							// Update name map
		}
		return;
	}
	if (VERBOSE)
		LOG << "finding final parameters for " << getName() << "\n";

	int sp = signature->getStackRegister();
	signature->setNumParams(0);			// Clear any old ideas
	StatementList stmts;
	getStatements(stmts);

	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		// For now, assume that all parameters will be m[]{-} or r[]{-} (Seems pretty reasonable)
		std::list<Exp*> results;
		s->searchAll(memOfWild, results);
		s->searchAll(regOfWild, results);
		while (results.size()) {
			bool allZero;
			Exp *e = results.front()->clone()->removeSubscripts(allZero);
			results.erase(results.begin());		// Remove current (=first) result from list
			if (allZero && signature->findParam(e) == -1
					// ? Often need to transfer from implit to explicit:
				 	// && signature->findImplicitParam(e) == -1
					) {
				if (signature->isStackLocal(prog, e) || e->getOper() == opLocal) {
					if (VERBOSE)
						LOG << "ignoring local " << e << "\n";
					continue;
				}
				if (e->isGlobal()) {
					if (VERBOSE)
						LOG << "ignoring global " << e << "\n";
					continue;
				}
				if (e->getMemDepth() > 1) {
					if (VERBOSE)
						LOG << "ignoring complex " << e << "\n";
					continue;
				}
				if (e->isMemOf() && e->getSubExp1()->isGlobal()) {
					if (VERBOSE)
						LOG << "ignoring m[global] " << e << "\n";
					continue;
				}
				if (e->isMemOf() && e->getSubExp1()->getOper() == opParam) {
					if (VERBOSE)
						LOG << "ignoring m[param] " << e << "\n";
					continue;
				}
				if (e->isMemOf() &&
						e->getSubExp1()->getOper() == opPlus &&
						e->getSubExp1()->getSubExp1()->isGlobal() &&
						e->getSubExp1()->getSubExp2()->isIntConst()) {
					if (VERBOSE)
						LOG << "ignoring m[global + int] " << e << "\n";
					continue;
				}
				if (e->isRegN(sp)) {
					if (VERBOSE)
						LOG << "ignoring stack pointer register\n";
					continue;
				}
				if (e->isMemOf() && e->getSubExp1()->isConst()) {
					if (VERBOSE)
						LOG << "ignoring m[const]\n";
					continue;
				}
#if 0
				// Has to be r[] or m[esp+K]. NOTE: PENTIUM SPECIFIC!
				if ((e->getOper() != opMemOf ||
						e->getSubExp1()->getOper() != opPlus ||
						!(*e->getSubExp1()->getSubExp1() == *Location::regOf(28)) ||
						e->getSubExp1()->getSubExp2()->getOper() != opIntConst) && e->getOper() != opRegOf) {
					if (VERBOSE)
						LOG << "ignoring non pentium " << e << "\n";
					continue;
				}
#endif
				if (VERBOSE)
					LOG << "found new parameter " << e << "\n";
				// Add this parameter to the signature (for now; creates parameter names)
				addParameter(e);
				// Insert it into the parameters StatementList
				insertParameter(e);
			}
		}
	}
}

void UserProc::trimParameters(int depth) {

	if (signature->isForced())
		return;

	if (VERBOSE)
		LOG << "trimming parameters for " << getName() << "\n";

	StatementList stmts;
	getStatements(stmts);

	// find parameters that are referenced (ignore calls to this)
	int nparams = signature->getNumParams();
	// int totparams = nparams + signature->getNumImplicitParams();
int totparams = nparams;
	std::vector<Exp*> params;
	bool referenced[64];
	assert(totparams <= (int)(sizeof(referenced)/sizeof(bool)));
	int i;
	for (i = 0; i < nparams; i++) {
		referenced[i] = false;
		// Push parameters implicitly defined e.g. m[r28{0}+8]{0}, (these are the parameters for the current proc)
		params.push_back(signature->getParamExp(i)->clone()->expSubscriptAllNull());
	}
#if 0
	for (i = 0; i < signature->getNumImplicitParams(); i++) {
		referenced[i + nparams] = false;
		// Same for the implicit parameters
		params.push_back(signature->getImplicitParamExp(i)->clone()->expSubscriptAllNull());
	}
#endif

	std::set<Statement*> excluded;
	StatementList::iterator it;
#if 0		// MVE: CHECK THIS!!!!!!!!!!!!!!!!!!!!!!
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		if (s->isCall() && ((CallStatement*)s)->getDestProc() == this) {
			// A self-recursive call
			CallStatement *call = (CallStatement*)s;
			for (int i = 0; i < signature->getNumImplicitParams(); i++) {
				Exp *e = call->getImplicitArgumentExp(i);
				if (e->isSubscript()) {
					Statement *ref = ((RefExp*)e)->getDef();
					if (ref && !ref->isImplicit())
						excluded.insert(ref);
				}
			}
		}
	}
#endif
	
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		if (!s->isCall() || ((CallStatement*)s)->getDestProc() != this) {
			for (int i = 0; i < totparams; i++) {
				Exp *p, *pe;
				//if (i < nparams) {
					p = Location::param(signature->getParamName(i), this);
					pe = signature->getParamExp(i);
				//} else {
				//	p = Location::param(signature->getImplicitParamName( i - nparams), this);
				//	pe = signature->getImplicitParamExp(i - nparams);
				//}
				if (!referenced[i] && excluded.find(s) == excluded.end() && 
						// Search for the named parameter (e.g. param1), and just in case, also for the expression
						// (e.g. r8{0})
						(s->usesExp(p) || s->usesExp(params[i]))) {
					referenced[i] = true;
					if (DEBUG_UNUSED) {
						LOG << "parameter " << p << " used by statement " << s->getNumber() << " : " << s->getKind() <<
							"\n";
					}
				}
				if (!referenced[i] && excluded.find(s) == excluded.end() &&
						s->isPhi() && *((PhiAssign*)s)->getLeft() == *pe) {
					if (DEBUG_UNUSED)
						LOG << "searching " << s << " for uses of " << params[i] << "\n";
					PhiAssign *pa = (PhiAssign*)s;
					PhiAssign::iterator it1;
					for (it1 = pa->begin(); it1 != pa->end(); it1++)
						if (it1->def == NULL) {
							referenced[i] = true;
							if (DEBUG_UNUSED)
								LOG << "parameter " << p << " used by phi statement " << s->getNumber() << "\n";
							break;
						}
				}
				// delete p;
			}
		}
	}

	for (i = 0; i < totparams; i++) {
		if (!referenced[i] && (depth == -1 || params[i]->getMemDepth() == depth)) {
			bool allZero;
			Exp *e = params[i]->removeSubscripts(allZero);
			if (VERBOSE) 
				LOG << "removing unused parameter " << e << "\n";
			removeParameter(e);
		}
	}
}

void UserProc::removeReturn(Exp *e) {
	if (theReturnStatement)
		theReturnStatement->removeReturn(e);
}

void Proc::removeParameter(Exp *e) {
	int n = signature->findParam(e);
	if (n != -1) {
		signature->removeParameter(n);
		for (std::set<CallStatement*>::iterator it = callerSet.begin(); it != callerSet.end(); it++) {
			if (DEBUG_UNUSED)
				LOG << "removing argument " << e << " in pos " << n << " from " << *it << "\n";
			(*it)->removeArgument(n);
		}
	}
#if 0
	n = signature->findImplicitParam(e);
	if (n != -1) {
		signature->removeImplicitParameter(n);
		for (std::set<CallStatement*>::iterator it = callerSet.begin(); it != callerSet.end(); it++) {
			// Don't remove an implicit argument if it is also a return of this call.
			// Implicit arguments are needed for each return in fixCallBypass
			if (DEBUG_UNUSED)
				LOG << "removing implicit argument " << e << " in pos " << n << " from " << *it << "\n";
			(*it)->removeImplicitArgument(n);
		}
	}
#endif
}

void Proc::removeReturn(Exp *e)
{
	signature->removeReturn(e);
}

#if 0		// This is done by dataflow now
void Proc::addReturn(Exp *e)
{
	signature->addReturn(e);
}

void UserProc::addReturn(Exp *e)
{
	Exp *e1 = e->clone();
	if (e1->getOper() == opMemOf) {
		e1->setSubExp1(e1->getSubExp1()->expSubscriptAllNull(/*cfg*/));
	}
	if (theReturnStatement)
		theReturnStatement->addReturn(e1);
	Proc::addReturn(e);
}
#endif

// Add the parameter to the signature. Used to say "and all known callers"; this will be handled by the recursion
// manager now
void UserProc::addParameter(Exp *e) {
	// In case it's already an implicit argument:
	removeParameter(e);

	// for (std::set<CallStatement*>::iterator it = callerSet.begin(); it != callerSet.end(); it++)
	//	(*it)->addArgument(e, this);
	signature->addParameter(e);
}

#if 0			// No, we are careful to sort parameters and returns now
void Proc::sortParameters() {
	// yes, this is a bubble sort
	for (int i = 0; i < signature->getNumParams() - 1; i++) {
		for (int j = 0; j < signature->getNumParams() - 1; j++) {
			for (int n = 0; n < signature->getNumParams() - 1; n++) {
				bool swapem = false;
				Exp *e = signature->getParamExp(n);
				Exp *f = signature->getParamExp(n + 1);
				if (e == NULL || f == NULL)
					break;
				if (e->getOper() == opMemOf && f->getOper() != opMemOf)
					swapem = true;
				else if (e->getOper() == opMemOf && f->getOper() == opMemOf) {
					if (e->getSubExp1()->getOper() == opPlus && f->getSubExp1()->getOper() == opPlus)
						if (e->getSubExp1()->getSubExp2()->isIntConst() && f->getSubExp1()->getSubExp2()->isIntConst())
							if (((Const*)e->getSubExp1()->getSubExp2())->getInt() >
									((Const*)f->getSubExp1()->getSubExp2())->getInt())
								swapem = true;
				}
				if (swapem) {
					const char *tmpname = strdup(signature->getParamName(n));
					Type *tmpty = signature->getParamType(n);
					signature->setParamName(n, signature->getParamName(n + 1));
					signature->setParamType(n, signature->getParamType(n + 1));
					signature->setParamExp(n, f);
					signature->setParamName(n + 1, tmpname);
					signature->setParamType(n + 1, tmpty);
					signature->setParamExp(n + 1, e);
					for (std::set<CallStatement*>::iterator it = callerSet.begin(); it != callerSet.end(); it++) {
						e = (*it)->getArgumentExp(n);
						(*it)->setArgumentExp(n, (*it)->getArgumentExp(n + 1));
						(*it)->setArgumentExp(n + 1, e);
					}
				}
			}
		}
	}
}
#endif

void UserProc::processFloatConstants()
{
	StatementList stmts;
	getStatements(stmts);

	Exp *match = new Ternary(opFsize,
						new Terminal(opWild), 
						new Terminal(opWild), 
						Location::memOf(new Terminal(opWild)));
	
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement *s = *it;

		std::list<Exp*> results;
		s->searchAll(match, results);
		for (std::list<Exp*>::iterator it1 = results.begin(); it1 != results.end(); it1++) {
			Ternary *fsize = (Ternary*) *it1;
			if (fsize->getSubExp3()->getOper() == opMemOf &&
					fsize->getSubExp3()->getSubExp1()->getOper() == opIntConst) {
				Exp *memof = fsize->getSubExp3();
				ADDRESS u = ((Const*)memof->getSubExp1())->getInt();
				bool ok;
				double d = prog->getFloatConstant(u, ok);
				if (ok) {
					LOG << "replacing " << memof << " with " << d << " in " << fsize << "\n";
					fsize->setSubExp3(new Const(d));
				}
			}
		}
		s->simplify();
	}
}

void UserProc::replaceExpressionsWithGlobals() {
	if (DFA_TYPE_ANALYSIS) {
		if (VERBOSE)
			LOG << "not replacing expressions with globals because -Td in force\n";
		return;
	}
	StatementList stmts;
	getStatements(stmts);
	int sp = signature->getStackRegister(prog);

	if (VERBOSE)
		LOG << "replacing expressions with globals\n";

	// start with calls because that's where we have the most types
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		if ((*it)->isCall()) {
			CallStatement *call = (CallStatement*)*it;
			// This loop seems to only look for the address of globals in a parameter (?!)
			for (int i = 0; i < call->getNumArguments(); i++) {
				Type *ty = call->getArgumentType(i);
				Exp *e = call->getArgumentExp(i);
				// The below assumes that the address of a global is an integer constant
				if (ty && ty->resolvesToPointer() && e->getOper() == opIntConst) {
					Type *pty = ty->asPointer()->getPointsTo();
					if (pty->resolvesToArray() && pty->asArray()->isUnbounded()) {
						ArrayType *a = (ArrayType*)pty->asArray()->clone();
						pty = a;
						a->setLength(1024);		// just something arbitrary
						if (i+1 < call->getNumArguments()) {
							Type *nt = call->getArgumentType(i+1);
							if (nt->isNamed())
								nt = ((NamedType*)nt)->resolvesTo();
							if (nt->isInteger() && call->getArgumentExp(i+1)->isIntConst())
								a->setLength(((Const*)call->getArgumentExp(i+1))->getInt());
						}
					}
					ADDRESS u = ((Const*)e)->getInt();
					prog->globalUsed(u);
					const char *gloName = prog->getGlobalName(u);
					if (gloName) {
						ADDRESS r = u - prog->getGlobalAddr((char*)gloName);
						Exp *ne;
						if (r) {
							Location *g = Location::global(strdup(gloName), this);
							// &global + r
							ne = new Binary(opPlus,
								new Unary(opAddrOf, g),
								new Const(r));
						} else {
							prog->setGlobalType((char*)gloName, pty);
							Location *g = Location::global(strdup(gloName), this);
							// &global
							ne = new Unary(opAddrOf, g);
						}
						call->setArgumentExp(i, ne);
						if (VERBOSE)
							LOG << "replacing argument " << e << " with " << ne << " in " << call << "\n";
					}
				}
			}
		}
	}


	// replace expressions with globals
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;

		// (a) Definitions
		LocationSet defs;
		s->getDefinitions(defs);
		LocationSet::iterator rr;
		for (rr = defs.begin(); rr != defs.end(); rr++) {
			if ((*rr)->getOper() == opMemOf && (*rr)->getSubExp1()->getOper() == opIntConst) {
				Exp *memof = *rr;
				ADDRESS u = ((Const*)memof->getSubExp1())->getInt();
				prog->globalUsed(u);
				const char *gloName = prog->getGlobalName(u);
				if (gloName) {
					ADDRESS r = u - prog->getGlobalAddr((char*)gloName);
					Exp *ne;
					if (r) {
						Location *g = Location::global(strdup(gloName), this);
						ne = Location::memOf(
							new Binary(opPlus,
								new Unary(opAddrOf, g),
								new Const(r)), this);
					} else {
						Type *ty = prog->getGlobalType((char*)gloName);
						if (s->isAssign() && ((Assign*)s)->getType()) {
							int bits = ((Assign*)s)->getType()->getSize();
							if (ty == NULL || ty->getSize() == 0)
								prog->setGlobalType((char*)gloName, new IntegerType(bits));
						}
						ty = prog->getGlobalType((char*)gloName);
						Location *g = Location::global(strdup(gloName), this);
						if (ty && ty->isArray()) 
							ne = new Binary(opArrayIndex, g, new Const(0));
						else 
							ne = g;
					}
					s->searchAndReplace(memof->clone(), ne);
				}
			}
		}

		// (b) Uses
		LocationSet refs;
		s->addUsedLocs(refs);
		for (rr = refs.begin(); rr != refs.end(); rr++) {
			if (((Exp*)*rr)->isSubscript()) {
				Statement *ref = ((RefExp*)*rr)->getDef();
				Exp *r1 = (*rr)->getSubExp1();
				if (symbolMap.find(r1) != symbolMap.end())
					continue;					// Ignore locals, etc
				// look for m[exp + K]{0}, replace it with m[exp * 1 + K]{0} in the hope that it will get picked 
				// up as a global array.
				if (ref == NULL && r1->getOper() == opMemOf && r1->getSubExp1()->getOper() == opPlus &&
						r1->getSubExp1()->getSubExp2()->getOper() == opIntConst) {
					r1->getSubExp1()->setSubExp1(new Binary(opMult, r1->getSubExp1()->getSubExp1(), new Const(1)));
				}
				// Is it m[CONSTANT]{-}
				if (ref == NULL && r1->getOper() == opMemOf && r1->getSubExp1()->getOper() == opIntConst) {
					Exp *memof = r1;
					ADDRESS u = ((Const*)memof->getSubExp1())->getInt();
					prog->globalUsed(u);
					const char *gloName = prog->getGlobalName(u);
					if (gloName) {
						ADDRESS r = u - prog->getGlobalAddr((char*)gloName);
						Exp *ne;
						if (r) {
							Unary *g = Location::global(strdup(gloName), this);
							ne = Location::memOf(
								new Binary(opPlus,
									new Unary(opAddrOf, g),
									new Const(r)), this);
						} else {
							Type *ty = prog->getGlobalType((char*)gloName);
							Unary *g = Location::global(strdup(gloName), this);
							if (ty && ty->isArray() && ty->getSize() > 0) 
								ne = new Binary(opArrayIndex,
									g,
									new Const(0));
							else 
								ne = g;
						}
						s->searchAndReplace(memof->clone(), ne);
					}
				// look for m[(blah * K1 + K2)]
				} else if (ref == NULL && r1->getOper() == opMemOf && r1->getSubExp1()->getOper() == opPlus &&
						r1->getSubExp1()->getSubExp1()->getOper() == opMult &&
						r1->getSubExp1()->getSubExp1()->getSubExp2() ->getOper() == opIntConst &&
						r1->getSubExp1()->getSubExp2()->getOper() == opIntConst) {
					Exp* blah = r1->getSubExp1()->getSubExp1()->getSubExp1();
					if (blah->isSubscript())
						blah = ((RefExp*)blah)->getSubExp1();
					if (blah->isRegN(sp))
						continue;					// sp can't base an array
					Exp *memof = r1;
					// K1 is the stride
					unsigned stride = ((Const*)memof->getSubExp1()->getSubExp1()->getSubExp2())->getInt();
					// u is K2
					ADDRESS u = ((Const*)memof->getSubExp1()->getSubExp2())->getInt();
					if (VERBOSE)
						LOG << "detected array ref with stride " << stride << "\n";
					prog->globalUsed(u);
					const char *gloName = prog->getGlobalName(u);
					if (gloName) {
						ADDRESS r = u - prog->getGlobalAddr((char*)gloName);
						Exp *ne = NULL;
						if (r) {
							// TOO HARD
						} else {
							Type *ty = prog->getGlobalType((char*)gloName);
							Location *g = Location::global(strdup(gloName), this);
							if (ty == NULL || ty->getSize() == 0) {
								LOG << "setting type of global to array\n";
								ty = new ArrayType(new IntegerType(stride*8),1);
								prog->setGlobalType((char*)gloName, ty);
							}

							if (ty && VERBOSE)
								LOG << "got type: " << ty->getCtype() << "\n";

							if (ty && ty->isArray() && ty->asArray()->getBaseType()->getSize() != stride*8) {
								if (VERBOSE)
									LOG << "forcing array base type size to stride\n";
								ty->asArray()->setLength(ty->asArray()->getLength() *
									ty->asArray()->getBaseType()->getSize() / (stride * 8));
								ty->asArray()->setBaseType(new IntegerType(stride*8));
								prog->setGlobalType((char*)gloName, ty);
							}

							if (ty && VERBOSE)
								LOG << "got type: " << ty->getCtype() << "\n";

							if (ty && ty->isArray() && ty->asArray()->getBaseType()->getSize() == stride*8) {
								LOG << "setting new exp to array ref\n";
								ne = new Binary(opArrayIndex,
									g, 
									memof->getSubExp1()->getSubExp1()->getSubExp1() ->clone());
								LOG << "set to " << ne << "\n";
							}
							/* else 
								ne = Location::memOf(new Binary(opPlus, 
										new Unary(opAddrOf, g), 
										memof->getSubExp1()->getSubExp1()->clone()
										)); */
						}
						if (ne)
							s->searchAndReplace(memof->clone(), ne);
					}
				}
			}
		}

		s->simplify();
	}
}

void UserProc::replaceExpressionsWithSymbols() {
#if 0			// Done in the back ends now
	StatementList stmts;
	getStatements(stmts);

	// replace expressions in regular statements with symbols
	// Note: O(MN) where M is the number of symbols, and N is the number of statements
	// FIXME: Should use the map properly
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		for (std::map<Exp*, Exp*,lessExpStar>::iterator it1 = symbolMap.begin(); it1 != symbolMap.end(); it1++) {
			bool ch = s->searchAndReplace((*it1).first, (*it1).second);
			if (ch && VERBOSE) {
				LOG << "std stmt: replace " << (*it1).first << " with " << (*it1).second << " result " << s << "\n";
			}
		}
	}
#endif
}

void UserProc::replaceExpressionsWithParameters(int depth) {
	StatementList stmts;
	getStatements(stmts);

	if (VERBOSE)
		LOG << "replacing expressions with parameters at depth " << depth << "\n";

	bool found = false;
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		if ((*it)->isCall()) {
			CallStatement *call = (CallStatement*)*it;
			for (int i = 0; i < call->getNumArguments(); i++) {
				Type *ty = call->getArgumentType(i);
				Exp *e = call->getArgumentExp(i);
				if (ty && ty->resolvesToPointer() && e->getOper() != opAddrOf && e->getMemDepth() == 0) {
					// Check for an expression representing the address of a local variable. NOTE: machine dependent
					if (signature->isAddrOfStackLocal(prog, e)) {
						// don't do locals here!
						continue;
					}

					Exp* ne;
					if (DFA_TYPE_ANALYSIS)
						ne = e;		// No a[m[e]]
					else {
						// Do the a[m[e]] hack
						Location *pe = Location::memOf(e, this);
						ne = new Unary(opAddrOf, pe);
					}
					if (VERBOSE)
						LOG << "replacing argument " << e << " with " << ne << " in " << call << "\n";
					call->setArgumentExp(i, ne);
					found = true;
				}
			}
		}
	}
	if (found) {
		// Must redo all the subscripting, just for the a[m[...]] thing!
		if (status != PROC_INCYCLE || depth == 0) {
			for (int d=0; d <= depth; d++)
				doRenameBlockVars(d, true);
		} else
			doRenameBlockVars(0, true);
	}

	// replace expressions in regular statements with parameters
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		for (unsigned i = 0; i < signature->getNumParams(); i++) {
			if (depth < 0 || signature->getParamExp(i)->getMemDepth() == depth) {
				Exp *r = signature->getParamExp(i)->clone();
				r = r->expSubscriptAllNull();
				// Remove the outer {0}, for where it appears on the LHS, and because we want to have param1{0}
				assert(r->isSubscript());	// There should always be one
				// if (r->getOper() == opSubscript)
				r = r->getSubExp1();
				Location* replace = Location::param( strdup((char*)signature->getParamName(i)), this);
				Exp *n;
				if (s->search(r, n)) {
					if (VERBOSE)
						LOG << "replacing " << r << " with " << replace << " in " << s << "\n";
#if 0
					s->searchAndReplace(r, replace);
					if (VERBOSE)
						LOG << "after: " << s << "\n";
#else
					symbolMap[r] = replace;			// Add to symbol map
					// Note: don't add to locals, since otherwise the back end will declare it twice
#endif
				}
			}
		}
	}
}

// Return an expression that is equivilent to e in terms of symbols. Creates new symbols as needed.
Exp *UserProc::getSymbolExp(Exp *le, Type *ty, bool lastPass) {
#if 0	// See below
	// Expression r[sp] (build just once per call)
	Exp* regSP = Location::regOf(signature->getStackRegister(prog));
	// The implicit definition for r[sp], if any
	Statement* defSP = cfg->findTheImplicitAssign(regSP);
	// The expression r[sp]{0} or r[sp]{-} as appropriate
	RefExp* refSP0 = new RefExp(regSP, defSP);
#endif

	Exp *e = NULL;
	if (symbolMap.find(le) == symbolMap.end()) {
		//if (le->getOper() == opMemOf && signature->isOpCompatStackLocal(le->getSubExp1()->getOper()) &&
		//		*le->getSubExp1()->getSubExp1() == *refSP0 &&
		//		le->getSubExp1()->getSubExp2()->isIntConst()) {	// }
#if 0	// This doesn't work any more anyway, because of an ordering issue
		if (signature->isStackLocal(prog, le)) {
			int le_n = ((Const*)le->getSubExp1()->getSubExp2())->getInt();
			// now test all the locals to see if this expression is an alias to one of them (for example,
			// a member of a compound typed local)
			// NOTE: Not efficient!
			for (SymbolMapType::iterator it = symbolMap.begin(); it != symbolMap.end(); it++) {
				Exp *base = it->first;
				assert(base);
				Location *local = (Location*)it->second;
				assert((local->isLocal() || local->isParam()) && local->getSubExp1()->getOper() == opStrConst);
				std::string name = ((Const*)local->getSubExp1())->getStr();
				std::map<std::string, Type*>::iterator ff = locals.find(name);
				if (ff == locals.end())
					continue;			// This symbol is not a local (e.g. a parameter); ignore
				Type *ty = ff->second;
				assert(ty);
				int size = ty->getSize() / 8;	 // getSize() returns bits!
				if (base->getOper() == opMemOf && signature->isOpCompatStackLocal(base->getSubExp1()->getOper()) &&
						*base->getSubExp1()->getSubExp1() == *refSP0 &&
						base->getSubExp1()->getSubExp2()->getOper() == opIntConst) {
					int base_n = ((Const*)base->getSubExp1()->getSubExp2()) ->getInt();
					if (le->getSubExp1()->getOper() == opPlus)
						// We have m[sp{0} + base_n]. It is probably not what we want...
						base_n = 0 - base_n;			// Treat m[sp{0} + 20] as m[sp{0} - -20]
					if (le_n <= base_n && le_n > base_n-size) {
						if (VERBOSE)
							LOG << "found alias to " << name.c_str() << ": " << le << "\n";
						int n = base_n - le_n;
						return new TypedExp(ty,
							Location::memOf(
								new Binary(opPlus, 
									new Unary(opAddrOf, 
										local->clone()),
									new Const(n)), this));
#if 0
						if (ty->resolvesToCompound()) {
							CompoundType *compound = ty->asCompound();
							return new Binary(opMemberAccess, local->clone(), 
							   new Const((char*)compound->getNameAtOffset(
							   (base_n - le_n)*8)));
						} else
							assert(false);
#endif
					}
				}
			}
		}
#endif
		
		if (ty == NULL && lastPass)
			ty = new IntegerType();
		else
			ty = new VoidType();			// HACK MVE

		if (ty) {
			// the default of just assigning an int type is bad..  if the locals is not an int then assigning it this
			// type early results in aliases to this local not being recognised 
			e = newLocal(ty->clone());
			symbolMap[le->clone()] = e;
			//e->clone();				// ? Supposed to be e = e->clone()?
		}
	} else {
		e = symbolMap[le]->clone();
		if (e->getOper() == opLocal && e->getSubExp1()->getOper() == opStrConst) {
			std::string name = ((Const*)e->getSubExp1())->getStr();
			Type *nty = ty;
			Type *ty = locals[name];
			assert(ty);
			if (nty && !(*ty == *nty) && nty->getSize() >= ty->getSize()) {
				// FIXME: should this be a type meeting?
				if (DEBUG_TA)
					LOG << "getSymbolExp: updating type of " << name.c_str() << " to " << nty->getCtype() << "\n";
				ty = nty;
				locals[name] = ty;
			}
			if (ty->resolvesToCompound()) {
				CompoundType *compound = ty->asCompound();
				if (VERBOSE)
					LOG << "found reference to first member of compound " << name.c_str() << ": " << le << "\n";
				char* nam = (char*)compound->getName(0);
				if (nam == NULL) nam = "??";
				return new TypedExp(ty, new Binary(opMemberAccess, e, new Const(nam)));
			}
		}
	}
	return e;
}

void UserProc::mapExpressionsToLocals(bool lastPass) {
	StatementList stmts;
	getStatements(stmts);

	if (VERBOSE) {
		LOG << "mapping expressions to locals for " << getName();
		if (lastPass)
			LOG << " last pass";
		LOG << "\n";
	}

	int sp = signature->getStackRegister(prog);
	if (getProven(Location::regOf(sp)) == NULL) {
		if (VERBOSE)
			LOG << "can't map locals since sp unproven\n";
		return;		// can't replace if nothing proven about sp
	}

	// start with calls because that's where we have the most types
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		if ((*it)->isCall()) {
			CallStatement *call = (CallStatement*)*it;
			for (int i = 0; i < call->getNumArguments(); i++) {
				Type *ty = call->getArgumentType(i);
				Exp *e = call->getArgumentExp(i);
				// If a pointer type and e is of the form m[sp{0} - K]:
				if (ty && ty->resolvesToPointer() && signature->isAddrOfStackLocal(prog, e)) {
					Exp *olde = e->clone();
					Type *pty = ty->asPointer()->getPointsTo();
					if (pty->resolvesToArray() && pty->asArray()->isUnbounded()) {
						ArrayType *a = (ArrayType*)pty->asArray()->clone();
						pty = a;
						a->setLength(1024);		// just something arbitrary
						if (i+1 < call->getNumArguments()) {
							Type *nt = call->getArgumentType(i+1);
							if (nt->isNamed())
								nt = ((NamedType*)nt)->resolvesTo();
							if (nt->isInteger() && call->getArgumentExp(i+1)->isIntConst())
								a->setLength(((Const*)call->getArgumentExp(i+1)) ->getInt());
						}
					}
					e = getSymbolExp(Location::memOf(e->clone(), this), pty);
					if (e) {
						Exp *ne = new Unary(opAddrOf, e);
						if (VERBOSE)
							LOG << "replacing argument " << olde << " with " << ne << " in " << call << "\n";
						call->setArgumentExp(i, ne);
					}
				}
			}
		}
	}

	// normalise sp usage (turn WILD + sp{0} into sp{0} + WILD)
	Exp *nn = new Binary(opPlus, new Terminal(opWild), new RefExp(Location::regOf(sp), NULL));
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		std::list<Exp*> results;
		s->searchAll(nn, results);
		for (std::list<Exp*>::iterator it1 = results.begin(); it1 != results.end(); it1++) {
			Exp *wild = (*it1)->getSubExp1();
			(*it1)->setSubExp1((*it1)->getSubExp2());
			(*it1)->setSubExp2(wild);
		}
	}

	// look for array locals
	// l = m[(sp{0} + WILD1) - K2]
	Exp *l = Location::memOf(new Binary(opMinus, 
				new Binary(opPlus,
					new RefExp(Location::regOf(sp), NULL),
					new Terminal(opWild)),
				new Terminal(opWildIntConst)));
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		std::list<Exp*> results;
		s->searchAll(l, results);
		for (std::list<Exp*>::iterator it1 = results.begin(); it1 != results.end(); it1++) {
			Exp *result = *it1;
			// arr = m[sp{0} - K2]
			Location *arr = Location::memOf(
				new Binary(opMinus, 
					new RefExp(Location::regOf(sp), NULL),
					result->getSubExp1()->getSubExp2()->clone()));
			int n = ((Const*)result->getSubExp1()->getSubExp2())->getInt();
			arr->setProc(this);
			Type *base = new IntegerType();
			if (s->isAssign() && ((Assign*)s)->getLeft() == result) {
				Type* at = ((Assign*)s)->getType();
				if(at && at->getSize() != 0)
					base = ((Assign*)s)->getType()->clone();
			}
			arr->setType(new ArrayType(base, n / (base->getSize() / 8)));
			if (VERBOSE)
				LOG << "found a local array using " << n << " bytes\n";
			Exp *replace = Location::memOf(
				new Binary(opPlus,
					new Unary(opAddrOf, arr),
					result->getSubExp1()->getSubExp1()->getSubExp2()->clone()), this);
			if (VERBOSE)
				LOG << "replacing " << result << " with " << replace << " in " << s << "\n";
			s->searchAndReplace(result->clone(), replace);
		}
	}

	// Stack offsets for local variables could be negative (most machines), positive (PA/RISC), or both (SPARC)
	if (signature->isLocalOffsetNegative())
		searchRegularLocals(opMinus, lastPass, sp, stmts);
	if (signature->isLocalOffsetPositive())
		searchRegularLocals(opPlus, lastPass, sp, stmts);
	// Ugh - m[sp] is a special case: neither positive or negative.  SPARC uses this to save %i0
	if (signature->isLocalOffsetPositive() && signature->isLocalOffsetNegative())
		searchRegularLocals(opWild, lastPass, sp, stmts);

}

void UserProc::searchRegularLocals(OPER minusOrPlus, bool lastPass, int sp, StatementList& stmts) {
	// replace expressions in regular statements with locals
	Location* l;
	if (minusOrPlus == opWild)
		// l = m[sp{0}]
		l = Location::memOf(
			new RefExp(Location::regOf(sp), NULL));
	else
		// l = m[sp{0} +/- K]
		l = Location::memOf(
			new Binary(minusOrPlus, 
				new RefExp(Location::regOf(sp), NULL),
				new Terminal(opWildIntConst)));
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		std::list<Exp*> results;
		s->searchAll(l, results);
		for (std::list<Exp*>::iterator it1 = results.begin(); it1 != results.end(); it1++) {
			Exp *result = *it1;
			Type *ty = result->getType();
			if (s->isAssign() && ((Assign*)s)->getLeft() == result)
				ty = ((Assign*)s)->getType();
			Exp *e = getSymbolExp(result, ty, lastPass);
			if (e) {
				Exp* search = result->clone();
				if (VERBOSE)
					LOG << "mapping " << search << " to " << e << " in " << s << "\n";
				//s->searchAndReplace(search, e);
			}
		}
		//s->simplify();
	}
}

bool UserProc::nameStackLocations() {
	Exp *match = signature->getStackWildcard();
	if (match == NULL) return false;

	bool found = false;
	StatementList stmts;
	getStatements(stmts);
	// create a symbol for every memory reference
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		Exp *memref; 
		if (s->search(match, memref)) {
			if (symbolMap.find(memref) == symbolMap.end()) {
				if (VERBOSE)
					LOG << "stack location found: " << memref << "\n";
				symbolMap[memref->clone()] = newLocal(new IntegerType());
			}
			assert(symbolMap.find(memref) != symbolMap.end());
			Location* locl = (Location*)symbolMap[memref]->getSubExp1();
			std::string name = ((Const*)locl)->getStr();
			if (memref->getType() != NULL)
				locals[name] = memref->getType();
#if 0		// No: type analysis instead of guessing types
			locals[name] = s->updateType(memref, locals[name]);
			if (VERBOSE)
				LOG << "updating type of " << name.c_str() << " to " << locals[name]->getCtype() << "\n";
#endif
			found = true;
		}
	}
	delete match;
	return found;
}

// Deprecated. Eventually replace with mapRegistersToLocals()
bool UserProc::nameRegisters() {
	static Exp *regOfWild = Location::regOf(new Terminal(opWild));
	bool found = false;
	int sp = signature->getStackRegister(prog);

	StatementList stmts;
	getStatements(stmts);
	// create a symbol for every register
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		Exp *reg; 
		std::list<Exp*> li;
		std::list<Exp*>::iterator ll;
		s->searchAll(regOfWild, li);
		for (ll = li.begin(); ll != li.end(); ++ll) {
			reg = *ll;
			if (reg->isRegN(sp))
				continue;				// Never name the stack pointer
			if (symbolMap.find(reg) != symbolMap.end())
				continue;
			if (VERBOSE)
				LOG << "register found: " << reg << "\n";
			Type *ty = reg->getType();
			if (ty == NULL)
				ty = new IntegerType();
			symbolMap[reg->clone()] = newLocal(ty);
			assert(symbolMap.find(reg) != symbolMap.end());
			Location* locl = (Location*)symbolMap[reg]->getSubExp1();
			std::string name = ((Const*)locl)->getStr();
			if (reg->getType() != NULL)
				locals[name] = reg->getType();
			else {
				//locals[name] = s->updateType(reg, locals[name]);
				// For now; should only affect ad hoc type analysis:
				locals[name] = new IntegerType();
				if (VERBOSE)
					LOG << "updating type of named register " << name.c_str() << " to " << locals[name]->getCtype() <<
						"\n";
			}
			found = true;
		}
	}

	return found;
}

// Core of the register replacing logic
void UserProc::regReplaceList(std::list<Exp**>& li) {
	std::list<Exp**>::iterator it;
	for (it = li.begin(); it != li.end(); it++) {
		Exp* reg = ((RefExp*)**it)->getSubExp1();
		Statement* def = ((RefExp*)**it)->getDef();
		Type *ty = def->getTypeFor(reg);
		if (symbolMap.find(reg) == symbolMap.end()) {
			symbolMap[reg] = newLocal(ty);
			Location* locl = (Location*)symbolMap[reg]->getSubExp1();
			std::string name = ((Const*)locl)->getStr();
			locals[name] = ty;
			if (VERBOSE)
				LOG << "replacing all " << reg << " with " << name << ", type " << ty->getCtype() << "\n";
		}
		// Now replace it in the IR
		//**it = symbolMap[reg];
	}
}

void UserProc::mapRegistersToLocals() {
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++)
		// Bounces back most times to UserProc::regReplaceList (above)
		(*it)->regReplace(this);
}

bool UserProc::removeNullStatements() {
	bool change = false;
	StatementList stmts;
	getStatements(stmts);
	// remove null code
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		if (s->isNullStatement()) {
			// A statement of the form x := x
			if (VERBOSE) {
				LOG << "removing null statement: " << s->getNumber() <<
				" " << s << "\n";
			}
			removeStatement(s);
#if 0
			// remove from reach sets
			StatementSet &reachout = s->getBB()->getReachOut();
			if (reachout.remove(s))
				cfg->computeReaches();		// Highly sus: do all or none!
				recalcDataflow();
#endif
			change = true;
		}
	}
	return change;
}

bool UserProc::processConstants() {
	if (DFA_TYPE_ANALYSIS) {
		if (VERBOSE)
			LOG << "not processing constants since -Td in force\n";
		return false;
	}
	if (VERBOSE)
		LOG << "process constants for " << getName() << "\n";
	StatementList stmts;
	getStatements(stmts);
	// process any constants in the statement
	StatementList::iterator it;
	bool paramsAdded = false;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		paramsAdded |= s->processConstants(prog);
	}
	return paramsAdded;
}

// Propagate statements, but don't remove
// Respect the memory depth (don't propagate FROM statements that have components of higher memory depth than memDepth)
// Return true if an indirect call is converted to direct
bool UserProc::propagateStatements(int memDepth) {
	if (VERBOSE)
		LOG << "--- begin propagating statements at depth " << memDepth << " ---\n";
	StatementList stmts;
	getStatements(stmts);
	// propagate any statements that can be
	StatementList::iterator it;
	bool convertedIndirect = false;
	// First pass: count the number of times each assignment LHS would be propagated somewhere
	std::map<Exp*, int, lessExpStar> destCounts;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		LocationSet exps;
		s->addUsedLocs(exps, true);		// True to also add uses from collectors
		LocationSet::iterator ll;
		for (ll = exps.begin(); ll != exps.end(); ll++) {
			Exp* e = *ll;
			if (!s->canPropagateToExp(e, memDepth)) continue;
			destCounts[e]++;				// Count propagatable expression e
		}
	}
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		if (s->isPhi()) continue;
		convertedIndirect |= s->propagateTo(memDepth, &destCounts);
	}
	simplify();
	propagateToCollector(memDepth);
	if (VERBOSE)
		LOG << "=== end propagating statements at depth " << memDepth << " ===\n";
	return convertedIndirect;
}

Statement *UserProc::getStmtAtLex(unsigned int begin, unsigned int end)
{
	StatementList stmts;
	getStatements(stmts);
	
	unsigned int lowest = begin;
	Statement *loweststmt = NULL;
	for (StatementList::iterator it = stmts.begin(); it != stmts.end(); it++)
		if (begin >= (*it)->getLexBegin() && begin <= lowest && begin <= (*it)->getLexEnd() &&
				(end == (unsigned)-1 || end < (*it)->getLexEnd())) {
			loweststmt = (*it);
			lowest = (*it)->getLexBegin();
		}
	return loweststmt;
}

void UserProc::promoteSignature() {
	signature = signature->promote(this);
}

Exp* UserProc::newLocal(Type* ty) {
	std::ostringstream os;
	os << "local" << nextLocal++;
	std::string name = os.str();
	locals[name] = ty;
	if (ty == NULL) {
		std::cerr << "null type passed to newLocal\n";
		assert(false);
	}
	if (VERBOSE)
		LOG << "assigning type " << ty->getCtype() << " to new " << name.c_str() << "\n";
	return Location::local(strdup(name.c_str()), this);
}

Type *UserProc::getLocalType(const char *nam)
{
	if (locals.find(nam) == locals.end())
		return NULL;
	return locals[nam];
}

void UserProc::setLocalType(const char *nam, Type *ty)
{
	locals[nam] = ty;
	if (VERBOSE)
		LOG << "setLocalType: updating type of " << nam << " to " << ty->getCtype() << "\n";
}

void UserProc::setExpSymbol(const char *nam, Exp *e, Type* ty)
{
	// NOTE: does not update symbols[]
	TypedExp *te = new TypedExp(ty, Location::local(strdup(nam), this));
	symbolMap[e] = te;
}

Exp *UserProc::expFromSymbol(const char *nam)
{
	for (SymbolMapType::iterator it = symbolMap.begin(); it != symbolMap.end(); it++) {
		Exp* e = it->second;
		if (e->isLocal() && !strcmp(((Const*)((Location*)e)->getSubExp1())->getStr(), nam))
			return it->first;
	}
	return NULL;
}

const char* UserProc::getLocalName(int n) { 
	int i = 0;
	for (std::map<std::string, Type*>::iterator it = locals.begin(); it != locals.end(); it++, i++)
		if (i == n)
			return it->first.c_str();
	return NULL;
}

char* UserProc::getSymbolName(Exp* e) {
	SymbolMapType::iterator it = symbolMap.find(e);
	if (it == symbolMap.end()) return NULL;
	Exp* loc = it->second;
	if (!loc->isLocal()) return NULL;
	return ((Const*)((Location*)loc)->getSubExp1())->getStr();
}


void UserProc::countRefs(RefCounter& refCounts) {
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
#if 0			
		if (s->isPhi()) {
			// FIXME: is this needed now?
			((PhiAssign*)s)->simplifyRefs();
			s->simplify();
		}
#endif
		// Don't count uses in implicit statements. There is no RHS of course, but you can still have x from m[x] on the
		// LHS and so on, and these are not real uses
		if (s->isImplicit()) continue;
		if (DEBUG_UNUSED)
			LOG << "counting references in " << s << "\n";
		LocationSet refs;
		s->addUsedLocs(refs, false);			// Ignore uses in collectors
		LocationSet::iterator rr;
		for (rr = refs.begin(); rr != refs.end(); rr++) {
			if (((Exp*)*rr)->isSubscript()) {
				Statement *ref = ((RefExp*)*rr)->getDef();
				if (ref && ref->getNumber()) {
					refCounts[ref]++;
					if (DEBUG_UNUSED)
						LOG << "counted ref to " << *rr << "\n";
				}
			}
		}
	}
	if (DEBUG_UNUSED) {
		RefCounter::iterator rr;
		LOG << "### reference counts for " << getName() << ":\n";
		for (rr = refCounts.begin(); rr != refCounts.end(); ++rr)
			LOG << "  " << rr->first->getNumber() << ":" << rr->second << "\t";
		LOG << "\n### end reference counts\n"; 
	}
}

// Note: call the below after translating from SSA form
// FIXME: this can be done before transforming out of SSA form now, surely...
void UserProc::removeUnusedLocals() {
	if (VERBOSE)
		LOG << "removing unused locals (final) for " << getName() << "\n";
	std::set<std::string> usedLocals;
	StatementList stmts;
	getStatements(stmts);
	// First count any uses of the locals
	StatementList::iterator ss;
	for (ss = stmts.begin(); ss != stmts.end(); ss++) {
		Statement* s = *ss;
		LocationSet refs;
		s->addUsedLocs(refs);
		LocationSet::iterator rr;
		for (rr = refs.begin(); rr != refs.end(); rr++) {
			Exp* r = *rr;
			//if (r->isSubscript())					// Presumably never seen now
			//	r = ((RefExp*)r)->getSubExp1();
			char* sym = findLocal(r);				// Look up raw expressions in the symbolMap, and check in symbols
			// Must be a real symbol, and not defined in this statement, unless it is a return statement (in which case
			// it is used outside this procedure), or a call statement. Consider local7 = local7+1 and
			// return local7 = local7+1 and local7 = call(local7+1), where in all cases, local7 is not used elsewhere
			// outside this procedure. With the assign, it can be deleted, but with the return or call statements, it
			// can't.
			if (sym && (s->isReturn() || s->isCall() || !s->definesLoc(r))) {
				std::string name(sym);
				usedLocals.insert(name);
				if (DEBUG_UNUSED)
					LOG << "counted local " << sym << " in " << s << "\n";
			}
		}
		if (s->isAssignment() && ((Assignment*)s)->getLeft()->isLocal()) {
			Assignment* as = (Assignment*)s;
			Const* c = (Const*)((Unary*)as->getLeft())->getSubExp1();
			std::string name(c->getStr());
			usedLocals.insert(name);
			if (VERBOSE) LOG << "Counted local " << name.c_str() << " on left of " << s << "\n";

		}
	}
	// Now record the unused ones in set removes
	std::map<std::string, Type*>::iterator it;
	std::set<std::string> removes;
#if 0
	int nextLocal = 0;
#endif
	for (it = locals.begin(); it != locals.end(); it++) {
		std::string& name = const_cast<std::string&>(it->first);
		// LOG << "Considering local " << name << "\n";
		if (usedLocals.find(name) == usedLocals.end()) {
			if (VERBOSE)
				LOG << "removed unused local " << name.c_str() << "\n";
			removes.insert(name);
		}
#if 0	// Ugh - still have to rename the variables.
		else {
			if (name.substr(0, 5) == "local") {
				// Make the locals consequtive
				std::ostringstream os;
				os << "local" << nextLocal++;
				name = os.str();
			}
		}
#endif
	}
	// Remove any definitions of the removed locals
	for (ss = stmts.begin(); ss != stmts.end(); ++ss) {
		Statement* s = *ss;
		LocationSet ls;
		LocationSet::iterator ll;
		s->getDefinitions(ls);
		for (ll = ls.begin(); ll != ls.end(); ++ll) {
			char* name = findLocal(*ll);
			if (name == NULL) continue;
			std::string str(name);
			if (removes.find(str) != removes.end()) {
				// Remove it. If an assign, delete it; otherwise (call), remove the define
				if (s->isAssignment()) {
					removeStatement(s);
					break;				// Break to next statement
				} else if (s->isCall())
					// Remove just this define. May end up removing several defines from this call.
					((CallStatement*)s)->removeDefine(*ll);
				// else if a ReturnStatement, don't attempt to remove it. The definition is used *outside* this proc.
			}
		}
	}
	// Finally, remove them from locals, so they don't get declared
	for (std::set<std::string>::iterator it1 = removes.begin(); it1 != removes.end(); it1++)
		locals.erase(*it1);
}

// Note: if depth < 0, consider all depths
void UserProc::remUnusedStmtEtc(RefCounter& refCounts, int depth) {
	StatementList stmts;
	getStatements(stmts);
	bool change;
	do {								// FIXME: check if this is ever needed
		change = false;
		StatementList::iterator ll = stmts.begin();
		while (ll != stmts.end()) {
			Statement* s = *ll;
			if (!s->isAssignment()) {
				// Never delete a statement other than an assignment (e.g. nothing "uses" a Jcond)
				ll++;
				continue;
			}
			Assignment* as = (Assignment*)s;
			Exp* asLeft = as->getLeft();
			if (asLeft && depth >= 0 && asLeft->getMemDepth() > depth) {
				ll++;
				continue;
			}
			if (asLeft && asLeft->getOper() == opGlobal) {
				// assignments to globals must always be kept
				ll++;
				continue;
			}
			if (asLeft->getOper() == opMemOf &&
					symbolMap.find(asLeft) == symbolMap.end() &&		// Real locals are OK
					(!as->isAssign() || !(*new RefExp(asLeft, NULL) == *((Assign*)s)->getRight()))) {
				// Looking for m[x] := anything but m[x]{-}
				// Assignments to memof anything-but-local must always be kept.
				ll++;
				continue;
			}
#if 0		// We actually want to remove this if no-one is using it, otherwise we'll create an interference that we
			// can't handle
			if (asLeft->getOper() == opParam) {
				ll++;
				continue;
			}
#endif
			if (asLeft->getOper() == opMemberAccess || asLeft->getOper() == opArrayIndex) {
				// can't say with these
				ll++;
				continue;
			}
			if (refCounts.find(s) == refCounts.end() || refCounts[s] == 0) {	// Care not to insert unnecessarily
				// First adjust the counts, due to statements only referenced by statements that are themselves unused.
				// Need to be careful not to count two refs to the same def as two; refCounts is a count of the number
				// of statements that use a definition, not the total number of refs
				StatementSet stmtsRefdByUnused;
				LocationSet components;
				s->addUsedLocs(components, false);		// Second parameter false to ignore uses in collectors
				LocationSet::iterator cc;
				for (cc = components.begin(); cc != components.end(); cc++) {
					if ((*cc)->isSubscript()) {
						stmtsRefdByUnused.insert(((RefExp*)*cc)->getDef());
					}
				}
				StatementSet::iterator dd;
				for (dd = stmtsRefdByUnused.begin(); dd != stmtsRefdByUnused.end(); dd++) {
					if (*dd == NULL) continue;
					if (DEBUG_UNUSED)
						LOG << "decrementing ref count of " << (*dd)->getNumber() << " because " << s->getNumber() <<
							" is unused\n";
					refCounts[*dd]--;
				}
				if (DEBUG_UNUSED)
					LOG << "removing unused statement " << s->getNumber() << " " << s << "\n";
				removeStatement(s);
				ll = stmts.erase(ll);	// So we don't try to re-remove it
				change = true;
				continue;				// Don't call getNext this time
			}
			ll++;
		}
	} while (change);
	updateBlockVars();					// Recaluclate at least the liveness
}

//
//	SSA code
//

void UserProc::fromSSAform() {
	if (VERBOSE)
		LOG << "transforming " << getName() << " from SSA\n";

	StatementList stmts;
	getStatements(stmts);
	igraph ig;

	// First split the live ranges where needed, i.e. when the type of a subscripted variable is different to
	// its previous type. Start at the top, because we don't want to rename parameters (e.g. argc)
	StatementList::iterator it;
	std::map<Exp*, Type*, lessExpStar> firstTypes;
	std::map<Exp*, Type*, lessExpStar>::iterator ff;
	// Start with the parameters. There is not always a use of every parameter, yet that location may be used with
	// a different type (e.g. envp used as int in test/sparc/fibo-O4)
	int n = signature->getNumParams();
	for (int i=0; i < n; i++) {
		Exp* namedParam = Location::param(signature->getParamName(i));
		firstTypes[namedParam] = signature->getParamType(i);
	}
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		LocationSet defs;
		s->getDefinitions(defs);
		LocationSet::iterator dd;
		for (dd = defs.begin(); dd != defs.end(); dd++) {
			Exp* base = *dd;
			Type* ty = s->getTypeFor(base);
			if (ty == NULL)				// Can happen e.g. when getting the type for %flags
				ty = new VoidType();
			ff = firstTypes.find(base);
			if (ff == firstTypes.end()) {
				// There is no first type yet. Record it.
				firstTypes[base] = ty;
			} else if (ff->second && !ty->isCompatibleWith(ff->second)) {
				// There already is a type for base, and it is different to the type for this definition.
				// Record an "interference" so it will get a new variable
				RefExp* ref = new RefExp(base, s);
				//ig[ref] = newLocal(ty);
				ig[ref] = getSymbolExp(ref, ty);
			}
		}
	}
	// Find the interferences generated by more than one version of a variable being live at the same program point
	cfg->findInterferences(ig);


	if (DEBUG_LIVENESS) {
		LOG << "  ig Interference graph:\n";
		igraph::iterator ii;
		for (ii = ig.begin(); ii != ig.end(); ii++)
			LOG << "  ig " << ii->first << " -> " << ii->second << "\n";
	}

	// First rename the variables (including phi's, but don't remove).  The below could be replaced by
	//  replaceExpressionsWithSymbols() now, except that then references don't get removed.
	// NOTE: it is not possible to postpone renaming these locals till the back end, since the same base location
	// may require different names at different locations, e.g. r28{-} is local0, r28{16} is local1
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		s->fromSSAform(ig);
	}

	// Now remove the phis
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		if (!s->isPhi()) continue;
		// Check that the base variables are all the same
		PhiAssign* pa = (PhiAssign*)s;
		LocationSet refs;
		pa->addUsedLocs(refs);
		Exp* first = pa->begin()->e;
		bool phiParamsSame = true;
		if (pa->getNumDefs() > 1) {
			PhiAssign::iterator uu;
			for (uu = ++pa->begin(); uu != pa->end(); uu++) {
				if (!(*uu->e == *first)) {
					phiParamsSame = false;
					break;
				}
			}
		}
		if (phiParamsSame) {
			// Is the left of the phi assignment the same base variable as all the operands?
			if (*pa->getLeft() == *first) {
				if (DEBUG_LIVENESS || DEBUG_UNUSED)
					LOG << "removing phi: left and all refs same or 0: " << s << "\n";
				// Just removing the refs will work, or removing the whole phi
				// NOTE: Removing the phi here may cause other statments to be not used. Soon I want to remove the
				// phi's earlier, so this code can be removed. - MVE
				removeStatement(s);
			} else
				// Need to replace the phi by an expression,
				// e.g. local0 = phi(r24{3}, r24{5}) becomes 
				//		local0 = r24
				pa->convertToAssign(first->clone());
		}
		else {
			// Need new local. Under certain circumstances (e.g. sparc/fromssa2) we needed a copy statement, but most
			// times that just creates more locals than is necessary.
			// TODO: find a safe way of doing the "optimisation" below
#if 1
			// Just replace all the definitions the phi statement refers to with tempLoc.
			// WRONG! There can be other uses of those definitions. Even if these are changed, what if we have to
			// change definition to two or more new variables? So really need copies, unless something clever is done.
			// Exp* tempLoc = newLocal(pa->getType());
			Exp* tempLoc = getSymbolExp(new RefExp(pa->getLeft(), pa), pa->getType());
			if (DEBUG_LIVENESS)
				LOG << "phi statement " << s << " requires local, using " << tempLoc << "\n";
			// For each definition ref'd in the phi
			PhiAssign::iterator rr;
			for (rr = pa->begin(); rr != pa->end(); rr++) {
#if 0
				// Replace the LHS of the definitions (use setLeftFor, since some could be calls with more than one
				// return) with the new temporary
				rr->def->setLeftFor(rr->e, tempLoc);
#else
				insertAssignAfter(rr->def, tempLoc, rr->e);
#endif
			}
			// Replace the RHS of the phi with tempLoc
			pa->convertToAssign(tempLoc);
#else
			// We can often just use the LHS of the phiassign. The problem is that the variable at the left of the
			// phiassign can overlap with other versions of the same named variable. It may have something to do
			// with one of the arguments of the phi statement being in the interference graph.
			// The proper solution would be to change the igraph so it is obvious what interferes with what.
			// Replace the LHS of the definitions with the left of the PhiAssign
			Exp* left = pa->getLeft();
			if (DEBUG_LIVENESS)
				LOG << "phi statement " << s << " requires back substitution, using " << left << "\n";
			// For each definition ref'd in the phi
			PhiAssign::iterator rr;
			for (rr = pa->begin(); rr != pa->end(); rr++) {
				// Replace the LHS of the definitions (use setLeftFor, since some could be calls with more than one
				// return) with left
				rr->def->setLeftFor(rr->e, left);
			}
#endif
		}
	}

	// Now remove subscripts from the symbol map
	SymbolMapType::iterator ss;
	SymbolMapType temp(symbolMap);		// Have to copy to a new map, since the ordering is changed by stripping subs!
	symbolMap.clear();
	for (ss = temp.begin(); ss != temp.end(); ++ss) {
		bool allZero;
		Exp* from = ss->first->removeSubscripts(allZero);
		if (allZero)
			symbolMap[from] = ss->second;		// Put the unsubscripted version into the symbolMap
		else
			// Put the subscripted version back, e.g. r24{39}, so it will be declared in the decompiled output (e.g.
			// local10 /* r24{39} */. This one will be the result of a liveness overlap, and the local is already
			symbolMap[ss->first] = ss->second;	// modified into the IR.
	}

	// Also the parameters
	StatementList::iterator pp;
	for (pp = parameters.begin(); pp != parameters.end(); ++pp) {
		bool allZero;
		Exp* lhs = ((Assignment*)*pp)->getLeft();
		Exp* clean = lhs->clone()->removeSubscripts(allZero);
		if (allZero)
			((Assignment*)*pp)->setLeft(clean);
		// Else leave them alone
	}
}

#if 0
void UserProc::insertArguments(StatementSet& rs) {
	cfg->insertArguments(rs);
}
#endif

bool inProve = false;

bool UserProc::canProveNow()
{
	return !inProve;
}

static Binary allEqAll(opEquals,
	new Terminal(opDefineAll),
	new Terminal(opDefineAll));

// this function was non-reentrant
bool UserProc::prove(Exp *query)
{
#if 0							// Re-entrancy is quite useful
	if (inProve) {
		LOG << "attempted reentry of prove, returning false\n";
		return false;
	}
#endif
	inProve = true;
	if (proven.find(query) != proven.end()) {
		inProve = false;
		if (DEBUG_PROOF) LOG << "prove returns true for " << query << " in " << getName() << "\n";
		return true;
	}

	if (Boomerang::get()->noProve) {
		inProve = false;
		return false;
	}

	Exp *original = query->clone();

	assert(query->getOper() == opEquals);
	
	// subscript locs on the right with {-} (NULL reference)
	LocationSet locs;
	query->getSubExp2()->addUsedLocs(locs);
	LocationSet::iterator xx;
	for (xx = locs.begin(); xx != locs.end(); xx++) {
		query->setSubExp2(query->getSubExp2()->expSubscriptValNull(*xx));
	}

	if (query->getSubExp1()->getOper() != opSubscript) {
		bool gotDef = false;
		// replace expression from return set with expression in the collector of the return 
		if (theReturnStatement) {
			Exp* def = theReturnStatement->findDefFor(query->getSubExp1());
			if (def) {
				query->setSubExp1(def);
				gotDef = true;
			}
		}
		if (!gotDef) {
			// OK, the thing I'm looking for isn't in the return collector, but perhaps there is an entry for <all>
			// If this is proved, then it is safe to say that x == x for any x with no definition reaching the exit
			Exp* left = ((Binary*)original)->getSubExp1();
			Exp* right = ((Binary*)original)->getSubExp2()->clone()->simplify();	// In case it's sp+0
			if (*left == *right &&								// x == x
					left->getOper() != opDefineAll &&			// Beware infinite recursion
					prove(&allEqAll)) {							// Recurse in case <all> not proven yet
				if (DEBUG_PROOF)
					LOG << "Using all=all for " << query->getSubExp1() << "\n" << "prove returns true\n";
				inProve = false;
				proven.insert(original);
				return true;
			}
			if (DEBUG_PROOF)
				LOG << "not in return collector: " << query->getSubExp1() << "\n" << "prove returns false\n";
			inProve = false;
			return false;
		}
	}

	// proven.insert(original);			// This was the "induction" step; also made prove() non re-entrant

	std::set<PhiAssign*> lastPhis;
	std::map<PhiAssign*, Exp*> cache;
	if (!prover(query, lastPhis, cache, original)) {
		//proven.erase(original);		// Undo the "induction" step
		inProve = false;
		if (DEBUG_PROOF) LOG << "prove returns false\n";
		return false;
	}
 
	proven.insert(original);			// Save the now proven equation
	inProve = false;
	if (DEBUG_PROOF) LOG << "prove returns true\n";
	return true;
}

bool UserProc::prover(Exp *query, std::set<PhiAssign*>& lastPhis, std::map<PhiAssign*, Exp*> &cache, Exp* original,
		PhiAssign* lastPhi /* = NULL */) {
	// A map that seems to be used to detect loops in the call graph:
	std::map<CallStatement*, Exp*> callwd;
	Exp *phiInd = query->getSubExp2()->clone();

	if (lastPhi && cache.find(lastPhi) != cache.end() && *cache[lastPhi] == *phiInd) {
		if (DEBUG_PROOF)
			LOG << "true - in the cache\n";
		return true;
	} 

	std::set<Statement*> refsTo;

	query = query->clone();
	bool change = true;
	bool swapped = false;
	while (change) {
		if (DEBUG_PROOF) {
			LOG << query << "\n";
		}
	
		change = false;
		if (query->getOper() == opEquals) {

			// same left and right means true
			if (*query->getSubExp1() == *query->getSubExp2()) {
				query = new Terminal(opTrue);
				change = true;
			}

			// move constants to the right
			Exp *plus = query->getSubExp1();
			Exp *s1s2 = plus ? plus->getSubExp2() : NULL;
			if (!change && plus->getOper() == opPlus && s1s2->isIntConst()) {
				query->setSubExp2(new Binary(opPlus,
					query->getSubExp2(),
					new Unary(opNeg, s1s2->clone())));
				query->setSubExp1(((Binary*)plus)->getSubExp1());
				change = true;
			}
			if (!change && plus->getOper() == opMinus && s1s2->isIntConst()) {
				query->setSubExp2(new Binary(opPlus, query->getSubExp2(), s1s2->clone()));
				query->setSubExp1(((Binary*)plus)->getSubExp1());
				change = true;
			}


			// substitute using a statement that has the same left as the query
			if (!change && query->getSubExp1()->getOper() == opSubscript) {
				RefExp *r = (RefExp*)query->getSubExp1();
				Statement *s = r->getDef();
				CallStatement *call = dynamic_cast<CallStatement*>(s);
				if (call) {
					// See if we can prove something about this register.
					// Note: formerly it could also have been the original register we are looking for ("proven" by
					// induction, but this is no longer done. Instead, we use the CycleList parameter below.)
					UserProc* destProc = (UserProc*)call->getDestProc();
					if (destProc && !destProc->isLib() && destProc->status <= PROC_PREPRES) {
						// The destination procedure does not have preservation proved as yet, because it is involved
						// in a cycle. Use the inductive preservation logic to determine whether query is true for
						// this procedure (involves extensive recursion)
						bool ret = destProc->inductivePreservation(this);
						if (DEBUG_PROOF)
							LOG << "inductive preservation for call from " << getName() << " to " << 
								destProc->getName() << ", returns " << (ret ? "true" : "false") << " for " << query <<
								" \n";
						return ret;
					} else {
						Exp *right = call->getProven(r->getSubExp1());	// getProven returns the right side of what is
						if (right) {									// proven about r (the LHS of query)
							right = right->clone();
							if (callwd.find(call) != callwd.end() && *callwd[call] == *query) {
								LOG << "found call loop to " << call->getDestProc()->getName() << " " << query << "\n";
								query = new Terminal(opFalse);
								change = true;
							} else {
								callwd[call] = query->clone();
								if (DEBUG_PROOF)
									LOG << "using proven for " << call->getDestProc()->getName() << " " 
										<< r->getSubExp1() << " = " << right << "\n";
								right = call->localiseExp(right);
								if (DEBUG_PROOF)
									LOG << "right with subs: " << right << "\n";
								query->setSubExp1(right);				// Replace LHS of query with right
								change = true;
							}
						}
					}
				} else if (s && s->isPhi()) {
					// for a phi, we have to prove the query for every statement
					PhiAssign *pa = (PhiAssign*)s;
					PhiAssign::iterator it;
					bool ok = true;
					if (lastPhis.find(pa) != lastPhis.end() || pa == lastPhi) {
						if (DEBUG_PROOF)
							LOG << "phi loop detected ";
						ok = (*query->getSubExp2() == *phiInd);
						if (ok && DEBUG_PROOF)
							LOG << "(set true due to induction)\n";		// FIXME: induction??!
						if (!ok && DEBUG_PROOF)
							LOG << "(set false " << query->getSubExp2() << " != " << phiInd << ")\n";
					} else {
						if (DEBUG_PROOF)
							LOG << "found " << s << " prove for each\n";
						for (it = pa->begin(); it != pa->end(); it++) {
							Exp *e = query->clone();
							RefExp *r1 = (RefExp*)e->getSubExp1();
							r1->setDef(it->def);
							if (DEBUG_PROOF)
								LOG << "proving for " << e << "\n";
							lastPhis.insert(lastPhi);
							if (!prover(e, lastPhis, cache, original, pa)) { 
								ok = false; 
								//delete e; 
								break; 
							}
							lastPhis.erase(lastPhi);
							//delete e;
						}
						if (ok)
							cache[pa] = query->getSubExp2()->clone();
					}
					if (ok)
						query = new Terminal(opTrue);
					else 
						query = new Terminal(opFalse);
					change = true;
				} else if (s && s->isAssign()) {
					if (s && refsTo.find(s) != refsTo.end()) {
						LOG << "detected ref loop " << s << "\n";
						LOG << "refsTo: ";
						std::set<Statement*>::iterator ll;
						for (ll = refsTo.begin(); ll != refsTo.end(); ++ll)
							LOG << (*ll)->getNumber() << ", ";
						LOG << "\n";
						assert(false);
					} else {
						refsTo.insert(s);
						query->setSubExp1(((Assign*)s)->getRight()->clone());
						change = true;
					}
				}
			}

			// remove memofs from both sides if possible
			if (!change && query->getSubExp1()->getOper() == opMemOf && query->getSubExp2()->getOper() == opMemOf) {
				query->setSubExp1(((Unary*)query->getSubExp1())->getSubExp1());
				query->setSubExp2(((Unary*)query->getSubExp2())->getSubExp1());
				change = true;
			}

			// is ok if both of the memofs are subscripted with NULL
			if (!change && query->getSubExp1()->getOper() == opSubscript &&
					query->getSubExp1()->getSubExp1()->getOper() == opMemOf &&
					((RefExp*)query->getSubExp1())->getDef() == NULL &&
					query->getSubExp2()->getOper() == opSubscript &&
					query->getSubExp2()->getSubExp1()->getOper() == opMemOf &&
					((RefExp*)query->getSubExp2())->getDef() == NULL) {
				query->setSubExp1(((Unary*)query->getSubExp1()->getSubExp1())->getSubExp1());
				query->setSubExp2(((Unary*)query->getSubExp2()->getSubExp1())->getSubExp1());
				change = true;
			}

			// find a memory def for the right if there is a memof on the left
			// FIXME: this seems pretty much like a bad hack!
			if (!change && query->getSubExp1()->getOper() == opMemOf) {
				StatementList stmts;
				getStatements(stmts);
				StatementList::iterator it;
				for (it = stmts.begin(); it != stmts.end(); it++) {
					Assign* s = (Assign*)*it;
					if (s->isAssign() && *s->getRight() == *query->getSubExp2() && s->getLeft()->getOper() == opMemOf) {
						query->setSubExp2(s->getLeft()->clone());
						change = true;
						break;
					}
				}
			}

			// last chance, swap left and right if haven't swapped before
			if (!change && !swapped) {
				Exp *e = query->getSubExp1();
				query->setSubExp1(query->getSubExp2());
				query->setSubExp2(e);
				change = true;
				swapped = true;
				refsTo.clear();
			}
		} else if (query->isIntConst()) {
			Const *c = (Const*)query;
			query = new Terminal(c->getInt() ? opTrue : opFalse);
		}

		Exp *old = query->clone();

		query = query->clone()->simplify();

		if (change && !(*old == *query) && DEBUG_PROOF) {
			LOG << old << "\n";
		}
		//delete old;
	}
	
	return query->getOper() == opTrue;
}

// Get the set of locations defined by this proc. In other words, the define set, currently called returns
void UserProc::getDefinitions(LocationSet& ls) {
	int n = signature->getNumReturns();
	for (int j=0; j < n; j++) {
		ls.insert(signature->getReturnExp(j));
	}
}

#if 0			// All going well, we don't need this any more
// "Local" member function, used below
void UserProc::doCountReturns(Statement* def, ReturnCounter& rc, Exp* loc)
{
	if (def == NULL) return;
	CallStatement* call = dynamic_cast<CallStatement*>(def);
	if (call == NULL) return;
	// We have a reference to a return of the call statement
	UserProc* proc = (UserProc*) call->getDestProc();
	//if (proc->isLib()) return;
	if (DEBUG_UNUSED) {
		LOG << " @@ counted use of return location " << loc << " for call to ";
		if (proc) 
			LOG << proc->getName();
		else
			LOG << "(null)";
		LOG << " at " << def->getNumber() << " in " << getName() << "\n";
	}
	// We want to count the return that corresponds to this loc. This can be a different expression to loc because
	// replacements are done in the call's return list as part of decompilation
	int n = call->findReturn(loc);
	if (n != -1) {
		Exp *ret = NULL;
		if (proc)
			ret = proc->getSignature()->getReturnExp(n);
		else {
			assert(call->isComputed());
			std::vector<Exp*> &returns = prog->getDefaultReturns();
			assert(n < (int)returns.size());
			ret = returns[n];
		}
		rc[proc].insert(ret);
	}
}

void UserProc::countUsedReturns(ReturnCounter& rc) {
	if (DEBUG_UNUSED)
		LOG << " @@ counting used returns in " << getName() << "\n";
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator ss;
	// For each statement this proc
	for (ss = stmts.begin(); ss != stmts.end(); ss++) {
		LocationSet used;
		(*ss)->addUsedLocs(used);
		LocationSet::iterator ll;
		// For each use this statement
		for (ll = used.begin(); ll != used.end(); ll++) {
			Statement* def;
			Exp* loc = *ll;
			if (loc->isLocal()) {
				// We want the raw expression here
				loc = getSymbolExp(((Const*)((Location*)loc)->getSubExp1())->getStr());
				if (loc == NULL) continue;		// Needed?
			}
			if (loc->isSubscript()) {
				// for this one reference
				def = ((RefExp*)loc)->getDef();
				doCountReturns(def, rc, ((RefExp*)loc)->getSubExp1());
#if 0
			} else if ((loc)->isPhi()) {
				StatementVec::iterator rr;
				PhiAssign& pa = (PhiAssign&)*loc;
				// for each reference this phi expression
				for (rr = pa.begin(); rr != pa.end(); rr++)
					doCountReturns(rr->def, rc, pa.getSubExp1());
#endif
			} 
		}
	}
}
#endif

#if 0
bool UserProc::removeUnusedReturns(ReturnCounter& rc) {
	std::set<Exp*, lessExpStar> removes;	// Else iterators confused
	std::set<Exp*, lessExpStar>& useSet = rc[this];
	for (unsigned i = 0; i < signature->getNumReturns(); i++) {
		Exp *ret = signature->getReturnExp(i);
		if (useSet.find(ret) == useSet.end()) {
			removes.insert(ret);
		}
	}
	std::set<Exp*, lessExpStar>::iterator it;
	// Exp* stackExp = NULL;
	// if (signature->isPromoted())
		// stackExp = Location::regOf(signature->getStackRegister());
	bool removedOne = false;
	for (it = removes.begin(); it != removes.end(); it++) {
		// The logic here was doubly screwed. For one thing, there should not have been a not on isPromoted.
		// For another, when signatures are not promoted, getStackRegister() (the one that does NOT take a Prog
		// parameter) always returned 0 (!!), so for unpromoted procs, the SP was removed.
		// Plus, it turns out that somehow even full signature procs need this removal
	 	//if ( signature->isPromoted() && !(*stackExp == **it))
		//if (!signature->isPromoted() &&	 (*stackExp == **it))
		//if (**it == *stackExp && signature->isFullSignature())
			// If known to have a full signature, don't attempt to remove the SP
			// continue;
		if (DEBUG_UNUSED)
			LOG << " @@ removing unused return " << *it << " in " << getName() << "\n";
		removeReturn(*it);
		removedOne = true;
	}
	return removedOne;
}
#endif

void Proc::addCallers(std::set<UserProc*>& callers) {
	std::set<CallStatement*>::iterator it;
	for (it = callerSet.begin(); it != callerSet.end(); it++) {
		UserProc* callerProc = (*it)->getProc();
		callers.insert(callerProc);
	}
}

void UserProc::addCallees(std::list<UserProc*>& callees) {
    // SLOW SLOW SLOW
    // this function is evil now... REALLY evil... hope it doesn't get called too often
	std::list<Proc*>::iterator it;
	for (it = calleeList.begin(); it != calleeList.end(); it++) {
		UserProc* callee = (UserProc*)(*it);
		if (callee->isLib()) continue;
		addCallee(callee);
	}
}

void UserProc::conTypeAnalysis() {
	if (DEBUG_TA)
		LOG << "type analysis for procedure " << getName() << "\n";
	Constraints consObj;
	LocationSet cons;
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator ss;
	// For each statement this proc
	int conscript = 0;
	for (ss = stmts.begin(); ss != stmts.end(); ss++) {
		cons.clear();
		// So we can co-erce constants:
		conscript = (*ss)->setConscripts(conscript);
		(*ss)->genConstraints(cons);
		consObj.addConstraints(cons);
		if (DEBUG_TA)
			LOG << (*ss) << "\n" << &cons << "\n";
		// Remove the sizes immediately the constraints are generated.
		// Otherwise, x and x*8* look like different expressions
		(*ss)->stripSizes();
	}

	std::list<ConstraintMap> solns;
	bool ret = consObj.solve(solns);
	if (VERBOSE || DEBUG_TA) {
		if (!ret)
			LOG << "** could not solve type constraints for proc " << getName() << "!\n";
		else if (solns.size() > 1)
			// Note: require cast to unsigned for OS X and 64-bit hosts
			LOG << "** " << (unsigned)solns.size() << " solutions to type constraints for proc " << getName() << "!\n";
	}
		
	std::list<ConstraintMap>::iterator it;
	int solnNum = 0;
	ConstraintMap::iterator cc;
	if (DEBUG_TA) {
		for (it = solns.begin(); it != solns.end(); it++) {
			LOG << "solution " << ++solnNum << " for proc " << getName() << "\n";
			ConstraintMap& cm = *it;
			for (cc = cm.begin(); cc != cm.end(); cc++)
				LOG << cc->first << " = " << cc->second << "\n";
			LOG << "\n";
		}
	}

	// Just use the first solution, if there is one
	Prog* prog = getProg();
	if (solns.size()) {
		ConstraintMap& cm = *solns.begin();
		for (cc = cm.begin(); cc != cm.end(); cc++) {
			// Ick. A workaround for now (see test/pentium/sumarray-O4)
			//assert(cc->first->isTypeOf());
if (!cc->first->isTypeOf()) continue;
			Exp* loc = ((Unary*)cc->first)->getSubExp1();
			assert(cc->second->isTypeVal());
			Type* ty = ((TypeVal*)cc->second)->getType();
			if (loc->isSubscript())
				loc = ((RefExp*)loc)->getSubExp1();
			if (loc->isGlobal()) {
				char* nam = ((Const*)((Unary*)loc)->getSubExp1())->getStr();
				prog->setGlobalType(nam, ty->clone());
			} else if (loc->isLocal()) {
				char* nam = ((Const*)((Unary*)loc)->getSubExp1())->getStr();
				setLocalType(nam, ty);
			} else if (loc->isIntConst()) {
				Const* con = (Const*)loc;
				int val = con->getInt();
				if (ty->isFloat()) {
					// Need heavy duty cast here
					// MVE: check this! Especially when a double prec float
					con->setFlt(*(float*)&val);
					con->setOper(opFltConst);
				} else if (ty->isCString()) {
					// Convert to a string
					char* str = prog->getStringConstant(val, true);
					if (str) {
						// Make a string
						con->setStr(str);
						con->setOper(opStrConst);
					}
				}
				else {
					if (ty->isInteger() && ty->getSize() && ty->getSize() != STD_SIZE)
						// Wrap the constant in a TypedExp (for a cast)
						castConst(con->getConscript(), ty);
				}
			}
		}
	}

	// Clear the conscripts. These confuse the fromSSA logic, causing infinite
	// loops
	for (ss = stmts.begin(); ss != stmts.end(); ss++) {
		(*ss)->clearConscripts();
	}
}




bool UserProc::searchAndReplace(Exp *search, Exp *replace)
{
	bool ch = false;
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		ch |= s->searchAndReplace(search, replace);	
	}
	return ch; 
}

unsigned fudge(StatementList::iterator x) {
	StatementList::iterator y = x;
	return *(unsigned*)&y;
}

Exp *UserProc::getProven(Exp *left) {
	// Note: proven information is in the form "r28 = (r28 + 4)"
	for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); it != proven.end(); it++) 
		if (*(*it)->getSubExp1() == *left)
			return (*it)->getSubExp2();
	// 	not found, try the signature
	// No! The below should only be for library functions!
	// return signature->getProven(left);
	return NULL;
}

bool UserProc::isPreserved(Exp* e) {
	Binary equate(opEquals, e, e);	// e == e
	return proven.find(&equate) != proven.end();
}

void UserProc::castConst(int num, Type* ty) {
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		if ((*it)->castConst(num, ty))
			break;
	}
}

// Process calls with ellipsis parameters. Return true if any signature parameter types added.
bool UserProc::ellipsisProcessing() {
	BB_IT it;
	BasicBlock::rtlrit rrit; StatementList::reverse_iterator srit;
	bool ch = false;
	for (it = cfg->begin(); it != cfg->end(); ++it) {
		CallStatement* c = (CallStatement*) (*it)->getLastStmt(rrit, srit);
		// Note: we may have removed some statements, so there may no longer be a last statement!
		if (c == NULL || !c->isCall()) continue;
		ch |= c->ellipsisProcessing(prog);
	}
	return ch;
}

class LibProcMemo : public Memo {
public:
	LibProcMemo(int mId) : Memo(mId) { }

	bool visited;
	Prog *prog;
	Signature *signature;						// r
	ADDRESS address;
	Proc *m_firstCaller;
	ADDRESS m_firstCallerAddr;
	std::set<Exp*, lessExpStar> proven;			// r
	std::set<CallStatement*> callerSet;
	Cluster *cluster;
};

Memo *LibProc::makeMemo(int mId)
{
	LibProcMemo *m = new LibProcMemo(mId);
	m->visited = visited;
	m->prog = prog;
	m->signature = signature;
	m->address = address;
	m->m_firstCaller = m_firstCaller;
	m->m_firstCallerAddr = m_firstCallerAddr;
	m->proven = proven;
	m->callerSet = callerSet;
	m->cluster = cluster;

//	signature->takeMemo(mId);
//	for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); it != proven.end(); it++)
//		(*it)->takeMemo(mId);

	return m;
}

void LibProc::readMemo(Memo *mm, bool dec)
{
	LibProcMemo *m = dynamic_cast<LibProcMemo*>(mm);
	visited = m->visited;
	prog = m->prog;
	signature = m->signature;
	address = m->address;
	m_firstCaller = m->m_firstCaller;
	m_firstCallerAddr = m->m_firstCallerAddr;
	proven = m->proven;
	callerSet = m->callerSet;
	cluster = m->cluster;

//	signature->restoreMemo(m->mId, dec);
//	for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); it != proven.end(); it++)
//		(*it)->restoreMemo(m->mId, dec);
}

class UserProcMemo : public Memo {
public:
	UserProcMemo(int mId) : Memo(mId) { }

	bool visited;
	Prog *prog;
	Signature *signature;						// r
	ADDRESS address;
	Proc *m_firstCaller;
	ADDRESS m_firstCallerAddr;
	std::set<Exp*, lessExpStar> proven;			// r
	std::set<CallStatement*> callerSet;
	Cluster *cluster;

	Cfg* cfg;
	ProcStatus status;
	std::map<std::string, Type*> locals;		// r
	UserProc::SymbolMapType symbolMap;			// r
	std::list<Proc*> calleeList;
};

Memo *UserProc::makeMemo(int mId)
{
	UserProcMemo *m = new UserProcMemo(mId);
	m->visited = visited;
	m->prog = prog;
	m->signature = signature;
	m->address = address;
	m->m_firstCaller = m_firstCaller;
	m->m_firstCallerAddr = m_firstCallerAddr;
	m->proven = proven;
	m->callerSet = callerSet;
	m->cluster = cluster;

	m->cfg = cfg;
	m->status = status;
	m->locals = locals;
	m->symbolMap = symbolMap;
	m->calleeList = calleeList;

	signature->takeMemo(mId);
	for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); it != proven.end(); it++)
		(*it)->takeMemo(mId);

	for (std::map<std::string, Type*>::iterator it = locals.begin(); it != locals.end(); it++)
		(*it).second->takeMemo(mId);

	for (SymbolMapType::iterator it = symbolMap.begin(); it != symbolMap.end(); it++) {
		(*it).first->takeMemo(mId);
		(*it).second->takeMemo(mId);
	}

	return m;
}

void UserProc::readMemo(Memo *mm, bool dec)
{
	UserProcMemo *m = dynamic_cast<UserProcMemo*>(mm);
	visited = m->visited;
	prog = m->prog;
	signature = m->signature;
	address = m->address;
	m_firstCaller = m->m_firstCaller;
	m_firstCallerAddr = m->m_firstCallerAddr;
	proven = m->proven;
	callerSet = m->callerSet;
	cluster = m->cluster;

	cfg = m->cfg;
	status = m->status;
	locals = m->locals;
	symbolMap = m->symbolMap;
	calleeList = m->calleeList;

	signature->restoreMemo(m->mId, dec);
	for (std::set<Exp*, lessExpStar>::iterator it = proven.begin(); it != proven.end(); it++)
		(*it)->restoreMemo(m->mId, dec);

	for (std::map<std::string, Type*>::iterator it = locals.begin(); it != locals.end(); it++)
		(*it).second->restoreMemo(m->mId, dec);

	for (SymbolMapType::iterator it = symbolMap.begin(); it != symbolMap.end(); it++) {
		(*it).first->restoreMemo(m->mId, dec);
		(*it).second->restoreMemo(m->mId, dec);
	}
}

// Before Type Analysis, refs like r28{0} have a NULL Statement pointer. After this, they will point to an
// implicit assignment for the location. Thus, during and after type analysis, you can find the type of any
// location by following the reference to the definition
// Note: you need something recursive to make sure that child subexpressions are processed before parents
// Example: m[r28{0} - 12]{0} could end up adding an implicit assignment for r28{0} with a null reference!
void UserProc::addImplicitAssigns() {
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator it;
	ImplicitConverter ic(cfg);
	StmtImplicitConverter sm(&ic, cfg);
	for (it = stmts.begin(); it != stmts.end(); it++)
		(*it)->accept(&sm);
	cfg->setImplicitsDone();
}

char* UserProc::lookupSym(Exp* e) {
	if (e->isTypedExp())
		e = ((TypedExp*)e)->getSubExp1();
	SymbolMapType::iterator it;
	it = symbolMap.find(e);
	if (it == symbolMap.end())
		return NULL;
	Exp* sym = it->second;
	assert(sym->isLocal() || sym->isParam());
	return ((Const*)((Location*)sym)->getSubExp1())->getStr();
}

void UserProc::symbolMapToLog() {
	LOG << "symbols:\n";
	SymbolMapType::iterator it;
	for (it = symbolMap.begin(); it != symbolMap.end(); it++)
		LOG << "  " << it->first << " maps to " << it->second << "\n";
	LOG << "end symbols\n";
}

void UserProc::dumpLocals(std::ostream& os) {
	os << "locals:\n";
	for (std::map<std::string, Type*>::iterator it = locals.begin(); it != locals.end(); it++) {
		os << it->second->getCtype() << " " << it->first.c_str() << " ";
		Exp *e = expFromSymbol((*it).first.c_str());
		// Beware: for some locals, expFromSymbol() returns NULL (? No longer?)
		if (e)
			os << e << "\n";
		else
			os << "-\n";
	}
	os << "end locals\n";
}

void UserProc::dumpSymbolMap() {
	SymbolMapType::iterator it;
	for (it = symbolMap.begin(); it != symbolMap.end(); it++)
		std::cerr << "  " << it->first << " maps to " << it->second << "\n";
}

void UserProc::dumpLocals() {
	std::stringstream ost;
	dumpLocals(ost);
	std::cerr << ost.str();
}

void UserProc::dumpIgraph(igraph& ig) {
	std::cerr << "Interference graph:\n";
	igraph::iterator ii;
	for (ii = ig.begin(); ii != ig.end(); ii++)
		std::cerr << ii->first << " -> " << ii->second << "\n";
}

void UserProc::updateArguments() {
	if (VERBOSE)
		LOG << "### update arguments for " << getName() << " ###\n";
	BB_IT it;
	BasicBlock::rtlrit rrit; StatementList::reverse_iterator srit;
	for (it = cfg->begin(); it != cfg->end(); ++it) {
		CallStatement* c = (CallStatement*) (*it)->getLastStmt(rrit, srit);
		// Note: we may have removed some statements, so there may no longer be a last statement!
		if (c == NULL || !c->isCall()) continue;
		c->updateArguments();
		//c->bypass();
		if (VERBOSE) {
			std::ostringstream ost;
			c->print(ost);
			LOG << ost.str().c_str() << "\n";
		}
	}
	if (VERBOSE)
		LOG << "=== end update arguments for " << getName() << "\n";
}

void UserProc::updateCallDefines() {
	if (VERBOSE)
		LOG << "### update call defines for " << getName() << " ###\n";
	StatementList stmts;
	getStatements(stmts);
	StatementList::iterator it;
	for (it = stmts.begin(); it != stmts.end(); it++) {
		CallStatement* call = dynamic_cast<CallStatement*>(*it);
		if (call == NULL) continue;
		call->updateDefines();
	}
}

// Update the parameters, in case the signature and hence ordering and filtering has changed, or the locations in the
// collector have changed
void UserProc::insertParameter(Exp* e) {

	if (filterParams(e))
		return;						// Filtered out
	if (isPreserved(e))
		return;						// Also filter out preserveds (filterParams doesn't do it, since we don't want to
									// filter out preserveds for arguments)
			
	// Wrap it in an implicit assignment; DFA based TA should update the type later
	ImplicitAssign* as = new ImplicitAssign(e->clone());
	// Insert as, in order, into the existing set of parameters
	StatementList::iterator nn;
	bool inserted = false;
	for (nn = parameters.begin(); nn != parameters.end(); ++nn) {
		// If the new assignment is less than the current one ...
		if (signature->argumentCompare(*as, *(Assign*)*nn)) {
			nn = parameters.insert(nn, as);		// ... then insert before this position
			inserted = true;
			break;
		}
	}
	if (!inserted)
		parameters.insert(parameters.end(), as);	// In case larger than all existing elements
}


// Filter out locations not possible as return locations. Return true to *remove* (filter *out*)
bool UserProc::filterReturns(Exp* e) {
	if (isPreserved(e))
		return true;			// Don't keep preserved locations
	switch (e->getOper()) {
		case opPC:	return true;			// Ignore %pc
		case opDefineAll: return true;		// Ignore <all>
		case opTemp: return true;			// Ignore all temps (should be local to one instruction)
		// Would like to handle at least %ZF, %CF one day. For now, filter them out
		case opZF: case opCF: case opFlags:
			return true;
#if 0							// Ugh - if the stack pointer is not considered a return, then it will not a define in
								// the callers, and then the whole preserved locations process falls apart
		case opRegOf: {
			int sp;
			if (signature == NULL)
				sp = 999;
			else
				sp = signature->getStackRegister(prog);
			int r = ((Const*)((Location*)e)->getSubExp1())->getInt();
			return r == sp;
		}
#endif
		case opMemOf: {
#if 0				// The below doesn't work for e.g. Sparc's m[sp]
			Exp* addr = ((Location*)e)->getSubExp1();
			if (addr->isIntConst())
				return true;			// Global memory location
			OPER op = addr->getOper();
			if (op == opPlus || op == opMinus) {
				// Check for local variable: m[sp +- K]
				int sp;
				if (signature == NULL)
					sp = 999;
				else
					sp = signature->getStackRegister(prog);
				Exp* s1 = ((Binary*)addr)->getSubExp1();
				if (s1->isSubscript())
					s1 = ((RefExp*)s1)->getSubExp1();
				if (!s1->isRegN(sp)) return false;
				Exp* s2 = ((Binary*)addr)->getSubExp2();
				if (!s2->isIntConst()) return false;
				// This will filter out sparc struct returns, at m[sp+64] (in the *parent's* stack)
				if (op == opPlus && ((Const*)s2)->getInt() == 64 && signature->getPlatform() == PLAT_SPARC)
					return false;
				return true;			// Don't allow local variables
			}
			return false;				// Might be some weird memory expression that is not a local
#endif
			// return signature->isStackLocal(prog, e);		// Filter out local variables
			// Actually, surely all sensible architectures will only every return in registers. So for now, just
			// filter out all mem-ofs
			return true;
		}
		case opGlobal:
			return true;				// Never return in globals
		default:
			return false;
	}
	return false;
}

// Filter out locations not possible as parameters or arguments. Return true to remove
bool UserProc::filterParams(Exp* e) {
	switch (e->getOper()) {
		case opPC:	return true;
		case opTemp: return true;
		case opRegOf: {
			int sp = 999;
			if (signature) sp = signature->getStackRegister(prog);
			int r = ((Const*)((Location*)e)->getSubExp1())->getInt();
			return r == sp;
		}
		case opMemOf: {
			Exp* addr = ((Location*)e)->getSubExp1();
			if (addr->isIntConst())
				return true;			// Global memory location
			if (addr->isSubscript() && ((RefExp*)addr)->isImplicitDef()) {
				Exp* reg = ((RefExp*)addr)->getSubExp1();
				int sp = 999;
				if (signature) sp = signature->getStackRegister(prog);
				if (reg->isRegN(sp))
					return true;		// Filter out m[sp{-}] assuming it is the return address
			}
			return false;				// Might be some weird memory expression that is not a local
		}
		case opGlobal:
			return true;				// Never use globals as argument locations (could appear on RHS of args)
		default:
			return false;
	}
	return false;
}

char* UserProc::findLocal(Exp* e) {
	if (e->isLocal())
		return ((Const*)((Unary*)e)->getSubExp1())->getStr();
	// Look it up in the symbol map
	char* name = lookupSym(e);
	if (name == NULL)
		return NULL;
	// Now make sure it is a local; some symbols (e.g. parameters) are in the symbol map but not locals
	std::string str(name);
	if (locals.find(str) != locals.end())
		return name;
	return NULL;
}

// Algorithm:
/* fixCallAndPhiRefs
	for each statement s in this proc
	  if s is a phi statement ps
		let r be a ref made up of lhs and s
		for each parameter p of ps
		  if p == r						// e.g. test/pentium/fromssa2 r28{56}
			remove p from ps
		let lhs be left hand side of ps
		allSame = true
		let first be a ref built from first p
		do bypass but not propagation on first
		if result is of the form lhs{x}
		  replace first with x
		for each parameter p of ps after the first
		  let current be a ref built from p
		  do bypass but not propagation on current
		  if result is of form lhs{x}
			replace cur with x
		  if first != current
			allSame = false
		if allSame
		  let best be ref built from the "best" parameter p in ps ({-} better than {assign} better than {call})
		  replace ps with an assignment lhs := best
	else (ordinary statement)
	  do bypass and propagation for s
*/

void UserProc::fixCallAndPhiRefs() {
	if (VERBOSE)
		LOG << "### start fix call and phi bypass analysis for " << getName() << " ###\n";

	std::map<Exp*, int, lessExpStar> destCounts;
	StatementList::iterator it;
	Statement* s;
	StatementList stmts;
	getStatements(stmts);
#if 0
	// First pass: count the number of times each assignment LHS would be propagated somewhere
	for (it = stmts.begin(); it != stmts.end(); it++) {
		Statement* s = *it;
		if (s->isPhi()) continue;
		LocationSet exps;
		s->addUsedLocs(exps, true);		// True to also add uses from collectors
		LocationSet::iterator ll;
		for (ll = exps.begin(); ll != exps.end(); ll++) {
			Exp* e = *ll;
			if (!s->canPropagateToExp(e, -1)) continue;
			destCounts[e]++;				// Count propagatable expression (use e not lhs so it has the ref)
		}
	}
#endif

	// Scan for situations like this:
	// 56 r28 := phi{6, 26}
	// ...
	// 26 r28 := r28{56}
	// So we can remove the second parameter, then reduce the phi to an assignment, then propagate it
	for (it = stmts.begin(); it != stmts.end(); it++) {
		s = *it;
		if (s->isPhi()) {
			PhiAssign* ps = (PhiAssign*)s;
			RefExp* r = new RefExp(ps->getLeft(), ps);
			for (PhiAssign::iterator p = ps->begin(); p != ps->end(); ) {
				Exp* current = new RefExp(p->e, p->def);
				if (*current == *r) {					// Will we ever see this?
					p = ps->erase(p);					// Erase this phi parameter
					continue;
				}
				// Chase the definition
				if (p->def) {
					if (!p->def->isAssign()) {
						++p;
						continue;
					}
					Exp* rhs = ((Assign*)p->def)->getRight();
					if (*rhs == *r) {					// Check if RHS is a single reference to ps
						p = ps->erase(p);				// Yes, erase this phi parameter
						continue;
					}
				}
				++p;
			}
		}
	}
		
	// Second pass
	for (it = stmts.begin(); it != stmts.end(); it++) {
		s = *it;
		if (s->isPhi()) {
			PhiAssign* ps = (PhiAssign*)s;
			if (ps->getNumDefs() == 0) continue;		// Can happen e.g. for m[...] := phi {} when this proc is
														// involved in a recursion group
			Exp* lhs = ps->getLeft();
			bool allSame = true;
			// Let first be a reference build from the first parameter
			PhiAssign::iterator p = ps->begin();
			Exp* first = new RefExp(p->e, p->def);
			// bypass to first
			CallBypasser cb(ps);
			first = first->accept(&cb);
			if (cb.isTopChanged())
				first = first->simplify();
			first = first->propagateAll(maxDepth);		// Propagate
			if (cb.isMod()) {							// Modified?
				// if first is of the form lhs{x}
				if (first->isSubscript() && *((RefExp*)first)->getSubExp1() == *lhs)
					// replace first with x
					p->def = ((RefExp*)first)->getDef();
			}
			// For each parameter p of ps after the first
			for (++p; p != ps->end(); ++p) {
				Exp* current = new RefExp(p->e, p->def);
				CallBypasser cb2(ps);
				current = current->accept(&cb2);
				if (cb2.isTopChanged())
					current = current->simplify();
				current = current->propagateAll(maxDepth);
				if (cb2.isMod())					// Modified?
					// if current is of the form lhs{x}
					if (current->isSubscript() && *((RefExp*)current)->getSubExp1() == *lhs)
						// replace current with x
						p->def = ((RefExp*)current)->getDef();
				if (!(*first == *current))
					allSame = false;
			}

			if (allSame) {
				// let best be ref built from the "best" parameter p in ps ({-} better than {assign} better than {call})
				p = ps->begin();
				RefExp* best = new RefExp(p->e, p->def);
				for (++p; p != ps->end(); ++p) {
					RefExp* current = new RefExp(p->e, p->def);
					if (current->isImplicitDef()) {
						best = current;
						break;
					}
					if (p->def->isAssign())
						best = current;
					// If p->def is a call, this is the worst case; keep only (via first) if all parameters are calls
				}
				ps->convertToAssign(best);
				if (VERBOSE)
					LOG << "redundant phi replaced with copy assign; now " << ps << "\n";
			}
		} else {	// Ordinary statement
			s->bypass();
		}
	}

	// Also do xxx in m[xxx] in the use collector
	UseCollector::iterator cc;
	for (cc = col.begin(); cc != col.end(); ++cc) {
		if (!(*cc)->isMemOf()) continue;
		Exp* addr = ((Location*)*cc)->getSubExp1();
		CallBypasser cb(NULL);
		addr = addr->accept(&cb);
		if (cb.isMod())
			((Location*)*cc)->setSubExp1(addr);
	}

	if (VERBOSE)
		LOG << "### end fix call and phi bypass analysis for " << getName() << " ###\n";
}

void UserProc::markAsNonChildless(CycleSet* cs) {
	BasicBlock::rtlrit rrit; StatementList::reverse_iterator srit;
	BB_IT it;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
		CallStatement* c = (CallStatement*) bb->getLastStmt(rrit, srit);
		if (c->isCall() && c->isChildless()) {
			UserProc* dest = (UserProc*)c->getDestProc();
			if (cs->find(dest) != cs->end()) 	// Part of the cycle?
				// Yes, set the callee return statement (making it non childless)
				c->setCalleeReturn(dest->getTheReturnStatement());
		}
	}
}

// Propagate into xxx of m[xxx] in the UseCollector (locations live at the entry of this proc)
void UserProc::propagateToCollector(int depth) {
	UseCollector::iterator it;
	for (it = col.begin(); it != col.end(); ++it) {
		if (!(*it)->isMemOf()) continue;
		Exp* addr = ((Location*)*it)->getSubExp1();
		LocationSet used;
		LocationSet::iterator uu;
		addr->addUsedLocs(used);
		for (uu = used.begin(); uu != used.end(); uu++) {
			RefExp* r = (RefExp*)*uu;
			if (!r->isSubscript()) continue;
			Assign* as = (Assign*)r->getDef();
			if (as == NULL || !as->isAssign()) continue;
			if (depth != -1 && depth != as->getLeft()->getMemDepth()) continue;
			bool ch;
			Exp* res = addr->clone()->searchReplaceAll(r, as->getRight(), ch);
			if (!ch) continue;				// No change
			Exp* memOfRes = Location::memOf(res)->simplify();
			// First check to see if memOfRes is already in the set
			if (col.exists(memOfRes)) {
				/* it = */ col.remove(it);		// Already exists; just remove the old one
				continue;
			} else {
				if (VERBOSE)
					LOG << "propagating " << r << " to " << as->getRight() << " in collector; result " << memOfRes <<
						"\n";
				((Location*)*it)->setSubExp1(res);	// Change the child of the memof
			}
		}
	}
}

// Get the initial parameters, based on this UserProc's use collector
void UserProc::initialParameters() {
	if (VERBOSE)
		LOG << "### initial parameters for " << getName() << "\n";
	parameters.clear();
	UseCollector::iterator cc;
	for (cc = col.begin(); cc != col.end(); ++cc)
		parameters.append(new ImplicitAssign((*cc)->clone()));
	if (VERBOSE) {
		std::ostringstream ost;
		printParams(ost);
		LOG << ost.str().c_str();
	}
}

bool UserProc::inductivePreservation(UserProc* topOfCycle) {
	// FIXME: This is not correct in general!! It should work OK for self recursion, but not for general mutual
	//recursion. Not that hard, just not done yet.
	return true;
}

bool UserProc::isLocal(Exp* e) {
	if (!e->isMemOf()) return false;			// Don't want say a register
	SymbolMapType::iterator ff = symbolMap.find(e);
	if (ff == symbolMap.end()) return false;
	Exp* mapTo = ff->second;
	return mapTo->isLocal();
}

// Temporary hack: is this m[sp{-} +/- K]?
bool UserProc::isLocalOrParam(Exp* e) {
	if (!e->isMemOf()) return false;			// Don't want say a register
	Exp* addr = ((Location*)e)->getSubExp1();
	if (!signature->isPromoted()) return false;	// Prevent an assert failure if using -E
	int sp = signature->getStackRegister();
	Exp* initSp = new RefExp(Location::regOf(sp), NULL);	// sp{-}
	if (*addr == *initSp) return true;			// Accept m[sp{-}]
	if (addr->getArity() != 2) return false;	// Require sp +/- K
	OPER op = ((Binary*)addr)->getOper();
	if (op != opPlus && op != opMinus) return false;
	Exp* left =  ((Binary*)addr)->getSubExp1();
	if (!(*left == *initSp)) return false;
	Exp* right = ((Binary*)addr)->getSubExp2();
	return right->isIntConst();
}

// Remove unused returns for this procedure, based on the equation returns = modifieds isect union(live at c) for all
// c calling this procedure.
// The intersection operation will only remove locations. Removing returns can have three effects for each component y
// used by that return (e.g. if return r24 := r25{10} + r26{20} is removed, statements 10 and 20 will be affected and
// y will take the values r25{10} and r26{20}):
// 1) a statement s defining a return becomes unused if the only use of its definition was y
// 2) a call statement c defining y will no longer have y live if the return was the only use of y. This could cause a
//	change to the returns of c's destination, so removeUnusedReturns has to be called for c's destination proc (if not
//	already scheduled).
// 3) if y is a parameter (i.e. y is of the form loc{-}), then the signature of this procedure changes, and all callers
//	have to have their arguments trimmed, and a similar process has to be applied to all those caller's removed
//	arguments as is applied here to the removed returns.
void UserProc::removeUnusedReturns(std::set<UserProc*>& removeRetSet) {
	if (theReturnStatement == NULL)
		return;
	if (DEBUG_UNUSED)
		LOG << "%%% removing unused returns for " << getName() << " %%%\n";
	LocationSet unionOfCallerLiveLocs;
	if (strcmp(getName(), "main") == 0)
		// Just insert one return for main. Note: at present, the first parameter is still the stack pointer
		unionOfCallerLiveLocs.insert(signature->getReturnExp(1));
	else {
		// For each caller
		std::set<CallStatement*>& callers = getCallers();
		std::set<CallStatement*>::iterator cc;
		for (cc = callers.begin(); cc != callers.end(); ++cc) {
			// Union in the set of locations live at this call
			UseCollector* useCol = (*cc)->getUseCollector();
			unionOfCallerLiveLocs.makeUnion(useCol->getLocSet());
		}
	}
	// Intersect with the given location set
	bool removedSome = false;
	ReturnStatement::iterator rr;
	for (rr = theReturnStatement->begin(); rr != theReturnStatement->end(); ) {
		Assign* a = (Assign*)*rr;
		if (!unionOfCallerLiveLocs.exists(a->getLeft())) {
			if (DEBUG_UNUSED)
				LOG << "%%%  removing unused return " << a << " from proc " << getName() << "\n";
			Exp* rhs = a->getRight();
			if (rhs->isSubscript()) {
				CallStatement* call = (CallStatement*)((RefExp*)rhs)->getDef();
				if (call && call->isCall()) {
					// Remove the liveness for rhs at this call; when dataflow is redone, it may not come back, in which
					// case we have more work to do
					Exp* base = ((RefExp*)rhs)->getSubExp1();
					call->removeLiveness(base);
				}
			}
			rr = theReturnStatement->erase(rr);
			removedSome = true;
		}
		else
			rr++;
	}

	if (DEBUG_UNUSED) {
		std::ostringstream ost;
		unionOfCallerLiveLocs.print(ost);
		LOG << "%%%  union of caller live locations for " << getName() << ": " << ost.str().c_str() << "\n";
		LOG << "%%%  final returns for " << getName() << ": " << theReturnStatement->getReturns().prints() << "\n";
	}

	// Now update myself, especially because the call livenesses are possibly incorrect (so it's pointless removing
	// unused returns for children, since the liveness that this depends on is possibly incorrect).
	updateForUseChange(removeRetSet);
}

// See comments above for removeUnusedReturns(). Need to save the old parameters and call livenesses, redo the dataflow
// and removal of unused statements, recalculate the parameters and call livenesses, and if either or both of these are
// changed, recurse to parents or children respectively.
void UserProc::updateForUseChange(std::set<UserProc*>& removeRetSet) {
	// We need to remember the parameters, and all the livenesses for all the calls, to see if these are changed
	// by removing returns
	if (DEBUG_UNUSED) {
		LOG << "%%% updating " << getName() << " for changes to uses (returns or arguments)\n";
		LOG << "%%% updating dataflow:\n";
	}

	// Save the old parameters and call liveness
	StatementList oldParameters(parameters);
	std::map<CallStatement*, UseCollector> callLiveness;
	BasicBlock::rtlrit rrit; StatementList::reverse_iterator srit;
	BB_IT it;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
		CallStatement* c = (CallStatement*) bb->getLastStmt(rrit, srit);
		// Note: we may have removed some statements, so there may no longer be a last statement!
		if (c == NULL || !c->isCall()) continue;
		UserProc* dest = (UserProc*)c->getDestProc();
		// Not interested in unanalysed indirect calls (not sure) or calls to lib procs
		if (dest == NULL || dest->isLib()) continue;
		callLiveness[c].makeCloneOf(*c->getUseCollector());
	}

	// Have to redo dataflow to get the liveness at the calls correct
	updateBlockVars();

	remUnusedStmtEtc();				// Also redoes parameters

	// Have the parameters changed? If so, then all callers will need to update their arguments, and do similar
	// analysis to the removal of returns
	findFinalParameters();
	if (parameters.size() != oldParameters.size()) {
		if (DEBUG_UNUSED)
			LOG << "%%%  parameters changed for " << getName() << "\n";
		std::set<CallStatement*>& callers = getCallers();
		std::set<CallStatement*>::iterator cc;
		std::set<UserProc*> callerProcs;
		std::set<UserProc*>::iterator pp;
		for (cc = callers.begin(); cc != callers.end(); ++cc) {
			(*cc)->updateDefines();
			// To prevent duplication, insert into this set, but do the updates immediately
			callerProcs.insert((*cc)->getProc());
		}
		for (pp = callerProcs.begin(); pp != callerProcs.end(); ++pp)
			(*pp)->updateForUseChange(removeRetSet);
	}
	// Check if the liveness of any calls has changed
	std::map<CallStatement*, UseCollector>::iterator ll;
	for (ll = callLiveness.begin(); ll != callLiveness.end(); ++ll) {
		CallStatement* call = ll->first;
		UseCollector& oldLiveness = ll->second;
		UseCollector& newLiveness = *call->getUseCollector();
		if (!(newLiveness == oldLiveness)) {
			if (DEBUG_UNUSED)
				LOG << "%%%  liveness for call to " << call->getDestProc()->getName() << " in " << getName() <<
					" changed\n";
			removeRetSet.insert((UserProc*)call->getDestProc());
		}
	}
}

void UserProc::clearUses() {
	if (VERBOSE)
		LOG << "### clearing usage for " << getName() << " ###\n";
	col.clear();
	BB_IT it;
	BasicBlock::rtlrit rrit; StatementList::reverse_iterator srit;
	for (it = cfg->begin(); it != cfg->end(); ++it) {
		CallStatement* c = (CallStatement*) (*it)->getLastStmt(rrit, srit);
		// Note: we may have removed some statements, so there may no longer be a last statement!
		if (c == NULL || !c->isCall()) continue;
		c->clearUseCollector();
	}
}

void UserProc::typeAnalysis() {
	if (VERBOSE)
		LOG << "### type analysis for " << getName() << " ###\n";

	// Data flow based type analysis
	// Want to be after all propagation, but before converting expressions to locals etc
	if (DFA_TYPE_ANALYSIS) {
		if (VERBOSE || DEBUG_TA)
			LOG << "--- start data flow based type analysis for " << getName() << " ---\n";

		// Now we need to add the implicit assignments. Doing this earlier is extremely problematic, because
		// of all the m[...] that change their sorting order as their arguments get subscripted or propagated into
		addImplicitAssigns();

		bool first = true;
		do {
			if (!first)
				propagateAtDepth(maxDepth);		// HACK: Can sometimes be needed, if call was indirect
												// FIXME: Check if still needed
			first = false;
			dfaTypeAnalysis();
		} while (ellipsisProcessing());
		if (VERBOSE || DEBUG_TA)
			LOG << "=== end type analysis for " << getName() << " ===\n";
	}

	else if (CON_TYPE_ANALYSIS) {
		// FIXME: if we want to do comparison
	}

	else {
		// Need to map the locals somewhere; usually TA does this
		mapExpressionsToLocals();
	}

	printXML();
}

// Copy the RTLs for the already decoded Indirect Control Transfer instructions
RTL* globalRtl = 0;
void UserProc::copyDecodedICTs() {
	BB_IT it;
	BasicBlock::rtlrit rrit; StatementList::reverse_iterator srit;
	for (PBB bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
		Statement* last = bb->getLastStmt(rrit, srit);
		if (last == NULL) continue;			// e.g. a BB with just a NOP in it
		if (!last->isHL_ICT()) continue;
		RTL* rtl = bb->getLastRtl();
		if (DEBUG_SWITCH)
			LOG << "Saving high level switch statement " << rtl << "\n";
		prog->addDecodedRtl(bb->getHiAddr(), rtl);
	}
}

// Find or insert a new implicit reference just before statement s, for address expression a with type t.
// Meet types if necessary
void UserProc::setImplicitRef(Statement* s, Exp* a, Type* ty) {
	PBB bb = s->getBB();			// Get s' enclosing BB
	std::list<RTL*> *rtls = bb->getRTLs();
	for (std::list<RTL*>::iterator rit = rtls->begin(); rit != rtls->end(); rit++) {
		std::list<Statement*>& stmts = (*rit)->getList();
		RTL::iterator it, itForS;
		RTL* rtlForS;
		for (it = stmts.begin(); it != stmts.end(); it++) {
			if (*it == s ||
					// Not the searched for statement. But if it is a call or return statement, it will be the last, and
					// s must be a substatement (e.g. argument, return, define, etc).
					((*it)->isCall() || (*it)->isReturn())) {
				// Found s. Search preceeding statements for an implicit reference with address a
				itForS = it;
				rtlForS = *rit;
				bool found = false;
				bool searchEarlierRtls = true;
				while (it != stmts.begin()) {
					ImpRefStatement* irs = (ImpRefStatement*) *--it;
					if (!irs->isImpRef()) {
						searchEarlierRtls = false;
						break;
					}
					if (*irs->getAddressExp() == *a) {
						found = true;
						searchEarlierRtls = false;
						break;
					}
				}
				while (searchEarlierRtls && rit != rtls->begin()) {
					for (std::list<RTL*>::reverse_iterator revit = rtls->rbegin(); revit != rtls->rend(); ++revit) {
						std::list<Statement*>& stmts2 = (*revit)->getList();
						it = stmts2.end();
						while (it != stmts2.begin()) {
							ImpRefStatement* irs = (ImpRefStatement*) *--it;
							if (!irs->isImpRef()) {
								searchEarlierRtls = false;
								break;
							}
							if (*irs->getAddressExp() == *a) {
								found = true;
								searchEarlierRtls = false;
								break;
							}
						}
					}
				}
				if (found) {
					ImpRefStatement* irs = (ImpRefStatement*)*it;
					bool ch;
					irs->meetWith(ty, ch);
				} else {
					ImpRefStatement* irs = new ImpRefStatement(ty, a);
					rtlForS->insertStmt(irs, itForS);
				}
				return;
			}
		}
	}
	assert(0);				// Could not find s withing its enclosing BB
}

