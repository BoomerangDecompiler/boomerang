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
 * 16 Apr 01 - Mike: Mods for boomerang
 */

#ifndef _PROG_H_
#define _PROG_H_

#include <map>
#include "BinaryFile.h"
#include "frontend.h"
#include "type.h"

class RTLInstDict;
class Proc;
class UserProc;
class LibProc;
class Signature;
class StatementSet;

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

class Global {
private:
    Type *type;
    ADDRESS uaddr;
    std::string nam;

public:
    Global(Type *type, ADDRESS uaddr, const char *nam) : type(type), 
                                                         uaddr(uaddr),
                                                         nam(nam) { }
    ~Global() { 
        if (type) 
            delete type; 
    }

    Type *getType() { return type; }
    void  setType(Type* ty) { type = ty; }
    ADDRESS getAddress() { return uaddr; }
    const char *getName() { return nam.c_str(); }
};

class Prog {
    // Phase of the interprocedural DFA (0=none, 1=phase 1, 2 = phase 2)
    int     interProcDFAphase;

public:
            Prog();                     // Default constructor
            Prog(BinaryFile *pBF, FrontEnd *pFE);
            ~Prog();
            Prog(const char* name);     // Constructor with name
    void    setName(const char *name);      // Set the name of this program
    Proc*   setNewProc(ADDRESS uNative);    // Set up new proc
    // Return a pointer to a new proc
    Proc*   newProc(const char* name, ADDRESS uNative, bool bLib = false);
    void    remProc(UserProc* proc);    // Remove the given UserProc
    char*   getName();                  // Get the name of this program
    int     getNumProcs();              // # of procedures stored in prog
    Proc*   getProc(int i) const;       // returns pointer to indexed proc
    // Find the Proc with given address
    Proc*   findProc(ADDRESS uAddr) const;
    // Find the Proc with the given name
    Proc*   findProc(const char *name) const;
    // Find the Proc that contains the given address
    Proc*   findContainingProc(ADDRESS uAddr) const;
    bool    isProcLabel (ADDRESS addr); // Checks if addr is a label or not
    // Create a dot file for all CFGs
    bool    createDotFile(const char*, bool bMainOnly = false) const;
    // get the filename of this program
    std::string  getNameNoPath() const;
    // This pair of functions allows the user to iterate through all the procs
    // The procs will appear in order of native address
    Proc*   getFirstProc(PROGMAP::const_iterator& it);
    Proc*   getNextProc(PROGMAP::const_iterator& it);
    Proc*   getEntryProc() { return m_procs.front(); }

    // This pair of functions allows the user to iterate through all the
    // UserProcs
    // The procs will appear in topdown order
    UserProc*   getFirstUserProc(std::list<Proc*>::iterator& it);
    UserProc*   getNextUserProc (std::list<Proc*>::iterator& it);

    // load/save the current program, project/location must be set.
    void        load();
    void        save();

    // clear the prog object NOTE: deletes everything!
    void        clear();

    // Lookup the given native address in the code section, returning
    // a host pointer corresponding to the same address
    const void* getCodeInfo(ADDRESS uAddr, const char*& last, int& delta);

    // Get the watcher.. other classes (such as the decoder) can alert
    // the watcher when there are changes.        
    ProgWatcher *getWatcher() { return m_watcher; }

    // Indicate that a watcher would like to be updated of status (only 1
    // watcher allowed at the moment, old watchers will be disconnected).
    void setWatcher(ProgWatcher *p) { m_watcher = p; }

    const char *getRegName(int idx) { return pFE->getRegName(idx); }

    void decode(ADDRESS a) { 
        if (findProc(a) == NULL) {
            pFE->decode(this, a);
            analyse();
        }
    }

    // Well form all the procedures/cfgs in this program
    bool wellForm();

    // Analyse any decoded procedures
    void analyse();

    // Insert arguments. Assumes all procedures have their formal parameters
    // recovered
    void insertArguments(StatementSet& rs);

    // Recover return locations
    void recoverReturnLocs();

    // Remove interprocedural edges
    void removeInterprocEdges();

    // Initialise and number all statements globally
    void initStatements();

    // Do decompilation
    void decompile();

    // All that used to be done in UserProc::decompile, but now done globally.
    // propagation, recalc DFA, remove null and unused statements, compressCfg,
    // process constants, promote signature, simplify a[m[]].
    void decompileProcs();

    // Remove null, unused, and restored statements
    void removeNullStmts();
    void removeUnusedStmts();
    void removeUnusedLocals();
    void removeRestoreStmts(StatementSet& rs);

    // Process constants
    void processConstants();

    // Remove unused return locations
    void removeUnusedReturns();

    // Convert from SSA form
    void fromSSAform();

    // Type analysis
    void typeAnalysis();

    // Generate dotty file
    void generateDotFile();

    // Generate code
    void generateCode(std::ostream &os);

    // Print this program (primarily for debugging)
    void print(std::ostream &out, bool withDF = false);

    // lookup a library procedure by name
    LibProc *getLibraryProc(const char *nam);

    // Get a library signature for a given name (used when creating a new
    // library proc.
    Signature *getLibSignature(const char *name);

    // Get the front end id used to make this prog
    platform getFrontEndId();

    Signature *getDefaultSignature(const char *name);

    std::vector<Exp*> &getDefaultParams();
    std::vector<Exp*> &getDefaultReturns();

    // Returns true if this is a win32 program
    bool isWin32();

    // Get a global variable if possible
    const char *getGlobal(ADDRESS uaddr);
    ADDRESS getGlobal(char *nam);

    // Make up a name for a new global at address uaddr
    // (or return an existing name if address already used)
    const char *newGlobal(ADDRESS uaddr);

    // Guess a global's type based on its name
    Type *guessGlobalType(const char *nam);

    // Indicate that a given global has been seen used in the program.
    void globalUsed(ADDRESS uaddr);

    // Get the type of a global variable
    Type *getGlobalType(char* nam);
    
    // Set the type of a global variable
    void setGlobalType(char* name, Type* ty);

    // get a string constant at a give address if appropriate
    char *getStringConstant(ADDRESS uaddr);
    double getFloatConstant(ADDRESS uaddr, bool &ok, int bits = 64);

    // Hacks for Mike
    MACHINE getMachine()                // Get a code for the machine
        { return pBF->GetMachine();}    // e.g. MACHINE_SPARC
    char* symbolByAddress(ADDRESS dest) // Get a symbol from an address
        { return pBF->SymbolByAddress(dest);}
    PSectionInfo getSectionInfoByAddr(ADDRESS a)
        { return pBF->GetSectionInfoByAddr(a);}
    bool processProc(int addr, UserProc* proc)  // Decode a proc
        { std::ofstream os;
          return pFE->processProc((unsigned)addr, proc, os);}
    // Read 2 or 4 bytes given a native address
    int readNative2(ADDRESS a) {return pBF->readNative2(a);}
    int readNative4(ADDRESS a) {return pBF->readNative4(a);}

    void readSymbolFile(const char *fname);

    // Public booleans that are set if and when a register jump or call is
    // found, respectively
    bool        bRegisterJump;
    bool        bRegisterCall;

    void printCallGraph();
    void printCallGraphXML();

protected:
    // Pointer to the BinaryFile object for the program
    BinaryFile* pBF;
    // Pointer to the FrontEnd object for the project
    FrontEnd *pFE;
    // globals to print at code generation time
    std::vector<Global*> globals;
    // Map of addresses to global symbols
    std::map<ADDRESS, const char*> *globalMap;
    std::string      m_name;            // name of the program
    std::list<Proc*> m_procs;           // list of procedures
    PROGMAP     m_procLabels;           // map from address to Proc*
    ProgWatcher *m_watcher;             // used for status updates
    // Next numbered proc will use this
    int m_iNumberedProc;
}; 

#endif
