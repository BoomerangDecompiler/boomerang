/*
 * Copyright (C) 2005, Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:	   dataflow.cpp
 * OVERVIEW:   Implementation of the DataFlow class
 *============================================================================*/

/*
 * $Revision$	// 1.43.2.24
 * 15 Mar 05 - Mike: Separated from cfg.cpp
 */

#include <sstream>

#include "dataflow.h"
#include "cfg.h"
#include "proc.h"
#include "exp.h"
#include "boomerang.h"
#include "visitor.h"
#include "log.h"
#include "frontend.h"

extern char debug_buffer[];		 // For prints functions


/*
 * Dominator frontier code largely as per Appel 2002 ("Modern Compiler Implementation in Java")
 */

void DataFlow::DFS(int p, int n) {
	if (dfnum[n] == 0) {
		dfnum[n] = N; vertex[N] = n; parent[n] = p;
		N++;
		// For each successor w of n
		PBB bb = BBs[n];
		std::vector<PBB>& outEdges = bb->getOutEdges();
		std::vector<PBB>::iterator oo;
		for (oo = outEdges.begin(); oo != outEdges.end(); oo++) {
			DFS(n, indices[*oo]);
		}
	}
}

void DataFlow::dominators(Cfg* cfg) {
	PBB r = cfg->getEntryBB();
	unsigned numBB = cfg->getNumBBs();
	BBs.resize(numBB, (PBB)-1);
	N = 0; BBs[0] = r;
	indices.clear();			// In case restart decompilation due to switch statements
	indices[r] = 0;
	// Initialise to "none"
	dfnum.resize(numBB, 0);
	semi.resize(numBB, -1);
	ancestor.resize(numBB, -1);
	idom.resize(numBB, -1);
	samedom.resize(numBB, -1);
	vertex.resize(numBB, -1);
	parent.resize(numBB, -1);
	best.resize(numBB, -1);
	bucket.resize(numBB);
	DF.resize(numBB);
	// Set up the BBs and indices vectors. Do this here because sometimes a BB can be unreachable (so relying on
	// in-edges doesn't work)
	std::list<PBB>::iterator ii;
	int idx = 1;
	for (ii = cfg->begin(); ii != cfg->end(); ii++) {
		PBB bb = *ii;
		if (bb != r) {	   // Entry BB r already done
			indices[bb] = idx;
			BBs[idx++] = bb;
		}
	}
	DFS(-1, 0);
	int i;
	for (i=N-1; i >= 1; i--) {
		int n = vertex[i]; int p = parent[n]; int s = p;
		/* These lines calculate the semi-dominator of n, based on the Semidominator Theorem */
		// for each predecessor v of n
		PBB bb = BBs[n];
		std::vector<PBB>& inEdges = bb->getInEdges();
		std::vector<PBB>::iterator it;
		for (it = inEdges.begin(); it != inEdges.end(); it++) {
			if (indices.find(*it) == indices.end()) {
				std::cerr << "BB not in indices: "; (*it)->print(std::cerr);
				assert(false);
			}
			int v = indices[*it];
			int sdash;
			if (dfnum[v] <= dfnum[n])
				sdash = v;
			else sdash = semi[ancestorWithLowestSemi(v)];
			if (dfnum[sdash] < dfnum[s])
				s = sdash;
		}
		semi[n] = s;
		/* Calculation of n'd dominator is deferred until the path from s to n has been linked into the forest */
		bucket[s].insert(n);
		Link(p, n);
		// for each v in bucket[p]
		std::set<int>::iterator jj;
		for (jj=bucket[p].begin(); jj != bucket[p].end(); jj++) {
			int v = *jj;
			/* Now that the path from p to v has been linked into the spanning forest, these lines calculate the
				dominator of v, based on the first clause of the Dominator Theorem, or else defer the calculation until
				y's dominator is known. */
			int y = ancestorWithLowestSemi(v);
			if (semi[y] == semi[v])
				idom[v] = p;		 		// Success!
			else samedom[v] = y;	 		// Defer
		}
		bucket[p].clear();
	}
	for (i=1; i < N-1; i++) {
		/* Now all the deferred dominator calculations, based on the second clause of the Dominator Theorem, are
			performed. */
		int n = vertex[i];
		if (samedom[n] != -1) {
			idom[n] = idom[samedom[n]];		// Deferred success!
		}
	}
	computeDF(0);							// Finally, compute the dominance frontiers
}

// Basically algorithm 9.10b of Appel 2002 (uses path compression for O(log N) amortised time per operation
// (overall O(N log N))
int DataFlow::ancestorWithLowestSemi(int v) {
	int a = ancestor[v];
	if (ancestor[a] != -1) {
		int b = ancestorWithLowestSemi(a);
		ancestor[v] = ancestor[a];
		if (dfnum[semi[b]] < dfnum[semi[best[v]]])
			best[v] = b;
	}
	return best[v];
}

void DataFlow::Link(int p, int n) {
	ancestor[n] = p; best[n] = n;
}

// Return true if n dominates w
bool DataFlow::doesDominate(int n, int w) {
	while (idom[w] != -1) {
		if (idom[w] == n)
			return true;
		w = idom[w];	 // Move up the dominator tree
	}
	return false;
}

void DataFlow::computeDF(int n) {
	std::set<int> S;
	/* THis loop computes DF_local[n] */
	// for each node y in succ(n)
	PBB bb = BBs[n];
	std::vector<PBB>& outEdges = bb->getOutEdges();
	std::vector<PBB>::iterator it;
	for (it = outEdges.begin(); it != outEdges.end(); it++) {
		int y = indices[*it];
		if (idom[y] != n)
			S.insert(y);
	}
	// for each child c of n in the dominator tree
	// Note: this is a linear search!
	int sz = ancestor.size();
	for (int c = 0; c < sz; c++) {
		if (idom[c] != n) continue;
		computeDF(c);
		/* This loop computes DF_up[c] */
		// for each element w of DF[c]
		std::set<int>& s = DF[c];
		std::set<int>::iterator ww;
		for (ww = s.begin(); ww != s.end(); ww++) {
			int w = *ww;
			// if n does not dominate w, or if n = w
			if (n == w || !doesDominate(n, w)) {
				S.insert(w);
			}
		}
	}
	DF[n] = S;
}	// end computeDF


bool DataFlow::canRename(Exp* e, UserProc* proc) {
	if (renameAllMemofs)		// When safe,
		return true;			//  allow memofs to be renamed
	if (proc->isLocalOrParam(e)) return true;	// But do rename memofs that are locals or params
	return e->canRename();		// Currently <=> return !e->isMemof();
}

// For debugging
void DataFlow::dumpA_phi() {
	std::map<Exp*, std::set<int>, lessExpStar>::iterator zz;
	std::cerr << "A_phi:\n";
	for (zz = A_phi.begin(); zz != A_phi.end(); ++zz) {
		std::cerr << zz->first << " -> ";
		std::set<int>& si = zz->second;
		std::set<int>::iterator qq;
		for (qq = si.begin(); qq != si.end(); ++qq)
			std::cerr << *qq << ", ";
		std::cerr << "\n";
	}
	std::cerr << "end A_phi\n";
}


bool DataFlow::placePhiFunctions(UserProc* proc) {
	// First free some memory no longer needed
	dfnum.resize(0);
	semi.resize(0);
	ancestor.resize(0);
	samedom.resize(0);
	vertex.resize(0);
	parent.resize(0);
	best.resize(0);
	bucket.resize(0);
	defsites.clear();			// Clear defsites map,
	defallsites.clear();
	A_orig.clear();				// and A_orig,
	defStmts.clear();			// and the map from variable to defining Stmt 

	bool change = false;

	// Set the sizes of needed vectors
	unsigned numBB = indices.size();
	Cfg* cfg = proc->getCFG();
	assert(numBB == cfg->getNumBBs());
	A_orig.resize(numBB);

	// We need to create A_orig for the current memory depth
	unsigned n;
	for (n=0; n < numBB; n++) {
		BasicBlock::rtlit rit; StatementList::iterator sit;
		PBB bb = BBs[n];
		for (Statement* s = bb->getFirstStmt(rit, sit); s; s = bb->getNextStmt(rit, sit)) {
			LocationSet ls;
			LocationSet::iterator it;
			s->getDefinitions(ls);
			if (s->isCall() && ((CallStatement*)s)->isChildless())		// If this is a childless call
				defallsites.insert(n);									// then this block defines every variable
			for (it = ls.begin(); it != ls.end(); it++) {
				if (canRename(*it, proc)) {
					A_orig[n].insert((*it)->clone());
					defStmts[*it] = s;
				}
			}
		}
	}

	// For each node n
	for (n=0; n < numBB; n++) {
		// For each variable a in A_orig[n]
		std::set<Exp*, lessExpStar>& s = A_orig[n];
		std::set<Exp*, lessExpStar>::iterator aa;
		for (aa = s.begin(); aa != s.end(); aa++) {
			Exp* a = *aa;
			defsites[a].insert(n);
		}
	}

	// For each variable a (in defsites, i.e. defined anywhere)
	std::map<Exp*, std::set<int>, lessExpStar>::iterator mm;
	for (mm = defsites.begin(); mm != defsites.end(); mm++) {
		Exp* a = (*mm).first;				// *mm is pair<Exp*, set<int>>

		// Special processing for define-alls
		// for each n in defallsites
		std::set<int>::iterator da;
		for (da = defallsites.begin(); da != defallsites.end(); ++da)
			defsites[a].insert(*da);

		// W <- defsites[a];
		std::set<int> W = defsites[a];		// set copy
		// While W not empty
		while (W.size()) {
			// Remove some node n from W
			int n = *W.begin();				// Copy first element
			W.erase(W.begin());				// Remove first element
			// for each y in DF[n]
			std::set<int>::iterator yy;
			std::set<int>& DFn = DF[n];
			for (yy = DFn.begin(); yy != DFn.end(); yy++) {
				int y = *yy;
				// if y not element of A_phi[a]
				std::set<int>& s = A_phi[a];
				if (s.find(y) == s.end()) {
					// Insert trivial phi function for a at top of block y: a := phi()
					change = true;
					Statement* as = new PhiAssign(a->clone());
					PBB Ybb = BBs[y];
					Ybb->prependStmt(as, proc);
					// A_phi[a] <- A_phi[a] U {y}
					s.insert(y);
					// if a !elementof A_orig[y]
					if (A_orig[y].find(a) == A_orig[y].end()) {
						// W <- W U {y}
						W.insert(y);
					}
				}
			}
		}
	}
	return change;
}		// end placePhiFunctions



static Exp* defineAll = new Terminal(opDefineAll);		// An expression representing <all>

// There is an entry in stacks[defineAll] that represents the latest definition from a define-all source. It is needed
// for variables that don't have a definition as yet (i.e. stacks[x].empty() is true). As soon as a real definition to
// x appears, stacks[defineAll] does not apply for variable x. This is needed to get correct operation of the use
// collectors in calls.

// Care with the Stacks object (a map from expression to stack); using Stacks[q].empty() can needlessly insert an empty
// stack
#define STACKS_EMPTY(q) (Stacks.find(q) == Stacks.end() || Stacks[q].empty())

// Subscript dataflow variables
bool DataFlow::renameBlockVars(UserProc* proc, int n, bool clearStacks /* = false */ ) {
	bool changed = false;

	// Need to clear the Stacks of old, renamed locations like m[esp-4] (these will be deleted, and will cause compare
	// failures in the Stacks, so it can't be correctly ordered and hence balanced etc, and will lead to segfaults)
	if (clearStacks) Stacks.clear();

	// For each statement S in block n
	BasicBlock::rtlit rit; StatementList::iterator sit;
	PBB bb = BBs[n];
	Statement* S;
	for (S = bb->getFirstStmt(rit, sit); S; S = bb->getNextStmt(rit, sit)) {
		// if S is not a phi function (per Appel)
		/* if (!S->isPhi()) */ {
			// For each use of some variable x in S (not just assignments)
			LocationSet locs;
			if (S->isPhi()) {
				PhiAssign* pa = (PhiAssign*)S;
				Exp* phiLeft = pa->getLeft();
				if (phiLeft->isMemOf() || phiLeft->isRegOf())
					phiLeft->getSubExp1()->addUsedLocs(locs);
				// A phi statement may use a location defined in a childless call, in which case its use collector
				// needs updating
				PhiAssign::iterator pp;
				for (pp = pa->begin(); pp != pa->end(); ++pp) {
					Statement* def = pp->def;
					if (def && def->isCall())
						((CallStatement*)def)->useBeforeDefine(phiLeft->clone());
				}
			}
			else {				// Not a phi assignment
				S->addUsedLocs(locs);
			}
			LocationSet::iterator xx;
			for (xx = locs.begin(); xx != locs.end(); xx++) {
				Exp* x = *xx;
				// Don't rename memOfs that are not renamable according to the current policy
				if (!canRename(x, proc)) continue;
				Statement* def = NULL;
				if (x->isSubscript()) {					// Already subscripted?
					// No renaming required, but redo the usage analysis, in case this is a new return, and also because
					// we may have just removed all call livenesses
					// Update use information in calls, and in the proc (for parameters)
					Exp* base = ((RefExp*)x)->getSubExp1();
					def = ((RefExp*)x)->getDef();
					if (def && def->isCall()) {
						// Calls have UseCollectors for locations that are used before definition at the call
						((CallStatement*)def)->useBeforeDefine(base->clone());
						continue;
					}
					// Update use collector in the proc (for parameters)
					if (def == NULL)
						proc->useBeforeDefine(base->clone());
					continue;							// Don't re-rename the renamed variable
				}
				// Else x is not subscripted yet
				if (STACKS_EMPTY(x)) {
					if (!Stacks[defineAll].empty())
						def = Stacks[defineAll].top();
					else {
						// If the both stacks are empty, use a NULL definition. This will be changed into a pointer
						// to an implicit definition at the start of type analysis, but not until all the m[...]
						// have stopped changing their expressions (complicates implicit assignments considerably).
						def = NULL;
						// Update the collector at the start of the UserProc
						proc->useBeforeDefine(x->clone());
					}
				}
				else
					def = Stacks[x].top();
				if (def && def->isCall())
					// Calls have UseCollectors for locations that are used before definition at the call
					((CallStatement*)def)->useBeforeDefine(x->clone());
				// Replace the use of x with x{def} in S
				changed = true;
				if (S->isPhi()) {
					Exp* phiLeft = ((PhiAssign*)S)->getLeft();
					phiLeft->setSubExp1(phiLeft->getSubExp1()->expSubscriptVar(x, def /*, this*/));
				} else {
					S->subscriptVar(x, def /*, this */);
				}
			}
		}

		// MVE: Check for Call and Return Statements; these have DefCollector objects that need to be updated
		// Do before the below, so CallStatements have not yet processed their defines
		if (S->isCall() || S->isReturn()) {
			DefCollector* col;
			if (S->isCall())
				col = ((CallStatement*)S)->getDefCollector();
			else
				col = ((ReturnStatement*)S)->getCollector();
			col->updateDefs(Stacks, proc);
		}

		// For each definition of some variable a in S
		LocationSet defs;
		S->getDefinitions(defs);
		LocationSet::iterator dd;
		for (dd = defs.begin(); dd != defs.end(); dd++) {
			Exp *a = *dd;
			// Don't consider a if it cannot be propagated
			bool suitable = canRename(a, proc);
			if (suitable) {
				// Push i onto Stacks[a]
				// Note: we clone a because otherwise it could be an expression that gets deleted through various
				// modifications. This is necessary because we do several passes of this algorithm to sort out the
				// memory expressions
				Stacks[a->clone()].push(S);
				// Replace definition of a with definition of a_i in S (we don't do this)
			}
			// FIXME: MVE: do we need this awful hack?
			if (a->getOper() == opLocal) {
				Exp *a1 = S->getProc()->expFromSymbol(((Const*)a->getSubExp1())->getStr());
				assert(a1);
				a = a1;
				// Stacks already has a definition for a (as just the bare local)
				if (suitable) {
					Stacks[a->clone()].push(S);
				}
			}
		}
		// Special processing for define-alls (presently, only childless calls).
// But note that only everythings at the current memory level are defined!
		if (S->isCall() && ((CallStatement*)S)->isChildless() && !Boomerang::get()->assumeABI) {
			// S is a childless call (and we're not assuming ABI compliance)
			Stacks[defineAll];										// Ensure that there is an entry for defineAll
			std::map<Exp*, std::stack<Statement*>, lessExpStar>::iterator dd;
			for (dd = Stacks.begin(); dd != Stacks.end(); ++dd) {
// if (dd->first->isMemDepth(memDepth))
					dd->second.push(S);								// Add a definition for all vars
			}
		}
	}

	// For each successor Y of block n
	std::vector<PBB>& outEdges = bb->getOutEdges();
	unsigned numSucc = outEdges.size();
	for (unsigned succ = 0; succ < numSucc; succ++) {
		PBB Ybb = outEdges[succ];
		// Suppose n is the jth predecessor of Y
		int j = Ybb->whichPred(bb);
		// For each phi-function in Y
		Statement* S;
		for (S = Ybb->getFirstStmt(rit, sit); S; S = Ybb->getNextStmt(rit, sit)) {
			PhiAssign* pa = dynamic_cast<PhiAssign*>(S);
			// if S is not a phi function, then quit the loop (no more phi's)
			// Wrong: do not quit the loop: there's an optimisation that turns a PhiAssign into an ordinary Assign.
			// So continue, not break.
			if (!pa) continue;
			// Suppose the jth operand of the phi is a
			// For now, just get the LHS
			Exp* a = pa->getLeft();
			// Only consider variables that can be renamed
			if (!canRename(a, proc)) continue;
			Statement* def;
			if (STACKS_EMPTY(a))
				def = NULL;				// No reaching definition
			else
				def = Stacks[a].top();
			// "Replace jth operand with a_i"
			pa->putAt(j, def, a);
		}
	}

	// For each child X of n
	// Note: linear search!
	unsigned numBB = proc->getCFG()->getNumBBs();
	for (unsigned X=0; X < numBB; X++) {
		if (idom[X] == n)
			renameBlockVars(proc, X);
	}

	// For each statement S in block n
	// NOTE: Because of the need to pop childless calls from the Stacks, it is important in my algorithm to process the
	// statments in the BB *backwards*. (It is not important in Appel's algorithm, since he always pushes a definition
	// for every variable defined on the Stacks).
	BasicBlock::rtlrit rrit; StatementList::reverse_iterator srit;
	for (S = bb->getLastStmt(rrit, srit); S; S = bb->getPrevStmt(rrit, srit)) {
		// For each definition of some variable a in S
		LocationSet defs;
		S->getDefinitions(defs);
		LocationSet::iterator dd;
		for (dd = defs.begin(); dd != defs.end(); dd++) {
			if (canRename(*dd, proc)) {
				// if ((*dd)->getMemDepth() == memDepth)
				std::map<Exp*, std::stack<Statement*>, lessExpStar>::iterator ss = Stacks.find(*dd);
				if (ss == Stacks.end()) {
					std::cerr << "Tried to pop " << *dd << " from Stacks; does not exist\n";
 					assert(0);
				}
					ss->second.pop();
			}
		}
		// Pop all defs due to childless calls
		if (S->isCall() && ((CallStatement*)S)->isChildless()) {
			std::map<Exp*, std::stack<Statement*>, lessExpStar>::iterator sss;
			for (sss = Stacks.begin(); sss != Stacks.end(); ++sss) {
				if (!sss->second.empty() && sss->second.top() == S) {
					sss->second.pop();
				}
			}
		}
	}
	return changed;
}

void DataFlow::dumpStacks() {
	std::cerr << "Stacks: " << Stacks.size() << " entries\n";
	std::map<Exp*, std::stack<Statement*>, lessExpStar>::iterator zz;
	for (zz = Stacks.begin(); zz != Stacks.end(); zz++) {
		std::cerr << "Var " << zz->first << " [ ";
		std::stack<Statement*>tt = zz->second;               // Copy the stack!
		while (!tt.empty()) {
			std::cerr << tt.top()->getNumber() << " "; tt.pop();
		}
		std::cerr << "]\n";
	}
}

void DataFlow::dumpDefsites() {
	std::map<Exp*, std::set<int>, lessExpStar>::iterator dd;
	for (dd = defsites.begin(); dd != defsites.end(); ++dd) {
		std::cerr << dd->first;
		std::set<int>::iterator ii;
		std::set<int>& si = dd->second;
		for (ii = si.begin(); ii != si.end(); ++ii)
			std::cerr << " " << *ii;
		std::cerr << "\n";
	}
}

void DataFlow::dumpA_orig() {
	int n = A_orig.size();
	for (int i=0; i < n; ++i) {
		std::cerr << i;
		std::set<Exp*, lessExpStar>::iterator ee;
		std::set<Exp*, lessExpStar>& se = A_orig[i];
		for (ee = se.begin(); ee != se.end(); ++ee)
			std::cerr << " " << *ee;
		std::cerr << "\n";
	}
}

void DefCollector::updateDefs(std::map<Exp*, std::stack<Statement*>, lessExpStar>& Stacks, UserProc* proc) {
	std::map<Exp*, std::stack<Statement*>, lessExpStar>::iterator it;
	for (it = Stacks.begin(); it != Stacks.end(); it++) {
		if (it->second.size() == 0)
			continue;					// This variable's definition doesn't reach here
		// Create an assignment of the form loc := loc{def}
		RefExp* re = new RefExp(it->first->clone(), it->second.top());
		Assign* as = new Assign(it->first->clone(), re);
		as->setProc(proc);				// Simplify sometimes needs this
		insert(as);
	}
	initialised = true;
}

// Find the definition for e that reaches this Collector. If none reaches here, return NULL
Exp* DefCollector::findDefFor(Exp* e) {
	iterator it;
	for (it = defs.begin(); it != defs.end(); ++it) {
		Exp* lhs = (*it)->getLeft();
		if (*lhs == *e)
			return (*it)->getRight();
	}
	return NULL;					// Not explicitly defined here
}

void UseCollector::print(std::ostream& os, bool html) {
	LocationSet::iterator it;
	bool first = true;
	for (it=locs.begin(); it != locs.end(); ++it) {
		if (first)
			first = false;
		else
			os << ",  ";
		(*it)->print(os, html);
	}
}

#define DEFCOL_COLS 120
void DefCollector::print(std::ostream& os, bool html) {
	iterator it;
	unsigned col = 36;
	bool first = true;
	for (it=defs.begin(); it != defs.end(); ++it) {
		std::ostringstream ost;
		(*it)->getLeft()->print(ost, html);
		ost << "=";
		(*it)->getRight()->print(ost, html);
		unsigned len = ost.str().length();
		if (first)
			first = false;
		else if (col+4+len >= DEFCOL_COLS) {		// 4 for a comma and three spaces
			if (col != DEFCOL_COLS-1) os << ",";	// Comma at end of line
			os << "\n                ";
			col = 16;
		} else {
			os << ",   ";
			col += 4;
		}
		os << ost.str().c_str();
		col += len;
	}
}

char* UseCollector::prints() {
	std::ostringstream ost;
	print(ost);
	strncpy(debug_buffer, ost.str().c_str(), DEBUG_BUFSIZE-1);
	debug_buffer[DEBUG_BUFSIZE-1] = '\0';
	return debug_buffer;
}

char* DefCollector::prints() {
	std::ostringstream ost;
	print(ost);
	strncpy(debug_buffer, ost.str().c_str(), DEBUG_BUFSIZE-1);
	debug_buffer[DEBUG_BUFSIZE-1] = '\0';
	return debug_buffer;
}

void UseCollector::dump() {
	std::ostringstream ost;
	print(ost);
	std::cerr << ost.str();
}

void DefCollector::dump() {
	std::ostringstream ost;
	print(ost);
	std::cerr << ost.str();
}

void UseCollector::makeCloneOf(UseCollector& other) {
	initialised = other.initialised;
	locs.clear();
	for (iterator it = other.begin(); it != other.end(); ++it)
		locs.insert((*it)->clone());
}

void DefCollector::makeCloneOf(DefCollector& other) {
	initialised = other.initialised;
	defs.clear();
	for (iterator it = other.begin(); it != other.end(); ++it)
		defs.insert((Assign*)(*it)->clone());
}

void DefCollector::searchReplaceAll(Exp* from, Exp* to, bool& change) {
	iterator it;
	for (it=defs.begin(); it != defs.end(); ++it)
		(*it)->searchAndReplace(from, to);
}

// Called from CallStatement::fromSSAform
void UseCollector::fromSSAform(igraph& ig, Statement* def) {
	LocationSet removes, inserts;
	iterator it;
	for (it = locs.begin(); it != locs.end(); ++it) {
		RefExp* ref = new RefExp(*it, def);			// Wrap it in a def
		Exp* ret = ref->fromSSA(ig);
		// If there is no change, ret will equal *it again (i.e. fromSSAform just removed the subscript)
		if (ret != *it) {							// Pointer comparison
			// There was a change; we want to replace *it with ret
			removes.insert(*it);
			inserts.insert(ret);
		}
	}
	for (it = removes.begin(); it != removes.end(); ++it)
		locs.remove(*it);
	for (it = inserts.begin(); it != inserts.end(); ++it)
		locs.insert(*it);
}

bool UseCollector::operator==(UseCollector& other) {
	if (other.initialised != initialised) return false;
	iterator it1, it2;
	if (other.locs.size() != locs.size()) return false;
	for (it1 = locs.begin(), it2 = other.locs.begin(); it1 != locs.end(); ++it1, ++it2)
		if (!(**it1 == **it2)) return false;
	return true;
}

void DefCollector::insert(Assign* a) {
	Exp* l = a->getLeft();
	if (existsOnLeft(l)) return;
	defs.insert(a);
}

void DataFlow::convertImplicits(Cfg* cfg) {
	// Convert statements in A_phi from m[...]{-} to m[...]{0}
	std::map<Exp*, std::set<int>, lessExpStar> A_phi_copy = A_phi;			// Object copy
	std::map<Exp*, std::set<int>, lessExpStar>::iterator it;
	ImplicitConverter ic(cfg);
	A_phi.clear();
	for (it = A_phi_copy.begin(); it != A_phi_copy.end(); ++it) {
		Exp* e = it->first->clone();
		e = e->accept(&ic);
		A_phi[e] = it->second;					// Copy the set (doesn't have to be deep)
	}

	std::map<Exp*, std::set<int>, lessExpStar > defsites_copy = defsites;	// Object copy
	std::map<Exp*, std::set<int>, lessExpStar >::iterator dd;
	defsites.clear();
	for (dd = A_phi_copy.begin(); dd != A_phi_copy.end(); ++dd) {
		Exp* e = dd->first->clone();
		e = e->accept(&ic);
		defsites[e] = dd->second;				// Copy the set (doesn't have to be deep)
	}

	std::vector<std::set<Exp*, lessExpStar> > A_orig_copy;
	std::vector<std::set<Exp*, lessExpStar> >::iterator oo;
	A_orig.clear();
	for (oo = A_orig_copy.begin(); oo != A_orig_copy.end(); ++oo) {
		std::set<Exp*, lessExpStar>& se = *oo;
		std::set<Exp*, lessExpStar> se_new;
		std::set<Exp*, lessExpStar>::iterator ee;
		for (ee = se.begin(); ee != se.end(); ++ee) {
			Exp* e = (*ee)->clone();
			e = e->accept(&ic);
			se_new.insert(e);
		}
		A_orig.insert(A_orig.end(), se_new);	// Copy the set (doesn't have to be a deep copy)
	}

}
