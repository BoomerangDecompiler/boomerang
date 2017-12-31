#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "IndirectJumpAnalyzer.h"


#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/CaseStatement.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/visitor/ConstGlobalConverter.h"
#include "boomerang/db/Global.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/FuncType.h"


// Switch High Level patterns

// With array processing, we get a new form, call it form 'a' (don't confuse with form 'A'):
// Pattern: <base>{}[<index>]{} where <index> could be <var> - <Kmin>
// TODO: use initializer lists
static SharedExp forma =
    RefExp::get(Binary::get(opArrayIndex,
                            RefExp::get(Terminal::get(opWild), (Statement *)-1),
                            Terminal::get(opWild)),
                (Statement *)-1);

// Pattern: m[<expr> * 4 + T ]
static SharedExp formA = Location::memOf(
    Binary::get(opPlus,
                Binary::get(opMult,
                            Terminal::get(opWild),
                            Const::get(4)),
                Terminal::get(opWildIntConst)));

// With array processing, we get a new form, call it form 'o' (don't confuse with form 'O'):
// Pattern: <base>{}[<index>]{} where <index> could be <var> - <Kmin>
// NOT COMPLETED YET!
static SharedExp formo =
    RefExp::get(Binary::get(opArrayIndex,
                            RefExp::get(Terminal::get(opWild), (Statement *)-1),
                            Terminal::get(opWild)),
                (Statement *)-1);

// Pattern: m[<expr> * 4 + T ] + T
static SharedExp formO =
    Binary::get(opPlus,
                Location::memOf(Binary::get(opPlus,
                                            Binary::get(opMult,
                                                        Terminal::get(opWild),
                                                        Const::get(4)),
                                            Terminal::get(opWildIntConst))),
                Terminal::get(opWildIntConst));

// Pattern: %pc + m[%pc     + (<expr> * 4) + k]
// where k is a small constant, typically 28 or 20
static SharedExp formR =
    Binary::get(opPlus,
                Terminal::get(opPC),
                Location::memOf(Binary::get(opPlus,
                                            Terminal::get(opPC),
                                            Binary::get(opPlus,
                                                        Binary::get(opMult,
                                                                    Terminal::get(opWild),
                                                                    Const::get(4)),
                                                        Const::get(opWildIntConst)))));

// Pattern: %pc + m[%pc + ((<expr> * 4) - k)] - k
// where k is a smallish constant, e.g. 288 (/usr/bin/vi 2.6, 0c4233c).
static SharedExp formr =
    Binary::get(opPlus,
                Terminal::get(opPC),
                Location::memOf(Binary::get(opPlus,
                                            Terminal::get(opPC),
                                            Binary::get(opMinus,
                                                        Binary::get(opMult,
                                                                    Terminal::get(opWild), Const::get(4)),
                                                        Terminal::get(opWildIntConst)))));


struct SwitchForm
{
    SharedConstExp pattern;
    char           type;
};

static SwitchForm hlForms[] =
{
    { forma, 'a' },
    { formA, 'A' },
    { formo, 'o' },
    { formO, 'O' },
    { formR, 'R' },
    { formr, 'r' }
};


// Vcall high level patterns
// Pattern 0: global<wild>[0]
static SharedExp vfc_funcptr =
    Binary::get(opArrayIndex,
                Location::get(opGlobal, Terminal::get(opWildStrConst), nullptr),
                Const::get(0));

// Pattern 1: m[ m[ <expr> + K1 ] + K2 ]
// K1 is vtable offset, K2 is virtual function offset (could come from m[A2], if A2 is in read-only memory
static SharedExp vfc_both = Location::memOf(
    Binary::get(opPlus,
                Location::memOf(Binary::get(opPlus,
                                            Terminal::get(opWild),
                                            Terminal::get(opWildIntConst))),
                Terminal::get(opWildIntConst)));

// Pattern 2: m[ m[ <expr> ] + K2]
static SharedExp vfc_vto =
    Location::memOf(Binary::get(opPlus,
                                Location::memOf(Terminal::get(opWild)),
                                Terminal::get(opWildIntConst)));

// Pattern 3: m[ m[ <expr> + K1] ]
static SharedExp vfc_vfo =
    Location::memOf(Location::memOf(Binary::get(opPlus,
                                                Terminal::get(opWild),
                                                Terminal::get(opWildIntConst))));

// Pattern 4: m[ m[ <expr> ] ]
static SharedExp vfc_none = Location::memOf(Location::memOf(Terminal::get(opWild)));

static SharedExp hlVfc[] = { vfc_funcptr, vfc_both, vfc_vto, vfc_vfo, vfc_none };


/// Find all the possible constant values that the location defined by s could be assigned with
static void findConstantValues(const Statement *s, std::list<int>& dests)
{
    if (s == nullptr) {
        return;
    }

    if (s->isPhi()) {
        // For each definition, recurse
        for (const auto& it : *((const PhiAssign *)s)) {
            findConstantValues(it.second.getDef(), dests);
        }
    }
    else if (s->isAssign()) {
        SharedExp rhs = ((const Assign *)s)->getRight();

        if (rhs->isIntConst()) {
            dests.push_back(rhs->access<Const>()->getInt());
        }
    }
}


void findSwParams(char form, SharedExp e, SharedExp& expr, Address& T)
{
    switch (form)
    {
    case 'a':
        {
            // Pattern: <base>{}[<index>]{}
            e = e->getSubExp1();
            SharedExp base = e->getSubExp1();

            if (base->isSubscript()) {
                base = base->getSubExp1();
            }

            auto     con     = base->access<Const, 1>();
            QString  gloName = con->getStr();
            UserProc *p      = std::static_pointer_cast<Location>(base)->getProc();
            Prog     *prog   = p->getProg();
            T    = prog->getGlobalAddr(gloName);
            expr = e->getSubExp2();
            break;
        }

    case 'A':
        {
            // Pattern: m[<expr> * 4 + T ]
            if (e->isSubscript()) {
                e = e->getSubExp1();
            }

            // b will be (<expr> * 4) + T
            SharedExp b = e->getSubExp1();
            T    = b->access<Const, 2>()->getAddr();
            expr = b->access<Exp, 1, 1>();
            break;
        }

    case 'O':
        {       // Form O
            // Pattern: m[<expr> * 4 + T ] + T
            T = e->access<Const, 2>()->getAddr();
            // l = m[<expr> * 4 + T ]:
            SharedExp l = e->getSubExp1();

            if (l->isSubscript()) {
                l = l->getSubExp1();
            }

            // b = <expr> * 4 + T:
            SharedExp b = l->getSubExp1();
            // b = <expr> * 4:
            b = b->getSubExp1();
            // expr = <expr>:
            expr = b->getSubExp1();
            break;
        }

    case 'R':
        {
            // Pattern: %pc + m[%pc     + (<expr> * 4) + k]
            T = Address::ZERO; // ?
            // l = m[%pc  + (<expr> * 4) + k]:
            SharedExp l = e->getSubExp2();

            if (l->isSubscript()) {
                l = l->getSubExp1();
            }

            // b = %pc    + (<expr> * 4) + k:
            SharedExp b = l->getSubExp1();
            // b = (<expr> * 4) + k:
            b = b->getSubExp2();
            // b = <expr> * 4:
            b = b->getSubExp1();
            // expr = <expr>:
            expr = b->getSubExp1();
            break;
        }

    case 'r':
        {
            // Pattern: %pc + m[%pc + ((<expr> * 4) - k)] - k
            T = Address::ZERO; // ?
            // b = %pc + m[%pc + ((<expr> * 4) - k)]:
            SharedExp b = e->getSubExp1();
            // l = m[%pc + ((<expr> * 4) - k)]:
            SharedExp l = b->getSubExp2();

            if (l->isSubscript()) {
                l = l->getSubExp1();
            }

            // b = %pc + ((<expr> * 4) - k)
            b = l->getSubExp1();
            // b = ((<expr> * 4) - k):
            b = b->getSubExp2();
            // b = <expr> * 4:
            b = b->getSubExp1();
            // expr = <expr>
            expr = b->getSubExp1();
            break;
        }

    default:
        expr = nullptr;
        T    = Address::INVALID;
    }

    // normalize address to native
    T = T.native();
}


bool IndirectJumpAnalyzer::decodeIndirectJmp(BasicBlock *bb, UserProc *proc)
{
#if CHECK_REAL_PHI_LOOPS
    rtlit rit;
    StatementList::iterator sit;
    Statement               *s = getFirstStmt(rit, sit);

    for (s = getFirstStmt(rit, sit); s; s = getNextStmt(rit, sit)) {
        if (!s->isPhi()) {
            continue;
        }

        Statement      *originalPhi = s;
        InstructionSet workSet, seenSet;
        workSet.insert(s);
        seenSet.insert(s);

        do {
            PhiAssign *pi = (PhiAssign *)*workSet.begin();
            workSet.remove(pi);
            PhiAssign::Definitions::iterator it;

            for (it = pi->begin(); it != pi->end(); it++) {
                if (it->def == nullptr) {
                    continue;
                }

                if (!it->def->isPhi()) {
                    continue;
                }

                if (seenSet.exists(it->def)) {
                    LOG_VERBOSE("Real phi loop involving statements %1 and %2",
                                originalPhi->getNumber(), pi->getNumber());
                    break;
                }
                else {
                    workSet.insert(it->def);
                    seenSet.insert(it->def);
                }
            }
        } while (workSet.size());
    }
#endif

    if (bb->isType(BBType::CompJump)) {
        assert(!bb->getRTLs()->empty());
        RTL *lastRtl = bb->getLastRTL();

        if (SETTING(debugSwitch)) {
            LOG_MSG("decodeIndirectJmp: %1", lastRtl->prints());
        }

        assert(!lastRtl->empty());
        CaseStatement *lastStmt = (CaseStatement *)lastRtl->back();

        // Note: some programs might not have the case expression propagated to, because of the -l switch (?)
        // We used to use ordinary propagation here to get the memory expression, but now it refuses to propagate memofs
        // because of the alias safety issue. Eventually, we should use an alias-safe incremental propagation, but for
        // now we'll assume no alias problems and force the propagation
        bool convert = false;
        lastStmt->propagateTo(convert, nullptr, nullptr, true /* force */);
        SharedExp e = lastStmt->getDest();

        char form = 0;

        for (auto& val : hlForms) {
            if (*e *= *val.pattern) { // *= compare ignores subscripts
                form = val.type;

                if (DEBUG_SWITCH) {
                    LOG_MSG("Indirect jump matches form %1", form);
                }

                break;
            }
        }

        if (form) {
            SwitchInfo *swi = new SwitchInfo;
            swi->chForm = form;
            Address   T = Address::INVALID;
            SharedExp expr;
            findSwParams(form, e, expr, T);

            if (expr) {
                swi->uTable    = T;
                swi->iNumTable = findNumCases(bb);

                // TMN: Added actual control of the array members, to possibly truncate what findNumCases()
                // thinks is the number of cases, when finding the first array element not pointing to code.
                if (form == 'A') {
                    const Prog *prog = proc->getProg();

                    for (int iPtr = 0; iPtr < swi->iNumTable; ++iPtr) {
                        Address uSwitch = Address(prog->readNative4(swi->uTable + iPtr * 4));

                        if (!Util::inRange(uSwitch, prog->getLimitTextLow(), prog->getLimitTextHigh())) {
                            if (DEBUG_SWITCH) {
                                LOG_MSG("Truncating type A indirect jump array to %1 entries "
                                        "due to finding an array entry pointing outside valid code; %2 isn't in %3..%4",
                                        iPtr, uSwitch, prog->getLimitTextLow(), prog->getLimitTextHigh());
                            }

                            // Found an array that isn't a pointer-to-code. Assume array has ended.
                            swi->iNumTable = iPtr;
                            break;
                        }
                    }
                }

                if (swi->iNumTable <= 0) {
                    LOG_WARN("Switch analysis failure at address %1", bb->getLowAddr());
                    return false;
                }

                // TODO: missing form = 'R' iOffset is not being set
                swi->iUpper = swi->iNumTable - 1;
                swi->iLower = 0;

                if ((expr->getOper() == opMinus) && expr->getSubExp2()->isIntConst()) {
                    swi->iLower  = std::static_pointer_cast<Const>(expr->getSubExp2())->getInt();
                    swi->iUpper += swi->iLower;
                    expr         = expr->getSubExp1();
                }

                swi->pSwitchVar = expr;
                lastStmt->setDest((SharedExp)nullptr);
                lastStmt->setSwitchInfo(swi);
                return swi->iNumTable != 0;
            }
        }
        else {
            // Did not match a switch pattern. Perhaps it is a Fortran style goto with constants at the leaves of the
            // phi tree. Basically, a location with a reference, e.g. m[r28{-} - 16]{87}
            if (e->isSubscript()) {
                SharedExp sub = e->getSubExp1();

                if (sub->isLocation()) {
                    // Yes, we have <location>{ref}. Follow the tree and store the constant values that <location>
                    // could be assigned to in dests
                    std::list<int> dests;
                    findConstantValues(std::static_pointer_cast<RefExp>(e)->getDef(), dests);
                    // The switch info wants an array of native addresses
                    size_t num_dests = dests.size();

                    if (num_dests) {
                        int *destArray = new int[num_dests];
                        std::copy(dests.begin(), dests.end(), destArray);
                        SwitchInfo *swi = new SwitchInfo;
                        swi->chForm     = 'F';                                     // The "Fortran" form
                        swi->pSwitchVar = e;
                        swi->uTable     = Address(HostAddress(destArray).value()); // WARN: HACK HACK HACK Abuse the uTable member as a pointer
                        swi->iNumTable  = (int)num_dests;
                        swi->iLower     = 1;                                       // Not used, except to compute
                        swi->iUpper     = (int)num_dests;                          // the number of options
                        lastStmt->setDest((SharedExp)nullptr);
                        lastStmt->setSwitchInfo(swi);
                        return true;
                    }
                }
            }
        }

        return false;
    }
    else if (bb->isType(BBType::CompCall)) {
        assert(!bb->getRTLs()->empty());
        RTL *lastRtl = bb->getLastRTL();

        if (DEBUG_SWITCH) {
            LOG_MSG("decodeIndirectJmp: COMPCALL:");
            LOG_MSG("%1", lastRtl->prints());
        }

        assert(!lastRtl->empty());
        CallStatement *lastStmt = (CallStatement *)lastRtl->back();
        SharedExp     e         = lastStmt->getDest();
        // Indirect calls may sometimes not be propagated to, because of limited propagation (-l switch).
        // Propagate to e, but only keep the changes if the expression matches (don't want excessive propagation to
        // a genuine function pointer expression, even though it's hard to imagine).
        e = e->propagateAll();

        // We also want to replace any m[K]{-} with the actual constant from the (presumably) read-only data section
        ConstGlobalConverter cgc(proc->getProg());
        e = e->accept(&cgc);
        // Simplify the result, e.g. for m[m[(r24{16} + m[0x8048d74]{-}) + 12]{-}]{-} get
        // m[m[(r24{16} + 20) + 12]{-}]{-}, want m[m[r24{16} + 32]{-}]{-}. Note also that making the
        // ConstGlobalConverter a simplifying expression modifier won't work in this case, since the simplifying
        // converter will only simplify the direct parent of the changed expression (which is r24{16} + 20).
        e = e->simplify();

        if (DEBUG_SWITCH) {
            LOG_MSG("decodeIndirect: propagated and const global converted call expression is %1", e);
        }

        int  n          = sizeof(hlVfc) / sizeof(SharedExp);
        bool recognised = false;
        int  i;

        for (i = 0; i < n; i++) {
            if (*e *= *hlVfc[i]) { // *= compare ignores subscripts
                recognised = true;

                if (DEBUG_SWITCH) {
                    LOG_MSG("Indirect call matches form %1", i);
                }

                break;
            }
        }

        if (!recognised) {
            return false;
        }

        lastStmt->setDest(e); // Keep the changes to the indirect call expression
        int       K1, K2;
        SharedExp vtExp, t1;
        Prog      *prog = proc->getProg();

        switch (i)
        {
        case 0:
            {
                // This is basically an indirection on a global function pointer.  If it is initialised, we have a
                // decodable entry point.  Note: it could also be a library function (e.g. Windows)
                // Pattern 0: global<name>{0}[0]{0}
                K2 = 0;

                if (e->isSubscript()) {
                    e = e->getSubExp1();
                }

                e  = e->getSubExp1(); // e is global<name>{0}[0]
                t1 = e->getSubExp2();
                auto t1_const = std::static_pointer_cast<Const>(t1);

                if (e->isArrayIndex() && (t1->isIntConst()) && (t1_const->getInt() == 0)) {
                    e = e->getSubExp1(); // e is global<name>{0}
                }

                if (e->isSubscript()) {
                    e = e->getSubExp1();                                                        // e is global<name>
                }

                std::shared_ptr<Const> con     = std::static_pointer_cast<Const>(e->getSubExp1()); // e is <name>
                Global                 *global = prog->getGlobal(con->getStr());
                assert(global);
                // Set the type to pointer to function, if not already
                SharedType ty = global->getType();

                if (!ty->isPointer() && !std::static_pointer_cast<PointerType>(ty)->getPointsTo()->isFunc()) {
                    global->setType(PointerType::get(FuncType::get()));
                }

                Address addr = global->getAddress();
                // FIXME: not sure how to find K1 from here. I think we need to find the earliest(?) entry in the data
                // map that overlaps with addr
                // For now, let K1 = 0:
                K1    = 0;
                vtExp = Const::get(addr);
                break;
            }

        case 1:
            {
                // Example pattern: e = m[m[r27{25} + 8]{-} + 8]{-}
                if (e->isSubscript()) {
                    e = e->getSubExp1();
                }

                e = e->getSubExp1();             // e = m[r27{25} + 8]{-} + 8
                SharedExp rhs = e->getSubExp2(); // rhs = 8
                K2 = std::static_pointer_cast<Const>(rhs)->getInt();
                SharedExp lhs = e->getSubExp1(); // lhs = m[r27{25} + 8]{-}

                if (lhs->isSubscript()) {
                    lhs = lhs->getSubExp1(); // lhs = m[r27{25} + 8]
                }

                vtExp = lhs;
                lhs   = lhs->getSubExp1(); // lhs =   r27{25} + 8
                SharedExp CK1 = lhs->getSubExp2();
                K1 = std::static_pointer_cast<Const>(CK1)->getInt();
                break;
            }

        case 2:
            {
                // Example pattern: e = m[m[r27{25}]{-} + 8]{-}
                if (e->isSubscript()) {
                    e = e->getSubExp1();
                }

                e = e->getSubExp1();             // e = m[r27{25}]{-} + 8
                SharedExp rhs = e->getSubExp2(); // rhs = 8
                K2 = std::static_pointer_cast<Const>(rhs)->getInt();
                SharedExp lhs = e->getSubExp1(); // lhs = m[r27{25}]{-}

                if (lhs->isSubscript()) {
                    lhs = lhs->getSubExp1(); // lhs = m[r27{25}]
                }

                vtExp = lhs;
                K1    = 0;
                break;
            }

        case 3:
            {
                // Example pattern: e = m[m[r27{25} + 8]{-}]{-}
                if (e->isSubscript()) {
                    e = e->getSubExp1();
                }

                e  = e->getSubExp1(); // e = m[r27{25} + 8]{-}
                K2 = 0;

                if (e->isSubscript()) {
                    e = e->getSubExp1(); // e = m[r27{25} + 8]
                }

                vtExp = e;
                SharedExp lhs = e->getSubExp1(); // lhs =   r27{25} + 8
                // Exp* object = ((Binary*)lhs)->getSubExp1();
                SharedExp CK1 = lhs->getSubExp2();
                K1 = std::static_pointer_cast<Const>(CK1)->getInt();
                break;
            }

        case 4:

            // Example pattern: e = m[m[r27{25}]{-}]{-}
            if (e->isSubscript()) {
                e = e->getSubExp1();
            }

            e  = e->getSubExp1(); // e = m[r27{25}]{-}
            K2 = 0;

            if (e->isSubscript()) {
                e = e->getSubExp1(); // e = m[r27{25}]
            }

            vtExp = e;
            K1    = 0;
            // Exp* object = ((Unary*)e)->getSubExp1();
            break;

        default:
            K1    = K2 = -1; // Suppress warnings
            vtExp = nullptr;
        }

        if (DEBUG_SWITCH) {
            LOG_MSG("Form %1: from statement %2 get e = %3, K1 = %4, K2 = %5, vtExp = %6",
                    i, lastStmt->getNumber(), lastStmt->getDest(), K1, K2, vtExp);
        }

        // The vt expression might not be a constant yet, because of expressions not fully propagated, or because of
        // m[K] in the expression (fixed with the ConstGlobalConverter).  If so, look it up in the defCollector in the
        // call
        vtExp = lastStmt->findDefFor(vtExp);

        if (vtExp && DEBUG_SWITCH) {
            LOG_MSG("VT expression boils down to this: %1", vtExp);
        }

        // Danger. For now, only do if -ic given
        bool decodeThru = SETTING(decodeThruIndCall);

        if (decodeThru && vtExp && vtExp->isIntConst()) {
            Address addr  = std::static_pointer_cast<Const>(vtExp)->getAddr();
            Address pfunc = Address(prog->readNative4(addr));

            if (prog->findFunction(pfunc) == nullptr) {
                // A new, undecoded procedure
                if (SETTING(noDecodeChildren)) {
                    return false;
                }

                prog->decodeEntryPoint(pfunc);
                // Since this was not decoded, this is a significant change, and we want to redecode the current
                // function now that the callee has been decoded
                return true;
            }
        }
    }

    return false;
}


int IndirectJumpAnalyzer::findNumCases(BasicBlock *bb)
{
    // should actually search from the statement to i
    for (BasicBlock *in : bb->getPredecessors()) {  // For each in-edge
        if (!in->isType(BBType::Twoway)) {          // look for a two-way BB
            continue;                               // Ignore all others
        }

        BranchStatement *lastStmt = dynamic_cast<BranchStatement *>(in->getLastStmt());
        assert(lastStmt != nullptr);
        SharedExp pCond = lastStmt->getCondExpr();
        if (pCond->getArity() != 2) {
            continue;
        }

        SharedExp rhs = pCond->getSubExp2();

        if (!rhs->isIntConst()) {
            continue;
        }

        int  k  = std::static_pointer_cast<Const>(rhs)->getInt();
        OPER op = pCond->getOper();

        if ((op == opGtr) || (op == opGtrUns)) {
            return k + 1;
        }

        if ((op == opGtrEq) || (op == opGtrEqUns)) {
            return k;
        }

        if ((op == opLess) || (op == opLessUns)) {
            return k;
        }

        if ((op == opLessEq) || (op == opLessEqUns)) {
            return k + 1;
        }
    }

    LOG_WARN("Could not find number of cases for n-way at address %1", bb->getLowAddr());
    return 1; // Bald faced guess if all else fails
}


void IndirectJumpAnalyzer::processSwitch(BasicBlock *bb, UserProc *proc)
{
    RTL *lastRTL = bb->getLastRTL();
    SwitchInfo *si = ((CaseStatement *)lastRTL->getHlStmt())->getSwitchInfo();

    if (SETTING(debugSwitch)) {
        LOG_MSG("Processing switch statement type %1 with table at %2, %3 entries, lo=%4, hi=%5",
                si->chForm, si->uTable, si->iNumTable, si->iLower, si->iUpper);
    }

    Address switchDestination;
    int iNumOut = si->iUpper - si->iLower + 1;
    int iNum    = iNumOut;

    // Emit an NWAY BB instead of the COMPJUMP. Also update the number of out edges.
    bb->setType(BBType::Nway);

    Prog *prog = proc->getProg();
    Cfg  *cfg = proc->getCFG();

    // Where there are repeated switch cases, we have repeated out-edges from the BB. Example:
    // switch (x) {
    //   case 3: case 5:
    //        do something;
    //        break;
    //     case 4: case 10:
    //        do something else
    // ... }
    // The switch statement is emitted assuming one out-edge for each switch value, which is assumed to be iLower+i
    // for the ith zero-based case. It may be that the code for case 5 above will be a goto to the code for case 3,
    // but a smarter back end could group them
    std::list<Address> dests;

    for (int i = 0; i < iNum; i++) {
        // Get the destination address from the switch table.
        if (si->chForm == 'H') {
            int iValue = prog->readNative4(si->uTable + i * 2);

            if (iValue == -1) {
                continue;
            }

            switchDestination = Address(prog->readNative4(si->uTable + i * 8 + 4));
        }
        else if (si->chForm == 'F') {
            switchDestination = Address(((int *)si->uTable.value())[i]);
        }
        else {
            switchDestination = Address(prog->readNative4(si->uTable + i * 4));
        }

        if ((si->chForm == 'O') || (si->chForm == 'R') || (si->chForm == 'r')) {
            // Offset: add table address to make a real pointer to code.  For type R, the table is relative to the
            // branch, so take iOffset. For others, iOffset is 0, so no harm
            if (si->chForm != 'R') {
                assert(si->iOffset == 0);
            }

            switchDestination += si->uTable - si->iOffset;
        }

        if (switchDestination < prog->getLimitTextHigh()) {
            // tq.visit(cfg, uSwitch, this);
            cfg->addEdge(bb, switchDestination, true);

            // Remember to decode the newly discovered switch code arms, if necessary
            // Don't do it right now, in case there are recursive switch statements (e.g. app7win.exe from
            // hackthissite.org)
            dests.push_back(switchDestination);
        }
        else {
            LOG_MSG("Switch table entry branches to past end of text section %1", switchDestination);

            // TMN: If we reached an array entry pointing outside the program text, we can be quite confident the array
            // has ended. Don't try to pull any more data from it.
            LOG_MSG("Assuming the end of the pointer-array has been reached at index %1", i);

            // TODO: Elevate this logic to the code calculating iNumTable, but still leave this code as a safeguard.
            // Q: Should iNumOut and m_iNumOutEdges really be adjusted (iNum - i) ?
            int numToRemove = iNum - i;

            // remove all table elements at index i and above
            while (numToRemove > 0) {
                bb->removeSuccessor(bb->getSuccessor(i));
                numToRemove--;
            }
            break;
        }
    }

    // Decode the newly discovered switch code arms, if any, and if not already decoded
    int count = 0;

    for (Address addr : dests) {
        char tmp[1024];
        count++;
        sprintf(tmp, "before decoding fragment %i of %zu (%s)", count, dests.size(), qPrintable(addr.toString()));
        Boomerang::get()->alertDecompileDebugPoint(proc, tmp);
        prog->decodeFragment(proc, addr);
    }
}
