#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "CallStatement.h"


#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/core/Boomerang.h"
#include "boomerang/db/proc/UserProc.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/statements/Assign.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/visitor/Localiser.h"
#include "boomerang/db/visitor/ImplicitConverter.h"
#include "boomerang/db/visitor/StmtImplicitConverter.h"
#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/db/visitor/StmtVisitor.h"
#include "boomerang/db/visitor/StmtExpVisitor.h"
#include "boomerang/db/visitor/StmtModifier.h"
#include "boomerang/db/visitor/StmtPartModifier.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/type/type/CharType.h"
#include "boomerang/type/type/FloatType.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/type/type/FuncType.h"
#include "boomerang/util/Log.h"



/// A helper class for updateArguments. It just dishes out a new argument
/// from one of the three sources:
///   the signature,
///   the callee parameters,
///   or the defCollector in the call
class ArgSourceProvider
{
public:
    enum Src
    {
        SRC_LIB,
        SRC_CALLEE,
        SRC_COL
    };

public:
    ArgSourceProvider(CallStatement *call);
    SharedExp nextArgLoc();              // Get the next location (not subscripted)
    SharedType curType(SharedExp e);     // Get the current location's type
    bool exists(SharedExp loc);          // True if the given location (not subscripted) exists as a source
    SharedExp localise(SharedExp e);     // Localise to this call if necessary

public:
    Src src;
    CallStatement *call;
    int i, n;                       // For SRC_LIB
    std::shared_ptr<Signature> callSig;
    StatementList::iterator pp;     // For SRC_CALLEE
    StatementList *calleeParams;
    DefCollector::iterator cc;      // For SRC_COL
    DefCollector *defCol;
};


ArgSourceProvider::ArgSourceProvider(CallStatement *_call)
    : call(_call)
{
    Function *procDest = call->getDestProc();

    if (procDest && procDest->isLib()) {
        src     = SRC_LIB;
        callSig = call->getSignature();
        n       = callSig->getNumParams();
        i       = 0;
    }
    else if (call->getCalleeReturn() != nullptr) {
        src          = SRC_CALLEE;
        calleeParams = &static_cast<UserProc *>(procDest)->getParameters();
        pp           = calleeParams->begin();
    }
    else {
        std::shared_ptr<Signature> destSig;

        if (procDest) {
            destSig = procDest->getSignature();
        }

        if (destSig && destSig->isForced()) {
            src     = SRC_LIB;
            callSig = destSig;
            n       = callSig->getNumParams();
            i       = 0;
        }
        else {
            src    = SRC_COL;
            defCol = call->getDefCollector();
            cc     = defCol->begin();
        }
    }
}


SharedExp ArgSourceProvider::nextArgLoc()
{
    SharedExp s;
    bool      allZero;

    switch (src)
    {
    case SRC_LIB:

        if (i == n) {
            return nullptr;
        }

        s = callSig->getParamExp(i++)->clone();
        s->removeSubscripts(allZero);     // e.g. m[sp{-} + 4] -> m[sp + 4]
        call->localiseComp(s);
        return s;

    case SRC_CALLEE:

        if (pp == calleeParams->end()) {
            return nullptr;
        }

        s = static_cast<Assignment *>(*pp++)->getLeft()->clone();
        s->removeSubscripts(allZero);
        call->localiseComp(s);     // Localise the components. Has the effect of translating into
        // the contect of this caller
        return s;

    case SRC_COL:

        if (cc == defCol->end()) {
            return nullptr;
        }

        // Give the location, i.e. the left hand side of the assignment
        return static_cast<Assign *>(*cc++)->getLeft();
    }

    return nullptr;     // Suppress warning
}


SharedExp ArgSourceProvider::localise(SharedExp e)
{
    if (src == SRC_COL) {
        // Provide the RHS of the current assignment
        SharedExp ret = static_cast<Assign *>(*std::prev(cc))->getRight();
        return ret;
    }

    // Else just use the call to localise
    return call->localiseExp(e);
}


SharedType ArgSourceProvider::curType(SharedExp e)
{
    Q_UNUSED(e);

    switch (src)
    {
    case SRC_LIB:
        return callSig->getParamType(i - 1);

    case SRC_CALLEE:
        {
            SharedType ty = static_cast<Assignment *>(*std::prev(pp))->getType();
            return ty;
        }

    case SRC_COL:
        {
            // Mostly, there won't be a type here, I would think...
            SharedType ty = (*std::prev(cc))->getType();
            return ty;
        }
    }

    return nullptr;     // Suppress warning
}


bool ArgSourceProvider::exists(SharedExp e)
{
    bool allZero;

    switch (src)
    {
    case SRC_LIB:

        if (callSig->hasEllipsis()) {
            // FIXME: for now, just don't check
            return true;
        }

        for (i = 0; i < n; i++) {
            SharedExp sigParam = callSig->getParamExp(i)->clone();
            sigParam->removeSubscripts(allZero);
            call->localiseComp(sigParam);

            if (*sigParam == *e) {
                return true;
            }
        }

        return false;

    case SRC_CALLEE:

        for (pp = calleeParams->begin(); pp != calleeParams->end(); ++pp) {
            SharedExp par = static_cast<Assignment *>(*pp)->getLeft()->clone();
            par->removeSubscripts(allZero);
            call->localiseComp(par);

            if (*par == *e) {
                return true;
            }
        }

        return false;

    case SRC_COL:
        return defCol->existsOnLeft(e);
    }

    return false;     // Suppress warning
}


CallStatement::CallStatement()
    : m_returnAfterCall(false)
    , m_calleeReturn(nullptr)
{
    m_kind      = StmtType::Call;
    m_procDest  = nullptr;
    m_signature = nullptr;
}


CallStatement::~CallStatement()
{
    qDeleteAll(m_defines);
}


int CallStatement::findDefine(SharedExp e)
{
    int i = 0;

    for (StatementList::const_iterator rr = m_defines.begin(); rr != m_defines.end(); ++rr, ++i) {
        SharedConstExp ret = (dynamic_cast<Assignment *>(*rr))->getLeft();

        if (*ret == *e) {
            return i;
        }
    }

    return -1;
}


SharedExp CallStatement::getProven(SharedExp e)
{
    if (m_procDest) {
        return m_procDest->getProven(e);
    }

    return nullptr;
}


void CallStatement::localiseComp(SharedExp e)
{
    if (e->isMemOf()) {
        e->setSubExp1(localiseExp(e->getSubExp1()));
    }
}


SharedExp CallStatement::localiseExp(SharedExp e)
{
    if (!m_defCol.isInitialised()) {
        return e;     // Don't attempt to subscript if the data flow not started yet
    }

    Localiser l(this);
    e = e->clone()->accept(&l);

    return e;
}


SharedExp CallStatement::findDefFor(SharedExp e) const
{
    return m_defCol.findDefFor(e);
}


SharedType CallStatement::getArgumentType(int i) const
{
    assert(Util::inRange(i, 0, getNumArguments()));
    StatementList::const_iterator aa = std::next(m_arguments.begin(), i);
    Assign *assign = dynamic_cast<Assign *>(*aa);
    assert(assign != nullptr);

    return assign->getType();
}


void CallStatement::setArgumentType(int i, SharedType ty)
{
    assert(Util::inRange(i, 0, getNumArguments()));
    StatementList::const_iterator aa = std::next(m_arguments.begin(), i);
    Assign *assign = dynamic_cast<Assign *>(*aa);
    assert(assign != nullptr);
    assign->setType(ty);
}


void CallStatement::setArguments(const StatementList& args)
{
    qDeleteAll(m_arguments);
    m_arguments.clear();
    m_arguments.append(args);

    for (StatementList::iterator ll = m_arguments.begin(); ll != m_arguments.end(); ++ll) {
        Assign *asgn = dynamic_cast<Assign *>(*ll);
        if (asgn) {
            asgn->setProc(m_proc);
            asgn->setBB(m_bb);
        }
    }
}


void CallStatement::setSigArguments()
{
    if (m_signature) {
        return;     // Already done
    }

    if (m_procDest == nullptr) {
        // FIXME: Need to check this
        return;
    }

    // Clone here because each call to procDest could have a different signature, modified by ellipsisProcessing
    m_signature = m_procDest->getSignature()->clone();
    m_procDest->addCaller(this);

    if (!m_procDest->isLib()) {
        return;     // Using dataflow analysis now
    }

    int n = m_signature->getNumParams();

    qDeleteAll(m_arguments);
    m_arguments.clear();

    for (int i = 0; i < n; i++) {
        SharedExp e = m_signature->getArgumentExp(i);
        assert(e);
        auto l = std::dynamic_pointer_cast<Location>(e);

        if (l) {
            l->setProc(m_proc);     // Needed?
        }

        Assign *as = new Assign(m_signature->getParamType(i)->clone(),
                                e->clone(),
                                e->clone());
        as->setProc(m_proc);
        as->setBB(m_bb);
        as->setNumber(m_number);     // So fromSSAForm will work later. But note: this call is probably not numbered yet!
        // as->setParent(this);
        m_arguments.append(as);
    }

    // initialize returns
    // FIXME: anything needed here?
}


bool CallStatement::search(const Exp& pattern, SharedExp& result) const
{
    if (GotoStatement::search(pattern, result)) {
        return true;
    }

    for (const Statement *stmt : m_defines) {
        if (stmt->search(pattern, result)) {
            return true;
        }
    }

    for (const Statement *stmt : m_arguments) {
        if (stmt->search(pattern, result)) {
            return true;
        }
    }

    return false;
}


bool CallStatement::searchAndReplace(const Exp& pattern, SharedExp replace, bool cc)
{
    bool change = GotoStatement::searchAndReplace(pattern, replace, cc);

    // FIXME: MVE: Check if we ever want to change the LHS of arguments or defines...
    for (Statement *ss : m_defines) {
        change |= ss->searchAndReplace(pattern, replace, cc);
    }

    for (Statement *ss : m_arguments) {
        change |= ss->searchAndReplace(pattern, replace, cc);
    }

    if (cc) {
        DefCollector::iterator dd;

        for (dd = m_defCol.begin(); dd != m_defCol.end(); ++dd) {
            change |= (*dd)->searchAndReplace(pattern, replace, cc);
        }
    }

    return change;
}


bool CallStatement::searchAll(const Exp& pattern, std::list<SharedExp>& result) const
{
    bool found = GotoStatement::searchAll(pattern, result);

    for (auto ss = m_defines.begin(); ss != m_defines.end(); ++ss) {
        if ((*ss)->searchAll(pattern, result)) {
            found = true;
        }
    }

    for (const Statement *ss : m_arguments) {
        if (ss->searchAll(pattern, result)) {
            found = true;
        }
    }

    return found;
}


void CallStatement::print(QTextStream& os, bool html) const
{
    os << qSetFieldWidth(4) << m_number << qSetFieldWidth(0) << " ";

    if (html) {
        os << "</td><td>";
        os << "<a name=\"stmt" << m_number << "\">";
    }

    // Define(s), if any
    if (m_defines.size() > 0) {
        if (m_defines.size() > 1) {
            os << "{";
        }

        bool first = true;

        for (StatementList::const_iterator rr = m_defines.begin(); rr != m_defines.end(); ++rr) {
            assert((*rr)->isAssignment());
            Assignment *as = static_cast<Assignment *>(*rr);

            if (first) {
                first = false;
            }
            else {
                os << ", ";
            }

            os << "*" << as->getType() << "* " << as->getLeft();

            if (as->isAssign()) {
                Assign *a = dynamic_cast<Assign *>(as);
                if (a) {
                    os << " := " << a->getRight();
                }
            }
        }

        if (m_defines.size() > 1) {
            os << "}";
        }

        os << " := ";
    }
    else if (isChildless()) {
        if (html) {
            os << "&lt;all&gt; := ";
        }
        else {
            os << "<all> := ";
        }
    }

    os << "CALL ";

    if (m_procDest) {
        os << m_procDest->getName();
    }
    else if (m_dest == nullptr) {
        os << "*no dest*";
    }
    else {
        if (m_dest->isIntConst()) {
            os << "0x" << QString::number(m_dest->access<Const>()->getInt(), 16);
        }
        else {
            m_dest->print(os, html);     // Could still be an expression
        }
    }

    // Print the actual arguments of the call
    if (isChildless()) {
        if (html) {
            os << "(&lt;all&gt;)";
        }
        else {
            os << "(<all>)";
        }
    }
    else {
        os << "(\n";

        for (const Statement *aa : m_arguments) {
            os << "                ";
            const Assignment *a = dynamic_cast<const Assignment *>(aa);
            if (a) {
                a->printCompact(os, html);
            }
            os << "\n";
        }

        os << "              )";
    }

    // Collected reaching definitions
    if (html) {
        os << "<br>";
    }
    else {
        os << "\n              ";
    }

    os << "Reaching definitions: ";
    m_defCol.print(os, html);

    if (html) {
        os << "<br>";
    }
    else {
        os << "\n              ";
    }

    os << "Live variables: ";
    m_useCol.print(os, html);

    if (html) {
        os << "</a></td>";
    }
}


void CallStatement::setReturnAfterCall(bool b)
{
    m_returnAfterCall = b;
}


bool CallStatement::isReturnAfterCall() const
{
    return m_returnAfterCall;
}


Statement *CallStatement::clone() const
{
    CallStatement *ret = new CallStatement();

    ret->m_dest       = m_dest->clone();
    ret->m_isComputed = m_isComputed;


    for (const Statement *stmt : m_arguments) {
        ret->m_arguments.append(stmt->clone());
    }

    for (const Statement *stmt : m_defines) {
        ret->m_defines.append(stmt->clone());
    }

    // Statement members
    ret->m_bb = m_bb;
    ret->m_proc   = m_proc;
    ret->m_number = m_number;
    return ret;
}


bool CallStatement::accept(StmtVisitor *visitor)
{
    return visitor->visit(this);
}


Function *CallStatement::getDestProc()
{
    return m_procDest;
}


const Function *CallStatement::getDestProc() const
{
    return m_procDest;
}


void CallStatement::setDestProc(Function *dest)
{
    assert(dest);
    // assert(procDest == nullptr);        // No: not convenient for unit testing
    m_procDest = dest;
}


void CallStatement::generateCode(ICodeGenerator *gen, const BasicBlock *parentBB)
{
    Function *dest = getDestProc();

    if ((dest == nullptr) && isComputed()) {
        gen->addIndCallStatement(m_dest, m_arguments, *calcResults());
        return;
    }

    std::unique_ptr<StatementList> results = calcResults();
    assert(dest);

    if (SETTING(noDecompile)) {
        if (m_procDest->getSignature()->getNumReturns() > 0) {
            Assign *as = new Assign(IntegerType::get(STD_SIZE),
                                    Unary::get(opRegOf, Const::get(24)),
                                    Unary::get(opRegOf, Const::get(24)));
            as->setProc(m_proc);
            as->setBB(const_cast<BasicBlock *>(parentBB));
            results->append(as);
        }

        // some hacks
        if ((dest->getName() == "printf") || (dest->getName() == "scanf")) {
            for (int i = 1; i < 3; i++) {
                SharedExp e = m_signature->getArgumentExp(i);
                assert(e);
                auto l = std::dynamic_pointer_cast<Location>(e);

                if (l) {
                    l->setProc(m_proc);     // Needed?
                }

                Assign *as = new Assign(m_signature->getParamType(i),
                                        e->clone(),
                                        e->clone());
                as->setProc(m_proc);
                as->setBB(const_cast<BasicBlock *>(parentBB));
                as->setNumber(m_number);     // So fromSSAForm will work later
                m_arguments.append(as);
            }
        }
    }

    if (dest->isLib() && !dest->getSignature()->getPreferredName().isEmpty()) {
        gen->addCallStatement(dest, dest->getSignature()->getPreferredName(), m_arguments, *results);
    }
    else {
        gen->addCallStatement(dest, dest->getName(), m_arguments, *results);
    }
}

void CallStatement::simplify()
{
    GotoStatement::simplify();

    for (Statement *ss : m_arguments) {
        ss->simplify();
    }

    for (Statement *ss : m_defines) {
        ss->simplify();
    }
}


bool CallStatement::usesExp(const Exp& e) const
{
    if (GotoStatement::usesExp(e)) {
        return true;
    }

    for (const Statement *ss : m_arguments) {
        if (ss->usesExp(e)) {
            return true;
        }
    }

    for (const Statement *ss : m_defines) {
        if (ss->usesExp(e)) {
            return true;
        }
    }

    return false;
}


void CallStatement::getDefinitions(LocationSet& defs) const
{
    for (auto dd = m_defines.begin(); dd != m_defines.end(); ++dd) {
        defs.insert(static_cast<Assignment *>(*dd)->getLeft());
    }

    // Childless calls are supposed to define everything.
    // In practice they don't really define things like %pc,
    // so we need some extra logic in getTypeFor()
    if (isChildless() && !SETTING(assumeABI)) {
        defs.insert(Terminal::get(opDefineAll));
    }
}


bool CallStatement::convertToDirect()
{
    if (!m_isComputed) {
        return false;
    }

    bool      convertIndirect = false;
    SharedExp e = m_dest;

    if (m_dest->isSubscript()) {
        Statement *def = e->access<RefExp>()->getDef();

        if (def && !def->isImplicit()) {
            return false;     // If an already defined global, don't convert
        }

        e = e->getSubExp1();
    }

    if ((e->getOper() == opArrayIndex) && e->getSubExp2()->isIntConst() && (e->access<Const, 2>()->getInt() == 0)) {
        e = e->getSubExp1();
    }

    // Can actually have name{0}[0]{0} !!
    if (e->isSubscript()) {
        e = e->access<RefExp, 1>();
    }

    if (e->isIntConst()) {
        // ADDRESS u = (ADDRESS)((Const*)e)->getInt();
        // Just convert it to a direct call!
        // FIXME: to be completed
    }
    else if (e->isMemOf()) {
        // It might be a global that has not been processed yet
        SharedExp sub = e->getSubExp1();

        if (sub->isIntConst()) {
            // m[K]: convert it to a global right here
            Address u = Address(sub->access<Const>()->getInt());
            m_proc->getProg()->markGlobalUsed(u);
            QString nam = m_proc->getProg()->newGlobalName(u);
            e      = Location::global(nam, m_proc);
            m_dest = RefExp::get(e, nullptr);
        }
    }

    if (!e->isGlobal()) {
        return false;
    }

    QString nam     = e->access<Const, 1>()->getStr();
    Prog    *prog   = m_proc->getProg();
    Address gloAddr = prog->getGlobalAddr(nam);
    Address dest    = Address(prog->readNative4(gloAddr));

    // We'd better do some limit checking on the value.
    // This does not guarantee that it's a valid proc pointer,
    // but it may help
    if ((dest < prog->getLimitTextLow()) || (dest > prog->getLimitTextHigh())) {
        return false;     // Not a valid proc pointer
    }

    Function *p       = prog->findFunction(nam);
    bool     bNewProc = p == nullptr;

    if (bNewProc) {
        p = prog->createFunction(dest);
    }

    LOG_VERBOSE("%1 procedure for call to global '%2' is %3",
                (bNewProc ? "new" : "existing"), nam, p->getName());

    // we need to:
    // 1) replace the current return set with the return set of the new procDest
    // 2) call fixCallBypass (now fixCallAndPhiRefs) on the enclosing procedure
    // 3) fix the arguments (this will only affect the implicit arguments, the regular arguments should
    //    be empty at this point)
    // 3a replace current arguments with those of the new proc
    // 3b copy the signature from the new proc
    // 4) change this to a non-indirect call
    m_procDest = p;
    auto sig = p->getSignature();
    // pDest is currently still global5{-}, but we may as well make it a constant now, since that's how it will be
    // treated now
    m_dest = Const::get(dest);

    // 1
    // 2
    m_proc->fixCallAndPhiRefs();

    // 3
    // 3a Do the same with the regular arguments
    qDeleteAll(m_arguments);
    m_arguments.clear();

    for (unsigned i = 0; i < sig->getNumParams(); i++) {
        SharedExp a   = sig->getParamExp(i);
        Assign    *as = new Assign(VoidType::get(), a->clone(), a->clone());
        as->setProc(m_proc);
        as->setBB(m_bb);
        m_arguments.append(as);
    }

    // LOG_STREAM() << "Step 3a: arguments now: ";
    // StatementList::iterator xx; for (xx = arguments.begin(); xx != arguments.end(); ++xx) {
    //        ((Assignment*)*xx)->printCompact(LOG_STREAM()); LOG_STREAM() << ", ";
    // } LOG_STREAM() << "\n";
    // implicitArguments = newimpargs;
    // assert((int)implicitArguments.size() == sig->getNumImplicitParams());

    // 3b
    m_signature = p->getSignature()->clone();

    // 4
    m_isComputed = false;
    m_proc->undoComputedBB(this);
    m_proc->addCallee(m_procDest);
    m_procDest->printDetailsXML();
    convertIndirect = true;

    LOG_VERBOSE("Result of convertToDirect: %1", this);

    return convertIndirect;
}


bool CallStatement::isCallToMemOffset() const
{
    return(getKind() == StmtType::Call && getDest()->getOper() == opMemOf &&
           getDest()->getSubExp1()->getOper() == opIntConst);
}


SharedExp CallStatement::getArgumentExp(int i) const
{
    assert(Util::inRange(i, 0, getNumArguments()));

    // stmt = m_arguments[i]
    const Assign *asgn = dynamic_cast<const Assign *>(*std::next(m_arguments.begin(), i));
    return asgn ? asgn->getRight() : nullptr;
}


void CallStatement::setArgumentExp(int i, SharedExp e)
{
    assert(Util::inRange(i, 0, getNumArguments()));

    Statement *stmt = *std::next(m_arguments.begin(), i);
    SharedExp& a = dynamic_cast<Assign *>(stmt)->getRightRef();
    a = e->clone();
}


int CallStatement::getNumArguments() const
{
    return m_arguments.size();
}


void CallStatement::setNumArguments(int n)
{
    const int oldSize = getNumArguments();

    if (oldSize > n) {
        StatementList::iterator start = std::next(m_arguments.begin(), n);
        qDeleteAll(start, m_arguments.end());
        m_arguments.erase(start, m_arguments.end());
    }

    // MVE: check if these need extra propagation
    for (int i = oldSize; i < n; i++) {
        SharedExp  a  = m_procDest->getSignature()->getArgumentExp(i);
        SharedType ty = m_procDest->getSignature()->getParamType(i);

        if ((ty == nullptr) && oldSize) {
            ty = m_procDest->getSignature()->getParamType(oldSize - 1);
        }

        if (ty == nullptr) {
            ty = VoidType::get();
        }

        Assign *as = new Assign(ty, a->clone(), a->clone());
        as->setProc(m_proc);
        as->setBB(m_bb);
        m_arguments.append(as);
    }
}


void CallStatement::removeArgument(int i)
{
    assert(Util::inRange(i, 0, getNumArguments()));

    StatementList::iterator aa = m_arguments.begin();
    std::advance(aa, i);
    delete *aa;
    m_arguments.erase(aa);
}


SharedType CallStatement::getTypeFor(SharedExp e) const
{
    // The defines "cache" what the destination proc is defining
    Assignment *as = m_defines.findOnLeft(e);

    if (as != nullptr) {
        return as->getType();
    }

    if (e->isPC()) {
        // Special case: just return void*
        return PointerType::get(VoidType::get());
    }

    return VoidType::get();
}


void CallStatement::setTypeFor(SharedExp e, SharedType ty)
{
    Assignment *as = m_defines.findOnLeft(e);

    if (as != nullptr) {
        return as->setType(ty);
    }

    // See if it is in our reaching definitions
    SharedExp ref = m_defCol.findDefFor(e);

    if ((ref == nullptr) || !ref->isSubscript()) {
        return;
    }

    Statement *def = ref->access<RefExp>()->getDef();

    if (def == nullptr) {
        return;
    }

    def->setTypeFor(e, ty);
}


bool CallStatement::objcSpecificProcessing(const QString& formatStr)
{
    Function *_proc = getDestProc();

    if (!_proc) {
        return false;
    }

    QString name(_proc->getName());

    if (name == "objc_msgSend") {
        if (!formatStr.isNull()) {
            int format = getNumArguments() - 1;
            int n      = 1;
            int p      = 0;

            while ((p = formatStr.indexOf(':', p)) != -1) {
                p++;     // Point past the :
                n++;
                addSigParam(PointerType::get(VoidType::get()), false);
            }

            setNumArguments(format + n);
            m_signature->setHasEllipsis(false); // So we don't do this again
            return true;
        }
        else {
            bool change = false;
            LOG_MSG("%1", this);

            for (int i = 0; i < getNumArguments(); i++) {
                SharedExp  e  = getArgumentExp(i);
                SharedType ty = getArgumentType(i);
                LOG_MSG("arg %1 e: %2 ty: %3", i, e, ty);

                if (!(ty->isPointer() && (std::static_pointer_cast<PointerType>(ty)->getPointsTo()->isChar()) && e->isIntConst())) {
                    Address addr = Address(e->access<Const>()->getInt());
                    LOG_MSG("Addr: %1", addr);

                    if (_proc->getProg()->isStringConstant(addr)) {
                        LOG_MSG("Making arg %1 of call c*", i);
                        setArgumentType(i, PointerType::get(CharType::get()));
                        change = true;
                    }
                    else if (_proc->getProg()->isCFStringConstant(addr)) {
                        Address addr2 = Address(_proc->getProg()->readNative4(addr + 8));
                        LOG_MSG("Arg %1 of call is a cfstring", i);
                        setArgumentType(i, PointerType::get(CharType::get()));
                        // TODO: we'd really like to change this to CFSTR(addr)
                        setArgumentExp(i, Const::get(addr2));
                        change = true;
                    }
                }
            }

            return change;
        }
    }

    return false;
}


bool CallStatement::ellipsisProcessing(Prog *prog)
{
    Q_UNUSED(prog);

    // if (getDestProc() == nullptr || !getDestProc()->getSignature()->hasEllipsis())
    if ((getDestProc() == nullptr) || !m_signature->hasEllipsis()) {
        return objcSpecificProcessing(nullptr);
    }

    // functions like printf almost always have too many args
    QString name(getDestProc()->getName());
    int     format = -1;

    if (((name == "printf") || (name == "scanf"))) {
        format = 0;
    }
    else if ((name == "sprintf") || (name == "fprintf") || (name == "sscanf")) {
        format = 1;
    }
    else if (getNumArguments() && getArgumentExp(getNumArguments() - 1)->isStrConst()) {
        format = getNumArguments() - 1;
    }
    else {
        return false;
    }

    LOG_VERBOSE("Ellipsis processing for %1", name);

    QString   formatStr = QString::null;
    SharedExp formatExp = getArgumentExp(format);

    // We sometimes see a[m[blah{...}]]
    if (formatExp->isAddrOf()) {
        formatExp = formatExp->getSubExp1();

        if (formatExp->isSubscript()) {
            formatExp = formatExp->getSubExp1();
        }

        if (formatExp->isMemOf()) {
            formatExp = formatExp->getSubExp1();
        }
    }

    if (formatExp->isSubscript()) {
        // Maybe it's defined to be a Const string
        Statement *def = formatExp->access<RefExp>()->getDef();

        if (def == nullptr) {
            return false;     // Not all nullptr refs get converted to implicits
        }

        if (def->isAssign()) {
            // This would be unusual; propagation would normally take care of this
            SharedExp rhs = static_cast<Assign *>(def)->getRight();

            if ((rhs == nullptr) || !rhs->isStrConst()) {
                return false;
            }

            formatStr = rhs->access<Const>()->getStr();
        }
        else if (def->isPhi()) {
            // More likely. Example: switch_gcc. Only need ONE candidate format string
            PhiAssign *pa = static_cast<PhiAssign *>(def);

            for (auto& v : *pa) {
                def = v.getDef();

                if (!def || !def->isAssign()) {
                    continue;
                }

                SharedExp rhs = static_cast<Assign *>(def)->getRight();

                if (!rhs || !rhs->isStrConst()) {
                    continue;
                }

                formatStr = rhs->access<Const>()->getStr();
                break;
            }

            if (formatStr.isNull()) {
                return false;
            }
        }
        else {
            return false;
        }
    }
    else if (formatExp->isStrConst()) {
        formatStr = formatExp->access<Const>()->getStr();
    }
    else {
        return false;
    }

    if (objcSpecificProcessing(formatStr)) {
        return true;
    }

    // actually have to parse it
    // Format string is: % [flags] [width] [.precision] [size] type
    int  n = 1;     // Count the format string itself (may also be "format" more arguments)
    char ch;
    // Set a flag if the name of the function is scanf/sscanf/fscanf
    bool isScanf = name.contains("scanf");
    int  p_idx   = 0;

    // TODO: use qregularexpression to match scanf arguments
    while ((p_idx = formatStr.indexOf('%', p_idx)) != -1) {
        p_idx++;                   // Point past the %
        bool veryLong = false;     // %lld or %L

        do {
            ch = formatStr[p_idx++].toLatin1();     // Skip size and precisionA

            switch (ch)
            {
            case '*':
                // Example: printf("Val: %*.*f\n", width, precision, val);
                n++;     // There is an extra parameter for the width or precision
                // This extra parameter is of type integer, never int* (so pass false as last argument)
                addSigParam(IntegerType::get(STD_SIZE), false);
                continue;

            case '-':
            case '+':
            case '#':
            case ' ':
                // Flag. Ignore
                continue;

            case '.':
                // Separates width and precision. Ignore.
                continue;

            case 'h':
            case 'l':

                // size of half or long. Argument is usually still one word. Ignore.
                // Exception: %llx
                // TODO: handle architectures where l implies two words
                // TODO: at least h has implications for scanf
                if (formatStr[p_idx] == 'l') {
                    // %llx
                    p_idx++;     // Skip second l
                    veryLong = true;
                }

                continue;

            case 'L':
                // long. TODO: handle L for long doubles.
                // n++;        // At least chew up one more parameter so later types are correct
                veryLong = true;
                continue;

            default:

                if (('0' <= ch) && (ch <= '9')) {
                    continue;     // width or precision
                }

                break;            // Else must be format type, handled below
            }

            break;
        } while (1);

        if (ch != '%') {     // Don't count %%
            n++;
        }

        switch (ch)
        {
        case 'd':
        case 'i':     // Signed integer
            addSigParam(IntegerType::get(veryLong ? 64 : 32), isScanf);
            break;

        case 'u':
        case 'x':
        case 'X':
        case 'o':     // Unsigned integer
            addSigParam(IntegerType::get(32, -1), isScanf);
            break;

        case 'f':
        case 'g':
        case 'G':
        case 'e':
        case 'E':     // Various floating point formats
            // Note that for scanf, %f means float, and %lf means double, whereas for printf, both of these mean
            // double
            addSigParam(FloatType::get(veryLong ? 128 : (isScanf ? 32 : 64)), isScanf);     // Note: may not be 64 bits
            // for some archs
            break;

        case 's':     // String
            addSigParam(PointerType::get(ArrayType::get(CharType::get())), isScanf);
            break;

        case 'c':     // Char
            addSigParam(CharType::get(), isScanf);
            break;

        case '%':
            break;     // Ignore %% (emits 1 percent char)

        default:
            LOG_WARN("Unhandled format character %1 in format string for call %2", ch, this);
        }
    }

    setNumArguments(format + n);
    m_signature->setHasEllipsis(false);     // So we don't do this again
    return true;
}


Assign *CallStatement::makeArgAssign(SharedType ty, SharedExp e)
{
    SharedExp lhs = e->clone();

    localiseComp(lhs);     // Localise the components of lhs (if needed)
    SharedExp rhs = localiseExp(e->clone());
    Assign    *as = new Assign(ty, lhs, rhs);
    as->setProc(m_proc);
    as->setBB(m_bb);
    // It may need implicit converting (e.g. sp{-} -> sp{0})
    Cfg *cfg = m_proc->getCFG();

    if (cfg->isImplicitsDone()) {
        ImplicitConverter     ic(cfg);
        StmtImplicitConverter sm(&ic, cfg);
        as->accept(&sm);
    }

    return as;
}


void CallStatement::addSigParam(SharedType ty, bool isScanf)
{
    if (isScanf) {
        ty = PointerType::get(ty);
    }

    m_signature->addParameter(ty);
    SharedExp paramExp = m_signature->getParamExp(m_signature->getNumParams() - 1);

    LOG_VERBOSE("EllipsisProcessing: adding parameter %1 of type %2", paramExp, ty->getCtype());

    if (m_arguments.size() < m_signature->getNumParams()) {
        Assign *as = makeArgAssign(ty, paramExp);
        m_arguments.append(as);
    }
}


bool CallStatement::isDefinition() const
{
    LocationSet defs;

    getDefinitions(defs);
    return defs.size() != 0;
}


bool CallStatement::definesLoc(SharedExp loc) const
{
    for (auto dd = m_defines.begin(); dd != m_defines.end(); ++dd) {
        SharedExp lhs = static_cast<Assign *>(*dd)->getLeft();

        if (*lhs == *loc) {
            return true;
        }
    }

    return false;
}


void CallStatement::updateDefines()
{
    std::shared_ptr<Signature> sig;

    if (m_procDest) {
        // The signature knows how to order the returns
        sig = m_procDest->getSignature();
    }
    else {
        // Else just use the enclosing proc's signature
        sig = m_proc->getSignature();
    }

    if (m_procDest && m_procDest->isLib()) {
        sig->setLibraryDefines(m_defines);     // Set the locations defined
        return;
    }
    else if (SETTING(assumeABI)) {
        // Risky: just assume the ABI caller save registers are defined
        Signature::setABIDefines(m_proc->getProg(), m_defines);
        return;
    }

    // Move the defines to a temporary list. We must make sure that all defines
    // that are not inserted into m_defines again are deleted.
    StatementList           oldDefines(m_defines);
    m_defines.clear();

    if (m_procDest && m_calleeReturn) {
        StatementList& modifieds = static_cast<UserProc *>(m_procDest)->getModifieds();

        for (Statement *mm : modifieds) {
            Assignment *as = static_cast<Assignment *>(mm);
            SharedExp  loc = as->getLeft();

            if (m_proc->filterReturns(loc)) {
                continue;
            }

            SharedType ty = as->getType();

            if (!oldDefines.existsOnLeft(loc)) {
                oldDefines.append(new ImplicitAssign(ty, loc));
            }
        }
    }
    else {
        // Ensure that everything in the UseCollector has an entry in oldDefines
        LocationSet::iterator ll;

        for (ll = m_useCol.begin(); ll != m_useCol.end(); ++ll) {
            SharedExp loc = *ll;

            if (m_proc->filterReturns(loc)) {
                continue;     // Filtered out
            }

            if (!oldDefines.existsOnLeft(loc)) {
                ImplicitAssign *as = new ImplicitAssign(loc->clone());
                as->setProc(m_proc);
                as->setBB(m_bb);
                oldDefines.append(as);
            }
        }
    }

    for (StatementList::reverse_iterator it = oldDefines.rbegin(); it != oldDefines.rend(); ++it) {
        // Make sure the LHS is still in the return or collector
        Assignment *as = static_cast<Assignment *>(*it);
        SharedExp  lhs = as->getLeft();

        if (m_calleeReturn) {
            if (!m_calleeReturn->definesLoc(lhs)) {
                delete *it;
                continue;     // Not in callee returns
            }
        }
        else if (!m_useCol.exists(lhs)) {
            delete *it;
            continue;     // Not in collector: delete it (don't copy it)
        }

        if (m_proc->filterReturns(lhs)) {
            delete *it;
            continue;     // Filtered out: delete it
        }

        // Insert as, in order, into the existing set of definitions
        bool inserted = false;

        for (StatementList::iterator nn = m_defines.begin(); nn != m_defines.end(); ++nn) {
            if (sig->returnCompare(*as, *static_cast<Assignment *>(*nn))) {     // If the new assignment is less than the current one
                nn       = m_defines.insert(nn, as);               // then insert before this position
                inserted = true;
                break;
            }
        }

        if (!inserted) {
            m_defines.push_back(as);     // In case larger than all existing elements
        }
    }
}


void CallStatement::updateArguments()
{
    /*
     * If this is a library call, source = signature
     *          else if there is a callee return, source = callee parameters
     *          else
     *            if a forced callee signature, source = signature
     *            else source is def collector in this call.
     *          oldArguments = arguments
     *          clear arguments
     *          for each arg lhs in source
     *                  if exists in oldArguments, leave alone
     *                  else if not filtered append assignment lhs=lhs to oldarguments
     *          for each argument as in oldArguments in reverse order
     *                  lhs = as->getLeft
     *                  if (lhs does not exist in source) continue
     *                  if filterParams(lhs) continue
     *                  insert as into arguments, considering sig->argumentCompare
     */
    // Note that if propagations are limited, arguments and collected reaching definitions can be in terms of phi
    // statements that have since been translated to assignments. So propagate through them now
    // FIXME: reconsider! There are problems (e.g. with test/pentium/fromSSA2, test/pentium/fbranch) if you
    // propagate
    // to the expressions in the arguments (e.g. m[esp{phi1}-20]) but don't propagate into ordinary statements that
    // define the actual argument. For example, you might have m[esp{-}-56] in the call, but the actual definition
    // of
    // the printf argument is still m[esp{phi1} -20] = "%d".
    if (EXPERIMENTAL) {
        bool convert;
        m_proc->propagateStatements(convert, 88);
    }

    // Do not delete statements in m_arguments since they are preserved by oldArguments
    StatementList oldArguments(m_arguments);
    m_arguments.clear();

    if (EXPERIMENTAL) {
        // I don't really know why this is needed, but I was seeing r28 := ((((((r28{-}-4)-4)-4)-8)-4)-4)-4:
        DefCollector::iterator dd;

        for (dd = m_defCol.begin(); dd != m_defCol.end(); ++dd) {
            (*dd)->simplify();
        }
    }

    auto sig = m_proc->getSignature();
    // Ensure everything in the callee's signature (if this is a library call), or the callee parameters (if
    // available),
    // or the def collector if not,  exists in oldArguments
    ArgSourceProvider asp(this);
    SharedExp         loc;

    while ((loc = asp.nextArgLoc()) != nullptr) {
        if (m_proc->filterParams(loc)) {
            continue;
        }

        if (!oldArguments.existsOnLeft(loc)) {
            // Check if the location is renamable. If not, localising won't work, since it relies on definitions
            // collected in the call, and you just get m[...]{-} even if there are definitions.
            SharedExp rhs;

            if (m_proc->canRename(loc)) {
                rhs = asp.localise(loc->clone());
            }
            else {
                rhs = loc->clone();
            }

            SharedType ty  = asp.curType(loc);
            Assign     *as = new Assign(ty, loc->clone(), rhs);
            as->setNumber(m_number);     // Give the assign the same statement number as the call (for now)
            // as->setParent(this);
            as->setProc(m_proc);
            as->setBB(m_bb);
            oldArguments.append(as);
        }
    }

    for (StatementList::reverse_iterator it = oldArguments.rbegin(); it != oldArguments.rend(); ++it) {
        // Make sure the LHS is still in the callee signature / callee parameters / use collector
        Assign    *as = static_cast<Assign *>(*it);
        SharedExp lhs = as->getLeft();

        if (!asp.exists(lhs)) {
            delete *it;
            continue;
        }

        if (m_proc->filterParams(lhs)) {
            // Filtered out: delete it
            delete *it;
            continue;
        }

        // Insert as, in order, into the existing set of definitions
        bool inserted = false;

        for (StatementList::iterator nn = m_arguments.begin(); nn != m_arguments.end(); ++nn) {
            if (sig->argumentCompare(*as, *static_cast<Assign *>(*nn))) {     // If the new assignment is less than the current one
                m_arguments.insert(nn, as);           // then insert before this position
                inserted = true;
                break;
            }
        }

        if (!inserted) {
            m_arguments.push_back(as); // In case larger than all existing elements
        }
    }
}


std::unique_ptr<StatementList> CallStatement::calcResults()
{
    std::unique_ptr<StatementList> result(new StatementList);

    if (m_procDest) {
        auto sig = m_procDest->getSignature();

        SharedExp rsp = Location::regOf(m_proc->getSignature()->getStackRegister(m_proc->getProg()));

        for (Statement *dd : m_defines) {
            SharedExp lhs = static_cast<Assignment *>(dd)->getLeft();

            // The stack pointer is allowed as a define, so remove it here as a special case non result
            if (*lhs == *rsp) {
                continue;
            }

            if (m_useCol.exists(lhs)) {
                result->append(dd);
            }
        }
    }
    else {
        // For a call with no destination at this late stage, use everything live at the call except for the stack
        // pointer register. Needs to be sorted
        UseCollector::iterator  rr;     // Iterates through reaching definitions
        StatementList::iterator nn;     // Iterates through new results
        auto sig = m_proc->getSignature();
        int  sp  = sig->getStackRegister();

        for (rr = m_useCol.begin(); rr != m_useCol.end(); ++rr) {
            SharedExp loc = *rr;

            if (m_proc->filterReturns(loc)) {
                continue;     // Ignore filtered locations
            }

            if (loc->isRegN(sp)) {
                continue;                                          // Ignore the stack pointer
            }

            ImplicitAssign *as      = new ImplicitAssign(loc);
            bool           inserted = false;

            for (nn = result->begin(); nn != result->end(); ++nn) {
                // If the new assignment is less than the current one,
                if (sig->returnCompare(*as, static_cast<const Assignment &>(**nn))) {
                    nn       = result->insert(nn, as);     // then insert before this position
                    inserted = true;
                    break;
                }
            }

            if (!inserted) {
                result->insert(result->end(), as);     // In case larger than all existing elements
            }
        }
    }

    return result;
}


void CallStatement::removeDefine(SharedExp e)
{
    for (StatementList::iterator ss = m_defines.begin(); ss != m_defines.end(); ++ss) {
        Assignment *as = static_cast<Assignment *>(*ss);

        if (*as->getLeft() == *e) {
            delete *ss;
            m_defines.erase(ss);
            return;
        }
    }

    LOG_WARN("Could not remove define %1 from call %2", e, this);
}


bool CallStatement::isChildless() const
{
    if (m_procDest == nullptr) {
        return true;
    }
    else if (m_procDest->isLib()) {
        return false;
    }

    // Early in the decompile process, recursive calls are treated as childless, so they use and define all
    if (static_cast<UserProc *>(m_procDest)->isEarlyRecursive()) {
        return true;
    }

    return m_calleeReturn == nullptr;
}


SharedExp CallStatement::bypassRef(const std::shared_ptr<RefExp>& r, bool& ch)
{
    SharedExp base = r->getSubExp1();
    SharedExp proven;

    ch = false;

    if (m_procDest && m_procDest->isLib()) {
        auto sig = m_procDest->getSignature();
        proven = sig->getProven(base);

        if (proven == nullptr) {                  // Not (known to be) preserved
            if (sig->findReturn(base) != -1) {
                return r->shared_from_this();     // Definately defined, it's the return
            }

            // Otherwise, not all that sure. Assume that library calls pass things like local variables
        }
    }
    else {
        // Was using the defines to decide if something is preserved, but consider sp+4 for stack based machines
        // Have to use the proven information for the callee (if any)
        if (m_procDest == nullptr) {
            return r->shared_from_this();     // Childless callees transmit nothing
        }

        // if (procDest->isLocal(base))                    // ICK! Need to prove locals and parameters through
        // calls...
        // FIXME: temporary HACK! Ignores alias issues.
        if (!m_procDest->isLib() && static_cast<const UserProc *>(m_procDest)->isLocalOrParamPattern(base)) {
            SharedExp ret = localiseExp(base->clone());     // Assume that it is proved as preserved
            ch = true;

            LOG_VERBOSE("%1 allowed to bypass call statement %2 ignoring aliasing; result %3", base, m_number, ret);
            return ret;
        }

        proven = m_procDest->getProven(base);     // e.g. r28+4
    }

    if (proven == nullptr) {
        return r->shared_from_this();                     // Can't bypass, since nothing proven
    }

    SharedExp to = localiseExp(base);                     // e.g. r28{17}
    assert(to);
    proven = proven->clone();                             // Don't modify the expressions in destProc->proven!
    proven = proven->searchReplaceAll(*base, to, ch);     // e.g. r28{17} + 4

    if (ch) {
        LOG_VERBOSE("Replacing %1 with %2", r, proven);
    }

    return proven;
}


void CallStatement::addDefine(ImplicitAssign *as)
{
    m_defines.append(as);
}


void CallStatement::eliminateDuplicateArgs()
{
    LocationSet ls;

    for (StatementList::iterator it = m_arguments.begin(); it != m_arguments.end();) {
        SharedExp lhs = static_cast<const Assignment *>(*it)->getLeft();

        if (ls.exists(lhs)) {
            // This is a duplicate
            delete *it;
            it = m_arguments.erase(it);
            continue;
        }

        ls.insert(lhs);
        ++it;
    }
}


void CallStatement::setNumber(int num)
{
    m_number = num;
    // Also number any existing arguments. Important for library procedures, since these have arguments set by the
    // front
    // end based in their signature

    for (Statement *stmt : m_arguments) {
        stmt->setNumber(num);
    }
}


bool CallStatement::accept(StmtModifier *v)
{
    bool visitChildren = true;
    v->visit(this, visitChildren);

    if (!visitChildren) {
        return true;
    }

    if (m_dest && v->m_mod) {
        m_dest = m_dest->accept(v->m_mod);
    }

    for (StatementList::iterator it = m_arguments.begin(); visitChildren && it != m_arguments.end(); it++) {
        (*it)->accept(v);
    }

    // For example: needed for CallBypasser so that a collected definition that happens to be another call gets
    // adjusted
    // I'm thinking no at present... let the bypass and propagate while possible logic take care of it, and leave
    // the
    // collectors as the rename logic set it
    // Well, sort it out with ignoreCollector()
    if (!v->ignoreCollector()) {
        DefCollector::iterator cc;

        for (cc = m_defCol.begin(); cc != m_defCol.end(); cc++) {
            (*cc)->accept(v);
        }
    }

    for (StatementList::iterator dd = m_defines.begin(); visitChildren && dd != m_defines.end(); ++dd) {
        (*dd)->accept(v);
    }

    return true;
}


bool CallStatement::accept(StmtExpVisitor *v)
{
    bool visitChildren = true;
    bool ret = v->visit(this, visitChildren);

    if (!visitChildren) {
        return ret;
    }

    if (ret && m_dest) {
        ret = m_dest->accept(v->ev);
    }


    for (StatementList::iterator it = m_arguments.begin(); ret && it != m_arguments.end(); it++) {
        ret = (*it)->accept(v);
    }

    // FIXME: surely collectors should be counted?
    return ret;
}


bool CallStatement::accept(StmtPartModifier *v)
{
    bool visitChildren = true;
    v->visit(this, visitChildren);

    if (m_dest && visitChildren) {
        m_dest = m_dest->accept(v->mod);
    }

    for (StatementList::iterator it = m_arguments.begin(); visitChildren && it != m_arguments.end(); it++) {
        (*it)->accept(v);
    }

    // For example: needed for CallBypasser so that a collected definition that happens to be another call gets
    // adjusted
    // But now I'm thinking no, the bypass and propagate while possible logic should take care of it.
    // Then again, what about the use collectors in calls? Best to do it.
    if (!v->ignoreCollector()) {
        DefCollector::iterator dd;

        for (dd = m_defCol.begin(); dd != m_defCol.end(); dd++) {
            (*dd)->accept(v);
        }

        UseCollector::iterator uu;

        for (uu = m_useCol.begin(); uu != m_useCol.end(); ++uu) {
            // I believe that these should never change at the top level, e.g. m[esp{30} + 4] -> m[esp{-} - 20]
            (*uu)->accept(v->mod);
        }
    }

    StatementList::iterator dd;

    for (dd = m_defines.begin(); visitChildren && dd != m_defines.end(); dd++) {
        (*dd)->accept(v);
    }

    return true;
}

