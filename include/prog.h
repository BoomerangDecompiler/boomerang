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
 * FILE:        prog.h
 * OVERVIEW:    interface for the program object.
 *============================================================================*/
/*
 * $Revision$
 * Created by Mike
 * 24 Mar 98 - Cristina
 *  Changed m_procs to be a list of procedure objects rather than pointers
 *      to procedure objects.  Similar for AddProc() and GetProc().
 *      added map of labels.
 * 26 Mar 98 - Cristina
 *  Changed AddProc() to NewProc() so that we have a reference to the  
 *      procedure and are able to change it during parsing.
 * 28 Jul 98 - Mike
 *  Now depends on PROC, which depends on CFG and RTL
 * 2 Sep 98 - Mike
 *  Added findProc(); m_procs now list of pointers to procs
 * 08 Apr 99 - Mike: Changes for HEADERS
 * 02 Jun 99 - Mike: Removed leading upper case on function names
 * 06 Jun 00 - Mike: Added members csrSrc, pBF
 * 06 Jul 00 - Mike: Added getFirstProc / getNextProc functions
 * 31 Jul 01 - Brian: Added new file hrtl.h for HRTL-related declarations.
 * 30 Aug 01 - Mike: mapLibParam has list (was vector)
 * 16 Apr 01 - Mike: Mods for boomerang
 */

#ifndef _PROG_H_
#define _PROG_H_

#include <map>
#include "exp.h"
#include "proc.h"
#include "rtl.h"

class BinaryFile;
class FrontEnd;
class RTLInstDict;
class Proc;
class UserProc;
class Signature;

typedef std::map<ADDRESS, Proc*, std::less<ADDRESS> > PROGMAP;

class ProgWatcher {
public:
	ProgWatcher() { }

	virtual void alert_complete() = 0;
	virtual void alert_new(Proc *p) = 0;
	virtual void alert_decode(ADDRESS pc, int nBytes) = 0;
	virtual void alert_baddecode(ADDRESS pc) = 0;
	virtual void alert_done(Proc *p, ADDRESS pc, ADDRESS last, int nBytes) = 0;
	virtual void alert_progress(unsigned long off, unsigned long size) = 0;
};

class Prog {
public:
            Prog();                     // Default constructor
			~Prog();
            Prog(const char* name);     // Constructor with name
    void    setName(const char *name);      // Set the name of this program
    Proc*   newProc(const char* name, ADDRESS uNative, bool bLib = false);
                                        // Returns a pointer to a new proc
    void    remProc(UserProc* proc);    // Remove the given UserProc
    char*   getName();                  // Get the name of this program
    int     getNumProcs();              // # of procedures stored in prog
    Proc*   getProc(int i) const;       // returns pointer to indexed proc
    Proc*   findProc(ADDRESS uAddr) const;// Find the Proc with given address
	Proc*   findProc(const char *name) const; // Find the Proc with the given name
	Proc*	findContainingProc(ADDRESS uAddr) const; // Find the Proc that contains the given address
    bool    isProcLabel (ADDRESS addr); // Checks if addr is a label or not
    // Create a dot file for all CFGs
    bool    createDotFile(const char*, bool bMainOnly = false) const;
    void    setArgv0(const char* p);    // Set the argv[0] pointer
    std::string& getProgPath();        // Get path to the translator executable
    std::string  getNameNoPath() const;      // Get the program name with no path
    // This pair of functions allows the user to iterate through all the procs
    // The procs will appear in order of native address
    Proc*   getFirstProc(PROGMAP::const_iterator& it);
    Proc*   getNextProc(PROGMAP::const_iterator& it);

	// load/save the current program, project/location must be set.
	void	load();
	void	save();

	// serialize the program
	bool serialize(std::ostream &ouf, int &len);
	// deserialize the program
	void deserialize(std::istream &inf);

	// clear the prog object NOTE: deletes everything!
	void	clear();

    // Lookup the given native address in the code section, returning
    // a host pointer corresponding to the same address
    const void* getCodeInfo(ADDRESS uAddr, const char*& last, int& delta);

	// Get the watcher.. other classes (such as the decoder) can alert
	// the watcher when there are changes.	
	ProgWatcher *getWatcher() { return m_watcher; }

	// Indicate that a watcher would like to be updated of status (only 1
	// watcher allowed at the moment, old watchers will be disconnected).
	void setWatcher(ProgWatcher *p) { m_watcher = p; }

	// Well form all the procedures/cfgs in this program
	bool wellForm();

	// Analyse any decoded procedures
	void analyse();

        // Print this program (primarily for debugging)
        void print(std::ostream &out);

	// map for global symbols
	std::map<std::string, TypedExp *> symbols;

	// search for a symbol which matches an expression
	bool findSymbolFor(Exp *e, std::string &sym, TypedExp* &sym_exp);

    // lookup a library procedure by name
    LibProc *getLibraryProc(const char *nam);

    // Pointer to the BinaryFile object for the program, which contains the
    // program image. Created in main()
    BinaryFile* pBF;

	// Pointer to the FrontEnd object for the project, which is used to decode
	// procedures.
	FrontEnd *pFE;

	// The filename being decompiled
	std::string filename;

	// The name of the project
	std::string project;

	// The full location of the project file
	std::string location;

    // Public object that keeps track of the coverage of the source program's
    // text segment
    Coverage cover;

    // Public booleans that are set if and when a register jump or call is
    // found, respectively
    bool        bRegisterJump;
    bool        bRegisterCall;

protected:
    std::string      m_name;                 // name of the executable
    std::list<Proc*> m_procs;                // list of procedures
    PROGMAP     m_procLabels;           // map from address to Proc*
    std::string      m_progPath;             // String with the path to this exec
	ProgWatcher *m_watcher;				// used for status updates
}; 

#endif
