/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*==============================================================================
 * FILE:        prog.cc
 * OVERVIEW:    Implementation of the program class. Holds information of
 *              interest to the whole program.
 *============================================================================*/

/*
 * $Revision$
 *
 * 18 Apr 02 - Mike: Mods for boomerang
 * 26 Apr 02 - Mike: common.hs read relative to BOOMDIR
 */

#ifndef BOOMDIR
#ifndef WIN32
#error BOOMDIR needs to be set
#endif
#endif

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200 
#pragma warning(disable:4786)
#endif 

#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "types.h"
#include "statement.h"
#include "exp.h"
#include "cfg.h"
#include "proc.h"
#include "util.h"                   // For str()
#include "register.h"
#include "rtl.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "prog.h"
#include "signature.h"
#include "analysis.h"
#include "boomerang.h"
#include "dom.h"

Prog::Prog()
    : interProcDFAphase(0),
      pBF(NULL),
      pFE(NULL),
      globalMap(NULL),
      m_watcher(NULL),  // First numbered proc will be 1, no initial watcher
      m_iNumberedProc(1) {
    // Default constructor
}

Prog::Prog(BinaryFile *pBF, FrontEnd *pFE)
    : interProcDFAphase(0),
      pBF(pBF),
      pFE(pFE),
      globalMap(NULL),
      m_watcher(NULL),  // First numbered proc will be 1, no initial watcher
      m_iNumberedProc(1) {
}

Prog::Prog(const char* name)
    : interProcDFAphase(0),
      pBF(NULL),
      pFE(NULL),
      globalMap(NULL),
      m_name(name),
      m_watcher(NULL),  // First numbered proc will be 1, no initial watcher
      m_iNumberedProc(1) {
    // Constructor taking a name. Technically, the allocation of the
    // space for the name could fail, but this is unlikely
}

Prog::~Prog() {
    if (pBF) delete pBF;
    if (pFE) delete pFE;
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        if (*it)
            delete *it;
    }
    m_procs.clear();
}

void Prog::setName (const char *name) {    // Assign a name to this program
    m_name = name;
}

char* Prog::getName() {
    return (char*) m_name.c_str();
}

// well form the entire program
bool Prog::wellForm() {
    bool wellformed = true;

    for (std::list<Proc *>::iterator it = m_procs.begin(); it != m_procs.end();
      it++)
        if (!(*it)->isLib()) {
            UserProc *u = (UserProc*)*it;
            wellformed &= u->getCFG()->wellFormCfg();
        }
    return wellformed;
}

// Analyse any procedures that are decoded
void Prog::analyse() {
    Analysis *analysis = new Analysis();
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        Proc *pProc = *it;
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;

        // decoded userproc.. analyse it
        analysis->analyse(p);
    }
    delete analysis;
}

// Globally initialise all statements
void Prog::initStatements() {
    int stmtNumber = 0;
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        Proc *pProc = *it;
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;

        // Sort by address, so the statement numbers will be sensible
        p->getCFG()->sortByAddress();
        // Initialise (set BB, proc, give lib calls parameters, etc) the
        // statements of this proc
        p->initStatements();
        // Also number them
        p->numberStatements(stmtNumber);
    }
}

void Prog::toSSAform() {
    if (Boomerang::get()->debugPrintSSA)
        std::cerr << "====== Debug Print SSA Form (no propagations) ======\n";
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        Proc *pProc = *it;
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;

        // Put this proc into implicit SSA form
        // For now, memory depth 0
        StatementSet empty;
        p->toSSAform(0, empty);
        if (Boomerang::get()->debugPrintSSA)
            p->print(std::cerr, true);
    }
    if (Boomerang::get()->debugPrintSSA)
        std::cerr << "====== End Debug Print SSA Form ======\n\n";
}

// Do decompilation
void Prog::decompile_issa() {
    initStatements();
    forwardGlobalDataflow();

    if (Boomerang::get()->debugPrintReach) {
        std::cerr << "====== Debug Print Reaching and Available Definitions "
                     "======\n";
        PROGMAP::iterator pp;
        std::list<PBB> workList;            // List of BBs still to be processed
        // Set of the same; used for quick membership test
        std::set<PBB> workSet;
        for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
            UserProc* proc = (UserProc*)pp->second;
            if (proc->isLib()) continue;
            Cfg* cfg = proc->getCFG();
            cfg->appendBBs(workList, workSet);
        }
        StatementSet ss;
        while (workList.size()) {
            PBB currBB = workList.front();
            workList.erase(workList.begin());
            currBB->print(std::cerr);
            std::cerr << "reach out: ";
            currBB->getReachOut().printNums(std::cerr); std::cerr << "\n";
            std::cerr << "avail out: ";
            currBB->getAvailOut().printNums(std::cerr); std::cerr << "\n\n";
        }
        std::cerr << "===== End debug reaching and available definitions print "
                     "=====\n\n";
    }

    toSSAform();

    // What used to be done in UserProc::decompile
    decompileProcs();

    if (Boomerang::get()->dotFile) {
        // Note: relies on interprocedural edges
        // Also, we don't want statements deleted (because we like to use
        // statement numbers in the graph)
        std::cerr << "generating dot file..." << std::endl;
        generateDotFile();
    }

    // Find the "restore set". This is the list of definitions that are
    // restored after being saved (popped after a push, loaded after a store,
    // etc)
    StatementSet restoreSet;
    findRestoreSet(restoreSet);
    removeRestoreRefs(restoreSet);
    if (VERBOSE) {
        std::cerr << "Restore set: "; restoreSet.printNums(std::cerr);
        std::cerr << "\n";
        restoreSet.prints();
        std::cerr << "\n";
        std::cerr << "=== After repair, following find restore set ===\n";
        print(std::cerr, true);
        std::cerr << "=== End after repair, following find restore set ===\n";
    }
 
    removeNullStmts();          // Remove null statements
    // Put unused statements before recovery of parameters, so we don't
    // get excess parameters. Arguments of calls should be used in the callee.
    removeUnusedStmts();        // Remove unused statements
    // Remove the restore statements
    removeRestoreStmts(restoreSet);
    restoreSet.clear();

    // Perform the first reverse global dataflow calculation. This is needed
    // to recover parameters and return locations
    reverseGlobalDataflow();
    recoverParameters();
    insertArguments(restoreSet);
    recoverReturnLocs();
    //repairDataflow(restoreSet);   // Repair the dataflow, to use arguments etc
    forwardGlobalDataflow();        // Repair the dataflow, to use arguments etc
    if (VERBOSE) {
        std::cerr << "==== After post argument and return value dataflow repair"
          "====\n";
        print(std::cerr, true);
        std::cerr << "==== End after post argument and return value dataflow "
            "repair ====\n";
    }

    // Convert from SSA to non-SSA form. First perform another reverse global
    // dataflow
    reverseGlobalDataflow();
    fromSSAform();

    // Remove interprocedural edges for structuring algorithms
    removeInterprocEdges();
}

void Prog::generateDotFile() {
    assert(Boomerang::get()->dotFile);
    std::ofstream of(Boomerang::get()->dotFile);
    of << "digraph Cfg {" << std::endl;

    // For now, add all the interprocedural edges
    // It is assumed that the call interprocedural edges are still active
    // from phase 2 of the global DFA
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        Proc *pProc = *it;
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;
        p->getCFG()->setReturnInterprocEdges();
    }

    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        Proc *pProc = *it;
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;
        // Subgraph for the proc name
        of << "\nsubgraph cluster_" << p->getName() << " {\n" <<
          "    color=gray;\n    label=" << p->getName() << ";\n";
        // Generate dotty CFG for this proc
        p->getCFG()->generateDotFile(of);
    }
    of << "}";
    of.close();

    // Remove the return interprocedural edges (in case the dataflow is redone)
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        Proc *pProc = *it;
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;
        p->getCFG()->clearReturnInterprocEdges();
    }

}

void Prog::generateCode(std::ostream &os) {
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        Proc *pProc = *it;
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;
        HLLCode *code = Boomerang::getHLLCode(p);
        p->generateCode(code);
        code->print(os);
        delete code;
    }
}

// Print this program, mainly for debugging
void Prog::print(std::ostream &out, bool withDF) {
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        Proc *pProc = *it;
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;

        // decoded userproc.. print it
        p->print(out, withDF);
    }
}

void Prog::deserialize(std::istream &inf) {
    int fid;
    int len;
    int nProcs, cProcs = 0;
    loadValue(inf, nProcs, false);

    while ((fid = loadFID(inf)) != -1) {
        switch (fid) {
//            case FID_PROJECT_NAME:
//                loadString(inf, project);
//                break;
//            case FID_FILENAME:
//                loadString(inf, filename);
//                break;
         case FID_FRONTEND:
                {
                    len = loadLen(inf);
                    std::streampos pos = inf.tellg();

                    //loadValue(inf, limitTextLow, false);
                    //loadValue(inf, limitTextHigh, false);
                    //loadValue(inf, textDelta, false);

                    std::string frontend;
                    loadString(inf, frontend);
                    pFE = FrontEnd::createById(frontend, pBF);
                    assert(pFE);

                    assert((int)(inf.tellg() - pos) == len);
                }
                break;
            case FID_PROC:
                {
                    len = loadLen(inf);
                    std::streampos pos = inf.tellg();
                    Proc *pProc = Proc::deserialize(this, inf);
                    assert((int)(inf.tellg() - pos) == len);
                    assert(pProc);
                    m_procs.push_back(pProc);   // Append this to list of procs
                    m_procLabels[pProc->getNativeAddress()] = pProc;
                    // alert the watcher of a new proc
                    if (m_watcher) m_watcher->alert_new(pProc);
                    cProcs++;
                }
                break;
            default:
                skipFID(inf, fid);
        }

        if (m_watcher) {
            m_watcher->alert_progress(cProcs, nProcs);
        }
    }
}

bool Prog::serialize(std::ostream &ouf, int &len) {
    int fid;
    //std::streampos st = ouf.tellp();

    int nProcs = 0, cProcs = 0;
    for (std::list<Proc *>::iterator it = m_procs.begin(); it != m_procs.end();
      it++)
        nProcs++;
    saveValue(ouf, nProcs, false);

    // write information about Prog    
//    saveFID(ouf, FID_PROJECT_NAME);
//    saveString(ouf, project);
//    saveFID(ouf, FID_FILENAME);
//    saveString(ouf, filename);

    // write frontend like info
    {
        saveFID(ouf, FID_FRONTEND);

        std::streampos pos = ouf.tellp();
        int len = -1;
        saveLen(ouf, -1, true);
        std::streampos posa = ouf.tellp();

        //saveValue(ouf, limitTextLow, false);
        //saveValue(ouf, limitTextHigh, false);
        //saveValue(ouf, textDelta, false);

        std::string frontend(pFE->getFrontEndId());
        saveString(ouf, frontend);

        std::streampos now = ouf.tellp();
        len = now - posa;
        ouf.seekp(pos);
        saveLen(ouf, len, true);
        ouf.seekp(now);
    }

    // write information about each proc
    for (
#ifndef WIN32
      std::list<Proc *>::iterator 
#endif
      it = m_procs.begin(); it != m_procs.end(); it++) {
        Proc *p = *it;

        fid = FID_PROC;
        saveFID(ouf, fid);

        std::streampos pos = ouf.tellp();
        int len = -1;
        saveLen(ouf, -1, true);
        std::streampos posa = ouf.tellp();

        assert(p->serialize(ouf, len));

        std::streampos now = ouf.tellp();
        assert((int)(now - posa) == len);
        ouf.seekp(pos);
        saveLen(ouf, len, true);
        ouf.seekp(now);
        cProcs++;

        if (m_watcher) {
            m_watcher->alert_progress(cProcs, nProcs);
        }
    }

    // ouf.close();     // Don't close streams, only files or file streams
    return true;
}


// clear the current project
void Prog::clear() {   
    m_name = std::string("");
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++)
        if (*it)
            delete *it;
    m_procs.clear();
    m_procLabels.clear();
    if (pBF)
        delete pBF;
    pBF = NULL;
    if (pFE)
        delete pFE;
    pFE = NULL;
}

/*==============================================================================
 * FUNCTION:    Prog::setNewProc
 * NOTE:        Formally Frontend::newProc
 * OVERVIEW:    Call this function when a procedure is discovered (usually by
 *                decoding a call instruction). That way, it is given a name
 *                that can be displayed in the dot file, etc. If we assign it
 *                a number now, then it will retain this number always
 * PARAMETERS:  prog  - program to add the new procedure to
 *              uAddr - Native address of the procedure entry point
 * RETURNS:     Pointer to the Proc object, or 0 if this is a deleted (not to
 *                be decoded) address
 *============================================================================*/
Proc* Prog::setNewProc(ADDRESS uAddr) {
    // this test fails when decoding sparc, why?  Please investigate - trent
    //assert(uAddr >= limitTextLow && uAddr < limitTextHigh);
    // Check if we already have this proc
    Proc* pProc = findProc(uAddr);
    if (pProc == (Proc*)-1)         // Already decoded and deleted?
        return 0;                   // Yes, exit with 0
    if (pProc)
        // Yes, we are done
        return pProc;
    char* pName = pBF->SymbolByAddress(uAddr);
    bool bLib = pBF->IsDynamicLinkedProc(uAddr);
    if (pName == 0) {
        // No name. Give it a numbered name
        std::ostringstream ost;
        ost << "proc" << m_iNumberedProc++;
        pName = strdup(ost.str().c_str());
    }
    pProc = newProc(pName, uAddr, bLib);
    return pProc;
}


/*==============================================================================
 * FUNCTION:       Prog::newProc
 * OVERVIEW:       Creates a new Proc object, adds it to the list of procs in
 *                  this Prog object, and adds the address to the list
 * PARAMETERS:     name: Name for the proc
 *                 uNative: Native address of the entry point of the proc
 *                 bLib: If true, this will be a libProc; else a UserProc
 * RETURNS:        A pointer to the new Proc object
 *============================================================================*/
Proc* Prog::newProc (const char* name, ADDRESS uNative, bool bLib /*= false*/) {
    Proc* pProc;
    std::string sname(name);
    if (bLib)
        pProc = new LibProc(this, sname, uNative);
    else
        pProc = new UserProc(this, sname, uNative);
    m_procs.push_back(pProc);       // Append this to list of procs
    m_procLabels[uNative] = pProc;
    // alert the watcher of a new proc
    if (m_watcher) m_watcher->alert_new(pProc);
    return pProc;
}

/*==============================================================================
 * FUNCTION:       Prog::remProc
 * OVERVIEW:       Removes the UserProc from this Prog object's list, and
 *                  deletes as much as possible of the Proc (Cfg, RTLists, etc)
 * PARAMETERS:     proc: pointer to the UserProc object to be removed
 * RETURNS:        <nothing>
 *============================================================================*/
void Prog::remProc(UserProc* uProc) {
    // Delete the cfg etc.
    uProc->deleteCFG();

    // Replace the entry in the procedure map with -1 as a warning not to
    // decode that address ever again
    m_procLabels[uProc->getNativeAddress()] = (Proc*)-1;

    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++)
        if (*it == uProc) {
            m_procs.erase(it);
            break;
        }

    // Delete the UserProc object as well
    delete uProc;
}

/*==============================================================================
 * FUNCTION:    Prog::getNumProcs
 * OVERVIEW:    Return the number of real (non deleted) procedures
 * PARAMETERS:  None
 * RETURNS:     The number of procedures
 *============================================================================*/
int Prog::getNumProcs() {
    return m_procs.size();
}


/*==============================================================================
 * FUNCTION:    Prog::getProc
 * OVERVIEW:    Return a pointer to the indexed Proc object
 * PARAMETERS:  Index of the proc
 * RETURNS:     Pointer to the Proc object, or 0 if index invalid
 *============================================================================*/
Proc* Prog::getProc(int idx) const {
    // Return the indexed procedure. If this is used often, we should use
    // a vector instead of a list
    // If index is invalid, result will be 0
    if ((idx < 0) || (idx >= (int)m_procs.size())) return 0;
    std::list<Proc*>::const_iterator it;
    it = m_procs.begin();
    for (int i=0; i < idx; i++)
        it++;
    return (*it);
}


/*==============================================================================
 * FUNCTION:    Prog::findProc
 * OVERVIEW:    Return a pointer to the associated Proc object, or 0 if none
 * NOTE:        Could return -1 for a deleted Proc
 * PARAMETERS:  Native address of the procedure entry point
 * RETURNS:     Pointer to the Proc object, or 0 if none, or -1 if deleted
 *============================================================================*/
Proc* Prog::findProc(ADDRESS uAddr) const {
    PROGMAP::const_iterator it;
    it = m_procLabels.find(uAddr);
    if (it == m_procLabels.end())
        return 0;
    else
        return (*it).second;
}

Proc* Prog::findProc(const char *name) const {   
    std::list<Proc *>::const_iterator it;
    for (it = m_procs.begin(); it != m_procs.end(); it++)
        if (!strcmp((*it)->getName(), name))
            return *it;
    return NULL;
}

// get a library procedure by name
LibProc *Prog::getLibraryProc(const char *nam) {
    Proc *p = findProc(nam);
    if (p && p->isLib())
        return (LibProc*)p;
    return (LibProc*)newProc(nam, NO_ADDRESS, true);
}

Signature* Prog::getLibSignature(const char *nam) {
    return pFE->getLibSignature(nam);
}

const char *Prog::getFrontEndId() {
    return pFE->getFrontEndId();
}

bool Prog::isWin32() {
    return pFE->isWin32();
}

const char *Prog::getGlobal(ADDRESS uaddr)
{
    return pBF->SymbolByAddress(uaddr);
}

void Prog::makeGlobal(ADDRESS uaddr, const char *name)
{
/*    if (globalMap == NULL) globalMap = pBF->GetDynamicGlobalMap();
    assert(globalMap && globalMap->find(uaddr) == globalMap->end());
    (*globalMap)[uaddr] = strdup(name);*/
}

// get a string constant at a given address if appropriate
char *Prog::getStringConstant(ADDRESS uaddr) {
    SectionInfo* si = pBF->GetSectionInfoByAddr(uaddr);
    if (si)
        return (char*)(uaddr + si->uHostAddr - si->uNativeAddr);
    return NULL;
}

/*==============================================================================
 * FUNCTION:    Prog::findContainingProc
 * OVERVIEW:    Return a pointer to the Proc object containing uAddr, or 0 if none
 * NOTE:        Could return -1 for a deleted Proc
 * PARAMETERS:  Native address to search for
 * RETURNS:     Pointer to the Proc object, or 0 if none, or -1 if deleted
 *============================================================================*/
Proc* Prog::findContainingProc(ADDRESS uAddr) const {
    for (std::list<Proc*>::const_iterator it = m_procs.begin();
      it != m_procs.end(); it++) {
        Proc *p = (*it);
        if (p->getNativeAddress() == uAddr)
            return p;
        if (p->isLib()) continue;

        UserProc *u = (UserProc *)p;
        if (u->containsAddr(uAddr))
            return p;
    }
    return NULL;
}

/*==============================================================================
 * FUNCTION:    Prog::isProcLabel
 * OVERVIEW:    Return true if this is a real procedure
 * PARAMETERS:  Native address of the procedure entry point
 * RETURNS:     True if a real (non deleted) proc
 *============================================================================*/
bool Prog::isProcLabel (ADDRESS addr) {
    if (m_procLabels[addr] == 0)
        return false;
    return true;
}

/*==============================================================================
 * FUNCTION:    Prog::getNameNoPath
 * OVERVIEW:    Get the name for the progam, without any path at the front
 * PARAMETERS:  None
 * RETURNS:     A string with the name
 *============================================================================*/
std::string Prog::getNameNoPath() const {
    unsigned n = m_name.rfind("/");
    if (n == std::string::npos) {
        return m_name;
    }

    return m_name.substr(n+1);
}

/*==============================================================================
 * FUNCTION:    Prog::getFirstProc
 * OVERVIEW:    Return a pointer to the first Proc object for this program
 * NOTE:        The it parameter must be passed to getNextProc
 * PARAMETERS:  it: An uninitialised PROGMAP::const_iterator
 * RETURNS:     A pointer to the first Proc object; could be 0 if none
 *============================================================================*/
Proc* Prog::getFirstProc(PROGMAP::const_iterator& it) {
    it = m_procLabels.begin();
    while (it != m_procLabels.end() && (it->second == (Proc*) -1))
        it++;
    if (it == m_procLabels.end())
        return 0;
    return it->second;
}

/*==============================================================================
 * FUNCTION:    Prog::getNextProc
 * OVERVIEW:    Return a pointer to the next Proc object for this program
 * NOTE:        The it parameter must be from a previous call to getFirstProc
 *                or getNextProc
 * PARAMETERS:  it: A PROGMAP::const_iterator as above
 * RETURNS:     A pointer to the next Proc object; could be 0 if no more
 *============================================================================*/
Proc* Prog::getNextProc(PROGMAP::const_iterator& it) {
    it++;
    while (it != m_procLabels.end() && (it->second == (Proc*) -1))
        it++;
    if (it == m_procLabels.end())
        return 0;
    return it->second;
}

/*==============================================================================
 * FUNCTION:    Prog::getFirstUserProc
 * OVERVIEW:    Return a pointer to the first UserProc object for this program
 * NOTE:        The it parameter must be passed to getNextUserProc
 * PARAMETERS:  it: An uninitialised std::list<Proc*>::iterator
 * RETURNS:     A pointer to the first UserProc object; could be 0 if none
 *============================================================================*/
UserProc* Prog::getFirstUserProc(std::list<Proc*>::iterator& it) {
    it = m_procs.begin();
    while (it != m_procs.end() && (*it)->isLib())
        it++;
    if (it == m_procs.end())
        return 0;
    return (UserProc*)*it;
}

/*==============================================================================
 * FUNCTION:    Prog::getNextUserProc
 * OVERVIEW:    Return a pointer to the next UserProc object for this program
 * NOTE:        The it parameter must be from a previous call to
 *                getFirstUserProc or getNextUserProc
 * PARAMETERS:  it: A std::list<Proc*>::iterator
 * RETURNS:     A pointer to the next UserProc object; could be 0 if no more
 *============================================================================*/
UserProc* Prog::getNextUserProc(std::list<Proc*>::iterator& it) {
    it++;
    while (it != m_procs.end() && (*it)->isLib())
        it++;
    if (it == m_procs.end())
        return 0;
    return (UserProc*)*it;
}

/*==============================================================================
 * FUNCTION:    getCodeInfo
 * OVERVIEW:    Lookup the given native address in the code section, returning
 *                a host pointer corresponding to the same address
 * PARAMETERS:  uNative: Native address of the candidate string or constant
 *              last: will be set to one past end of the code section (host)
 *              delta: will be set to the difference between the host and
 *                native addresses
 * RETURNS:     Host pointer if in range; NULL if not
 *              Also sets 2 reference parameters (see above)
 *============================================================================*/
const void* Prog::getCodeInfo(ADDRESS uAddr, const char*& last, int& delta) {
    delta=0;
    last=0;
#ifdef WIN32
    // this is broken obviously
    return NULL;
#else
    int n = pBF->GetNumSections();
    int i;
    // Search all code and read-only sections
    for (i=0; i < n; i++) {
        SectionInfo* pSect = pBF->GetSectionInfo(i);
        if ((!pSect->bCode) && (!pSect->bReadOnly))
            continue;
        if ((uAddr < pSect->uNativeAddr) ||
          (uAddr >= pSect->uNativeAddr + pSect->uSectionSize))
            continue;           // Try the next section
        delta = pSect->uHostAddr - pSect->uNativeAddr;
        last = (const char*) (pSect->uHostAddr + pSect->uSectionSize);
        const char* p = (const char *) (uAddr + delta);
        return p;
    }
    return NULL;
#endif
}

void updateWorkListFwd(PBB currBB, std::list<PBB>&workList,
  std::set<PBB>& workSet) {
    // Insert outedges of currBB into the worklist, unless already there
    std::vector<PBB>& outs = currBB->getOutEdges();
    int n = outs.size();
    for (int i=0; i < n; i++) {
        PBB currOut = outs[i];
        if (workSet.find(currOut) != workSet.end()) {
            workList.push_front(currOut);
            workSet.insert(currOut);
        }
    }
}

void updateWorkListRev(PBB currBB, std::list<PBB>&workList,
  std::set<PBB>& workSet) {
    // Insert inedges of currBB into the worklist, unless already there
    std::vector<PBB>& ins = currBB->getInEdges();
    int n = ins.size();
    for (int i=0; i < n; i++) {
        PBB currIn = ins[i];
        if (workSet.find(currIn) != workSet.end()) {
            workList.push_front(currIn);
            workSet.insert(currIn);
        }
    }
}

// Calculate forwards global dataflow. The whole program is treated as
// one large dataflow problem
// This is done only once (as a global dataflow problem); dataflow might be
// recalculated repeatedly one proc at a time as substitutions are made
void Prog::forwardGlobalDataflow() {
    PROGMAP::iterator pp;
    std::list<PBB> workList;            // List of BBs still to be processed
    // Set of the same; used for quick membership test
    std::set<PBB> workSet; 
    // Sort the BBs into approximatly preorder
    // Note: the ideal order differs for phase 1 and 2
    // This order should be ideal for phase 2, and so-so for phase 1
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        Cfg* cfg = proc->getCFG();
        cfg->establishDFTOrder();
        cfg->sortByFirstDFT();
        // Insert all BBs into the worklist and workset. For many programs,
        // it would be enough to insert the first BB of main, but some may
        // have BBs that don't have explicit in-edges
        cfg->appendBBs(workList, workSet);
    }

    // Set up for phase 1
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        Cfg* cfg = proc->getCFG();
        cfg->setReturnInterprocEdges();
        // Clear the dataflow info for this proc's cfg
        cfg->clearReaches();
        cfg->clearAvailable();
    }
    bool change;
    // Phase 1
    if (Boomerang::get()->debugDataflow)
        std::cerr << "Global dataflow phase 1\n";
    while (workList.size()) {
        PBB currBB = workList.front();
        workList.erase(workList.begin());
        workSet.erase(currBB);
        change  = currBB->calcReaches(1);   // Reaching definitions
        change |= currBB->calcAvailable(1); // Available definitions
        if (change) updateWorkListFwd(currBB, workList, workSet);
    };

    // Set up for Phase 2
    workSet.clear();            // Should be clear already
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        Cfg* cfg = proc->getCFG();
        // Save reaching and available defs from phase 1
        cfg->saveForwardFlow(proc);
        // Clear the dataflow info for this proc's cfg
        // Note: leave available definitions alone; won't recalc this phase
        proc->getCFG()->clearReaches();
        // Clear the return interprocedural edges
        cfg->clearReturnInterprocEdges();
        // Set the call interprocedural edges
        cfg->setCallInterprocEdges();
        cfg->appendBBs(workList, workSet);
    }
    // Phase 2
    if (Boomerang::get()->debugDataflow)
        std::cerr << "Global dataflow phase 2\n";
    while (workList.size()) {
        PBB currBB = workList.front();
        workList.erase(workList.begin());
        workSet.erase(currBB);
        change = currBB->calcReaches(2);   // Reaching definitions
        // Don't need available definitions in phase 2
        if (change) updateWorkListFwd(currBB, workList, workSet);
    };
    if (Boomerang::get()->debugDataflow)
        std::cerr << "Global dataflow complete\n";

    // Reset to address order (so prints are easier to read)
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        Cfg* cfg = proc->getCFG();
        cfg->sortByAddress();
    }
}

// Calculate reverse global dataflow. The whole program is treated as
// one large dataflow problem
void Prog::reverseGlobalDataflow() {
    PROGMAP::iterator pp;
    std::list<PBB> workList;            // List of BBs still to be processed
    // Set of the same; used for quick membership test
    std::set<PBB> workSet; 
    // Sort the BBs into approximatly postorder
    // Note: the ideal order differs for phase 1 and 2
    // This order should be ideal for phase 2, and so-so for phase 1
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        Cfg* cfg = proc->getCFG();
        cfg->establishDFTOrder();
        cfg->sortByLastDFT();
        // Insert all BBs into the worklist and workset. For many programs,
        // it would be enough to insert the first BB of main, but some may
        // have BBs that don't have explicit in-edges
        cfg->appendBBs(workList, workSet);
    }

    // Set up for phase 1
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        Cfg* cfg = proc->getCFG();
        cfg->clearReturnInterprocEdges();
        cfg->setCallInterprocEdges();
        // Clear the dataflow info for this proc's cfg
        cfg->clearLiveness();
        cfg->clearDeadness();
    }
    bool change;
    // Phase 1
    while (workList.size()) {
        PBB currBB = workList.back();
        workList.erase(--workList.end());
        workSet.erase(currBB);
        change  = currBB->calcLiveness(1);  // Live locations
        change |= currBB->calcDeadness(1);  // Dead locations
        if (change) updateWorkListRev(currBB, workList, workSet);
    };

    // Set up for Phase 2
    workSet.clear();            // Should be clear already
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        Cfg* cfg = proc->getCFG();
        // Save live and dead locations from phase 1
        cfg->saveReverseFlow(proc);
        // Clear the dataflow info for this proc's cfg
        // Note: leave dead locations alone; won't recalc this phase
        proc->getCFG()->clearLiveness();
        // Clear the call interprocedural edges
        cfg->clearCallInterprocEdges();
        // Set the return interprocedural edges
        cfg->setReturnInterprocEdges();
        cfg->appendBBs(workList, workSet);
    }
    // Phase 2
    while (workList.size()) {
        PBB currBB = workList.back();
        workList.erase(--workList.end());
        workSet.erase(currBB);
        change = currBB->calcLiveness(2);   // Live locations
        // Don't need available definitions in phase 2
        if (change) updateWorkListRev(currBB, workList, workSet);
    }
}

void Prog::recoverParameters() {
    PROGMAP::iterator pp;
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        proc->recoverParameters();
    }
}

void Prog::insertArguments(StatementSet& rs) {
    PROGMAP::iterator pp;
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        proc->insertArguments(rs);
    }
}


void Prog::fromSSAform() {
    // Calculate the interference graph
    igraph ig;
    PROGMAP::iterator pp;
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        Cfg* cfg = proc->getCFG();
        cfg->calcLiveness(ig);
    }

    if (Boomerang::get()->vFlag) {
        std::cerr << "==== Start interference graph ===\n";
        igraph::iterator xx;
        for (xx=ig.begin(); xx != ig.end(); xx++)
            std::cerr << (*xx).first << " = " << (*xx).second << "\n";
        std::cerr << "==== End interference graph ===\n\n";
    }


    // Take out of SSA mode
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        proc->fromSSAform(ig);
    }

    if (Boomerang::get()->vFlag) {
        std::cerr << "===== After transformation from SSA form =====\n";
        for (std::list<Proc*>::iterator it = m_procs.begin();
              it != m_procs.end(); it++) {
            Proc *pProc = *it;
            if (pProc->isLib()) continue;
            UserProc *p = (UserProc*)pProc;
            p->print(std::cerr, true);
        }
        std::cerr << "===== End after transformation from SSA =====\n\n";
    }
}

// Note: assumes that the last GDFA was a backward flow one
void Prog::removeInterprocEdges() {
    for (std::list<Proc*>::iterator it = m_procs.begin();
          it != m_procs.end(); it++) {
        UserProc *proc = (UserProc*)*it;
        if (proc->isLib()) continue;
        Cfg* cfg = proc->getCFG();
        // Clear the return interprocedural edges
        cfg->clearReturnInterprocEdges();
    }
}

/*==============================================================================
 * FUNCTION:    Prog::decompileProcs
 * OVERVIEW:    Do all the things that UserProc::decompile used to do, except
 *                at a global level. This is needed for programs as simple as
 *                twoproc; propagations in main are needed into proc1 and so on.
 * PARAMETERS:  None
 * RETURNS:     Nothing
 *============================================================================*/
void Prog::decompileProcs() {
    // First calculate the vector of proc memory depths
    std::vector<int> depths(getNumProcs()); // Initialise to known correct size
    std::list<Proc*>::iterator pp;
    UserProc* proc;
    std::list<PBB> allBBs;
    int i=0;
    int maxMaxDepth = 0;                // Max of all proc's max depths
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        int depth = proc->findMaxDepth();
        depths[i++] = depth;
        if (depth > maxMaxDepth)
            maxMaxDepth = depth;
    }

    int depth = 0;
    while (true) {
        i=0;
        // First propagate for all procs at this level
        for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
            proc = (UserProc*)(*pp);
            if (proc->isLib()) continue;
            if (depth > depths[i]) continue;
            proc->propagateStatements(depth);
        }
        if (VERBOSE) {
            std::cerr << "===== After propagate at memory depth " <<
              depth << " =====\n";
            for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
                proc = (UserProc*)(*pp);
                if (proc->isLib()) continue;
                proc->print(std::cerr, true);
            }
            std::cerr << "===== End propagate at memory depth " <<
              depth << " =====\n\n";
        }

        // Move to the next memory depth
        depth++;
        if (depth > maxMaxDepth)
            break;

        // Repair the DFA ready for next level of propagation
        i=0;
        for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
            proc = (UserProc*)(*pp);
            if (proc->isLib()) continue;
            if (depth <= depths[i]) {
                StatementSet empty;
                proc->repairDataflow(depth, empty);
            }
        }
        if (VERBOSE) {
            std::cerr << "===== After dataflow repair at memory depth " <<
              depth << " =====\n";
            for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
                proc = (UserProc*)(*pp);
                if (proc->isLib()) continue;
                proc->print(std::cerr, true);
            }
            std::cerr << "===== End after dataflow repair at memory depth " <<
              depth << " =====\n\n";
        }
    }
        
    // Now all the other things that were in UserProc::decompile()
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        proc->complete();
    }
} 

void Prog::removeNullStmts() {
    // Remove null statements
    if (VERBOSE) std::cerr << "Removing null statements\n";
    UserProc* proc;
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        if (!Boomerang::get()->noRemoveNull)
            proc->removeNullStatements();
    }
}

// Remove unused statements
typedef std::map<Statement*, int> RefCounter;
void Prog::removeUnusedStmts() {
    RefCounter refCounts;           // The map
    UserProc* proc;
    std::list<Proc*>::iterator pp;
    // Count the references globally first
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        if (!Boomerang::get()->noRemoveNull)
            proc->countRefs(refCounts);
    }
    // Now remove any that have no used (globally)
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        if (!Boomerang::get()->noRemoveNull)
            proc->removeUnusedStatements(refCounts);
    }

    if (VERBOSE) {
        std::cerr << "===== After removing unused statements "
          "=====\n";
        for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
            proc = (UserProc*)(*pp);
            if (proc->isLib()) continue;
                proc->print(std::cerr, true);
        }
        std::cerr << "===== End after removing unused "
          "statements =====\n\n";
    }
}

void Prog::recoverReturnLocs() {
    // Recover return locations
    UserProc* proc;
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        proc->recoverReturnLocs();
    }
}

// Not currently used
void Prog::processConstants() {
    // Process constants
    if (VERBOSE) std::cerr << "Processing constants\n";
    UserProc* proc;
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        proc->processConstants();
    }
}

void Prog::repairDataflow(StatementSet& rs) {
    // A hell of a hack!!
    if (VERBOSE) std::cerr << "Repairing dataflow\n";
    UserProc* proc;
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        proc->repairDataflow(1, rs);
    }
}

void Prog::findRestoreSet(StatementSet& restoreSet) {
    UserProc* proc;
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        proc->findRestoreSet_issa(restoreSet);
    }
}

void Prog::removeRestoreRefs(StatementSet& restoredSet) {
    UserProc* proc;
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        proc->removeRestoreRefs(restoredSet);
    }

}
void Prog::removeRestoreStmts(StatementSet& restoredSet) {
    // To be completed
}

void Prog::decompile() {
    // For each proc
    UserProc* proc;
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;

        // Sort by address, so printouts make sense
        Cfg* cfg = proc->getCFG();
        cfg->sortByAddress();
        // Initialise statements
        proc->initStatements();

        // Compute dominance frontier
        DOM* d = new DOM;
        cfg->dominators(d);


        // For each memory depth
        int maxDepth = proc->findMaxDepth();
        if (Boomerang::get()->maxMemDepth < maxDepth)
            maxDepth = Boomerang::get()->maxMemDepth;
        for (int depth = 0; depth <= maxDepth; depth++) {

            // Place the phi functions for this memory depth
            cfg->placePhiFunctions(d, depth);

            // Number the statements
            int stmtNumber = 0;
            proc->numberStatements(stmtNumber); 


            // Rename variables
            cfg->renameBlockVars(d, 0, depth);

            // Print if requested
            if (Boomerang::get()->debugPrintSSA) {
                std::cerr << "=== Debug Print SSA for " << proc->getName()
                  << " at memory depth " << depth << " (no propagations) ===\n";
                proc->print(std::cerr, true);
                std::cerr << "=== End Debug Print SSA for " <<
                  proc->getName() << " at depth " << depth << " ===\n\n";
            }

            // Propagate at this memory depth
            proc->propagateStatements(depth);
            if (VERBOSE) {
                std::cerr << "=== After propagate for " << proc->getName() <<
                  " at memory depth " << depth << " ===\n";
                proc->print(std::cerr, true);
                std::cerr << "=== End propagate for " << proc->getName() <<
                  " at depth " << depth << " ===\n\n";
            }
        }

        // Find the "restore set"
        StatementSet restoreSet;
        proc->findRestoreSet(restoreSet);
        if (VERBOSE) {
            std::cerr << "=== Restore set for " << proc->getName() << " ===\n";
            StmtSetIter rr;
            for (Statement* r = restoreSet.getFirst(rr); r;
              r = restoreSet.getNext(rr))
                std::cerr << std::dec << r->getNumber() << " ";
            std::cerr << "\n\n";
        }

        igraph ig;      // FIXME: need to make an attempt to calculate this!
        //proc->fromSSAform(ig);

        if (Boomerang::get()->vFlag) {
            std::cerr << "===== After transformation from SSA form =====\n";
            for (std::list<Proc*>::iterator it = m_procs.begin();
                  it != m_procs.end(); it++) {
                Proc *pProc = *it;
                if (pProc->isLib()) continue;
                UserProc *p = (UserProc*)pProc;
                p->print(std::cerr, true);
            }
            std::cerr << "===== End after transformation from SSA =====\n\n";
        }

        delete d;


    }       // for each proc
}
