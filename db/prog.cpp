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
#include "types.h"
#include "dataflow.h"
#include "exp.h"
#include "proc.h"
#include "cfg.h"
#include "util.h"                   // For str()
#include "register.h"
#include "rtl.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "prog.h"
#include "signature.h"
#include "analysis.h"

Prog::Prog()
    : pBF(NULL),
      pFE(NULL),
      m_watcher(NULL)   // First numbered proc will be 1, no initial watcher
{
    // Default constructor
}

Prog::~Prog()
{
    if (pBF) delete pBF;
    if (pFE) delete pFE;
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end();
      it++) {
        if (*it)
            delete *it;
    }
    m_procs.clear();
}

Prog::Prog(const char* name)
    : pBF(NULL),
      pFE(NULL),
      filename(""),
      project(""),
      location(""),
      m_name(name),
      m_watcher(NULL)   // First numbered proc will be 1, no initial watcher
{
    // Constructor taking a name. Technically, the allocation of the
    // space for the name could fail, but this is unlikely
}

void Prog::setName (const char *name)      // Assign a name to this program
{
    m_name = name;
}

char* Prog::getName()
{
    return (char*) m_name.c_str();
}

// well form the entire program
bool Prog::wellForm()
{
	bool wellformed = true;

	for (std::list<Proc *>::iterator it = m_procs.begin(); it != m_procs.end(); it++)
		if (!(*it)->isLib()) {
			UserProc *u = (UserProc*)*it;
			wellformed &= u->getCFG()->wellFormCfg();
		}
	return wellformed;
}

// Analyse any procedures that are decoded
void Prog::analyse()
{
	Analysis *analysis = new Analysis();
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		Proc *pProc = *it;
		if (pProc->isLib()) continue;
		UserProc *p = (UserProc*)pProc;
		if (!p->isDecoded()) continue;

		// decoded userproc.. analyse it			
		analysis->analyse(p);
	}
}

// Do decompilation
void Prog::decompile()
{
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		Proc *pProc = *it;
		if (pProc->isLib()) continue;
		UserProc *p = (UserProc*)pProc;
		if (!p->isDecoded()) continue;

		// decoded userproc.. decompile it			
		p->decompile();
	}
}

// Print this program, mainly for debugging
void Prog::print(std::ostream &out)
{
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		Proc *pProc = *it;
		if (pProc->isLib()) continue;
		UserProc *p = (UserProc*)pProc;
		if (!p->isDecoded()) continue;

		// decoded userproc.. print it
		p->print(out);
	}
}


bool Prog::findSymbolFor(Exp *e, std::string &sym, TypedExp* &sym_exp)
{
	Exp *e1 = e;
	if (e->getOper() == opTypedExp)
		e1 = e->getSubExp1();
	for (std::map<std::string, TypedExp *>::iterator it = symbols.begin(); it != symbols.end(); it++)		
		if (*(*it).second->getSubExp1() == *e1)
		{
			sym = (*it).first;
			sym_exp = (*it).second;
			return true;
		}
	return false;
}

void Prog::deserialize(std::istream &inf)
{
    int fid;
    int len;
    int nProcs, cProcs = 0;
    loadValue(inf, nProcs, false);

    while ((fid = loadFID(inf)) != -1) {
        switch (fid) {
            case FID_PROJECT_NAME:
                loadString(inf, project);
                break;
            case FID_FILENAME:
                loadString(inf, filename);
                break;
			case FID_SYMBOL:				
				{
					len = loadLen(inf);
                    std::streampos pos = inf.tellg();
					std::string s;
					loadString(inf, s);
                    Exp *e = Exp::deserialize(inf);
                    assert((int)(inf.tellg() - pos) == len);
					assert(e->getOper() == opTypedExp);
					symbols[s] = (TypedExp*)e;
				}
				break;
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
                    m_procs.push_back(pProc);       // Append this to list of procs
                    m_procLabels[pProc->getNativeAddress()] = pProc;
                    if (m_watcher) m_watcher->alert_new(pProc);  // alert the watcher of a new proc
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

bool Prog::serialize(std::ostream &ouf, int &len)
{
	int fid;
	std::streampos st = ouf.tellp();

	int nProcs = 0, cProcs = 0;
    for (std::list<Proc *>::iterator it = m_procs.begin(); it != m_procs.end(); it++)
        nProcs++;
    saveValue(ouf, nProcs, false);

    // write information about Prog    
    saveFID(ouf, FID_PROJECT_NAME);
    saveString(ouf, project);
    saveFID(ouf, FID_FILENAME);
    saveString(ouf, filename);

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

	// write global symbols
	for (std::map<std::string, TypedExp *>::iterator its = symbols.begin(); its != symbols.end(); its++) {
		saveFID(ouf, FID_SYMBOL);

        std::streampos pos = ouf.tellp();
        int len = -1;
        saveLen(ouf, -1, true);
        std::streampos posa = ouf.tellp();

		saveString(ouf, (*its).first);
		assert((*its).second->serialize(ouf, len));

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
void Prog::clear()
{   
    project = std::string("");
    location = std::string("");
    filename = std::string("");
    m_name = std::string("");
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++)
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
	symbols.clear();
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
Proc* Prog::newProc (const char* name, ADDRESS uNative, bool bLib /*= false*/)
{
    Proc* pProc;
    std::string sname(name);
    if (bLib)
        pProc = new LibProc(this, sname, uNative);
    else
        pProc = new UserProc(this, sname, uNative);
    m_procs.push_back(pProc);       // Append this to list of procs
    m_procLabels[uNative] = pProc;
    if (m_watcher) m_watcher->alert_new(pProc);  // alert the watcher of a new proc
    return pProc;
}

/*==============================================================================
 * FUNCTION:       Prog::remProc
 * OVERVIEW:       Removes the UserProc from this Prog object's list, and
 *                  deletes as much as possible of the Proc (Cfg, RTLists, etc)
 * PARAMETERS:     proc: pointer to the UserProc object to be removed
 * RETURNS:        <nothing>
 *============================================================================*/
void Prog::remProc(UserProc* uProc)
{
    // Delete the cfg etc.
    uProc->deleteCFG();

    // Replace the entry in the procedure map with -1 as a warning not to
    // decode that address ever again
    m_procLabels[uProc->getNativeAddress()] = (Proc*)-1;

	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++)
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
int Prog::getNumProcs()
{
    return m_procs.size();
}


/*==============================================================================
 * FUNCTION:    Prog::getProc
 * OVERVIEW:    Return a pointer to the indexed Proc object
 * PARAMETERS:  Index of the proc
 * RETURNS:     Pointer to the Proc object, or 0 if index invalid
 *============================================================================*/
Proc* Prog::getProc(int idx) const
{
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
Proc* Prog::findProc(ADDRESS uAddr) const
{
    PROGMAP::const_iterator it;
    it = m_procLabels.find(uAddr);
    if (it == m_procLabels.end())
        return 0;
    else
        return (*it).second;
}

Proc* Prog::findProc(const char *name) const
{	
    std::list<Proc *>::const_iterator it;
	for (it = m_procs.begin(); it != m_procs.end(); it++)
		if (!strcmp((*it)->getName(), name))
			return *it;
	return NULL;
}

// get a library procedure by name
LibProc *Prog::getLibraryProc(const char *nam)
{
	Proc *p = findProc(nam);
	if (p && p->isLib())
		return (LibProc*)p;
	return (LibProc*)newProc(nam, NO_ADDRESS, true);
}


/*==============================================================================
 * FUNCTION:    Prog::findContainingProc
 * OVERVIEW:    Return a pointer to the Proc object containing uAddr, or 0 if none
 * NOTE:        Could return -1 for a deleted Proc
 * PARAMETERS:  Native address to search for
 * RETURNS:     Pointer to the Proc object, or 0 if none, or -1 if deleted
 *============================================================================*/
Proc* Prog::findContainingProc(ADDRESS uAddr) const
{
	for (std::list<Proc*>::const_iterator it = m_procs.begin(); it != m_procs.end(); it++) {
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
bool Prog::isProcLabel (ADDRESS addr)
{
    if (m_procLabels[addr] == 0)
        return false;
    return true;
}

/*==============================================================================
 * FUNCTION:    Prog::setArgv0
 * OVERVIEW:    Accepts argv[0] from main. This makes it easier to find
 *              some files, such as common.h.
 * PARAMETERS:  Pointer to the "zeroth" parameter, i.e. argv[0], which
 *              has a complete path (though not necessarily absolute)
 *              to the executable.
 * RETURNS:     <nothing>
 *============================================================================*/
void Prog::setArgv0(const char* p)
{
    m_progPath = p;
    // Chop off after the last slash
    size_t j = m_progPath.rfind("/");
    if (j != m_progPath.length())
    {
        // Do the chop; keep the trailing slash
        m_progPath = m_progPath.substr(0, j+1);
    }
    else {
        std::cerr << "? No slash in argv[0]!" << std::endl;
        exit(1);
    }
}

/*==============================================================================
 * FUNCTION:       Prog::getProgPath
 * OVERVIEW:       Returns the path to the current executable. The path has a
 *                 trailing slash, or is empty (if called without a path, e.g.
 *                 "uqbtss foo" as opposed to "../uqbtss foo")
 * PARAMETERS:     None
 * PRECONDITION:   Must have called SetArgv[0] after construction of Prog object
 * RETURNS:        A pointer to argv[0]
 *============================================================================*/
std::string& Prog::getProgPath()
{
    return m_progPath;
}



/*==============================================================================
 * FUNCTION:    Prog::getNameNoPath
 * OVERVIEW:    Get the name for the progam, without any path at the front
 * PARAMETERS:  None
 * RETURNS:     A string with the name
 *============================================================================*/
std::string Prog::getNameNoPath() const
{
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
Proc* Prog::getFirstProc(PROGMAP::const_iterator& it)
{
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
Proc* Prog::getNextProc(PROGMAP::const_iterator& it)
{
    it++;
    while (it != m_procLabels.end() && (it->second == (Proc*) -1))
        it++;
    if (it == m_procLabels.end())
        return 0;
    return it->second;
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
const void* Prog::getCodeInfo(ADDRESS uAddr, const char*& last, int& delta)
{
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

