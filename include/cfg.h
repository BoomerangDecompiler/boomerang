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

/*==============================================================================
 * FILE:       cfg.h
 * OVERVIEW:   Interface for a control flow graph, based on basic block nodes.
 *============================================================================*/

/*
 * $Revision$    // 1.69.2.7
 * 18 Apr 02 - Mike: Mods for boomerang
 * 04 Dec 02 - Mike: Added isJmpZ
 */

#ifndef _CFG_H_
#define _CFG_H_

#include <stdio.h>        // For FILE
#include <list>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <string>

#if defined(_MSC_VER)
#pragma warning(disable:4290)
#endif

#include "types.h"
#include "exphelp.h"    // For lessExpStar
#include "basicblock.h"    // For the BB nodes
#include "dataflow.h"    // For embedded class DataFlow

#define DEBUG_LIVENESS    (Boomerang::get()->debugLiveness)

class Proc;
class Prog;
class UserProc;
class UseSet;
class LocationSet;
class SSACounts;
class BinaryFile;
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
typedef std::map<ADDRESS, PBB, std::less<ADDRESS> >      MAPBB;

class Cfg {
typedef std::set<CallStatement*> sCallStatement;
typedef std::map<Exp*, Statement*, lessExpStar> mExpStatement;
        bool            m_bWellFormed, structured;
        bool            bImplicitsDone;
        int             lastLabel;
        UserProc *      myProc;
        std::list<PBB>  m_listBB;
        std::vector<PBB> Ordering;
        std::vector<PBB> revOrdering;

        MAPBB           m_mapBB;

        BasicBlock *    entryBB;
        BasicBlock *    exitBB;
        sCallStatement  callSites;
        mExpStatement   implicitMap;

public:
        class BBAlreadyExistsError : public std::exception {
        public:
                            PBB pBB;
                            BBAlreadyExistsError(PBB pBB) : pBB(pBB) { }
        };
                        Cfg();
                        ~Cfg();
        void            setProc(UserProc* proc);
        void            clear();
        unsigned        getNumBBs() {return m_listBB.size();} //!<Get the number of BBs
        const Cfg&      operator=(const Cfg& other); /* Copy constructor */


        PBB             newBB ( std::list<RTL*>* pRtls, BBTYPE bbType, int iNumOutEdges) throw(BBAlreadyExistsError);
        BasicBlock *    newIncompleteBB(ADDRESS addr);
        //Remove the incomplete BB at uAddr, if any. Was used when dealing with the SKIP instruction, but no longer.
        void            removeIncBB(ADDRESS uAddr);
        void            addOutEdge(PBB pBB, ADDRESS adr, bool bSetLabel = false);
        void            addOutEdge(BasicBlock * pBB, BasicBlock * pDestBB, bool bSetLabel = false);
        void            setLabel(BasicBlock * pBB);
        PBB             getFirstBB(BB_IT& it);
        BasicBlock *    getNextBB(BB_IT& it);

        /*
         * An alternative to the above is to use begin() and end():
         */
typedef BB_IT           iterator;
        iterator        begin() { return m_listBB.begin(); }
        iterator        end()   { return m_listBB.end(); }
        bool            label ( ADDRESS uNativeAddr, PBB& pNewBB );
        bool            isIncomplete ( ADDRESS uNativeAddr );
        bool            existsBB ( ADDRESS uNativeAddr );
        void            sortByAddress ();
        void            sortByFirstDFT();
        void            sortByLastDFT();
        void            updateVectorBB();

        bool            wellFormCfg ( );
        bool            mergeBBs ( PBB pb1, PBB pb2 );
        bool            compressCfg ( );
        bool            establishDFTOrder();
        bool            establishRevDFTOrder();

        int             pbbToIndex (PBB pBB);
        void            unTraverse ( );
        bool            isWellFormed ( );
        bool            isOrphan ( ADDRESS uAddr);
        bool            joinBB( PBB pb1, PBB pb2);

        void            removeBB( PBB bb);
        void            resetDFASets(); //!< Resets the DFA sets of all the BBs.
        void            addCall(CallStatement* call);
        sCallStatement& getCalls();
        void            searchAndReplace(Exp* search, Exp* replace);
        bool            searchAll(Exp* search, std::list<Exp*> &result);
        //! Set the return value for this CFG (assumes there is only one exit bb)
        void            setReturnVal(Exp *e); //TODO: not used, nor defined.
        Exp *           getReturnVal();
        void            structure();
        void            addJunctionStatements();
        void            removeJunctionStatements();
        /*
         * Resolves goto/branch destinations to statements
         * Good to do this late, as removing statements doesn't update this information.
         */
        void            resolveGotos(); //TODO: not used, nor defined

        //! Virtual Function Call analysis
        void            virtualFunctionCalls(Prog* prog); //TODO: not used, nor defined

        std::vector<PBB> m_vectorBB;

        //! return a bb given an address
        PBB             bbForAddr(ADDRESS addr) { return m_mapBB[addr]; }
        void            simplify();
        void            undoComputedBB(Statement* stmt);

private:

        PBB             splitBB (PBB pBB, ADDRESS uNativeAddr, PBB pNewBB = 0, bool bDelRtls = false);
        void            completeMerge(PBB pb1, PBB pb2, bool bDelete = false);
        bool            checkEntryBB();

public:

        PBB             splitForBranch(PBB pBB, RTL* rtl, BranchStatement* br1, BranchStatement* br2, BB_IT& it);

        /////////////////////////////////////////////////////////////////////////
        // Control flow analysis stuff, lifted from Doug Simon's honours thesis.
        /////////////////////////////////////////////////////////////////////////
        void            setTimeStamps();
        PBB             commonPDom(PBB curImmPDom, PBB succImmPDom);
        void            findImmedPDom();
        void            structConds();
        void            structLoops();
        void            checkConds();
        void            determineLoopType(PBB header, bool* &loopNodes);
        void            findLoopFollow(PBB header, bool* &loopNodes);
        void            tagNodesInLoop(PBB header, bool* &loopNodes);

        void            removeUnneededLabels(HLLCode *hll);
        void            generateDotFile(std::ofstream& of);


        /////////////////////////////////////////////////////////////////////////
        // Get the entry-point or exit BB
        /////////////////////////////////////////////////////////////////////////
        PBB             getEntryBB() { return entryBB;}
        PBB             getExitBB()     { return exitBB;}

        /////////////////////////////////////////////////////////////////////////
        // Set the entry-point BB (and exit BB as well)
        /////////////////////////////////////////////////////////////////////////
        void            setEntryBB(PBB bb);
        void            setExitBB(PBB bb);

        PBB             findRetNode();
        void            addNewOutEdge(PBB fromBB, PBB newOutEdge);
        /////////////////////////////////////////////////////////////////////////
        // print this cfg, mainly for debugging
        /////////////////////////////////////////////////////////////////////////
        void            print(std::ostream &out, bool html = false);
        void            printToLog();
        void            dump();                // Dump to std::cerr
        void            dumpImplicitMap();    // Dump the implicit map to std::cerr
        bool            decodeIndirectJmp(UserProc* proc);

        /////////////////////////////////////////////////////////////////////////
        // Implicit assignments
        /////////////////////////////////////////////////////////////////////////
        Statement *     findImplicitAssign(Exp* x);
        Statement *     findTheImplicitAssign(Exp* x);
        Statement *     findImplicitParamAssign(Parameter* p);
        void            removeImplicitAssign(Exp* x);
        bool            implicitsDone() {return bImplicitsDone;}  //!<  True if implicits have been created
        void            setImplicitsDone() { bImplicitsDone = true; } //!< Call when implicits have been created
        void            findInterferences(ConnectionGraph& ig);
        void            appendBBs(std::list<PBB>& worklist, std::set<PBB>& workset);
        void            removeUsedGlobals(std::set<Global*> &unusedGlobals);
protected:
        void            addBB(BasicBlock * bb) { m_listBB.push_back(bb); }
        friend class XMLProgParser;
};                /* Cfg */

#endif
