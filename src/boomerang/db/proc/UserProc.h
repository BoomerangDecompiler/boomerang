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


class Binary;
class UserProc;


enum ProcStatus : uint8_t
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
     * A map between machine dependent locations and their corresponding symbolic,
     * machine independent representations.
     * Example: m[r28{0} - 8] -> local5; this means that *after* transforming
     * out of SSA form, any locations not specifically mapped otherwise
     * (e.g. m[r28{0} - 8]{55} -> local6) will get this name.
     *
     * It is a *multi*map because one location can have several default names
     * differentiated by type.
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
    QString toString() const;

    /// \copydoc Function::isNoReturn
    virtual bool isNoReturn() const override;

    /// \copydoc Function::renameParameter
    virtual void renameParameter(const QString& oldName, const QString& newName) override;

    /// \copydoc Function::getProven
    virtual SharedExp getProven(SharedExp left) override;

    /// \copydoc Function::getPremised
    virtual SharedExp getPremised(SharedExp left) override;

    /// \copydoc Function::isPreserved
    virtual bool isPreserved(SharedExp e) override;

public:
    /// \returns a pointer to the CFG object.
    Cfg *getCFG() { return m_cfg.get(); }
    const Cfg *getCFG() const { return m_cfg.get(); }

    /// Returns a pointer to the DataFlow object.
    DataFlow *getDataFlow() { return &m_df; }
    const DataFlow *getDataFlow() const { return &m_df; }

    const std::shared_ptr<ProcSet>& getRecursionGroup() { return m_recursionGroup; }
    void setRecursionGroup(const std::shared_ptr<ProcSet>& recursionGroup) { m_recursionGroup = recursionGroup; }

    ProcStatus getStatus() const { return m_status; }
    void setStatus(ProcStatus s);

    /// Returns whether or not this procedure can be decoded (i.e. has it already been decoded).
    bool isDecoded() const { return m_status >= PROC_DECODED; }
    bool isDecompiled() const { return m_status >= PROC_FINAL; }

    /// Records that this procedure has been decoded.
    void setDecoded();

    bool isEarlyRecursive() const { return m_recursionGroup != nullptr && m_status <= PROC_INCYCLE; }
    bool doesRecurseTo(UserProc *proc) const { return m_recursionGroup && m_recursionGroup->find(proc) != m_recursionGroup->end(); }

    /**
     * Get the BB with the entry point address for this procedure.
     * \note (not always the first BB)
     * \returns   Pointer to the entry point BB, or nullptr if not found
     */
    BasicBlock *getEntryBB();

    /// Set the entry BB for this procedure (constructor has the entry address)
    void setEntryBB();

    /// Decompile this procedure, and all callees.
    void decompileRecursive();

public:
    // statement related

    /// Update statement numbers
    void numberStatements() const;

    /// \returns all statements in this UserProc
    void getStatements(StatementList& stmts) const;

    /// Remove (but not delete) \p stmt from this UserProc
    /// \returns true iff successfully removed
    bool removeStatement(Statement *stmt);

    Assign *insertAssignAfter(Statement *s, SharedExp left, SharedExp right);

    /// Insert statement \p stmt after statement \p afterThis.
    /// \note this procedure is designed for the front end, where enclosing BBs are not set up yet.
    /// So this is an inefficient linear search!
    bool insertStatementAfter(Statement *afterThis, Statement *stmt);

public:
    // parameter related

    StatementList& getParameters() { return m_parameters; }
    const StatementList& getParameters() const { return m_parameters; }

    /// Add the parameter to the signature
    void addParameterToSignature(SharedExp e, SharedType ty);

    /**
     * Insert into parameters list correctly sorted.
     * Update the parameters, in case the signature and hence ordering
     * and filtering has changed, or the locations in the collector have changed
     */
    void insertParameter(SharedExp e, SharedType ty);

    SharedConstType getParamType(const QString& name) const;
    SharedType getParamType(const QString& name);

    void setParamType(const char *name, SharedType ty);
    void setParamType(int idx, SharedType ty);

    /// Map expressions to locals and initial parameters
    void mapLocalsAndParams();

    /// e is a parameter location, e.g. r8 or m[r28{0}+8]. Lookup a symbol for it
    /// Find the implicit definition for \a e and lookup a symbol
    QString lookupParam(SharedConstExp e) const;

    /**
     * Filter out locations not possible as parameters or arguments.
     * \returns true if \p e should be filtered out (i.e. removed)
     * \sa UserProc::filterReturns
     */
    bool filterParams(SharedExp e);

public:
    // return related

    /// \returns the address of the RTL that contains the one and only return statement
    Address getRetAddr();

    /// \param rtlAddr the address of the RTL containing \p retStmt
    void setRetStmt(ReturnStatement *retStmt, Address rtlAddr);

    ReturnStatement *getRetStmt() { return m_retStatement; }
    const ReturnStatement *getRetStmt() const { return m_retStatement; }

    void removeRetStmt() { m_retStatement = nullptr; }

    /**
     * Filter out locations not possible as return locations.
     * \returns true if \p e  should be filtered out (i.e. removed)
     * \sa UserProc::filterParams
     */
    bool filterReturns(SharedExp e);

public:
    // local variable related

    const std::map<QString, SharedType>& getLocals() const { return m_locals; }
    std::map<QString, SharedType>& getLocals() { return m_locals; }

    /**
     * Return the next available local variable; make it the given type.
     * \note was returning TypedExp*.
     * If \p name is non null, use that name
     */
    SharedExp createLocal(SharedType ty, const SharedExp& e, char *name = nullptr);

    /// Add a new local supplying all needed information.
    void addLocal(SharedType ty, const QString& name, SharedExp e);

    /// Check if \p r is already mapped to a local, else add one
    void ensureExpIsMappedToLocal(const std::shared_ptr<RefExp>& ref);

    /**
     * Return an expression that is equivalent to e in terms of local variables.
     * Creates new locals as needed.
     */
    SharedExp getSymbolExp(SharedExp le, SharedType ty = nullptr, bool lastPass = false);

    /// Determine whether e is a local, either as a true opLocal (e.g. generated by fromSSA), or if it is in the
    /// symbol map and the name is in the locals map. If it is a local, return its name, else nullptr
    QString findLocal(const SharedExp& e, SharedType ty);

    /// return a local's type
    SharedConstType getLocalType(const QString& name) const;
    void setLocalType(const QString& name, SharedType ty);

    /// Is this m[sp{-} +/- K]?
    /// \returns true if \p e could represent a stack local or stack param
    bool isLocalOrParamPattern(SharedConstExp e) const;

public:
    // symbol related
    SymbolMap& getSymbolMap() { return m_symbolMap; }
    const SymbolMap& getSymbolMap() const { return m_symbolMap; }

    /// \returns a symbol's exp (note: the original exp, like r24, not local1)
    SharedConstExp expFromSymbol(const QString& name) const;

    void mapSymbolTo(const SharedConstExp& from, SharedExp to);

    QString lookupSym(const SharedConstExp& e, SharedConstType ty) const;

    /// Lookup a specific symbol for the given ref
    QString lookupSymFromRef(const std::shared_ptr<const RefExp>& ref) const;

    /// Lookup a specific symbol if any, else the general one if any
    QString lookupSymFromRefAny(const std::shared_ptr<const RefExp>& ref) const;

public:
    // call / recursion related

    /**
     * Mark calls involved in the recursion cycle as non childless
     * (each child has had middleDecompile called on it now).
     * \todo Not sure that this is needed...
     */
    void markAsNonChildless(const std::shared_ptr<ProcSet>& cs);

    /// Find the procs the calls point to.
    /// To be called after decoding all procs.
    void assignProcsToCalls();

    /// Get the callees.
    std::list<Function *>& getCallees() { return m_calleeList; }

    /**
     * Add this callee to the set of callees for this proc
     * \param  callee A pointer to the callee function
     */
    void addCallee(Function *callee);

public:
    bool canRename(SharedConstExp e) const { return m_df.canRename(e); }

    /// perform final simplifications
    void finalSimplify();

    /// Remove unused statements.
    void remUnusedStmtEtc();

    /// Propagate into xxx of m[xxx] in the UseCollector (locations live at the entry of this proc)
    void propagateToCollector();

    UseCollector& getUseCollector() { return m_procUseCollector; }
    const UseCollector& getUseCollector() const { return m_procUseCollector; }


    /// Prove any arbitary property of this procedure.
    /// If \p conditional is true, do not save the result,
    /// as it may be conditional on premises stored in other procedures
    /// \note this function was non-reentrant, but now reentrancy is frequently used
    bool prove(const std::shared_ptr<Binary>& query, bool conditional = false);

    /// promote the signature if possible
    void promoteSignature();

    /// Find the type of the local or parameter \a e
    SharedType getTypeForLocation(const SharedExp& e);
    SharedConstType getTypeForLocation(const SharedConstExp& e) const;

    QString findFirstSymbol(const SharedConstExp& e) const;

    /// Get a name like eax or o2 from r24 or r8
    /// \todo write tests for getRegName in all combinations of r[1] r[tmp+1] etc.
    QString getRegName(SharedExp r);

    /// Change BB containing this statement from a COMPCALL to a CALL.
    void undoComputedBB(Statement *stmt) const { m_cfg->undoComputedBB(stmt); }

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
    /// True if a local exists with name \p name
    bool existsLocal(const QString& name) const;

    /// Return a string for a new local suitable for \p e
    QString newLocalName(const SharedExp& e);

    /// helper function for prove()
    bool prover(SharedExp query, std::set<PhiAssign *>& lastPhis, std::map<PhiAssign *, SharedExp>& cache,
                PhiAssign *lastPhi = nullptr);

    // FIXME: is this the same as lookupSym() now?
    /// Lookup the expression in the symbol map. Return nullptr or a C string with the symbol. Use the Type* ty to
    /// select from several names in the multimap; the name corresponding to the first compatible type is returned
    SharedExp getSymbolFor(const SharedConstExp& e, const SharedConstType& ty) const;

    /// Set a location as a new premise, i.e. assume e=e
    void setPremise(SharedExp e)
    {
        e = e->clone();
        m_recurPremises[e] = e;
    }

    void killPremise(const SharedExp& e) { m_recurPremises.erase(e); }

private:
    /**
     * The status of this user procedure.
     * Status: undecoded .. final decompiled
     */
    ProcStatus m_status = PROC_UNDECODED;
    mutable short m_dfgCount = 0; ///< used in dotty output
    int m_nextLocal = 0; ///< Number of the next local. Can't use locals.size() because some get deleted

    std::unique_ptr<Cfg> m_cfg; ///< The control flow graph.

    /// DataFlow object. Holds information relevant to transforming to and from SSA form.
    DataFlow m_df;

    /**
     * The list of parameters, ordered and filtered.
     * Note that a LocationList could be used, but then there would be nowhere
     * to store the types (for DFA based TA)
     *
     * The RHS is just ignored; the list is of ImplicitAssigns.
     *
     * \note DESIGN ISSUE: it would be nice for the parameters' implicit assignments
     * to be the sole definitions, i.e. not need other implicit assignments for these.
     * But the targets of RefExp's are not expected to change address, so they are
     * not suitable at present (since the addresses regularly get changed
     * as the parameters get recreated).
     */
    StatementList m_parameters;

    SymbolMap m_symbolMap;

    /// Set of callees (Procedures that this procedure calls).
    /// Used for call graph, among other things
    std::list<Function *> m_calleeList;

    /**
     * Somewhat DEPRECATED now. Eventually use the localTable.
     * This map records the names and types for local variables. It should be a subset of the symbolMap, which also
     * stores parameters.
     * It is a convenient place to store the types of locals after conversion from SSA form,
     * since it is then difficult to access the definitions of locations.
     * This map could be combined with symbolMap below, but beware of parameters
     * (in symbols but not locals)
     */
    std::map<QString, SharedType> m_locals;

    /**
     * A collector for initial parameters (locations used before being defined).
     * Note that final parameters don't use this;
     * it's only of use during group decompilation analysis (sorting out recursion)
     */
    UseCollector m_procUseCollector;

    std::shared_ptr<ProcSet> m_recursionGroup;

    /**
     * We ensure that there is only one return statement now.
     * See code in frontend/frontend.cpp handling case StmtType::Ret.
     * If no return statement, this will be nullptr.
     */
    ReturnStatement *m_retStatement = nullptr;
};
