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
 *============================================================================*/
#ifndef _PROG_H_
#define _PROG_H_

#include <map>
#include "BinaryFile.h"
#include "frontend.h"
#include "type.h"
#include "cluster.h"
// TODO: refactor Prog Global handling into separate class
class RTLInstDict;
class Function;
class UserProc;
class LibProc;
class Signature;
class Statement;
class StatementSet;
class Cluster;
class XMLProgParser;

typedef std::map<ADDRESS, Function *, std::less<ADDRESS>> PROGMAP;

class Global {
  private:
    Type *type;
    ADDRESS uaddr;
    std::string nam;

  public:
    Global(Type *_type, ADDRESS _uaddr, const std::string &_nam) : type(_type), uaddr(_uaddr), nam(_nam) {}
    virtual ~Global();

    Type *getType() { return type; }
    void setType(Type *ty) { type = ty; }
    void meetType(Type *ty);
    ADDRESS getAddress() { return uaddr; }
    bool addressWithinGlobal(ADDRESS addr) {
        // TODO: use getType()->getBytes()
        if (addr == uaddr)
            return true;
        return (addr > uaddr) && addr <= (uaddr + (getType()->getSize() / 8));
    }
    const char *getName() { return nam.c_str(); }
    Exp *getInitialValue(Prog *prog);
    void print(std::ostream &os, Prog *prog); // Print to stream os

  protected:
    Global() : type(nullptr), uaddr(ADDRESS::g(0L)), nam("") {}
    friend class XMLProgParser;
}; // class Global

class Prog {
  public:
    typedef std::map<ADDRESS, std::string> mAddressString;
    Prog();
    virtual ~Prog();
    Prog(const char *name);
    void setFrontEnd(FrontEnd *fe);
    void setName(const char *name);
    Function *setNewProc(ADDRESS uNative);
    Function *newProc(const char *name, ADDRESS uNative, bool bLib = false);
    Function *newProc(const QString &name, ADDRESS uNative, bool bLib = false) {
        return newProc(qPrintable(name), uNative, bLib);
    }

    void remProc(UserProc *proc);
    void removeProc(const QString &name);
    QString getName(); // Get the name of this program
    QString getPath() { return m_path; }
    QString getPathAndName() { return (m_path + m_name); }
    int getNumProcs();
    int getNumUserProcs();
    Function *getProc(int i) const;
    Function *findProc(ADDRESS uAddr) const;
    Function *findProc(const QString &name) const;
    Function *findContainingProc(ADDRESS uAddr) const;
    bool isProcLabel(ADDRESS addr);
    std::string getNameNoPath() const;
    std::string getNameNoPathNoExt() const;
    Function *getFirstProc(PROGMAP::const_iterator &it);
    Function *getNextProc(PROGMAP::const_iterator &it);
    UserProc *getFirstUserProc(std::list<Function *>::iterator &it);
    UserProc *getNextUserProc(std::list<Function *>::iterator &it);

    void clear();
    const void *getCodeInfo(ADDRESS uAddr, const char *&last, int &delta);
    const char *getRegName(int idx) { return pFE->getRegName(idx); }
    int getRegSize(int idx) { return pFE->getRegSize(idx); }

    void decodeEntryPoint(ADDRESS a);
    void setEntryPoint(ADDRESS a);
    void decodeEverythingUndecoded();
    void decodeFragment(UserProc *proc, ADDRESS a);
    void reDecode(UserProc *proc);
    bool wellForm();
    void finishDecode();
    void decompile();
    void removeUnusedGlobals();
    void removeRestoreStmts(StatementSet &rs);
    void globalTypeAnalysis();
    bool removeUnusedReturns();
    void fromSSAform();
    void conTypeAnalysis();
    void dfaTypeAnalysis();
    void rangeAnalysis();
    void generateDotFile();
    void generateCode(std::ostream &os);
    void generateCode(Cluster *cluster = nullptr, UserProc *proc = nullptr, bool intermixRTL = false);
    void generateRTL(Cluster *cluster = nullptr, UserProc *proc = nullptr);
    void print(std::ostream &out);
    LibProc *getLibraryProc(const char *nam);
    Signature *getLibSignature(const std::string &name);
    void rereadLibSignatures();
    Statement *getStmtAtLex(Cluster *cluster, unsigned int begin, unsigned int end);
    platform getFrontEndId();

    mAddressString &getSymbols();

    Signature *getDefaultSignature(const char *name);

    std::vector<Exp *> &getDefaultParams();
    std::vector<Exp *> &getDefaultReturns();
    bool isWin32();
    const char *getGlobalName(ADDRESS uaddr);
    ADDRESS getGlobalAddr(const char *nam);
    Global *getGlobal(const char *nam);
    const char *newGlobalName(ADDRESS uaddr);
    Type *guessGlobalType(const char *nam, ADDRESS u);
    ArrayType *makeArrayType(ADDRESS u, Type *t);
    bool globalUsed(ADDRESS uaddr, Type *knownType = nullptr);
    Type *getGlobalType(const char *nam);
    void setGlobalType(const char *name, Type *ty);
    void dumpGlobals();
    const char *getStringConstant(ADDRESS uaddr, bool knownString = false);
    double getFloatConstant(ADDRESS uaddr, bool &ok, int bits = 64);

    // Hacks for Mike
    //! Get a code for the machine e.g. MACHINE_SPARC
    MACHINE getMachine() { return pLoaderIface->GetMachine(); }
    SymbolTableInterface *getBinarySymbolTable() {
        return pLoaderPlugin ? qobject_cast<SymbolTableInterface *>(pLoaderPlugin) : nullptr;
    }
    //! Get a symbol from an address
    const char *symbolByAddress(ADDRESS dest) {
        return getBinarySymbolTable() ? getBinarySymbolTable()->SymbolByAddress(dest) : nullptr;
    }

    SectionInfo *getSectionInfoByAddr(ADDRESS a) { return pSections->GetSectionInfoByAddr(a); }
    ADDRESS getLimitTextLow() { return pSections->getLimitTextLow(); }
    ADDRESS getLimitTextHigh() { return pSections->getLimitTextHigh(); }
    bool isReadOnly(ADDRESS a) { return pSections->isReadOnly(a); }
    bool isStringConstant(ADDRESS a) { return pSections->isStringConstant(a); }
    bool isCFStringConstant(ADDRESS a) { return pSections->isCFStringConstant(a); }

    // Read 1, 2, 4, or 8 bytes given a native address
    int readNative1(ADDRESS a) { return pBinaryData->readNative1(a); }
    int readNative2(ADDRESS a) { return pBinaryData->readNative2(a); }
    int readNative4(ADDRESS a) { return pBinaryData->readNative4(a); }
    float readNativeFloat4(ADDRESS a) { return pBinaryData->readNativeFloat4(a); }
    double readNativeFloat8(ADDRESS a) { return pBinaryData->readNativeFloat8(a); }
    QWord readNative8(ADDRESS a) { return pBinaryData->readNative8(a); }
    Exp *readNativeAs(ADDRESS uaddr, Type *type);
    ptrdiff_t getTextDelta() { return pSections->getTextDelta(); }

    bool isDynamicLinkedProcPointer(ADDRESS dest) { return pLoaderIface->IsDynamicLinkedProcPointer(dest); }
    const char *GetDynamicProcName(ADDRESS uNative) { return pLoaderIface->GetDynamicProcName(uNative); }

    bool processProc(ADDRESS addr, UserProc *proc) { // Decode a proc
        std::ofstream os;
        return pFE->processProc(addr, proc, os);
    }
    void readSymbolFile(const char *fname);
    size_t getImageSize() { return pLoaderIface->getImageSize(); }
    ADDRESS getImageBase() { return pLoaderIface->getImageBase(); }
    void printSymbolsToFile();
    void printCallGraph();
    void printCallGraphXML();

    Cluster *getRootCluster() { return m_rootCluster; }
    Cluster *findCluster(const std::string &name) { return m_rootCluster->find(name); }
    Cluster *getDefaultCluster(const QString &name);
    bool clusterUsed(Cluster *c);

    //! Add the given RTL to the front end's map from address to aldready-decoded-RTL
    void addDecodedRtl(ADDRESS a, RTL *rtl) { pFE->addDecodedRtl(a, rtl); }

    Exp *addReloc(Exp *e, ADDRESS lc);

    // Public booleans that are set if and when a register jump or call is
    // found, respectively
    bool bRegisterJump;
    bool bRegisterCall;
    // list of UserProcs for entry point(s)
    std::list<UserProc *> entryProcs;

  protected:
    QObject *pLoaderPlugin; //!< Pointer to the instance returned by loader plugin
    LoaderInterface *pLoaderIface = nullptr;
    BinaryData *pBinaryData = nullptr; //!< Stored BinaryData interface for faster access.
    SectionInterface *pSections = nullptr;
    SymbolTableInterface *pSymbols = nullptr;
    FrontEnd *pFE; //!< Pointer to the FrontEnd object for the project

    /* Persistent state */
    QString m_name;            // name of the program
    QString m_path;            // its full path
    std::list<Function *> m_procs; //!< list of procedures
    PROGMAP m_procLabels;      //!< map from address to Proc*
    // FIXME: is a set of Globals the most appropriate data structure? Surely not.
    std::set<Global *> globals; //!< globals to print at code generation time
    DataIntervalMap globalMap;  //!< Map from address to DataInterval (has size, name, type)
    int m_iNumberedProc;        //!< Next numbered proc will use this
    Cluster *m_rootCluster;     //!< Root of the cluster tree

    friend class XMLProgParser;
}; // class Prog

#endif
