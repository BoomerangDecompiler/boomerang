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


#include "boomerang/util/Address.h"

#include <QFile>
#include <QTextStream>

#include <list>
#include <map>
#include <memory>


class Signature;
class Function;
class IFrontEnd;
class Prog;


/**
 * A modules is a grouping of functions irrespective of relationship.
 * For example, the Object Oriented Programming concept of a Class is a Module.
 * Modules can contain other Modules to form a tree.
 */
class Module
{
public:
    /// The type for the list of functions.
    typedef std::list<Function *>           FunctionList;
    typedef std::map<Address, Function *>   FunctionMap;

    typedef FunctionList::iterator          iterator;
    typedef FunctionList::const_iterator    const_iterator;

public:
    Module();
    Module(const QString& name, Prog *prog, IFrontEnd *fe);
    Module(const Module& other) = delete;
    Module(Module&& other) = default;

    virtual ~Module();

    Module& operator=(const Module& other) = delete;
    Module& operator=(Module&& other) = default;

public:
    QString getName() const { return m_name; }
    void setName(const QString& name) { m_name = name; }

    size_t getNumChildren() const;
    Module *getChild(size_t n);
    void addChild(Module *n);
    void removeChild(Module *n);

    Module *getUpstream() const;
    bool hasChildren() const;

    /// \todo unused
    Module *find(const QString& name);

    virtual bool isAggregate() const { return false; }

    void openStream(const char *ext);
    void openStreams(const char *ext);
    void closeStreams();

    QTextStream& getStream() { return m_strm; }
    QString makeDirs() const;
    QString getOutPath(const char *ext) const;

    /**
     * Prints a tree graph.
     */
    void printTree(QTextStream& out) const;

    /// Record the \a fnc location in the Address -> Function map
    /// If \a fnc is nullptr - remove given function from the map.
    void setLocationMap(Address loc, Function *fnc);

    Prog *getProg() { return m_prog; }

    // Function list management
    const FunctionList& getFunctionList() const { return m_functionList; }
    FunctionList& getFunctionList()       { return m_functionList; }

    iterator begin()       { return m_functionList.begin(); }
    const_iterator begin() const { return m_functionList.begin(); }
    iterator end()         { return m_functionList.end(); }
    const_iterator end()   const { return m_functionList.end(); }
    size_t size()  const { return m_functionList.size(); }
    bool empty() const { return m_functionList.empty(); }


    /**
     * Creates a new Function object, adds it to the list of procs in this Module,
     * and adds the address to the list
     * \param name            Name of the new function
     * \param entryAddr       Address of the entry point of the function
     * \param libraryFunction If true, this will be a libProc; else a UserProc
     * \returns A pointer to the new Function object
     */
    Function *createFunction(const QString& name, Address entryAddr, bool libraryFunction = false);

    /// \returns the function with name \p name
    Function *getFunction(const QString& name) const;

    /// \returns the function with entry address \p entryAddr
    Function *getFunction(Address entryAddr) const;

    void updateLibrarySignatures();

private:
    /// Retrieve Win32 PDB debug information for the function \p function
    void addWin32DbgInfo(Function *function);

private:
    FunctionList m_functionList; ///< The Functions in the module
    FunctionMap m_labelsToProcs;
    IFrontEnd *m_currentFrontend = nullptr;

protected:
    QString m_name;
    std::vector<Module *> m_children;
    Module *m_parent = nullptr;
    Prog *m_prog     = nullptr;
    QFile m_out;
    QTextStream m_strm;
    QString m_stream_ext;
};
