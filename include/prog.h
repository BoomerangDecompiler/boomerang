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

/*=============================================================================
 * FILE:		prog.h
 * OVERVIEW:	interface for the program object.
 *============================================================================*/
/*
 * $Revision$	// 1.73.2.5
 * 16 Apr 01 - Mike: Mods for boomerang
 */

#ifndef _PROG_H_
#define _PROG_H_

#include <map>
#include "BinaryFile.h"
#include "frontend.h"
#include "type.h"
#include "cluster.h"

class RTLInstDict;
class Proc;
class UserProc;
class LibProc;
class Signature;
class Statement;
class StatementSet;
class Cluster;
class XMLProgParser;

typedef std::map<ADDRESS, Proc*, std::less<ADDRESS> > PROGMAP;

class Global {
private:
	Type *type;
	ADDRESS uaddr;
	std::string nam;

public:
					Global(Type *type, ADDRESS uaddr, const char *nam) : type(type), uaddr(uaddr), nam(nam) { }
virtual				~Global();

		Type		*getType() { return type; }
		void  		setType(Type* ty) { type = ty; }
		void  		meetType(Type* ty);
		ADDRESS		getAddress() { return uaddr; }
		const char *getName() { return nam.c_str(); }
		Exp*		getInitialValue(Prog* prog);	// Get the initial value as an expression
													// (or NULL if not initialised)
		void		print(std::ostream& os, Prog* prog);	// Print to stream os

protected:
					Global() : type(NULL), uaddr(0), nam("") { }
		friend class XMLProgParser;
};		// class Global

class Prog {
public:
					Prog();							// Default constructor
virtual				~Prog();
					Prog(const char* name);			// Constructor with name
		void		setFrontEnd(FrontEnd* fe);
		void		setName(const char *name);		// Set the name of this program
		Proc*		setNewProc(ADDRESS uNative);	// Set up new proc
		// Return a pointer to a new proc
		Proc*		newProc(const char* name, ADDRESS uNative, bool bLib = false);
		void		remProc(UserProc* proc);		// Remove the given UserProc
		char*		getName();						// Get the name of this program
		const char *getPath() { return m_path.c_str(); }
		const char *getPathAndName() {return (m_path+m_name).c_str(); }
		int			getNumProcs();					// # of procedures stored in prog
		int			getNumUserProcs();				// # of user procedures stored in prog
		Proc*		getProc(int i) const;			// returns pointer to indexed proc
		// Find the Proc with given address, NULL if none, -1 if deleted
		Proc*		findProc(ADDRESS uAddr) const;
		// Find the Proc with the given name
		Proc*		findProc(const char *name) const;
		// Find the Proc that contains the given address
		Proc*		findContainingProc(ADDRESS uAddr) const;
		bool		isProcLabel (ADDRESS addr); 	// Checks if addr is a label or not
		// Create a dot file for all CFGs
		bool		createDotFile(const char*, bool bMainOnly = false) const;
		// get the filename of this program
		std::string	 getNameNoPath() const;
		std::string  getNameNoPathNoExt() const;
		// This pair of functions allows the user to iterate through all the procs
		// The procs will appear in order of native address
		Proc*		getFirstProc(PROGMAP::const_iterator& it);
		Proc*		getNextProc(PROGMAP::const_iterator& it);

		// This pair of functions allows the user to iterate through all the UserProcs
		// The procs will appear in topdown order
		UserProc*	getFirstUserProc(std::list<Proc*>::iterator& it);
		UserProc*	getNextUserProc (std::list<Proc*>::iterator& it);

		// list of UserProcs for entry point(s)
		std::list<UserProc*> entryProcs;	

		// clear the prog object NOTE: deletes everything!
		void		clear();

		// Lookup the given native address in the code section, returning a host pointer corresponding to the same
		// address
		const void* getCodeInfo(ADDRESS uAddr, const char*& last, int& delta);

		const char *getRegName(int idx) { return pFE->getRegName(idx); }
		int getRegSize(int idx) { return pFE->getRegSize(idx); }

		void		decodeEntryPoint(ADDRESS a);
		void		setEntryPoint(ADDRESS a);			// As per the above, but don't decode
		void		decodeEverythingUndecoded();
		void		decodeFragment(UserProc* proc, ADDRESS a);

		// Re-decode this proc from scratch
		void		reDecode(UserProc* proc);

		// Well form all the procedures/cfgs in this program
		bool		wellForm();
		
		// last fixes after decoding everything
		void		finishDecode();

		// Recover return locations
		void		recoverReturnLocs();

		// Remove interprocedural edges
		void		removeInterprocEdges();

		// Do the main non-global decompilation steps
		void		decompile();

		// All that used to be done in UserProc::decompile, but now done globally: propagation, recalc DFA, remove null
		// and unused statements, compressCfg, process constants, promote signature, simplify a[m[]].
		void		decompileProcs();

		// Remove null, unused, and restored statements
		void		removeNullStmts();
		void		removeUnusedStmts();
		void		removeUnusedGlobals();
		void		removeUnusedLocals();
		void		removeRestoreStmts(StatementSet& rs);

		// Process constants
		void		processConstants();

		// Type analysis
		void		globalTypeAnalysis();

		/// Remove unused return locations
		/// \return true if any returns are removed
		bool		removeUnusedReturns();

		// Convert from SSA form
		void		fromSSAform();

		// Type analysis
		void		conTypeAnalysis();
		void		dfaTypeAnalysis();

		// Range analysis
		void		rangeAnalysis();

		// Generate dotty file
		void		generateDotFile();

		// Generate code
		void		generateCode(std::ostream &os);
		void		generateCode(Cluster *cluster = NULL, UserProc *proc = NULL, bool intermixRTL = false);
		void		generateRTL(Cluster *cluster = NULL, UserProc *proc = NULL);

		// Print this program (primarily for debugging)
		void		print(std::ostream &out);

		// lookup a library procedure by name; create if does not exist
		LibProc		*getLibraryProc(const char *nam);

		// Get a library signature for a given name (used when creating a new library proc.
		Signature	*getLibSignature(const char *name);
		void		rereadLibSignatures();

		Statement	*getStmtAtLex(Cluster *cluster, unsigned int begin, unsigned int end);

		// Get the front end id used to make this prog
		platform	getFrontEndId();

		std::map<ADDRESS, std::string> &getSymbols();

		Signature	*getDefaultSignature(const char *name);

		std::vector<Exp*> &getDefaultParams();
		std::vector<Exp*> &getDefaultReturns();

		// Returns true if this is a win32 program
		bool		isWin32();

		// Get a global variable if possible, looking up the loader's symbol table if necessary
		const char	*getGlobalName(ADDRESS uaddr);
		ADDRESS		getGlobalAddr(char *nam);
		Global*		getGlobal(char *nam);

		// Make up a name for a new global at address uaddr (or return an existing name if address already used)
		const char	*newGlobalName(ADDRESS uaddr);

		// Guess a global's type based on its name and address
		Type		*guessGlobalType(const char *nam, ADDRESS u);

		// Make an array type for the global array at u. Mainly, set the length sensibly
		ArrayType*	makeArrayType(ADDRESS u, Type* t);

		// Indicate that a given global has been seen used in the program.
		bool		globalUsed(ADDRESS uaddr, Type* knownType = NULL);

		// Get the type of a global variable
		Type		*getGlobalType(char* nam);
		
		// Set the type of a global variable
		void		setGlobalType(const char* name, Type* ty);

		// Dump the globals to stderr for debugging
		void		dumpGlobals();

		// get a string constant at a give address if appropriate
		char		*getStringConstant(ADDRESS uaddr, bool knownString = false);
		double		getFloatConstant(ADDRESS uaddr, bool &ok, int bits = 64);

		// Hacks for Mike
		MACHINE		getMachine()				// Get a code for the machine
						{ return pBF->GetMachine();}	// e.g. MACHINE_SPARC
		const char*	symbolByAddress(ADDRESS dest) // Get a symbol from an address
						{ return pBF->SymbolByAddress(dest);}
		PSectionInfo getSectionInfoByAddr(ADDRESS a)
						{ return pBF->GetSectionInfoByAddr(a);}
		ADDRESS		getLimitTextLow() {return pBF->getLimitTextLow();}
		ADDRESS		getLimitTextHigh() {return pBF->getLimitTextHigh();}
		bool		isReadOnly(ADDRESS a) { return pBF->isReadOnly(a); }
		// Read 2, 4, or 8 bytes given a native address
		int			readNative1(ADDRESS a) {return pBF->readNative1(a);}
		int			readNative2(ADDRESS a) {return pBF->readNative2(a);}
		int			readNative4(ADDRESS a) {return pBF->readNative4(a);}
		float		readNativeFloat4(ADDRESS a) {return pBF->readNativeFloat4(a);}
		double		readNativeFloat8(ADDRESS a) {return pBF->readNativeFloat8(a);}
		QWord		readNative8(ADDRESS a) {return pBF->readNative8(a);}
		Exp	  		*readNativeAs(ADDRESS uaddr, Type *type);
		int			getTextDelta() { return pBF->getTextDelta(); }

		bool		isDynamicLinkedProcPointer(ADDRESS dest) { return pBF->IsDynamicLinkedProcPointer(dest); }
		const char*	GetDynamicProcName(ADDRESS uNative) { return pBF->GetDynamicProcName(uNative); }

		bool		processProc(int addr, UserProc* proc)	// Decode a proc
						{ std::ofstream os; return pFE->processProc((unsigned)addr, proc, os);}

		void		readSymbolFile(const char *fname);
		unsigned	getImageSize() { return pBF->getImageSize(); }
		ADDRESS		getImageBase() { return pBF->getImageBase(); }

		// Public booleans that are set if and when a register jump or call is
		// found, respectively
		bool		bRegisterJump;
		bool		bRegisterCall;

		void		printSymbolsToFile();
		void		printCallGraph();
		void		printCallGraphXML();

		Cluster		*getRootCluster() { return m_rootCluster; }
		Cluster		*findCluster(const char *name) { return m_rootCluster->find(name); }
		Cluster		*getDefaultCluster(const char *name);
		bool		clusterUsed(Cluster *c);

		// Add the given RTL to the front end's map from address to aldready-decoded-RTL
		void		addDecodedRtl(ADDRESS a, RTL* rtl) {
						pFE->addDecodedRtl(a, rtl); }

		// This does extra processing on a constant.  The Exp* is expected to be a Const,
		// and the ADDRESS is the native location from which the constant was read.
		Exp			*addReloc(Exp *e, ADDRESS lc);

protected:
		BinaryFile*	pBF;					// Pointer to the BinaryFile object for the program
		FrontEnd	*pFE;					// Pointer to the FrontEnd object for the project

		/* Persistent state */
		std::string	m_name, m_path;			// name of the program and its full path
		std::list<Proc*> m_procs;			// list of procedures
		PROGMAP		m_procLabels;			// map from address to Proc*
		// FIXME: is a set of Globals the most appropriate data structure? Surely not.
		std::set<Global*> globals;			// globals to print at code generation time
		//std::map<ADDRESS, const char*> *globalMap; // Map of addresses to global symbols
		DataIntervalMap globalMap;			// Map from address to DataInterval (has size, name, type)
		int			m_iNumberedProc;		// Next numbered proc will use this
		Cluster		*m_rootCluster;			// Root of the cluster tree

		friend class XMLProgParser;
};	// class Prog

#endif
