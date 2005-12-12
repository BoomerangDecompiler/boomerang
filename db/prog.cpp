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
 * FILE:		prog.cpp
 * OVERVIEW:	Implementation of the program class. Holds information of
 *				interest to the whole program.
 *============================================================================*/

/*
 * $Revision$	// 1.126.2.14
 *
 * 18 Apr 02 - Mike: Mods for boomerang
 * 26 Apr 02 - Mike: common.hs read relative to BOOMDIR
 * 20 Jul 04 - Mike: Got rid of BOOMDIR
 */

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
#ifdef WIN32
#include <direct.h>					// For Windows mkdir()
#endif

#include "type.h"
#include "cluster.h"
#include "types.h"
#include "statement.h"
#include "exp.h"
#include "cfg.h"
#include "proc.h"
#include "util.h"					// For lockFileWrite etc
#include "register.h"
#include "rtl.h"
#include "BinaryFile.h"
#include "frontend.h"
#include "prog.h"
#include "signature.h"
#include "boomerang.h"
#include "ansi-c-parser.h"
#include "config.h"
#include "managed.h"
#include "log.h"

#include <sys/stat.h>
#include <sys/types.h>

Prog::Prog() :
		pBF(NULL),
		pFE(NULL),
		m_iNumberedProc(1),
		m_rootCluster(new Cluster("prog")) {
	// Default constructor
}

void Prog::setFrontEnd(FrontEnd *pFE) {
	pBF = pFE->getBinaryFile();
	this->pFE = pFE;
	if (pBF && pBF->getFilename()) {
		m_name = pBF->getFilename();
		m_rootCluster = new Cluster(getNameNoPath().c_str());
	}
}

Prog::Prog(const char* name) :
		pBF(NULL),
		pFE(NULL),
		m_name(name),
		m_iNumberedProc(1),
		m_rootCluster(new Cluster(getNameNoPath().c_str())) {
	// Constructor taking a name. Technically, the allocation of the space for the name could fail, but this is unlikely
	 m_path = m_name;
}

Prog::~Prog() {
	if (pBF) delete pBF;
	if (pFE) delete pFE;
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		if (*it)
			delete *it;
	}
	m_procs.clear();
}

void Prog::setName (const char *name) {	   // Assign a name to this program
	m_name = name;
	m_rootCluster->setName(name);
}

char* Prog::getName() {
	return (char*) m_name.c_str();
}

// well form the entire program
bool Prog::wellForm() {
	bool wellformed = true;

	for (std::list<Proc *>::iterator it = m_procs.begin(); it != m_procs.end(); it++)
		if (!(*it)->isLib()) {
			UserProc *u = (UserProc*)*it;
			wellformed &= u->getCFG()->wellFormCfg();
		}
	return wellformed;
}

// last fixes after decoding everything
// was in analysis.cpp
void Prog::finishDecode()
{
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		Proc *pProc = *it;

		if (pProc->isLib()) continue;
		UserProc *p = (UserProc*)pProc;
		if (!p->isDecoded()) continue;
		
		p->assignProcsToCalls();
		p->finalSimplify();
	}

}

void Prog::generateDotFile() {
	assert(Boomerang::get()->dotFile);
	std::ofstream of(Boomerang::get()->dotFile);
	of << "digraph Cfg {" << std::endl;

	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		Proc *pProc = *it;
		if (pProc->isLib()) continue;
		UserProc *p = (UserProc*)pProc;
		if (!p->isDecoded()) continue;
		// Subgraph for the proc name
		of << "\nsubgraph cluster_" << p->getName() << " {\n" << "	   color=gray;\n	label=" << p->getName() <<
			";\n";
		// Generate dotty CFG for this proc
		p->getCFG()->generateDotFile(of);
	}
	of << "}";
	of.close();

}

void Prog::generateCode(Cluster *cluster, UserProc *proc, bool intermixRTL) {
	std::string basedir = m_rootCluster->makeDirs();
	std::ofstream os;
	if (cluster) {
		cluster->openStream("c");
		cluster->closeStreams();
	}
	if (cluster == NULL || cluster == m_rootCluster) {
		os.open(m_rootCluster->getOutPath("c"));
		if (proc == NULL) {
			HLLCode *code = Boomerang::get()->getHLLCode();
			bool global = false;
			for (std::set<Global*>::iterator it1 = globals.begin(); it1 != globals.end(); it1++) {
				// Check for an initial value
				Exp *e = NULL;
				e = (*it1)->getInitialValue(this);
				if (e) {
					code->AddGlobal((*it1)->getName(), (*it1)->getType(), e);
					global = true;
				}
			}
			if (global) code->print(os);		// Avoid blank line if no globals
		}
	}

	// First declare prototypes for all but the first proc
	std::list<Proc*>::iterator it = m_procs.begin();
	bool first = true, proto = false;
	for (it = m_procs.begin(); it != m_procs.end(); it++) {
		if ((*it)->isLib()) continue;
		if (first) {
			first = false;
			continue;
		}
		proto = true;
		UserProc* up = (UserProc*)*it;
		HLLCode *code = Boomerang::get()->getHLLCode(up);
		code->AddPrototype(up);					// May be the wrong signature if up has ellipsis
		if (cluster == NULL || cluster == m_rootCluster)
			code->print(os);
	}
	if (proto && cluster == NULL || cluster == m_rootCluster)
		os << "\n";				// Separate prototype(s) from first proc
		
	for (it = m_procs.begin(); it != m_procs.end(); it++) {
		Proc *pProc = *it;
		if (pProc->isLib()) continue;
		UserProc *up = (UserProc*)pProc;
		if (!up->isDecoded()) continue;
		if (proc != NULL && up != proc)
			continue;
		up->getCFG()->compressCfg();
		HLLCode *code = Boomerang::get()->getHLLCode(up);
		up->generateCode(code);
		if (up->getCluster() == m_rootCluster) {
			if (cluster == NULL || cluster == m_rootCluster)
				code->print(os);
		} else {
			if (cluster == NULL || cluster == up->getCluster()) {
				up->getCluster()->openStream("c");
				code->print(up->getCluster()->getStream());
			}
		}
	}
	os.close();
	m_rootCluster->closeStreams();
}

void Prog::generateRTL(Cluster *cluster, UserProc *proc) {
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		Proc *pProc = *it;
		if (pProc->isLib()) continue;
		UserProc *p = (UserProc*)pProc;
		if (!p->isDecoded()) continue;
		if (proc != NULL && p != proc)
			continue;
		if (cluster != NULL && p->getCluster() != cluster)
			continue;

		p->getCluster()->openStream("rtl");
		p->print(p->getCluster()->getStream());
    }
    m_rootCluster->closeStreams();
}

Statement *Prog::getStmtAtLex(Cluster *cluster, unsigned int begin, unsigned int end)
{
    for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
        Proc *pProc = *it;
        if (pProc->isLib()) continue;
        UserProc *p = (UserProc*)pProc;
        if (!p->isDecoded()) continue;
		if (cluster != NULL && p->getCluster() != cluster)
			continue;

		if (p->getCluster() == cluster) {
			Statement *s = p->getStmtAtLex(begin, end);
			if (s)
				return s;
		}
	}
	return NULL;
}


const char *Cluster::makeDirs()
{
	std::string path;
	if (parent)
		path = parent->makeDirs();
	else
		path = Boomerang::get()->getOutputPath();		 
	if (getNumChildren() > 0 || parent == NULL) {
		path = path + "/" + name;
#ifdef WIN32
		mkdir(path.c_str());
#else
		mkdir(path.c_str(), 0777);
#endif
	}
	return strdup(path.c_str());
}

void Cluster::removeChild(Cluster *n)
{
	std::vector<Cluster*>::iterator it;
	for (it = children.begin(); it != children.end(); it++)
		if (*it == n)
			break;
	assert(it != children.end());
	children.erase(it);
}

void Cluster::addChild(Cluster *n)
{ 
	if (n->parent)
		n->parent->removeChild(n);
	children.push_back(n); 
	n->parent = this; 
}

Cluster *Cluster::find(const char *nam)
{
	if (name == nam)
		return this;
	for (unsigned i = 0; i < children.size(); i++) {
		Cluster *c = children[i]->find(nam);
		if (c)
			return c;
	}
	return NULL;
}

bool Prog::clusterUsed(Cluster *c)
{
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++)
		if ((*it)->getCluster() == c)
			return true;
	return false;
}

void Prog::generateCode(std::ostream &os) {
	HLLCode *code = Boomerang::get()->getHLLCode();
	for (std::set<Global*>::iterator it1 = globals.begin(); it1 != globals.end(); it1++) {
		// Check for an initial value
		Exp *e = NULL;
		e = (*it1)->getInitialValue(this);
		if (e)
			code->AddGlobal((*it1)->getName(), (*it1)->getType(), e);
	}
	code->print(os);
	delete code;
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
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
void Prog::print(std::ostream &out) {
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		Proc *pProc = *it;
		if (pProc->isLib()) continue;
		UserProc *p = (UserProc*)pProc;
		if (!p->isDecoded()) continue;

		// decoded userproc.. print it
		p->print(out);
	}
}

// clear the current project
void Prog::clear() {   
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
}

/*==============================================================================
 * FUNCTION:	Prog::setNewProc
 * NOTE:		Formally Frontend::newProc
 * OVERVIEW:	Call this function when a procedure is discovered (usually by
 *				  decoding a call instruction). That way, it is given a name
 *				  that can be displayed in the dot file, etc. If we assign it
 *				  a number now, then it will retain this number always
 * PARAMETERS:	uAddr - Native address of the procedure entry point
 * RETURNS:		Pointer to the Proc object, or 0 if this is a deleted (not to
 *				  be decoded) address
 *============================================================================*/
Proc* Prog::setNewProc(ADDRESS uAddr) {
	// this test fails when decoding sparc, why?  Please investigate - trent
	// Likely because it is in the Procedure Linkage Table (.plt), which for Sparc is in the data section
	//assert(uAddr >= limitTextLow && uAddr < limitTextHigh);
	// Check if we already have this proc
	Proc* pProc = findProc(uAddr);
	if (pProc == (Proc*)-1)			// Already decoded and deleted?
		return 0;					// Yes, exit with 0
	if (pProc)
		// Yes, we are done
		return pProc;
	const char* pName = pBF->SymbolByAddress(uAddr);
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
 * FUNCTION:	Prog::newProc
 * OVERVIEW:	Creates a new Proc object, adds it to the list of procs in this Prog object, and adds the address to
 *					the list
 * PARAMETERS:	name: Name for the proc
 *				uNative: Native address of the entry point of the proc
 *				bLib: If true, this will be a libProc; else a UserProc
 * RETURNS:		A pointer to the new Proc object
 *============================================================================*/
Proc* Prog::newProc (const char* name, ADDRESS uNative, bool bLib /*= false*/) {
	Proc* pProc;
	std::string sname(name);
	if (bLib)
		pProc = new LibProc(this, sname, uNative);
	else
		pProc = new UserProc(this, sname, uNative);
	m_procs.push_back(pProc);		// Append this to list of procs
	m_procLabels[uNative] = pProc;
	// alert the watchers of a new proc
	Boomerang::get()->alert_new(pProc);
	return pProc;
}

/*==============================================================================
 * FUNCTION:	   Prog::remProc
 * OVERVIEW:	   Removes the UserProc from this Prog object's list, and deletes as much as possible of the Proc
 * PARAMETERS:	   proc: pointer to the UserProc object to be removed
 * RETURNS:		   <nothing>
 *============================================================================*/
void Prog::remProc(UserProc* uProc) {
	// Delete the cfg etc.
	uProc->deleteCFG();

	// Replace the entry in the procedure map with -1 as a warning not to decode that address ever again
	m_procLabels[uProc->getNativeAddress()] = (Proc*)-1;

	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		if (*it == uProc) {
			m_procs.erase(it);
			break;
		}
	}

	// Delete the UserProc object as well
	delete uProc;
}

/*==============================================================================
 * FUNCTION:	Prog::getNumProcs
 * OVERVIEW:	Return the number of real (non deleted) procedures
 * PARAMETERS:	None
 * RETURNS:		The number of procedures
 *============================================================================*/
int Prog::getNumProcs() {
	return m_procs.size();
}

int Prog::getNumUserProcs() {
	int n = 0;
	for (std::list<Proc*>::const_iterator it = m_procs.begin(); it != m_procs.end(); it++)
		if (!(*it)->isLib())
			n++;
	return n;
}

/*==============================================================================
 * FUNCTION:	Prog::getProc
 * OVERVIEW:	Return a pointer to the indexed Proc object
 * PARAMETERS:	Index of the proc
 * RETURNS:		Pointer to the Proc object, or 0 if index invalid
 *============================================================================*/
Proc* Prog::getProc(int idx) const {
	// Return the indexed procedure. If this is used often, we should use a vector instead of a list
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
 * NOTE:		Could return -1 for a deleted Proc
 * PARAMETERS:	Native address of the procedure entry point
 * RETURNS:		Pointer to the Proc object, or 0 if none, or -1 if deleted
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

// get a library procedure by name; create if does not exist
LibProc *Prog::getLibraryProc(const char *nam) {
	Proc *p = findProc(nam);
	if (p && p->isLib())
		return (LibProc*)p;
	return (LibProc*)newProc(nam, NO_ADDRESS, true);
}

Signature* Prog::getLibSignature(const char *nam) {
	return pFE->getLibSignature(nam);
}

void Prog::rereadLibSignatures()
{
	pFE->readLibraryCatalog();
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		if ((*it)->isLib()) {
			(*it)->setSignature(getLibSignature((*it)->getName()));
			std::set<CallStatement*> &callers = (*it)->getCallers();
			for (std::set<CallStatement*>::iterator it1 = callers.begin(); it1 != callers.end(); it1++)
				(*it1)->setSigArguments();
			Boomerang::get()->alert_update_signature(*it);
		}
	}
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
	// FIXME: inefficient
	for (std::set<Global*>::iterator it = globals.begin(); it != globals.end(); it++) {
		if ((*it)->getAddress() == uaddr)
			return (*it)->getName();
		else if ((*it)->getAddress() < uaddr &&
				(*it)->getAddress() + (*it)->getType()->getSize() / 8 > uaddr)
			return (*it)->getName();
	}
	if (pBF)
		return pBF->SymbolByAddress(uaddr);
	return NULL;
}

void Prog::dumpGlobals() {
	for (std::set<Global*>::iterator it = globals.begin(); it != globals.end(); it++) {
		(*it)->print(std::cerr, this);
		std::cerr << "\n";
	}
}
		
ADDRESS Prog::getGlobalAddr(char *nam)
{
   	for (std::set<Global*>::iterator it = globals.begin(); it != globals.end(); it++) {
        if (!strcmp((*it)->getName(), nam))
        	return (*it)->getAddress();
   	}
	return pBF->GetAddressByName(nam);
}

Global* Prog::getGlobal(char *nam) {
   	for (std::set<Global*>::iterator it = globals.begin(); it != globals.end(); it++) {
        if (!strcmp((*it)->getName(), nam))
        	return *it;
   	}
	return NULL;
}

void Prog::globalUsed(ADDRESS uaddr, Type* knownType) {
    Global* global;
    
    for (std::set<Global*>::iterator it = globals.begin(); it != globals.end(); it++) {
        if ((*it)->getAddress() == uaddr) {
			if (knownType) (*it)->meetType(knownType);
            return;
		}
        else if ((*it)->getAddress() < uaddr && (*it)->getAddress() + (*it)->getType()->getSize() / 8 > uaddr) {
			if (knownType) (*it)->meetType(knownType);
            return;
		}
	}
	
#if 0
    if (uaddr < 0x10000) {
        // This happens in windows code because you can pass a low value integer instead 
        // of a string to some functions.
		if (VERBOSE)
			LOG << "warning: ignoring stupid request for global at address " << uaddr << "\n";
        return;
    }
#endif
    const char *nam = newGlobalName(uaddr); 
    Type *ty;
	if (knownType)
		ty = knownType;
	else
		ty = guessGlobalType(nam, uaddr);
		
	global = new Global(ty, uaddr, nam);
    globals.insert(global);

    if (VERBOSE) {
        LOG << "globalUsed: name " << nam << ", address " << uaddr;
		if (knownType)
			LOG << ", known type " << ty->getCtype() << "\n";
		else
			LOG << ", guessed type " << ty->getCtype() << "\n";
	}
}

std::map<ADDRESS, std::string> &Prog::getSymbols()
{
	return pBF->getSymbols();
}

ArrayType* Prog::makeArrayType(ADDRESS u, Type* t) {
	const char* nam = newGlobalName(u);
	int sz = pBF->GetSizeByName(nam);
	if (sz == 0)
		return new ArrayType(t);		// An "unbounded" array
	int n = t->getSize()/8;
	if (n == 0) n = 1;
	return new ArrayType(t, sz/n);
}

Type *Prog::guessGlobalType(const char *nam, ADDRESS u) {
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
		case 1: case 2: case 4: case 8:
			ty = new IntegerType(sz*8);
			break;
		default:
			ty = new ArrayType(new CharType(), sz);
	}
	return ty;
}

const char *Prog::newGlobalName(ADDRESS uaddr)
{
	const char *nam = getGlobalName(uaddr);
	if (nam == NULL) {
		std::ostringstream os;
		os << "global" << globals.size();
		nam = strdup(os.str().c_str());
		if (VERBOSE)
			LOG << "naming new global: " << nam << " at address " << uaddr << "\n";
	} 
	return nam;
}

Type *Prog::getGlobalType(char* nam) {
 	for (std::set<Global*>::iterator it = globals.begin(); it != globals.end(); it++)
		if (!strcmp((*it)->getName(), nam))
			return (*it)->getType();
	return NULL;
}

void Prog::setGlobalType(const char* nam, Type* ty) {
	// FIXME: inefficient
	for (std::set<Global*>::iterator it = globals.begin(); it != globals.end(); it++) {
		if (!strcmp((*it)->getName(), nam)) {
			(*it)->setType(ty);
			return;
		}
	}
}

// get a string constant at a given address if appropriate
// if knownString, it is already known to be a char*
char *Prog::getStringConstant(ADDRESS uaddr, bool knownString /* = false */) {
	SectionInfo* si = pBF->GetSectionInfoByAddr(uaddr);
	// Too many compilers put constants, including string constants, into read/write sections
	//if (si && si->bReadOnly)
	if (si && !si->bBss) {
		// At this stage, only support ascii, null terminated, non unicode strings.
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
 * FUNCTION:	Prog::findContainingProc
 * OVERVIEW:	Return a pointer to the Proc object containing uAddr, or 0 if none
 * NOTE:		Could return -1 for a deleted Proc
 * PARAMETERS:	Native address to search for
 * RETURNS:		Pointer to the Proc object, or 0 if none, or -1 if deleted
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
 * FUNCTION:	Prog::isProcLabel
 * OVERVIEW:	Return true if this is a real procedure
 * PARAMETERS:	Native address of the procedure entry point
 * RETURNS:		True if a real (non deleted) proc
 *============================================================================*/
bool Prog::isProcLabel (ADDRESS addr) {
	if (m_procLabels[addr] == 0)
		return false;
	return true;
}

/*==============================================================================
 * FUNCTION:	Prog::getNameNoPath
 * OVERVIEW:	Get the name for the progam, without any path at the front
 * PARAMETERS:	None
 * RETURNS:		A string with the name
 *============================================================================*/
std::string Prog::getNameNoPath() const {
	unsigned n = m_name.rfind("/");
	if (n == std::string::npos) {
		n = m_name.rfind("\\");
		if (n == std::string::npos)
			return m_name;
	}

	return m_name.substr(n+1);
}

/*==============================================================================
 * FUNCTION:	Prog::getFirstProc
 * OVERVIEW:	Return a pointer to the first Proc object for this program
 * NOTE:		The it parameter must be passed to getNextProc
 * PARAMETERS:	it: An uninitialised PROGMAP::const_iterator
 * RETURNS:		A pointer to the first Proc object; could be 0 if none
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
 * FUNCTION:	Prog::getNextProc
 * OVERVIEW:	Return a pointer to the next Proc object for this program
 * NOTE:		The it parameter must be from a previous call to getFirstProc or getNextProc
 * PARAMETERS:	it: A PROGMAP::const_iterator as above
 * RETURNS:		A pointer to the next Proc object; could be 0 if no more
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
 * FUNCTION:	Prog::getFirstUserProc
 * OVERVIEW:	Return a pointer to the first UserProc object for this program
 * NOTE:		The it parameter must be passed to getNextUserProc
 * PARAMETERS:	it: An uninitialised std::list<Proc*>::iterator
 * RETURNS:		A pointer to the first UserProc object; could be 0 if none
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
 * FUNCTION:	Prog::getNextUserProc
 * OVERVIEW:	Return a pointer to the next UserProc object for this program
 * NOTE:		The it parameter must be from a previous call to
 *				  getFirstUserProc or getNextUserProc
 * PARAMETERS:	it: A std::list<Proc*>::iterator
 * RETURNS:		A pointer to the next UserProc object; could be 0 if no more
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
 * FUNCTION:	getCodeInfo
 * OVERVIEW:	Lookup the given native address in the code section, returning a host pointer corresponding to the same
 *				 address
 * PARAMETERS:	uNative: Native address of the candidate string or constant
 *				last: will be set to one past end of the code section (host)
 *				delta: will be set to the difference between the host and native addresses
 * RETURNS:		Host pointer if in range; NULL if not
 *				Also sets 2 reference parameters (see above)
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
		if ((uAddr < pSect->uNativeAddr) || (uAddr >= pSect->uNativeAddr + pSect->uSectionSize))
			continue;			// Try the next section
		delta = pSect->uHostAddr - pSect->uNativeAddr;
		last = (const char*) (pSect->uHostAddr + pSect->uSectionSize);
		const char* p = (const char *) (uAddr + delta);
		return p;
	}
	return NULL;
#endif
}

void Prog::decodeEntryPoint(ADDRESS a) { 
	Proc* p = (UserProc*)findProc(a);
	if (p == NULL || (!p->isLib() && !((UserProc*)p)->isDecoded())) {
		pFE->decode(this, a);
		finishDecode();
	}
	if (p == NULL)
		p = findProc(a);
	assert(p);
	if (!p->isLib())				// -sf procs marked as __nodecode are treated as library procs (?)
		entryProcs.push_back((UserProc*)p);
}

void Prog::setEntryPoint(ADDRESS a) {
	Proc* p = (UserProc*)findProc(a);
	if (p == NULL || (!p->isLib()))
		entryProcs.push_back((UserProc*)p);
}

void Prog::decodeEverythingUndecoded() {
	std::list<Proc*>::iterator pp;
	for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
		UserProc *up = (UserProc*) *pp;
		if (up == NULL) continue;	// Probably not needed
		if (up->isLib()) continue;
		if (up->isDecoded()) continue;
		pFE->decode(this, up->getNativeAddress());
	}
	finishDecode();
}

void Prog::decompile() {
	assert(m_procs.size());

	if (VERBOSE) 
		LOG << (int)m_procs.size() << " procedures\n";

	// Start decompiling each entry point
	std::list<UserProc*>::iterator ee;
	for (ee = entryProcs.begin(); ee != entryProcs.end(); ++ee) {
		std::cerr << "decompiling entry point " << (*ee)->getName() << "\n";
		LOG		  << "decompiling entry point " << (*ee)->getName() << "\n";
		(*ee)->decompile(new CycleList);
	}

	// Just in case there are any Procs not in the call graph. 
	std::list<Proc*>::iterator pp;
	if (Boomerang::get()->decodeMain && !Boomerang::get()->noDecodeChildren) {
		for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
			UserProc* proc = (UserProc*)(*pp);
			if (proc->isLib()) continue;
			if (proc->isDecompiled()) continue;
			proc->decompile(new CycleList);
		}
	}

	// Type analysis, if requested
	if (Boomerang::get()->conTypeAnalysis && Boomerang::get()->dfaTypeAnalysis) {
		std::cerr << "can't use two types of type analysis at once!\n";
		Boomerang::get()->conTypeAnalysis = false;
	}
	globalTypeAnalysis();


	if (!Boomerang::get()->noDecompile) {
		if (!Boomerang::get()->noRemoveReturns) {
			// A final pass to remove returns not used by any caller
			if (VERBOSE)
				LOG << "prog: global removing unused returns\n";
			removeUnusedReturns();
		}

		// print XML after removing returns
		for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
			UserProc* proc = (UserProc*)(*pp);
			if (proc->isLib()) continue;
			proc->printXML();
		}
	}

	if (VERBOSE)
		LOG << "transforming from SSA\n";

	// Now it is OK to transform out of SSA form
	fromSSAform();

	// Note: removeUnusedLocals() is now in UserProc::generateCode()

	removeUnusedGlobals();
}

void Prog::removeUnusedGlobals() {
	if (VERBOSE)
		LOG << "removing unused globals\n";

    // seach for used globals
	std::list<Exp*> usedGlobals;
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		if ((*it)->isLib())	continue;
		UserProc *u = (UserProc*)(*it);
		Exp* search = new Location(opGlobal, new Terminal(opWild), u);
		// Search each statement in u, excepting implicit assignments (their uses don't count, since they don't really
		// exist in the program representation)
		StatementList stmts;
		StatementList::iterator ss;
		u->getStatements(stmts);
		for (ss = stmts.begin(); ss != stmts.end(); ++ss) {
			Statement* s = *ss;
			if (s->isImplicit()) continue;			// Ignore the uses in ImplicitAssigns
			bool found = s->searchAll(search, usedGlobals);
			if (found && DEBUG_UNUSED)
				LOG << " a global is used by stmt " << s->getNumber() << "\n";
		}
	}

	// make a map to find a global by its name (could be a global var too)
	std::map<std::string, Global*> namedGlobals;
	for (std::set<Global*>::iterator it = globals.begin(); it != globals.end(); it++)
		namedGlobals[(*it)->getName()] = (*it);
		
    // rebuild the globals vector
    char* name;
    Global* usedGlobal;
    
    globals.clear();
	for (std::list<Exp*>::iterator it = usedGlobals.begin(); it != usedGlobals.end(); it++) {
		if (DEBUG_UNUSED)
			LOG << " " << *it << " is used\n";
     	name = ((Const*)(*it)->getSubExp1())->getStr();
     	usedGlobal=namedGlobals[name];
     	if(usedGlobal) {
     		globals.insert(usedGlobal);
		} else {
      		std::cerr << "warning: an expression refers to a nonexistent global";
    	}
	}

#if 0
	if (VERBOSE)
		LOG << "removing unused globals\n";
	std::list<Exp*> result;
	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++) {
		if ((*it)->isLib())
			continue;
		UserProc *u = (UserProc*)(*it);
		u->searchAll(new Location(opGlobal, new Terminal(opWild), u), result);
	}
	// MVE: The following seems incredibly round about
	std::map<std::string, Global*> unusedGlobals, usedGlobals;
	for (std::vector<Global*>::iterator it = globals.begin(); it != globals.end(); it++)
		unusedGlobals[(*it)->getName()] = *it;
	usedGlobals = unusedGlobals;
	for (std::list<Exp*>::iterator it = result.begin(); it != result.end(); it++)
		unusedGlobals.erase(((Const*)(*it)->getSubExp1())->getStr());
	for (std::map<std::string, Global*>::iterator it = unusedGlobals.begin(); it != unusedGlobals.end(); it++) {
		if (VERBOSE)
			LOG << "unused global " << it->first.c_str() << " at address " << it->second->getAddress() << "\n";
		usedGlobals.erase((*it).first);
	}
	globals.clear();
	for (std::map<std::string, Global*>::iterator it = usedGlobals.begin(); it != usedGlobals.end(); it++) {
		globals.push_back((*it).second);
	}
#endif
}

#if 0
void Prog::removeUnusedReturns() {
	// The counter
	UserProc::ReturnCounter rc;
	// Two worksets; one of procs whose return sets have changed, and one of their callers
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
		// Iterate through the workset, looking for uses of returns from calls in these procs (initially, all procs;
		// later, callers of procs whose returns set has been reduced
		// FIXME: the ordering is not consistent; it is affecting whether output is correct or not!!
		// (so worksets not working)
		std::set<UserProc*>::iterator it;
		for (it = callerSet.begin(); it != callerSet.end(); it++)
			(*it)->countUsedReturns(rc);

		// Count a return reference for main. Looking for the name "main" is good, becuase if it's not a
		// C program, then it won't have a main, and it probably (?) won't return an int
		UserProc* m = (UserProc*) findProc("main");
		if (m) {
			// Note: it's position 1, because position 0 is the stack pointer
			// Note: if it's SPARC (or perhaps other architectures), there may be only one
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

		// Having globally counted the returns, remove the unused ones. Note: after the first pass, we have only
		// counted those callers which call procs in calleeSet, so only consider those procs
		for (it = calleeSet.begin(); it != calleeSet.end(); it++) {
			UserProc* proc = *it;
			if (proc->isLib()) continue;
			if (Boomerang::get()->debugUnusedRetsAndParams)
				LOG << " @@ removeUnusedReturns: considering callee " << proc->getName() << "\n";
			bool thisChange = proc->removeUnusedReturns(rc);
			if (thisChange && !Boomerang::get()->noRemoveNull) {
				// It may be that now there are more unused statements (especially for SPARC programs)
				UserProc::RefCounter refCounts;
				// Count the references first
				proc->countRefs(refCounts);
				// Now remove any that have no used
				proc->removeUnusedStatements(refCounts, -1);
				// It may also be that there are now some parameters unused, in particular esp
				proc->trimParameters();
				// and one more time
				refCounts.clear();
				proc->countRefs(refCounts);
				proc->removeUnusedStatements(refCounts, -1);
			}
			change |= thisChange;

			if (thisChange) {
				std::list<UserProc*> thisProcCallees;
				proc->addCallees(thisProcCallees);
				newCalleeSet.insert(thisProcCallees.begin(), thisProcCallees.end());
				std::list<UserProc*>::iterator cc;
				for (cc = thisProcCallees.begin(); cc != thisProcCallees.end(); cc++)
					(*cc)->addCallers(callerSet);
			}

		}
		calleeSet = newCalleeSet;
	} while (change);
}
#else
// This is the global removing of unused returns. The initial idea is simple enough: remove some returns according to
// the formula returns(p) = modifys(p) isect union(live at c) for all c calling p.
// However, removing returns reduces the uses, leading to three effects:
// 1) The statement that defines the return, if only used by that return, becomes unused
// 2) if the return is implicitly defined, then the parameters may be reduced, which affects all callers
// 3) if the return is defined at a call, the location may no longer be live at the call. If not, you need to check
//   the child, and do the union again (hence needing a list of callers) to find out if this change also affects that
//	 child.
void Prog::removeUnusedReturns() {
	// For each UserProc. Each proc may process many others, so this may duplicate some work. Really need a worklist of
	// procedures not yet processed.
	// Define a workset for the procedures who have to have their returns checked
	std::set<UserProc*> removeRetSet;
	std::list<Proc*>::iterator pp;
	for (pp = m_procs.begin(); pp != m_procs.end(); ++pp) {
		UserProc* proc = (UserProc*)(*pp);
		if (proc->isLib()) continue;
		if (!proc->isDecoded()) continue;		// e.g. use -sf file to just prototype the proc
		removeRetSet.insert(proc);
	}
	std::set<UserProc*>::iterator it;
	while (removeRetSet.size()) {
		it = removeRetSet.begin();		// Pick the first element of the set
		(*it)->removeUnusedReturns(removeRetSet);
		// Note: removing the currently processed item here should prevent unnecessary reprocessing of self recursive
		// procedures
		removeRetSet.erase(it);			// Remove the current element (may no longer be the first)
	}
}
#endif


#if 0		// For time being, this is in UserProc::generateCode()
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
		if (Boomerang::get()->vFlag) {
			LOG << "===== Before transformation from SSA form for " << proc->getName() << " =====\n";
			proc->printToLog();
			LOG << "===== End before transformation from SSA for " << proc->getName() << " =====\n\n";
			proc->printDFG();
		}
		proc->fromSSAform();
		if (Boomerang::get()->vFlag) {
			LOG << "===== after transformation from SSA form for " << proc->getName() << " =====\n";
			proc->printToLog();
			LOG << "===== end after transformation from SSA for " << proc->getName() << " =====\n\n";
		}
	}
}

void Prog::conTypeAnalysis() {
	if (VERBOSE || DEBUG_TA)
		LOG << "=== start constraint-based type analysis ===\n";
	// FIXME: This needs to be done bottom of the call-tree first, with repeat until no change for cycles
	// in the call graph
	std::list<Proc*>::iterator pp;
	for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
		UserProc* proc = (UserProc*)(*pp);
		if (proc->isLib()) continue;
		if (!proc->isDecoded()) continue;
		proc->conTypeAnalysis();
	}
	if (VERBOSE || DEBUG_TA)
		LOG << "=== end type analysis ===\n";
}

void Prog::globalTypeAnalysis() {
	if (VERBOSE || DEBUG_TA)
		LOG << "### start data-flow-based type analysis ###\n";
	std::list<Proc*>::iterator pp;
	for (pp = m_procs.begin(); pp != m_procs.end(); pp++) {
		UserProc* proc = (UserProc*)(*pp);
		if (proc->isLib()) continue;
		if (!proc->isDecoded()) continue;
		proc->typeAnalysis();
	}
	if (VERBOSE || DEBUG_TA)
		LOG << "### end type analysis ###\n";
}


void Prog::printCallGraph() {
	std::string fname = Boomerang::get()->getOutputPath() + "callgraph.out";
	int fd = lockFileWrite(fname.c_str());
	std::ofstream f(fname.c_str());
	std::set<Proc*> seen;
	std::map<Proc*, int> spaces;
	std::map<Proc*, Proc*> parent;
	std::list<Proc*> procList;
	std::list<UserProc*>::iterator pp;
	for (pp = entryProcs.begin(); pp != entryProcs.end(); ++pp)
		procList.push_back(*pp);
	spaces[procList.front()] = 0;
	while (procList.size()) {
		Proc *p = procList.front();
		procList.erase(procList.begin());
		if ((unsigned)p == NO_ADDRESS)
			continue;
		if (seen.find(p) == seen.end()) {
			seen.insert(p);
			int n = spaces[p];
			for (int i = 0; i < n; i++)
				f << "	 ";
			f << p->getName() << " @ " << std::hex << p->getNativeAddress();
			if (parent.find(p) != parent.end())
				f << " [parent=" << parent[p]->getName() << "]"; 
			f << std::endl;
			if (!p->isLib()) {
				n++;
				UserProc *u = (UserProc*)p;
				std::list<Proc*> &calleeList = u->getCallees();
				for (std::list<Proc*>::reverse_iterator it1 = calleeList.rbegin(); it1 != calleeList.rend(); it1++) {
					procList.push_front(*it1);
					spaces[*it1] = n;
					parent[*it1] = p;
				}
			}
		}
	}
	f.close();
	unlockFile(fd);
}

void printProcsRecursive(Proc* proc, int indent, std::ofstream &f,std::set<Proc*> &seen)
{
    bool fisttime=false;
	if (seen.find(proc) == seen.end()) {
		seen.insert(proc);
		fisttime=true;
	}
	for (int i = 0; i < indent; i++)
		f << "	 ";

	if(!proc->isLib() && fisttime) // seen lib proc
	{
		f << "0x" << std::hex << proc->getNativeAddress();
		f << " __nodecode __incomplete void " << proc->getName() << "();\n";

    	UserProc *u = (UserProc*)proc;
    	std::list<Proc*> &calleeList = u->getCallees();
    	for (std::list<Proc*>::iterator it1 = calleeList.begin(); it1 != calleeList.end(); it1++) {
            printProcsRecursive(*it1,indent+1,f,seen);
        }
       	for (int i = 0; i < indent; i++)
   			f << "	 ";
		f << "// End of " << proc->getName() << "\n";
    } else {
        f << "// " << proc->getName() << "();\n";
    }
}

void Prog::printSymbolsToFile() {
    std::cerr << "entering Prog::printSymbolsToFile\n";
	std::string fname = Boomerang::get()->getOutputPath() + "symbols.h";
	int fd = lockFileWrite(fname.c_str());
	std::ofstream f(fname.c_str());

	/* Print procs */
    f << "/* Functions: */\n";
    std::set<Proc*> seen;
	std::list<UserProc*>::iterator pp;
	for (pp = entryProcs.begin(); pp != entryProcs.end(); ++pp)
    	printProcsRecursive(*pp, 0, f, seen);

    f << "/* Leftovers: */\n";
	std::list<Proc*>::iterator it; // don't forget the rest
	for (it = m_procs.begin(); it != m_procs.end(); it++)
		if (!(*it)->isLib() && seen.find(*it) == seen.end()) {
      		printProcsRecursive(*it,0,f,seen);
		}

	f.close();
	unlockFile(fd);
    std::cerr << "leaving Prog::printSymbolsToFile\n";
}

void Prog::printCallGraphXML() {
	if (!Boomerang::get()->dumpXML)
		return;
	std::list<Proc*>::iterator it;
	for (it = m_procs.begin(); it != m_procs.end(); it++)
		(*it)->clearVisited();
	std::string fname = Boomerang::get()->getOutputPath() + "callgraph.xml";
	int fd = lockFileWrite(fname.c_str());
	std::ofstream f(fname.c_str());
	f << "<prog name=\"" << getName() << "\">\n";
	f << "	 <callgraph>\n";
	std::list<UserProc*>::iterator pp;
	for (pp = entryProcs.begin(); pp != entryProcs.end(); ++pp)
		(*pp)->printCallGraphXML(f, 2);
	for (it = m_procs.begin(); it != m_procs.end(); it++) {
		if (!(*it)->isVisited() && !(*it)->isLib()) {
			(*it)->printCallGraphXML(f, 2);
		}
	}
	f << "	 </callgraph>\n";
	f << "</prog>\n";
	f.close();
	unlockFile(fd);
}

void Prog::readSymbolFile(const char *fname) {
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

	for (std::list<Symbol*>::iterator it = par->symbols.begin(); it != par->symbols.end(); it++) {
		if ((*it)->sig) {
			Proc* p = newProc((*it)->sig->getName(), (*it)->addr, pBF->IsDynamicLinkedProcPointer((*it)->addr) ||
				// NODECODE isn't really the right modifier; perhaps we should have a LIB modifier,
				// to specifically specify that this function obeys library calling conventions
				(*it)->mods->noDecode);
			if (!(*it)->mods->incomplete) {
				p->setSignature((*it)->sig->clone());
				p->getSignature()->setForced(true);
			}
		} else {
			const char *nam = (*it)->nam.c_str();
			if (strlen(nam) == 0) {
				nam = newGlobalName((*it)->addr);
			}
			Type *ty = (*it)->ty;
			if (ty == NULL) {
				ty = guessGlobalType(nam, (*it)->addr);
			}
			globals.insert(new Global(ty, (*it)->addr, nam));
		}
	}

	for (std::list<SymbolRef*>::iterator it2 = par->refs.begin(); it2 != par->refs.end(); it2++) {
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
	if (si == NULL)
		return NULL;
	if (type->isCString()) {
		char* str = prog->getStringConstant(uaddr, true);
		if (str) {
			// Make a global string
			return new Const(str);
		}
	} else if (type->isPointer() && ((PointerType*)type)->getPointsTo()->resolvesToFunc()) {
		ADDRESS init = prog->readNative4(uaddr);	
		Proc* dest = prog->findProc(init);
		if (dest)
			// Make a function constant. Back end should know how to emit the correct language-dependent code
			return new Const(dest);
	} else if (type->isArray()) {
		Type *baseType = type->asArray()->getBaseType();
		e = new Terminal(opNil);
		for (int i = (int)type->asArray()->getLength() - 1; i >= 0; --i)
			e = new Binary(opList, prog->readNativeAs(uaddr + i * baseType->getSize()/8, baseType), e);
		LOG << "calculated init for array global: " << e << "\n";
		if (e->getOper() == opNil)
			e = NULL;
	}
	if (e == NULL) 
		e = prog->readNativeAs(uaddr, type);
	return e;
}

void Global::print(std::ostream& os, Prog* prog) {
	Exp* init = getInitialValue(prog);
	os << type << " " << nam << " at " << std::hex << uaddr << std::dec << " initial value " <<
		(init ? init->prints() : "<none>");
}

Exp *Prog::readNativeAs(ADDRESS uaddr, Type *type)
{
	Exp *e = NULL;
	PSectionInfo si = getSectionInfoByAddr(uaddr);
	if (si == NULL)
		return NULL;
	switch(type->getSize()) {
	case 8:
		e = new Const(
			(int)*(char*)(uaddr + si->uHostAddr - si->uNativeAddr));
		break;
	case 16:
		// Note: must respect endianness
		e = new Const(readNative2(uaddr));
		break;
	case 32:
	default:
		// Note: must respect endianness and type
		if (type->isFloat())
			e = new Const(readNativeFloat4(uaddr));
		else
			e = new Const(readNative4(uaddr));
		break;
	case 64:
		if (type->isFloat())
			e = new Const(readNativeFloat8(uaddr));
		else
			e = new Const(readNative8(uaddr));
	}
	return e;
}

void Global::meetType(Type* ty) {
	bool ch;
	type = type->meetWith(ty, ch);
}

void Prog::reDecode(UserProc* proc) {
	std::ofstream os;
	pFE->processProc(proc->getNativeAddress(), proc, os);
}





class ClusterMemo : public Memo {
public:
	ClusterMemo(int mId) : Memo(mId) { }

	std::string name;
	std::vector<Cluster*> children;
	Cluster *parent;
};

Memo *Cluster::makeMemo(int mId)
{
	ClusterMemo *m = new ClusterMemo(mId);
	m->name = name;
	m->children = children;
	m->parent = parent;
	return m;
}

void Cluster::readMemo(Memo *mm, bool dec)
{
	ClusterMemo *m = dynamic_cast<ClusterMemo*>(mm);

	name = m->name;
	children = m->children;
	parent = m->parent;

	for (std::vector<Cluster*>::iterator it = children.begin(); it != children.end(); it++)
		(*it)->restoreMemo(m->mId, dec);
}

class GlobalMemo : public Memo {
public:
	GlobalMemo(int mId) : Memo(mId) { }

	Type *type;
	ADDRESS uaddr;
	std::string nam;
};

Memo *Global::makeMemo(int mId)
{
	GlobalMemo *m = new GlobalMemo(mId);
	m->type = type;
	m->uaddr = uaddr;
	m->nam = nam;

	type->takeMemo(mId);
	return m;
}

void Global::readMemo(Memo *mm, bool dec)
{
	GlobalMemo *m = dynamic_cast<GlobalMemo*>(mm);

	type = m->type;
	uaddr = m->uaddr;
	nam = m->nam;

	type->restoreMemo(m->mId, dec);
}

class ProgMemo : public Memo {
public:
	ProgMemo(int m) : Memo(m) { }

	std::string m_name, m_path;
	std::list<Proc*> m_procs;
	PROGMAP m_procLabels;
	std::set<Global*> globals;
	DataIntervalMap globalMap;
	int m_iNumberedProc;
	Cluster *m_rootCluster;
};

Memo *Prog::makeMemo(int mId)
{
	ProgMemo *m = new ProgMemo(mId);
	m->m_name = m_name;
	m->m_path = m_path;
	m->m_procs = m_procs;
	m->m_procLabels = m_procLabels;
	m->globals = globals;
	m->globalMap = globalMap;
	m->m_iNumberedProc = m_iNumberedProc;
	m->m_rootCluster = m_rootCluster;

	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++)
		(*it)->takeMemo(m->mId);
	m_rootCluster->takeMemo(m->mId);
	for (std::set<Global*>::iterator it = globals.begin(); it != globals.end(); it++)
		(*it)->takeMemo(m->mId);

	return m;
}

void Prog::readMemo(Memo *mm, bool dec)
{
	ProgMemo *m = dynamic_cast<ProgMemo*>(mm);
	m_name = m->m_name;
	m_path = m->m_path;
	m_procs = m->m_procs;
	m_procLabels = m->m_procLabels;
	globals = m->globals;
	globalMap = m->globalMap;
	m_iNumberedProc = m->m_iNumberedProc;
	m_rootCluster = m->m_rootCluster;

	for (std::list<Proc*>::iterator it = m_procs.begin(); it != m_procs.end(); it++)
		(*it)->restoreMemo(m->mId, dec);
	m_rootCluster->restoreMemo(m->mId, dec);
	for (std::set<Global*>::iterator it = globals.begin(); it != globals.end(); it++)
		(*it)->restoreMemo(m->mId, dec);
}

/*

	After every undoable operation:
		
										 ? ||
		prog->takeMemo();				 1 |.1|

		always deletes any memos before the cursor, adds new memo leaving cursor pointing to new memo


	For first undo:

										 ? |.4321|
		prog->restoreMemo(inc);			 4 |5.4321|

		takes a memo of previous state, always leaves cursor pointing at current state

	For second undo:
										 4 |5.4321|
		prog->restoreMemo(inc);			 3 |54.321|

	To redo:

										 3	|54.321|
		prog->restoreMemo(dec);			 4	|5.4321|
		
 */

void Memoisable::takeMemo(int mId)
{
	if (cur_memo != memos.end() && (*cur_memo)->mId == mId && mId != -1)
		return;

	if (cur_memo != memos.begin()) {
		std::list<Memo*>::iterator it = memos.begin();
		while (it != cur_memo)
			it = memos.erase(it);
	}

	if (mId == -1) {
		if (cur_memo == memos.end())
			mId = 1;
		else
			mId = memos.front()->mId + 1;
	}

	Memo *m = makeMemo(mId);

	memos.push_front(m);
	cur_memo = memos.begin();
}

void Memoisable::restoreMemo(int mId, bool dec)
{
	if (memos.begin() == memos.end())
		return;

	if ((*cur_memo)->mId == mId && mId != -1)
		return;

	if (dec) {
		if (cur_memo == memos.begin())
			return;
		cur_memo--;
	} else {
		cur_memo++;
		if (cur_memo == memos.end()) {
			cur_memo--;
			return;
		}
	}

	Memo *m = *cur_memo;
	if (m->mId != mId && mId != -1)
		return;

	readMemo(m, dec);
}


bool Memoisable::canRestore(bool dec)
{
	if (memos.begin() == memos.end())
		return false;

	if (dec) {
		if (cur_memo == memos.begin())
			return false;
	} else {
		cur_memo++;
		if (cur_memo == memos.end()) {
			cur_memo--;
			return false;
		}
		cur_memo--;
	}
	return true;
}

void Memoisable::takeMemo()
{
	takeMemo(-1);
}

void Memoisable::restoreMemo(bool dec)
{
	restoreMemo(-1, dec);
}
