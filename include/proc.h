/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2000-2001, Sun Microsystems, Inc
 * Copyright (C) 2002-2006, Trent Waddington and Mike Van Emmerik
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file       proc.h
  * OVERVIEW:   Interface for the procedure classes, which are used to store information about variables in the
  *                procedure such as parameters and locals.
  ******************************************************************************/

#ifndef _PROC_H_
#define _PROC_H_

#include "exp.h" // For lessExpStar
#include "cfg.h" // For cfg->simplify()
//#include "hllcode.h"
#include "memo.h"
#include "dataflow.h"  // For class UseCollector
#include "statement.h" // For embedded ReturnStatement pointer, etc

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
struct lessTI;
class Type;
class RTL;
class HLLCode;
class SyntaxNode;
class Parameter;
class Argument;
class Signature;
class Module;
class XMLProgParser;
class QTextStream;
class Log;
/***************************************************************************/ /**
  * Procedure class.
  ******************************************************************************/
/// Interface for the procedure classes, which are used to store information about variables in the
/// procedure such as parameters and locals.
class Function {
protected:
    friend class XMLProgParser;

public:
    Function(ADDRESS uNative, Signature *sig, Module *mod);
    virtual ~Function();

    void eraseFromParent();
    QString getName() const;
    void setName(const QString &nam);
    ADDRESS getNativeAddress() const;
    void setNativeAddress(ADDRESS a);
    Prog *getProg() { return prog; } //!< Get the program this procedure belongs to.
    void setProg(Prog *p) { prog = p; }
    Function *getFirstCaller();
    //! Set the first procedure that calls this procedure (or null for main/start).
    void setFirstCaller(Function *p) {
        if (m_firstCaller == nullptr)
            m_firstCaller = p;
    }
    Signature *getSignature() { return signature; } //!< Returns a pointer to the Signature
    void setSignature(Signature *sig) { signature = sig; }

    virtual void renameParam(const char *oldName, const char *newName);

    void matchParams(std::list<Exp *> &, UserProc &);

    std::list<Type> *getParamTypeList(const std::list<Exp *> &);
    virtual bool isLib() { return false; } //!< Return true if this is a library proc
    virtual bool isNoReturn() = 0;         //!< Return true if this procedure doesn't return

    /**
     * OutPut operator for a Proc object.
     */
    friend QTextStream &operator<<(QTextStream &os, const Function &proc);

    virtual Exp *getProven(Exp *left) = 0;   //!< Get the RHS, if any, that is proven for left
    virtual Exp *getPremised(Exp *left) = 0; //!< Get the RHS, if any, that is premised for left
    virtual bool isPreserved(Exp *e) = 0;    //!< Return whether e is preserved by this proc
    void setProvenTrue(Exp *fact);

    /**
     * Get the callers
     * Note: the callers will be in a random order (determined by memory allocation)
     */
    std::set<CallStatement *> &getCallers() { return callerSet; }

    //! Add to the set of callers
    void addCaller(CallStatement *caller) { callerSet.insert(caller); }
    void addCallers(std::set<UserProc *> &callers);

    void removeParameter(Exp *e);
    virtual void removeReturn(Exp *e);
    // virtual void        addReturn(Exp *e);
    //        void        sortParameters();

    virtual void printCallGraphXML(QTextStream &os, int depth, bool = true);
    void printDetailsXML();
    void clearVisited() { Visited = false; }
    bool isVisited() { return Visited; }

    Module *getParent() { return Parent; }
    void setParent(Module *c);
    void removeFromParent();
private:
    virtual void deleteCFG() {}
protected:
    typedef std::map<Exp *, Exp *, lessExpStar> mExpExp;
    bool Visited;
    Prog *prog;
    Signature *signature;
    ///////////////////////////////////////////////////
    // Persistent state
    ///////////////////////////////////////////////////
    ADDRESS address;
    Function *m_firstCaller;
    ADDRESS m_firstCallerAddr;
    // FIXME: shouldn't provenTrue be in UserProc, with logic associated with the signature doing the equivalent thing
    // for LibProcs?
    mExpExp provenTrue;
    // Cache of queries proven false (to save time)
    // mExpExp provenFalse;
    mExpExp recurPremises;
    std::set<CallStatement *> callerSet;
    Module *Parent;

    Function();
}; // class Proc

/***************************************************************************/ /**
  * LibProc class.
  ******************************************************************************/
class LibProc : public Function {
protected:
    friend class XMLProgParser;

public:
    LibProc(Module *mod, const QString &name, ADDRESS address);
    virtual ~LibProc();
    bool isLib() { return true; } //!< Return true, since is a library proc
    virtual bool isNoReturn();
    virtual Exp *getProven(Exp *left);
    virtual Exp *getPremised(Exp * /*left*/) { return nullptr; } //!< Get the RHS that is premised for left
    virtual bool isPreserved(Exp *e);                            //!< Return whether e is preserved by this proc
    void getInternalStatements(StatementList &internal);

protected:
    LibProc() : Function() {}
};

enum ProcStatus {
    PROC_UNDECODED,  ///< Has not even been decoded
    PROC_DECODED,    ///< Decoded, no attempt at decompiling
    PROC_SORTED,     ///< Decoded, and CFG has been sorted by address
    PROC_VISITED,    ///< Has been visited on the way down in decompile()
    PROC_INCYCLE,    ///< Is involved in cycles, has not completed early decompilation as yet
    PROC_PRESERVEDS, ///< Has had preservation analysis done
    PROC_EARLYDONE,  ///< Has completed everything except the global analyses
    PROC_FINAL,      ///< Has had final decompilation
    // , PROC_RETURNS   ///< Has had returns intersected with all caller's defines
    PROC_CODE_GENERATED ///< Has had code generated
};

typedef std::set<UserProc *> ProcSet;
typedef std::list<UserProc *> ProcList;

/***************************************************************************/ /**
  * UserProc class.
  ******************************************************************************/

class UserProc : public Function {
protected:
    friend class XMLProgParser;
    Cfg *cfg; //!< The control flow graph.

    /**
     * The status of this user procedure.
     * Status: undecoded .. final decompiled
     */
    ProcStatus status;

    /*
     * Somewhat DEPRECATED now. Eventually use the localTable.
     * This map records the names and types for local variables. It should be a subset of the symbolMap, which also
     * stores parameters.
     * It is a convenient place to store the types of locals after
     * conversion from SSA form, since it is then difficult to access the definitions of locations.
     * This map could be combined with symbolMap below, but beware of parameters (in symbols but not locals)
     */
    std::map<QString, SharedType > locals;

    int nextLocal = 0; //!< Number of the next local. Can't use locals.size() because some get deleted
    int nextParam = 0; //!< Number for param1, param2, etc

public:
    /**
     * A map between machine dependent locations and their corresponding symbolic, machine independent
     * representations.  Example: m[r28{0} - 8] -> local5; this means that *after* transforming out of SSA
     * form, any locations not specifically mapped otherwise (e.g. m[r28{0} - 8]{55} -> local6) will get this
     * name.
     * It is a *multi*map because one location can have several default names differentiated by type.
     * E.g. r24 -> eax for int, r24 -> eax_1 for float
     */
    typedef std::multimap<const Exp *, Exp *, lessExpStar> SymbolMap;

private:
    SymbolMap symbolMap;
    /**
     * The local "symbol table", which is aware of overlaps
     */
    DataIntervalMap localTable;

    /**
     * Set of callees (Procedures that this procedure calls). Used for call graph, among other things
     */
    std::list<Function *> calleeList;
    UseCollector col;
    StatementList parameters;

    /**
     * The set of address-escaped locals and parameters. If in this list, they should not be propagated
     */
    LocationSet addressEscapedVars;

    // The modifieds for the procedure are now stored in the return statement

    /**
     * DataFlow object. Holds information relevant to transforming to and from SSA form.
     */
    DataFlow df;
    int stmtNumber;
    std::shared_ptr<ProcSet> cycleGrp;

public:
    UserProc(Module *mod, const QString &name, ADDRESS address);
    virtual ~UserProc();
    void setDecoded();
    void unDecode();
    //! Returns a pointer to the CFG object.
    Cfg *getCFG() { return cfg; }
    //! Returns a pointer to the DataFlow object.
    DataFlow *getDataFlow() { return &df; }
    void deleteCFG() override;
    virtual bool isNoReturn() override;

    SyntaxNode *getAST();
    void printAST(SyntaxNode *a = nullptr);

    //! Returns whether or not this procedure can be decoded (i.e. has it already been decoded).
    bool isDecoded() { return status >= PROC_DECODED; }
    bool isDecompiled() { return status >= PROC_FINAL; }
    bool isEarlyRecursive() const { return cycleGrp != nullptr && status <= PROC_INCYCLE; }
    bool doesRecurseTo(UserProc *p) { return cycleGrp && cycleGrp->find(p) != cycleGrp->end(); }

    bool isSorted() { return status >= PROC_SORTED; }
    void setSorted() { setStatus(PROC_SORTED); }

    ProcStatus getStatus() { return status; }
    void setStatus(ProcStatus s);
    void generateCode(HLLCode *hll);

    void print(QTextStream &out, bool html = false) const;
    void printParams(QTextStream &out, bool html = false) const;
    char *prints();
    void dump();

    void printDFG() const;
    void printSymbolMap(QTextStream &out, bool html = false) const;
    void dumpSymbolMap();
    void dumpSymbolMapx();
    void testSymbolMap();
    void dumpLocals(QTextStream &os, bool html = false) const;
    void dumpLocals();
    //! simplify the statements in this proc
    void simplify() { cfg->simplify(); }
    std::shared_ptr<ProcSet> decompile(ProcList *path, int &indent);
    void initialiseDecompile();
    void earlyDecompile();
    std::shared_ptr<ProcSet> middleDecompile(ProcList *path, int indent);
    void recursionGroupAnalysis(ProcList *path, int indent);

    void typeAnalysis();

    // Split the set of cycle-associated procs into individual subcycles.
    // void        findSubCycles(CycleList& path, CycleSet& cs, CycleSetSet& sset);

    bool inductivePreservation(UserProc *);
    void markAsNonChildless(const std::shared_ptr<ProcSet> &cs);

    void updateCalls();
    void branchAnalysis();
    void fixUglyBranches();
    void placePhiFunctions() { df.placePhiFunctions(this); }
    bool doRenameBlockVars(int pass, bool clearStacks = false);
    bool canRename(Exp *e) { return df.canRename(e, this); }

    Instruction *getStmtAtLex(unsigned int begin, unsigned int end);

    void initStatements();
    void numberStatements();
    bool nameStackLocations();
    void removeRedundantPhis();
    void findPreserveds();
    void findSpPreservation();
    void removeSpAssignsIfPossible();
    void removeMatchingAssignsIfPossible(Exp *e);
    void updateReturnTypes();
    void fixCallAndPhiRefs();
    void initialParameters();
    void mapLocalsAndParams();
    void findFinalParameters();
    int nextParamNum() { return ++nextParam; }
    void addParameter(Exp *e, SharedType ty);
    void insertParameter(Exp *e, SharedType ty);
    //        void        addNewReturns(int depth);
    void updateArguments();
    void updateCallDefines();
    void replaceSimpleGlobalConstants();
    void reverseStrengthReduction();

    void trimParameters(int depth = -1);
    void processFloatConstants();
    // void        mapExpressionsToParameters();   ///< must be in SSA form
    void mapExpressionsToLocals(bool lastPass = false);
    void addParameterSymbols();
    bool isLocal(Exp *e);
    bool isLocalOrParam(Exp *e);
    bool isLocalOrParamPattern(Exp *e);
    bool existsLocal(const QString &name);
    bool isAddressEscapedVar(Exp *e) { return addressEscapedVars.exists(e); }
    bool isPropagatable(Exp *e);
    void assignProcsToCalls();
    void finalSimplify();
    void eliminateDuplicateArgs();

private:
    void searchRegularLocals(OPER minusOrPlus, bool lastPass, int sp, StatementList &stmts);
    QString newLocalName(Exp &e);

public:
    bool removeNullStatements();
    bool removeDeadStatements();
    typedef std::map<Instruction *, int> RefCounter;
    void countRefs(RefCounter &refCounts);

    void remUnusedStmtEtc();
    void remUnusedStmtEtc(RefCounter &refCounts /* , int depth*/);
    void removeUnusedLocals();
    void mapTempsToLocals();
    void removeCallLiveness();
    bool propagateAndRemoveStatements();
    bool propagateStatements(bool &convert, int pass);
    void findLiveAtDomPhi(LocationSet &usedByDomPhi);
#if USE_DOMINANCE_NUMS
    void setDominanceNumbers();
#endif
    void propagateToCollector();
    void clearUses();

    // int        findMaxDepth();                    ///< Find max memory nesting depth.

    void fromSSAform();
    void findPhiUnites(ConnectionGraph &pu);
    void insertAssignAfter(Instruction *s, Exp *left, Exp *right);
    void removeSubscriptsFromSymbols();
    void removeSubscriptsFromParameters();

    void insertStatementAfter(Instruction *s, Instruction *a);
    void nameParameterPhis();
    void mapParameters();

    void conTypeAnalysis();
    void dfaTypeAnalysis();
    void dfa_analyze_scaled_array_ref(Instruction *s, Prog *prog);

    void dfa_analyze_implict_assigns(Instruction *s, Prog *prog);
    bool ellipsisProcessing();

    // For the final pass of removing returns that are never used
    // typedef    std::map<UserProc*, std::set<Exp*, lessExpStar> > ReturnCounter;
    bool doesParamChainToCall(Exp *param, UserProc *p, ProcSet *Visited);
    bool isRetNonFakeUsed(CallStatement *c, Exp *loc, UserProc *p, ProcSet *Visited);

    bool removeRedundantParameters();
    bool removeRedundantReturns(std::set<UserProc *> &removeRetSet);
    bool checkForGainfulUse(Exp *e, ProcSet &Visited);
    void updateForUseChange(std::set<UserProc *> &removeRetSet);
    bool prove(Binary * query, bool conditional = false);

    bool prover(Exp *query, std::set<PhiAssign *> &lastPhis, std::map<PhiAssign *, Exp *> &cache,
                PhiAssign *lastPhi = nullptr);
    void promoteSignature();
    void getStatements(StatementList &stmts) const;
    virtual void removeReturn(Exp *e) override;
    void removeStatement(Instruction *stmt);
    bool searchAll(const Exp &search, std::list<Exp *> &result);

    void getDefinitions(LocationSet &defs);
    void addImplicitAssigns();
    void makeSymbolsImplicit();
    void makeParamsImplicit();
    StatementList &getParameters() { return parameters; }
    StatementList &getModifieds() { return theReturnStatement->getModifieds(); }

    Exp *getSymbolExp(Exp *le, SharedType ty = nullptr, bool lastPass = false);
    Exp *newLocal(SharedType ty, Exp &e, char *nam = nullptr);
    void addLocal(SharedType ty, const QString &nam, Exp *e);
    SharedType getLocalType(const QString &nam);
    void setLocalType(const QString &nam, SharedType ty);
    SharedType getParamType(const QString &nam);
    const Exp *expFromSymbol(const QString &nam) const;
    void mapSymbolTo(const Exp *from, Exp *to);
    void mapSymbolToRepl(const Exp *from, Exp *oldTo, Exp *newTo);
    void removeSymbolMapping(const Exp *from, Exp *to);
    Exp *getSymbolFor(const Exp *e, SharedType ty);
    QString lookupSym(const Exp &e, SharedType ty);
    QString lookupSymFromRef(RefExp &r);
    QString lookupSymFromRefAny(RefExp &r);
    QString lookupParam(Exp *e);
    void checkLocalFor(RefExp &r);
    SharedType getTypeForLocation(const Exp *e);
    const SharedType getTypeForLocation(const Exp *e) const;
    QString findLocal(Exp &e, SharedType ty);
    QString findLocalFromRef(RefExp &r);
    QString findFirstSymbol(Exp *e);
    int getNumLocals() { return (int)locals.size(); }
    QString getLocalName(int n);
    QString getSymbolName(Exp *e);
    void renameLocal(const char *oldName, const char *newName);
    virtual void renameParam(const char *oldName, const char *newName) override;

    QString getRegName(Exp *r);
    void setParamType(const char *nam, SharedType ty);
    void setParamType(int idx, SharedType ty);

    BasicBlock *getEntryBB();
    void setEntryBB();
    //! Get the callees.
    std::list<Function *> &getCallees() { return calleeList; }
    void addCallee(Function *callee);
    // void                addCallees(std::list<UserProc*>& callees);
    bool containsAddr(ADDRESS uAddr);
    //! Change BB containing this statement from a COMPCALL to a CALL.
    void undoComputedBB(Instruction *stmt) { cfg->undoComputedBB(stmt); }
    virtual Exp *getProven(Exp *left) override;
    virtual Exp *getPremised(Exp *left) override;
    //! Set a location as a new premise, i.e. assume e=e
    void setPremise(Exp *e) {
        e = e->clone();
        recurPremises[e] = e;
    }
    void killPremise(Exp *e) { recurPremises.erase(e); }
    virtual bool isPreserved(Exp *e) override;

    virtual void printCallGraphXML(QTextStream &os, int depth, bool recurse = true) override;
    void printDecodedXML();
    void printAnalysedXML();
    void printSSAXML();
    void printXML();
    void printUseGraph();

    bool searchAndReplace(const Exp &search, Exp *replace);
    void castConst(int num, SharedType ty);
    /// Add a location to the UseCollector; this means this location is used before defined,
    /// and hence is an *initial* parameter.
    /// \note final parameters don't use this information; it's only for handling recursion.
    void useBeforeDefine(Exp *loc) { col.insert(loc); }
    void processDecodedICTs();

private:
    ReturnStatement *theReturnStatement;
    mutable int DFGcount; //!< used in dotty output
public:
    ADDRESS getTheReturnAddr() { return theReturnStatement == nullptr ? NO_ADDRESS : theReturnStatement->getRetAddr(); }
    void setTheReturnAddr(ReturnStatement *s, ADDRESS r) {
        assert(theReturnStatement == nullptr);
        theReturnStatement = s;
        theReturnStatement->setRetAddr(r);
    }
    ReturnStatement *getTheReturnStatement() { return theReturnStatement; }
    bool filterReturns(Exp *e);
    bool filterParams(Exp *e);
    void setImplicitRef(Instruction *s, Exp *a, SharedType ty);

    void verifyPHIs();
    void debugPrintAll(const char *c);

protected:
    UserProc();
    void setCFG(Cfg *c) { cfg = c; }
}; // class UserProc
Log &operator<<(Log &out, const UserProc &c);

#endif
