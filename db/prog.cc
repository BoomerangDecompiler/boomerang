/*
 * Copyright (C) 1998-2001, The University of Queensland
 * Copyright (C) 2001, Sun Microsystems, Inc
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
#error BOOMDIR needs to be set
#endif

/*==============================================================================
 * Dependencies.
 *============================================================================*/

#include <fstream>
#include <sstream>
#include "prog.h"
#include "proc.h"
#include "util.h"                   // For str()

Prog::Prog()
	: m_iNumberedProc(1)			// First numbered proc will be 1
{
	// Default constructor
}

Prog::Prog(const char* name)
	: m_name(name), m_iNumberedProc(1)			// First numbered proc will be 1
{
	// Constructor taking a name. Technically, the allocation of the
	// space for the name could fail, but this is unlikely
}

void Prog::setName (std::string& name)	    // Assign a name to this program
{
	m_name = name;
}

char* Prog::getName()
{
	return (char*) m_name.c_str();
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
Proc* Prog::newProc (char* name, ADDRESS uNative, bool bLib /*= false*/)
{
	Proc* pProc;
    std::string sname(name);
	if (bLib)
		pProc = new LibProc(sname, uNative);
	else
		pProc = new UserProc(sname, uNative);
	m_procs.push_back(pProc);		// Append this to list of procs
	m_procLabels[uNative] = pProc;
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

    // Delete the UserProc object as well
    delete uProc;
}

/*==============================================================================
 * FUNCTION:	Prog::getNumProcs
 * OVERVIEW:	Return the number of real (non deleted) procedures
 * PARAMETERS:	None
 * RETURNS:		The number of procedures
 *============================================================================*/
int Prog::getNumProcs()
{
	return m_procs.size();
}


/*==============================================================================
 * FUNCTION:	Prog::getProc
 * OVERVIEW:	Return a pointer to the indexed Proc object
 * PARAMETERS:	Index of the proc
 * RETURNS:		Pointer to the Proc object, or 0 if index invalid
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
 * FUNCTION:	Prog::findProc
 * OVERVIEW:	Return a pointer to the associated Proc object, or 0 if none
 * NOTE:        Could return -1 for a deleted Proc
 * PARAMETERS:	Native address of the procedure entry point
 * RETURNS:		Pointer to the Proc object, or 0 if none, or -1 if deleted
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

/*==============================================================================
 * FUNCTION:	Prog::isProcLabel
 * OVERVIEW:	Return true if this is a real procedure
 * PARAMETERS:	Native address of the procedure entry point
 * RETURNS:		True if a real (non deleted) proc
 *============================================================================*/
bool Prog::isProcLabel (ADDRESS addr)
{
	if (m_procLabels[addr] == 0)
		return false;
	return true;
}

/*==============================================================================
 * FUNCTION:	Prog::visitProc
 * OVERVIEW:	Call this function when a procedure is discovered (usually by
 *                decoding a call instruction). That way, it is given a name
 *                that can be displayed in the dot file, etc. If we assign it
 *                a number now, then it will retain this number always
 * PARAMETERS:	Native address of the procedure entry point
 * RETURNS:		Pointer to the Proc object, or 0 if this is a deleted (not to
 *                be decoded) address
 *============================================================================*/
Proc* Prog::visitProc(ADDRESS uAddr)
{
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
        pName = str(ost);
	}
	return newProc(pName, uAddr, bLib);
}

/*==============================================================================
 * FUNCTION:	Prog::setArgv0
 * OVERVIEW:	Accepts argv[0] from main. This makes it easier to find
 *				some files, such as common.h.
 * PARAMETERS:	Pointer to the "zeroth" parameter, i.e. argv[0], which
 *				has a complete path (though not necessarily absolute)
 *				to the executable.
 * RETURNS:		<nothing>
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
const std::string& Prog::getProgPath()
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
	std::ifstream* ifs = new std::ifstream();

	std::string sPath = BOOMDIR"/include/common.hs";
	if (m_progPath.size() != 0)
	{
		// Now append the file name
		sPath = m_progPath + sPath;
	}
	ifs->open(sPath.c_str());

	if (!(*ifs))
	{
		std::cerr << "can't open `" << sPath << "'" << std::endl;
		exit(1);
	}

	std::string fname;
	std::string s;

	while (!ifs->eof())
	{
		(*ifs) >> fname;
		if (ifs->eof()) break;
		while (fname[0] == '#')
		{
			// Comment. Ignore till end of line
			ifs->ignore(100, '\n');
			(*ifs) >> fname;
			continue;
		}

		if (mapLibParam.find(fname) != mapLibParam.end()) {

			std::cerr << "entry for `" << fname << "' already read from `";
			std::cerr << sPath << "'\n";
		}

		std::list<TypedExp*>& paramList = mapLibParam[fname];

		do {
			char c;
			ifs->get(c);
            while ((c == ' ') || (c == '\t')) {
                ifs->get(c);
            }
			if (c == '\n') {
				break;
            }
            // last char (c) was not white space or a newline
			ifs->putback(c);
			(*ifs) >> s;
            Type* ty;
			switch(s[0]) {
				case 'i':
					ty = new Type(INTEGER, 32, true); break;
				case 's':
					ty = new Type(INTEGER, 16, true); break;
                case 'b':
                    ty = new Type(INTEGER,  8, true); break;
				case 'p':
					if (s[1] == 'd')
						ty = new Type(DATA_ADDRESS);
					else
						// Must be pf, pointer to function
						ty = new Type(FUNC_ADDRESS);
					break;
				case 'f':
					if (s[1] == 's')
                        // fs: floating point, single precision
						ty = new Type(FLOATP, 32);
					else
                        // fd: floating point, double precision
                        ty = new Type(FLOATP, 64);
					break;
				case '.':
					ty = new Type(VARARGS, 0); break;
				case 'v':
					ty = new Type(TVOID, 0); break;
				default:
					ty = new Type(UNKNOWN);
			}
            // For library functions, we don't as yet store the name of the
            // parameter. So for now, we just use Const(0) as the expression
            TypedExp* e = new TypedExp(*ty, new Const(0));
            paramList.push_back(e);
            delete ty;
		} while(1);			// Terminated by end of line
	}
}

/*==============================================================================
 * FUNCTION:	Prog::getNameNoPath
 * OVERVIEW:	Get the name for the progam, without any path at the front
 * PARAMETERS:	None
 * RETURNS:		A string with the name
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
 * FUNCTION:	Prog::getFirstProc
 * OVERVIEW:	Return a pointer to the first Proc object for this program
 * NOTE:        The it parameter must be passed to getNextProc
 * PARAMETERS:  it: An uninitialised PROGMAP::const_iterator
 * RETURNS:		A pointer to the first Proc object; could be 0 if none
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
 * FUNCTION:	Prog::getNextProc
 * OVERVIEW:	Return a pointer to the next Proc object for this program
 * NOTE:        The it parameter must be from a previous call to getFirstProc
 *                or getNextProc
 * PARAMETERS:  it: A PROGMAP::const_iterator as above
 * RETURNS:		A pointer to the next Proc object; could be 0 if no more
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
 * FUNCTION:	Prog::getTextLimits
 * OVERVIEW:	Set the limitTextLow and limitTextHigh members; also textDelta
 * NOTE:        The binary file must already be loaded
 * PARAMETERS:  <none>
 * RETURNS:		<nothing>
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

