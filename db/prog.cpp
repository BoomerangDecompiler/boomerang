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

Prog::Prog()
    : pBF(NULL),
      pFE(NULL),
      m_iNumberedProc(1),
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
      m_name(name), m_iNumberedProc(1),
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

void analysis(UserProc *p);

// Decode any procedures that are not decoded
void Prog::decode()
{
	bool change = true;
	while (change) {
		change = false;
		for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
			Proc *pProc = *it;
			if (pProc->isLib()) continue;
			UserProc *p = (UserProc*)pProc;
			if (p->isDecoded()) continue;

			// undecoded userproc.. decode it			
			change = true;
			std::ofstream os;
			int res = pFE->processProc(p->getNativeAddress(), p, os);
			if (res == 1)
				p->setDecoded();
			else
				break;
		}
	}
}

// Analyse any procedures that are decoded
void Prog::analyse()
{
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		Proc *pProc = *it;
		if (pProc->isLib()) continue;
		UserProc *p = (UserProc*)pProc;
		if (!p->isDecoded()) continue;

		// decoded userproc.. analyse it			
		analysis(p);
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

LibProc *Prog::getLibraryProc(const char *nam)
{
	Proc *p = findProc(nam);
	if (p && p->isLib())
		return (LibProc*)p;
	return (LibProc*)newProc(nam, NO_ADDRESS, true);
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

					loadValue(inf, limitTextLow, false);
					loadValue(inf, limitTextHigh, false);
					loadValue(inf, textDelta, false);

					std::string frontend;
					loadString(inf, frontend);
					pFE = FrontEnd::createById(frontend, this, textDelta, limitTextHigh);
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

		saveValue(ouf, limitTextLow, false);
		saveValue(ouf, limitTextHigh, false);
		saveValue(ouf, textDelta, false);

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
    m_iNumberedProc = 1;
	symbols.clear();

	for (std::map<std::string, Signature*>::iterator it1 = mapLibParam.begin(); it1 != mapLibParam.end(); it1++)
		if ((*it1).second)
			delete (*it1).second;
	mapLibParam.clear();
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
 * FUNCTION:    Prog::visitProc
 * OVERVIEW:    Call this function when a procedure is discovered (usually by
 *                decoding a call instruction). That way, it is given a name
 *                that can be displayed in the dot file, etc. If we assign it
 *                a number now, then it will retain this number always
 * PARAMETERS:  Native address of the procedure entry point
 * RETURNS:     Pointer to the Proc object, or 0 if this is a deleted (not to
 *                be decoded) address
 *============================================================================*/
Proc* Prog::visitProc(ADDRESS uAddr)
{
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
    if (pName == 0)
    {
        // No name. Give it a numbered name
        std::ostringstream ost;
        ost << "proc" << m_iNumberedProc++;
        pName = strdup(ost.str().c_str());
    }
    pProc = newProc(pName, uAddr, bLib);
    return pProc;
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
 * FUNCTION:       Prog::readLibParams
 * OVERVIEW:       Read the include/common.hs file, and store the results in a
 *                 map from string (function name) to integer (compressed type
 *                 information)
 * PARAMETERS:     None
 * RETURNS:        <nothing>
 *============================================================================*/
void Prog::readLibParams()
{
    std::ifstream ifs;


#ifdef WIN32
    std::string sPath = getProgPath() + "..\\include\\common.hs";
#else
    std::string sPath = BOOMDIR"/include/common.hs";
#endif
    ifs.open(sPath.c_str());

    if (!ifs.good())
    {
        std::cerr << "can't open `" << sPath << "'" << std::endl;
        exit(1);
    }

    std::string fname;
    std::string s;

    while (!ifs.eof())
    {
        ifs >> fname;
        if (ifs.eof()) break;
        while (fname[0] == '#')
        {
            // Comment. Ignore till end of line
            ifs.ignore(100, '\n');
            ifs >> fname;
            continue;
        }

		Signature *sig = NULL;
		std::string signame = fname;
		if (signame[0] != '-') {
			signame = "-stdc";
		} else {
			ifs >> fname;
	        	if (ifs.eof()) break;
		}
		signame += "-";
		signame += pFE->getFrontEndId();
		sig = Signature::instantiate(signame.c_str(), fname.c_str());
		assert(sig);

        if (mapLibParam.find(fname) != mapLibParam.end()) {

            std::cerr << "entry for `" << fname << "' already read from `";
            std::cerr << sPath << "'\n";
	    *((char *)0) = 1;
        }

        mapLibParam[fname] = sig;

		bool isret = true;

        while (!ifs.eof()) {
            char c;
            ifs.get(c);
            while (((c == ' ') || (c == '\t')) && !ifs.eof()) {
                ifs.get(c);
            }
            if (c == '\n' || ifs.eof()) {
                break;
            }
            // last char (c) was not white space or a newline
            ifs.putback(c);
            ifs >> s;
			if (s == "")   // EOF does this
				break;
            Type* ty;
            switch(s[0]) {
                case 'i':
                    ty = new IntegerType(32, true); break;
                case 's':
                    ty = new IntegerType(16, true); break;
                case 'b':
                    ty = new IntegerType(8, true); break;
                case 'p':
                    if (s[1] == 'd')
                        ty = new PointerType(new VoidType());
                    else
                        // Must be pf, pointer to function
                        ty = new PointerType(new FuncType());
                    break;
                case 'f':
                    if (s[1] == 's')
                        // fs: floating point, single precision
                        ty = new FloatType(32);
                    else
                        // fd: floating point, double precision
                        ty = new FloatType(64);
                    break;
                case '.':
                    // FIX
     		    break;
                case 'v':
                    ty = new VoidType(); break;
                default:
		    assert(false);
            }
			if (isret) {
				sig->setReturnType(ty);
				isret = false;
			} else {
		        sig->addParameter(ty);
			}
        }
    }
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
 * FUNCTION:    Prog::getTextLimits
 * OVERVIEW:    Set the limitTextLow and limitTextHigh members; also textDelta
 * NOTE:        The binary file must already be loaded
 * PARAMETERS:  <none>
 * RETURNS:     <nothing>
 *============================================================================*/
void Prog::getTextLimits()
{
    int n = pBF->GetNumSections();
    limitTextLow = 0xFFFFFFFF;
    limitTextHigh = 0;
    textDelta = 0;
    for (int i=0; i < n; i++) {
        SectionInfo* pSect = pBF->GetSectionInfo(i);
        if (pSect->bCode) {
            // The .plt section is an anomaly. It's code, but we never want to
            // decode it, and in Sparc ELF files, it's actually in the data
            // segment (so it can be modified). For now, we make this ugly
            // exception
            if (strcmp(".plt", pSect->pSectionName) == 0)
                continue;
            if (pSect->uNativeAddr < limitTextLow)
                limitTextLow = pSect->uNativeAddr;
            ADDRESS hiAddress = pSect->uNativeAddr + pSect->uSectionSize;
            if (hiAddress > limitTextHigh)
                limitTextHigh = hiAddress;
            if (textDelta == 0)
                textDelta = pSect->uHostAddr - pSect->uNativeAddr;
            else
                assert(textDelta ==
                    (int) (pSect->uHostAddr - pSect->uNativeAddr));
        }
    }
}

bool Prog::LoadBinary(const char *fname)
{
    clear();
    pBF = BinaryFile::Load(fname);
    if (pBF == NULL) return false;

    getTextLimits();


	pFE = FrontEnd::instantiate(pBF->GetMachine(), this, textDelta, limitTextHigh);
	if (pFE == NULL) return false;

	readLibParams();

        bool gotMain;
        ADDRESS a = pFE->getMainEntryPoint(gotMain);
        if (a == NO_ADDRESS) return false;

	visitProc(a);
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

