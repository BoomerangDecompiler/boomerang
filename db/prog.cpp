/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
 * Copyright (C) 2002-2003, Trent Waddington
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
#include <math.h>

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
#include "ansi-c-parser.h"
#include "config.h"

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
    if (pBF && pBF->getFilename()) 
        m_name = pBF->getFilename();
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
        pProc->printDetailsXML();
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;
        if (p->isAnalysed()) continue;

        p->setAnalysed();

        // decoded userproc.. analyse it
        p->getCFG()->sortByAddress();
        analysis->analyse(p);
        p->printAnalysedXML();
    }
    delete analysis;
}

void Prog::generateDotFile() {
    assert(Boomerang::get()->dotFile);
    std::ofstream of(Boomerang::get()->dotFile);
    of << "digraph Cfg {" << std::endl;

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

}

void Prog::generateCode(std::ostream &os) {
    HLLCode *code = Boomerang::get()->getHLLCode();
    for (std::vector<Global*>::iterator it1 = globals.begin(); 
         it1 != globals.end(); it1++) {
        // Check for an initial value
        Exp *e = NULL;
        e = (*it1)->getInitialValue(this);
        if (e)
            code->AddGlobal((*it1)->getName(), (*it1)->getType(), e);
    }
    code->print(os);
    delete code;
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        Proc *pProc = *it;
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;
        p->getCFG()->compressCfg();
        code = Boomerang::get()->getHLLCode(p);
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

platform Prog::getFrontEndId() {
    return pFE->getFrontEndId();
}

Signature *Prog::getDefaultSignature(const char *name)
{
    return pFE->getDefaultSignature(name);
}

std::vector<Exp*> &Prog::getDefaultParams()
{
    return pFE->getDefaultParams();
}

std::vector<Exp*> &Prog::getDefaultReturns()
{
    return pFE->getDefaultReturns();
}

bool Prog::isWin32() {
    return pFE->isWin32();
}

const char *Prog::getGlobalName(ADDRESS uaddr)
{
    for (unsigned i = 0; i < globals.size(); i++)
        if (globals[i]->getAddress() == uaddr)
            return globals[i]->getName();
        else if (globals[i]->getAddress() < uaddr &&
                 globals[i]->getAddress() + 
                    globals[i]->getType()->getSize() / 8 > uaddr)
            return globals[i]->getName();
    return pBF->SymbolByAddress(uaddr);
}

ADDRESS Prog::getGlobalAddr(char *nam)
{
    for (unsigned i = 0; i < globals.size(); i++)
        if (!strcmp(globals[i]->getName(), nam))
            return globals[i]->getAddress();
    return pBF->GetAddressByName(nam);
}

Global* Prog::getGlobal(char *nam) {
    for (unsigned i = 0; i < globals.size(); i++)
        if (!strcmp(globals[i]->getName(), nam))
            return globals[i];
    return NULL;
}

void Prog::globalUsed(ADDRESS uaddr)
{
    for (unsigned i = 0; i < globals.size(); i++)
        if (globals[i]->getAddress() == uaddr)
            return;
        else if (globals[i]->getAddress() < uaddr &&
                 globals[i]->getAddress() + 
                    globals[i]->getType()->getSize() / 8 > uaddr)
            return;
    if (uaddr < 0x10000) {
        // This happens in windows code because you can pass a low value integer instead 
        // of a string to some functions.
        LOG << "warning: ignoring stupid request for global at address " << uaddr << "\n";
        return;
    }
    const char *nam = newGlobal(uaddr); 
    Type *ty = guessGlobalType(nam, uaddr);
    globals.push_back(new Global(ty, uaddr, nam));
}

Type *Prog::guessGlobalType(const char *nam, ADDRESS u)
{
    int sz = pBF->GetSizeByName(nam);
    if (sz == 0) {
        // Check if it might be a string
        char* str = getStringConstant(u);
        if (str)
            // return char* and hope it is dealt with properly
            return new PointerType(new CharType());
    }
    Type *ty;
    switch(sz) {
        case 1:
        case 2:
        case 4:
        case 8:
            ty = new IntegerType(sz*8);
            break;
        default:
            ty = new ArrayType(new CharType(), sz);
    }
    return ty;
}

const char *Prog::newGlobal(ADDRESS uaddr)
{
    const char *nam = getGlobalName(uaddr);
    if (nam == NULL) {
        std::ostringstream os;
        os << "global" << globals.size();
        nam = strdup(os.str().c_str());
        if (VERBOSE)
            LOG << "adding new global: " << nam << " at address " << uaddr 
                << "\n";
    } 
    return nam;
}

Type *Prog::getGlobalType(char* nam) {
    for (unsigned i = 0; i < globals.size(); i++)
        if (!strcmp(globals[i]->getName(), nam))
            return globals[i]->getType();
    return NULL;
}

void Prog::setGlobalType(const char* nam, Type* ty) {
    for (unsigned i = 0; i < globals.size(); i++)
        if (!strcmp(globals[i]->getName(), nam))
            globals[i]->setType(ty);
}

// get a string constant at a given address if appropriate
// if knownString, it is already known to be a char*
char *Prog::getStringConstant(ADDRESS uaddr, bool knownString /* = false */) {
    SectionInfo* si = pBF->GetSectionInfoByAddr(uaddr);
    // Too many compilers put constants, including string constants, into
    // read/write sections
    //if (si && si->bReadOnly)
    if (si && !si->bBss) {
        // At this stage, only support ascii, null terminated, non unicode
        // strings.
        // At least 4 of the first 6 chars should be printable ascii
        char* p = (char*)(uaddr + si->uHostAddr - si->uNativeAddr);
        if (knownString)
            // No need to guess... this is hopefully a known string
            return p;
        int printable = 0;
        char last = 0;
        for (int i=0; i < 6; i++) {
            char c = p[i];
            if (c == 0) break;
            if (c >= ' ' && c < '\x7F') printable++;
            last = c;
        }
        if (printable >= 4)
            return p;
        // Just a hack while type propagations are not yet ready
        if (last == '\n' && printable >= 2)
            return p;
    }
    return NULL;
}

double Prog::getFloatConstant(ADDRESS uaddr, bool &ok, int bits) {
    ok = true;
    SectionInfo* si = pBF->GetSectionInfoByAddr(uaddr);
    if (si && si->bReadOnly)
        if (bits == 64) {
            return pBF->readNativeFloat8(uaddr);
        } else {
            assert(bits == 32);
            return pBF->readNativeFloat4(uaddr);
        }
    ok = false;
    return 0.0;
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

void Prog::insertArguments(StatementSet& rs) {
    PROGMAP::iterator pp;
    for (pp = m_procLabels.begin(); pp != m_procLabels.end(); pp++) {
        UserProc* proc = (UserProc*)pp->second;
        if (proc->isLib()) continue;
        proc->insertArguments(rs);
    }
}


void Prog::decompile() {
    assert(m_procs.size());

    if (VERBOSE) 
        LOG << "Decompiling " << (int)m_procs.size() << " procedures\n";

    UserProc* entryProc = (UserProc*) m_procs.front();
    if (entryProc && !entryProc->isLib()) {
        if (VERBOSE)
            LOG << "starting with " << entryProc->getName() << "\n";
        entryProc->decompile();
    }

    // Just in case there are any Procs not in the call graph
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        UserProc* proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        proc->decompile();
    }

    if (!Boomerang::get()->noDecompile) {
        if (VERBOSE)
            LOG << "removing unused returns\n";

        // A final pass to remove return locations not used by any caller
        if (!Boomerang::get()->noRemoveReturns) 
            removeUnusedReturns();

        // print XML after removing returns
        for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
            UserProc* proc = (UserProc*)(*pp);
            if (proc->isLib()) continue;
            proc->printXML();
        }
    }

    if (Boomerang::get()->debugTA)
        typeAnalysis();

    if (VERBOSE)
        LOG << "transforming from SSA\n";

    // Now it is OK to transform out of SSA form
    fromSSAform();

    // A final pass to remove unused locals
    // Now in UserProc::generateCode()
    //removeUnusedLocals();
}

void Prog::removeUnusedReturns() {
    // The counter
    UserProc::ReturnCounter rc;
    // Two worksets; one of procs whose return sets have changed, and
    // one of their callers
    std::set<UserProc*> calleeSet, callerSet, newCalleeSet;

    // First count (globally) the used returns
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        UserProc* proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        calleeSet.insert(proc);
        callerSet.insert(proc);
    }

    bool change;
    do {
        rc.clear();
        // Iterate through the workset, looking for uses of returns
        // from calls in these procs (initially, all procs; later, callers
        // of procs whose returns set has been reduced
        // FIXME: the ordering is not consistent; it is affecting
        // whether output is correct or not!! (so worksets not working)
        std::set<UserProc*>::iterator it;
        for (it = callerSet.begin(); it != callerSet.end(); it++)
            (*it)->countUsedReturns(rc);

        // Count a return reference for main. Looking for the name "main" is
        // good, becuase if it's not a C program, then it won't have a main,
        // and it probably (?) won't return an int
        UserProc* m = (UserProc*) findProc("main");
        if (m) {
            // Note: it's position 1, because position 0 is the stack pointer
            // Note: if it's SPARC (or perhaps other architectures), there may
            // be only one
            Signature* sig = m->getSignature();
            Exp* r;
            if (sig->getNumReturns() == 1)
                r = sig->getReturnExp(0);
            else r = sig->getReturnExp(1);
            rc[m].insert(r);
        }

        newCalleeSet.clear();
        callerSet.clear();
        change = false;

        // Having globally counted the returns, remove the unused ones
        // Note: after the first pass, we have only counted those callers which
        // call procs in calleeSet, so only consider those procs
        for (it = calleeSet.begin(); it != calleeSet.end(); it++) {
            UserProc* proc = *it;
            if (proc->isLib()) continue;
            if (Boomerang::get()->debugUnusedRets)
                LOG << " @@ removeUnusedReturns: considering callee " 
                    << proc->getName() << "\n";
            bool thisChange = proc->removeUnusedReturns(rc);
            if (thisChange && !Boomerang::get()->noRemoveNull) {
                // It may be that now there are more unused statements
                // (especially for SPARC programs)
                UserProc::RefCounter refCounts;
                // Count the references first
                proc->countRefs(refCounts);
                // Now remove any that have no used
                proc->removeUnusedStatements(refCounts, -1);
                // It may also be that there are now some parameters unused,
                // in particular esp
                proc->trimParameters();
                // and one more time
                refCounts.clear();
                proc->countRefs(refCounts);
                proc->removeUnusedStatements(refCounts, -1);
            }
            change |= thisChange;
            if (thisChange) {
                std::set<UserProc*> thisProcCallees;
                proc->addCallees(thisProcCallees);
                newCalleeSet.insert(thisProcCallees.begin(),
                  thisProcCallees.end());
                std::set<UserProc*>::iterator cc;
                for (cc = thisProcCallees.begin(); cc != thisProcCallees.end();
                      cc++)
                    (*cc)->addCallers(callerSet);
            }
        }
        calleeSet = newCalleeSet;
    } while (change);
}

#if 0       // For time being, this is in UserProc::generateCode()
void Prog::removeUnusedLocals() {
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        UserProc* proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        proc->removeUnusedLocals();
    }
}
#endif

// Have to transform out of SSA form after the above final pass
void Prog::fromSSAform() {
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        UserProc* proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        proc->fromSSAform();
        if (Boomerang::get()->vFlag) {
            LOG << "===== After transformation from SSA form for " 
                << proc->getName() << " =====\n";
            proc->printToLog(true);
            LOG << "===== End after transformation from SSA for " <<
              proc->getName() << " =====\n\n";
        }
    }
}

void Prog::typeAnalysis() {
    if (VERBOSE || DEBUG_TA)
        LOG << "=== Start Type Analysis ===\n";
    // FIXME: This needs to be done bottom of the call-tree first, with repeat
    // until no change for cycles in the call graph
    std::list<Proc*>::iterator pp;
    for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
        UserProc* proc = (UserProc*)(*pp);
        if (proc->isLib()) continue;
        proc->typeAnalysis(this);
    }
    if (VERBOSE || DEBUG_TA)
        LOG << "=== End Type Analysis ===\n";
}

void Prog::printCallGraph() {
    std::string fname = Boomerang::get()->getOutputPath() 
                        + "callgraph.out";
    int fd = lockFileWrite(fname.c_str());
    std::ofstream f(fname.c_str());
    std::set<Proc*> seen;
    std::map<Proc*, int> spaces;
    std::map<Proc*, Proc*> parent;
    std::list<Proc*> queue;
    queue.push_back(getEntryProc());
    spaces[queue.front()] = 0;
    while (queue.size()) {
        Proc *p = queue.front();
        queue.erase(queue.begin());
        if ((unsigned)p == NO_ADDRESS)
            continue;
        if (seen.find(p) == seen.end()) {
            seen.insert(p);
            int n = spaces[p];
            for (int i = 0; i < n; i++)
                f << "   ";
            f << p->getName();
            if (parent.find(p) != parent.end())
                f << " [parent=" << parent[p]->getName() << "]"; 
            f << std::endl;
            if (!p->isLib()) {
                n++;
                UserProc *u = (UserProc*)p;
                std::set<Proc*> &calleeSet = u->getCallees();
                for (std::set<Proc*>::reverse_iterator it1 = calleeSet.rbegin();
                     it1 != calleeSet.rend(); it1++) {
                    queue.push_front(*it1);
                    spaces[*it1] = n;
                    parent[*it1] = p;
                }
            }
        }
    }
    f.close();
    unlockFile(fd);
}

void Prog::printCallGraphXML() {
    if (!Boomerang::get()->dumpXML)
        return;
	std::list<Proc*>::iterator it;
    for (it = m_procs.begin(); it != m_procs.end();
         it++)
        (*it)->clearVisited();
    std::string fname = Boomerang::get()->getOutputPath()
                        + "callgraph.xml";
    int fd = lockFileWrite(fname.c_str());
    std::ofstream f(fname.c_str());
    f << "<prog name=\"" << getName() << "\">\n";
    f << "   <callgraph>\n";
    Proc *entry = getEntryProc();
    if (!entry->isLib())
        entry->printCallGraphXML(f, 2);
    for (it = m_procs.begin(); it != m_procs.end();
         it++)
        if (!(*it)->isVisited() && !(*it)->isLib()) {
            (*it)->printCallGraphXML(f, 2);
        }
    f << "   </callgraph>\n";
    f << "</prog>\n";
    f.close();
    unlockFile(fd);
}

void Prog::readSymbolFile(const char *fname)
{
    std::ifstream ifs;

    ifs.open(fname);

    if (!ifs.good()) {
        LOG << "can't open `" << fname << "'\n";
        exit(1);
    }

    AnsiCParser *par = new AnsiCParser(ifs, false);
    platform plat = getFrontEndId();
    callconv cc = CONV_C;
    if (isWin32()) cc = CONV_PASCAL;
    par->yyparse(plat, cc);

    for (std::list<Symbol*>::iterator it = par->symbols.begin();
         it != par->symbols.end(); it++) {
        if ((*it)->sig) {
            // probably wanna do something with this
            Proc *p = newProc((*it)->sig->getName(), (*it)->addr,
              (*it)->mods->noDecode);
            if (!(*it)->mods->incomplete)
                p->setSignature((*it)->sig->clone());
        } else {
            const char *nam = (*it)->nam.c_str();
            if (strlen(nam) == 0) {
                nam = newGlobal((*it)->addr);
            }
            Type *ty = (*it)->ty;
            if (ty == NULL) {
                ty = guessGlobalType(nam, (*it)->addr);
            }
            globals.push_back(new Global(ty, (*it)->addr, nam));
        }
    }

    for (std::list<SymbolRef*>::iterator it2 = par->refs.begin();
         it2 != par->refs.end(); it2++) {
        pFE->addRefHint((*it2)->addr, (*it2)->nam.c_str());
    }

    delete par;
    ifs.close();
}


Exp* Global::getInitialValue(Prog* prog) {
    Exp* e = NULL;
    PSectionInfo si = prog->getSectionInfoByAddr(uaddr);
    if (si && si->bBss)
        // This global is in the BSS, so it can't be initialised
        return NULL;
    if (type->isPointer() &&
      ((PointerType*)type)->getPointsTo()->resolvesToChar()) {
        char* str = prog->getStringConstant(uaddr, true);
        if (str) {
            // Make a global string
            return new Const(escapeStr(str));
        }
    } else if (type->isPointer() &&
      ((PointerType*)type)->getPointsTo()->resolvesToFunc()) {
        ADDRESS init = prog->readNative4(uaddr);    
        Proc* dest = prog->findProc(init);
        if (dest)
            // Make a function constant. Back end should know how to emit
            // the correct language-dependent code
            return new Const(dest);
    }
    if (e == NULL) switch(type->getSize()) {
    case 8:
        e = new Const(
            (int)*(char*)(uaddr + si->uHostAddr - si->uNativeAddr));
        break;
    case 16:
        // Note: must respect endianness
        e = new Const(prog->readNative2(uaddr));
        break;
    case 32:
    default:
        // Note: must respect endianness and type
        if (type->isFloat())
            e = new Const(prog->readNativeFloat4(uaddr));
        else
            e = new Const(prog->readNative4(uaddr));
        break;
    case 64:
        if (type->isFloat())
            e = new Const(prog->readNativeFloat8(uaddr));
        else
            e = new Const(prog->readNative8(uaddr));
    }
    return e;
}
