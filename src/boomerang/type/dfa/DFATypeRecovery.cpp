#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "DFATypeRecovery.h"


#include "boomerang/core/Boomerang.h"
#include "boomerang/core/Project.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/GotoStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/ImpRefStatement.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/type/dfa/DFATypeAnalyzer.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/type/type/BooleanType.h"
#include "boomerang/type/type/CharType.h"
#include "boomerang/type/type/CompoundType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/type/type/FuncType.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/SizeType.h"
#include "boomerang/type/type/UnionType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"
#include "boomerang/visitor/expvisitor/ExpVisitor.h"


#include <sstream>
#include <cstring>
#include <utility>


#define DFA_ITER_LIMIT    (100)

// idx + K; leave idx wild
static const Binary unscaledArrayPat(opPlus, Terminal::get(opWild), Terminal::get(opWildIntConst));


void DFATypeRecovery::dumpResults(StatementList& stmts, int iter)
{
    LOG_VERBOSE("%1 iterations", iter);

    for (Statement *s : stmts) {
        LOG_VERBOSE("%1", s); // Print the statement; has dest type

        // Now print type for each constant in this Statement
        std::list<std::shared_ptr<Const> >           lc;
        s->findConstants(lc);

        for (auto cc = lc.begin(); cc != lc.end(); ++cc) {
            LOG_MSG("    %1, %2", (*cc)->getType()->getCtype(), *cc);
        }

        // If s is a call, also display its return types
        CallStatement *call = dynamic_cast<CallStatement *>(s);

        if (call && call->isCall()) {
            ReturnStatement *rs = call->getCalleeReturn();
            UseCollector *uc = call->getUseCollector();

            if (!rs || !uc) {
                continue;
            }

            LOG_VERBOSE("  returns:");

            for (ReturnStatement::iterator rr = rs->begin(); rr != rs->end(); ++rr) {
                // Intersect the callee's returns with the live locations at the call, i.e. make sure that they
                // exist in *uc
                Assignment *assgn = dynamic_cast<Assignment *>(*rr);
                if (!assgn) {
                    continue;
                }
                SharedExp  lhs    = assgn->getLeft();

                if (!uc->exists(lhs)) {
                    continue; // Intersection fails
                }

                LOG_VERBOSE("    %1 %2", assgn->getType()->getCtype(), assgn->getLeft());
            }
        }
    }
}


// m[idx*K1 + K2]; leave idx wild
static Location
    scaledArrayPat(opMemOf,
                   Binary::get(opPlus,
                               Binary::get(opMult,
                                           Terminal::get(opWild),
                                           Terminal::get(opWildIntConst)
                                           ),
                               Terminal::get(opWildIntConst)
                               ),
                   nullptr);

void DFATypeRecovery::dfa_analyze_scaled_array_ref(Statement *s)
{
    UserProc  *pr   = s->getProc();
    Prog      *prog = pr->getProg();
    SharedExp arr;

    std::list<SharedExp> result;
    s->searchAll(scaledArrayPat, result);

    // query: (memOf (opPlus (opMult ? ?:IntConst) ?:IntConst))
    // rewrite_as (opArrayIndex (global `(getOrCreateGlobalName arg3) ) arg2 ) assert (= (typeSize
    // (getOrCreateGlobalName arg3)) (arg1))
    for (SharedExp rr : result) {
        // Type* ty = s->getTypeFor(*rr);
        // FIXME: should check that we use with array type...
        // Find idx and K2
        assert(rr->getSubExp1() == rr->getSubExp1());
        SharedExp t   = rr->getSubExp1(); // idx*K1 + K2
        SharedExp l   = t->getSubExp1();  // idx*K1
        SharedExp r   = t->getSubExp2();  // K2
        Address   K2  = r->access<Const>()->getAddr().native();
        SharedExp idx = l->getSubExp1();

        // Replace with the array expression
        QString name = prog->getGlobalNameByAddr(K2);

        if (name.isEmpty()) {
            name = prog->newGlobalName(K2);
        }

        arr = Binary::get(opArrayIndex, Location::global(name, pr), idx);

        if (s->searchAndReplace(scaledArrayPat, arr)) {
            if (s->isImplicit()) {
                // Register an array of appropriate type
                prog->markGlobalUsed(K2, ArrayType::get(static_cast<const ImplicitAssign *>(s)->getType()));
            }
        }
    }
}


void DFATypeRecovery::dfa_analyze_implict_assigns(Statement *s)
{
    if (!s->isImplicit()) {
        return;
    }

    UserProc *proc = s->getProc();
    assert(proc);
    Prog *prog = proc->getProg();
    assert(prog);

    SharedExp lhs = static_cast<const ImplicitAssign *>(s)->getLeft();
    // Note: parameters are not explicit any more
    // if (lhs->isParam()) { // }

    bool       allZero      = false;
    SharedExp  slhs         = lhs->clone()->removeSubscripts(allZero);
    SharedType implicitType = static_cast<const ImplicitAssign *>(s)->getType();
    int        i            = proc->getSignature()->findParam(slhs);

    if (i != -1) {
        proc->setParamType(i, implicitType);
    }
    else if (lhs->isMemOf()) {
        SharedExp sub = lhs->getSubExp1();

        if (sub->isIntConst()) {
            // We have a m[K] := -
            Address K = sub->access<Const>()->getAddr();
            prog->markGlobalUsed(K, implicitType);
        }
    }
    else if (lhs->isGlobal()) {
        assert(std::dynamic_pointer_cast<Location>(lhs) != nullptr);
        QString gname = lhs->access<Const, 1>()->getStr();
        prog->setGlobalType(gname, implicitType);
    }
}


void DFATypeRecovery::recoverFunctionTypes(Function *function)
{
    if (function->isLib()) {
        LOG_VERBOSE("Not using DFA type analysis on library function '%1'", function->getName());
        return;
    }

    if (function->getProg()->getProject()->getSettings()->debugTA) {
        LOG_VERBOSE("--- Start data flow based type analysis for %1 ---", getName());
    }

    bool first = true;
    UserProc *up = dynamic_cast<UserProc *>(function);
    assert(up != nullptr);

    do {
        if (first) {
            // Subscript the discovered extra parameters
            PassManager::get()->executePass(PassID::BlockVarRename, up);
            PassManager::get()->executePass(PassID::StatementPropagation, up);
            PassManager::get()->executePass(PassID::ImplicitPlacement, up);
        }

        first = false;
        dfaTypeAnalysis(up);

        // There used to be a pass here to insert casts. This is best left until global type analysis is complete,
        // so do it just before translating from SSA form (which is the where type information becomes inaccessible)
    } while (up->ellipsisProcessing());

    PassManager::get()->executePass(PassID::BBSimplify, up); // In case there are new struct members

    if (function->getProg()->getProject()->getSettings()->debugTA) {
        LOG_VERBOSE("=== End type analysis for %1 ===", getName());
    }
}


void DFATypeRecovery::dfaTypeAnalysis(UserProc *proc)
{
    Cfg             *cfg  = proc->getCFG();
    DataIntervalMap localsMap(proc); // map of all local variables of proc

    proc->getProg()->getProject()->alertDecompileDebugPoint(proc, "Before DFA type analysis");

    // First use the type information from the signature.
    // Sometimes needed to split variables (e.g. argc as a
    // int and char* in sparc/switch_gcc)
    bool          ch = dfaTypeAnalysis(proc->getSignature().get(), cfg);
    StatementList stmts;
    proc->getStatements(stmts);

    int iter = 0;

    for (iter = 1; iter <= DFA_ITER_LIMIT; ++iter) {
        ch = false;

        for (Statement *stmt : stmts) {
            Statement *before = nullptr;

            if (proc->getProg()->getProject()->getSettings()->debugTA) {
                before = stmt->clone();
            }

            DFATypeAnalyzer ana;
            stmt->accept(&ana);
            const bool thisCh  = ana.hasChanged();

            if (thisCh) {
                ch = true;

                if (proc->getProg()->getProject()->getSettings()->debugTA) {
                    LOG_VERBOSE("  Caused change:\n"
                                "    FROM: %1\n"
                                "    TO:   %2",
                                before, stmt);
                }
            }

            if (proc->getProg()->getProject()->getSettings()->debugTA) {
                delete before;
            }
        }

        if (!ch) {
            // No more changes: round robin algorithm terminates
            break;
        }
    }

    if (ch) {
        LOG_VERBOSE("Iteration limit exceeded for dfaTypeAnalysis of procedure '%1'", proc->getName());
    }

    if (proc->getProg()->getProject()->getSettings()->debugTA) {
        LOG_MSG("### Results for data flow based type analysis for %1 ###", proc->getName());
        dumpResults(stmts, iter);
        LOG_MSG("### End results for Data flow based type analysis for %1 ###", proc->getName());
    }

    // Now use the type information gathered

    proc->getProg()->getProject()->alertDecompileDebugPoint(proc, "Before other uses of DFA type analysis");
    proc->debugPrintAll("Before other uses of DFA type analysis");

    Prog *_prog = proc->getProg();

    for (Statement *s : stmts) {
        // 1) constants
        std::list<std::shared_ptr<Const> > lc;
        s->findConstants(lc);

        for (const std::shared_ptr<Const>& con : lc) {
            if (!con || (con->getOper() == opStrConst)) {
                continue;
            }

            SharedType t   = con->getType();
            int        val = con->getInt();

            if (t && t->resolvesToPointer()) {
                auto       pt       = t->as<PointerType>();
                SharedType baseType = pt->getPointsTo();

                if (baseType->resolvesToChar()) {
                    // Convert to a string    MVE: check for read-only?
                    // Also, distinguish between pointer to one char, and ptr to many?
                    const char *str = _prog->getStringConstant(Address(val), true);

                    if (str) {
                        // Make a string
                        con->setStr(str);
                        con->setOper(opStrConst);
                    }
                }
                else if (baseType->resolvesToInteger() || baseType->resolvesToFloat() || baseType->resolvesToSize()) {
                    Address addr = Address(con->getInt()); // TODO: use getAddr
                    _prog->markGlobalUsed(addr, baseType);
                    QString gloName = _prog->getGlobalNameByAddr(addr);

                    if (!gloName.isEmpty()) {
                        Address   r = addr - _prog->getGlobalAddrByName(gloName);
                        SharedExp ne;

                        if (!r.isZero()) { // TODO: what if r is NO_ADDR ?
                            SharedExp g = Location::global(gloName, proc);
                            ne = Location::memOf(Binary::get(opPlus, Unary::get(opAddrOf, g), Const::get(r)), proc);
                        }
                        else {
                            SharedType ty     = _prog->getGlobalType(gloName);
                            Assign     *assgn = dynamic_cast<Assign *>(s);

                            if (assgn && s->isAssign() && assgn->getType()) {
                                size_t bits = assgn->getType()->getSize();

                                if ((ty == nullptr) || (ty->getSize() == 0)) {
                                    _prog->setGlobalType(gloName, IntegerType::get(bits));
                                }
                            }

                            SharedExp g = Location::global(gloName, proc);

                            if (g && ty && ty->resolvesToArray()) {
                                ne = Binary::get(opArrayIndex, g, Const::get(0));
                            }
                            else {
                                ne = g;
                            }
                        }

                        SharedExp memof = Location::memOf(con);

                        if (!s->searchAndReplace(*memof, ne)) {
                            ne = nullptr;
                        }
                    }
                }
                else if (baseType->resolvesToArray()) {
                    // We have found a constant in s which has type pointer to array of alpha. We can't get the parent
                    // of con, but we can find it with the pattern unscaledArrayPat.
                    std::list<SharedExp> result;
                    s->searchAll(unscaledArrayPat, result);

                    for (auto& elem : result) {
                        // idx + K
                        auto bin_rr = std::dynamic_pointer_cast<Binary>(elem);

                        if (!bin_rr) {
                            assert(false);
                            return;
                        }

                        auto constK = bin_rr->access<Const, 2>();

                        // Note: keep searching till we find the pattern with this constant, since other constants may
                        // not be used as pointer to array type.
                        if (constK != con) {
                            continue;
                        }

                        Address   K   = Address(constK->getInt());
                        SharedExp idx = bin_rr->getSubExp1();
                        SharedExp arr = Unary::get(
                            opAddrOf, Binary::get(opArrayIndex, Location::global(_prog->getGlobalNameByAddr(K), proc), idx));
                        // Beware of changing expressions in implicit assignments... map can become invalid
                        bool isImplicit = s->isImplicit();

                        if (isImplicit) {
                            cfg->removeImplicitAssign(static_cast<ImplicitAssign *>(s)->getLeft());
                        }

                        if (!s->searchAndReplace(unscaledArrayPat, arr)) {
                            arr = nullptr; // remove if not emplaced in s
                        }

                        // s will likely have an m[a[array]], so simplify
                        s->simplifyAddr();

                        if (isImplicit) {
                            // Replace the implicit assignment entry. Note that s' lhs has changed
                            cfg->findOrCreateImplicitAssign(static_cast<ImplicitAssign *>(s)->getLeft());
                        }

                        // Ensure that the global is declared
                        // Ugh... I think that arrays and pointers to arrays are muddled!
                        _prog->markGlobalUsed(K, baseType);
                    }
                }
            }
            else if (t->resolvesToFloat()) {
                if (con->isIntConst()) {
                    // Reinterpret as a float (and convert to double)
                    // con->setFlt(reinterpret_cast<float>(con->getInt()));
                    int tmp = con->getInt();
                    con->setFlt(*reinterpret_cast<float *>(&tmp)); // Reinterpret to float, then cast to double
                    con->setOper(opFltConst);
                    con->setType(FloatType::get(64)); // TODO: why double ? float might do
                }

                // MVE: more work if double?
            }
            else { /* if (t->resolvesToArray()) */
                _prog->markGlobalUsed(Address(val), t);
            }
        }

        // 2) Search for the scaled array pattern and replace it with an array use m[idx*K1 + K2]
        dfa_analyze_scaled_array_ref(s);

        // 3) Check implicit assigns for parameter and global types.
        dfa_analyze_implict_assigns(s);

        // 4) Add the locals (soon globals as well) to the localTable, to sort out the overlaps
        if (s->isTyping()) {
            SharedExp  addrExp = nullptr;
            SharedType typeExp = nullptr;

            if (s->isAssignment()) {
                SharedExp lhs = static_cast<Assignment *>(s)->getLeft();

                if (lhs->isMemOf()) {
                    addrExp = lhs->getSubExp1();
                    typeExp = static_cast<Assignment *>(s)->getType();
                }
            }
            else {
                // Assume an implicit reference
                addrExp = static_cast<ImpRefStatement *>(s)->getAddressExp();

                if (addrExp->isTypedExp() && addrExp->access<TypedExp>()->getType()->resolvesToPointer()) {
                    addrExp = addrExp->getSubExp1();
                }

                typeExp = static_cast<ImpRefStatement *>(s)->getType();

                // typeExp should be a pointer expression, or a union of pointer types
                if (typeExp->resolvesToUnion()) {
                    typeExp = typeExp->as<UnionType>()->dereferenceUnion();
                }
                else if (typeExp->resolvesToPointer()) {
                    typeExp = typeExp->as<PointerType>()->getPointsTo();
                }
                else {
                    assert(false);
                    return;
                }
            }

            const int spIndex = Util::getStackRegisterIndex(_prog);
            if (addrExp && proc->getSignature()->isAddrOfStackLocal(spIndex, addrExp)) {
                int localAddressOffset = 0;

                if ((addrExp->getArity() == 2) && proc->getSignature()->isOpCompatStackLocal(addrExp->getOper())) {
                    auto K = addrExp->access<Const, 2>();

                    if (K->isConst()) {
                        localAddressOffset = K->getInt();

                        if (addrExp->getOper() == opMinus) {
                            localAddressOffset = -localAddressOffset;
                        }
                    }
                }

                const SharedType& ty = static_cast<TypingStatement *>(s)->getType();
                LOG_VERBOSE2("Type analysis for '%1': Adding addrExp '%2' with type %3 to local table", proc->getName(), addrExp, ty);
                SharedExp loc_mem = Location::memOf(addrExp);
                localsMap.insertItem(Address(localAddressOffset), proc->lookupSym(loc_mem, ty), typeExp);
            }
        }
    }

    proc->debugPrintAll("After application of DFA type analysis");
    proc->getProg()->getProject()->alertDecompileDebugPoint(proc, "After DFA type analysis");
}


bool DFATypeRecovery::dfaTypeAnalysis(Signature *sig, Cfg *cfg)
{
    bool ch = false;

    for (const std::shared_ptr<Parameter>& it : sig->getParameters()) {
        // Parameters should be defined in an implicit assignment
        Statement *def = cfg->findImplicitParamAssign(it.get());

        if (def) { // But sometimes they are not used, and hence have no implicit definition
            bool thisCh = false;
            def->meetWithFor(it->getType(), it->getExp(), thisCh);

            if (thisCh) {
                ch = true;

                if (cfg->getProc()->getProg()->getProject()->getSettings()->debugTA) {
                    LOG_MSG("  sig caused change: %1 %2", it->getType()->getCtype(), it->getName());
                }
            }
        }
    }

    return ch;
}


bool DFATypeRecovery::dfaTypeAnalysis(Statement *i)
{
    Q_UNUSED(i);
    assert(false);
    return false;
}


// This is the core of the data-flow-based type analysis algorithm: implementing the meet operator.
// In classic lattice-based terms, the TOP type is void; there is no BOTTOM type since we handle overconstraints with
// unions.
// Consider various pieces of knowledge about the types. There could be:
// a) void: no information. Void meet x = x.
// b) size only: find a size large enough to contain the two types.
// c) broad type only, e.g. floating point
// d) signedness, no size
// e) size, no signedness
// f) broad type, size, and (for integer broad type), signedness

// ch set true if any change

