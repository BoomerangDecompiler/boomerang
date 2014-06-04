/*
 * Copyright (C) 1997-2005, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file    cfg.h
  * \brief   Interface for a control flow graph, based on basic block nodes.
  ******************************************************************************/

#ifndef _CFG_H_
#define _CFG_H_

#include "types.h"
#include "exphelp.h"    // For lessExpStar
#include "basicblock.h" // For the BB nodes
#include "dataflow.h"   // For embedded class DataFlow

#include <cstdio> // For FILE
#include <list>
#include <vector>
#include <set>
#include <map>
#include <string>
#define DEBUG_LIVENESS (Boomerang::get()->debugLiveness)

class Function;
class Prog;
class UserProc;
class UseSet;
class LocationSet;
class SSACounts;
class BasicBlock;
class HLLCode;
class CallStatement;
class BranchStatement;
class RTL;
struct DOM;
class XMLProgParser;
class Global;
class Parameter;

#define BTHEN 0
#define BELSE 1

// A type for the ADDRESS to BB map
typedef std::map<ADDRESS, BasicBlock *, std::less<ADDRESS>> MAPBB;

class Cfg {
    typedef std::set<CallStatement *> sCallStatement;
    typedef std::map<Exp *, Instruction *, lessExpStar> mExpStatement;
    mutable bool WellFormed;
    bool structured;
    bool ImplicitsDone;
    int lastLabel;
    UserProc *myProc;
    std::list<BasicBlock *> m_listBB;
    std::vector<BasicBlock *> Ordering;
    std::vector<BasicBlock *> revOrdering;
    MAPBB m_mapBB;
    BasicBlock *entryBB;
    BasicBlock *exitBB;
    sCallStatement CallSites;
    mExpStatement implicitMap;

  public:
    class BBAlreadyExistsError : public std::exception {
      public:
        BasicBlock *pBB;
        BBAlreadyExistsError(BasicBlock *_pBB) : pBB(_pBB) {}
    };
    Cfg();
    ~Cfg();
    void setProc(UserProc *proc);
    void clear();
    size_t getNumBBs() { return m_listBB.size(); } //!<Get the number of BBs
    const Cfg &operator=(const Cfg &other);        /* Copy constructor */

    BasicBlock *newBB(std::list<RTL *> *pRtls, BBTYPE bbType, uint32_t iNumOutEdges) throw(BBAlreadyExistsError);
    BasicBlock *newIncompleteBB(ADDRESS addr);
    void addOutEdge(BasicBlock *pBB, ADDRESS adr, bool bSetLabel = false);
    void addOutEdge(BasicBlock *pBB, BasicBlock *pDestBB, bool bSetLabel = false);
    void setLabel(BasicBlock *pBB);
    BasicBlock *getFirstBB(BB_IT &it);
    const BasicBlock *getFirstBB(BBC_IT &it) const;

    BasicBlock *getNextBB(BB_IT &it);
    const BasicBlock *getNextBB(BBC_IT &it) const;

    /*
     * An alternative to the above is to use begin() and end():
     */
    typedef BB_IT iterator;
    iterator begin() { return m_listBB.begin(); }
    iterator end() { return m_listBB.end(); }
    bool label(ADDRESS uNativeAddr, BasicBlock *&pNewBB);
    bool isIncomplete(ADDRESS uNativeAddr) const;
    bool existsBB(ADDRESS uNativeAddr) const;
    void sortByAddress();
    void sortByFirstDFT();
    void sortByLastDFT();

    bool wellFormCfg() const;
    bool mergeBBs(BasicBlock *pb1, BasicBlock *pb2);
    bool compressCfg();
    bool establishDFTOrder();
    bool establishRevDFTOrder();

    int pbbToIndex(BasicBlock *pBB);
    void unTraverse();
    bool isWellFormed();
    bool isOrphan(ADDRESS uAddr);
    bool joinBB(BasicBlock *pb1, BasicBlock *pb2);

    void removeBB(BasicBlock *bb);
    void addCall(CallStatement *call);
    sCallStatement &getCalls();
    void searchAndReplace(const Exp &search, Exp *replace);
    bool searchAll(const Exp &search, std::list<Exp *> &result);
    Exp *getReturnVal();
    void structure();
    void addJunctionStatements();
    void removeJunctionStatements();

    //! return a bb given an address
    BasicBlock *bbForAddr(ADDRESS addr) { return m_mapBB[addr]; }
    void simplify();
    void undoComputedBB(Instruction *stmt);

  private:
    BasicBlock *splitBB(BasicBlock *pBB, ADDRESS uNativeAddr, BasicBlock *pNewBB = 0, bool bDelRtls = false);
    void completeMerge(BasicBlock *pb1, BasicBlock *pb2, bool bDelete = false);
    bool checkEntryBB();

  public:
    BasicBlock *splitForBranch(BasicBlock *pBB, RTL *rtl, BranchStatement *br1, BranchStatement *br2, BB_IT &it);

    /////////////////////////////////////////////////////////////////////////
    // Control flow analysis stuff, lifted from Doug Simon's honours thesis.
    /////////////////////////////////////////////////////////////////////////
    void setTimeStamps();
    BasicBlock *commonPDom(BasicBlock *curImmPDom, BasicBlock *succImmPDom);
    void findImmedPDom();
    void structConds();
    void structLoops();
    void checkConds();
    void determineLoopType(BasicBlock *header, bool *&loopNodes);
    void findLoopFollow(BasicBlock *header, bool *&loopNodes);
    void tagNodesInLoop(BasicBlock *header, bool *&loopNodes);

    void removeUnneededLabels(HLLCode *hll);
    void generateDotFile(QTextStream &of);

    /////////////////////////////////////////////////////////////////////////
    // Get the entry-point or exit BB
    /////////////////////////////////////////////////////////////////////////
    BasicBlock *getEntryBB() { return entryBB; }
    BasicBlock *getExitBB() { return exitBB; }

    /////////////////////////////////////////////////////////////////////////
    // Set the entry-point BB (and exit BB as well)
    /////////////////////////////////////////////////////////////////////////
    void setEntryBB(BasicBlock *bb);
    void setExitBB(BasicBlock *bb);

    BasicBlock *findRetNode();
    void addNewOutEdge(BasicBlock *fromBB, BasicBlock *newOutEdge);
    /////////////////////////////////////////////////////////////////////////
    // print this cfg, mainly for debugging
    /////////////////////////////////////////////////////////////////////////
    void print(QTextStream &out, bool html = false);
    void printToLog();
    void dump();            // Dump to LOG_STREAM()
    void dumpImplicitMap(); // Dump the implicit map to LOG_STREAM()
    bool decodeIndirectJmp(UserProc *proc);

    /////////////////////////////////////////////////////////////////////////
    // Implicit assignments
    /////////////////////////////////////////////////////////////////////////
    Instruction *findImplicitAssign(Exp *x);
    Instruction *findTheImplicitAssign(Exp *x);
    Instruction *findImplicitParamAssign(Parameter *p);
    void removeImplicitAssign(Exp *x);
    bool implicitsDone() { return ImplicitsDone; }    //!<  True if implicits have been created
    void setImplicitsDone() { ImplicitsDone = true; } //!< Call when implicits have been created
    void findInterferences(ConnectionGraph &ig);
    void appendBBs(std::list<BasicBlock *> &worklist, std::set<BasicBlock *> &workset);
    void removeUsedGlobals(std::set<Global *> &unusedGlobals);
    void bbSearchAll(Exp *search, std::list<Exp *> &result, bool ch);

    bool removeOrphanBBs();
protected:
    void addBB(BasicBlock *bb) { m_listBB.push_back(bb); }
    friend class XMLProgParser;
}; /* Cfg */

#endif
