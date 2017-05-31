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
#include "include/BinaryFile.h"
#include "include/frontend.h"
#include "include/type.h"
#include "include/module.h"
#include "include/util.h"

// TODO: refactor Prog Global handling into separate class
class RTLInstDict;
class Function;
class UserProc;
class LibProc;
class Signature;
class Instruction;
class InstructionSet;
class Module;
class XMLProgParser;
struct BinarySymbol;
class HLLCode;

class Global : public Printable {
private:
    SharedType type;
    ADDRESS uaddr;
    QString nam;
    Prog *Parent;
public:
    Global(SharedType _type, ADDRESS _uaddr, const QString &_nam,Prog *_p) : type(_type), uaddr(_uaddr), nam(_nam),Parent(_p) {}
    virtual ~Global();

    SharedType getType() { return type; }
    void setType(SharedType ty) { type = ty; }
    void meetType(SharedType ty);
    ADDRESS getAddress() { return uaddr; }
    bool addressWithinGlobal(ADDRESS addr) {
        // TODO: use getType()->getBytes()
        if (addr == uaddr)
            return true;
        return (addr > uaddr) && (addr <= (uaddr + getType()->getBytes()));
    }
    const QString &getName() const { return nam; }
    SharedExp getInitialValue(Prog *prog) const;
    QString toString() const;

protected:
    Global() : type(nullptr), uaddr(ADDRESS::g(0L)) {}
    friend class XMLProgParser;
}; // class Global

class Prog : public QObject {
    Q_OBJECT
    class IBinaryImage *Image;
    SymTab *BinarySymbols;
public:
    /// The type for the list of functions.
    typedef std::list<Module *>               ModuleListType;
    typedef ModuleListType::iterator                iterator;
    typedef ModuleListType::const_iterator    const_iterator;
private:
    ModuleListType ModuleList;  ///< The Modules that make up this program

public:
    typedef std::map<ADDRESS, BinarySymbol *> mAddressToSymbol;
    //Prog();
    virtual ~Prog();
    Prog(const QString &name);
    void setFrontEnd(FrontEnd *_pFE);
    FrontEnd *getFrontEnd() { return DefaultFrontend; }
    void setName(const char *name);
    Function *setNewProc(ADDRESS uNative);

    void removeProc(const QString &name);
    QString getName(); // Get the name of this program
    QString getPath() { return m_path; }
    QString getPathAndName() { return (m_path + m_name); }
    int getNumProcs(bool user_only=true);

    Function *findProc(ADDRESS uAddr) const;
    Function *findProc(const QString &name) const;
    Function *findContainingProc(ADDRESS uAddr) const;
    bool isProcLabel(ADDRESS addr);
    QString getNameNoPath() const;
    QString getNameNoPathNoExt() const;
    UserProc *getFirstUserProc(std::list<Function *>::iterator &it);
    UserProc *getNextUserProc(std::list<Function *>::iterator &it);

    void clear();
    const void *getCodeInfo(ADDRESS uAddr, const char *&last, int &delta);
    QString getRegName(int idx) const { return DefaultFrontend->getRegName(idx); }
    int getRegSize(int idx) { return DefaultFrontend->getRegSize(idx); }

    void decodeEntryPoint(ADDRESS a);
    void setEntryPoint(ADDRESS a);
    void decodeEverythingUndecoded();
    void decodeFragment(UserProc *proc, ADDRESS a);
    void reDecode(UserProc *proc);
    bool wellForm();
    void finishDecode();
    void decompile();
    void removeUnusedGlobals();
    void removeRestoreStmts(InstructionSet &rs);
    void globalTypeAnalysis();
    bool removeUnusedReturns();
    void fromSSAform();
    void conTypeAnalysis();
    void dfaTypeAnalysis();
    void rangeAnalysis();
    void generateDotFile();
    void generateCode(QTextStream &os);
    void generateCode(Module *cluster = nullptr, UserProc *proc = nullptr, bool intermixRTL = false);
    void generateRTL(Module *cluster = nullptr, UserProc *proc = nullptr);
    void print(QTextStream &out);
    LibProc *getLibraryProc(const QString &nam);
    Signature *getLibSignature(const QString &name);
    Instruction *getStmtAtLex(Module *cluster, unsigned int begin, unsigned int end);
    platform getFrontEndId();


    std::shared_ptr<Signature> getDefaultSignature(const char *name);

    std::vector<SharedExp> &getDefaultParams();
    std::vector<SharedExp> &getDefaultReturns();
    bool isWin32();
    QString getGlobalName(ADDRESS uaddr);
    ADDRESS getGlobalAddr(const QString &nam);
    Global *getGlobal(const QString &nam);
    QString newGlobalName(ADDRESS uaddr);
    SharedType guessGlobalType(const QString &nam, ADDRESS u);
    std::shared_ptr<ArrayType> makeArrayType(ADDRESS u, SharedType t);
    bool globalUsed(ADDRESS uaddr, SharedType knownType = nullptr);
    SharedType getGlobalType(const QString &nam);
    void setGlobalType(const QString &name, SharedType ty);
    void dumpGlobals();
    const char *getStringConstant(ADDRESS uaddr, bool knownString = false);
    double getFloatConstant(ADDRESS uaddr, bool &ok, int bits = 64);

    // Hacks for Mike
    //! Get a code for the machine e.g. MACHINE_SPARC
    MACHINE getMachine() { return pLoaderIface->getMachine(); }
    //! Get a symbol from an address
    QString symbolByAddress(ADDRESS dest);

    const IBinarySection *getSectionInfoByAddr(ADDRESS a);
    ADDRESS getLimitTextLow();
    ADDRESS getLimitTextHigh();
    bool isReadOnly(ADDRESS a);
    bool isStringConstant(ADDRESS a);
    bool isCFStringConstant(ADDRESS a);

    // Read 1, 2, 4, or 8 bytes given a native address
    int readNative1(ADDRESS a);
    int readNative2(ADDRESS a);
    int readNative4(ADDRESS a);
    SharedExp readNativeAs(ADDRESS uaddr, SharedType type);

    bool isDynamicLinkedProcPointer(ADDRESS dest);
    const QString &GetDynamicProcName(ADDRESS uNative);

    bool processProc(ADDRESS addr, UserProc *proc) { // Decode a proc
        QTextStream os(stderr); // rtl output target
        return DefaultFrontend->processProc(addr, proc, os);
    }
    void readSymbolFile(const QString &fname);
    size_t getImageSize() { return pLoaderIface->getImageSize(); }
    ADDRESS getImageBase() { return pLoaderIface->getImageBase(); }
    void printSymbolsToFile();
    void printCallGraph();
    void printCallGraphXML();

    Module *getRootCluster() { return m_rootCluster; }
    Module *findModule(const QString &name);
    Module *getDefaultModule(const QString &name);
    bool moduleUsed(Module *c);

    //! Add the given RTL to the front end's map from address to aldready-decoded-RTL
    void addDecodedRtl(ADDRESS a, RTL *rtl) { DefaultFrontend->addDecodedRtl(a, rtl); }

    SharedExp addReloc(SharedExp e, ADDRESS lc);

    // Public booleans that are set if and when a register jump or call is
    // found, respectively
    bool bRegisterJump;
    bool bRegisterCall;
    // list of UserProcs for entry point(s)
    std::list<UserProc *> entryProcs;

    Module *getOrInsertModule(const QString &name, const ModuleFactory &fact=DefaultModFactory(), FrontEnd *frontend=nullptr);

    const ModuleListType &  getModuleList() const { return ModuleList; }
    ModuleListType       &  getModuleList()       { return ModuleList; }

    iterator                begin()       { return ModuleList.begin(); }
    const_iterator          begin() const { return ModuleList.begin(); }
    iterator                end  ()       { return ModuleList.end();   }
    const_iterator          end  () const { return ModuleList.end();   }
    size_t                  size()  const { return ModuleList.size(); }
    bool                    empty() const { return ModuleList.empty(); }
    void generateDataSectionCode(QString section_name, ADDRESS section_start, uint32_t size, HLLCode *code);
signals:
    void rereadLibSignatures();

protected:
    QObject *pLoaderPlugin; //!< Pointer to the instance returned by loader plugin
    LoaderInterface *pLoaderIface = nullptr;
    FrontEnd *DefaultFrontend; //!< Pointer to the FrontEnd object for the project

    /* Persistent state */
    QString m_name;            // name of the program
    QString m_path;            // its full path
    // FIXME: is a set of Globals the most appropriate data structure? Surely not.
    std::set<Global *> globals; //!< globals to print at code generation time
    DataIntervalMap globalMap;  //!< Map from address to DataInterval (has size, name, type)
    int m_iNumberedProc;        //!< Next numbered proc will use this
    Module *m_rootCluster;     //!< Root of the cluster tree

    friend class XMLProgParser;
}; // class Prog
