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


/***************************************************************************/ /**
 * \file       dfa.cpp
 * \brief   Implementation of class Type functions related to solving type analysis in an iterative,
 * data-flow-based manner
 ******************************************************************************/

#include "boomerang/core/Boomerang.h"

#include "boomerang/db/Signature.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/statements/GotoStatement.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/ImpRefStatement.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/BoolAssign.h"
#include "boomerang/db/Visitor.h"

#include "boomerang/type/type/CompoundType.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/type/type/UnionType.h"
#include "boomerang/type/type/FuncType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/type/type/SizeType.h"
#include "boomerang/type/type/BooleanType.h"
#include "boomerang/type/type/CharType.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Util.h"


#include <sstream>
#include <cstring>
#include <utility>

static int nextUnionNumber = 0;

#define DFA_ITER_LIMIT    (100)

// idx + K; leave idx wild
static const Binary unscaledArrayPat(opPlus, Terminal::get(opWild), Terminal::get(opWildIntConst));

static DFATypeRecovery s_type_recovery;

void DFATypeRecovery::dumpResults(StatementList& stmts, int iter)
{
    LOG_VERBOSE("%1 iterations", iter);

    for (Statement *s : stmts) {
        LOG_VERBOSE("%1", s); // Print the statement; has dest type

        // Now print type for each constant in this Statement
        std::list<std::shared_ptr<Const> >           lc;
        std::list<std::shared_ptr<Const> >::iterator cc;
        s->findConstants(lc);

        if (lc.size()) {
            for (cc = lc.begin(); cc != lc.end(); ++cc) {
                LOG_MSG("    %1, %2", (*cc)->getType()->getCtype(), *cc);
            }
        }

        // If s is a call, also display its return types
        CallStatement *call = dynamic_cast<CallStatement *>(s);

        if (s->isCall() && call) {
            ReturnStatement *rs = call->getCalleeReturn();

            if (rs == nullptr) {
                continue;
            }

            UseCollector *uc = call->getUseCollector();

            LOG_VERBOSE("  returns:");
            for (ReturnStatement::iterator rr = rs->begin(); rr != rs->end(); ++rr) {
                // Intersect the callee's returns with the live locations at the call, i.e. make sure that they
                // exist in *uc
                Assignment *assgn = dynamic_cast<Assignment *>(*rr);
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
        QString nam = prog->getGlobalName(K2);

        if (nam.isEmpty()) {
            nam = prog->newGlobalName(K2);
        }

        arr = Binary::get(opArrayIndex, Location::global(nam, pr), idx);

        if (s->searchAndReplace(scaledArrayPat, arr)) {
            if (s->isImplicit()) {
                // Register an array of appropriate type
                prog->markGlobalUsed(K2, ArrayType::get(((ImplicitAssign *)s)->getType()));
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

    SharedExp lhs = ((ImplicitAssign *)s)->getLeft();
    // Note: parameters are not explicit any more
    // if (lhs->isParam()) { // }

    bool       allZero = false;
    SharedExp slhs = lhs->clone()->removeSubscripts(allZero);
    SharedType iType = ((ImplicitAssign *)s)->getType();
    int i     = proc->getSignature()->findParam(slhs);

    if (i != -1) {
        proc->setParamType(i, iType);
    }
    else if (lhs->isMemOf()) {
        SharedExp sub = lhs->getSubExp1();

        if (sub->isIntConst()) {
            // We have a m[K] := -
                     Address K = sub->access<Const>()->getAddr();
            prog->markGlobalUsed(K, iType);
        }
    }
    else if (lhs->isGlobal()) {
        assert(std::dynamic_pointer_cast<Location>(lhs) != nullptr);
        QString gname = lhs->access<Const, 1>()->getStr();
        prog->setGlobalType(gname, iType);
    }
}


void DFATypeRecovery::recoverFunctionTypes(Function *)
{
    assert(false);
}


void DFATypeRecovery::dfaTypeAnalysis(Function *f)
{
    if (f->isLib()) {
        LOG_VERBOSE("Not using DFA type analysis on library function '%1'", f->getName());
        return;
    }

    UserProc *proc = static_cast<UserProc *>(f);
    Cfg      *cfg  = proc->getCFG();
    DataIntervalMap localsMap(proc); // map of all local variables of proc

    Boomerang::get()->alertDecompileDebugPoint(proc, "Before DFA type analysis");

    // First use the type information from the signature. Sometimes needed to split variables (e.g. argc as a
    // int and char* in sparc/switch_gcc)
    bool          ch = proc->getSignature()->dfaTypeAnalysis(cfg);
    StatementList stmts;
    proc->getStatements(stmts);

    int iter = 0;

    for (iter = 1; iter <= DFA_ITER_LIMIT; ++iter) {
        ch = false;

        for (Statement *stmt : stmts) {
            bool      thisCh  = false;
            Statement *before = nullptr;

            if (DEBUG_TA) {
                before = stmt->clone();
            }

            stmt->dfaTypeAnalysis(thisCh);

            if (thisCh) {
                ch = true;

                if (DEBUG_TA) {
                    LOG_VERBOSE("  Caused change:\n    FROM: %1\n    TO:   %2", before, stmt);
                }
            }

            if (DEBUG_TA) {
                delete before;
            }
        }

        if (!ch) {
            // No more changes: round robin algorithm terminates
            break;
        }
    }

    if (ch) {
        LOG_WARN("Iteration limit exceeded for dfaTypeAnalysis of procedure '%1'", proc->getName());
    }

    if (DEBUG_TA) {
        LOG_MSG("### Results for data flow based type analysis for %1 ###", proc->getName());
        dumpResults(stmts, iter);
        LOG_MSG("### End results for Data flow based Type Analysis for %1 ###", proc->getName());
    }

    // Now use the type information gathered

    Boomerang::get()->alertDecompileDebugPoint(proc, "Before other uses of DFA type analysis");
    proc->debugPrintAll("Before other uses of DFA type analysis");

    Prog *_prog = proc->getProg();

    for (Statement *s : stmts) {
        // 1) constants
        std::list<std::shared_ptr<Const> > lc;
        s->findConstants(lc);

        for (const std::shared_ptr<Const>& con : lc) {
            if (!con || con->getOper() == opStrConst) {
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
                    QString gloName = _prog->getGlobalName(addr);

                    if (!gloName.isEmpty()) {
                        Address   r = addr - _prog->getGlobalAddr(gloName);
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
                            opAddrOf, Binary::get(opArrayIndex, Location::global(_prog->getGlobalName(K), proc), idx));
                        // Beware of changing expressions in implicit assignments... map can become invalid
                        bool isImplicit = s->isImplicit();

                        if (isImplicit) {
                            cfg->removeImplicitAssign(((ImplicitAssign *)s)->getLeft());
                        }

                        if (!s->searchAndReplace(unscaledArrayPat, arr)) {
                            arr = nullptr; // remove if not emplaced in s
                        }

                        // s will likely have an m[a[array]], so simplify
                        s->simplifyAddr();

                        if (isImplicit) {
                            // Replace the implicit assignment entry. Note that s' lhs has changed
                            cfg->findImplicitAssign(((ImplicitAssign *)s)->getLeft());
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
                    con->setFlt(*(float *)&tmp);      // Reinterpret to float, then cast to double
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
                SharedExp lhs = ((Assignment *)s)->getLeft();

                if (lhs->isMemOf()) {
                    addrExp = lhs->getSubExp1();
                    typeExp = ((Assignment *)s)->getType();
                }
            }
            else {
                // Assume an implicit reference
                addrExp = ((ImpRefStatement *)s)->getAddressExp();

                if (addrExp->isTypedExp() && addrExp->access<TypedExp>()->getType()->resolvesToPointer()) {
                    addrExp = addrExp->getSubExp1();
                }

                typeExp = ((ImpRefStatement *)s)->getType();

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

            if (addrExp && proc->getSignature()->isAddrOfStackLocal(_prog, addrExp)) {
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

                const SharedType& ty = ((TypingStatement *)s)->getType();
                LOG_VERBOSE("In proc %1 adding addrExp %2 with type %3 to local table", proc->getName(), addrExp, ty);
                SharedExp loc_mem = Location::memOf(addrExp);
                localsMap.insertItem(Address(localAddressOffset), proc->lookupSym(loc_mem, ty), typeExp);
            }
        }
    }

    proc->debugPrintAll("After application of DFA type analysis");

    Boomerang::get()->alertDecompileDebugPoint(proc, "After DFA type analysis");
}


bool DFATypeRecovery::dfaTypeAnalysis(Signature *sig, Cfg *cfg)
{
    Q_UNUSED(sig);
    Q_UNUSED(cfg);
    assert(false);
    return false;
}


bool DFATypeRecovery::dfaTypeAnalysis(Statement *i)
{
    Q_UNUSED(i);
    assert(false);
    return false;
}


void UserProc::dfaTypeAnalysis()
{
    s_type_recovery.dfaTypeAnalysis(this);
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

SharedType VoidType::meetWith(SharedType other, bool& ch, bool /*bHighestPtr*/) const
{
    // void meet x = x
    ch |= !other->resolvesToVoid();
    return other->clone();
}


SharedType FuncType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((FuncType *)this)->shared_from_this();
    }

    // NOTE: at present, compares names as well as types and num parameters
    if (*this == *other) {
        return ((FuncType *)this)->shared_from_this();
    }

    return createUnion(other, ch, bHighestPtr);
}


SharedType IntegerType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((IntegerType *)this)->shared_from_this();
    }

    if (other->resolvesToInteger()) {
        std::shared_ptr<IntegerType> otherInt = other->as<IntegerType>();
        std::shared_ptr<IntegerType> result = std::dynamic_pointer_cast<IntegerType>(this->clone());

        // Signedness
        if (otherInt->signedness > 0) {
            result->signedness++;
        }
        else if (otherInt->signedness < 0) {
            result->signedness--;
        }

        ch |= ((result->signedness > 0) != (signedness > 0)); // Changed from signed to not necessarily signed
        ch |= ((result->signedness < 0) != (signedness < 0)); // Changed from unsigned to not necessarily unsigned

        // Size. Assume 0 indicates unknown size
        result->size = std::max(size, otherInt->size);
        ch  |= (result->size != size);

        return result;
    }
    else if (other->resolvesToSize()) {
        std::shared_ptr<IntegerType> result = std::dynamic_pointer_cast<IntegerType>(this->clone());
        std::shared_ptr<SizeType> other_sz = other->as<SizeType>();

        if (size == 0) { // Doubt this will ever happen
            result->size = other_sz->getSize();
            ch = true;
            return result;
        }

        if (size == other_sz->getSize()) {
            return result;
        }

        LOG_VERBOSE("Integer size %1 meet with SizeType size %2!", size, other_sz->getSize());

        result->size = std::max(size, other_sz->getSize());
        ch   = result->size != size;
        return result;
    }

    return createUnion(other, ch, bHighestPtr);
}


SharedType FloatType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((FloatType *)this)->shared_from_this();
    }

    if (other->resolvesToFloat() || other->resolvesToSize()) {
        const size_t newSize = std::max(size, other->getSize());
        ch |= (newSize != size);
        return FloatType::get(newSize);
    }

    return createUnion(other, ch, bHighestPtr);
}


SharedType BooleanType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid() || other->resolvesToBoolean()) {
        return ((BooleanType *)this)->shared_from_this();
    }

    return createUnion(other, ch, bHighestPtr);
}


SharedType CharType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid() || other->resolvesToChar()) {
        return ((CharType *)this)->shared_from_this();
    }

    // Also allow char to merge with integer
    if (other->resolvesToInteger()) {
        ch = true;
        return other->clone();
    }

    if (other->resolvesToSize() && (other->as<SizeType>()->getSize() == 8)) {
        return ((CharType *)this)->shared_from_this();
    }

    return createUnion(other, ch, bHighestPtr);
}


SharedType PointerType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((PointerType *)this)->shared_from_this();
    }

    if (other->resolvesToSize() && (other->as<SizeType>()->getSize() == STD_SIZE)) {
        return ((PointerType *)this)->shared_from_this();
    }

    if (!other->resolvesToPointer()) {
        // Would be good to understand class hierarchies, so we know if a* is the same as b* when b is a subclass of a
        return createUnion(other, ch, bHighestPtr);
    }

    auto otherPtr = other->as<PointerType>();

    if (pointsToAlpha() && !otherPtr->pointsToAlpha()) {
        ch = true;

        // Can't point to self; impossible to compare, print, etc
        if (otherPtr->getPointsTo() == shared_from_this()) {
            return VoidType::get(); // TODO: pointer to void at least ?
        }

        return PointerType::get(otherPtr->getPointsTo());
    }

    // We have a meeting of two pointers.
    SharedType thisBase  = points_to;
    SharedType otherBase = otherPtr->points_to;

    if (bHighestPtr) {
        // We want the greatest type of thisBase and otherBase
        if (thisBase->isSubTypeOrEqual(otherBase)) {
            return other->clone();
        }

        if (otherBase->isSubTypeOrEqual(thisBase)) {
            return ((PointerType *)this)->shared_from_this();
        }

        // There may be another type that is a superset of this and other; for now return void*
        return PointerType::get(VoidType::get());
    }

    // See if the base types will meet
    if (otherBase->resolvesToPointer()) {
        if (thisBase->resolvesToPointer() && (thisBase->as<PointerType>()->getPointsTo() == thisBase)) {
            LOG_VERBOSE("HACK! BAD POINTER 1");
        }

        if (otherBase->resolvesToPointer() && (otherBase->as<PointerType>()->getPointsTo() == otherBase)) {
            LOG_VERBOSE("HACK! BAD POINTER 2");
        }

        if (thisBase == otherBase) {                          // Note: compare pointers
            return ((PointerType *)this)->shared_from_this(); // Crude attempt to prevent stack overflow
        }

        if (*thisBase == *otherBase) {
            return ((PointerType *)this)->shared_from_this();
        }

        if (pointerDepth() == otherPtr->pointerDepth()) {
            SharedType fType = getFinalPointsTo();

            if (fType->resolvesToVoid()) {
                return other->clone();
            }

            SharedType ofType = otherPtr->getFinalPointsTo();

            if (ofType->resolvesToVoid()) {
                return ((PointerType *)this)->shared_from_this();
            }

            if (*fType == *ofType) {
                return ((PointerType *)this)->shared_from_this();
            }
        }
    }

    if (thisBase->isCompatibleWith(*otherBase)) {
        // meet recursively if the types are compatible
        return PointerType::get(points_to->meetWith(otherBase, ch, bHighestPtr));
    }

    // The bases did not meet successfully. Union the pointers.
    return createUnion(other, ch, bHighestPtr);
}


SharedType ArrayType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((ArrayType *)this)->shared_from_this();
    }

    if (other->resolvesToArray()) {
        auto       otherArr = other->as<ArrayType>();
        SharedType newBase  = BaseType->clone()->meetWith(otherArr->BaseType, ch, bHighestPtr);
        size_t newLength = m_length;

        if (*newBase != *BaseType) {
            ch        = true;
            newLength = convertLength(newBase);
        }

        newLength = std::min(newLength, other->as<ArrayType>()->getLength());
        return ArrayType::get(newBase, newLength);
    }

    if (*BaseType == *other) {
        return ((ArrayType *)this)->shared_from_this();
    }

    /*
     * checks if 'other' is compatible with the ArrayType, if it is
     * checks if other's 'completeness' is less then current BaseType, if it is, unchanged type is returned.
     * checks if sizes of BaseType and other match, if they do, checks if other is less complete ( SizeType vs NonSize type ),
     *  if that happens unchanged type is returned
     * then it clones the BaseType and tries to 'meetWith' with other, if this results in unchanged type, unchanged type is returned
     * otherwise a new ArrayType is returned, with it's size recalculated based on new BaseType
     */
    if (isCompatible(*other, false)) { // compatible with all ?
        size_t bitsize  = BaseType->getSize();
        size_t new_size = other->getSize();

        if (BaseType->isComplete() && !other->isComplete()) {
            // complete types win
            return std::const_pointer_cast<Type>(this->shared_from_this());
        }

        if (bitsize == new_size) {
            // same size, prefer Int/Float over SizeType
            if (!BaseType->isSize() && other->isSize()) {
                return std::const_pointer_cast<Type>(this->shared_from_this());
            }
        }

        auto bt = BaseType->clone();
        bool base_changed;
        auto res = bt->meetWith(other, base_changed);

        if (res == bt) {
            return std::const_pointer_cast<Type>(this->shared_from_this());
        }

        size_t new_length = m_length;

        if (m_length != NO_BOUND) {
            new_length = (m_length * bitsize) / new_size;
        }

        return ArrayType::get(res, new_length);
    }

    // Needs work?
    return createUnion(other, ch, bHighestPtr);
}


SharedType NamedType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    SharedType rt = resolvesTo();

    if (rt) {
        SharedType ret = rt->meetWith(other, ch, bHighestPtr);

        if (ret == rt) { // Retain the named type, much better than some compound type
            return ((NamedType *)this)->shared_from_this();
        }

        return ret;              // Otherwise, whatever the result is
    }

    if (other->resolvesToVoid()) {
        return ((NamedType *)this)->shared_from_this();
    }

    if (*this == *other) {
        return ((NamedType *)this)->shared_from_this();
    }

    return createUnion(other, ch, bHighestPtr);
}


SharedType CompoundType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((CompoundType *)this)->shared_from_this();
    }

    if (!other->resolvesToCompound()) {
        if (m_types[0]->isCompatibleWith(*other)) {
            // struct meet first element = struct
            return ((CompoundType *)this)->shared_from_this();
        }

        return createUnion(other, ch, bHighestPtr);
    }

    auto otherCmp = other->as<CompoundType>();

    if (otherCmp->isSuperStructOf(((CompoundType *)this)->shared_from_this())) {
        // The other structure has a superset of my struct's offsets. Preserve the names etc of the bigger struct.
        ch = true;
        return other;
    }

    if (isSubStructOf(otherCmp)) {
        // This is a superstruct of other
        ch = true;
        return ((CompoundType *)this)->shared_from_this();
    }

    if (*this == *other) {
        return ((CompoundType *)this)->shared_from_this();
    }

    // Not compatible structs. Create a union of both complete structs.
    // NOTE: may be possible to take advantage of some overlaps of the two structures some day.
    return createUnion(other, ch, bHighestPtr);
}


#ifdef PRINT_UNION
unsigned unionCount = 0;
#endif

SharedType UnionType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((UnionType *)this)->shared_from_this();
    }

    if (other->resolvesToUnion()) {
        if (this == other.get()) {                          // Note: pointer comparison
            return ((UnionType *)this)->shared_from_this(); // Avoid infinite recursion
        }

        std::shared_ptr<UnionType> otherUnion = other->as<UnionType>();
        std::shared_ptr<UnionType> result(UnionType::get());

        *result = *this;

        for (UnionElement elem : otherUnion->li) {
            result = result->meetWith(elem.type, ch, bHighestPtr)->as<UnionType>();
        }

        return result;
    }

    // Other is a non union type
    if (other->resolvesToPointer() && (other->as<PointerType>()->getPointsTo().get() == this)) {
        LOG_WARN("Attempt to union '%1' with pointer to self!", this->getCtype());
        return ((UnionType *)this)->shared_from_this();
    }

    //    int subtypes_count = 0;
    //    for (it = li.begin(); it != li.end(); ++it) {
    //        Type &v(*it->type);
    //        if(v.isCompound()) {
    //            subtypes_count += ((CompoundType &)v).getNumTypes();
    //        }
    //        else if(v.isUnion()) {
    //            subtypes_count += ((UnionType &)v).getNumTypes();
    //        }
    //        else
    //            subtypes_count+=1;
    //    }
    //    if(subtypes_count>9) {
    //        qDebug() << getCtype();
    //        qDebug() << other->getCtype();
    //        qDebug() << "*****";
    //    }

    // Match 'other' agains all fields of 'this' UnionType
    // if a field is found that requires no change to 'meet', this type is returned unchanged
    // if a new meetWith result is 'better' given simplistic type description length heuristic measure
    // then the meetWith result, and this types field iterator are stored.

    int bestMeetScore = INT_MAX;
    UnionEntrySet::const_iterator bestElem = li.end();

    for (auto it = li.begin(); it != li.end(); ++it) {
        SharedType v = it->type;

        if (!v->isCompatibleWith(*other)) {
            continue;
        }

        ch = false;
        SharedType meet_res = v->meetWith(other, ch, bHighestPtr);
        if (!ch) {
            // Fully compatible type alerady present in this union
            return ((UnionType *)this)->shared_from_this();
        }

        const int currentScore = meet_res->getCtype().size();
        if (currentScore < bestMeetScore) {
            // we have found a better match, store it
            bestElem = it;
            bestMeetScore = currentScore;
        }
    }


    std::shared_ptr<UnionType> result = UnionType::get();
    for (UnionEntrySet::const_iterator it = li.begin(); it != li.end(); it++) {
        if (it == bestElem) {
            // this is the element to be replaced
            continue;
        }

        result->addType(it->type, it->name);
    }

    UnionElement ne;
    if (bestElem != li.end()) {
        ne.name = bestElem->name;
        ne.type = bestElem->type->meetWith(other, ch, bHighestPtr); // we know this works because the types are compatible
    }
    else {
        // Other is not compatible with any of my component types. Add a new type.
        ne.name = QString("x%1").arg(++nextUnionNumber);
        ne.type = other->clone();
    }

    result->addType(ne.type, ne.name);
    ch = true;
    return result;
}


SharedType SizeType::meetWith(SharedType other, bool& ch, bool bHighestPtr) const
{
    if (other->resolvesToVoid()) {
        return ((SizeType *)this)->shared_from_this();
    }

    if (other->resolvesToSize()) {
        SharedType result = this->clone();
        if (other->as<SizeType>()->size != size) {
            LOG_VERBOSE("Size %1 meet with size %2!", size, other->as<SizeType>()->size);
        }
        result->setSize(std::max(result->getSize(), other->as<SizeType>()->getSize()));

        return result;
    }

    ch = true;

    if (other->resolvesToInteger()) {
        if (other->getSize() == 0) {
            other->setSize(size);
            return other->clone();
        }

        if (other->getSize() != size) {
            LOG_WARN("Size %1 meet with %2; allowing temporarily",
                     size, other->getCtype());
        }

        return other->clone();
    }

    return createUnion(other, ch, bHighestPtr);
}


SharedType Statement::meetWithFor(SharedType ty, SharedExp e, bool& ch)
{
    bool       thisCh  = false;
    SharedType typeFor = getTypeFor(e);

    assert(typeFor);
    SharedType newType = typeFor->meetWith(ty, thisCh);

    if (thisCh) {
        ch = true;
        setTypeFor(e, newType->clone());
    }

    return newType;
}


SharedType Type::createUnion(SharedType other, bool& ch, bool bHighestPtr /* = false */) const
{
    assert(!resolvesToUnion());     // `this' should not be a UnionType

    if (other->resolvesToUnion()) { // Put all the hard union logic in one place
        return other->meetWith(((Type *)this)->shared_from_this(), ch, bHighestPtr)->clone();
    }

    // Check for anytype meet compound with anytype as first element
    if (other->resolvesToCompound()) {
        auto       otherComp = other->as<CompoundType>();
        SharedType firstType = otherComp->getType((unsigned)0);

        if (firstType->isCompatibleWith(*this)) {
            // struct meet first element = struct
            return other->clone();
        }
    }

    // Check for anytype meet array of anytype
    if (other->resolvesToArray()) {
        auto       otherArr = other->as<ArrayType>();
        SharedType elemTy   = otherArr->getBaseType();

        if (elemTy->isCompatibleWith(*this)) {
            // x meet array[x] == array
            ch = true; // since 'this' type is not an array, but the returned type is
            return other->clone();
        }
    }

    char name[20];
#if PRINT_UNION
    if (unionCount == 999) {                         // Adjust the count to catch the one you want
        LOG_MSG("createUnion breakpokint"); // Note: you need two breakpoints (also in UnionType::meetWith)
    }
#endif
    sprintf(name, "x%d", ++nextUnionNumber);
    auto u = std::make_shared<UnionType>();
    u->addType(this->clone(), name);
    sprintf(name, "x%d", ++nextUnionNumber);
    u->addType(other->clone(), name);
    ch = true;
#if PRINT_UNION
    LOG_MSG("  %1 Created union from %2 and %3, result is %4",
            ++unionCount, getCtype(), other->getCtype(), u->getCtype());
#endif
    return u;
}


// Special operators for handling addition and subtraction in a data flow based type analysis
//                    ta=
//  tb=       alpha*     int      pi
//  beta*     bottom    void*    void*
//  int        void*     int      pi
//  pi         void*     pi       pi
SharedType sigmaSum(SharedType ta, SharedType tb)
{
    bool ch;

    if (ta->resolvesToPointer()) {
        if (tb->resolvesToPointer()) {
            return ta->createUnion(tb, ch);
        }

        return PointerType::get(VoidType::get());
    }

    if (ta->resolvesToInteger()) {
        if (tb->resolvesToPointer()) {
            return PointerType::get(VoidType::get());
        }

        return tb->clone();
    }

    if (tb->resolvesToPointer()) {
        return PointerType::get(VoidType::get());
    }

    return ta->clone();
}


//                    tc=
//  to=        beta*    int        pi
// alpha*    int        bottom    int
// int        void*    int        pi
// pi        pi        pi        pi
SharedType sigmaAddend(SharedType tc, SharedType to)
{
    bool ch;

    if (tc->resolvesToPointer()) {
        if (to->resolvesToPointer()) {
            return IntegerType::get(STD_SIZE, 0);
        }

        if (to->resolvesToInteger()) {
            return PointerType::get(VoidType::get());
        }

        return to->clone();
    }

    if (tc->resolvesToInteger()) {
        if (to->resolvesToPointer()) {
            return tc->createUnion(to, ch);
        }

        return to->clone();
    }

    if (to->resolvesToPointer()) {
        return IntegerType::get(STD_SIZE, 0);
    }

    return tc->clone();
}


//                    tc=
//  tb=        beta*    int        pi
// alpha*    bottom    void*    void*
// int        void*    int        pi
// pi        void*    int        pi
SharedType deltaMinuend(SharedType tc, SharedType tb)
{
    bool ch;

    if (tc->resolvesToPointer()) {
        if (tb->resolvesToPointer()) {
            return tc->createUnion(tb, ch);
        }

        return PointerType::get(VoidType::get());
    }

    if (tc->resolvesToInteger()) {
        if (tb->resolvesToPointer()) {
            return PointerType::get(VoidType::get());
        }

        return tc->clone();
    }

    if (tb->resolvesToPointer()) {
        return PointerType::get(VoidType::get());
    }

    return tc->clone();
}


//                    tc=
//  ta=        beta*    int        pi
// alpha*    int        void*    pi
// int        bottom    int        int
// pi        int        pi        pi
SharedType deltaSubtrahend(SharedType tc, SharedType ta)
{
    bool ch;

    if (tc->resolvesToPointer()) {
        if (ta->resolvesToPointer()) {
            return IntegerType::get(STD_SIZE, 0);
        }

        if (ta->resolvesToInteger()) {
            return tc->createUnion(ta, ch);
        }

        return IntegerType::get(STD_SIZE, 0);
    }

    if (tc->resolvesToInteger()) {
        if (ta->resolvesToPointer()) {
            return PointerType::get(VoidType::get());
        }
    }

    return ta->clone();
}


//          ta=
// tb=      alpha*  int        pi
// beta*    int     bottom    int
// int      void*   int        pi
// pi       pi      int        pi
SharedType deltaDifference(SharedType ta, SharedType tb)
{
    bool ch;

    if (ta->resolvesToPointer()) {
        if (tb->resolvesToPointer()) {
            return IntegerType::get(STD_SIZE, 0);
        }

        if (tb->resolvesToInteger()) {
            return PointerType::get(VoidType::get());
        }

        return tb->clone();
    }

    if (ta->resolvesToInteger()) {
        if (tb->resolvesToPointer()) {
            return ta->createUnion(tb, ch);
        }

        return IntegerType::get(STD_SIZE, 0);
    }

    if (tb->resolvesToPointer()) {
        return IntegerType::get(STD_SIZE, 0);
    }

    return ta->clone();
}


//    //    //    //    //    //    //    //    //    //    //
//                                        //
//    ascendType: draw type information    //
//        up the expression tree            //
//                                        //
//    //    //    //    //    //    //    //    //    //    //

SharedType Binary::ascendType()
{
    if (m_oper == opFlagCall) {
        return VoidType::get();
    }

    SharedType ta = subExp1->ascendType();
    SharedType tb = subExp2->ascendType();

    switch (m_oper)
    {
    case opPlus:
        return sigmaSum(ta, tb);

    // Do I need to check here for Array* promotion? I think checking in descendType is enough
    case opMinus:
        return deltaDifference(ta, tb);

    case opMult:
    case opDiv:
        return IntegerType::get(ta->getSize(), -1);

    case opMults:
    case opDivs:
    case opShiftRA:
        return IntegerType::get(ta->getSize(), +1);

    case opBitAnd:
    case opBitOr:
    case opBitXor:
    case opShiftR:
    case opShiftL:
        return IntegerType::get(ta->getSize(), 0);

    case opLess:
    case opGtr:
    case opLessEq:
    case opGtrEq:
    case opLessUns:
    case opGtrUns:
    case opLessEqUns:
    case opGtrEqUns:
        return BooleanType::get();

    case opFMinus:
    case opFPlus:
        return FloatType::get(ta->getSize());

    default:
        // Many more cases to implement
        return VoidType::get();
    }
}


// Constants and subscripted locations are at the leaves of the expression tree. Just return their stored types.
SharedType RefExp::ascendType()
{
    if (m_def == nullptr) {
        LOG_WARN("Null reference in '%1'", this->prints());
        return VoidType::get();
    }

    return m_def->getTypeFor(subExp1);
}


SharedType Const::ascendType()
{
    if (m_type->resolvesToVoid()) {
        switch (m_oper)
        {
        case opIntConst:
            // could be anything, Boolean, Character, we could be bit fiddling pointers for all we know - trentw
            break;

        case opLongConst:
            m_type = IntegerType::get(STD_SIZE * 2, 0);
            break;

        case opFltConst:
            m_type = FloatType::get(64);
            break;

        case opStrConst:
            m_type = PointerType::get(CharType::get());
            break;

        case opFuncConst:
            m_type = FuncType::get();          // More needed here?
            break;

        default:
            assert(0); // Bad Const
        }
    }

    return m_type;
}


// Can also find various terminals at the leaves of an expression tree
SharedType Terminal::ascendType()
{
    switch (m_oper)
    {
    case opPC:
        return IntegerType::get(STD_SIZE, -1);

    case opCF:
    case opZF:
        return BooleanType::get();

    case opDefineAll:
        return VoidType::get();

    case opFlags:
        return IntegerType::get(STD_SIZE, -1);

    default:
        LOG_WARN("Unknown type %1", this->toString());
        return VoidType::get();
    }
}


SharedType Unary::ascendType()
{
    SharedType ta = subExp1->ascendType();

    switch (m_oper)
    {
    case opMemOf:

        if (ta->resolvesToPointer()) {
            return ta->as<PointerType>()->getPointsTo();
        }
        else {
            return VoidType::get(); // NOT SURE! Really should be bottom
        }

        break;

    case opAddrOf:
        return PointerType::get(ta);

        break;

    default:
        break;
    }

    return VoidType::get();
}


SharedType Ternary::ascendType()
{
    switch (m_oper)
    {
    case opFsize:
        return FloatType::get(subExp2->access<Const>()->getInt());

    case opZfill:
    case opSgnEx:
        {
            int toSize = subExp2->access<Const>()->getInt();
            return Type::newIntegerLikeType(toSize, m_oper == opZfill ? -1 : 1);
        }

    default:
        break;
    }

    return VoidType::get();
}


SharedType TypedExp::ascendType()
{
    return type;
}


//    //    //    //    //    //    //    //    //    //    //
//                                        //
//    descendType: push type information    //
//        down the expression tree        //
//                                        //
//    //    //    //    //    //    //    //    //    //    //

void Binary::descendType(SharedType parentType, bool& ch, Statement *s)
{
    if (m_oper == opFlagCall) {
        return;
    }

    SharedType ta = subExp1->ascendType();
    SharedType tb = subExp2->ascendType();
    SharedType nt; // "New" type for certain operators
    // The following is an idea of Mike's that is not yet implemented well. It is designed to handle the situation
    // where the only reference to a local is where its address is taken. In the current implementation, it incorrectly
    // triggers with every ordinary local reference, causing esp to appear used in the final program

    switch (m_oper)
    {
    case opPlus:
        ta = ta->meetWith(sigmaAddend(parentType, tb), ch);
        subExp1->descendType(ta, ch, s);
        tb = tb->meetWith(sigmaAddend(parentType, ta), ch);
        subExp2->descendType(tb, ch, s);
        break;

    case opMinus:
        ta = ta->meetWith(deltaMinuend(parentType, tb), ch);
        subExp1->descendType(ta, ch, s);
        tb = tb->meetWith(deltaSubtrahend(parentType, ta), ch);
        subExp2->descendType(tb, ch, s);
        break;

    case opGtrUns:
    case opLessUns:
    case opGtrEqUns:
    case opLessEqUns:
        nt = IntegerType::get(ta->getSize(), -1); // Used as unsigned
        ta = ta->meetWith(nt, ch);
        tb = tb->meetWith(nt, ch);
        subExp1->descendType(ta, ch, s);
        subExp2->descendType(tb, ch, s);
        break;

    case opGtr:
    case opLess:
    case opGtrEq:
    case opLessEq:
        nt = IntegerType::get(ta->getSize(), +1); // Used as signed
        ta = ta->meetWith(nt, ch);
        tb = tb->meetWith(nt, ch);
        subExp1->descendType(ta, ch, s);
        subExp2->descendType(tb, ch, s);
        break;

    case opBitAnd:
    case opBitOr:
    case opBitXor:
    case opShiftR:
    case opShiftL:
    case opMults:
    case opDivs:
    case opShiftRA:
    case opMult:
    case opDiv:
        {
            int signedness;

            switch (m_oper)
            {
            case opBitAnd:
            case opBitOr:
            case opBitXor:
            case opShiftR:
            case opShiftL:
                signedness = 0;
                break;

            case opMults:
            case opDivs:
            case opShiftRA:
                signedness = 1;
                break;

            case opMult:
            case opDiv:
                signedness = -1;
                break;

            default:
                signedness = 0;
                break; // Unknown signedness
            }

            int parentSize = parentType->getSize();
            ta = ta->meetWith(IntegerType::get(parentSize, signedness), ch);
            subExp1->descendType(ta, ch, s);

            if ((m_oper == opShiftL) || (m_oper == opShiftR) || (m_oper == opShiftRA)) {
                // These operators are not symmetric; doesn't force a signedness on the second operand
                // FIXME: should there be a gentle bias twowards unsigned? Generally, you can't shift by negative
                // amounts.
                signedness = 0;
            }

            tb = tb->meetWith(IntegerType::get(parentSize, signedness), ch);
            subExp2->descendType(tb, ch, s);
            break;
        }

    default:
        // Many more cases to implement
        break;
    }
}


void RefExp::descendType(SharedType parentType, bool& ch, Statement *s)
{
    assert(getSubExp1());
    if (m_def == nullptr) {
        LOG_ERROR("Cannot descendType of expression '%1' since it does not have a defining statement!", getSubExp1());
        ch = false;
        return;
    }

    SharedType newType = m_def->meetWithFor(parentType, subExp1, ch);
    // In case subExp1 is a m[...]
    subExp1->descendType(newType, ch, s);
}


void Const::descendType(SharedType parentType, bool& ch, Statement *)
{
    bool thisCh = false;

    m_type = m_type->meetWith(parentType, thisCh);
    ch    |= thisCh;

    if (thisCh) {
        // May need to change the representation
        if (m_type->resolvesToFloat()) {
            if (m_oper == opIntConst) {
                m_oper = opFltConst;
                m_type = FloatType::get(64);
                float f = *(float *)&u.i;
                u.d = (double)f;
            }
            else if (m_oper == opLongConst) {
                m_oper = opFltConst;
                m_type = FloatType::get(64);
                double d = *(double *)&u.ll;
                u.d = d;
            }
        }

        // May be other cases
    }
}


// match m[l1{} + K] pattern
static bool match_l1_K(SharedExp in, std::vector<SharedExp>& matches)
{
    if (!in->isMemOf()) {
        return false;
    }

    auto as_bin = std::dynamic_pointer_cast<Binary>(in->getSubExp1());

    if (!as_bin || (as_bin->getOper() != opPlus)) {
        return false;
    }

    if (!as_bin->access<Exp, 2>()->isIntConst()) {
        return false;
    }

    if (!as_bin->access<Exp, 1>()->isSubscript()) {
        return false;
    }

    auto refexp = std::static_pointer_cast<RefExp>(as_bin->getSubExp1());

    if (!refexp->getSubExp1()->isLocation()) {
        return false;
    }

    matches.push_back(refexp);
    matches.push_back(as_bin->getSubExp2());
    return true;
}


void Unary::descendType(SharedType parentType, bool& ch, Statement *s)
{
    UserProc *owner_proc = s->getProc();
    auto     sig         = owner_proc != nullptr ? owner_proc->getSignature() : nullptr;
    Prog     *prog       = owner_proc->getProg();

    std::vector<SharedExp> matches;

    switch (m_oper)
    {
    case opMemOf:
        {
            auto as_bin = std::dynamic_pointer_cast<Binary>(subExp1);

            // Check for m[x*K1 + K2]: array with base K2 and stride K1
            if (as_bin && (as_bin->getOper() == opPlus) && (as_bin->getSubExp1()->getOper() == opMult) &&
                as_bin->getSubExp2()->isIntConst() &&
                as_bin->getSubExp1()->getSubExp2()->isIntConst()) {
                SharedExp leftOfPlus = as_bin->getSubExp1();
                // We would expect the stride to be the same size as the base type
                size_t stride = leftOfPlus->access<Const, 2>()->getInt();

                if (DEBUG_TA && (stride * 8 != parentType->getSize())) {
                    LOG_WARN("Type WARNING: apparent array reference at %1 has stride %2 bits, but parent type %3 has size %4",
                             shared_from_this(), stride * 8, parentType->getCtype(), parentType->getSize());
                }

                // The index is integer type
                SharedExp x = leftOfPlus->getSubExp1();
                x->descendType(IntegerType::get(parentType->getSize(), 0), ch, s);
                // K2 is of type <array of parentType>
                auto    constK2 = subExp1->access<Const, 2>();
                        Address intK2   = Address(constK2->getInt()); // TODO: use getAddr ?
                constK2->descendType(prog->makeArrayType(intK2, parentType), ch, s);
            }
            else if (match_l1_K(shared_from_this(), matches)) {
                // m[l1 + K]
                auto       l1     = std::static_pointer_cast<Location>(matches[0]->access<Location, 1>());
                SharedType l1Type = l1->ascendType();
                int        K      = matches[1]->access<Const>()->getInt();

                if (l1Type->resolvesToPointer()) {
                    // This is a struct reference m[ptr + K]; ptr points to the struct and K is an offset into it
                    // First find out if we already have struct information
                    SharedType st(l1Type->as<PointerType>()->getPointsTo());

                    if (st->resolvesToCompound()) {
                        auto ct = st->as<CompoundType>();

                        if (ct->isGeneric()) {
                            ct->updateGenericMember(K, parentType, ch);
                        }
                        else {
                            // would like to force a simplify here; I guess it will happen soon enough
                        }
                    }
                    else {
                        // Need to create a generic stuct with a least one member at offset K
                        auto ct = CompoundType::get(true);
                        ct->updateGenericMember(K, parentType, ch);
                    }
                }
                else {
                    // K must be the pointer, so this is a global array
                    // FIXME: finish this case
                }

                // FIXME: many other cases
            }
            else {
                subExp1->descendType(PointerType::get(parentType), ch, s);
            }

            break;
        }

    case opAddrOf:

        if (parentType->resolvesToPointer()) {
            subExp1->descendType(parentType->as<PointerType>()->getPointsTo(), ch, s);
        }

        break;

    case opGlobal:
        {
            Prog       *_prog = s->getProc()->getProg();
            QString    name   = subExp1->access<Const>()->getStr();
            SharedType ty     = _prog->getGlobalType(name);

            if (ty) {
                ty = ty->meetWith(parentType, ch);

                if (ch) {
                    _prog->setGlobalType(name, ty);
                }
            }

            break;
        }

    default:
        break;
    }
}


void Ternary::descendType(SharedType /*parentType*/, bool& ch, Statement *s)
{
    switch (m_oper)
    {
    case opFsize:
        subExp3->descendType(FloatType::get(subExp1->access<Const>()->getInt()), ch, s);
        break;

    case opZfill:
    case opSgnEx:
        {
            int        fromSize = subExp1->access<Const>()->getInt();
            SharedType fromType;
            fromType = Type::newIntegerLikeType(fromSize, m_oper == opZfill ? -1 : 1);
            subExp3->descendType(fromType, ch, s);
            break;
        }

    default:
        break;
    }
}


void TypedExp::descendType(SharedType, bool &, Statement *)
{
}


void Terminal::descendType(SharedType, bool &, Statement *)
{
}


/// Data flow based type analysis.
/// Meet the parameters with their current types.
/// \returns true if a change
bool Signature::dfaTypeAnalysis(Cfg *cfg)
{
    bool ch = false;

    for (std::shared_ptr<Parameter>& it : m_params) {
        // Parameters should be defined in an implicit assignment
        Statement *def = cfg->findImplicitParamAssign(it.get());

        if (def) { // But sometimes they are not used, and hence have no implicit definition
            bool thisCh = false;
            def->meetWithFor(it->getType(), it->getExp(), thisCh);

            if (thisCh) {
                ch = true;

                if (DEBUG_TA) {
                    LOG_MSG("  sig caused change: %1 %2", it->getType()->getCtype(), it->getName());
                }
            }
        }
    }

    return ch;
}


// Note: to prevent infinite recursion, CompoundType, ArrayType, and UnionType implement this function as a delegation
// to isCompatible()
bool Type::isCompatibleWith(const Type& other, bool all /* = false */) const
{
    if (other.resolvesToCompound() || other.resolvesToArray() || other.resolvesToUnion()) {
        return other.isCompatible(*this, all);
    }

    return isCompatible(other, all);
}


bool VoidType::isCompatible(const Type& /*other*/, bool /*all*/) const
{
    return true; // Void is compatible with any type
}


bool SizeType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    size_t otherSize = other.getSize();

    if (other.resolvesToFunc()) {
        return false;
    }

    // FIXME: why is there a test for size 0 here?
    // This is because some signatures leave us with 0-sized NamedType -> using GLEnum when it was not defined.
    if (otherSize == size) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToArray()) {
        return isCompatibleWith(*((const ArrayType&)other).getBaseType());
    }

    // return false;
    // For now, size32 and double will be considered compatible (helps test/pentium/global2)
    return false;
}


bool IntegerType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToInteger()) {
        return true;
    }

    if (other.resolvesToChar()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToSize() && (((const SizeType&)other).getSize() == size)) {
        return true;
    }

    return false;
}


bool FloatType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToFloat()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToArray()) {
        return isCompatibleWith(*((const ArrayType&)other).getBaseType());
    }

    if (other.resolvesToSize() && (((const SizeType&)other).getSize() == size)) {
        return true;
    }

    return false;
}


bool CharType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToChar()) {
        return true;
    }

    if (other.resolvesToInteger()) {
        return true;
    }

    if (other.resolvesToSize() && (((const SizeType&)other).getSize() == 8)) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToArray()) {
        return isCompatibleWith(*((const ArrayType&)other).getBaseType());
    }

    return false;
}


bool BooleanType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToBoolean()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToSize() && (((const SizeType&)other).getSize() == 1)) {
        return true;
    }

    return false;
}


bool FuncType::isCompatible(const Type& other, bool /*all*/) const
{
    assert(signature);

    if (other.resolvesToVoid()) {
        return true;
    }

    if (*this == other) {
        return true; // MVE: should not compare names!
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToSize() && (((const SizeType&)other).getSize() == STD_SIZE)) {
        return true;
    }

    if (other.resolvesToFunc()) {
        assert(other.as<FuncType>()->signature);

        if (*other.as<FuncType>()->signature == *signature) {
            return true;
        }
    }

    return false;
}


bool PointerType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (other.resolvesToSize() && (((const SizeType&)other).getSize() == STD_SIZE)) {
        return true;
    }

    if (!other.resolvesToPointer()) {
        return false;
    }

    return points_to->isCompatibleWith(*other.as<PointerType>()->points_to);
}


bool NamedType::isCompatible(const Type& other, bool /*all*/) const
{
    if (other.isNamed() && (name == ((const NamedType&)other).getName())) {
        return true;
    }

    SharedType resTo = resolvesTo();

    if (resTo) {
        return resolvesTo()->isCompatibleWith(other);
    }

    if (other.resolvesToVoid()) {
        return true;
    }

    return(*this == other);
}


bool ArrayType::isCompatible(const Type& other, bool all) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToArray() && BaseType->isCompatibleWith(*other.as<ArrayType>()->BaseType)) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (!all && BaseType->isCompatibleWith(other)) {
        return true; // An array of x is compatible with x
    }

    return false;
}


bool UnionType::isCompatible(const Type& other, bool all) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        if (this == &other) { // Note: pointer comparison
            return true;      // Avoid infinite recursion
        }

        const UnionType& otherUnion((const UnionType&)other);

        // Unions are compatible if one is a subset of the other
        if (li.size() < otherUnion.li.size()) {
            for (const UnionElement& e : li) {
                if (!otherUnion.isCompatible(*e.type, all)) {
                    return false;
                }
            }
        }
        else {
            for (const UnionElement& e : otherUnion.li) {
                if (!isCompatible(*e.type, all)) {
                    return false;
                }
            }
        }

        return true;
    }

    // Other is not a UnionType
    for (const UnionElement& e : li) {
        if (other.isCompatibleWith(*e.type, all)) {
            return true;
        }
    }

    return false;
}


bool CompoundType::isCompatible(const Type& other, bool all) const
{
    if (other.resolvesToVoid()) {
        return true;
    }

    if (other.resolvesToUnion()) {
        return other.isCompatibleWith(*this);
    }

    if (!other.resolvesToCompound()) {
        // Used to always return false here. But in fact, a struct is compatible with its first member (if all is false)
        return !all && m_types[0]->isCompatibleWith(other);
    }

    auto   otherComp = other.as<CompoundType>();
    size_t n         = otherComp->getNumTypes();

    if (n != m_types.size()) {
        return false; // Is a subcompound compatible with a supercompound?
    }

    for (size_t i = 0; i < n; i++) {
        if (!m_types[i]->isCompatibleWith(*otherComp->m_types[i])) {
            return false;
        }
    }

    return true;
}


bool Type::isSubTypeOrEqual(SharedType other)
{
    if (resolvesToVoid()) {
        return true;
    }

    if (*this == *other) {
        return true;
    }

    if (this->resolvesToCompound() && other->resolvesToCompound()) {
        return this->as<CompoundType>()->isSubStructOf(other);
    }

    // Not really sure here
    return false;
}


SharedType Type::dereference()
{
    if (resolvesToPointer()) {
        return as<PointerType>()->getPointsTo();
    }

    if (resolvesToUnion()) {
        return as<UnionType>()->dereferenceUnion();
    }

    return VoidType::get(); // Can't dereference this type. Note: should probably be bottom
}


// Dereference this union. If it is a union of pointers, return a union of the dereferenced items. Else return VoidType
// (note: should probably be bottom)
SharedType UnionType::dereferenceUnion()
{
    auto ret = UnionType::get();
    char name[20];

    UnionEntrySet::iterator it;

    for (it = li.begin(); it != li.end(); ++it) {
        SharedType elem = it->type->dereference();

        if (elem->resolvesToVoid()) {
            return elem; // Return void for the whole thing
        }

        sprintf(name, "x%d", ++nextUnionNumber);
        ret->addType(elem->clone(), name);
    }

    return ret;
}
