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
#include "boomerang/core/Boomerang.h"
#include "boomerang/db/IndirectJumpAnalyzer.h"
#include "boomerang/db/InterferenceFinder.h"
#include "boomerang/db/Module.h"
#include "boomerang/db/Register.h"
#include "boomerang/db/RTL.h"
#include "boomerang/db/Prog.h"
#include "boomerang/db/Signature.h"
#include "boomerang/db/UseCollector.h"
#include "boomerang/db/BasicBlock.h"
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
#include "boomerang/db/visitor/ExpVisitor.h"
#include "boomerang/db/visitor/ImplicitConverter.h"
#include "boomerang/db/visitor/ExpDestCounter.h"
#include "boomerang/db/visitor/StmtDestCounter.h"
#include "boomerang/db/visitor/ExpSSAXformer.h"
#include "boomerang/db/visitor/StmtImplicitConverter.h"
#include "boomerang/db/visitor/CallBypasser.h"
#include "boomerang/db/visitor/TempToLocalMapper.h"
#include "boomerang/db/visitor/StmtExpVisitor.h"
#include "boomerang/db/visitor/StmtDestCounter.h"
#include "boomerang/passes/PassManager.h"
#include "boomerang/passes/dataflow/BlockVarRenamePass.h"
#include "boomerang/type/TypeRecovery.h"
#include "boomerang/type/type/IntegerType.h"
#include "boomerang/type/type/VoidType.h"
#include "boomerang/type/type/PointerType.h"
#include "boomerang/type/type/ArrayType.h"
#include "boomerang/util/StatementSet.h"
#include "boomerang/util/ConnectionGraph.h"
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
    : Function(address, new Signature(name), module)
    , m_df(this)
    , m_recursionGroup(nullptr)
    , m_retStatement(nullptr)
    , DFGcount(0)
    , m_cfg(new Cfg(this))
    , m_status(PROC_UNDECODED)
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
    m_status = s;
    Boomerang::get()->alertProcStatusChange(this);
}


bool UserProc::containsAddr(Address addr) const
{
    for (BasicBlock *bb : *m_cfg) {
        if (bb->getRTLs() && (addr >= bb->getLowAddr()) && (addr <= bb->getHiAddr())) {
            return true;
        }
    }

    return false;
}


QString UserProc::toString() const
{
    QString     tgt;
    QTextStream ost(&tgt);

    this->print(ost);
    return tgt;
}


void UserProc::renameParam(const char *oldName, const char *newName)
{
    Function::renameParam(oldName, newName);
    // cfg->searchAndReplace(Location::param(oldName, this), Location::param(newName, this));
}


void UserProc::setParamType(const char *name, SharedType ty)
{
    m_signature->setParamType(name, ty);
}


void UserProc::setParamType(int idx, SharedType ty)
{
    auto it = (idx  < m_parameters.size()) ? std::next(m_parameters.begin(), idx) : m_parameters.end();

    if (it != m_parameters.end()) {
        Assignment *a = static_cast<Assignment *>(*it);
        a->setType(ty);
        // Sometimes the signature isn't up to date with the latest parameters
        m_signature->setParamType(a->getLeft(), ty);
    }
}


void UserProc::renameLocal(const char *oldName, const char *newName)
{
    SharedType     ty     = m_locals[oldName];
    SharedConstExp oldExp = expFromSymbol(oldName);

    m_locals.erase(oldName);
    SharedExp oldLoc = getSymbolFor(oldExp, ty);
    auto      newLoc = Location::local(newName, this);

    mapSymbolToRepl(oldExp, oldLoc, newLoc);
    m_locals[newName] = ty;

    StatementList stmts;
    getStatements(stmts);
    for (Statement *stmt : stmts) {
        stmt->searchAndReplace(*oldLoc, newLoc);
    }
}


void UserProc::printUseGraph()
{
    QString filePath = Boomerang::get()->getSettings()->getOutputDirectory().absoluteFilePath(getName() + "-usegraph.dot");
    QFile   file(filePath);

    if (!file.open(QFile::Text | QFile::WriteOnly)) {
        LOG_ERROR("Can't write to file %1", file.fileName());
        return;
    }

    QTextStream out(&file);
    out << "digraph " << getName() << " {\n";
    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        if (s->isPhi()) {
            out << s->getNumber() << " [shape=diamond];\n";
        }

        LocationSet refs;
        s->addUsedLocs(refs);

        for (SharedExp rr : refs) {
            if (rr->isSubscript()) {
                auto r = rr->access<RefExp>();

                if (r->getDef()) {
                    out << r->getDef()->getNumber() << " -> " << s->getNumber() << ";\n";
                }
            }
        }
    }

    out << "}\n";
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


char *UserProc::prints() const
{
    QString     tgt;
    QTextStream ost(&tgt);

    print(ost);
    strncpy(debug_buffer, qPrintable(tgt), DEBUG_BUFSIZE - 1);
    debug_buffer[DEBUG_BUFSIZE - 1] = '\0';
    return debug_buffer;
}


void UserProc::dump() const
{
    QTextStream q_cerr(stderr);

    print(q_cerr);
}


void UserProc::printDFG() const
{
    QString fname = QString("%1%2-%3-dfg.dot")
                       .arg(Boomerang::get()->getSettings()->getOutputDirectory().absolutePath())
                       .arg(getName())
                       .arg(DFGcount);

    DFGcount++;
    LOG_MSG("Outputing DFG to '%1'", fname);
    QFile file(fname);

    if (!file.open(QFile::WriteOnly)) {
        LOG_WARN("Can't open DFG '%1'", fname);
        return;
    }

    QTextStream out(&file);
    out << "digraph " << getName() << " {\n";
    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        if (s->isPhi()) {
            out << s->getNumber() << " [shape=\"triangle\"];\n";
        }

        if (s->isCall()) {
            out << s->getNumber() << " [shape=\"box\"];\n";
        }

        if (s->isBranch()) {
            out << s->getNumber() << " [shape=\"diamond\"];\n";
        }

        LocationSet refs;
        s->addUsedLocs(refs);

        for (SharedExp rr : refs) {
            auto r = std::dynamic_pointer_cast<RefExp>(rr);

            if (r) {
                if (r->getDef()) {
                    out << r->getDef()->getNumber();
                }
                else {
                    out << "input";
                }

                out << " -> ";

                if (s->isReturn()) {
                    out << "output";
                }
                else {
                    out << s->getNumber();
                }

                out << ";\n";
            }
        }
    }

    out << "}\n";
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
    ProcList callStack;
    decompile(callStack);
}


std::shared_ptr<ProcSet> UserProc::decompile(ProcList &callStack)
{
    /* Cycle detection logic:
     * *********************
     * cycleGrp is an initially null pointer to a set of procedures, representing the procedures involved in the current
     * recursion group, if any. These procedures have to be analysed together as a group, after individual pre-group
     * analysis.
     * child is a set of procedures, cleared at the top of decompile(), representing the cycles associated with the
     * current procedure and all of its children. If this is empty, the current procedure is not involved in recursion,
     * and can be decompiled up to and including removing unused statements.
     * callStack is an initially empty list of procedures, representing the call stack from the current entry point to the
     * current procedure, inclusive.
     * If (after all children have been processed: important!) the first element in callStack and also cycleGrp is the current
     * procedure, we have the maximal set of distinct cycles, so we can do the recursion group analysis and return an empty
     * set. At the end of the recursion group analysis, the whole group is complete, ready for the global analyses.
     *
     *   cycleSet decompile(ProcList callStack)        // call stack initially empty
     *     child = new ProcSet
     *     push this proc to the call stack
     *     for each child c called by this proc
     *       if c has already been visited but not finished
     *         // have new cycle
     *         if c is in callStack
     *           // this is a completely new cycle
     *           insert every proc from c to the end of callStack into child
     *         else
     *           // this is a new branch of an existing cycle
     *           child = c->cycleGrp
     *           find first element f of callStack that is in cycleGrp
     *           insert every proc after f to the end of callStack into child
     *           for each element e of child
     *         insert e->cycleGrp into child
     *         e->cycleGrp = child
     *       else
     *         // no new cycle
     *         tmp = c->decompile(callStack)
     *         child = union(child, tmp)
     *         set return statement in call to that of c
     *
     *     if (child empty)
     *       earlyDecompile()
     *       child = middleDecompile()
     *       removeUnusedStatments()            // Not involved in recursion
     *     else
     *       // Is involved in recursion
     *       find first element f in callStack that is also in cycleGrp
     *       if (f == this)             // The big test: have we got the complete strongly connected component?
     *         recursionGroupAnalysis() // Yes, we have
     *         child = new ProcSet      // Don't add these processed cycles to the parent
     *     remove last element (= this) from callStack
     *     return child
     */


    LOG_MSG("%1 procedure '%2'", (m_status >= PROC_VISITED) ? "Re-discovering" : "Discovering", getName());
    Boomerang::get()->alertDiscovered(this, this);

    // Prevent infinite loops when there are cycles in the call graph (should never happen now)
    if (m_status >= PROC_FINAL) {
        LOG_WARN("Proc %1 already has status PROC_FINAL", getName());
        return nullptr; // Already decompiled
    }

    std::shared_ptr<ProcSet> recursionGroup = std::make_shared<ProcSet>();
    if (m_status < PROC_DECODED) {
        // Can happen e.g. if a callee is visible only after analysing a switch statement
        // Actually decoding for the first time, not REdecoding
        if (!m_prog->reDecode(this)) {
            return recursionGroup;
        }
    }

    if (m_status < PROC_VISITED) {
        setStatus(PROC_VISITED); // We have at least visited this proc "on the way down"
    }

    callStack.push_back(this);

    if (SETTING(decodeChildren)) {
        // Recurse to callees first, to perform a depth first search
        for (BasicBlock *bb : *m_cfg) {
            if (bb->getType() != BBType::Call) {
                continue;
            }

            // The call Statement will be in the last RTL in this BB
            CallStatement *call = static_cast<CallStatement *>(bb->getRTLs()->back()->getHlStmt());

            if (!call->isCall()) {
                LOG_WARN("BB at address %1 is a CALL but last stmt is not a call: %2", bb->getLowAddr(), call);
                continue;
            }

            assert(call->isCall());
            UserProc *callee = dynamic_cast<UserProc *>(call->getDestProc());

            if (callee == nullptr) { // not an user proc, or missing dest
                continue;
            }

            if (callee->getStatus() == PROC_FINAL) {
                // Already decompiled, but the return statement still needs to be set for this call
                call->setCalleeReturn(callee->getTheReturnStatement());
                continue;
            }

            // if the callee has already been visited but not done (apart from global analyses, i.e. we have a new cycle)
            if ((callee->getStatus() >= PROC_VISITED) && (callee->getStatus() <= PROC_EARLYDONE)) {
                // if callee is in callStack
                ProcList::iterator calleeIt = std::find(callStack.begin(), callStack.end(), callee);

                if (calleeIt != callStack.end()) {
                    // This is a completely new cycle
                    assert(calleeIt != callStack.end());
                    recursionGroup->insert(calleeIt, callStack.end());
                }
                else if (callee->m_recursionGroup) {
                    // This is new branch of an existing cycle
                    recursionGroup = callee->m_recursionGroup;

                    // Find first element func of callStack that is in callee->recursionGroup
                    ProcList::iterator _pi = std::find_if(callStack.begin(), callStack.end(),
                        [callee] (UserProc *func) {
                            return callee->m_recursionGroup->find(func) != callee->m_recursionGroup->end();
                        });

                    // Insert every proc after func to the end of path into child
                    assert(_pi != callStack.end());
                    recursionGroup->insert(std::next(_pi), callStack.end());
                }

                // update the recursion group for each element in the new group;
                // this will union all the recursion groups that are reached along the call path.
                ProcSet oldRecursionGroup = *recursionGroup;
                for (UserProc *proc : oldRecursionGroup) {
                    if (proc->m_recursionGroup) {
                        recursionGroup->insert(proc->m_recursionGroup->begin(), proc->m_recursionGroup->end());
                    }
                }

                // update the recursion group from the old one(s) to the new one
                for (UserProc *proc : *recursionGroup) {
                    proc->m_recursionGroup = recursionGroup;
                }

                setStatus(PROC_INCYCLE);
            }
            else {
                // No new cycle
                LOG_VERBOSE("Preparing to decompile callee '%1' of '%2'", callee->getName(), getName());

                callee->promoteSignature();
                std::shared_ptr<ProcSet> tmp = callee->decompile(callStack);
                recursionGroup->insert(tmp->begin(), tmp->end());
                // Child has at least done middleDecompile(), possibly more
                call->setCalleeReturn(callee->getTheReturnStatement());

                if (!tmp->empty()) {
                    setStatus(PROC_INCYCLE);
                }
            }
        }
    }

    // if no child involved in recursion
    if (recursionGroup->empty()) {
        Boomerang::get()->alertDecompiling(this);
        LOG_MSG("Decompiling procedure '%1'", getName());

        earlyDecompile();
        recursionGroup = middleDecompile(callStack);

        // If there is a switch statement, middleDecompile could contribute some cycles.
        // If so, we need to test for the recursion logic again
        if (!recursionGroup->empty()) {
            // We've just come back out of decompile(), so we've lost the current proc from the path.
            callStack.push_back(this);
        }
    }

    if (recursionGroup->empty()) {
        remUnusedStmtEtc(); // Do the whole works
        setStatus(PROC_FINAL);
        Boomerang::get()->alertEndDecompile(this);
    }
    else if (m_recursionGroup) {
        // This proc's callees, and hence this proc, is/are involved in recursion.
        // Find first element f in path that is also in our recursion group
        ProcList::iterator f = std::find_if(callStack.begin(), callStack.end(),
            [this] (UserProc *func) {
                return m_recursionGroup->find(func) != m_recursionGroup->end();
            });

        // The big test: have we found the whole strongly connected component (in the call graph)?
        if (*f == this) {
            // Yes, process these procs as a group
            recursionGroupAnalysis(callStack); // Includes remUnusedStmtEtc on all procs in cycleGrp
            setStatus(PROC_FINAL);
            Boomerang::get()->alertEndDecompile(this);
            recursionGroup->clear();
            recursionGroup = std::make_shared<ProcSet>();
        }
    }

    // Remove last element (= this) from path
    // The if should not be neccesary, but nestedswitch needs it
    if (!callStack.empty()) {
        assert(std::find(callStack.begin(), callStack.end(), this) != callStack.end());

        if (callStack.back() != this) {
            LOG_WARN("Last UserProc in UserProc::decompile/path is not this!");
        }

        callStack.remove(this);
    }
    else {
        LOG_WARN("Empty path when trying to remove last proc");
    }

    LOG_VERBOSE("Finished decompile of '%1'", getName());
    return recursionGroup;
}


void UserProc::debugPrintAll(const char *step_name)
{
    if (SETTING(verboseOutput)) {
        numberStatements();

        LOG_SEPARATE(getName(), "--- debug print %1 for %2 ---", step_name, getName());
        LOG_SEPARATE(getName(), "%1", this->toString());
        LOG_SEPARATE(getName(), "=== end debug print %1 for %2 ===", step_name, getName());
        SeparateLogger::getOrCreateLog(getName()).flush();
    }
}


void UserProc::earlyDecompile()
{
    Boomerang::get()->alertStartDecompile(this);
    Boomerang::get()->alertDecompileDebugPoint(this, "Before Initialise");

    PassManager::get()->executePass(PassID::StatementInit, this);
    PassManager::get()->executePass(PassID::Dominators, this);

    if (!SETTING(decompile)) {
        LOG_MSG("Not decompiling.");
        setStatus(PROC_FINAL); // ??!
        return;
    }

    debugPrintAll("After Decoding");
    Boomerang::get()->alertDecompileDebugPoint(this, "After Initialise");

    if (m_status >= PROC_EARLYDONE) {
        return;
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "Before Early");
    LOG_VERBOSE("### Beginning early decompile for '%1' ###", getName());

    // Update the defines in the calls. Will redo if involved in recursion
    PassManager::get()->executePass(PassID::CallDefineUpdate, this);
    PassManager::get()->executePass(PassID::GlobalConstReplace, this);

    // First placement of phi functions, renaming, and initial propagation. This is mostly for the stack pointer
    // TODO: Check if this makes sense. It seems to me that we only want to do one pass of propagation here, since
    // the status == check had been knobbled below. Hopefully, one call to placing phi functions etc will be
    // equivalent to depth 0 in the old scheme
    PassManager::get()->executePass(PassID::PhiPlacement, this);


    // Rename variables
    PassManager::get()->executePass(PassID::BlockVarRename, this);
    PassManager::get()->executePass(PassID::StatementPropagation, this);

    Boomerang::get()->alertDecompileDebugPoint(this, "After Early");
}


std::shared_ptr<ProcSet> UserProc::middleDecompile(ProcList &callStack)
{
    assert(callStack.back() == this);
    Boomerang::get()->alertDecompileDebugPoint(this, "Before Middle");
    LOG_VERBOSE("### Beginning middleDecompile for '%1' ###", getName());

    // The call bypass logic should be staged as well. For example, consider m[r1{11}]{11} where 11 is a call.
    // The first stage bypass yields m[r1{2}]{11}, which needs another round of propagation to yield m[r1{-}-32]{11}
    // (which can safely be processed at depth 1).
    // Except that this is now inherent in the visitor nature of the latest algorithm.
    PassManager::get()->executePass(PassID::CallAndPhiFix, this); // Bypass children that are finalised (if any)
    debugPrintAll("After call and phi bypass (1)");

    if (getStatus() != PROC_INCYCLE) { // FIXME: need this test?
        PassManager::get()->executePass(PassID::StatementPropagation, this);
    }

    // This part used to be calle middleDecompile():

    PassManager::get()->executePass(PassID::SPPreservation, this);
    // Oops - the idea of splitting the sp from the rest of the preservations was to allow correct naming of locals
    // so you are alias conservative. But of course some locals are ebp (etc) based, and so these will never be correct
    // until all the registers have preservation analysis done. So I may as well do them all together here.
    PassManager::get()->executePass(PassID::PreservationAnalysis, this);
    PassManager::get()->executePass(PassID::CallAndPhiFix, this); // Propagate and bypass sp

    debugPrintAll("After preservation, bypass and propagation");

    // Oh, no, we keep doing preservations till almost the end...
    // setStatus(PROC_PRESERVEDS);        // Preservation done

    if (SETTING(usePromotion)) {
        // We want functions other than main to be promoted. Needed before mapExpressionsToLocals
        promoteSignature();
    }

    // The problem with doing locals too early is that the symbol map ends up with some {-} and some {0}
    // Also, once named as a local, it is tempting to propagate the memory location, but that might be unsafe if the
    // address is taken. But see mapLocalsAndParams just a page below.
    // mapExpressionsToLocals();

    // Update the arguments for calls (mainly for the non recursion affected calls)
    // We have only done limited propagation and collecting to this point. Need e.g. to put m[esp-K]
    // into the collectors of calls, so when a stack parameter is created, it will be correctly localised
    // Note that we'd like to limit propagation before this point, because we have not yet created any arguments, so
    // it is possible to get "excessive propagation" to parameters. In fact, because uses vary so much throughout a
    // program, it may end up better not limiting propagation until very late in the decompilation, and undoing some
    // propagation just before removing unused statements. Or even later, if that is possible.
    // For now, we create the initial arguments here (relatively early), and live with the fact that some apparently
    // distinct memof argument expressions (e.g. m[eax{30}] and m[esp{40}-4]) will turn out to be duplicates, and so
    // the duplicates must be eliminated.
    bool change = PassManager::get()->executePass(PassID::PhiPlacement, this);

    PassManager::get()->executePass(PassID::BlockVarRename, this);
    PassManager::get()->executePass(PassID::StatementPropagation, this); // Otherwise sometimes sp is not fully propagated
    PassManager::get()->executePass(PassID::CallArgumentUpdate, this);
    PassManager::get()->executePass(PassID::StrengthReductionReversal, this);

    // Repeat until no change
    int pass = 3;

    do {
        // Redo the renaming process to take into account the arguments
        change = PassManager::get()->executePass(PassID::PhiPlacement, this);
        change |= PassManager::get()->executePass(PassID::BlockVarRename, this); // E.g. for new arguments

        // Seed the return statement with reaching definitions
        // FIXME: does this have to be in this loop?
        if (m_retStatement) {
            m_retStatement->updateModifieds(); // Everything including new arguments reaching the exit
            m_retStatement->updateReturns();
        }

        // Print if requested
        if (SETTING(verboseOutput)) { // was if debugPrintSSA
            LOG_SEPARATE(getName(), "--- Debug print SSA for %1 pass %2 (no propagations) ---", getName(), pass);
            LOG_SEPARATE(getName(), "%1", this->toString());
            LOG_SEPARATE(getName(), "=== End debug print SSA for %1 pass %2 (no propagations) ===", getName(), pass);
        }

        if (!SETTING(dotFile).isEmpty()) { // Require -gd now (though doesn't listen to file name)
            printDFG();
        }

        Boomerang::get()->alertDecompileSSADepth(this, pass); // FIXME: need depth -> pass in GUI code

// (* Was: mapping expressions to Parameters as we go *)

        // FIXME: Check if this is needed any more. At least fib seems to need it at present.
        if (SETTING(changeSignatures)) {
            // addNewReturns(depth);
            for (int i = 0; i < 3; i++) { // FIXME: should be iterate until no change
                LOG_VERBOSE("### update returns loop iteration %1 ###", i);

                if (getStatus() != PROC_INCYCLE) {
                    PassManager::get()->executePass(PassID::BlockVarRename, this);
                }

                PassManager::get()->executePass(PassID::PreservationAnalysis, this);
                PassManager::get()->executePass(PassID::CallDefineUpdate, this); // Returns have uses which affect call defines (if childless)
                PassManager::get()->executePass(PassID::CallAndPhiFix, this);
                PassManager::get()->executePass(PassID::PreservationAnalysis, this); // Preserveds subtract from returns
            }

            if (SETTING(verboseOutput)) {
                debugPrintAll("SSA (after updating returns");
            }
        }

        // Print if requested
        if (SETTING(verboseOutput)) { // was if debugPrintSSA
            debugPrintAll("SSA (after trimming return set)");
        }

        Boomerang::get()->alertDecompileBeforePropagate(this, pass);
        Boomerang::get()->alertDecompileDebugPoint(this, "Before propagating statements");

        change |= PassManager::get()->executePass(PassID::StatementPropagation, this);
        change |= PassManager::get()->executePass(PassID::BlockVarRename, this);

        Boomerang::get()->alertDecompileAfterPropagate(this, pass);
        Boomerang::get()->alertDecompileDebugPoint(this, "after propagating statements");

         // this is just to make it readable, do NOT rely on these statements being removed
        PassManager::get()->executePass(PassID::AssignRemoval, this);
    } while (change && ++pass < 12);

    // At this point, there will be some memofs that have still not been renamed. They have been prevented from
    // getting renamed so that they didn't get renamed incorrectly (usually as {-}), when propagation and/or bypassing
    // may have ended up changing the address expression. There is now no chance that this will happen, so we need
    // to rename the existing memofs. Note that this can still link uses to definitions, e.g.
    // 50 r26 := phi(...)
    // 51 m[r26{50}] := 99;
    //    ... := m[r26{50}]{should be 51}

    LOG_VERBOSE("### allowing SSA renaming of all memof expressions ###");

    m_df.setRenameLocalsParams(true);

    // Now we need another pass to inert phis for the memofs, rename them and propagate them
    ++pass;

    change = PassManager::get()->executePass(PassID::PhiPlacement, this);
    change |= PassManager::get()->executePass(PassID::BlockVarRename, this);

    debugPrintAll("after setting phis for memofs, renaming them");
    PassManager::get()->executePass(PassID::StatementPropagation, this);

    // Now that memofs are renamed, the bypassing for memofs can work
    PassManager::get()->executePass(PassID::CallAndPhiFix, this); // Bypass children that are finalised (if any)

    if (SETTING(nameParameters)) {
        // ? Crazy time to do this... haven't even done "final" parameters as yet
        // mapExpressionsToParameters();
    }

    // Check for indirect jumps or calls not already removed by propagation of constants
    bool changed = false;
    IndirectJumpAnalyzer analyzer;

    for (BasicBlock *bb : *m_cfg) {
        changed |= analyzer.decodeIndirectJmp(bb, this);
    }

    if (changed) {
        // There was at least one indirect jump or call found and decoded. That means that most of what has been done
        // to this function so far is invalid. So redo everything. Very expensive!!
        // Code pointed to by the switch table entries has merely had FrontEnd::processFragment() called on it
        LOG_MSG("Restarting decompilation of '%1' because indirect jumps or calls have been analyzed", getName());

        Boomerang::get()->alertDecompileDebugPoint(
            this, "Before restarting decompilation because indirect jumps or calls have been analyzed");

        // First copy any new indirect jumps or calls that were decoded this time around. Just copy them all, the map
        // will prevent duplicates
        processDecodedICTs();
        // Now, decode from scratch
        m_retStatement = nullptr;
        m_cfg->clear();

        if (!m_prog->reDecode(this)) {
            return std::shared_ptr<ProcSet>(new ProcSet());
        }

        m_df.setRenameLocalsParams(false);              // Start again with memofs
        setStatus(PROC_VISITED);                        // Back to only visited progress
        assert(callStack.back() == this);
        callStack.pop_back();                                // Remove self from call stack
        std::shared_ptr<ProcSet> ret = decompile(callStack); // Restart decompiling this proc
        callStack.push_back(this);                           // Restore self to call stack
        // It is important to keep the result of this call for the recursion analysis
        return ret;
    }

    PassManager::get()->executePass(PassID::PreservationAnalysis, this);

    // Used to be later...
    if (SETTING(nameParameters)) {
        // findPreserveds();        // FIXME: is this necessary here?
        // fixCallBypass();    // FIXME: surely this is not necessary now?
        // trimParameters();    // FIXME: surely there aren't any parameters to trim yet?
        debugPrintAll("after replacing expressions, trimming params and returns");
    }

    PassManager::get()->executePass(PassID::DuplicateArgsRemoval, this);

    setStatus(PROC_EARLYDONE);

    Boomerang::get()->alertDecompileDebugPoint(this, "after middle");

    return std::make_shared<ProcSet>();
}


void UserProc::remUnusedStmtEtc()
{
    Boomerang::get()->alertDecompiling(this);
    Boomerang::get()->alertDecompileDebugPoint(this, "Before Final");

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

        if (SETTING(verboseOutput)) {
            debugPrintAll("after propagating locals");
        }
    }

    PassManager::get()->executePass(PassID::UnusedStatementRemoval, this);

    Boomerang::get()->alertDecompileAfterRemoveStmts(this, 1);

    PassManager::get()->executePass(PassID::FinalParameterSearch, this);

    if (SETTING(nameParameters)) {
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
    Boomerang::get()->alertDecompileDebugPoint(this, "after final");
}


void UserProc::recursionGroupAnalysis(ProcList &callStack)
{
    /* Overall algorithm:
     *  for each proc in the group
     *          initialise
     *          earlyDecompile
     *  for each proc in the group
     *          middleDecompile
     *  mark all calls involved in cs as non-childless
     *  for each proc in cs
     *          update parameters and returns, redoing call bypass, until no change
     *  for each proc in cs
     *          remove unused statements
     *  for each proc in cs
     *          update parameters and returns, redoing call bypass, until no change
     */

    LOG_VERBOSE("# # # recursion group analysis for # # #");
    for (UserProc *proc : *m_recursionGroup) {
        LOG_VERBOSE("    %1", proc->getName());
    }
    LOG_VERBOSE("# # #");

    // TODO: This requres a "do while changed" loop
    for (int i = 0; i < 3; i++) {
        ProcSet visited;
        assert(callStack.back() == this);
        decompileProcInRecursionGroup(callStack, visited);
    }

    // while no change
    for (int i = 0; i < 2; i++) {
        for (UserProc *proc : *m_recursionGroup) {
            proc->remUnusedStmtEtc(); // Also does final parameters and arguments at present
        }
    }

    LOG_VERBOSE("=== End recursion group analysis ===");
    Boomerang::get()->alertEndDecompile(this);
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
                Function *p = m_prog->findFunction(call->getFixedDest());

                if (p == nullptr) {
                    LOG_FATAL("Cannot find proc for dest %1 in call at %2",
                              call->getFixedDest(), rtl->getAddress());
                }

                call->setDestProc(p);
            }

            // call->setSigArguments();        // But BBs not set yet; will get done in initStatements()
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


void UserProc::searchRegularLocals(OPER minusOrPlus, bool lastPass, int sp, StatementList& stmts)
{
    // replace expressions in regular statements with locals
    SharedExp l;

    if (minusOrPlus == opWild) {
        // l = m[sp{0}]
        l = Location::memOf(RefExp::get(Location::regOf(sp), nullptr));
    }
    else {
        // l = m[sp{0} +/- K]
        l = Location::memOf(
            Binary::get(minusOrPlus, RefExp::get(Location::regOf(sp), nullptr), Terminal::get(opWildIntConst)));
    }

    for (Statement *s : stmts) {
        std::list<SharedExp> results;
        s->searchAll(*l, results);

        for (auto result : results) {
            SharedType ty = s->getTypeFor(result);
            SharedExp  e  = getSymbolExp(result, ty, lastPass);

            if (e) {
                SharedExp search = result->clone();
                LOG_VERBOSE("Mapping %1 to %2 in %3", search, e, s);

                // s->searchAndReplace(search, e);
            }
        }

        // s->simplify();
    }
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

    m_locals[name] = ty;

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


SharedType UserProc::getLocalType(const QString& name)
{
    if (m_locals.find(name) == m_locals.end()) {
        return nullptr;
    }

    SharedType ty = m_locals[name];
    return ty;
}


void UserProc::setLocalType(const QString& name, SharedType ty)
{
    m_locals[name] = ty;

    LOG_VERBOSE("Updating type of %1 to %2", name, ty->getCtype());
}


SharedType UserProc::getParamType(const QString& name)
{
    for (unsigned int i = 0; i < m_signature->getNumParams(); i++) {
        if (name == m_signature->getParamName(i)) {
            return m_signature->getParamType(i);
        }
    }

    return nullptr;
}


void UserProc::mapSymbolToRepl(const SharedConstExp& from, SharedExp oldTo, SharedExp newTo)
{
    removeSymbolMapping(from, oldTo);
    mapSymbolTo(from, newTo); // The compiler could optimise this call to a fall through
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


SharedExp UserProc::getSymbolFor(const SharedConstExp& from, SharedType ty)
{
    SymbolMap::iterator ff = m_symbolMap.find(from);

    while (ff != m_symbolMap.end() && *ff->first == *from) {
        SharedExp currTo = ff->second;
        assert(currTo->isLocal() || currTo->isParam());
        QString    name   = std::static_pointer_cast<Const>(currTo->getSubExp1())->getStr();
        SharedType currTy = getLocalType(name);

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


void UserProc::removeSymbolMapping(const SharedConstExp& from, SharedExp to)
{
    SymbolMap::iterator it = m_symbolMap.find(from);

    while (it != m_symbolMap.end() && *it->first == *from) {
        if (*it->second == *to) {
            m_symbolMap.erase(it);
            return;
        }

        ++it;
    }
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
        if (DEBUG_PROOF) {
            LOG_MSG("found true in provenTrue cache %1 in %2", query, getName());
        }

        return true;
    }

    if (!SETTING(useProof)) {
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
                if (DEBUG_PROOF) {
                    LOG_MSG("Using all=all for %1", query->getSubExp1());
                    LOG_MSG("Prove returns true");
                }

                m_provenTrue[origLeft->clone()] = right;
                return true;
            }

            if (DEBUG_PROOF) {
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

    if (DEBUG_PROOF) {
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
        if (DEBUG_PROOF) {
            LOG_MSG("true - in the phi cache");
        }

        return true;
    }

    std::set<Statement *> refsTo;

    query = query->clone();
    bool change  = true;
    bool swapped = false;

    while (change) {
        if (DEBUG_PROOF) {
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
                                if (DEBUG_PROOF) {
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

                                if (DEBUG_PROOF) {
                                    LOG_MSG("New required premise %1 for %2", newQuery, destProc->getName());
                                }

                                // Pass conditional as true, since even if proven, this is conditional on other things
                                bool result = destProc->prove(newQuery, true);
                                destProc->killPremise(base);

                                if (result) {
                                    if (DEBUG_PROOF) {
                                        LOG_MSG("Conditional preservation with new premise %1 succeeds for %2",
                                                newQuery, destProc->getName());
                                    }

                                    // Use the new conditionally proven result
                                    auto queryLeft = call->localiseExp(base->clone());
                                    query->setSubExp1(queryLeft);
                                    return destProc->prover(query, lastPhis, cache, lastPhi);
                                }
                                else {
                                    if (DEBUG_PROOF) {
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

                            if (DEBUG_PROOF) {
                                LOG_MSG("Using proven for %1 %2 = %3", call->getDestProc()->getName(),
                                        r->getSubExp1(), right);
                            }

                            right = call->localiseExp(right);

                            if (DEBUG_PROOF) {
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

                        if (DEBUG_PROOF) {
                            if (ok) {
                                LOG_MSG("Phi loop detected (set true due to induction)"); // FIXME: induction??!
                            }
                            else {
                                LOG_MSG("Phi loop detected (set false: %1 != %2)", query->getSubExp2(), phiInd);
                            }
                        }
                    }
                    else {
                        if (DEBUG_PROOF) {
                            LOG_MSG("Found %1 prove for each, ", s);
                        }

                        for (RefExp &pi : *pa) {
                            auto e  = query->clone();
                            auto r1 = e->access<RefExp, 1>();
                            r1->setDef(pi.getDef());

                            if (DEBUG_PROOF) {
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

        if (change && !(*old == *query) && DEBUG_PROOF) {
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


bool UserProc::ellipsisProcessing()
{
    bool ch = false;

    for (BasicBlock *bb : *m_cfg) {
        BasicBlock::RTLRIterator        rrit;
        StatementList::reverse_iterator srit;
        CallStatement *c = dynamic_cast<CallStatement *>(bb->getLastStmt(rrit, srit));

        // Note: we may have removed some statements, so there may no longer be a last statement!
        if (c == nullptr) {
            continue;
        }

        ch |= c->ellipsisProcessing(m_prog);
    }

    if (ch) {
        PassManager::get()->executePass(PassID::CallAndPhiFix, this);
    }

    return ch;
}


QString UserProc::lookupParam(SharedExp e)
{
    // Originally e.g. m[esp+K]
    Statement *def = m_cfg->findTheImplicitAssign(e);

    if (def == nullptr) {
        LOG_ERROR("No implicit definition for parameter %1!", e);
        return QString::null;
    }

    auto       re = RefExp::get(e, def);
    SharedType ty = def->getTypeFor(e);
    return lookupSym(re, ty);
}


QString UserProc::lookupSymFromRef(const std::shared_ptr<RefExp>& r)
{
    Statement *def = r->getDef();

    if (!def) {
        LOG_WARN("Unknown def for RefExp '%1' in '%2'", r->toString(), getName());
        return QString::null;
    }

    auto       base = r->getSubExp1();
    SharedType ty   = def->getTypeFor(base);
    return lookupSym(r, ty);
}


QString UserProc::lookupSymFromRefAny(const std::shared_ptr<RefExp>& r)
{
    Statement *def = r->getDef();

    if (!def) {
        LOG_WARN("Unknown def for RefExp '%1' in '%2'", r->toString(), getName());
        return QString::null;
    }

    SharedExp  base = r->getSubExp1();
    SharedType ty   = def->getTypeFor(base);
    QString    ret  = lookupSym(r, ty);

    if (!ret.isNull()) {
        return ret;             // Found a specific symbol
    }

    return lookupSym(base, ty); // Check for a general symbol
}


QString UserProc::lookupSym(const SharedConstExp& arg, SharedType ty)
{
    SharedConstExp e = arg;

    if (arg->isTypedExp()) {
        e = e->getSubExp1();
    }

    SymbolMap::iterator it;
    it = m_symbolMap.find(e);

    while (it != m_symbolMap.end() && *it->first == *e) {
        auto sym = it->second;
        assert(sym->isLocal() || sym->isParam());
        QString    name = sym->access<Const, 1>()->getStr();
        SharedType type = getLocalType(name);

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
            const SharedType ty = getTypeForLocation(it.second);
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


void UserProc::dumpSymbolMap() const
{
    for (const auto& val : m_symbolMap) {
        SharedType ty = getTypeForLocation(val.second);
        LOG_MSG("  %1 maps to %2 type %3", val.first, val.second, (ty ? qPrintable(ty->getCtype()) : "NULL"));
    }
}


void UserProc::dumpSymbolMapx() const
{
    for (const auto& val : m_symbolMap) {
        SharedType ty = getTypeForLocation(val.second);
        LOG_MSG("  %1 maps to %2 type %3", val.first, val.second, (ty ? qPrintable(ty->getCtype()) : "NULL"));
        val.first->printx(2);
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

        m_signature->addParameter(a->getType(), paramName, a->getLeft());
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
            int sp = 999;

            if (m_signature) {
                sp = m_signature->getStackRegister(m_prog);
            }

            int r = e->access<Const, 1>()->getInt();
            return r == sp;
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
                    sp = m_signature->getStackRegister(m_prog);
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


QString UserProc::findLocalFromRef(const std::shared_ptr<RefExp>& r)
{
    Statement  *def = r->getDef();
    SharedExp  base = r->getSubExp1();
    SharedType ty   = def->getTypeFor(base);
    // QString name = lookupSym(*base, ty); ?? this actually worked a bit
    QString name = lookupSym(r, ty);

    if (name.isNull()) {
        return name;
    }

    // Now make sure it is a local; some symbols (e.g. parameters) are in the symbol map but not locals
    if (m_locals.find(name) != m_locals.end()) {
        return name;
    }

    return QString::null;
}


QString UserProc::findFirstSymbol(const SharedExp& e)
{
    SymbolMap::iterator ff = m_symbolMap.find(e);

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
    UseCollector::iterator it;

    for (it = m_procUseCollector.begin(); it != m_procUseCollector.end();) {
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

    int               sp = m_signature->getStackRegister();
    static const auto initSp(RefExp::get(Location::regOf(sp), nullptr)); // sp{-}

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


bool UserProc::doesParamChainToCall(SharedExp param, UserProc *p, ProcSet *visited)
{
    BasicBlock::RTLRIterator        rrit;
    StatementList::reverse_iterator srit;

    for (BasicBlock *pb : *m_cfg) {
        CallStatement *c = static_cast<CallStatement *>(pb->getLastStmt(rrit, srit));

        if ((c == nullptr) || !c->isCall()) {
            continue; // Only interested in calls
        }

        UserProc *dest = static_cast<UserProc *>(c->getDestProc());

        if (!dest || dest->isLib()) {
            continue;    // Only interested in calls to UserProcs
        }

        if (dest == p) { // Pointer comparison is OK here
            // This is a recursive call to p. Check for an argument of the form param{-}
            // FIXME: should be looking for component
            const StatementList& args = c->getArguments();
            for (StatementList::const_iterator aa = args.begin(); aa != args.end(); ++aa) {
                const Assign *a = dynamic_cast<const Assign *>(*aa);
                SharedExp rhs = a ? a->getRight() : nullptr;

                if (rhs && rhs->isSubscript() && rhs->access<RefExp>()->isImplicitDef()) {
                    SharedExp base = rhs->getSubExp1();

                    // Check if this argument location matches loc
                    if (*base == *param) {
                        // We have a call to p that takes param{-} as an argument
                        return true;
                    }
                }
            }
        }
        else {
            if (dest->doesRecurseTo(p)) {
                // We have come to a call that is not to p, but is in the same recursion group as p and this proc.
                visited->insert(this);

                if (visited->find(dest) != visited->end()) {
                    // Recurse to the next proc
                    bool res = dest->doesParamChainToCall(param, p, visited);

                    if (res) {
                        return true;
                    }

                    // TODO: Else consider more calls this proc
                }
            }
        }
    }

    return false;
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

                if (u.existsImplicit(bparam)) {
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

                    if (argUses.existsImplicit(bparam)) {
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

        if (uses.existsImplicit(bparam)) {
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

    Boomerang::get()->alertDecompileDebugPoint(this, "Before removing redundant parameters");

    if (DEBUG_UNUSED) {
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
        bparam = bparam->accept(&ic);                            // Now m[sp{0}+K]{0}
        assert(bparam->isSubscript());
        bparam = bparam->access<Exp, 1>();                       // now m[sp{0}+K] (bare parameter)

        ProcSet visited;

        if (checkForGainfulUse(bparam, visited)) {
            newParameters.append(*pp); // Keep this parameter
        }
        else {
            // Remove the parameter
            ret = true;

            if (DEBUG_UNUSED) {
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

    getParameters() = newParameters;

    if (DEBUG_UNUSED) {
        LOG_MSG("%%% end removing unused parameters for %1", getName());
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "after removing redundant parameters");

    return ret;
}


bool UserProc::removeRedundantReturns(std::set<UserProc *>& removeRetSet)
{
    Boomerang::get()->alertDecompiling(this);
    Boomerang::get()->alertDecompileDebugPoint(this, "before removing unused returns");
    // First remove the unused parameters
    bool removedParams = removeRedundantParameters();

    if (m_retStatement == nullptr) {
        return removedParams;
    }

    if (DEBUG_UNUSED) {
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

            for (unsigned int i = 0; i < m_signature->getNumReturns(); i++) {
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

                if (DEBUG_UNUSED) {
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

        if (DEBUG_UNUSED) {
            LOG_MSG("%%%  removing unused return %1 from proc %2", a, getName());
        }

        // If a component of the RHS referenced a call statement, the liveness used to be killed here.
        // This was wrong; you need to notice the liveness changing inside updateForUseChange() to correctly
        // recurse to callee
        rr          = m_retStatement->erase(rr);
        removedRets = true;
    }

    if (DEBUG_UNUSED) {
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

        for (CallStatement *call : m_callerSet) {
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

    if (m_retStatement->getNumReturns() == 1) {
        const Assign *a = static_cast<Assign *>(m_retStatement->getReturns().front());
        m_signature->setRetType(a->getType());
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "after removing unused and redundant returns");
    return removedRets || removedParams;
}


void UserProc::updateForUseChange(std::set<UserProc *>& removeRetSet)
{
    // We need to remember the parameters, and all the livenesses for all the calls, to see if these are changed
    // by removing returns
    if (DEBUG_UNUSED) {
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
        if (DEBUG_UNUSED) {
            LOG_MSG("%%%  parameters changed for %1", getName());
        }

        std::set<CallStatement *>& callers = getCallers();

        for (CallStatement *cc : callers) {
            cc->updateArguments();
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
            if (DEBUG_UNUSED) {
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

        if (DEBUG_SWITCH) {
            LOG_MSG("Saving high level switch statement %1", rtl);
        }

        m_prog->addDecodedRTL(bb->getHiAddr(), rtl);
        // Now decode those new targets, adding out edges as well
        //        if (last->isCase())
        //            bb->processSwitch(this);
    }
}


void UserProc::mapLocalsAndParams()
{
    Boomerang::get()->alertDecompileDebugPoint(this, "Before mapping locals from dfa type analysis");

    if (DEBUG_TA) {
        LOG_MSG("### Mapping expressions to local variables for %1 ###", getName());
    }

    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        s->dfaMapLocals();
    }

    if (DEBUG_TA) {
        LOG_MSG("### End mapping expressions to local variables for %1 ###", getName());
    }
}


#if USE_DOMINANCE_NUMS
void UserProc::setDominanceNumbers()
{
    int currNum = 1;

    m_df.setDominanceNums(0, currNum);
}
#endif


void UserProc::findPhiUnites(ConnectionGraph& pu)
{
    StatementList stmts;

    getStatements(stmts);

    for (Statement *stmt : stmts) {
        if (!stmt->isPhi()) {
            continue;
        }

        PhiAssign *pa   = static_cast<PhiAssign *>(stmt);
        SharedExp lhs   = pa->getLeft();
        auto      reLhs = RefExp::get(lhs, pa);

        for (RefExp& v : *pa) {
            assert(v.getSubExp1());
            auto re = RefExp::get(v.getSubExp1(), v.getDef());
            pu.connect(reLhs, re);
        }
    }
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


SharedType UserProc::getTypeForLocation(const SharedConstExp& e)
{
    QString name = e->access<Const, 1>()->getStr();

    if (e->isLocal()) {
        if (m_locals.find(name) != m_locals.end()) {
            return m_locals[name];
        }
    }

    // Sometimes parameters use opLocal, so fall through
    return getParamType(name);
}


const SharedType UserProc::getTypeForLocation(const SharedConstExp& e) const
{
    return const_cast<UserProc *>(this)->getTypeForLocation(e);
}


bool UserProc::existsLocal(const QString& name) const
{
    return m_locals.find(name) != m_locals.end();
}


void UserProc::checkLocalFor(const std::shared_ptr<RefExp>& r)
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


void UserProc::decompileProcInRecursionGroup(ProcList &callStack, ProcSet &visited)
{
    UserProc *current = callStack.back();
    visited.insert(current);

    for (Function *c : current->getCallees()) {
        if (c->isLib()) {
            continue;
        }

        UserProc *callee = static_cast<UserProc *>(c);
        if (visited.find(callee) != visited.end()) {
            continue;
        }
        else if (m_recursionGroup->find(callee) == m_recursionGroup->end()) {
            // not in recursion group any more
            continue;
        }

        // visit unvisited callees first
        callStack.push_back(callee);
        decompileProcInRecursionGroup(callStack, visited);
        callStack.pop_back();
    }

    current->setStatus(PROC_INCYCLE); // So the calls are treated as childless
    Boomerang::get()->alertDecompiling(current);
    current->earlyDecompile();

    // The standard preservation analysis should automatically perform conditional preservation.
    current->middleDecompile(callStack);
    current->setStatus(PROC_PRESERVEDS);

    // Mark all the relevant calls as non childless (will harmlessly get done again later)
    // FIXME: why exactly do we do this?
    current->markAsNonChildless(m_recursionGroup);

    // Need to propagate into the initial arguments, since arguments are uses, and we are about to remove unused
    // statements.
    current->mapLocalsAndParams();
    PassManager::get()->executePass(PassID::CallArgumentUpdate, this);
    PassManager::get()->executePass(PassID::Dominators, this);
    PassManager::get()->executePass(PassID::StatementPropagation, this); // Need to propagate into arguments
}

