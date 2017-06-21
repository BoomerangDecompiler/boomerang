#include "statementhelper.h"

#include "core/log.h"

// Common to BranchStatement and BoolAssign
// Return true if this is now a floating point Branch
bool condToRelational(SharedExp& pCond, BranchType jtCond)
{
	pCond = pCond->simplifyArith()->simplify();

	QString     tgt;
	QTextStream os(&tgt);
	pCond->print(os);

	OPER condOp = pCond->getOper();

	if ((condOp == opFlagCall) && pCond->access<Const, 1>()->getStr().startsWith("SUBFLAGS")) {
		OPER op = opWild;
		// Special for PPC unsigned compares; may be other cases in the future
		bool makeUns = pCond->access<Const, 1>()->getStr().startsWith("SUBFLAGSNL");

		switch (jtCond)
		{
		case BRANCH_JE:
			op = opEquals;
			break;

		case BRANCH_JNE:
			op = opNotEqual;
			break;

		case BRANCH_JSL:

			if (makeUns) {
				op = opLessUns;
			}
			else {
				op = opLess;
			}

			break;

		case BRANCH_JSLE:

			if (makeUns) {
				op = opLessEqUns;
			}
			else {
				op = opLessEq;
			}

			break;

		case BRANCH_JSGE:

			if (makeUns) {
				op = opGtrEqUns;
			}
			else {
				op = opGtrEq;
			}

			break;

		case BRANCH_JSG:

			if (makeUns) {
				op = opGtrUns;
			}
			else {
				op = opGtr;
			}

			break;

		case BRANCH_JUL:
			op = opLessUns;
			break;

		case BRANCH_JULE:
			op = opLessEqUns;
			break;

		case BRANCH_JUGE:
			op = opGtrEqUns;
			break;

		case BRANCH_JUG:
			op = opGtrUns;
			break;

		case BRANCH_JMI:

			/*     pCond
			 *      /      \
			 *  Const       opList
			 * "SUBFLAGS"    /    \
			 * P1    opList
			 *     /     \
			 *   P2    opList
			 *          /     \
			 *        P3     opNil */
			pCond =
				Binary::get(opLess,     // P3 < 0
							pCond->getSubExp2()->getSubExp2()->getSubExp2()->getSubExp1()->clone(), Const::get(0));
			break;

		case BRANCH_JPOS:
			pCond =
				Binary::get(opGtrEq,     // P3 >= 0
							pCond->getSubExp2()->getSubExp2()->getSubExp2()->getSubExp1()->clone(), Const::get(0));
			break;

		case BRANCH_JOF:
		case BRANCH_JNOF:
		case BRANCH_JPAR:
			break;
		}

		if (op != opWild) {
			pCond = Binary::get(op,
								pCond->getSubExp2()->getSubExp1()->clone(),                    // P1
								pCond->getSubExp2()->getSubExp2()->getSubExp1()->clone());     // P2
		}
	}
	else if ((condOp == opFlagCall) && pCond->access<Const, 1>()->getStr().startsWith("LOGICALFLAGS")) {
		// Exp *e = pCond;
		OPER op = opWild;

		switch (jtCond)
		{
		case BRANCH_JE:
			op = opEquals;
			break;

		case BRANCH_JNE:
			op = opNotEqual;
			break;

		case BRANCH_JMI:
			op = opLess;
			break;

		case BRANCH_JPOS:
			op = opGtrEq;
			break;

		// FIXME: This next set is quite shakey. Really, we should pull all the individual flag definitions out of
		// the flag definitions, and substitute these into the equivalent conditions for the branches (a big, ugly
		// job).
		case BRANCH_JSL:
			op = opLess;
			break;

		case BRANCH_JSLE:
			op = opLessEq;
			break;

		case BRANCH_JSGE:
			op = opGtrEq;
			break;

		case BRANCH_JSG:
			op = opGtr;
			break;

		// These next few seem to fluke working fine on architectures like X86, SPARC, and 68K which clear the
		// carry on all logical operations.
		case BRANCH_JUL:
			op = opLessUns;
			break;     // NOTE: this is equivalent to never branching, since nothing

		// can be unsigned less than zero
		case BRANCH_JULE:
			op = opLessEqUns;
			break;

		case BRANCH_JUGE:
			op = opGtrEqUns;
			break;     // Similarly, this is equivalent to always branching

		case BRANCH_JUG:
			op = opGtrUns;
			break;

		case BRANCH_JPAR:
			{
				// This is pentium specific too; see below for more notes.

				/*
				 *                                      pCond
				 *                                      /    \
				 *                                Const        opList
				 *                      "LOGICALFLAGS8"        /    \
				 *                                      opBitAnd    opNil
				 *                                           /        \
				 *                                 opFlagCall        opIntConst
				 *                                  /        \            mask
				 *                              Const        opList
				 *                          "SETFFLAGS"      /    \
				 *                                          P1    opList
				 *                                        /    \
				 *                                      P2    opNil
				 */
				SharedExp flagsParam = pCond->getSubExp2()->getSubExp1();
				SharedExp test       = flagsParam;

				if (test->isSubscript()) {
					test = test->getSubExp1();
				}

				if (test->isTemp()) {
					return false;     // Just not propagated yet
				}

				int mask = 0;

				if (flagsParam->getOper() == opBitAnd) {
					SharedExp setFlagsParam = flagsParam->getSubExp2();

					if (setFlagsParam->isIntConst()) {
						mask = setFlagsParam->access<Const>()->getInt();
					}
				}

				SharedExp at_opFlagsCall_List = flagsParam->getSubExp1()->getSubExp2();
				// Sometimes the mask includes the 0x4 bit, but we expect that to be off all the time. So effectively
				// the branch is for any one of the (one or two) bits being on. For example, if the mask is 0x41, we
				// are branching of less (0x1) or equal (0x41).
				mask &= 0x41;
				OPER _op;

				switch (mask)
				{
				case 0:
					LOG << "WARNING: unhandled pentium branch if parity with pCond = " << pCond << "\n";
					return false;

				case 1:
					_op = opLess;
					break;

				case 0x40:
					_op = opEquals;
					break;

				case 0x41:
					_op = opLessEq;
					break;

				default:
					_op = opWild;     // Not possible, but avoid a compiler warning
					break;
				}

				pCond = Binary::get(_op, at_opFlagsCall_List->getSubExp1()->clone(),
									at_opFlagsCall_List->getSubExp2()->getSubExp1()->clone());
				return true;     // This is a floating point comparison
			}

		default:
			break;
		}

		if (op != opWild) {
			pCond = Binary::get(op, pCond->getSubExp2()->getSubExp1()->clone(), Const::get(0));
		}
	}
	else if ((condOp == opFlagCall) && pCond->access<Const, 1>()->getStr().startsWith("SETFFLAGS")) {
		// Exp *e = pCond;
		OPER op = opWild;

		switch (jtCond)
		{
		case BRANCH_JE:
			op = opEquals;
			break;

		case BRANCH_JNE:
			op = opNotEqual;
			break;

		case BRANCH_JMI:
			op = opLess;
			break;

		case BRANCH_JPOS:
			op = opGtrEq;
			break;

		case BRANCH_JSL:
			op = opLess;
			break;

		case BRANCH_JSLE:
			op = opLessEq;
			break;

		case BRANCH_JSGE:
			op = opGtrEq;
			break;

		case BRANCH_JSG:
			op = opGtr;
			break;

		default:
			break;
		}

		if (op != opWild) {
			pCond = Binary::get(op, pCond->getSubExp2()->getSubExp1()->clone(),
								pCond->getSubExp2()->getSubExp2()->getSubExp1()->clone());
		}
	}
	// ICK! This is all PENTIUM SPECIFIC... needs to go somewhere else.
	// Might be of the form (SETFFLAGS(...) & MASK) RELOP INTCONST where MASK could be a combination of 1, 4, and
	// 40,
	// and relop could be == or ~=.  There could also be an XOR 40h after the AND
	// From MSVC 6, we can also see MASK = 0x44, 0x41, 0x5 followed by jump if (even) parity (see above)
	// %fflags = 0..0.0 00 >
	// %fflags = 0..0.1 01 <
	// %fflags = 1..0.0 40 =
	// %fflags = 1..1.1 45 not comparable
	// Example: (SETTFLAGS(...) & 1) ~= 0
	// left = SETFFLAGS(...) & 1
	// left1 = SETFFLAGS(...) left2 = int 1, k = 0, mask = 1
	else if ((condOp == opEquals) || (condOp == opNotEqual)) {
		SharedExp left     = pCond->getSubExp1();
		SharedExp right    = pCond->getSubExp2();
		bool      hasXor40 = false;

		if ((left->getOper() == opBitXor) && right->isIntConst()) {
			SharedExp r2 = left->getSubExp2();

			if (r2->isIntConst()) {
				int k2 = r2->access<Const>()->getInt();

				if (k2 == 0x40) {
					hasXor40 = true;
					left     = left->getSubExp1();
				}
			}
		}

		if ((left->getOper() == opBitAnd) && right->isIntConst()) {
			SharedExp left1 = left->getSubExp1();
			SharedExp left2 = left->getSubExp2();
			int       k     = right->access<Const>()->getInt();
			// Only interested in 40, 1
			k &= 0x41;

			if ((left1->getOper() == opFlagCall) && left2->isIntConst()) {
				int mask = left2->access<Const>()->getInt();
				// Only interested in 1, 40
				mask &= 0x41;
				OPER op = opWild;

				if (hasXor40) {
					assert(k == 0);
					op = condOp;
				}
				else {
					switch (mask)
					{
					case 1:

						if (((condOp == opEquals) && (k == 0)) || ((condOp == opNotEqual) && (k == 1))) {
							op = opGtrEq;
						}
						else {
							op = opLess;
						}

						break;

					case 0x40:

						if (((condOp == opEquals) && (k == 0)) || ((condOp == opNotEqual) && (k == 0x40))) {
							op = opNotEqual;
						}
						else {
							op = opEquals;
						}

						break;

					case 0x41:

						switch (k)
						{
						case 0:

							if (condOp == opEquals) {
								op = opGtr;
							}
							else {
								op = opLessEq;
							}

							break;

						case 1:

							if (condOp == opEquals) {
								op = opLess;
							}
							else {
								op = opGtrEq;
							}

							break;

						case 0x40:

							if (condOp == opEquals) {
								op = opEquals;
							}
							else {
								op = opNotEqual;
							}

							break;

						default:
							LOG_STREAM() << "BranchStatement::simplify: k is " << QString::number(k, 16) << "\n";
							assert(0);
						}

						break;

					default:
						LOG_STREAM() << "BranchStatement::simplify: Mask is " << QString::number(mask, 16) << "\n";
						assert(0);
					}
				}

				if (op != opWild) {
					pCond = Binary::get(op, left1->getSubExp2()->getSubExp1(),
										left1->getSubExp2()->getSubExp2()->getSubExp1());
					return true;     // This is now a float comparison
				}
			}
		}
	}

	return false;
}
