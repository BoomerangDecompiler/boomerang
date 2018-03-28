#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include <map>

#include "boomerang/type/type/Type.h"
#include "boomerang/type/DataIntervalMap.h"
#include "boomerang/db/binary/BinarySymbolTable.h"
#include "boomerang/db/binary/BinaryFile.h"
#include "boomerang/db/Module.h"
#include "boomerang/util/Util.h"
#include "boomerang/frontend/Frontend.h"


class RTLInstDict;
class Function;
class UserProc;
class LibProc;
class Signature;
class Statement;
class StatementSet;
class Module;
class BinarySection;
class ICodeGenerator;
class Global;
class BinarySymbol;
class BinaryFile;
class Project;


class Prog
{
public:
    /// The type for the list of functions.
    typedef std::list<std::unique_ptr<Module>>  ModuleList;
    typedef std::map<Address, BinarySymbol *>   AddressToSymbolMap;

public:
    Prog(const QString& name, Project *project);
    Prog(const Prog& other) = delete;
    Prog(Prog&& other) = default;

    virtual ~Prog();

    Prog& operator=(const Prog& other) = delete;
    Prog& operator=(Prog&& other) = default;

public:
    /// Change the FrontEnd. Takes ownership of the pointer.
    void setFrontEnd(IFrontEnd *fe);
    IFrontEnd *getFrontEnd() const { return m_defaultFrontend; }

    Project *getProject() { return m_project; }
    const Project *getProject() const { return m_project; }

    /// Assign a new name to this program
    void setName(const QString& name);
    QString getName() const { return m_name; }

    BinaryFile *getBinaryFile() { return m_binaryFile; }
    const BinaryFile *getBinaryFile() const { return m_binaryFile; }

    /**
     * Creates a new empty module.
     * \param name   The name of the new module.
     * \param parent The parent of the new module.
     * \param modFactory Determines the type of Module to be created.
     * \returns the new module, or nullptr if there already exists a module with the same name and parent.
     */
    Module *createModule(const QString& name, Module *parent = nullptr, const ModuleFactory& modFactory = DefaultModFactory());

    /**
     * Create or retrieve existing module
     * \param frontend for the module, if nullptr set it to program's default frontend.
     * \param fact abstract factory object that creates Module instance
     * \param name retrieve/create module with this name.
     */
    Module *getOrInsertModule(const QString& name, const ModuleFactory& fact = DefaultModFactory(), IFrontEnd *frontend = nullptr);

    const ModuleList& getModuleList() const { return m_moduleList; }

    /**
     * Create a new unnamed function at address \p addr.
     * Call this method when a function is discovered (usually by
     * decoding a call instruction). That way, it is given a name
     * that can be displayed in the dot file, etc. If we assign it
     * a number now, then it will retain this number always.
     *
     * \param entryAddr Address of the entry point of the function
     * \returns Pointer to the Function, or nullptr if this is a deleted
     * (not to be decoded) address
     */
    Function *createFunction(Address entryAddr);

    /// Removes the function with name \p name.
    /// If there is no such function, nothing happens.
    void removeFunction(const QString& name);

    /// \param userOnly If true, only count user functions, not lbrary functions.
    /// \returns the number of functions in this program.
    int getNumFunctions(bool userOnly = true) const;

    /// \returns the function with entry address \p entryAddr,
    /// or nullptr if no such function exists.
    Function *findFunction(Address entryAddr) const;

    /// \returns the function with name \p name,
    /// or nullptr if no such function exists.
    Function *findFunction(const QString& name) const;

    QString getRegName(int idx) const { return m_defaultFrontend->getRegName(idx); }
    int getRegSize(int idx) const { return m_defaultFrontend->getRegSize(idx); }

    // globals
    const std::set<std::shared_ptr<Global>>& getGlobals() const { return m_globals; }


    // Decoding

    /// Decode from entry point given as an agrument
    void decodeEntryPoint(Address entryAddr);

    /// If \p entryAddress is the entry address of a function,
    /// add the function to the list of entry points.
    void addEntryPoint(Address entryAddr);

    /// Decode a procedure fragment of \p proc starting at address \p addr.
    void decodeFragment(UserProc *proc, Address addr);

    /// Re-decode this proc from scratch
    bool reDecode(UserProc *proc);

    /// last fixes after decoding everything
    void finishDecode();

    /// Check the wellformedness of all the procedures/cfgs in this program
    bool isWellFormed() const;

    /// Do the main non-global decompilation steps
    void decompile();

    /// Do global type analysis.
    /// \note For now, it just does local type analysis for every procedure of the program.
    void globalTypeAnalysis();

    /// As the name suggests, removes globals unused in the decompiled code.
    void removeUnusedGlobals();

    /**
     * Remove unused return locations.
     * This is the global removing of unused and redundant returns. The initial idea
     * is simple enough: remove some returns according to the formula:
     * returns(p) = modifieds(p) isect union(live at c) for all c calling p.
     *
     * However, removing returns reduces the uses, leading to three effects:
     * 1) The statement that defines the return, if only used by that return, becomes unused
     * 2) if the return is implicitly defined, then the parameters may be reduced, which affects all callers
     * 3) if the return is defined at a call, the location may no longer be live at the call. If not, you need to check
     *    the child, and do the union again (hence needing a list of callers) to find out if this change also affects that
     *    child.
     * \returns true if any change
     */
    bool removeUnusedReturns();

    /// Have to transform out of SSA form after the above final pass
    /// Convert from SSA form
    void fromSSAForm();

    /// lookup a library procedure by name; create if does not exist
    LibProc *getOrCreateLibraryProc(const QString& name);

    /// Get the front end id used to make this prog
    Platform getFrontEndId() const;

    std::shared_ptr<Signature> getDefaultSignature(const char *name) const;

    /// Returns true if this is a win32 program
    bool isWin32() const;

    /// Get a global variable if possible, looking up the loader's symbol table if necessary
    QString getGlobalName(Address addr) const;

    /// Get a named global variable if possible, looking up the loader's symbol table if necessary
    Address getGlobalAddr(const QString& name) const;
    Global *getGlobal(const QString& name) const;

    /// Make up a name for a new global at address \a uaddr
    /// (or return an existing name if address already used)
    QString newGlobalName(Address uaddr);

    /// Guess a global's type based on its name and address
    SharedType guessGlobalType(const QString& name, Address addr) const;

    /// Make an array type for the global array starting at \p startAddr.
    /// Mainly, set the length sensibly
    std::shared_ptr<ArrayType> makeArrayType(Address startAddr, SharedType baseType);

    /// Indicate that a given global has been seen used in the program.
    /// \returns true on success, false on failure (e.g. existing incompatible type already present)
    bool markGlobalUsed(Address uaddr, SharedType knownType = nullptr);

    /// Get the type of a global variable
    SharedType getGlobalType(const QString& name) const;

    /// Set the type of a global variable
    void setGlobalType(const QString& name, SharedType ty);

    /// get a string constant at a given address if appropriate
    /// if knownString, it is already known to be a char*
    /// get a string constant at a give address if appropriate
    const char *getStringConstant(Address uaddr, bool knownString = false) const;
    double getFloatConstant(Address uaddr, bool& ok, int bits = 64) const;

    // Hacks for Mike
    /// Get a code for the machine e.g. MACHINE_SPARC
    Machine getMachine() const;

    /// Get a symbol from an address
    QString getSymbolNameByAddress(Address dest) const;

    const BinarySection *getSectionByAddr(Address a) const;
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
    const QString& getDynamicProcName(Address addr) const;

    void readSymbolFile(const QString& fname);

    void printSymbolsToFile() const;
    void printCallGraph(const QString &fileName = "callgraph.dot") const;

    Module *getRootModule() const { return m_rootModule; }
    Module *findModule(const QString& name) const;

    /// \returns the default module for a symbol with name \p name.
    Module *getModuleForSymbol(const QString& symbolName);
    bool isModuleUsed(Module *module) const;

    /// Add the given RTL to the front end's map from address to already-decoded-RTL
    void addDecodedRTL(Address a, RTL *rtl) { m_defaultFrontend->addDecodedRTL(a, rtl); }

    /**
     * This does extra processing on a constant. The expression \p e
     * is expected to be a Const, and the Address \p location
     * is the native location from which the constant was read.
     * \returns processed Exp
     */
    SharedExp addReloc(SharedExp e, Address location);

    void updateLibrarySignatures();

private:
    QString m_name;             ///< name of the program
    Project *m_project = nullptr;
    BinaryFile *m_binaryFile;
    Module *m_rootModule;       ///< Root of the module tree
    ModuleList m_moduleList;    ///< The Modules that make up this program

    /// list of UserProcs for entry point(s)
    std::list<UserProc *> m_entryProcs;

    IFrontEnd *m_defaultFrontend; ///< Pointer to the FrontEnd object for the project

    // FIXME: is a set of Globals the most appropriate data structure? Surely not.
    std::set<std::shared_ptr<Global>> m_globals; ///< globals to print at code generation time
    DataIntervalMap m_globalMap;  ///< Map from address to DataInterval (has size, name, type)
};
