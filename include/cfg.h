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

/*==============================================================================
 * Control Flow Graph class. Contains all the BasicBlock objects for a procedure.  These BBs contain all the RTLs for
 * the procedure, so by traversing the Cfg, one traverses the whole procedure.
 *============================================================================*/
class Cfg {
        /*
         * Pointer to the UserProc object that contains this CFG object
         */
        UserProc*    myProc;

        /*
         * The list of pointers to BBs.
         */
        std::list<PBB> m_listBB;

        /*
         * Ordering of BBs for control flow structuring
         */
        std::vector<PBB> Ordering;
        std::vector<PBB> revOrdering;

        /*
         * The ADDRESS to PBB map.
         */
        MAPBB        m_mapBB;

        /*
         * The entry and exit BBs.
         */
        BasicBlock* entryBB;
        BasicBlock* exitBB;

        /*
         * True if well formed.
         */
        bool        m_bWellFormed, structured;

        /*
         * Set of the call instructions in this procedure.
         */
        std::set<CallStatement*> callSites;

        /*
         * Last label (positive integer) used by any BB this Cfg
         */
        int            lastLabel;

        /*
         * Map from expression to implicit assignment. The purpose is to prevent multiple implicit assignments for
         * the same location.
         */

        std::map<Exp*, Statement*, lessExpStar> implicitMap;

        bool            bImplicitsDone;     // True when the implicits are done; they can cause problems (e.g. with
                                            // ad-hoc global assignment)

public:
                        Cfg();
                        ~Cfg();
        void            setProc(UserProc* proc);
        void            clear();
        unsigned        getNumBBs() {return m_listBB.size();} //!<Get the number of BBs

        /*
         * Equality operator.
         */
        const Cfg&      operator=(const Cfg& other); /* Copy constructor */

        class BBAlreadyExistsError : public std::exception {
        public:
            PBB pBB;
            BBAlreadyExistsError(PBB pBB) : pBB(pBB) { }
        };

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
        typedef     BB_IT iterator;
        iterator    begin() { return m_listBB.begin(); }
        iterator    end()   { return m_listBB.end(); }
        bool        label ( ADDRESS uNativeAddr, PBB& pNewBB );
        bool        isIncomplete ( ADDRESS uNativeAddr );
        bool        existsBB ( ADDRESS uNativeAddr );
        void        sortByAddress ();
        void        sortByFirstDFT();
        void        sortByLastDFT();
        void        updateVectorBB();

        bool        wellFormCfg ( );
        bool        mergeBBs ( PBB pb1, PBB pb2 );
        bool        compressCfg ( );
        bool        establishDFTOrder();
        bool        establishRevDFTOrder();

        int         pbbToIndex (PBB pBB);
        void        unTraverse ( );
        bool        isWellFormed ( );
        bool        isOrphan ( ADDRESS uAddr);
        bool        joinBB( PBB pb1, PBB pb2);

        /*
         * Completely remove a BB from the CFG.
         */
        void        removeBB( PBB bb);

        /*
         * Resets the DFA sets of all the BBs.
         */
        void        resetDFASets();

        /*
         * Add a call to the set of calls within this procedure.
         */
        void        addCall(CallStatement* call);

        /*
         * Get the set of calls within this procedure.
         */
        std::set<CallStatement*>& getCalls();

        /*
         * Replace all instances of search with replace.  Can be type sensitive if reqd
         */
        void        searchAndReplace(Exp* search, Exp* replace);
        bool        searchAll(Exp* search, std::list<Exp*> &result);

        /*
         * Set the return value for this CFG (assumes there is only one exit bb)
         */
        void        setReturnVal(Exp *e);
        Exp            *getReturnVal();

        /*
         * Structures the control flow graph
         */
        void        structure();

        /*
         * Add/Remove Junction statements
         */
        void        addJunctionStatements();
        void        removeJunctionStatements();

        /*
         * Resolves goto/branch destinations to statements
         * Good to do this late, as removing statements doesn't update this information.
         */
        void        resolveGotos();

        /*
         * Virtual Function Call analysis
         */
        void        virtualFunctionCalls(Prog* prog);

        std::vector<PBB> m_vectorBB; // faster access

        /* return a bb given an address */
        PBB            bbForAddr(ADDRESS addr) { return m_mapBB[addr]; }

        /* Simplify all the expressions in the CFG
         */
        void        simplify();

        /*
         * Change the BB enclosing stmt to be CALL, not COMPCALL
         */
        void        undoComputedBB(Statement* stmt);

private:

        PBB            splitBB (PBB pBB, ADDRESS uNativeAddr, PBB pNewBB = 0, bool bDelRtls = false);
        void        completeMerge(PBB pb1, PBB pb2, bool bDelete);
        bool        checkEntryBB();

public:
        PBB         splitForBranch(PBB pBB, RTL* rtl, BranchStatement* br1, BranchStatement* br2, BB_IT& it);

        /*
         * Control flow analysis stuff, lifted from Doug Simon's honours thesis.
         */
        void        setTimeStamps();
        PBB         commonPDom(PBB curImmPDom, PBB succImmPDom);
        void        findImmedPDom();
        void        structConds();
        void        structLoops();
        void        checkConds();
        void        determineLoopType(PBB header, bool* &loopNodes);
        void        findLoopFollow(PBB header, bool* &loopNodes);
        void        tagNodesInLoop(PBB header, bool* &loopNodes);

        void        removeUnneededLabels(HLLCode *hll);
        void        generateDotFile(std::ofstream& of);


        /*
         * Get the entry-point or exit BB
         */
        PBB         getEntryBB() { return entryBB;}
        PBB         getExitBB()     { return exitBB;}

        /*
         * Set the entry-point BB (and exit BB as well)
         */
        void        setEntryBB(PBB bb);
        void        setExitBB(PBB bb);

        PBB         findRetNode();
        void        addNewOutEdge(PBB fromBB, PBB newOutEdge);
        /*
         * print this cfg, mainly for debugging
         */
        void        print(std::ostream &out, bool html = false);
        void        printToLog();
        void        dump();                // Dump to std::cerr
        void        dumpImplicitMap();    // Dump the implicit map to std::cerr
        bool        decodeIndirectJmp(UserProc* proc);

        /*
         * Implicit assignments
         */
        Statement * findImplicitAssign(Exp* x);
        Statement * findTheImplicitAssign(Exp* x);         //!< Find the existing implicit assign for x (if any)
        Statement * findImplicitParamAssign(Parameter* p); //!< Find exiting implicit assign for parameter p
        void        removeImplicitAssign(Exp* x);          //!< Remove an existing implicit assignment for x
        bool        implicitsDone()                        //!<  True if implicits have been created
                    {return bImplicitsDone;}
        void        setImplicitsDone() { bImplicitsDone = true; } //!< Call when implicits have been created
        void        findInterferences(ConnectionGraph& ig);
        void        appendBBs(std::list<PBB>& worklist, std::set<PBB>& workset);
        void        removeUsedGlobals(std::set<Global*> &unusedGlobals);

protected:
        void        addBB(BasicBlock * bb) { m_listBB.push_back(bb); }
        friend class XMLProgParser;
};                /* Cfg */

#endif
