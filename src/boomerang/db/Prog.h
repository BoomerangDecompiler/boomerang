#pragma once

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

/*========================================================================*//**
* \file        prog.h
* OVERVIEW:    interface for the program object.
******************************************************************************/

#include <map>

#include "boomerang/core/BinaryFileFactory.h"
#include "boomerang/type/Type.h"
#include "boomerang/db/SymTab.h"
#include "boomerang/db/Module.h"
#include "boomerang/util/Util.h"
#include "boomerang/loader/IBinaryFile.h"

#include "boomerang/frontend/Frontend.h"

// TODO: refactor Prog Global handling into separate class
class RTLInstDict;
class Function;
class UserProc;
class LibProc;
class Signature;
class Statement;
class InstructionSet;
class Module;
class IBinarySection;
class ICodeGenerator;
class Global;
struct BinarySymbol;


class Prog : public QObject
{
    Q_OBJECT

public:
    /// The type for the list of functions.
    typedef std::list<Module *>          ModuleList;
    typedef ModuleList::iterator         iterator;
    typedef ModuleList::const_iterator   const_iterator;

public:
    typedef std::map<Address, BinarySymbol *> AddressToSymbolMap;

public:
    Prog(const QString& name);
    virtual ~Prog();

    void setFrontEnd(IFrontEnd *fe);

    IFrontEnd *getFrontEnd() const { return m_defaultFrontend; }

    /// Assign a name to this program
    void setName(const QString& name);

    /***************************************************************************/ /**
     * \note     Formally Frontend::newProc
     * \brief    Create a new unnamed Proc at address \p addr.
     *
     * Call this function when a procedure is discovered (usually by
     * decoding a call instruction). That way, it is given a name
     * that can be displayed in the dot file, etc. If we assign it
     * a number now, then it will retain this number always
     * \param addr    Native address of the procedure entry point
     * \returns       Pointer to the Proc object, or 0 if this is a deleted (not to
     *                be decoded) address
     ******************************************************************************/
    Function *createProc(Address addr);

    void removeProc(const QString& name);

    QString getName() const { return m_name; } ///< Get the name of this program

    QString getPath() const { return m_path; }
    QString getPathAndName() const { return m_path + m_name; }

    /***************************************************************************/ /**
     *
     * \brief    Return the number of user (non deleted, non library) procedures
     * \returns  The number of procedures
     ******************************************************************************/
    int getNumProcs(bool user_only = true) const;

    /***************************************************************************/ /**
    * \brief    Return a pointer to the associated Proc object, or nullptr if none
    * \note        Could return -1 for a deleted Proc
    * \param uAddr - Native address of the procedure entry point
    * \returns Pointer to the Proc object, or 0 if none, or -1 if deleted
    ******************************************************************************/
    Function *findProc(Address uAddr) const;

    /***************************************************************************/ /**
    * \brief    Return a pointer to the associated Proc object, or nullptr if none
    * \note        Could return -1 for a deleted Proc
    * \param name - name of the searched-for procedure
    * \returns Pointer to the Proc object, or 0 if none, or -1 if deleted
    ******************************************************************************/
    Function *findProc(const QString& name) const;

    /***************************************************************************/ /**
     * \brief    Return a pointer to the Proc object containing uAddr, or 0 if none
     * \note     Could return nullptr for a deleted Proc
     * \param uAddr - Native address to search for
     * \returns       Pointer to the Proc object, or 0 if none, or -1 if deleted
     ******************************************************************************/
    Function *findContainingProc(Address uAddr) const;

    /***************************************************************************/ /**
     * \brief    Return true if this is a real procedure
     * \param addr   Native address of the procedure entry point
     * \returns      True if a real (non deleted) proc
     ******************************************************************************/
    bool isProcLabel(Address addr) const;

    /***************************************************************************/ /**
     * \brief Get the name for the progam, without any path at the front
     * \returns A string with the name
     ******************************************************************************/
    QString getNameNoPath() const;

    /***************************************************************************/ /**
     * \brief Get the name for the progam, without any path at the front, and no extension
     * \sa Prog::getNameNoPath
     * \returns A string with the name
     ******************************************************************************/
    QString getNameNoPathNoExt() const;
    UserProc *getFirstUserProc(std::list<Function *>::iterator& it) const;
    UserProc *getNextUserProc(std::list<Function *>::iterator& it) const;

    /// clear the prog object
    /// \note deletes everything!
    void clear();

    /***************************************************************************/ /**
     * \brief    Lookup the given native address in the code section,
     *           returning a host pointer corresponding to the same address
     *
     * \param uAddr Native address of the candidate string or constant
     * \param last  will be set to one past end of the code section (host)
     * \param delta will be set to the difference between the host and native addresses
     * \returns     Host pointer if in range; nullptr if not
     *              Also sets 2 reference parameters (see above)
     ******************************************************************************/
    const void *getCodeInfo(Address uAddr, const char *& last, int& delta) const;

    const std::set<Global*>& getGlobals() const { return m_globals; }

    QString getRegName(int idx) const { return m_defaultFrontend->getRegName(idx); }
    int getRegSize(int idx) const { return m_defaultFrontend->getRegSize(idx); }

    /***************************************************************************/ /**
     * \brief    Decode from entry point given as an agrument
     * \param a -  Native address of the entry point
     ******************************************************************************/
    void decodeEntryPoint(Address a);

    /***************************************************************************/ /**
     * \brief    Add entry point given as an agrument to the list of entryProcs
     * \param a -  Native address of the entry point
     ******************************************************************************/
    void setEntryPoint(Address a);
    void decodeEverythingUndecoded();
    void decodeFragment(UserProc *proc, Address a);

    /// Re-decode this proc from scratch
    void reDecode(UserProc *proc);

    /// Well form all the procedures/cfgs in this program
    bool wellForm() const;

    /// last fixes after decoding everything
    void finishDecode();

    /// Do the main non-global decompilation steps
    void decompile();

    /// As the name suggests, removes globals unused in the decompiled code.
    void removeUnusedGlobals();
    void removeRestoreStmts(InstructionSet& rs);
    void globalTypeAnalysis();

    /***************************************************************************/ /**
     * \brief    Remove unused return locations
     *
     * This is the global removing of unused and redundant returns. The initial idea
     * is simple enough: remove some returns according to the formula:
     * returns(p) = modifieds(p) isect union(live at c) for all c calling p.
     * However, removing returns reduces the uses, leading to three effects:
     * 1) The statement that defines the return, if only used by that return, becomes unused
     * 2) if the return is implicitly defined, then the parameters may be reduced, which affects all callers
     * 3) if the return is defined at a call, the location may no longer be live at the call. If not, you need to check
     *    the child, and do the union again (hence needing a list of callers) to find out if this change also affects that
     *    child.
     * \returns true if any change
     ******************************************************************************/
    bool removeUnusedReturns();

    /// Have to transform out of SSA form after the above final pass
    /// Convert from SSA form
    void fromSSAform();

    /// Constraint based type analysis
    void conTypeAnalysis();
    void dfaTypeAnalysis();
    void rangeAnalysis();

    /// Generate dotty file
    void generateDotFile() const;
    void generateRTL(Module *cluster = nullptr, UserProc *proc = nullptr) const;

    /// Print this program (primarily for debugging)
    void print(QTextStream& out) const;

    /// lookup a library procedure by name; create if does not exist
    LibProc *getLibraryProc(const QString& nam) const;
    Signature *getLibSignature(const QString& name) const;
    Statement *getStmtAtLex(Module *cluster, unsigned int begin, unsigned int end) const;

    /// Get the front end id used to make this prog
    Platform getFrontEndId() const;

    std::shared_ptr<Signature> getDefaultSignature(const char *name) const;

    std::vector<SharedExp>& getDefaultParams();

    std::vector<SharedExp>& getDefaultReturns();

    /// Returns true if this is a win32 program
    bool isWin32() const;

    /// Get a global variable if possible, looking up the loader's symbol table if necessary
    QString getGlobalName(Address uaddr) const;

    /// Get a named global variable if possible, looking up the loader's symbol table if necessary
    Address getGlobalAddr(const QString& nam) const;
    Global *getGlobal(const QString& nam) const;

    /// Make up a name for a new global at address \a uaddr
    /// (or return an existing name if address already used)
    QString newGlobalName(Address uaddr);

    /// Guess a global's type based on its name and address
    SharedType guessGlobalType(const QString& nam, Address u) const;

    /// Make an array type for the global array at u. Mainly, set the length sensibly
    std::shared_ptr<ArrayType> makeArrayType(Address u, SharedType t);

    /// Indicate that a given global has been seen used in the program.
    bool markGlobalUsed(Address uaddr, SharedType knownType = nullptr);

    /// Get the type of a global variable
    SharedType getGlobalType(const QString& nam) const;

    /// Set the type of a global variable
    void setGlobalType(const QString& name, SharedType ty);

    /// Dump the globals to stderr for debugging
    void dumpGlobals() const;

    /// get a string constant at a given address if appropriate
    /// if knownString, it is already known to be a char*
    /// get a string constant at a give address if appropriate
    const char *getStringConstant(Address uaddr, bool knownString = false) const;
    double getFloatConstant(Address uaddr, bool& ok, int bits = 64) const;

    // Hacks for Mike
    /// Get a code for the machine e.g. MACHINE_SPARC
    Machine getMachine() const;

    /// Get a symbol from an address
    QString getSymbolByAddress(Address dest) const;

    const IBinarySection *getSectionInfoByAddr(Address a) const;
    Address getLimitTextLow() const;
    Address getLimitTextHigh() const;

    bool isReadOnly(Address a) const;
    bool isStringConstant(Address a) const;
    bool isCFStringConstant(Address a) const;

    // Read 1, 2, 4, or 8 bytes given a native address
    int readNative1(Address a) const;
    int readNative2(Address a) const;
    int readNative4(Address a) const;
    SharedExp readNativeAs(Address uaddr, SharedType type) const;

    bool isDynamicLinkedProcPointer(Address dest) const;
    const QString& getDynamicProcName(Address uNative) const;

    bool processProc(Address addr, UserProc *proc) // Decode a proc
    {
        QTextStream os(stderr);                    // rtl output target

        return m_defaultFrontend->processProc(addr, proc, os);
    }

    void readSymbolFile(const QString& fname);

    void printSymbolsToFile() const;
    void printCallGraph() const;
    void printCallGraphXML() const;

    Module *getRootModule() const { return m_rootCluster; }
    Module *findModule(const QString& name) const;
    Module *getDefaultModule(const QString& name);
    bool isModuleUsed(Module *c) const;

    /// Add the given RTL to the front end's map from address to aldready-decoded-RTL
    void addDecodedRtl(Address a, RTL *rtl) { m_defaultFrontend->addDecodedRtl(a, rtl); }

    /***************************************************************************/ /**
     * \brief This does extra processing on a constant.
     * The Exp* \a e is expected to be a Const, and the ADDRESS \a lc is the native
     * location from which the constant was read.
     * \returns processed Exp
     ******************************************************************************/
    SharedExp addReloc(SharedExp e, Address lc);


    /// Create or retrieve existing module
    /// \param frontend for the module, if nullptr set it to program's default frontend.
    /// \param fact abstract factory object that creates Module instance
    /// \param name retrieve/create module with this name.
    Module *getOrInsertModule(const QString& name, const ModuleFactory& fact = DefaultModFactory(), IFrontEnd *frontend = nullptr);

    const ModuleList& getModuleList() const { return m_moduleList; }
    ModuleList& getModuleList()       { return m_moduleList; }

    iterator begin()       { return m_moduleList.begin(); }
    const_iterator begin() const { return m_moduleList.begin(); }
    iterator end()         { return m_moduleList.end(); }
    const_iterator end()   const { return m_moduleList.end(); }

    size_t size()  const { return m_moduleList.size(); }
    bool empty() const { return m_moduleList.empty(); }

signals:
    void rereadLibSignatures();

public:
    // Public booleans that are set if and when a register jump or call is
    // found, respectively
    bool bRegisterJump;
    bool bRegisterCall;

protected:
    // list of UserProcs for entry point(s)
    std::list<UserProc *> m_entryProcs;

    IFileLoader *m_loaderIface = nullptr;
    IFrontEnd *m_defaultFrontend; ///< Pointer to the FrontEnd object for the project

    /* Persistent state */
    QString m_name;               // name of the program
    QString m_path;               // its full path
    // FIXME: is a set of Globals the most appropriate data structure? Surely not.
    std::set<Global *> m_globals; ///< globals to print at code generation time
    DataIntervalMap m_globalMap;  ///< Map from address to DataInterval (has size, name, type)
    int m_iNumberedProc;          ///< Next numbered proc will use this
    Module *m_rootCluster;        ///< Root of the cluster tree

    class IBinaryImage *m_image;
    SymTab *m_binarySymbols;

private:
    ModuleList m_moduleList;  ///< The Modules that make up this program
};
