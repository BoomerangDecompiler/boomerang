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
#include "dataflow.h"
#include "exp.h"
#include "cfg.h"
#include "proc.h"
#include "rtl.h"
#include "BinaryFile.h"
#include "frontend.h"

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

    // load/save the current program, project/location must be set.
    void        load();
    void        save();

    // serialize the program
    bool serialize(std::ostream &ouf, int &len);
    // deserialize the program
    void deserialize(std::istream &inf);

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

    // Calculate fowrard-flow global dataflow for all procs (i.e. reaching and
    // available definitions, as one large dataflow problem).
    // Similar to the [SW93] paper
    void forwardGlobalDataflow();

    // Calculate reverse-flow global dataflow for all procs (i.e. live and
    // dead locations, as one large dataflow problem).
    // Very similar to the [SW93] paper
    void reverseGlobalDataflow();

    // Do decompilation
    void decompile();

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
    const char *getFrontEndId();

    // Returns true if this is a win32 program
    bool isWin32();

    // Get a global variable if possible
    const char *getGlobal(ADDRESS uaddr);

    // Make a global variable
    void makeGlobal(ADDRESS uaddr, const char *name);

    // get a string constant at a give address if appropriate
    char *getStringConstant(ADDRESS uaddr);

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

    // Public booleans that are set if and when a register jump or call is
    // found, respectively
    bool        bRegisterJump;
    bool        bRegisterCall;

protected:
    // Pointer to the BinaryFile object for the program
    BinaryFile* pBF;
    // Pointer to the FrontEnd object for the project
    FrontEnd *pFE;
    // Map of addresses to global symbols
    std::map<ADDRESS, const char*> *globalMap;
    std::string      m_name;            // name of the program
    std::list<Proc*> m_procs;           // list of procedures
    PROGMAP     m_procLabels;           // map from address to Proc*
    ProgWatcher *m_watcher;             // used for status updates
    // Next numbered proc will use this
    int m_iNumberedProc;
    // A map to make fixing dataflow much easier/faster
    std::map<Exp*, Statement*, lessExpStar> memOfAssigns;
}; 

#endif
