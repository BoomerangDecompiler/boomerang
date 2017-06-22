/*
 * Copyright (C) 2002-2006, Trent Waddington and Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
 * \file       statement.cpp
 * \brief   Implementation of the Statement and related classes.
 ******************************************************************************/

#include "util/Log.h"

#include "db/exp.h"
#include "db/cfg.h"
#include "db/proc.h"
#include "db/prog.h"
#include "db/basicblock.h"
#include "db/rtl.h" // For debugging code
#include "db/signature.h"
#include "db/dataflow.h"
#include "db/statements/junctionstatement.h"
#include "db/statements/callstatement.h"
#include "db/statements/phiassign.h"
#include "db/statements/imprefstatement.h"
#include "db/statements/implicitassign.h"
#include "db/statements/branchstatement.h"
#include "db/statements/casestatement.h"
#include "db/statements/boolassign.h"

#include "codegen/ICodeGenerator.h"
#include "util/Util.h"
#include "db/visitor.h"


#include <cassert>
#include <cstring>
#include <sstream>
#include <cstddef>
#include <algorithm>

extern char         debug_buffer[]; // For prints functions
extern QTextStream& alignStream(QTextStream& str, int align);


void Instruction::setProc(UserProc *p)
{
	m_proc = p;
	LocationSet exps, defs;
	addUsedLocs(exps);
	getDefinitions(defs);
	exps.makeUnion(defs);

	for (auto ll = exps.begin(); ll != exps.end(); ll++) {
		auto l = std::dynamic_pointer_cast<Location>(*ll);

		if (l) {
			l->setProc(p);
		}
	}
}


bool Instruction::mayAlias(SharedExp e1, SharedExp e2, int size) const
{
	if (*e1 == *e2) {
		return true;
	}

	// Pass the expressions both ways. Saves checking things like m[exp] vs m[exp+K] and m[exp+K] vs m[exp] explicitly
	// (only need to check one of these cases)
	bool b = (calcMayAlias(e1, e2, size) && calcMayAlias(e2, e1, size));

	if (b && VERBOSE) {
		LOG << "May alias: " << e1 << " and " << e2 << " size " << size << "\n";
	}

	return b;
}


bool Instruction::calcMayAlias(SharedExp e1, SharedExp e2, int size) const
{
	// currently only considers memory aliasing..
	if (!e1->isMemOf() || !e2->isMemOf()) {
		return false;
	}

	SharedExp e1a = e1->getSubExp1();
	SharedExp e2a = e2->getSubExp1();

	// constant memory accesses
	if (e1a->isIntConst() && e2a->isIntConst()) {
		ADDRESS   a1   = e1a->access<Const>()->getAddr();
		ADDRESS   a2   = e2a->access<Const>()->getAddr();
		ptrdiff_t diff = (a1 - a2).m_value;

		if (diff < 0) {
			diff = -diff;
		}

		if (diff * 8 >= size) {
			return false;
		}
	}

	// same left op constant memory accesses
	if ((e1a->getArity() == 2) && (e1a->getOper() == e2a->getOper()) && e1a->getSubExp2()->isIntConst() &&
		e2a->getSubExp2()->isIntConst() && (*e1a->getSubExp1() == *e2a->getSubExp1())) {
		int i1   = e1a->access<Const, 2>()->getInt();
		int i2   = e2a->access<Const, 2>()->getInt();
		int diff = i1 - i2;

		if (diff < 0) {
			diff = -diff;
		}

		if (diff * 8 >= size) {
			return false;
		}
	}

	// [left] vs [left +/- constant] memory accesses
	if (((e2a->getOper() == opPlus) || (e2a->getOper() == opMinus)) && (*e1a == *e2a->getSubExp1()) &&
		e2a->getSubExp2()->isIntConst()) {
		int i1   = 0;
		int i2   = e2a->access<Const, 2>()->getInt();
		int diff = i1 - i2;

		if (diff < 0) {
			diff = -diff;
		}

		if (diff * 8 >= size) {
			return false;
		}
	}

	// Don't need [left +/- constant ] vs [left] because called twice with
	// args reversed
	return true;
}


bool Instruction::isFirstStatementInBB() const
{
	assert(m_parent);
	assert(m_parent->getRTLs());
	assert(m_parent->getRTLs()->size());
	assert(m_parent->getRTLs()->front());
	assert(m_parent->getRTLs()->front()->size());
	return this == m_parent->getRTLs()->front()->front();
}


bool Instruction::isLastStatementInBB() const
{
	assert(m_parent);
	return this == m_parent->getLastStmt();
}


Instruction *Instruction::getPreviousStatementInBB() const
{
	assert(m_parent);
	std::list<RTL *> *rtls = m_parent->getRTLs();
	assert(rtls);
	Instruction *previous = nullptr;

	for (auto rtl : *rtls) {
		for (Instruction *it : *rtl) {
			if (it == this) {
				return previous;
			}

			previous = it;
		}
	}

	return nullptr;
}


Instruction *Instruction::getNextStatementInBB() const
{
	assert(m_parent);
	std::list<RTL *> *rtls = m_parent->getRTLs();
	assert(rtls);
	bool wantNext = false;

	for (auto rtl : *rtls) {
		for (Instruction *it : *rtl) {
			if (wantNext) {
				return it;
			}

			if (it == this) {
				wantNext = true;
			}
		}
	}

	return nullptr;
}


/***************************************************************************/ /**
 * \brief Output operator for Instruction*
 *        Just makes it easier to use e.g. LOG_STREAM() << myStmtStar
 * \param os output stream to send to
 * \param s  ptr to Statement to print to the stream
 * \returns copy of os (for concatenation)
 ******************************************************************************/
QTextStream& operator<<(QTextStream& os, const Instruction *s)
{
	if (s == nullptr) {
		os << "nullptr ";
		return os;
	}

	s->print(os);
	return os;
}


bool Instruction::isFlagAssgn() const
{
	if (m_kind != STMT_ASSIGN) {
		return false;
	}

	OPER op = ((Assign *)this)->getRight()->getOper();
	return(op == opFlagCall);
}


char *Instruction::prints() const
{
	QString     tgt;
	QTextStream ost(&tgt);

	print(ost);
	strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
	debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
	return debug_buffer;
}


void Instruction::dump() const
{
	QTextStream q_cerr(stderr);

	print(q_cerr);
	q_cerr << "\n";
}


bool Instruction::canPropagateToExp(Exp& e)
{
	if (!e.isSubscript()) {
		return false;
	}

	RefExp& re((RefExp&)e);

	if (re.isImplicitDef()) {
		// Can't propagate statement "-" or "0" (implicit assignments)
		return false;
	}

	Instruction *def = re.getDef();

	//    if (def == this)
	// Don't propagate to self! Can happen with %pc's (?!)
	//        return false;
	if (def->isNullStatement()) {
		// Don't propagate a null statement! Can happen with %pc's (would have no effect, and would infinitely loop)
		return false;
	}

	if (!def->isAssign()) {
		return false; // Only propagate ordinary assignments (so far)
	}

	Assign *adef = (Assign *)def;

	if (adef->getType()->isArray()) {
		// Assigning to an array, don't propagate (Could be alias problems?)
		return false;
	}

	return true;
}


static int propagate_progress = 0;

bool Instruction::propagateTo(bool& convert, std::map<SharedExp, int, lessExpStar> *destCounts /* = nullptr */,
							  LocationSet *usedByDomPhi /* = nullptr */, bool force /* = false */)
{
	if (++propagate_progress > 1000) {
		LOG_STREAM() << 'p';
		LOG_STREAM().flush();
		propagate_progress = 0;
	}

	bool change;
	int  changes = 0;
	// int sp = proc->getSignature()->getStackRegister(proc->getProg());
	// Exp* regSp = Location::regOf(sp);
	int propMaxDepth = Boomerang::get()->propMaxDepth;

	do {
		LocationSet exps;
		// addUsedLocs(..,true) -> true to also add uses from collectors. For example, want to propagate into
		// the reaching definitions of calls. Third parameter defaults to false, to
		// find all locations, not just those inside m[...]
		addUsedLocs(exps, true);
		LocationSet::iterator ll;
		change = false; // True if changed this iteration of the do/while loop

		// Example: m[r24{10}] := r25{20} + m[r26{30}]
		// exps has r24{10}, r25{30}, m[r26{30}], r26{30}
		for (ll = exps.begin(); ll != exps.end(); ll++) {
			SharedExp e = *ll;

			if (!canPropagateToExp(*e)) {
				continue;
			}

			assert(dynamic_cast<Assignment *>(e->access<RefExp>()->getDef()) != nullptr);
			Assignment *def = (Assignment *)(e->access<RefExp>()->getDef());
			SharedExp  rhs  = def->getRight();

			// If force is true, ignore the fact that a memof should not be propagated (for switch analysis)
			if (rhs->containsBadMemof(m_proc) && !(force && rhs->isMemOf())) {
				// Must never propagate unsubscripted memofs, or memofs that don't yet have symbols. You could be
				// propagating past a definition, thereby invalidating the IR
				continue;
			}

			SharedExp lhs = def->getLeft();

			if (EXPERIMENTAL) {
#if 0
				// This is the old "don't propagate x=f(x)" heuristic. Hopefully it will work better now that we always
				// propagate into memofs etc. However, it might need a "and we're inside the right kind of loop"
				// condition
				LocationSet used;
				def->addUsedLocs(used);
				RefExp left(def->getLeft(), (Statement *)-1);
				RefExp *right = dynamic_cast<RefExp *>(def->getRight());

				// Beware of x := x{something else} (because we do want to do copy propagation)
				if (used.exists(&left) && !(right && (*right->getSubExp1() == *left.getSubExp1()))) {
					// We have something like eax = eax + 1
					continue;
				}
#else
				// This is Mike's experimental propagation limiting heuristic. At present, it is:
				// for each component of def->rhs
				//   test if the base expression is in the set usedByDomPhi
				//     if so, check if this statement OW overwrites a parameter (like ebx = ebx-1)
				//     if so, check for propagating past this overwriting statement, i.e.
				//        domNum(def) <= domNum(OW) && dimNum(OW) < domNum(def)
				//        if so, don't propagate (heuristic takes effect)
				if (usedByDomPhi) {
					LocationSet rhsComps;
					rhs->addUsedLocs(rhsComps);
					LocationSet::iterator rcit;
					bool doNotPropagate = false;

					for (rcit = rhsComps.begin(); rcit != rhsComps.end(); ++rcit) {
						if (!(*rcit)->isSubscript()) {
							continue; // Sometimes %pc sneaks in
						}

						SharedExp rhsBase = (*rcit)->getSubExp1();
						// We don't know the statement number for the one definition in usedInDomPhi that might exist,
						// so we use findNS()
						SharedExp OW = usedByDomPhi->findNS(rhsBase);

						if (OW) {
							Instruction *OWdef = OW->access<RefExp>()->getDef();

							if (!OWdef->isAssign()) {
								continue;
							}

							SharedExp   lhsOWdef = ((Assign *)OWdef)->getLeft();
							LocationSet OWcomps;
							def->addUsedLocs(OWcomps);
							LocationSet::iterator cc;
							bool isOverwrite = false;

							for (cc = OWcomps.begin(); cc != OWcomps.end(); ++cc) {
								if (**cc *= *lhsOWdef) {
									isOverwrite = true;
									break;
								}
							}

							if (isOverwrite) {
								// Now check for propagating a component past OWdef
								if ((def->getDomNumber() <= OWdef->getDomNumber()) &&
									(OWdef->getDomNumber() < m_dominanceNum)) {
									// The heuristic kicks in
									doNotPropagate = true;
								}

								break;
							}

							if (OW) {
								LOG_STREAM() << "Ow is " << OW << "\n";
							}
						}
					}

					if (doNotPropagate) {
						if (VERBOSE) {
							LOG << "% propagation of " << def->getNumber() << " into " << m_number
								<< " prevented by the "
								"propagate past overwriting statement in loop heuristic\n";
						}

						continue;
					}
				}
#endif
			}


			// Check if the -l flag (propMaxDepth) prevents this propagation
			if (destCounts && !lhs->isFlags()) { // Always propagate to %flags
				std::map<SharedExp, int, lessExpStar>::iterator ff = destCounts->find(e);

				if ((ff != destCounts->end()) && (ff->second > 1) && (rhs->getComplexityDepth(m_proc) >= propMaxDepth)) {
					if (!def->getRight()->containsFlags()) {
						// This propagation is prevented by the -l limit
						continue;
					}
				}
			}

			change |= doPropagateTo(e, def, convert);
		}
	} while (change && ++changes < 10);

	// Simplify is very costly, especially for calls. I hope that doing one simplify at the end will not affect any
	// result...
	simplify();
	return changes > 0; // Note: change is only for the last time around the do/while loop
}


bool Instruction::propagateFlagsTo()
{
	// FIXME: convert is uninitialized ?
	bool change  = false;
	bool convert = false;
	int  changes = 0;

	do {
		LocationSet exps;
		addUsedLocs(exps, true);
		LocationSet::iterator ll;

		for (ll = exps.begin(); ll != exps.end(); ll++) {
			SharedExp e = *ll;

			if (!e->isSubscript()) {
				continue;     // e.g. %pc
			}

			Assignment *def = dynamic_cast<Assignment *>(e->access<RefExp>()->getDef());

			if ((def == nullptr) || (nullptr == def->getRight())) {     // process if it has definition with rhs
				continue;
			}

			SharedExp base = e->access<Exp, 1>();     // Either RefExp or Location ?

			if (base->isFlags() || base->isMainFlag()) {
				change |= doPropagateTo(e, def, convert);
			}
		}
	} while (change && ++changes < 10);

	simplify();
	return change;
}


bool Instruction::doPropagateTo(SharedExp e, Assignment *def, bool& convert)
{
	// Respect the -p N switch
	if (Boomerang::get()->numToPropagate >= 0) {
		if (Boomerang::get()->numToPropagate == 0) {
			return false;
		}

		Boomerang::get()->numToPropagate--;
	}

	if (VERBOSE) {
		LOG << "propagating " << def << "\n"
			<< "       into " << this << "\n";
	}

	bool change = replaceRef(e, def, convert);

	if (VERBOSE) {
		LOG << "     result " << this << "\n\n";
	}

	return change;
}


bool Instruction::replaceRef(SharedExp e, Assignment *def, bool& convert)
{
	SharedExp rhs = def->getRight();

	assert(rhs);

	SharedExp base = e->getSubExp1();
	// Could be propagating %flags into %CF
	SharedExp lhs = def->getLeft();

	if ((base->getOper() == opCF) && lhs->isFlags()) {
		if (!rhs->isFlagCall()) {
			return false;
		}

		QString str = rhs->access<Const, 1>()->getStr();

		// FIXME: check SUBFLAGSFL handling, and implement it if needed
		if (str.startsWith("SUBFLAGS") && (str != "SUBFLAGSFL")) {
			/* When the carry flag is used bare, and was defined in a subtract of the form lhs - rhs, then CF has
			 * the value (lhs <u rhs).  lhs and rhs are the first and second parameters of the flagcall.
			 * Note: the flagcall is a binary, with a Const (the name) and a list of expressions:
			 *       defRhs
			 *      /      \
			 * Const       opList
			 * "SUBFLAGS"    /    \
			 *           P1    opList
			 *         /     \
			 *       P2    opList
			 *    /     \
			 *  P3     opNil
			 */
			SharedExp relExp = Binary::get(opLessUns,
										   rhs->getSubExp2()->getSubExp1(),
										   rhs->getSubExp2()->getSubExp2()->getSubExp1());
			searchAndReplace(*RefExp::get(Terminal::get(opCF), def), relExp, true);
			return true;
		}
	}

	// need something similar for %ZF
	if ((base->getOper() == opZF) && lhs->isFlags()) {
		if (!rhs->isFlagCall()) {
			return false;
		}

		QString str = rhs->access<Const, 1>()->getStr();

		if (str.startsWith("SUBFLAGS") && (str != "SUBFLAGSFL")) {
			// for zf we're only interested in if the result part of the subflags is equal to zero
			SharedExp relExp = Binary::get(opEquals,
										   rhs->getSubExp2()->getSubExp2()->getSubExp2()->getSubExp1(),
										   Const::get(0));
			searchAndReplace(*RefExp::get(Terminal::get(opZF), def), relExp, true);
			return true;
		}

		if (str == "SUBFLAGSFL") {
			// for float zf we'll replace the ZF with (P1==P2)
			SharedExp relExp = Binary::get(opEquals,
										   rhs->getSubExp2()->getSubExp1(),
										   rhs->getSubExp2()->getSubExp2()->getSubExp1()
										   );
			searchAndReplace(*RefExp::get(Terminal::get(opZF), def), relExp, true);
			return true;
		}
	}

	// do the replacement
	// bool convert = doReplaceRef(re, rhs);
	bool ret = searchAndReplace(*e, rhs, true);     // Last parameter true to change collectors
	// assert(ret);

	if (ret && isCall()) {
		convert |= ((CallStatement *)this)->convertToDirect();
	}

	return ret;
}


bool Instruction::isNullStatement() const
{
	if (m_kind != STMT_ASSIGN) {
		return false;
	}

	SharedExp right = ((Assign *)this)->getRight();

	if (right->isSubscript()) {
		// Must refer to self to be null
		return this == right->access<RefExp>()->getDef();
	}
	else {
		// Null if left == right
		return *((Assign *)this)->getLeft() == *right;
	}
}


bool Instruction::isFpush() const
{
	if (m_kind != STMT_ASSIGN) {
		return false;
	}

	return ((Assign *)this)->getRight()->getOper() == opFpush;
}


bool Instruction::isFpop() const
{
	if (m_kind != STMT_ASSIGN) {
		return false;
	}

	return ((Assign *)this)->getRight()->getOper() == opFpop;
}


int Instruction::setConscripts(int n)
{
	StmtConscriptSetter scs(n, false);

	accept(&scs);
	return scs.getLast();
}


void Instruction::clearConscripts()
{
	StmtConscriptSetter scs(0, true);

	accept(&scs);
}


bool Instruction::castConst(int num, SharedType ty)
{
	ExpConstCaster ecc(num, ty);
	StmtModifier   scc(&ecc);

	accept(&scc);
	return ecc.isChanged();
}


void Instruction::stripSizes()
{
	SizeStripper ss;
	StmtModifier sm(&ss);

	accept(&sm);
}


void Instruction::bypass()
{
	CallBypasser     cb(this);
	StmtPartModifier sm(&cb);     // Use the Part modifier so we don't change the top level of LHS of assigns etc

	accept(&sm);

	if (cb.isTopChanged()) {
		simplify();     // E.g. m[esp{20}] := blah -> m[esp{-}-20+4] := blah
	}
}


void Instruction::addUsedLocs(LocationSet& used, bool cc /* = false */, bool memOnly /*= false */)
{
	UsedLocsFinder  ulf(used, memOnly);
	UsedLocsVisitor ulv(&ulf, cc);

	accept(&ulv);
}


bool Instruction::addUsedLocals(LocationSet& used)
{
	UsedLocalFinder ulf(used, m_proc);
	UsedLocsVisitor ulv(&ulf, false);

	accept(&ulv);
	return ulf.wasAllFound();
}


void Instruction::subscriptVar(SharedExp e, Instruction *def /*, Cfg* cfg */)
{
	ExpSubscripter  es(e, def /*, cfg*/);
	StmtSubscripter ss(&es);

	accept(&ss);
}


void Instruction::findConstants(std::list<std::shared_ptr<Const> >& lc)
{
	ConstFinder     cf(lc);
	StmtConstFinder scf(&cf);

	accept(&scf);
}


void Instruction::mapRegistersToLocals()
{
	ExpRegMapper  erm(m_proc);
	StmtRegMapper srm(&erm);

	accept(&srm);
}


void Instruction::insertCasts()
{
	// First we postvisit expressions using a StmtModifier and an ExpCastInserter
	ExpCastInserter eci(m_proc);
	StmtModifier    sm(&eci, true);     // True to ignore collectors

	accept(&sm);
	// Now handle the LHS of assigns that happen to be m[...], using a StmtCastInserter
	StmtCastInserter sci;
	accept(&sci);
}


void Instruction::replaceSubscriptsWithLocals()
{
	ExpSsaXformer  esx(m_proc);
	StmtSsaXformer ssx(&esx, m_proc);

	accept(&ssx);
}


void Instruction::dfaMapLocals()
{
	DfaLocalMapper dlm(m_proc);
	StmtModifier   sm(&dlm, true);     // True to ignore def collector in return statement

	accept(&sm);

	if (VERBOSE && dlm.change) {
		LOG << "statement mapped with new local(s): " << m_number << "\n";
	}
}


// A helper class for comparing Assignment*'s sensibly
bool lessAssignment::operator()(const Assignment *x, const Assignment *y) const
{
	return(*x->getLeft() < *y->getLeft());     // Compare the LHS expressions
}


// Repeat the above for Assign's; sometimes the compiler doesn't (yet) understand that Assign's are Assignment's
bool lessAssign::operator()(const Assign *x, const Assign *y) const
{
	return(*x->getLeft() < *y->getLeft());     // Compare the LHS expressions
}
