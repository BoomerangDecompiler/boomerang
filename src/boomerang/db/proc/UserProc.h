#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "Proc.h"

#include "boomerang/db/CFG.h"
#include "boomerang/db/DataFlow.h"
#include "boomerang/db/UseCollector.h"
#include "boomerang/db/statements/ReturnStatement.h"
#include "boomerang/db/exp/Binary.h"

#include <list>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <cassert>


class Prog;
class UserProc;
class Cfg;
class BasicBlock;
class Exp;
class TypedExp;
class Type;
class RTL;
class ICodeGenerator;
class Parameter;
class Argument;
class Signature;
class Module;
class QTextStream;
class Log;


enum ProcStatus
{
    PROC_UNDECODED,     ///< Has not even been decoded
    PROC_DECODED,       ///< Decoded, no attempt at decompiling
    PROC_VISITED,       ///< Has been visited on the way down in decompile()
    PROC_INCYCLE,       ///< Is involved in cycles, has not completed early decompilation as yet
    PROC_PRESERVEDS,    ///< Has had preservation analysis done
    PROC_EARLYDONE,     ///< Has completed everything except the global analyses
    PROC_FINAL,         ///< Has had final decompilation
    PROC_CODE_GENERATED ///< Has had code generated
};


typedef std::set<UserProc *>    ProcSet;
typedef std::list<UserProc *>   ProcList;


/**
 * UserProc class.
 */
class UserProc : public Function
{
public:
    /**
     * A map between machine dependent locations and their corresponding symbolic, machine independent
     * representations.  Example: m[r28{0} - 8] -> local5; this means that *after* transforming out of SSA
     * form, any locations not specifically mapped otherwise (e.g. m[r28{0} - 8]{55} -> local6) will get this
     * name.
     * It is a *multi*map because one location can have several default names differentiated by type.
     * E.g. r24 -> eax for int, r24 -> eax_1 for float
     */
    typedef std::multimap<SharedConstExp, SharedExp, lessExpStar>   SymbolMap;

public:
    /**
     * \param address Address of entry point of function
     * \param name    Name of function
     * \param mod     Module that contains this function
     */
    UserProc(Address address, const QString& name, Module *mod);
    UserProc(const UserProc&) = delete;
    UserProc(UserProc&&) = default;

    virtual ~UserProc() override;

    UserProc& operator=(const UserProc&) = delete;
    UserProc& operator=(UserProc&&) = default;

public:
    /// \copydoc Printable::toString
    QString toString() const;

    /// Returns a pointer to the CFG object.
    Cfg *getCFG() { return m_cfg; }
    const Cfg *getCFG() const { return m_cfg; }

    /// Returns a pointer to the DataFlow object.
    DataFlow *getDataFlow() { return &m_df; }
    const DataFlow *getDataFlow() const { return &m_df; }

    /// \copydoc Function::isNoReturn
    virtual bool isNoReturn() const override;

    /// Returns whether or not this procedure can be decoded (i.e. has it already been decoded).
    bool isDecoded() const { return m_status >= PROC_DECODED; }
    bool isDecompiled() const { return m_status >= PROC_FINAL; }
    bool isEarlyRecursive() const { return m_recursionGroup != nullptr && m_status <= PROC_INCYCLE; }
    bool doesRecurseTo(UserProc *proc) const { return m_recursionGroup && m_recursionGroup->find(proc) != m_recursionGroup->end(); }

    ProcStatus getStatus() const { return m_status; }
    void setStatus(ProcStatus s);

    /// Records that this procedure has been decoded.
    void setDecoded();

    /// Deletes the whole Cfg for this proc object.
    void deleteCFG();

    /**
     * Decompile this procedure, and all callees.
     */
    void decompile();

public:
    /**
     * Mark calls involved in the recursion cycle as non childless
     * (each child has had middleDecompile called on it now).
     * \todo Not sure that this is needed...
     */
    void markAsNonChildless(const std::shared_ptr<ProcSet>& cs);

public:
    /// Map expressions to locals and initial parameters
    void mapLocalsAndParams();


    /// Add the parameter to the signature
    void addParameter(SharedExp e, SharedType ty);

    /**
     * Insert into parameters list correctly sorted.
     * Update the parameters, in case the signature and hence ordering
     * and filtering has changed, or the locations in the collector have changed
     */
    void insertParameter(SharedExp e, SharedType ty);

public:
    /// Update statement numbers
    void numberStatements() const;

    bool canRename(SharedConstExp e) const { return m_df.canRename(e); }

    /// Is this m[sp{-} +/- K]?
    /// True if \p e could represent a stack local or stack param
    bool isLocalOrParamPattern(SharedConstExp e) const;

    /// True if a local exists with name \p name
    bool existsLocal(const QString& name) const;

    bool isAddressEscapedVar(SharedConstExp e) const { return m_addressEscapedVars.contains(e); }

    /// Find the procs the calls point to.
    /// To be called after decoding all procs.
    void assignProcsToCalls();

    /// perform final simplifications
    void finalSimplify();

    /// Remove unused statements.
    void remUnusedStmtEtc();

    const std::map<QString, SharedType>& getLocals() const { return m_locals; }
    std::map<QString, SharedType>& getLocals() { return m_locals; }

    /// Propagate into xxx of m[xxx] in the UseCollector (locations live at the entry of this proc)
    void propagateToCollector();


    void insertAssignAfter(Statement *s, SharedExp left, SharedExp right);

    /// Insert statement \a a after statement \a s.
    /// \note this procedure is designed for the front end, where enclosing BBs are not set up yet.
    /// So this is an inefficient linear search!
    void insertStatementAfter(Statement *afterThis, Statement *stmt);

public:
    /**
     * Remove any returns that are not used by any callers
     *
     * Remove unused returns for this procedure, based on the equation:
     * returns = modifieds isect union(live at c) for all c calling this procedure.
     * The intersection operation will only remove locations. Removing returns can have three effects for each component
     * y used by that return (e.g. if return r24 := r25{10} + r26{20} is removed, statements 10 and 20 will be affected
     * and y will take the values r25{10} and r26{20}):
     * 1) a statement s defining a return becomes unused if the only use of its definition was y
     * 2) a call statement c defining y will no longer have y live if the return was the only use of y. This could cause a
     *    change to the returns of c's destination, so removeRedundantReturns has to be called for c's destination proc (if
     * it
     *    turns out to be the only definition, and that proc was not already scheduled for return removing).
     * 3) if y is a parameter (i.e. y is of the form loc{0}), then the signature of this procedure changes, and all callers
     *    have to have their arguments trimmed, and a similar process has to be applied to all those caller's removed
     *    arguments as is applied here to the removed returns.
     *
     * The \a removeRetSet is the set of procedures to process with this logic; caller in Prog calls all elements in this
     * set (only add procs to this set, never remove)
     *
     * \returns true if any change
     */
    bool removeRedundantReturns(std::set<UserProc *>& removeRetSet);

private:
    /// Remove redundant parameters. Return true if remove any
    bool removeRedundantParameters();

    /**
     * Check for a gainful use of bparam{0} in this proc.
     * Return with true when the first such use is found.
     * Ignore uses in return statements of recursive functions,
     * and phi statements that define them.
     * Procs in \p visited are already visited.
     *
     * \returns true if location \p e is used gainfully in this procedure.
     */
    bool checkForGainfulUse(SharedExp e, ProcSet& Visited);

    /**
     * Update parameters and call livenesses to take into account the changes
     * caused by removing a return from this procedure, or a callee's parameter
     * (which affects this procedure's arguments, which are also uses).
     *
     * Need to save the old parameters and call livenesses, redo the dataflow and
     * removal of unused statements, recalculate the parameters and call livenesses,
     * and if either or both of these are changed, recurse to parents or those calls'
     * children respectively. (When call livenesses change like this, it means that
     * the recently removed return was the only use of that liveness, i.e. there was a
     * return chain.)
     * \sa removeRedundantReturns().
     */
    void updateForUseChange(std::set<UserProc *>& removeRetSet);

public:
    /// this function was non-reentrant, but now reentrancy is frequently used
    /// prove any arbitary property of this procedure. If conditional is true, do not save the result, as it may
    /// be conditional on premises stored in other procedures
    bool prove(const std::shared_ptr<Binary>& query, bool conditional = false);

    /// promote the signature if possible
    void promoteSignature();

    /// get all statements
    /// Get to a statement list, so they come out in a reasonable and consistent order
    /// get all the statements
    void getStatements(StatementList& stmts) const;
    void removeStatement(Statement *stmt);

    StatementList& getParameters() { return m_parameters; }
    const StatementList& getModifieds() { return m_retStatement->getModifieds(); }


    /// Return an expression that is equivalent to e in terms of symbols.
    /// Creates new symbols as needed.

    /**
     * Return an expression that is equivalent to e in terms of local variables.
     * Creates new locals as needed.
     */
    SharedExp getSymbolExp(SharedExp le, SharedType ty = nullptr, bool lastPass = false);

    /**
     * Return the next available local variable; make it the given type.
     * \note was returning TypedExp*.
     * If \p name is non null, use that name
     */
    SharedExp createLocal(SharedType ty, const SharedExp& e, char *name = nullptr);

    /**
     * Add a new local supplying all needed information.
     */
    void addLocal(SharedType ty, const QString& name, SharedExp e);

    /// return a local's type
    SharedConstType getLocalType(const QString& name) const;
    void setLocalType(const QString& name, SharedType ty);

    SharedConstType getParamType(const QString& name) const;
    SharedType getParamType(const QString& name);

    SymbolMap& getSymbolMap() { return m_symbolMap; }
    const SymbolMap& getSymbolMap() const { return m_symbolMap; }

    void clearSymbolMap() { m_symbolMap.clear(); }

    /// \returns a symbol's exp (note: the original exp, like r24, not local1)
    SharedConstExp expFromSymbol(const QString& name) const;

    void mapSymbolTo(const SharedConstExp& from, SharedExp to);

    /// As above but with replacement
    void mapSymbolToRepl(const SharedConstExp& from, SharedConstExp oldTo, SharedExp newTo);

    /// Remove this mapping
    void removeSymbolMapping(const SharedConstExp& from, const SharedConstExp& to);

    // FIXME: is this the same as lookupSym() now?
    /// Lookup the expression in the symbol map. Return nullptr or a C string with the symbol. Use the Type* ty to
    /// select from several names in the multimap; the name corresponding to the first compatible type is returned
    SharedExp getSymbolFor(const SharedConstExp& e, const SharedConstType& ty) const;

    QString lookupSym(const SharedConstExp& e, SharedConstType ty) const;

    /// Lookup a specific symbol for the given ref
    QString lookupSymFromRef(const std::shared_ptr<const RefExp>& ref) const;

    /// Lookup a specific symbol if any, else the general one if any
    QString lookupSymFromRefAny(const std::shared_ptr<const RefExp>& ref) const;

    /// e is a parameter location, e.g. r8 or m[r28{0}+8]. Lookup a symbol for it
    /// Find the implicit definition for \a e and lookup a symbol
    QString lookupParam(SharedConstExp e) const;

    /// Check if \a r is already mapped to a local, else add one
    void checkLocalFor(const std::shared_ptr<RefExp>& r);

    /// Find the type of the local or parameter \a e
    SharedType getTypeForLocation(const SharedExp& e);
    SharedConstType getTypeForLocation(const SharedConstExp& e) const;

    /// Determine whether e is a local, either as a true opLocal (e.g. generated by fromSSA), or if it is in the
    /// symbol map and the name is in the locals map. If it is a local, return its name, else nullptr
    QString findLocal(const SharedExp& e, SharedType ty);
    QString findLocalFromRef(const std::shared_ptr<RefExp>& r);
    QString findFirstSymbol(const SharedConstExp& e) const;

    void renameLocal(const QString& oldName, const QString& newName);
    virtual void renameParameter(const QString& oldName, const QString& newName) override;

    /// WARN: write tests for getRegName in all combinations of r[1] r[tmp+1] etc.
    /// Get a name like eax or o2 from r24 or r8
    QString getRegName(SharedExp r);

    void setParamType(const char *name, SharedType ty);
    void setParamType(int idx, SharedType ty);

    /**
     * Get the BB with the entry point address for this procedure.
     * \note (not always the first BB)
     * \returns   Pointer to the entry point BB, or nullptr if not found
     */
    BasicBlock *getEntryBB();

    /// Set the entry BB for this procedure (constructor has the entry address)
    void setEntryBB();

    /// Get the callees.
    std::list<Function *>& getCallees() { return m_calleeList; }

    /**
     * Add this callee to the set of callees for this proc
     * \param  callee A pointer to the callee function
     */
    void addCallee(Function *callee);

    /// \returns true if this procedure contains the given address
    bool containsAddr(Address addr) const;

    /// Change BB containing this statement from a COMPCALL to a CALL.
    void undoComputedBB(Statement *stmt) const { m_cfg->undoComputedBB(stmt); }

    /// \copydoc Function::getProven
    virtual SharedExp getProven(SharedExp left) override;

    /// \copydoc Function::getPremised
    virtual SharedExp getPremised(SharedExp left) override;

    /// Set a location as a new premise, i.e. assume e=e
    void setPremise(SharedExp e)
    {
        e = e->clone();
        m_recurPremises[e] = e;
    }

    void killPremise(const SharedExp& e) { m_recurPremises.erase(e); }

    /// \copydoc Function::isPreserved
    virtual bool isPreserved(SharedExp e) override;

    bool searchAndReplace(const Exp& search, SharedExp replace);

    /// Add a location to the UseCollector; this means this location is used before defined,
    /// and hence is an *initial* parameter.
    /// \note final parameters don't use this information; it's only for handling recursion.
    void useBeforeDefine(const SharedExp& loc) { m_procUseCollector.insert(loc); }

    bool allPhisHaveDefs() const;

    /**
     * Copy the RTLs for the already decoded Indirect Control Transfer instructions,
     * and decode any new targets in this CFG.
     *
     * Note that we have to delay the new target decoding till now,
     * because otherwise we will attempt to decode nested switch statements
     * without having any SSA renaming, propagation, etc
     */
    void processDecodedICTs();

public:
    Address getTheReturnAddr() { return m_retStatement == nullptr ? Address::INVALID : m_retStatement->getRetAddr(); }
    void setTheReturnAddr(ReturnStatement *s, Address r)
    {
        assert(m_retStatement == nullptr);
        m_retStatement = s;
        m_retStatement->setRetAddr(r);
    }

    ReturnStatement *getTheReturnStatement() { return m_retStatement; }

    /**
     * Decide whether to filter out \p e (return true) or keep it
     * Filter out locations not possible as return locations. Return true to *remove* (filter *out*)
     * \returns true if \p e  should be filtered out
     */
    bool filterReturns(SharedExp e);

    /**
     * Decide whether to filter out \p e (return true) or keep it
     * Filter out locations not possible as parameters or arguments. Return true to remove
     * \returns true if \p e should be filtered out
     * \sa UserProc::filterReturns
     */
    bool filterParams(SharedExp e);

    UseCollector& getUseCollector() { return m_procUseCollector; }
    const UseCollector& getUseCollector() const { return m_procUseCollector; }

public:
    /// print this proc, mainly for debugging
    void print(QTextStream& out, bool html = false) const;
    void printParams(QTextStream& out, bool html = false) const;

    /// Print just the symbol map
    void printSymbolMap(QTextStream& out, bool html = false) const;

    /// For debugging
    void dumpLocals(QTextStream& os, bool html = false) const;

    void printDFG() const;
    void printUseGraph() const;

    void debugPrintAll(const QString& stepName);

private:
    void searchRegularLocals(OPER minusOrPlus, bool lastPass, int sp, StatementList& stmts);

    /// Return a string for a new local suitable for \p e
    QString newLocalName(const SharedExp& e);

    /// helper function for prove()
    bool prover(SharedExp query, std::set<PhiAssign *>& lastPhis, std::map<PhiAssign *, SharedExp>& cache,
                PhiAssign *lastPhi = nullptr);

private:
    SymbolMap m_symbolMap;

    /// Set of callees (Procedures that this procedure calls).
    /// Used for call graph, among other things
    std::list<Function *> m_calleeList;

    /**
     * A collector for initial parameters (locations used before being defined).
     * Note that final parameters don't use this;
     * it's only of use during group decompilation analysis (sorting out recursion)
     */
    UseCollector m_procUseCollector;

    /**
     * The list of parameters, ordered and filtered.
     * Note that a LocationList could be used, but then there would be nowhere to store the types (for DFA based TA)
     * The RHS is just ignored; the list is of ImplicitAssigns.
     * DESIGN ISSUE: it would be nice for the parameters' implicit assignments to be the sole definitions, i.e. not
     * need other implicit assignments for these. But the targets of RefExp's are not expected to change address,
     * so they are not suitable at present (since the addresses regularly get changed as the parameters get
     * recreated).
     */
    StatementList m_parameters;

    /// The set of address-escaped locals and parameters. If in this list, they should not be propagated
    LocationSet m_addressEscapedVars;

    /// DataFlow object. Holds information relevant to transforming to and from SSA form.
    DataFlow m_df;

public:
    std::shared_ptr<ProcSet> m_recursionGroup;

    /**
     * We ensure that there is only one return statement now.
     * See code in frontend/frontend.cpp handling case StmtType::Ret.
     * If no return statement, this will be nullptr.
     */
    ReturnStatement *m_retStatement;

private:
    Cfg *m_cfg; ///< The control flow graph.

    /**
     * The status of this user procedure.
     * Status: undecoded .. final decompiled
     */
    ProcStatus m_status;

    /*
     * Somewhat DEPRECATED now. Eventually use the localTable.
     * This map records the names and types for local variables. It should be a subset of the symbolMap, which also
     * stores parameters.
     * It is a convenient place to store the types of locals after conversion from SSA form,
     * since it is then difficult to access the definitions of locations.
     * This map could be combined with symbolMap below, but beware of parameters
     * (in symbols but not locals)
     */
    std::map<QString, SharedType> m_locals;

    int m_nextLocal = 0; ///< Number of the next local. Can't use locals.size() because some get deleted
    mutable int m_dfgCount; ///< used in dotty output
};
