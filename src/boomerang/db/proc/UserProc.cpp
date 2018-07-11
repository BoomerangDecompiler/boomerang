#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "UserProc.h"


#include "boomerang/codegen/ICodeGenerator.h"
#include "boomerang/core/Project.h"
#include "boomerang/db/BasicBlock.h"
#include "boomerang/db/IndirectJumpAnalyzer.h"
#include "boomerang/db/InterferenceFinder.h"
#include "boomerang/db/Module.h"
#include "boomerang/db/ProcDecompiler.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/signature/Signature.h"
#include "boomerang/db/UseCollector.h"
#include "boomerang/db/exp/Location.h"
#include "boomerang/db/exp/RefExp.h"
#include "boomerang/db/exp/Terminal.h"
#include "boomerang/db/exp/Ternary.h"
#include "boomerang/db/exp/TypedExp.h"
#include "boomerang/db/statements/PhiAssign.h"
#include "boomerang/db/statements/CallStatement.h"
#include "boomerang/db/statements/BranchStatement.h"
#include "boomerang/db/statements/ImplicitAssign.h"
#include "boomerang/db/statements/ImpRefStatement.h"
#include "boomerang/visitor/expmodifier/ImplicitConverter.h"
#include "boomerang/visitor/expvisitor/ExpDestCounter.h"
#include "boomerang/visitor/expmodifier/ExpSSAXformer.h"
#include "boomerang/visitor/expmodifier/CallBypasser.h"
#include "boomerang/visitor/expvisitor/TempToLocalMapper.h"
#include "boomerang/visitor/stmtexpvisitor/StmtDestCounter.h"
#include "boomerang/visitor/stmtmodifier/StmtImplicitConverter.h"
#include "boomerang/visitor/stmtexpvisitor/StmtExpVisitor.h"
#include "boomerang/visitor/stmtexpvisitor/StmtDestCounter.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/passes/dataflow/BlockVarRenamePass.h"
#include "boomerang/type/TypeRecovery.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/util/StatementSet.h"
#include "boomerang/util/ConnectionGraph.h"
#include "boomerang/util/DFGWriter.h"
#include "boomerang/util/UseGraphWriter.h"
#include "boomerang/util/Log.h"
#include "boomerang/util/Types.h"
#include "boomerang/util/Util.h"

#include <QFile>
#include <QTextStream>
#include <QSet>

#include <sstream>
#include <algorithm> // For find()
#include <cstring>


UserProc::UserProc(Address address, const QString& name, Module *module)
    : Function(address, std::make_shared<Signature>(name), module)
    , m_df(this)
    , m_recursionGroup(nullptr)
    , m_retStatement(nullptr)
    , m_cfg(new Cfg(this))
    , m_status(PROC_UNDECODED)
    , m_dfgCount(0)
{
}


UserProc::~UserProc()
{
    deleteCFG();

    qDeleteAll(m_parameters);
}


bool UserProc::isNoReturn() const
{
    // undecoded procs are assumed to always return (and define everything)
    if (!this->isDecoded()) {
        return false;
    }

    BasicBlock *exitbb = m_cfg->getExitBB();

    if (exitbb == nullptr) {
        return true;
    }

    if (exitbb->getNumPredecessors() == 1) {
        Statement *s = exitbb->getPredecessor(0)->getLastStmt();

        if (!s || !s->isCall()) {
            return false;
        }

        const CallStatement *call = static_cast<const CallStatement *>(s);

        if (call->getDestProc() && call->getDestProc()->isNoReturn()) {
            return true;
        }
    }

    return false;
}


void UserProc::setStatus(ProcStatus s)
{
    if (m_status != s) {
        m_status = s;
        m_prog->getProject()->alertProcStatusChanged(this);
    }
}


QString UserProc::toString() const
{
    QString     tgt;
    QTextStream ost(&tgt);

    this->print(ost);
    return tgt;
}


void UserProc::renameParameter(const QString& oldName, const QString& newName)
{
    Function::renameParameter(oldName, newName);
    // cfg->searchAndReplace(Location::param(oldName, this), Location::param(newName, this));
}


void UserProc::setParamType(const char *name, SharedType ty)
{
    m_signature->setParamType(name, ty);
}


void UserProc::setParamType(int idx, SharedType ty)
{
    if (static_cast<size_t>(idx) >= m_parameters.size()) {
        // index out of range
        return;
    }

    auto it = std::next(m_parameters.begin(), idx);
    assert(it != m_parameters.end());
    assert((*it)->isAssignment());

    Assignment *a = static_cast<Assignment *>(*it);
    a->setType(ty);
    // Sometimes the signature isn't up to date with the latest parameters
    m_signature->setParamType(a->getLeft(), ty);
}


void UserProc::printUseGraph() const
{
    const Settings *settings = getProg()->getProject()->getSettings();
    const QString filePath = settings->getOutputDirectory()
        .absoluteFilePath(getName() + "-usegraph.dot");
    UseGraphWriter().writeUseGraph(this, filePath);
}


void UserProc::deleteCFG()
{
    delete m_cfg;
    m_cfg = nullptr;
}


void UserProc::setDecoded()
{
    setStatus(PROC_DECODED);
}


BasicBlock *UserProc::getEntryBB()
{
    return m_cfg->getEntryBB();
}


void UserProc::setEntryBB()
{
    BasicBlock *entryBB = m_cfg->getBBStartingAt(m_entryAddress);
    m_cfg->setEntryAndExitBB(entryBB);
}


void UserProc::addCallee(Function *callee)
{
    // is it already in? (this is much slower than using a set)
    if (std::find(m_calleeList.begin(), m_calleeList.end(), callee) != m_calleeList.end()) {
        // already present
        return;
    }

    m_calleeList.push_back(callee);
}


void UserProc::print(QTextStream& out, bool html) const
{
    numberStatements();

    QString tgt1;
    QString tgt2;

    QTextStream ost1(&tgt1);
    QTextStream ost2(&tgt2);


    printParams(ost1, html);
    dumpLocals(ost1, html);
    m_procUseCollector.print(ost2, html);

    m_signature->print(out, html);
    out << "\n";

    if (html) {
        out << "<br>";
    }

    out << "in module " << m_module->getName() << "\n";

    if (html) {
        out << "<br>";
    }

    out << tgt1;
    printSymbolMap(out, html);

    if (html) {
        out << "<br>";
    }

    out << "live variables:\n";

    if (tgt2 == QString::null) {
        out << "  <None>\n";
    }
    else {
        out << "  " << tgt2 << "\n";
    }

    if (html) {
        out << "<br>";
    }

    QString     tgt3;
    QTextStream ost3(&tgt3);
    m_cfg->print(ost3, html);
    out << tgt3 << "\n";
}


void UserProc::printParams(QTextStream& out, bool html /*= false*/) const
{
    if (html) {
        out << "<br>";
    }

    out << "parameters: ";

    if (!m_parameters.empty()) {
        bool first = true;

        for (auto const& elem : m_parameters) {
            if (first) {
                first = false;
            }
            else {
                out << ", ";
            }

            out << static_cast<Assignment *>(elem)->getType() << " " << static_cast<Assignment *>(elem)->getLeft();
        }
    }
    else {
        out << "<None>";
    }

    out << "\n";

    if (html) {
        out << "<br>";
    }
}


void UserProc::printDFG() const
{
    const QString fname = QString("%1%2-%3-dfg.dot")
        .arg(m_prog->getProject()->getSettings()->getOutputDirectory().absolutePath())
        .arg(getName())
        .arg(m_dfgCount++);

    DFGWriter().printDFG(this, fname);
}


void UserProc::numberStatements() const
{
    int stmtNumber = 0;

    for (BasicBlock *bb : *m_cfg) {
        BasicBlock::RTLIterator rit;
        StatementList::iterator sit;
        for (Statement *s = bb->getFirstStmt(rit, sit); s; s = bb->getNextStmt(rit, sit)) {
            s->setNumber(++stmtNumber);
        }
    }
}


void UserProc::getStatements(StatementList& stmts) const
{
    for (const BasicBlock *bb : *m_cfg) {
        bb->appendStatementsTo(stmts);
    }

    for (Statement *s : stmts) {
        if (s->getProc() == nullptr) {
            s->setProc(const_cast<UserProc *>(this));
        }
    }
}


void UserProc::removeStatement(Statement *stmt)
{
    // remove anything proven about this statement
    for (auto it = m_provenTrue.begin(); it != m_provenTrue.end();) {
        LocationSet refs;
        it->second->addUsedLocs(refs);
        it->first->addUsedLocs(refs); // Could be say m[esp{99} - 4] on LHS and we are deleting stmt 99
        bool usesIt = false;
        for (SharedExp r : refs) {
            if (r->isSubscript() && (r->access<RefExp>()->getDef() == stmt)) {
                usesIt = true;
                break;
            }
        }

        if (usesIt) {
            LOG_VERBOSE("Removing proven true exp %1 = %2 that uses statement being removed.",
                        it->first, it->second);

            it = m_provenTrue.erase(it);
            continue;
        }

        ++it;
    }

    // remove from BB/RTL
    BasicBlock       *bb   = stmt->getBB(); // Get our enclosing BB
    RTLList *rtls = bb->getRTLs();

    for (auto& rtl : *rtls) {
        for (RTL::iterator it = rtl->begin(); it != rtl->end(); ++it) {
            if (*it == stmt) {
                rtl->erase(it);
                return;
            }
        }
    }
}


void UserProc::insertAssignAfter(Statement *s, SharedExp left, SharedExp right)
{
    RTL::iterator it;
    RTL *stmts;
    BasicBlock *bb = nullptr;

    if (s == nullptr) {
        // This means right is supposed to be a parameter. We can insert the assignment at the start of the entryBB
        bb = m_cfg->getEntryBB();
        RTLList *rtls = bb->getRTLs();
        assert(!rtls->empty()); // Entry BB should have at least 1 RTL
        stmts = rtls->front().get();
        it    = stmts->begin();
    }
    else {
        // An ordinary definition; put the assignment at the end of s's BB
        bb = s->getBB(); // Get the enclosing BB for s
        RTLList *rtls = bb->getRTLs();
        assert(!rtls->empty()); // If s is defined here, there should be at least 1 RTL
        stmts = rtls->back().get();
        it    = stmts->end(); // Insert before the end
    }

    Assign *as = new Assign(left, right);
    stmts->insert(it, as);
    as->setProc(this);
    as->setBB(bb);
}


void UserProc::insertStatementAfter(Statement *afterThis, Statement *stmt)
{
    for (BasicBlock *bb : *m_cfg) {
        RTLList *rtls = bb->getRTLs();

        if (rtls == nullptr) {
            continue; // e.g. bb is (as yet) invalid
        }

        for (const auto& rtl : *rtls) {
            for (RTL::iterator ss = rtl->begin(); ss != rtl->end(); ++ss) {
                if (*ss == afterThis) {
                    rtl->insert(std::next(ss), stmt);
                    stmt->setBB(bb);
                    return;
                }
            }
        }
    }

    assert(false); // Should have found this statement in this BB
}


void UserProc::decompile()
{
    ProcDecompiler().decompileRecursive(this);
}


void UserProc::debugPrintAll(const QString& stepName)
{
    if (m_prog->getProject()->getSettings()->verboseOutput) {
        numberStatements();

        QDir outputDir = m_prog->getProject()->getSettings()->getOutputDirectory();
        QString filePath = outputDir.absoluteFilePath(getName());

        LOG_SEPARATE(filePath, "--- debug print %1 for %2 ---", stepName, getName());
        LOG_SEPARATE(filePath, "%1", this->toString());
        LOG_SEPARATE(filePath, "=== end debug print %1 for %2 ===", stepName, getName());
    }
}


void UserProc::remUnusedStmtEtc()
{
    m_prog->getProject()->alertDecompiling(this);
    m_prog->getProject()->alertDecompileDebugPoint(this, "Before Final");

    LOG_VERBOSE("### Removing unused statements for %1 ###", getName());

    // Perform type analysis. If we are relying (as we are at present) on TA to perform ellipsis processing,
    // do the local TA pass now. Ellipsis processing often reveals additional uses (e.g. additional parameters
    // to printf/scanf), and removing unused statements is unsafe without full use information
    if (getStatus() < PROC_FINAL) {
        PassManager::get()->executePass(PassID::LocalTypeAnalysis, this);

        // Now that locals are identified, redo the dataflow
        PassManager::get()->executePass(PassID::PhiPlacement, this);

        PassManager::get()->executePass(PassID::BlockVarRename, this);       // Rename the locals
        PassManager::get()->executePass(PassID::StatementPropagation, this); // Surely need propagation too

        if (m_prog->getProject()->getSettings()->verboseOutput) {
            debugPrintAll("after propagating locals");
        }
    }

    PassManager::get()->executePass(PassID::UnusedStatementRemoval, this);
    PassManager::get()->executePass(PassID::FinalParameterSearch, this);

    if (m_prog->getProject()->getSettings()->nameParameters) {
        // Replace the existing temporary parameters with the final ones:
        // mapExpressionsToParameters();
        PassManager::get()->executePass(PassID::ParameterSymbolMap, this);
        debugPrintAll("after adding new parameters");
    }

    // Or just CallArgumentUpdate?
    PassManager::get()->executePass(PassID::CallDefineUpdate, this);
    PassManager::get()->executePass(PassID::CallArgumentUpdate, this);

    const bool removedBBs = PassManager::get()->executePass(PassID::BranchAnalysis, this);

    if (removedBBs) {
        // redo the data flow
        PassManager::get()->executePass(PassID::Dominators, this);

        // recalculate phi assignments of referencing BBs.
        for (BasicBlock *bb : *m_cfg) {
            BasicBlock::RTLIterator rtlIt;
            StatementList::iterator stmtIt;

            for (Statement *stmt = bb->getFirstStmt(rtlIt, stmtIt); stmt; stmt = bb->getNextStmt(rtlIt, stmtIt)) {
                if (!stmt->isPhi()) {
                    continue;
                }

                PhiAssign *phiStmt = dynamic_cast<PhiAssign *>(stmt);
                assert(phiStmt);

                PhiAssign::PhiDefs& defs = phiStmt->getDefs();

                for (PhiAssign::PhiDefs::iterator defIt = defs.begin(); defIt != defs.end();) {
                    if (!m_cfg->hasBB(defIt->first)) {
                        // remove phi reference to deleted bb
                        defIt = defs.erase(defIt);
                    }
                    else {
                        ++defIt;
                    }
                }
            }
        }
    }

    debugPrintAll("after remove unused statements etc");
    m_prog->getProject()->alertDecompileDebugPoint(this, "after final");
}


void UserProc::assignProcsToCalls()
{
    for (BasicBlock *bb : *m_cfg) {
        RTLList *rtls = bb->getRTLs();

        if (rtls == nullptr) {
            continue;
        }

        for (auto& rtl : *rtls) {
            if (!rtl->isCall()) {
                continue;
            }

            CallStatement *call = static_cast<CallStatement *>(rtl->back());

            if ((call->getDestProc() == nullptr) && !call->isComputed()) {
                Function *p = m_prog->getFunctionByAddr(call->getFixedDest());

                if (p == nullptr) {
                    LOG_FATAL("Cannot find proc for dest %1 in call at %2",
                              call->getFixedDest(), rtl->getAddress());
                }

                call->setDestProc(p);
            }
        }
    }
}


void UserProc::finalSimplify()
{
    for (BasicBlock *bb : *m_cfg) {
        RTLList *rtls = bb->getRTLs();

        if (rtls == nullptr) {
            continue;
        }

        for (auto& rtl : *rtls) {
            for (Statement *stmt : *rtl) {
                stmt->simplifyAddr();
                // Also simplify everything; in particular, stack offsets are
                // often negative, so we at least canonicalise [esp + -8] to [esp-8]
                stmt->simplify();
            }
        }
    }
}


void UserProc::addParameter(SharedExp e, SharedType ty)
{
    // In case it's already an implicit argument:
    removeParameter(e);

    m_signature->addParameter(e, ty);
}



SharedExp UserProc::getSymbolExp(SharedExp le, SharedType ty, bool lastPass)
{
    SharedExp e = nullptr;

    // check for references to the middle of a local
    if (le->isMemOf() && (le->getSubExp1()->getOper() == opMinus) && le->getSubExp1()->getSubExp1()->isSubscript() &&
        le->getSubExp1()->getSubExp1()->getSubExp1()->isRegN(m_signature->getStackRegister()) &&
        le->getSubExp1()->getSubExp2()->isIntConst()) {
        for (auto& elem : m_symbolMap) {
            if (!(elem).second->isLocal()) {
                continue;
            }

            QString name = (elem).second->access<Const, 1>()->getStr();

            if (m_locals.find(name) == m_locals.end()) {
                continue;
            }

            SharedType     lty = m_locals[name];
            SharedConstExp loc = elem.first;

            if (loc->isMemOf() && (loc->getSubExp1()->getOper() == opMinus) &&
                loc->access<Exp, 1, 1>()->isSubscript() &&
                loc->access<Exp, 1, 1, 1>()->isRegN(m_signature->getStackRegister()) &&
                loc->access<Exp, 1, 2>()->isIntConst()) {
                int n = -loc->access<Const, 1, 2>()->getInt();
                int m = -le->access<Const, 1, 2>()->getInt();

                if ((m > n) && (m < n + static_cast<int>(lty->getSize() / 8))) {
                    e = Location::memOf(Binary::get(opPlus, Unary::get(opAddrOf, elem.second->clone()), Const::get(m - n)));
                    LOG_VERBOSE("Seems %1 is in the middle of %2 returning %3", le, loc, e);
                    return e;
                }
            }
        }
    }

    if (m_symbolMap.find(le) == m_symbolMap.end()) {
        if (ty == nullptr) {
            if (lastPass) {
                ty = IntegerType::get(STD_SIZE);
            }
            else {
                ty = VoidType::get(); // HACK MVE
            }
        }

        // the default of just assigning an int type is bad..  if the locals is not an int then assigning it this
        // type early results in aliases to this local not being recognised
        if (ty) {
            //            Exp* base = le;
            //            if (le->isSubscript())
            //                base = ((RefExp*)le)->getSubExp1();
            // NOTE: using base below instead of le does not enhance anything, but causes us to lose def information
            e = createLocal(ty->clone(), le);
            mapSymbolTo(le->clone(), e);
            e = e->clone();
        }
    }
    else {
        e = getSymbolFor(le, ty);
    }

    return e;
}


void UserProc::promoteSignature()
{
    m_signature = m_signature->promote(this);
}


QString UserProc::newLocalName(const SharedExp& e)
{
    QString     tgt;
    QTextStream ost(&tgt);

    if (e->isSubscript() && e->getSubExp1()->isRegOf()) {
        // Assume that it's better to know what register this location was created from
        QString regName = getRegName(e->getSubExp1());
        int     tag     = 0;

        do {
            ost.flush();
            tgt.clear();
            ost << regName << "_" << ++tag;
        } while (m_locals.find(tgt) != m_locals.end());

        return tgt;
    }

    ost << "local" << m_nextLocal++;
    return tgt;
}


SharedExp UserProc::createLocal(SharedType ty, const SharedExp& e, char *name /* = nullptr */)
{
    QString localName = (name != nullptr) ? name : newLocalName(e);

    m_locals[localName] = ty;

    if (ty == nullptr) {
        LOG_FATAL("Null type passed to newLocal");
    }

    LOG_VERBOSE2("Assigning type %1 to new %2", ty->getCtype(), localName);

    return Location::local(localName, this);
}


void UserProc::addLocal(SharedType ty, const QString& name, SharedExp e)
{
    // symbolMap is a multimap now; you might have r8->o0 for integers and r8->o0_1 for char*
    // assert(symbolMap.find(e) == symbolMap.end());
    mapSymbolTo(e, Location::local(name, this));
    // assert(locals.find(name) == locals.end());        // Could be r10{20} -> o2, r10{30}->o2 now
    m_locals[name] = ty;
}


SharedConstType UserProc::getLocalType(const QString& name) const
{
    auto it = m_locals.find(name);
    return (it != m_locals.end()) ? it->second : nullptr;
}


void UserProc::setLocalType(const QString& name, SharedType ty)
{
    m_locals[name] = ty;

    LOG_VERBOSE("Updating type of '%1' to %2", name, ty->getCtype());
}


SharedConstType UserProc::getParamType(const QString& name) const
{
    for (int i = 0; i < m_signature->getNumParams(); i++) {
        if (name == m_signature->getParamName(i)) {
            return m_signature->getParamType(i);
        }
    }

    return nullptr;
}


SharedType UserProc::getParamType(const QString& name)
{
    for (int i = 0; i < m_signature->getNumParams(); i++) {
        if (name == m_signature->getParamName(i)) {
            return m_signature->getParamType(i);
        }
    }

    return nullptr;
}


void UserProc::mapSymbolTo(const SharedConstExp& from, SharedExp to)
{
    SymbolMap::iterator it = m_symbolMap.find(from);

    while (it != m_symbolMap.end() && *it->first == *from) {
        if (*it->second == *to) {
            return; // Already in the multimap
        }

        ++it;
    }

    std::pair<SharedConstExp, SharedExp> pr = { from, to };
    m_symbolMap.insert(pr);
}


SharedExp UserProc::getSymbolFor(const SharedConstExp& from, const SharedConstType& ty) const
{
    SymbolMap::const_iterator ff = m_symbolMap.find(from);

    while (ff != m_symbolMap.end() && *ff->first == *from) {
        SharedExp currTo = ff->second;
        assert(currTo->isLocal() || currTo->isParam());
        QString    name   = std::static_pointer_cast<Const>(currTo->getSubExp1())->getStr();
        SharedConstType currTy = getLocalType(name);

        if (currTy == nullptr) {
            currTy = getParamType(name);
        }

        if (currTy && currTy->isCompatibleWith(*ty)) {
            return currTo;
        }

        ++ff;
    }

    return nullptr;
}


SharedConstExp UserProc::expFromSymbol(const QString& name) const
{
    for (const std::pair<SharedConstExp, SharedExp>& it : m_symbolMap) {
        auto e = it.second;

        if (e->isLocal() && (e->access<Const, 1>()->getStr() == name)) {
            return it.first;
        }
    }

    return nullptr;
}


static std::shared_ptr<Binary> allEqAll = Binary::get(opEquals, Terminal::get(opDefineAll), Terminal::get(opDefineAll));


bool UserProc::prove(const std::shared_ptr<Binary>& query, bool conditional /* = false */)
{
    assert(query->isEquality());
    SharedExp queryLeft  = query->getSubExp1();
    SharedExp queryRight = query->getSubExp2();

    if ((m_provenTrue.find(queryLeft) != m_provenTrue.end()) && (*m_provenTrue[queryLeft] == *queryRight)) {
        if (m_prog->getProject()->getSettings()->debugProof) {
            LOG_MSG("found true in provenTrue cache %1 in %2", query, getName());
        }

        return true;
    }

    if (!m_prog->getProject()->getSettings()->useProof) {
        return false;
    }

    SharedExp original(query->clone());
    SharedExp origLeft  = original->getSubExp1();
    SharedExp origRight = original->getSubExp2();

    // subscript locs on the right with {-} (nullptr reference)
    LocationSet locs;
    query->getSubExp2()->addUsedLocs(locs);

    for (const SharedExp& xx : locs) {
        query->setSubExp2(query->getSubExp2()->expSubscriptValNull(xx));
    }

    if (query->getSubExp1()->getOper() != opSubscript) {
        bool gotDef = false;

        // replace expression from return set with expression in the collector of the return
        if (m_retStatement) {
            auto def = m_retStatement->findDefFor(query->getSubExp1());

            if (def) {
                query->setSubExp1(def);
                gotDef = true;
            }
        }

        if (!gotDef) {
            // OK, the thing I'm looking for isn't in the return collector, but perhaps there is an entry for <all>
            // If this is proved, then it is safe to say that x == x for any x with no definition reaching the exit
            auto right = origRight->clone()->simplify(); // In case it's sp+0

            if ((*origLeft == *right) &&                 // x == x
                (origLeft->getOper() != opDefineAll) &&  // Beware infinite recursion
                prove(allEqAll)) {                       // Recurse in case <all> not proven yet
                    if (m_prog->getProject()->getSettings()->debugProof) {
                    LOG_MSG("Using all=all for %1", query->getSubExp1());
                    LOG_MSG("Prove returns true");
                }

                m_provenTrue[origLeft->clone()] = right;
                return true;
            }

            if (m_prog->getProject()->getSettings()->debugProof) {
                LOG_MSG("Not in return collector: %1", query->getSubExp1());
                LOG_MSG("Prove returns false");
            }

            return false;
        }
    }

    if (m_recursionGroup) { // If in involved in a recursion cycle
        //    then save the original query as a premise for bypassing calls
        m_recurPremises[origLeft->clone()] = origRight;
    }

    std::set<PhiAssign *>            lastPhis;
    std::map<PhiAssign *, SharedExp> cache;
    bool result = prover(query, lastPhis, cache);

    if (m_recursionGroup) {
        m_recurPremises.erase(origLeft); // Remove the premise, regardless of result
    }

    if (m_prog->getProject()->getSettings()->debugProof) {
        LOG_MSG("Prove returns %1 for %2 in %3", (result ? "true" : "false"), query, getName());
    }

    if (!conditional) {
        if (result) {
            m_provenTrue[origLeft] = origRight; // Save the now proven equation
        }
    }

    return result;
}


bool UserProc::prover(SharedExp query, std::set<PhiAssign *>& lastPhis, std::map<PhiAssign *, SharedExp>& cache,
                      PhiAssign *lastPhi /* = nullptr */)
{
    // A map that seems to be used to detect loops in the call graph:
    std::map<CallStatement *, SharedExp> called;
    auto phiInd = query->getSubExp2()->clone();

    if (lastPhi && (cache.find(lastPhi) != cache.end()) && (*cache[lastPhi] == *phiInd)) {
        if (m_prog->getProject()->getSettings()->debugProof) {
            LOG_MSG("true - in the phi cache");
        }

        return true;
    }

    std::set<Statement *> refsTo;

    query = query->clone();
    bool change  = true;
    bool swapped = false;

    while (change) {
        if (m_prog->getProject()->getSettings()->debugProof) {
            LOG_MSG("%1", query);
        }

        change = false;

        if (query->getOper() == opEquals) {
            // same left and right means true
            if (*query->getSubExp1() == *query->getSubExp2()) {
                query  = Terminal::get(opTrue);
                change = true;
            }

            // move constants to the right
            if (!change) {
                auto plus = query->getSubExp1();
                auto s1s2 = plus ? plus->getSubExp2() : nullptr;

                if (plus && s1s2) {
                    if ((plus->getOper() == opPlus) && s1s2->isIntConst()) {
                        query->setSubExp2(Binary::get(opPlus, query->getSubExp2(), Unary::get(opNeg, s1s2->clone())));
                        query->setSubExp1(plus->getSubExp1());
                        change = true;
                    }

                    if ((plus->getOper() == opMinus) && s1s2->isIntConst()) {
                        query->setSubExp2(Binary::get(opPlus, query->getSubExp2(), s1s2->clone()));
                        query->setSubExp1(plus->getSubExp1());
                        change = true;
                    }
                }
            }

            // substitute using a statement that has the same left as the query
            if (!change && (query->getSubExp1()->getOper() == opSubscript)) {
                auto          r     = query->access<RefExp, 1>();
                Statement     *s    = r->getDef();
                CallStatement *call = dynamic_cast<CallStatement *>(s);

                if (call) {
                    // See if we can prove something about this register.
                    UserProc *destProc = dynamic_cast<UserProc *>(call->getDestProc());
                    auto     base      = r->getSubExp1();

                    if (destProc && !destProc->isLib() && (destProc->m_recursionGroup != nullptr) &&
                        (destProc->m_recursionGroup->find(this) != destProc->m_recursionGroup->end())) {
                        // The destination procedure may not have preservation proved as yet, because it is involved
                        // in our recursion group. Use the conditional preservation logic to determine whether query is
                        // true for this procedure
                        auto provenTo = destProc->getProven(base);

                        if (provenTo) {
                            // There is a proven preservation. Use it to bypass the call
                            auto queryLeft = call->localiseExp(provenTo->clone());
                            query->setSubExp1(queryLeft);
                            // Now try everything on the result
                            return prover(query, lastPhis, cache, lastPhi);
                        }
                        else {
                            // Check if the required preservation is one of the premises already assumed
                            auto premisedTo = destProc->getPremised(base);

                            if (premisedTo) {
                                if (m_prog->getProject()->getSettings()->debugProof) {
                                    LOG_MSG("Conditional preservation for call from %1 to %2, allows bypassing", getName(), destProc->getName());
                                }

                                auto queryLeft = call->localiseExp(premisedTo->clone());
                                query->setSubExp1(queryLeft);
                                return prover(query, lastPhis, cache, lastPhi);
                            }
                            else {
                                // There is no proof, and it's not one of the premises. It may yet succeed, by making
                                // another premise! Example: try to prove esp, depends on whether ebp is preserved, so
                                // recurse to check ebp's preservation. Won't infinitely loop because of the premise map
                                // FIXME: what if it needs a rx = rx + K preservation?
                                auto newQuery = Binary::get(opEquals, base->clone(), base->clone());
                                destProc->setPremise(base);

                                if (m_prog->getProject()->getSettings()->debugProof) {
                                    LOG_MSG("New required premise %1 for %2", newQuery, destProc->getName());
                                }

                                // Pass conditional as true, since even if proven, this is conditional on other things
                                bool result = destProc->prove(newQuery, true);
                                destProc->killPremise(base);

                                if (result) {
                                    if (m_prog->getProject()->getSettings()->debugProof) {
                                        LOG_MSG("Conditional preservation with new premise %1 succeeds for %2",
                                                newQuery, destProc->getName());
                                    }

                                    // Use the new conditionally proven result
                                    auto queryLeft = call->localiseExp(base->clone());
                                    query->setSubExp1(queryLeft);
                                    return destProc->prover(query, lastPhis, cache, lastPhi);
                                }
                                else {
                                    if (m_prog->getProject()->getSettings()->debugProof) {
                                        LOG_MSG("Conditional preservation required premise %1 fails!", newQuery);
                                    }

                                    // Do nothing else; the outer proof will likely fail
                                }
                            }
                        }
                    } // End call involved in this recursion group

                    // Seems reasonable that recursive procs need protection from call loops too
                    auto right = call->getProven(r->getSubExp1()); // getProven returns the right side of what is

                    if (right) {                                   //    proven about r (the LHS of query)
                        right = right->clone();

                        if ((called.find(call) != called.end()) && (*called[call] == *query)) {
                            LOG_MSG("Found call loop to %1 %2", call->getDestProc()->getName(), query);
                            query  = Terminal::get(opFalse);
                            change = true;
                        }
                        else {
                            called[call] = query->clone();

                            if (m_prog->getProject()->getSettings()->debugProof) {
                                LOG_MSG("Using proven for %1 %2 = %3", call->getDestProc()->getName(),
                                        r->getSubExp1(), right);
                            }

                            right = call->localiseExp(right);

                            if (m_prog->getProject()->getSettings()->debugProof) {
                                LOG_MSG("Right with subs: %1", right);
                            }

                            query->setSubExp1(right); // Replace LHS of query with right
                            change = true;
                        }
                    }
                }
                else if (s && s->isPhi()) {
                    // for a phi, we have to prove the query for every statement
                    PhiAssign           *pa = static_cast<PhiAssign *>(s);
                    bool                ok = true;

                    if ((lastPhis.find(pa) != lastPhis.end()) || (pa == lastPhi)) {
                        ok = (*query->getSubExp2() == *phiInd);

                        if (m_prog->getProject()->getSettings()->debugProof) {
                            if (ok) {
                                LOG_MSG("Phi loop detected (set true due to induction)"); // FIXME: induction??!
                            }
                            else {
                                LOG_MSG("Phi loop detected (set false: %1 != %2)", query->getSubExp2(), phiInd);
                            }
                        }
                    }
                    else {
                        if (m_prog->getProject()->getSettings()->debugProof) {
                            LOG_MSG("Found %1 prove for each, ", s);
                        }

                        for (RefExp &pi : *pa) {
                            auto e  = query->clone();
                            auto r1 = e->access<RefExp, 1>();
                            r1->setDef(pi.getDef());

                            if (m_prog->getProject()->getSettings()->debugProof) {
                                LOG_MSG("proving for %1", e);
                            }

                            lastPhis.insert(lastPhi);

                            if (!prover(e, lastPhis, cache, pa)) {
                                ok = false;
                                // delete e;
                                break;
                            }

                            lastPhis.erase(lastPhi);
                            // delete e;
                        }

                        if (ok) {
                            cache[pa] = query->getSubExp2()->clone();
                        }
                    }

                    if (ok) {
                        query = Terminal::get(opTrue);
                    }
                    else {
                        query = Terminal::get(opFalse);
                    }

                    change = true;
                }
                else if (s && s->isAssign()) {
                    if (s && (refsTo.find(s) != refsTo.end())) {
                        LOG_ERROR("Detected ref loop %1", s);
                        LOG_ERROR("refsTo: ");

                        for (Statement *ins : refsTo) {
                            LOG_MSG("  %1, ", ins->getNumber());
                        }

                        assert(false);
                    }
                    else {
                        refsTo.insert(s);
                        query->setSubExp1(static_cast<Assign *>(s)->getRight()->clone());
                        change = true;
                    }
                }
            }

            // remove memofs from both sides if possible
            if (!change && (query->getSubExp1()->getOper() == opMemOf) && (query->getSubExp2()->getOper() == opMemOf)) {
                query->setSubExp1(query->getSubExp1()->getSubExp1());
                query->setSubExp2(query->getSubExp2()->getSubExp1());
                change = true;
            }

            // is ok if both of the memofs are subscripted with nullptr
            if (!change && (query->getSubExp1()->getOper() == opSubscript) &&
                (query->access<Exp, 1, 1>()->getOper() == opMemOf) && (query->access<RefExp, 1>()->getDef() == nullptr) &&
                (query->access<Exp, 2>()->getOper() == opSubscript) && (query->access<Exp, 2, 1>()->getOper() == opMemOf) &&
                (query->access<RefExp, 2>()->getDef() == nullptr)) {
                query->setSubExp1(query->getSubExp1()->getSubExp1()->getSubExp1());
                query->setSubExp2(query->getSubExp2()->getSubExp1()->getSubExp1());
                change = true;
            }

            // find a memory def for the right if there is a memof on the left
            // FIXME: this seems pretty much like a bad hack!
            if (!change && (query->getSubExp1()->getOper() == opMemOf)) {
                StatementList stmts;
                getStatements(stmts);

                for (Statement *s : stmts) {
                    Assign *as = dynamic_cast<Assign *>(s);

                    if (as && (*as->getRight() == *query->getSubExp2()) && (as->getLeft()->getOper() == opMemOf)) {
                        query->setSubExp2(as->getLeft()->clone());
                        change = true;
                        break;
                    }
                }
            }

            // last chance, swap left and right if haven't swapped before
            if (!change && !swapped) {
                auto e = query->getSubExp1();
                query->setSubExp1(query->getSubExp2());
                query->setSubExp2(e);
                change  = true;
                swapped = true;
                refsTo.clear();
            }
        }
        else if (query->isIntConst()) {
            auto c = query->access<Const>();
            query = Terminal::get(c->getInt() ? opTrue : opFalse);
        }

        auto old = query->clone();

        auto query_prev = query;
        query = query->clone()->simplify();

        if (change && !(*old == *query) && m_prog->getProject()->getSettings()->debugProof) {
            LOG_MSG("%1", old);
        }
    }

    return query->getOper() == opTrue;
}


bool UserProc::searchAndReplace(const Exp& search, SharedExp replace)
{
    bool          ch = false;
    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        ch |= s->searchAndReplace(search, replace);
    }

    return ch;
}


SharedExp UserProc::getProven(SharedExp left)
{
    // Note: proven information is in the form r28 mapsto (r28 + 4)
    auto it = m_provenTrue.find(left);

    if (it != m_provenTrue.end()) {
        return it->second;
    }

    //     not found, try the signature
    // No! The below should only be for library functions!
    // return signature->getProven(left);
    return nullptr;
}


SharedExp UserProc::getPremised(SharedExp left)
{
    auto it = m_recurPremises.find(left);

    if (it != m_recurPremises.end()) {
        return it->second;
    }

    return nullptr;
}


bool UserProc::isPreserved(SharedExp e)
{
    return m_provenTrue.find(e) != m_provenTrue.end() && *m_provenTrue[e] == *e;
}


QString UserProc::lookupParam(SharedConstExp e) const
{
    // Originally e.g. m[esp+K]
    Statement *def = m_cfg->findTheImplicitAssign(e);

    if (def == nullptr) {
        LOG_ERROR("No implicit definition for parameter %1!", e);
        return QString::null;
    }

    SharedConstType ty  = def->getTypeFor(e);
    return lookupSym(RefExp::get(std::const_pointer_cast<Exp>(e), def), ty);
}


QString UserProc::lookupSymFromRef(const std::shared_ptr<const RefExp>& ref) const
{
    const Statement *def = ref->getDef();

    if (!def) {
        LOG_WARN("Unknown def for RefExp '%1' in '%2'", ref->toString(), getName());
        return QString::null;
    }

    SharedConstExp  base = ref->getSubExp1();
    SharedConstType ty   = def->getTypeFor(base);
    return lookupSym(ref, ty);
}


QString UserProc::lookupSymFromRefAny(const std::shared_ptr<const RefExp>& ref) const
{
    const Statement *def = ref->getDef();

    if (!def) {
        LOG_WARN("Unknown def for RefExp '%1' in '%2'", ref->toString(), getName());
        return QString::null;
    }

    SharedConstExp  base = ref->getSubExp1();
    SharedConstType ty   = def->getTypeFor(base);

    // Check for specific symbol
    const QString   ret  = lookupSym(ref, ty);
    if (!ret.isNull()) {
        return ret;
    }

    return lookupSym(base, ty); // Check for a general symbol
}


QString UserProc::lookupSym(const SharedConstExp& arg, SharedConstType ty) const
{
    SharedConstExp e = arg;

    if (arg->isTypedExp()) {
        e = e->getSubExp1();
    }

    SymbolMap::const_iterator it = m_symbolMap.find(e);

    while (it != m_symbolMap.end() && *it->first == *e) {
        SharedExp sym = it->second;
        assert(sym->isLocal() || sym->isParam());

        const QString    name = sym->access<Const, 1>()->getStr();
        SharedConstType type = getLocalType(name);

        if (type == nullptr) {
            type = getParamType(name); // Ick currently linear search
        }

        if (type && type->isCompatibleWith(*ty)) {
            return name;
        }

        ++it;
    }

    // Else there is no symbol
    return QString::null;
}


void UserProc::printSymbolMap(QTextStream& out, bool html /*= false*/) const
{
    if (html) {
        out << "<br>";
    }

    out << "symbols:\n";

    if (m_symbolMap.empty()) {
        out << "  <None>\n";
    }
    else {
        for (const std::pair<SharedConstExp, SharedExp>& it : m_symbolMap) {
            SharedConstType ty = getTypeForLocation(it.second);
            out << "  " << it.first << " maps to " << it.second << " type " << (ty ? qPrintable(ty->getCtype()) : "<unknown>") << "\n";

            if (html) {
                out << "<br>";
            }
        }
    }

    if (html) {
        out << "<br>";
    }
}


void UserProc::dumpLocals(QTextStream& os, bool html) const
{
    if (html) {
        os << "<br>";
    }

    os << "locals:\n";

    if (m_locals.empty()) {
        os << "  <None>\n";
    }
    else {
        for (const std::pair<QString, SharedType>& local_entry : m_locals) {
            os << local_entry.second->getCtype() << " " << local_entry.first << " ";
            SharedConstExp e = expFromSymbol(local_entry.first);

            // Beware: for some locals, expFromSymbol() returns nullptr (? No longer?)
            if (e) {
                os << "  " << e << "\n";
            }
            else {
                os << "  -\n";
            }
        }
    }

    if (html) {
        os << "<br>";
    }
}


void UserProc::insertParameter(SharedExp e, SharedType ty)
{
    if (filterParams(e)) {
        return; // Filtered out
    }

    // Used to filter out preserved locations here: no! Propagation and dead code elimination solve the problem.
    // See test/pentium/restoredparam for an example where you must not remove restored locations

    // Wrap it in an implicit assignment; DFA based TA should update the type later
    ImplicitAssign *as = new ImplicitAssign(ty->clone(), e->clone());

    // Insert as, in order, into the existing set of parameters
    bool inserted = false;

    for (StatementList::iterator nn = m_parameters.begin(); nn != m_parameters.end(); ++nn) {
        // If the new assignment is less than the current one ...
        if (m_signature->argumentCompare(*as, static_cast<const Assignment &>(**nn))) {
            nn       = m_parameters.insert(nn, as); // ... then insert before this position
            inserted = true;
            break;
        }
    }

    if (!inserted) {
        m_parameters.insert(m_parameters.end(), as); // In case larger than all existing elements
    }

    // update the signature
    m_signature->setNumParams(0);
    int i = 1;

    for (Statement *param : m_parameters) {
        Assignment *a = static_cast<Assignment *>(param);
        QString paramName = QString("param%1").arg(i++);

        m_signature->addParameter(paramName, a->getLeft(), a->getType());
    }
}


bool UserProc::filterReturns(SharedExp e)
{
    if (isPreserved(e)) {
        // If it is preserved, then it can't be a return (since we don't change it)
        return true;
    }

    switch (e->getOper())
    {
    case opPC:
        return true; // Ignore %pc

    case opDefineAll:
        return true; // Ignore <all>

    case opTemp:
        return true; // Ignore all temps (should be local to one instruction)

    // Would like to handle at least %ZF, %CF one day. For now, filter them out
    case opZF:
    case opCF:
    case opFlags:
        return true;

    case opMemOf:
        // return signature->isStackLocal(prog, e);        // Filter out local variables
        // Actually, surely all sensible architectures will only every return in registers. So for now, just
        // filter out all mem-ofs
        return true;

    case opGlobal:
        return true; // Never return in globals

    default:
        return false;
    }
}


bool UserProc::filterParams(SharedExp e)
{
    switch (e->getOper())
    {
    case opPC:
        return true;

    case opTemp:
        return true;

    case opRegOf:
        {
            const int sp = Util::getStackRegisterIndex(m_prog);
            return e->access<Const, 1>()->getInt() == sp;
        }

    case opMemOf:
        {
            auto addr = e->getSubExp1();

            if (addr->isIntConst()) {
                return true; // Global memory location
            }

            if (addr->isSubscript() && addr->access<RefExp>()->isImplicitDef()) {
                auto reg = addr->getSubExp1();
                int  sp  = 999;

                if (m_signature) {
                    sp = Util::getStackRegisterIndex(m_prog);
                }

                if (reg->isRegN(sp)) {
                    return true; // Filter out m[sp{-}] assuming it is the return address
                }
            }

            return false; // Might be some weird memory expression that is not a local
        }

    case opGlobal:
        return true; // Never use globals as argument locations (could appear on RHS of args)

    default:
        return false;
    }
}


QString UserProc::findLocal(const SharedExp& e, SharedType ty)
{
    if (e->isLocal()) {
        return e->access<Const, 1>()->getStr();
    }

    // Look it up in the symbol map
    QString name = lookupSym(e, ty);

    if (name.isNull()) {
        return name;
    }

    // Now make sure it is a local; some symbols (e.g. parameters) are in the symbol map but not locals
    if (m_locals.find(name) != m_locals.end()) {
        return name;
    }

    return QString::null;
}


QString UserProc::findFirstSymbol(const SharedConstExp& exp) const
{
    SymbolMap::const_iterator ff = m_symbolMap.find(exp);

    if (ff == m_symbolMap.end()) {
        return QString::null;
    }

    return std::static_pointer_cast<Const>(ff->second->getSubExp1())->getStr();
}


void UserProc::markAsNonChildless(const std::shared_ptr<ProcSet>& cs)
{
    BasicBlock::RTLRIterator        rrit;
    StatementList::reverse_iterator srit;

    for (BasicBlock *bb : *m_cfg) {
        CallStatement *c = dynamic_cast<CallStatement *>(bb->getLastStmt(rrit, srit));

        if (c && c->isChildless()) {
            UserProc *dest = static_cast<UserProc *>(c->getDestProc());

            if (cs->find(dest) != cs->end()) { // Part of the cycle?
                // Yes, set the callee return statement (making it non childless)
                c->setCalleeReturn(dest->getTheReturnStatement());
            }
        }
    }
}


void UserProc::propagateToCollector()
{
    for (auto it = m_procUseCollector.begin(); it != m_procUseCollector.end();) {
        if (!(*it)->isMemOf()) {
            ++it;
            continue;
        }

        auto        addr = (*it)->getSubExp1();
        LocationSet used;
        addr->addUsedLocs(used);

        for (const SharedExp& v : used) {
            if (!v->isSubscript()) {
                continue;
            }

            auto   r   = v->access<RefExp>();
            if (!r->getDef() || !r->getDef()->isAssign()) {
                continue;
            }

            Assign *as = static_cast<Assign *>(r->getDef());

            bool ch;
            auto res = addr->clone()->searchReplaceAll(*r, as->getRight(), ch);

            if (!ch) {
                continue; // No change
            }

            auto memOfRes = Location::memOf(res)->simplify();

            // First check to see if memOfRes is already in the set
            if (m_procUseCollector.exists(memOfRes)) {
                // Take care not to use an iterator to the newly erased element.
                /* it = */
                m_procUseCollector.remove(it++);            // Already exists; just remove the old one
                continue;
            }
            else {
                LOG_VERBOSE("Propagating %1 to %2 in collector; result %3",
                            r, as->getRight(), memOfRes);
                (*it)->setSubExp1(res); // Change the child of the memof
            }
        }

        ++it; // it is iterated either with the erase, or the continue, or here
    }
}


bool UserProc::isLocalOrParamPattern(SharedConstExp e) const
{
    if (!e->isMemOf()) {
        return false; // Don't want say a register
    }

    SharedConstExp addr = e->getSubExp1();

    if (!m_signature->isPromoted()) {
        return false; // Prevent an assert failure if using -E
    }

    const int sp = m_signature->getStackRegister();
    const auto initSp(RefExp::get(Location::regOf(sp), nullptr)); // sp{-}

    if (*addr == *initSp) {
        return true; // Accept m[sp{-}]
    }

    if (addr->getArity() != 2) {
        return false; // Require sp +/- K
    }

    OPER op = addr->getOper();

    if ((op != opPlus) && (op != opMinus)) {
        return false;
    }

    SharedConstExp left = addr->getSubExp1();

    if (!(*left == *initSp)) {
        return false;
    }

    SharedConstExp right = addr->getSubExp2();
    return right->isIntConst();
}


bool UserProc::checkForGainfulUse(SharedExp bparam, ProcSet& visited)
{
    visited.insert(this); // Prevent infinite recursion

    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        // Special checking for recursive calls
        if (s->isCall()) {
            CallStatement *c    = static_cast<CallStatement *>(s);
            UserProc      *dest = dynamic_cast<UserProc *>(c->getDestProc());

            if (dest && dest->doesRecurseTo(this)) {
                // In the destination expression?
                LocationSet u;
                c->getDest()->addUsedLocs(u);

                if (u.containsImplicit(bparam)) {
                    return true; // Used by the destination expression
                }

                // Else check for arguments of the form lloc := f(bparam{0})
                const StatementList& args = c->getArguments();

                for (StatementList::const_iterator aa = args.begin(); aa != args.end(); ++aa) {
                    const Assign *a = dynamic_cast<const Assign *>(*aa);
                    SharedExp   rhs = a ? a->getRight() : nullptr;
                    if (!rhs) {
                        continue;
                    }

                    LocationSet argUses;
                    rhs->addUsedLocs(argUses);

                    if (argUses.containsImplicit(bparam)) {
                        SharedExp lloc = static_cast<Assign *>(*aa)->getLeft();

                        if ((visited.find(dest) == visited.end()) && dest->checkForGainfulUse(lloc, visited)) {
                            return true;
                        }
                    }
                }

                // If get to here, then none of the arguments is of this form, and we can ignore this call
                continue;
            }
        }
        else if (s->isReturn()) {
            if (m_recursionGroup && !m_recursionGroup->empty()) { // If this function is involved in recursion
                continue;                               //  then ignore this return statement
            }
        }
        else if (s->isPhi() && (m_retStatement != nullptr) && m_recursionGroup && !m_recursionGroup->empty()) {
            SharedExp  phiLeft = static_cast<PhiAssign *>(s)->getLeft();
            auto       refPhi  = RefExp::get(phiLeft, s);
            bool       foundPhi = false;

            for (Statement *stmt : *m_retStatement) {
                SharedExp   rhs = static_cast<Assign *>(stmt)->getRight();
                LocationSet uses;
                rhs->addUsedLocs(uses);

                if (uses.contains(refPhi)) {
                    // s is a phi that defines a component of a recursive return. Ignore it
                    foundPhi = true;
                    break;
                }
            }

            if (foundPhi) {
                continue; // Ignore this phi
            }
        }

        // Otherwise, consider uses in s
        LocationSet uses;
        s->addUsedLocs(uses);

        if (uses.containsImplicit(bparam)) {
            return true; // A gainful use
        }
    }                    // for each statement s

    return false;
}


bool UserProc::removeRedundantParameters()
{
    if (m_signature->isForced()) {
        // Assume that no extra parameters would have been inserted... not sure always valid
        return false;
    }

    bool          ret = false;
    StatementList newParameters;

    m_prog->getProject()->alertDecompileDebugPoint(this, "Before removing redundant parameters");

    if (m_prog->getProject()->getSettings()->debugUnused) {
        LOG_MSG("%%% removing unused parameters for %1", getName());
    }

    // Note: this would be far more efficient if we had def-use information
    for (StatementList::iterator pp = m_parameters.begin(); pp != m_parameters.end(); ++pp) {
        SharedExp param = static_cast<Assignment *>(*pp)->getLeft();
        bool      az;
        SharedExp bparam = param->clone()->removeSubscripts(az); // FIXME: why does main have subscripts on parameters?
        // Memory parameters will be of the form m[sp + K]; convert to m[sp{0} + K] as will be found in uses
        bparam = bparam->expSubscriptAllNull();                  // Now m[sp{-}+K]{-}
        ImplicitConverter ic(m_cfg);
        bparam = bparam->acceptModifier(&ic);                            // Now m[sp{0}+K]{0}
        assert(bparam->isSubscript());
        bparam = bparam->access<Exp, 1>();                       // now m[sp{0}+K] (bare parameter)

        ProcSet visited;

        if (checkForGainfulUse(bparam, visited)) {
            newParameters.append(*pp); // Keep this parameter
        }
        else {
            // Remove the parameter
            ret = true;

            if (m_prog->getProject()->getSettings()->debugUnused) {
                LOG_MSG(" %%% removing unused parameter %1 in %2", param, getName());
            }

            // Check if it is in the symbol map. If so, delete it; a local will be created later
            SymbolMap::iterator ss = m_symbolMap.find(param);

            if (ss != m_symbolMap.end()) {
                m_symbolMap.erase(ss);           // Kill the symbol
            }

            m_signature->removeParameter(param); // Also remove from the signature
            m_cfg->removeImplicitAssign(param);  // Remove the implicit assignment so it doesn't come back
        }
    }

    m_parameters = newParameters;

    if (m_prog->getProject()->getSettings()->debugUnused) {
        LOG_MSG("%%% end removing unused parameters for %1", getName());
    }

    m_prog->getProject()->alertDecompileDebugPoint(this, "after removing redundant parameters");

    return ret;
}


bool UserProc::removeRedundantReturns(std::set<UserProc *>& removeRetSet)
{
    m_prog->getProject()->alertDecompiling(this);
    m_prog->getProject()->alertDecompileDebugPoint(this, "before removing unused returns");
    // First remove the unused parameters
    bool removedParams = removeRedundantParameters();

    if (m_retStatement == nullptr) {
        return removedParams;
    }

    if (m_prog->getProject()->getSettings()->debugUnused) {
        LOG_MSG("%%% removing unused returns for %1 %%%", getName());
    }

    if (m_signature->isForced()) {
        // Respect the forced signature, but use it to remove returns if necessary
        bool removedRets = false;

        for (ReturnStatement::iterator rr = m_retStatement->begin(); rr != m_retStatement->end();) {
            Assign    *a  = static_cast<Assign *>(*rr);
            SharedExp lhs = a->getLeft();
            // For each location in the returns, check if in the signature
            bool found = false;

            for (int i = 0; i < m_signature->getNumReturns(); i++) {
                if (*m_signature->getReturnExp(i) == *lhs) {
                    found = true;
                    break;
                }
            }

            if (found) {
                ++rr; // Yes, in signature; OK
            }
            else {
                // This return is not in the signature. Remove it
                rr          = m_retStatement->erase(rr);
                removedRets = true;

                if (m_prog->getProject()->getSettings()->debugUnused) {
                    LOG_MSG("%%%  removing unused return %1 from proc %2 (forced signature)", a, getName());
                }
            }
        }

        if (removedRets) {
            // Still may have effects on calls or now unused statements
            updateForUseChange(removeRetSet);
        }

        return removedRets;
    }

    // FIXME: this needs to be more sensible when we don't decompile down from main! Probably should assume just the
    // first return is valid, for example (presently assume none are valid)
    LocationSet unionOfCallerLiveLocs;

    if (getName() == "main") { // Probably not needed: main is forced so handled above
        // Just insert one return for main. Note: at present, the first parameter is still the stack pointer
        if (m_signature->getNumReturns() <= 1) {
            // handle the case of missing main() signature
            LOG_WARN("main signature definition is missing; assuming void main()");
        }
        else {
            unionOfCallerLiveLocs.insert(m_signature->getReturnExp(1));
        }
    }
    else {
        // For each caller
        std::set<CallStatement *>& callers = getCallers();

        for (CallStatement *cc : callers) {
#if RECURSION_WIP
            // TODO: prevent function from blocking it's own removals, needs more work
            if (cc->getProc()->doesRecurseTo(this)) {
                continue;
            }
#endif
            // Union in the set of locations live at this call
            UseCollector *useCol = cc->getUseCollector();
            unionOfCallerLiveLocs.makeUnion(useCol->getLocSet());
        }
    }

    // Intersect with the current returns
    bool removedRets = false;

    for (auto rr = m_retStatement->begin(); rr != m_retStatement->end();) {
        Assign *a = static_cast<Assign *>(*rr);

        if (unionOfCallerLiveLocs.contains(a->getLeft())) {
            ++rr;
            continue;
        }

        if (m_prog->getProject()->getSettings()->debugUnused) {
            LOG_MSG("%%%  removing unused return %1 from proc %2", a, getName());
        }

        // If a component of the RHS referenced a call statement, the liveness used to be killed here.
        // This was wrong; you need to notice the liveness changing inside updateForUseChange() to correctly
        // recurse to callee
        rr          = m_retStatement->erase(rr);
        removedRets = true;
    }

    if (m_prog->getProject()->getSettings()->debugUnused) {
        QString     tgt;
        QTextStream ost(&tgt);
        unionOfCallerLiveLocs.print(ost);
        LOG_MSG("%%%  union of caller live locations for %1: %2", getName(), tgt);
        LOG_MSG("%%%  final returns for %1: %2", getName(), m_retStatement->getReturns().prints());
    }

    // removing returns might result in params that can be removed, might as well do it now.
    removedParams |= removeRedundantParameters();

    ProcSet updateSet; // Set of procs to update

    if (removedParams || removedRets) {
        // Update the statements that call us

        for (CallStatement *call : m_callers) {
            PassManager::get()->executePass(PassID::CallArgumentUpdate, this);
            updateSet.insert(call->getProc());    // Make sure we redo the dataflow
            removeRetSet.insert(call->getProc()); // Also schedule caller proc for more analysis
        }

        // Now update myself
        updateForUseChange(removeRetSet);

        // Update any other procs that need updating
        updateSet.erase(this); // Already done this proc

        while (updateSet.size()) {
            UserProc *proc = *updateSet.begin();
            updateSet.erase(proc);
            proc->updateForUseChange(removeRetSet);
        }
    }

    m_prog->getProject()->alertDecompileDebugPoint(this, "after removing unused and redundant returns");
    return removedRets || removedParams;
}


void UserProc::updateForUseChange(std::set<UserProc *>& removeRetSet)
{
    // We need to remember the parameters, and all the livenesses for all the calls, to see if these are changed
    // by removing returns
    if (m_prog->getProject()->getSettings()->debugUnused) {
        LOG_MSG("%%% updating %1 for changes to uses (returns or arguments)", getName());
        LOG_MSG("%%% updating dataflow:");
    }

    // Save the old parameters and call liveness
    const size_t oldNumParameters = m_parameters.size();
    std::map<CallStatement *, UseCollector> callLiveness;

    for (BasicBlock *bb : *m_cfg) {
        BasicBlock::RTLRIterator        rrit;
        StatementList::reverse_iterator srit;
        CallStatement *c = dynamic_cast<CallStatement *>(bb->getLastStmt(rrit, srit));

        // Note: we may have removed some statements, so there may no longer be a last statement!
        if (c == nullptr) {
            continue;
        }

        UserProc *dest = dynamic_cast<UserProc *>(c->getDestProc());

        // Not interested in unanalysed indirect calls (not sure) or calls to lib procs
        if (dest == nullptr) {
            continue;
        }

        callLiveness[c].makeCloneOf(*c->getUseCollector());
    }

    // Have to redo dataflow to get the liveness at the calls correct
    PassManager::get()->executePass(PassID::CallLivenessRemoval, this); // Want to recompute the call livenesses
    PassManager::get()->executePass(PassID::BlockVarRename, this);

    remUnusedStmtEtc(); // Also redoes parameters

    // Have the parameters changed? If so, then all callers will need to update their arguments, and do similar
    // analysis to the removal of returns
    // findFinalParameters();
    removeRedundantParameters();

    if (m_parameters.size() != oldNumParameters) {
        if (m_prog->getProject()->getSettings()->debugUnused) {
            LOG_MSG("%%%  parameters changed for %1", getName());
        }

        std::set<CallStatement *>& callers = getCallers();
        const bool experimental = m_prog->getProject()->getSettings()->experimental;

        for (CallStatement *cc : callers) {
            cc->updateArguments(experimental);
            // Schedule the callers for analysis
            removeRetSet.insert(cc->getProc());
        }
    }

    // Check if the liveness of any calls has changed
    for (auto ll = callLiveness.begin(); ll != callLiveness.end(); ++ll) {
        CallStatement *call             = ll->first;
        const UseCollector& oldLiveness = ll->second;
        const UseCollector& newLiveness = *call->getUseCollector();

        if (newLiveness != oldLiveness) {
            if (m_prog->getProject()->getSettings()->debugUnused) {
                LOG_MSG("%%%  Liveness for call to %1 in %2 changed",
                        call->getDestProc()->getName(), getName());
            }

            removeRetSet.insert(static_cast<UserProc *>(call->getDestProc()));
        }
    }
}


bool UserProc::allPhisHaveDefs() const
{
    StatementList stmts;
    getStatements(stmts);

    for (const Statement *stmt : stmts) {
        if (!stmt->isPhi()) {
            continue; // Might be able to optimise this a bit
        }

        const PhiAssign *pa = static_cast<const PhiAssign *>(stmt);

        for (const auto& ref : *pa) {
            if (!ref.getDef()) {
                return false;
            }
        }
    }

    return true;
}


void UserProc::processDecodedICTs()
{
    for (BasicBlock *bb : *m_cfg) {
        BasicBlock::RTLRIterator        rrit;
        StatementList::reverse_iterator srit;
        Statement *last = bb->getLastStmt(rrit, srit);

        if (last == nullptr) {
            continue; // e.g. a BB with just a NOP in it
        }

        if (!last->isHL_ICT()) {
            continue;
        }

        RTL *rtl = bb->getLastRTL();

        if (m_prog->getProject()->getSettings()->debugSwitch) {
            LOG_MSG("Saving high level switch statement:\n%1", rtl);
        }

        m_prog->getFrontEnd()->saveDecodedRTL(bb->getHiAddr(), rtl);
        // Now decode those new targets, adding out edges as well
        //        if (last->isCase())
        //            bb->processSwitch(this);
    }
}


void UserProc::mapLocalsAndParams()
{
    m_prog->getProject()->alertDecompileDebugPoint(this, "Before mapping locals from dfa type analysis");

    LOG_VERBOSE("### Mapping expressions to local variables for %1 ###", getName());

    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        s->dfaMapLocals();
    }

    LOG_VERBOSE("### End mapping expressions to local variables for %1 ###", getName());
}



QString UserProc::getRegName(SharedExp r)
{
    assert(r->isRegOf());

    // assert(r->getSubExp1()->isConst());
    if (r->getSubExp1()->isConst()) {
        int            regNum = r->access<Const, 1>()->getInt();
        const QString& regName(m_prog->getRegName(regNum));
        assert(!regName.isEmpty());

        if (regName[0] == '%') {
            return regName.mid(1); // Skip % if %eax
        }

        return regName;
    }

    LOG_WARN("Will try to build register name from [tmp+X]!");

    // TODO: how to handle register file lookups ?
    // in some cases the form might be r[tmp+value]
    // just return this expression :(
    // WARN: this is a hack to prevent crashing when r->subExp1 is not const
    QString     tgt;
    QTextStream ostr(&tgt);

    r->getSubExp1()->print(ostr);

    return tgt;
}


SharedType UserProc::getTypeForLocation(const SharedExp& e)
{
    const QString name = e->access<Const, 1>()->getStr();
    if (e->isLocal()) {
        auto it = m_locals.find(name);
        if (it != m_locals.end()) {
            return it->second;
        }
    }

    // Sometimes parameters use opLocal, so fall through
    return getParamType(name);
}


SharedConstType UserProc::getTypeForLocation(const SharedConstExp& e) const
{
    const QString name = e->access<Const, 1>()->getStr();
    if (e->isLocal()) {
        auto it = m_locals.find(name);
        if (it != m_locals.end()) {
            return it->second;
        }
    }

    // Sometimes parameters use opLocal, so fall through
    return getParamType(name);
}


bool UserProc::existsLocal(const QString& name) const
{
    return m_locals.find(name) != m_locals.end();
}


void UserProc::ensureExpIsMappedToLocal(const std::shared_ptr<RefExp>& r)
{
    if (!lookupSymFromRefAny(r).isNull()) {
        return; // Already have a symbol for r
    }

    Statement *def = r->getDef();

    if (!def) {
        return; // TODO: should this be logged ?
    }

    SharedExp  base = r->getSubExp1();
    SharedType ty   = def->getTypeFor(base);
    // No, get its name from the front end
    QString locName = nullptr;

    if (base->isRegOf()) {
        locName = getRegName(base);

        // Create a new local, for the base name if it doesn't exist yet, so we don't need several names for the
        // same combination of location and type. However if it does already exist, addLocal will allocate a
        // new name. Example: r8{0}->argc type int, r8->o0 type int, now r8->o0_1 type char*.
        if (existsLocal(locName)) {
            locName = newLocalName(r);
        }
    }
    else {
        locName = newLocalName(r);
    }

    addLocal(ty, locName, base);
}
