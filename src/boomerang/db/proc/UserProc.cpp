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


void UserProc::setParamType(const char *nam, SharedType ty)
{
    m_signature->setParamType(nam, ty);
}


void UserProc::setParamType(int idx, SharedType ty)
{
    auto it = std::next(m_parameters.begin(), idx);

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


void UserProc::initStatements()
{
    BasicBlock::RTLIterator       rit;
    StatementList::iterator sit;

    for (BasicBlock *bb : *m_cfg) {
        for (Statement *stmt = bb->getFirstStmt(rit, sit); stmt != nullptr; stmt = bb->getNextStmt(rit, sit)) {
            stmt->setProc(this);
            stmt->setBB(bb);
            CallStatement *call = dynamic_cast<CallStatement *>(stmt);

            if (call) {
                call->setSigArguments();

                // Remove out edges of BBs of noreturn calls (e.g. call BBs to abort())
                if (call->getDestProc() && call->getDestProc()->isNoReturn() && (bb->getNumSuccessors() == 1)) {
                    BasicBlock *nextBB = bb->getSuccessor(0);

                    if ((nextBB != m_cfg->getExitBB()) || (m_cfg->getExitBB()->getNumPredecessors() != 1)) {
                        nextBB->removePredecessor(bb);
                        bb->removeAllSuccessors();
                    }
                }
            }
        }
    }
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


void UserProc::insertStatementAfter(Statement *s, Statement *a)
{
    for (BasicBlock *bb : *m_cfg) {
        RTLList *rtls = bb->getRTLs();

        if (rtls == nullptr) {
            continue; // e.g. bb is (as yet) invalid
        }

        for (const auto& rtl : *rtls) {
            for (RTL::iterator ss = rtl->begin(); ss != rtl->end(); ++ss) {
                if (*ss == s) {
                    ++ss; // This is the point to insert before
                    rtl->insert(ss, a);
                    a->setBB(bb);
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
                LOG_VERBOSE("Visiting on the way down child %1 from %2", callee->getName(), getName());

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

        initialiseDecompile(); // Sort the CFG, number statements, etc
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

    LOG_VERBOSE("End decompile(%1)", getName());
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


/*    *    *    *    *    *    *    *    *    *    *    *
*                                            *
*        D e c o m p i l e   p r o p e r        *
*            ( i n i t i a l )                *
*                                            *
*    *    *    *    *    *    *    *    *    *    *    */
void UserProc::initialiseDecompile()
{
    Boomerang::get()->alertStartDecompile(this);
    Boomerang::get()->alertDecompileDebugPoint(this, "Before Initialise");

    LOG_VERBOSE("Initialise decompile for %1", getName());

    // Initialise statements
    initStatements();

    debugPrintAll("Before SSA");

    // Compute dominance frontier
    m_df.calculateDominators();

    if (!SETTING(decompile)) {
        LOG_MSG("Not decompiling.");
        setStatus(PROC_FINAL); // ??!
        return;
    }

    debugPrintAll("After Decoding");
    Boomerang::get()->alertDecompileDebugPoint(this, "After Initialise");
}


void UserProc::earlyDecompile()
{
    if (m_status >= PROC_EARLYDONE) {
        return;
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "Before Early");
    LOG_VERBOSE("Early decompile for %1", getName());

    // Update the defines in the calls. Will redo if involved in recursion
    updateCallDefines();

    // This is useful for obj-c
    replaceSimpleGlobalConstants();

    // First placement of phi functions, renaming, and initial propagation. This is mostly for the stack pointer
    // TODO: Check if this makes sense. It seems to me that we only want to do one pass of propagation here, since
    // the status == check had been knobbled below. Hopefully, one call to placing phi functions etc will be
    // equivalent to depth 0 in the old scheme
    LOG_VERBOSE("Placing phi functions 1st pass");

    // Place the phi functions
    m_df.placePhiFunctions();
    debugPrintAll("after placing phis (1)");

    // Rename variables
    LOG_VERBOSE("Renaming block variables 1st pass");
    doRenameBlockVars(1, true);
    debugPrintAll("after rename (1)");

    bool convert;
    propagateStatements(convert, 1);

    debugPrintAll("After propagation (1)");

    Boomerang::get()->alertDecompileDebugPoint(this, "After Early");
}


std::shared_ptr<ProcSet> UserProc::middleDecompile(ProcList &callStack)
{
    assert(callStack.back() == this);
    Boomerang::get()->alertDecompileDebugPoint(this, "Before Middle");

    // The call bypass logic should be staged as well. For example, consider m[r1{11}]{11} where 11 is a call.
    // The first stage bypass yields m[r1{2}]{11}, which needs another round of propagation to yield m[r1{-}-32]{11}
    // (which can safely be processed at depth 1).
    // Except that this is now inherent in the visitor nature of the latest algorithm.
    fixCallAndPhiRefs(); // Bypass children that are finalised (if any)
    bool convert;

    if (m_status != PROC_INCYCLE) { // FIXME: need this test?
        propagateStatements(convert, 2);
    }

    debugPrintAll("After call and phi bypass (1)");

    // This part used to be calle middleDecompile():

    findSpPreservation();
    // Oops - the idea of splitting the sp from the rest of the preservations was to allow correct naming of locals
    // so you are alias conservative. But of course some locals are ebp (etc) based, and so these will never be correct
    // until all the registers have preservation analysis done. So I may as well do them all together here.
    findPreserveds();
    fixCallAndPhiRefs(); // Propagate and bypass sp

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
    bool change = m_df.placePhiFunctions();

    doRenameBlockVars(2);
    propagateStatements(convert, 2); // Otherwise sometimes sp is not fully propagated
    updateArguments();
    reverseStrengthReduction();

    // Repeat until no change
    int pass;

    for (pass = 3; pass <= 12; ++pass) {
        // Redo the renaming process to take into account the arguments
        LOG_VERBOSE("Renaming block variables (2) pass %1", pass);

        // Rename variables
        change = m_df.placePhiFunctions();
        change |= doRenameBlockVars(pass, false); // E.g. for new arguments

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

                if (m_status != PROC_INCYCLE) {
                    doRenameBlockVars(pass, true);
                }

                findPreserveds();
                updateCallDefines(); // Returns have uses which affect call defines (if childless)
                fixCallAndPhiRefs();
                findPreserveds();    // Preserveds subtract from returns
            }

            if (SETTING(verboseOutput)) {
                LOG_SEPARATE(getName(), "--- debug print SSA for %1 at pass %2 (after updating returns) ---", getName(), pass);
                LOG_SEPARATE(getName(), "%1", this->toString());
                LOG_SEPARATE(getName(), "=== end debug print SSA for %1 at pass %2 ===", getName(), pass);
            }
        }

        // Print if requested
        if (SETTING(verboseOutput)) { // was if debugPrintSSA
            LOG_SEPARATE(getName(), "--- debug print SSA for %1 at pass %2 (after trimming return set) ---", getName(), pass);
            LOG_SEPARATE(getName(), "%1", this->toString());
            LOG_SEPARATE(getName(), "=== end debug print SSA for %1 at pass %2 ===", getName(), pass);
        }

        Boomerang::get()->alertDecompileBeforePropagate(this, pass);
        Boomerang::get()->alertDecompileDebugPoint(this, "Before propagating statements");

        // Propagate
        bool _convert; // True when indirect call converted to direct

        do {
            _convert = false;
            LOG_VERBOSE("Propagating at pass %1", pass);
            change |= propagateStatements(_convert, pass);
            change |= doRenameBlockVars(pass, true);

            // If you have an indirect to direct call conversion, some propagations that were blocked by
            // the indirect call might now succeed, and may be needed to prevent alias problems
            // FIXME: I think that the below, and even the convert parameter to propagateStatements(), is no longer
            // needed - MVE
            if (_convert) {
                LOG_VERBOSE("About to restart propagations and dataflow at pass %1 due to conversion of indirect to direct call(s)", pass);

                m_df.setRenameLocalsParams(false);
                change |= doRenameBlockVars(0, true); // Initial dataflow level 0
                LOG_SEPARATE(getName(), "After rename (2) of %1:", getName());
                LOG_SEPARATE(getName(), "%1", this->toString());
                LOG_SEPARATE(getName(), "Done after rename (2) of %1:", getName());
            }
        } while (_convert);

        if (SETTING(verboseOutput)) {
            LOG_SEPARATE(getName(), "--- after propagate for %1 at pass %2 ---", getName(), pass);
            LOG_SEPARATE(getName(), "%1", this->toString());
            LOG_SEPARATE(getName(), "=== End propagate for %1 at pass %2 ===", getName(), pass);
        }

        Boomerang::get()->alertDecompileAfterPropagate(this, pass);
        Boomerang::get()->alertDecompileDebugPoint(this, "after propagating statements");

        // this is just to make it readable, do NOT rely on these statements being removed
        removeSpAssignsIfPossible();
        // The problem with removing %flags and %CF is that %CF is a subset of %flags
        // removeMatchingAssignsIfPossible(Terminal::get(opFlags));
        // removeMatchingAssignsIfPossible(Terminal::get(opCF));
        removeMatchingAssignsIfPossible(Unary::get(opTemp, Terminal::get(opWildStrConst)));
        removeMatchingAssignsIfPossible(Terminal::get(opPC));

        // processTypes();

        if (!change) {
            break; // Until no change
        }
    }

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

    LOG_VERBOSE("Setting phis, renaming block variables after memofs renamable pass %1", pass);

    change = m_df.placePhiFunctions();
    doRenameBlockVars(pass, false); // MVE: do we want this parameter false or not?

    debugPrintAll("after setting phis for memofs, renaming them");
    propagateStatements(convert, pass);
    // Now that memofs are renamed, the bypassing for memofs can work
    fixCallAndPhiRefs(); // Bypass children that are finalised (if any)

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

    findPreserveds();

    // Used to be later...
    if (SETTING(nameParameters)) {
        // findPreserveds();        // FIXME: is this necessary here?
        // fixCallBypass();    // FIXME: surely this is not necessary now?
        // trimParameters();    // FIXME: surely there aren't any parameters to trim yet?
        debugPrintAll("after replacing expressions, trimming params and returns");
    }

    eliminateDuplicateArgs();

    LOG_VERBOSE("===== End early decompile for %1 =====", getName());

    setStatus(PROC_EARLYDONE);

    Boomerang::get()->alertDecompileDebugPoint(this, "after middle");

    return std::make_shared<ProcSet>();
}


void UserProc::remUnusedStmtEtc()
{
    Boomerang::get()->alertDecompiling(this);
    Boomerang::get()->alertDecompileDebugPoint(this, "Before Final");

    LOG_VERBOSE("--- Remove unused statements for %1 ---", getName());
    // A temporary hack to remove %CF = %CF{7} when 7 isn't a SUBFLAGS
    //    if (theReturnStatement)
    //        theReturnStatement->specialProcessing();

    // Perform type analysis. If we are relying (as we are at present) on TA to perform ellipsis processing,
    // do the local TA pass now. Ellipsis processing often reveals additional uses (e.g. additional parameters
    // to printf/scanf), and removing unused statements is unsafe without full use information
    if (m_status < PROC_FINAL) {
        typeAnalysis();

        // Now that locals are identified, redo the dataflow
        m_df.placePhiFunctions();

        doRenameBlockVars(20);            // Rename the locals
        bool convert = false;
        propagateStatements(convert, 20); // Surely need propagation too

        if (SETTING(verboseOutput)) {
            debugPrintAll("after propagating locals");
        }
    }

    // Only remove unused statements after decompiling as much as possible of the proc
    // Remove unused statements
    RefCounter refCounts; // The map
    // Count the references first
    countRefs(refCounts);

    // Now remove any that have no used
    if (SETTING(removeNull)) {
        remUnusedStmtEtc(refCounts);
    }

    // Remove null statements
    if (SETTING(removeNull)) {
        removeNullStatements();
    }


    if (SETTING(removeNull)) {
        debugPrintAll("after removing unused and null statements pass 1");
    }

    Boomerang::get()->alertDecompileAfterRemoveStmts(this, 1);

    findFinalParameters();

    if (SETTING(nameParameters)) {
        // Replace the existing temporary parameters with the final ones:
        // mapExpressionsToParameters();
        addParameterSymbols();
        debugPrintAll("after adding new parameters");
    }

    updateCalls(); // Or just updateArguments?

    bool removedBBs = branchAnalysis();
    fixUglyBranches();

    if (removedBBs) {
        // redo the data flow
        m_df.calculateDominators();

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


void UserProc::remUnusedStmtEtc(RefCounter& refCounts)
{
    Boomerang::get()->alertDecompileDebugPoint(this, "before remUnusedStmtEtc");

    StatementList stmts;
    getStatements(stmts);
    bool change;

    do { // FIXME: check if this is ever needed
        change = false;
        StatementList::iterator ll = stmts.begin();

        while (ll != stmts.end()) {
            Statement *s = *ll;

            if (!s->isAssignment()) {
                // Never delete a statement other than an assignment (e.g. nothing "uses" a Jcond)
                ++ll;
                continue;
            }

            const Assignment *as    = static_cast<const Assignment *>(s);
            SharedConstExp  asLeft = as->getLeft();

            // If depth < 0, consider all depths
            // if (asLeft && depth >= 0 && asLeft->getMemDepth() > depth) {
            //    ++ll;
            //    continue;
            // }
            if (asLeft && (asLeft->getOper() == opGlobal)) {
                // assignments to globals must always be kept
                ++ll;
                continue;
            }

            // If it's a memof and renameable it can still be deleted
            if ((asLeft->getOper() == opMemOf) && !canRename(asLeft)) {
                // Assignments to memof-anything-but-local must always be kept.
                ++ll;
                continue;
            }

            if ((asLeft->getOper() == opMemberAccess) || (asLeft->getOper() == opArrayIndex)) {
                // can't say with these; conservatively never remove them
                ++ll;
                continue;
            }

            if ((refCounts.find(s) == refCounts.end()) || (refCounts[s] == 0)) { // Care not to insert unnecessarily
                // First adjust the counts, due to statements only referenced by statements that are themselves unused.
                // Need to be careful not to count two refs to the same def as two; refCounts is a count of the number
                // of statements that use a definition, not the total number of refs
                StatementSet stmtsRefdByUnused;
                LocationSet    components;
                s->addUsedLocs(components, false); // Second parameter false to ignore uses in collectors

                for (auto cc = components.begin(); cc != components.end(); ++cc) {
                    if ((*cc)->isSubscript()) {
                        stmtsRefdByUnused.insert((*cc)->access<RefExp>()->getDef());
                    }
                }

                for (auto dd = stmtsRefdByUnused.begin(); dd != stmtsRefdByUnused.end(); ++dd) {
                    if (*dd == nullptr) {
                        continue;
                    }

                    if (DEBUG_UNUSED) {
                        LOG_MSG("Decrementing ref count of %1 because %2 is unused", (*dd)->getNumber(), s->getNumber());
                    }

                    refCounts[*dd]--;
                }

                if (DEBUG_UNUSED) {
                    LOG_MSG("Removing unused statement %1 %2", s->getNumber(), s);
                }

                removeStatement(s);
                ll     = stmts.erase(ll); // So we don't try to re-remove it
                change = true;
                continue;                 // Don't call getNext this time
            }

            ++ll;
        }
    } while (change);

    // Recaluclate at least the livenesses. Example: first call to printf in test/pentium/fromssa2, eax used only in a
    // removed statement, so liveness in the call needs to be removed
    removeCallLiveness();  // Kill all existing livenesses
    doRenameBlockVars(-2); // Recalculate new livenesses
    setStatus(PROC_FINAL); // Now fully decompiled (apart from one final pass, and transforming out of SSA form)

    Boomerang::get()->alertDecompileDebugPoint(this, "after remUnusedStmtEtc");
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


void UserProc::updateCalls()
{
    LOG_VERBOSE("### updateCalls for %1 ###", getName());

    updateCallDefines();
    updateArguments();
    debugPrintAll("After update calls");
}


bool UserProc::branchAnalysis()
{
    Boomerang::get()->alertDecompileDebugPoint(this, "Before branch analysis");
    StatementList stmts;
    getStatements(stmts);

    std::set<BasicBlock *> bbsToRemove;

    for (Statement *stmt : stmts) {
        if (!stmt->isBranch()) {
            continue;
        }

        BranchStatement *firstBranch = static_cast<BranchStatement *>(stmt);

        if (!firstBranch->getFallBB() || !firstBranch->getTakenBB()) {
            continue;
        }

        StatementList fallstmts;
        firstBranch->getFallBB()->appendStatementsTo(fallstmts);
        Statement *nextAfterBranch = !fallstmts.empty() ? fallstmts.front() : nullptr;

        if (nextAfterBranch && nextAfterBranch->isBranch()) {
            BranchStatement *secondBranch = static_cast<BranchStatement *>(nextAfterBranch);

            //   branch to A if cond1
            //   branch to B if cond2
            // A: something
            // B:
            // ->
            //   branch to B if !cond1 && cond2
            // A: something
            // B:
            //
            if ((secondBranch->getFallBB() == firstBranch->getTakenBB()) &&
                (secondBranch->getBB()->getNumPredecessors() == 1)) {

                SharedExp cond =
                    Binary::get(opAnd, Unary::get(opNot, firstBranch->getCondExpr()), secondBranch->getCondExpr()->clone());
                firstBranch->setCondExpr(cond->simplify());

                firstBranch->setDest(secondBranch->getFixedDest());
                firstBranch->setTakenBB(secondBranch->getTakenBB());
                firstBranch->setFallBB(secondBranch->getFallBB());

                // remove second branch BB
                BasicBlock *secondBranchBB = secondBranch->getBB();

                assert(secondBranchBB->getNumPredecessors() == 0);
                assert(secondBranchBB->getNumSuccessors() == 2);
                BasicBlock *succ1 = secondBranch->getBB()->getSuccessor(BTHEN);
                BasicBlock *succ2 = secondBranch->getBB()->getSuccessor(BELSE);

                secondBranchBB->removeSuccessor(succ1);
                secondBranchBB->removeSuccessor(succ2);
                succ1->removePredecessor(secondBranchBB);
                succ2->removePredecessor(secondBranchBB);

                bbsToRemove.insert(secondBranchBB);
            }

            //   branch to B if cond1
            //   branch to B if cond2
            // A: something
            // B:
            // ->
            //   branch to B if cond1 || cond2
            // A: something
            // B:
            if ((secondBranch->getTakenBB() == firstBranch->getTakenBB()) &&
                (secondBranch->getBB()->getNumPredecessors() == 1)) {

                SharedExp cond = Binary::get(opOr, firstBranch->getCondExpr(), secondBranch->getCondExpr()->clone());
                firstBranch->setCondExpr(cond->simplify());

                firstBranch->setFallBB(secondBranch->getFallBB());

                BasicBlock *secondBranchBB = secondBranch->getBB();
                assert(secondBranchBB->getNumPredecessors() == 0);
                assert(secondBranchBB->getNumSuccessors() == 2);
                BasicBlock *succ1 = secondBranchBB->getSuccessor(BTHEN);
                BasicBlock *succ2 = secondBranchBB->getSuccessor(BELSE);

                secondBranchBB->removeSuccessor(succ1);
                secondBranchBB->removeSuccessor(succ2);
                succ1->removePredecessor(secondBranchBB);
                succ2->removePredecessor(secondBranchBB);

                bbsToRemove.insert(secondBranchBB);
            }
        }
    }

    const bool removedBBs = !bbsToRemove.empty();
    for (BasicBlock *bb : bbsToRemove) {
        m_cfg->removeBB(bb);
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "After branch analysis.");
    return removedBBs;
}


void UserProc::fixUglyBranches()
{
    LOG_VERBOSE("### fixUglyBranches for '%1' ###", getName());

    StatementList stmts;
    getStatements(stmts);

    for (auto stmt : stmts) {
        if (!stmt->isBranch()) {
            continue;
        }

        SharedExp hl = static_cast<BranchStatement *>(stmt)->getCondExpr();

        // of the form: x{n} - 1 >= 0
        if (hl && (hl->getOper() == opGtrEq) && hl->getSubExp2()->isIntConst() &&
            (hl->access<Const, 2>()->getInt() == 0) && (hl->getSubExp1()->getOper() == opMinus) &&
            hl->getSubExp1()->getSubExp2()->isIntConst() && (hl->access<Const, 1, 2>()->getInt() == 1) &&
            hl->getSubExp1()->getSubExp1()->isSubscript()) {
            Statement *n = hl->access<RefExp, 1, 1>()->getDef();

            if (n && n->isPhi()) {
                PhiAssign *p = static_cast<PhiAssign *>(n);

                for (const auto& phi : *p) {
                    if (!phi.getDef()->isAssign()) {
                        continue;
                    }

                    Assign *a = static_cast<Assign *>(phi.getDef());

                    if (*a->getRight() == *hl->getSubExp1()) {
                        hl->setSubExp1(RefExp::get(a->getLeft(), a));
                        break;
                    }
                }
            }
        }
    }

    debugPrintAll("After fixUglyBranches");
}


bool UserProc::doRenameBlockVars(int pass, bool clearStacks)
{
    LOG_VERBOSE("### Rename block vars for %1 pass %2, clear = %3 ###", getName(), pass, clearStacks);
    bool b = m_df.renameBlockVars(0, clearStacks);
    LOG_VERBOSE("df.renameBlockVars return %1", (b ? "true" : "false"));
    return b;
}


void UserProc::findSpPreservation()
{
    LOG_VERBOSE("Finding stack pointer preservation for %1", getName());

    bool stdsp = false; // FIXME: are these really used?
    // Note: need this non-virtual version most of the time, since nothing proved yet
    int sp = m_signature->getStackRegister(m_prog);

    for (int n = 0; n < 2; n++) {
        // may need to do multiple times due to dependencies FIXME: efficiency! Needed any more?

        // Special case for 32-bit stack-based machines (e.g. Pentium).
        // RISC machines generally preserve the stack pointer (so no special case required)
        for (int p = 0; !stdsp && p < 8; p++) {
            if (DEBUG_PROOF) {
                LOG_MSG("Attempting to prove sp = sp + %1 for %2", p * 4, getName());
            }

            stdsp = prove(
                Binary::get(opEquals,
                            Location::regOf(sp),
                            Binary::get(opPlus, Location::regOf(sp), Const::get(p * 4))));
        }
    }

    if (DEBUG_PROOF) {
        LOG_MSG("Proven for %1:", getName());

        for (auto& elem : m_provenTrue) {
            LOG_MSG("    %1 = %2", elem.first, elem.second);
        }
    }
}


void UserProc::findPreserveds()
{
    std::set<SharedExp> removes;

    LOG_VERBOSE("Finding preserveds for %1", getName());

    Boomerang::get()->alertDecompileDebugPoint(this, "before finding preserveds");

    if (m_retStatement == nullptr) {
        if (DEBUG_PROOF) {
            LOG_MSG("Can't find preservations as there is no return statement!");
        }

        Boomerang::get()->alertDecompileDebugPoint(this, "after finding preserveds (no return)");
        return;
    }

    // prove preservation for all modifieds in the return statement
    StatementList& modifieds = m_retStatement->getModifieds();

    for (ReturnStatement::iterator mm = modifieds.begin(); mm != modifieds.end(); ++mm) {
        SharedExp lhs      = static_cast<Assignment *>(*mm)->getLeft();
        auto      equation = Binary::get(opEquals, lhs, lhs);

        if (DEBUG_PROOF) {
            LOG_MSG("attempting to prove %1 is preserved by %2", equation, getName());
        }

        if (prove(equation)) {
            removes.insert(equation);
        }
    }

    if (DEBUG_PROOF) {
        LOG_MSG("### proven true for procedure %1:", getName());

        for (auto& elem : m_provenTrue) {
            LOG_MSG("  %1 = %2", elem.first, elem.second);
        }

        LOG_MSG("### End proven true for procedure %1", getName());
    }

    // Remove the preserved locations from the modifieds and the returns
    for (auto pp = m_provenTrue.begin(); pp != m_provenTrue.end(); ++pp) {
        SharedExp lhs = pp->first;
        SharedExp rhs = pp->second;

        // Has to be of the form loc = loc, not say loc+4, otherwise the bypass logic won't see the add of 4
        if (!(*lhs == *rhs)) {
            continue;
        }

        m_retStatement->removeModified(lhs);
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "After finding preserveds");
}


void UserProc::removeSpAssignsIfPossible()
{
    // if there are no uses of sp other than sp{-} in the whole procedure,
    // we can safely remove all assignments to sp, this will make the output
    // more readable for human eyes.

    auto sp(Location::regOf(m_signature->getStackRegister(m_prog)));
    bool foundone = false;

    StatementList stmts;

    getStatements(stmts);

    for (auto stmt : stmts) {
        if (stmt->isAssign() && (*static_cast<Assign *>(stmt)->getLeft() == *sp)) {
            foundone = true;
        }

        LocationSet refs;
        stmt->addUsedLocs(refs);

        for (const SharedExp& rr : refs) {
            if (rr->isSubscript() && (*rr->getSubExp1() == *sp)) {
                Statement *def = rr->access<RefExp>()->getDef();

                if (def && (def->getProc() == this)) {
                    return;
                }
            }
        }
    }

    if (!foundone) {
        return;
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "Before removing stack pointer assigns.");

    for (auto& stmt : stmts) {
        if (stmt->isAssign()) {
            Assign *a = static_cast<Assign *>(stmt);

            if (*a->getLeft() == *sp) {
                removeStatement(a);
            }
        }
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "After removing stack pointer assigns.");
}


void UserProc::removeMatchingAssignsIfPossible(SharedExp e)
{
    // if there are no uses of %flags in the whole procedure,
    // we can safely remove all assignments to %flags, this will make the output
    // more readable for human eyes and makes short circuit analysis easier.

    bool foundone = false;

    StatementList stmts;

    getStatements(stmts);

    for (auto stmt : stmts) {
        if (stmt->isAssign() && (*static_cast<const Assign *>(stmt)->getLeft() == *e)) {
            foundone = true;
        }

        if (stmt->isPhi()) {
            if (*static_cast<const PhiAssign *>(stmt)->getLeft() == *e) {
                foundone = true;
            }

            continue;
        }

        LocationSet refs;
        stmt->addUsedLocs(refs);

        for (const SharedExp& rr : refs) {
            if (rr->isSubscript() && (*rr->getSubExp1() == *e)) {
                Statement *def = rr->access<RefExp>()->getDef();

                if (def && (def->getProc() == this)) {
                    return;
                }
            }
        }
    }

    if (!foundone) {
        return;
    }

    QString     res_str;
    QTextStream str(&res_str);
    str << "Before removing matching assigns (" << e << ").";
    Boomerang::get()->alertDecompileDebugPoint(this, qPrintable(res_str));
    LOG_VERBOSE(res_str);

    for (auto& stmt : stmts) {
        if ((stmt)->isAssign()) {
            Assign *a = static_cast<Assign *>(stmt);

            if (*a->getLeft() == *e) {
                removeStatement(a);
            }
        }
        else if ((stmt)->isPhi()) {
            PhiAssign *a = static_cast<PhiAssign *>(stmt);

            if (*a->getLeft() == *e) {
                removeStatement(a);
            }
        }
    }

    res_str.clear();
    str << "After removing matching assigns (" << e << ").";
    Boomerang::get()->alertDecompileDebugPoint(this, qPrintable(res_str));
    LOG_VERBOSE(res_str);
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


// m[WILD]{-}
static SharedExp memOfWild = RefExp::get(Location::memOf(Terminal::get(opWild)), nullptr);
// r[WILD INT]{-}
static SharedExp regOfWild = RefExp::get(Location::regOf(Terminal::get(opWildIntConst)), nullptr);


void UserProc::findFinalParameters()
{
    Boomerang::get()->alertDecompileDebugPoint(this, "before find final parameters.");

    qDeleteAll(m_parameters);
    m_parameters.clear();

    if (m_signature->isForced()) {
        // Copy from signature
        int               n = m_signature->getNumParams();
        ImplicitConverter ic(m_cfg);

        for (int i = 0; i < n; ++i) {
            SharedExp             paramLoc = m_signature->getParamExp(i)->clone(); // E.g. m[r28 + 4]
            LocationSet           components;
            paramLoc->addUsedLocs(components);

            for (auto cc = components.begin(); cc != components.end(); ++cc) {
                if (*cc != paramLoc) {                       // Don't subscript outer level
                    paramLoc->expSubscriptVar(*cc, nullptr); // E.g. r28 -> r28{-}
                    paramLoc->accept(&ic);                   // E.g. r28{-} -> r28{0}
                }
            }

            m_parameters.append(new ImplicitAssign(m_signature->getParamType(i), paramLoc));
            QString   name       = m_signature->getParamName(i);
            SharedExp param      = Location::param(name, this);
            SharedExp reParamLoc = RefExp::get(paramLoc, m_cfg->findImplicitAssign(paramLoc));
            mapSymbolTo(reParamLoc, param); // Update name map
        }

        return;
    }

    if (DEBUG_PARAMS) {
        LOG_VERBOSE("Finding final parameters for %1", getName());
    }

    //    int sp = signature->getStackRegister();
    m_signature->setNumParams(0); // Clear any old ideas
    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        // Assume that all parameters will be m[]{0} or r[]{0}, and in the implicit definitions at the start of the
        // program
        if (!s->isImplicit()) {
            // Note: phis can get converted to assignments, but I hope that this is only later on: check this!
            break; // Stop after reading all implicit assignments
        }

        SharedExp e = static_cast<ImplicitAssign *>(s)->getLeft();

        if (m_signature->findParam(e) == -1) {
            if (DEBUG_PARAMS) {
                LOG_VERBOSE("Potential param %1", e);
            }

            // I believe that the only true parameters will be registers or memofs that look like locals (stack
            // pararameters)
            if (!(e->isRegOf() || isLocalOrParamPattern(e))) {
                continue;
            }

            if (DEBUG_PARAMS) {
                LOG_VERBOSE("Found new parameter %1", e);
            }

            SharedType ty = static_cast<ImplicitAssign *>(s)->getType();
            // Add this parameter to the signature (for now; creates parameter names)
            addParameter(e, ty);
            // Insert it into the parameters StatementList, in sensible order
            insertParameter(e, ty);
        }
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "after find final parameters.");
}


void UserProc::addParameter(SharedExp e, SharedType ty)
{
    // In case it's already an implicit argument:
    removeParameter(e);

    m_signature->addParameter(e, ty);
}


void UserProc::addParameterSymbols()
{
    ImplicitConverter       ic(m_cfg);
    int i = 0;

    for (auto it = m_parameters.begin(); it != m_parameters.end(); ++it, ++i) {
        SharedExp lhs = static_cast<Assignment *>(*it)->getLeft();
        lhs = lhs->expSubscriptAllNull();
        lhs = lhs->accept(&ic);
        SharedExp to = Location::param(m_signature->getParamName(i), this);
        mapSymbolTo(lhs, to);
    }
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

            QString nam = (elem).second->access<Const, 1>()->getStr();

            if (m_locals.find(nam) == m_locals.end()) {
                continue;
            }

            SharedType     lty = m_locals[nam];
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


bool UserProc::removeNullStatements()
{
    bool          change = false;
    StatementList stmts;
    getStatements(stmts);
    // remove null code
    for (Statement *s : stmts) {
        if (s->isNullStatement()) {
            // A statement of the form x := x
            LOG_VERBOSE("Removing null statement: %1 %2", s->getNumber(), s);

            removeStatement(s);
            change = true;
        }
    }

    return change;
}


bool UserProc::propagateStatements(bool& convert, int pass)
{
    LOG_VERBOSE("--- Begin propagating statements pass %1 ---", pass);
    StatementList stmts;
    getStatements(stmts);

    // Find the locations that are used by a live, dominating phi-function
    LocationSet usedByDomPhi;
    findLiveAtDomPhi(usedByDomPhi);

    // Next pass: count the number of times each assignment LHS would be propagated somewhere
    std::map<SharedExp, int, lessExpStar> destCounts;

    // Also maintain a set of locations which are used by phi statements
    for (Statement *s : stmts) {
        ExpDestCounter  edc(destCounts);
        StmtDestCounter sdc(&edc);
        s->accept(&sdc);
    }

#if USE_DOMINANCE_NUMS
    // A third pass for dominance numbers
    setDominanceNumbers();
#endif
    // A fourth pass to propagate only the flags (these must be propagated even if it results in extra locals)
    bool change = false;

    for (Statement *s : stmts) {
        if (!s->isPhi()) {
            change |= s->propagateFlagsTo();
        }
    }

    // Finally the actual propagation
    convert = false;

    for (Statement *s : stmts) {
        if (!s->isPhi()) {
            change |= s->propagateTo(convert, &destCounts, &usedByDomPhi);
        }
    }

    simplify();
    propagateToCollector();
    LOG_VERBOSE("=== End propagating statements at pass %1 ===", pass);
    return change;
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

    LOG_VERBOSE("Assigning type %1 to new %2", ty->getCtype(), localName);

    return Location::local(localName, this);
}


void UserProc::addLocal(SharedType ty, const QString& nam, SharedExp e)
{
    // symbolMap is a multimap now; you might have r8->o0 for integers and r8->o0_1 for char*
    // assert(symbolMap.find(e) == symbolMap.end());
    mapSymbolTo(e, Location::local(nam, this));
    // assert(locals.find(nam) == locals.end());        // Could be r10{20} -> o2, r10{30}->o2 now
    m_locals[nam] = ty;
}


SharedType UserProc::getLocalType(const QString& nam)
{
    if (m_locals.find(nam) == m_locals.end()) {
        return nullptr;
    }

    SharedType ty = m_locals[nam];
    return ty;
}


void UserProc::setLocalType(const QString& nam, SharedType ty)
{
    m_locals[nam] = ty;

    LOG_VERBOSE("Updating type of %1 to %2", nam, ty->getCtype());
}


SharedType UserProc::getParamType(const QString& nam)
{
    for (unsigned int i = 0; i < m_signature->getNumParams(); i++) {
        if (nam == m_signature->getParamName(i)) {
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


SharedConstExp UserProc::expFromSymbol(const QString& nam) const
{
    for (const std::pair<SharedConstExp, SharedExp>& it : m_symbolMap) {
        auto e = it.second;

        if (e->isLocal() && (e->access<Const, 1>()->getStr() == nam)) {
            return it.first;
        }
    }

    return nullptr;
}


void UserProc::countRefs(RefCounter& refCounts)
{
    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        // Don't count uses in implicit statements. There is no RHS of course, but you can still have x from m[x] on the
        // LHS and so on, and these are not real uses
        if (s->isImplicit()) {
            continue;
        }

        if (DEBUG_UNUSED) {
            LOG_MSG("Counting references in %1", s);
        }

        LocationSet refs;
        s->addUsedLocs(refs, false); // Ignore uses in collectors

        for (const SharedExp& rr : refs) {
            if (rr->isSubscript()) {
                Statement *def = rr->access<RefExp>()->getDef();

                // Used to not count implicit refs here (def->getNumber() == 0), meaning that implicit definitions get
                // removed as dead code! But these are the ideal place to read off final parameters, and it is
                // guaranteed now that implicit statements are sorted out for us by now (for dfa type analysis)
                if (def /* && def->getNumber() */) {
                    refCounts[def]++;

                    if (DEBUG_UNUSED) {
                        LOG_MSG("counted ref to %1", rr);
                    }
                }
            }
        }
    }

    if (DEBUG_UNUSED) {
        LOG_MSG("### Reference counts for %1:", getName());

        for (RefCounter::iterator rr = refCounts.begin(); rr != refCounts.end(); ++rr) {
            LOG_MSG("  %1: %2", rr->first->getNumber(), rr->second);
        }

        LOG_MSG("### end reference counts");
    }
}


void UserProc::removeUnusedLocals()
{
    Boomerang::get()->alertDecompileDebugPoint(this, "Before removing unused locals");

    LOG_VERBOSE("Removing unused locals (final) for %1", getName());

    QSet<QString> usedLocals;
    StatementList stmts;
    getStatements(stmts);

    // First count any uses of the locals
    bool all = false;

    for (Statement *s : stmts) {
        LocationSet locs;
        all |= s->addUsedLocals(locs);

        for (SharedExp u : locs) {
            // Must be a real symbol, and not defined in this statement, unless it is a return statement (in which case
            // it is used outside this procedure), or a call statement. Consider local7 = local7+1 and
            // return local7 = local7+1 and local7 = call(local7+1), where in all cases, local7 is not used elsewhere
            // outside this procedure. With the assign, it can be deleted, but with the return or call statements, it
            // can't.
            if ((s->isReturn() || s->isCall() || !s->definesLoc(u))) {
                if (!u->isLocal()) {
                    continue;
                }

                QString name(u->access<Const, 1>()->getStr());
                usedLocals.insert(name);

                if (DEBUG_UNUSED) {
                    LOG_MSG("Counted local %1 in %2", name, s);
                }
            }
        }

        if (s->isAssignment() && !s->isImplicit() && static_cast<Assignment *>(s)->getLeft()->isLocal()) {
            Assignment *as = static_cast<Assignment *>(s);
            auto       c   = as->getLeft()->access<Const, 1>();
            QString    name(c->getStr());
            usedLocals.insert(name);

            if (DEBUG_UNUSED) {
                LOG_MSG("Counted local %1 on left of %2", name, s);
            }
        }
    }

    // Now record the unused ones in set removes

    QSet<QString> removes;

    for (auto it = m_locals.begin(); it != m_locals.end(); ++it) {
        const QString& name(it->first);

        if (all && removes.size()) {
            LOG_VERBOSE("WARNING: defineall seen in procedure %1, so not removing %2 locals",
                        name, removes.size());
        }

        if ((usedLocals.find(name) == usedLocals.end()) && !all) {
            if (SETTING(verboseOutput)) {
                LOG_VERBOSE("Removed unused local %1", name);
            }

            removes.insert(name);
        }
    }

    // Remove any definitions of the removed locals
    for (Statement *s : stmts) {
        LocationSet ls;
        s->getDefinitions(ls);

        for (auto ll = ls.begin(); ll != ls.end(); ++ll) {
            SharedType ty   = s->getTypeFor(*ll);
            QString    name = findLocal(*ll, ty);

            if (name.isNull()) {
                continue;
            }

            if (removes.find(name) != removes.end()) {
                // Remove it. If an assign, delete it; otherwise (call), remove the define
                if (s->isAssignment()) {
                    removeStatement(s);
                    break; // Break to next statement
                }
                else if (s->isCall()) {
                    // Remove just this define. May end up removing several defines from this call.
                    static_cast<CallStatement *>(s)->removeDefine(*ll);
                }

                // else if a ReturnStatement, don't attempt to remove it. The definition is used *outside* this proc.
            }
        }
    }

    // Finally, remove them from locals, so they don't get declared
    for (QString str : removes) {
        m_locals.erase(str);
    }

    // Also remove them from the symbols, since symbols are a superset of locals at present
    for (SymbolMap::iterator sm = m_symbolMap.begin(); sm != m_symbolMap.end();) {
        SharedExp mapsTo = sm->second;

        if (mapsTo->isLocal()) {
            QString tmpName = mapsTo->access<Const, 1>()->getStr();

            if (removes.find(tmpName) != removes.end()) {
                sm = m_symbolMap.erase(sm);
                continue;
            }
        }

        ++sm;
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "After removing unused locals");
}


void UserProc::fromSSAForm()
{
    Boomerang::get()->alertDecompiling(this);

    LOG_VERBOSE("Transforming %1 from SSA form");

    Boomerang::get()->alertDecompileDebugPoint(this, "Before transforming from SSA form");

    if (m_cfg->getNumBBs() >= 100) { // Only for the larger procs
        // Note: emit newline at end of this proc, so we can distinguish getting stuck in this proc with doing a lot of
        // little procs that don't get messages. Also, looks better with progress dots
        LOG_VERBOSE(" transforming out of SSA form: %1 with %2 BBs", getName(), m_cfg->getNumBBs());
    }

    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        // Map registers to initial local variables
        s->mapRegistersToLocals();
        // Insert casts where needed, as types are about to become inaccessible
        s->insertCasts();
    }

    // First split the live ranges where needed by reason of type incompatibility, i.e. when the type of a subscripted
    // variable is different to its previous type. Start at the top, because we don't want to rename parameters (e.g.
    // argc)
    typedef std::pair<SharedType, SharedExp>                 FirstTypeEnt;
    typedef std::map<SharedExp, FirstTypeEnt, lessExpStar>   FirstTypesMap;

    FirstTypesMap           firstTypes;
    FirstTypesMap::iterator ff;
    ConnectionGraph         ig; // The interference graph; these can't have the same local variable
    ConnectionGraph         pu; // The Phi Unites: these need the same local variable or copies

    for (Statement *s : stmts) {
        LocationSet defs;
        s->getDefinitions(defs);

        for (SharedExp base : defs) {
            SharedType ty   = s->getTypeFor(base);

            if (ty == nullptr) { // Can happen e.g. when getting the type for %flags
                ty = VoidType::get();
            }

            LOG_VERBOSE("Got type %1 for %2 from %3", ty->prints(), base, s);
            ff = firstTypes.find(base);
            SharedExp ref = RefExp::get(base, s);

            if (ff == firstTypes.end()) {
                // There is no first type yet. Record it.
                FirstTypeEnt fte;
                fte.first        = ty;
                fte.second       = ref;
                firstTypes[base] = fte;
            }
            else if (ff->second.first && !ty->isCompatibleWith(*ff->second.first)) {
                if (SETTING(debugLiveness)) {
                    LOG_MSG("Def of %1 at %2 type %3 is not compatible with first type %4.",
                            base, s->getNumber(), ty, ff->second.first);
                }

                // There already is a type for base, and it is different to the type for this definition.
                // Record an "interference" so it will get a new variable
                if (!ty->isVoid()) { // just ignore void interferences ??!!
                    ig.connect(ref, ff->second.second);
                }
            }
        }
    }
    assert(ig.allRefsHaveDefs());

    // Find the interferences generated by more than one version of a variable being live at the same program point
    InterferenceFinder(m_cfg).findInterferences(ig);
    assert(ig.allRefsHaveDefs());

    // Find the set of locations that are "united" by phi-functions
    // FIXME: are these going to be trivially predictable?
    findPhiUnites(pu);

    if (SETTING(debugLiveness)) {
        LOG_MSG("## ig interference graph:");

        for (ConnectionGraph::iterator ii = ig.begin(); ii != ig.end(); ++ii) {
            LOG_MSG("   ig %1 -> %2", ii->first, ii->second);
        }

        LOG_MSG("## pu phi unites graph:");

        for (ConnectionGraph::iterator ii = pu.begin(); ii != pu.end(); ++ii) {
            LOG_MSG("   pu %1 -> %2", ii->first, ii->second);
        }
    }

    // Choose one of each interfering location to give a new name to
    assert(ig.allRefsHaveDefs());

    for (ConnectionGraph::iterator ii = ig.begin(); ii != ig.end(); ++ii) {
        auto    r1    = ii->first->access<RefExp>();
        auto    r2    = ii->second->access<RefExp>(); // r1 -> r2 and vice versa
        QString name1 = lookupSymFromRefAny(r1);
        QString name2 = lookupSymFromRefAny(r2);

        if (!name1.isNull() && !name2.isNull() && (name1 != name2)) {
            continue; // Already different names, probably because of the redundant mapping
        }

        std::shared_ptr<RefExp> rename;

        if (r1->isImplicitDef()) {
            // If r1 is an implicit definition, don't rename it (it is probably a parameter, and should retain its
            // current name)
            rename = r2;
        }
        else if (r2->isImplicitDef()) {
            rename = r1; // Ditto for r2
        }

        if (rename == nullptr) {
            Statement *def2 = r2->getDef();

            if (def2->isPhi()) { // Prefer the destinations of phis
                rename = r2;
            }
            else {
                rename = r1;
            }
        }

        SharedType ty    = rename->getDef()->getTypeFor(rename->getSubExp1());
        SharedExp  local = createLocal(ty, rename);

        if (SETTING(debugLiveness)) {
            LOG_MSG("Renaming %1 to %2", rename, local);
        }

        mapSymbolTo(rename, local);
    }

    // Implement part of the Phi Unites list, where renamings or parameters have broken them, by renaming
    // The rest of them will be done as phis are removed
    // The idea is that where l1 and l2 have to unite, and exactly one of them already has a local/name, you can
    // implement the unification by giving the unnamed one the same name as the named one, as long as they don't
    // interfere
    for (ConnectionGraph::iterator ii = pu.begin(); ii != pu.end(); ++ii) {
        auto    r1    = ii->first->access<RefExp>();
        auto    r2    = ii->second->access<RefExp>();
        QString name1 = lookupSymFromRef(r1);
        QString name2 = lookupSymFromRef(r2);

        if (!name1.isNull() && !name2.isNull() && !ig.isConnected(r1, *r2)) {
            // There is a case where this is unhelpful, and it happen in test/pentium/fromssa2. We have renamed the
            // destination of the phi to ebx_1, and that leaves the two phi operands as ebx. However, we attempt to
            // unite them here, which will cause one of the operands to become ebx_1, so the neat oprimisation of
            // replacing the phi with one copy doesn't work. The result is an extra copy.
            // So check of r1 is a phi and r2 one of its operands, and all other operands for the phi have the same
            // name. If so, don't rename.
            Statement *def1 = r1->getDef();

            if (def1->isPhi()) {
                bool                allSame     = true;
                bool                r2IsOperand = false;
                QString             firstName   = QString::null;
                PhiAssign           *pa = static_cast<PhiAssign *>(def1);

                for (RefExp &refExp : *pa) {
                    auto re(RefExp::get(refExp.getSubExp1(), refExp.getDef()));

                    if (*re == *r2) {
                        r2IsOperand = true;
                    }

                    if (firstName.isNull()) {
                        firstName = lookupSymFromRefAny(re);
                    }
                    else {
                        QString tmp = lookupSymFromRefAny(re);

                        if (tmp.isNull() || (firstName != tmp)) {
                            allSame = false;
                            break;
                        }
                    }
                }

                if (allSame && r2IsOperand) {
                    continue; // This situation has happened, don't map now
                }
            }

            mapSymbolTo(r2, Location::local(name1, this));
            continue;
        }
    }

    /*    *    *    *    *    *    *    *    *    *    *    *    *    *    *\
    *                                                        *
    *     IR gets changed with hard locals and params here    *
    *                                                        *
    \*    *    *    *    *    *    *    *    *    *    *    *    *    *    */

    // First rename the variables (including phi's, but don't remove).
    // NOTE: it is not possible to postpone renaming these locals till the back end, since the same base location
    // may require different names at different locations, e.g. r28{0} is local0, r28{16} is local1
    // Update symbols and parameters, particularly for the stack pointer inside memofs.
    // NOTE: the ordering of the below operations is critical! Re-ordering may well prevent e.g. parameters from
    // renaming successfully.
    verifyPHIs();
    nameParameterPhis();
    mapLocalsAndParams();
    mapParameters();
    removeSubscriptsFromSymbols();
    removeSubscriptsFromParameters();

    for (Statement *s : stmts) {
        s->replaceSubscriptsWithLocals();
    }

    // Now remove the phis
    for (Statement *s : stmts) {
        if (!s->isPhi()) {
            continue;
        }

        // Check if the base variables are all the same
        PhiAssign *phi = static_cast<PhiAssign *>(s);

        if (phi->begin() == phi->end()) {
            // no params to this phi, just remove it
            LOG_VERBOSE("Phi with no params, removing: %1", s);

            removeStatement(s);
            continue;
        }

        LocationSet refs;
        phi->addUsedLocs(refs);
        bool      phiParamsSame = true;
        SharedExp first         = nullptr;

        if (phi->getNumDefs() > 1) {
            for (RefExp& pi : *phi) {
                if (pi.getSubExp1() == nullptr) {
                    continue;
                }

                if (first == nullptr) {
                    first = pi.getSubExp1();
                    continue;
                }

                if (!(*(pi.getSubExp1()) == *first)) {
                    phiParamsSame = false;
                    break;
                }
            }
        }

        if (phiParamsSame && first) {
            // Is the left of the phi assignment the same base variable as all the operands?
            if (*phi->getLeft() == *first) {
                if (SETTING(debugLiveness) || DEBUG_UNUSED) {
                    LOG_MSG("Removing phi: left and all refs same or 0: %1", s);
                }

                // Just removing the refs will work, or removing the whole phi
                // NOTE: Removing the phi here may cause other statments to be not used.
                removeStatement(s);
            }
            else {
                // Need to replace the phi by an expression,
                // e.g. local0 = phi(r24{3}, r24{5}) becomes
                //        local0 = r24
                phi->convertToAssign(first->clone());
            }
        }
        else {
            // Need new local(s) for phi operands that have different names from the lhs

            // This way is costly in copies, but has no problems with extending live ranges
            // Exp* tempLoc = newLocal(pa->getType());
            SharedExp tempLoc = getSymbolExp(RefExp::get(phi->getLeft(), phi), phi->getType());

            if (SETTING(debugLiveness)) {
                LOG_MSG("Phi statement %1 requires local, using %2", s, tempLoc);
            }

            // For each definition ref'd in the phi
            for (RefExp &pi : *phi) {
                if (pi.getSubExp1() == nullptr) {
                    continue;
                }

                insertAssignAfter(pi.getDef(), tempLoc, pi.getSubExp1());
            }

            // Replace the RHS of the phi with tempLoc
            phi->convertToAssign(tempLoc);
        }
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "after transforming from SSA form");
}


void UserProc::mapParameters()
{
    // Replace the parameters with their mappings
    StatementList::iterator pp;

    for (pp = m_parameters.begin(); pp != m_parameters.end(); ++pp) {
        SharedExp lhs        = static_cast<Assignment *>(*pp)->getLeft();
        QString   mappedName = lookupParam(lhs);

        if (mappedName.isNull()) {
            LOG_WARN("No symbol mapping for parameter %1", lhs);
            bool      allZero;
            SharedExp clean = lhs->clone()->removeSubscripts(allZero);

            if (allZero) {
                static_cast<Assignment *>(*pp)->setLeft(clean);
            }

            // Else leave them alone
        }
        else {
            static_cast<Assignment *>(*pp)->setLeft(Location::param(mappedName, this));
        }
    }
}


void UserProc::removeSubscriptsFromSymbols()
{
    // Basically, use the symbol map to map the symbols in the symbol map!
    // However, do not remove subscripts from the outer level; they are still needed for comments in the output and also
    // for when removing subscripts from parameters (still need the {0})
    // Since this will potentially change the ordering of entries, need to copy the map
    SymbolMap sm2 = m_symbolMap; // Object copy

    SymbolMap::iterator it;
    m_symbolMap.clear();
    ExpSsaXformer esx(this);

    for (it = sm2.begin(); it != sm2.end(); ++it) {
        SharedExp from = std::const_pointer_cast<Exp>(it->first);

        if (from->isSubscript()) {
            // As noted above, don't touch the outer level of subscripts
            SharedExp& sub = from->refSubExp1();
            sub = sub->accept(&esx);
        }
        else {
            from = from->accept(&esx);
        }

        mapSymbolTo(from, it->second);
    }
}


void UserProc::removeSubscriptsFromParameters()
{
    ExpSsaXformer esx(this);

    for (Statement *param : m_parameters) {
        SharedExp left = static_cast<Assignment *>(param)->getLeft();
        left = left->accept(&esx);
        static_cast<Assignment *>(param)->setLeft(left);
    }
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
        fixCallAndPhiRefs();
    }

    return ch;
}


void UserProc::addImplicitAssigns()
{
    Boomerang::get()->alertDecompileDebugPoint(this, "before adding implicit assigns");

    StatementList stmts;
    getStatements(stmts);
    ImplicitConverter     ic(m_cfg);
    StmtImplicitConverter sm(&ic, m_cfg);

    for (Statement *stmt : stmts) {
        stmt->accept(&sm);
    }

    m_cfg->setImplicitsDone();
    m_df.convertImplicits(); // Some maps have m[...]{-} need to be m[...]{0} now
    makeSymbolsImplicit();

    Boomerang::get()->alertDecompileDebugPoint(this, "after adding implicit assigns");
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


void UserProc::updateArguments()
{
    Boomerang::get()->alertDecompiling(this);
    LOG_VERBOSE("### Update arguments for %1 ###", getName());
    Boomerang::get()->alertDecompileDebugPoint(this, "Before updating arguments");
    BasicBlock::RTLRIterator        rrit;
    StatementList::reverse_iterator srit;

    for (BasicBlock *it : *m_cfg) {
        CallStatement *c = dynamic_cast<CallStatement *>(it->getLastStmt(rrit, srit));

        // Note: we may have removed some statements, so there may no longer be a last statement!
        if (c == nullptr) {
            continue;
        }

        c->updateArguments();
        // c->bypass();
        LOG_VERBOSE("%1", c);
    }

    LOG_VERBOSE("=== End update arguments for %1", getName());
    Boomerang::get()->alertDecompileDebugPoint(this, "After updating arguments");
}


void UserProc::updateCallDefines()
{
    LOG_VERBOSE("### Update call defines for %1 ###", getName());

    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        CallStatement *call = dynamic_cast<CallStatement *>(s);
        if (call) {
            call->updateDefines();
        }
    }
}


void UserProc::replaceSimpleGlobalConstants()
{
    LOG_VERBOSE("### Replace simple global constants for %1 ###", getName());

    StatementList stmts;
    getStatements(stmts);

    for (Statement *st : stmts) {
        Assign *assgn = dynamic_cast<Assign *>(st);

        if (assgn == nullptr) {
            continue;
        }

        if (!assgn->getRight()->isMemOf()) {
            continue;
        }

        if (!assgn->getRight()->getSubExp1()->isIntConst()) {
            continue;
        }

        Address addr = assgn->getRight()->access<Const, 1>()->getAddr();
        LOG_VERBOSE("Assign %1");

        if (m_prog->isReadOnly(addr)) {
            LOG_VERBOSE("is readonly");
            int val = 0;

            switch (assgn->getType()->getSize())
            {
            case 8:
                val = m_prog->readNative1(addr);
                break;

            case 16:
                val = m_prog->readNative2(addr);
                break;

            case 32:
                val = m_prog->readNative4(addr);
                break;

            default:
                assert(false);
            }

            assgn->setRight(Const::get(val));
        }
    }
}


void UserProc::reverseStrengthReduction()
{
    Boomerang::get()->alertDecompileDebugPoint(this, "Before reversing strength reduction");

    StatementList stmts;
    getStatements(stmts);

    for (Statement *s : stmts) {
        if (!s->isAssign()) {
            continue;
        }

        Assign *as = static_cast<Assign *>(s);

        // of the form x = x{p} + c
        if ((as->getRight()->getOper() == opPlus) && as->getRight()->getSubExp1()->isSubscript() &&
            (*as->getLeft() == *as->getRight()->getSubExp1()->getSubExp1()) &&
            as->getRight()->getSubExp2()->isIntConst()) {
            int  c = as->getRight()->access<Const, 2>()->getInt();
            auto r = as->getRight()->access<RefExp, 1>();

            if (r->getDef() && r->getDef()->isPhi()) {
                PhiAssign *p = static_cast<PhiAssign *>(r->getDef());

                if (p->getNumDefs() == 2) {
                    Statement *first  = p->begin()->getDef();
                    Statement *second = p->rbegin()->getDef();

                    if (first == as) {
                        // want the increment in second
                        std::swap(first, second);
                    }

                    // first must be of form x := 0
                    if (first && first->isAssign() && static_cast<Assign *>(first)->getRight()->isIntConst() &&
                        static_cast<Assign *>(first)->getRight()->access<Const>()->getInt() == 0) {
                        // ok, fun, now we need to find every reference to p and
                        // replace with x{p} * c
                        StatementList stmts2;
                        getStatements(stmts2);

                        for (auto it2 = stmts2.begin(); it2 != stmts2.end(); ++it2) {
                            if (*it2 != as) {
                                (*it2)->searchAndReplace(*r, Binary::get(opMult, r->clone(), Const::get(c)));
                            }
                        }

                        // that done we can replace c with 1 in as
                        as->getRight()->access<Const, 2>()->setInt(1);
                    }
                }
            }
        }
    }

    Boomerang::get()->alertDecompileDebugPoint(this, "After reversing strength reduction");
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


// Perform call and phi statement bypassing at depth d <- missing
void UserProc::fixCallAndPhiRefs()
{
    /* Algorithm:
     *      for each statement s in this proc
     *        if s is a phi statement ps
     *              let r be a ref made up of lhs and s
     *              for each parameter p of ps
     *                if p == r                        // e.g. test/pentium/fromssa2 r28{56}
     *                      remove p from ps
     *              let lhs be left hand side of ps
     *              allSame = true
     *              let first be a ref built from first p
     *              do bypass but not propagation on first
     *              if result is of the form lhs{x}
     *                replace first with x
     *              for each parameter p of ps after the first
     *                let current be a ref built from p
     *                do bypass but not propagation on current
     *                if result is of form lhs{x}
     *                      replace cur with x
     *                if first != current
     *                      allSame = false
     *              if allSame
     *                let best be ref built from the "best" parameter p in ps ({-} better than {assign} better than {call})
     *                replace ps with an assignment lhs := best
     *      else (ordinary statement)
     *        do bypass and propagation for s
     */
    LOG_VERBOSE("### Start fix call and phi bypass analysis for %1 ###", getName());

    Boomerang::get()->alertDecompileDebugPoint(this, "Before fixing call and phi refs");

    std::map<SharedExp, int, lessExpStar> destCounts;
    StatementList stmts;
    getStatements(stmts);

    // a[m[]] hack, aint nothing better.
    bool found = true;

    for (Statement *s : stmts) {
        if (!s->isCall()) {
            continue;
        }

        CallStatement *call = static_cast<CallStatement *>(s);

        for (auto& elem : call->getArguments()) {
            Assign *a = static_cast<Assign *>(elem);

            if (!a->getType()->resolvesToPointer()) {
                continue;
            }

            SharedExp e = a->getRight();

            if ((e->getOper() == opPlus) || (e->getOper() == opMinus)) {
                if (e->getSubExp2()->isIntConst()) {
                    if (e->getSubExp1()->isSubscript() &&
                        e->getSubExp1()->getSubExp1()->isRegN(m_signature->getStackRegister()) &&
                        (((e->access<RefExp, 1>())->getDef() == nullptr) ||
                         (e->access<RefExp, 1>())->getDef()->isImplicit())) {
                        a->setRight(Unary::get(opAddrOf, Location::memOf(e->clone())));
                        found = true;
                    }
                }
            }
        }
    }

    if (found) {
        doRenameBlockVars(2);
    }

    // Scan for situations like this:
    // 56 r28 := phi{6, 26}
    // ...
    // 26 r28 := r28{56}
    // So we can remove the second parameter,
    // then reduce the phi to an assignment, then propagate it
    for (Statement *s : stmts) {
        if (!s->isPhi()) {
            continue;
        }

        PhiAssign *phi = static_cast<PhiAssign *>(s);
        std::shared_ptr<RefExp> refExp = RefExp::get(phi->getLeft(), phi);

        phi->removeAllReferences(refExp);
    }

    // Second pass
    for (Statement *s : stmts) {
        if (!s->isPhi()) { // Ordinary statement
            s->bypass();
            continue;
        }

        PhiAssign *phi = static_cast<PhiAssign *>(s);

        if (phi->getNumDefs() == 0) {
            continue; // Can happen e.g. for m[...] := phi {} when this proc is
        }

        // involved in a recursion group
        auto lhs     = phi->getLeft();
        bool allSame = true;
        // Let first be a reference built from the first parameter
        PhiAssign::iterator phi_iter = phi->begin();

        while (phi_iter != phi->end() && phi_iter->getSubExp1() == nullptr) {
            ++phi_iter;                // Skip any null parameters
        }

        assert(phi_iter != phi->end()); // Should have been deleted
        RefExp&  phi_inf = *phi_iter;
        SharedExp first = RefExp::get(phi_inf.getSubExp1(), phi_inf.getDef());

        // bypass to first
        CallBypasser cb(phi);
        first = first->accept(&cb);

        if (cb.isTopChanged()) {
            first = first->simplify();
        }

        first = first->propagateAll(); // Propagate everything repeatedly

        if (cb.isMod()) {              // Modified?
            // if first is of the form lhs{x}
            if (first->isSubscript() && (*first->getSubExp1() == *lhs)) {
                // replace first with x
                phi_inf.setDef(first->access<RefExp>()->getDef());
            }
        }

        // For each parameter p of ps after the first
        for (++phi_iter; phi_iter != phi->end(); ++phi_iter) {
            assert(phi_iter->getSubExp1());
            RefExp&      phi_inf2 = *phi_iter;
            SharedExp    current = RefExp::get(phi_inf2.getSubExp1(), phi_inf2.getDef());
            CallBypasser cb2(phi);
            current = current->accept(&cb2);

            if (cb2.isTopChanged()) {
                current = current->simplify();
            }

            current = current->propagateAll();

            if (cb2.isMod()) { // Modified?
                // if current is of the form lhs{x}
                if (current->isSubscript() && (*current->getSubExp1() == *lhs)) {
                    // replace current with x
                    phi_inf2.setDef(current->access<RefExp>()->getDef());
                }
            }

            if (!(*first == *current)) {
                allSame = false;
            }
        }

        if (allSame) {
            // let best be ref built from the "best" parameter p in ps ({-} better than {assign} better than {call})
            phi_iter = phi->begin();

            while (phi_iter != phi->end() && phi_iter->getSubExp1() == nullptr) {
                ++phi_iter;                // Skip any null parameters
            }

            assert(phi_iter != phi->end()); // Should have been deleted
            auto best = RefExp::get(phi_iter->getSubExp1(), phi_iter->getDef());

            for (++phi_iter; phi_iter != phi->end(); ++phi_iter) {
                assert(phi_iter->getSubExp1());
                auto current = RefExp::get(phi_iter->getSubExp1(), phi_iter->getDef());

                if (current->isImplicitDef()) {
                    best = current;
                    break;
                }

                if (phi_iter->getDef()->isAssign()) {
                    best = current;
                }

                // If phi_iter->second.def is a call, this is the worst case; keep only (via first)
                // if all parameters are calls
            }

            phi->convertToAssign(best);
            LOG_VERBOSE("Redundant phi replaced with copy assign; now %1", phi);
        }
    }

    // Also do xxx in m[xxx] in the use collector
    for (const SharedExp& cc : m_procUseCollector) {
        if (!cc->isMemOf()) {
            continue;
        }

        auto         addr = cc->getSubExp1();
        CallBypasser cb(nullptr);
        addr = addr->accept(&cb);

        if (cb.isMod()) {
            cc->setSubExp1(addr);
        }
    }

    LOG_VERBOSE("### End fix call and phi bypass analysis for %1 ###", getName());

    Boomerang::get()->alertDecompileDebugPoint(this, "after fixing call and phi refs");
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


void UserProc::initialParameters()
{
    LOG_VERBOSE("### Initial parameters for %1", getName());
    qDeleteAll(m_parameters);
    m_parameters.clear();

    for (const SharedExp& v : m_procUseCollector) {
        m_parameters.append(new ImplicitAssign(v->clone()));
    }

    if (SETTING(verboseOutput)) {
        QString     tgt;
        QTextStream ost(&tgt);
        printParams(ost);
        LOG_MSG(tgt);
    }
}


bool UserProc::inductivePreservation(UserProc * /*topOfCycle*/)
{
    // FIXME: This is not correct in general!! It should work OK for self recursion,
    // but not for general mutual recursion. Not that hard, just not done yet.
    return true;
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
    BasicBlock::RTLRIterator              rrit;
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
            // This is a recursive call to p. Check for an argument of the form param{-} FIXME: should be looking for
            // component
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

    m_parameters = newParameters;

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
    ReturnStatement::iterator rr;

    for (rr = m_retStatement->begin(); rr != m_retStatement->end();) {
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
            call->updateArguments();              // Update caller's arguments
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
        Assign *a = static_cast<Assign *>(m_retStatement->getReturns().front());
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
    removeCallLiveness(); // Want to recompute the call livenesses
    doRenameBlockVars(-3, true);

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


void UserProc::typeAnalysis()
{
    LOG_VERBOSE("### Type analysis for %1 ###", getName());

    // Now we need to add the implicit assignments. Doing this earlier is extremely problematic, because
    // of all the m[...] that change their sorting order as their arguments get subscripted or propagated into
    // Do this regardless of whether doing dfa-based TA, so things like finding parameters can rely on implicit assigns
    addImplicitAssigns();

    ITypeRecovery *rec = Boomerang::get()->getOrCreateProject()->getTypeRecoveryEngine();
    // Data flow based type analysis
    // Want to be after all propagation, but before converting expressions to locals etc
    if (DFA_TYPE_ANALYSIS) {
        rec->recoverFunctionTypes(this);
    }
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


void UserProc::eliminateDuplicateArgs()
{
    LOG_VERBOSE("### Eliminate duplicate args for %1 ###", getName());

    BasicBlock::RTLRIterator              rrit;
    StatementList::reverse_iterator srit;

    for (BasicBlock *bb : *m_cfg) {
        CallStatement *c = dynamic_cast<CallStatement *>(bb->getLastStmt(rrit, srit));

        // Note: we may have removed some statements, so there may no longer be a last statement!
        if (c == nullptr) {
            continue;
        }

        c->eliminateDuplicateArgs();
    }
}


void UserProc::removeCallLiveness()
{
    LOG_VERBOSE("### Removing call livenesses for %1 ###", getName());

    BasicBlock::RTLRIterator              rrit;
    StatementList::reverse_iterator srit;

    for (BasicBlock *bb : *m_cfg) {
        CallStatement *c = dynamic_cast<CallStatement *>(bb->getLastStmt(rrit, srit));

        // Note: we may have removed some statements, so there may no longer be a last statement!
        if (c == nullptr) {
            continue;
        }

        c->removeAllLive();
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


void UserProc::makeSymbolsImplicit()
{
    SymbolMap sm2 = m_symbolMap; // Copy the whole map; necessary because the keys (Exps) change
    m_symbolMap.clear();
    ImplicitConverter ic(m_cfg);

    for (auto it = sm2.begin(); it != sm2.end(); ++it) {
        SharedExp impFrom = std::const_pointer_cast<Exp>(it->first)->accept(&ic);
        mapSymbolTo(impFrom, it->second);
    }
}


void UserProc::findLiveAtDomPhi(LocationSet& usedByDomPhi)
{
    LocationSet usedByDomPhi0;
    std::map<SharedExp, PhiAssign *, lessExpStar> defdByPhi;

    m_df.findLiveAtDomPhi(usedByDomPhi, usedByDomPhi0, defdByPhi);

    // Note that the above is not the complete algorithm; it has found the dead phi-functions in the defdAtPhi
    for (auto it = defdByPhi.begin(); it != defdByPhi.end(); ++it) {
        // For each phi parameter, remove from the final usedByDomPhi set
        for (RefExp& v : *it->second) {
            assert(v.getSubExp1());
            auto wrappedParam = RefExp::get(v.getSubExp1(), v.getDef());
            usedByDomPhi.remove(wrappedParam);
        }

        // Now remove the actual phi-function (a PhiAssign Statement)
        // Ick - some problem with return statements not using their returns until more analysis is done
        // removeStatement(it->second);
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


void UserProc::verifyPHIs()
{
    StatementList stmts;

    getStatements(stmts);

    for (Statement *st : stmts) {
        if (!st->isPhi()) {
            continue; // Might be able to optimise this a bit
        }

        PhiAssign *pi = static_cast<PhiAssign *>(st);

        for (const auto& pas : *pi) {
            Q_UNUSED(pas);
            assert(pas.getDef());
        }
    }
}


void UserProc::nameParameterPhis()
{
    StatementList stmts;

    getStatements(stmts);

    for (Statement *insn : stmts) {
        if (!insn->isPhi()) {
            continue; // Might be able to optimise this a bit
        }

        PhiAssign *pi = static_cast<PhiAssign *>(insn);
        // See if the destination has a symbol already
        SharedExp lhs    = pi->getLeft();
        auto      lhsRef = RefExp::get(lhs, pi);

        if (findFirstSymbol(lhsRef) != nullptr) {
            continue;                         // Already mapped to something
        }

        bool       multiple  = false;         // True if find more than one unique parameter
        QString    firstName = QString::null; // The name for the first parameter found
        SharedType ty        = pi->getType();

        for (RefExp& v : *pi) {
            if (v.getDef()->isImplicit()) {
                QString name = lookupSym(RefExp::get(v.getSubExp1(), v.getDef()), ty);

                if (!name.isNull()) {
                    if (!firstName.isNull() && (firstName != name)) {
                        multiple = true;
                        break;
                    }

                    firstName = name; // Remember this candidate
                }
            }
        }

        if (multiple || firstName.isNull()) {
            continue;
        }

        mapSymbolTo(lhsRef, Location::param(firstName, this));
    }
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
    current->initialiseDecompile();   // Sort the CFG, number statements, etc
    current->earlyDecompile();

    // The standard preservation analysis should automatically perform conditional preservation.
    current->middleDecompile(callStack);
    current->setStatus(PROC_PRESERVEDS);

    // Mark all the relevant calls as non childless (will harmlessly get done again later)
    // FIXME: why exactly do we do this?
    current->markAsNonChildless(m_recursionGroup);

    // Need to propagate into the initial arguments, since arguments are uses, and we are about to remove unused
    // statements.
    bool convert;

    current->mapLocalsAndParams();
    current->updateArguments();
    current->propagateStatements(convert, 0); // Need to propagate into arguments
}

