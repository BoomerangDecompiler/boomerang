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


#include "boomerang/type/type/CompoundType.h"

#include <list>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <fstream>

#include <QtCore/QTextStream>
#include <QtCore/QFile>

class Function;
class Prog;
class IFrontEnd;


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
    void setName(const QString& nam) { m_name = nam; }

    size_t getNumChildren() const;
    Module *getChild(size_t n);
    void addChild(Module *n);
    void removeChild(Module *n);

    Module *getUpstream() const;
    bool hasChildren() const;

    /// \todo unused
    Module *find(const QString& nam);

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

    /// \returns the signature of the function with name \p name
    std::shared_ptr<Signature> getLibSignature(const QString& name);

    void updateLibrarySignatures();

private:
    /// Retrieve Win32 PDB debug information for the function \p function
    void addWin32DbgInfo(Function *function);

private:
    FunctionList m_functionList; ///< The Functions in the module
    FunctionMap m_labelsToProcs;
    IFrontEnd *m_currentFrontend;

protected:
    QString m_name;
    Module *m_parent = nullptr;
    std::vector<Module *> m_children;
    Prog *m_prog           = nullptr;
    QFile m_out;
    QTextStream m_strm;
    QString m_stream_ext;
};


class Class : public Module
{
protected:
    std::shared_ptr<CompoundType> m_type;

public:
    Class(const QString& name, Prog *_prog, IFrontEnd *fe)
        : Module(name, _prog, fe)
        , m_type(CompoundType::get())
    {
    }

    /// A Class tends to be aggregated into the parent Module,
    /// this isn't the case with Java, but hey, we're not doing that yet.
    virtual bool isAggregate() const override { return true; }
};


class ModuleFactory
{
public:
    virtual ~ModuleFactory() = default;
    virtual Module *create(const QString& name, Prog *parent, IFrontEnd *fe) const = 0;
};


class DefaultModFactory : public ModuleFactory
{
public:
    virtual ~DefaultModFactory() override = default;
    Module *create(const QString& name, Prog *parent, IFrontEnd *fe) const override
    {
        return new Module(name, parent, fe);
    }
};


struct ClassModFactory : public ModuleFactory
{
    virtual ~ClassModFactory() override = default;

    Module *create(const QString& name, Prog *parent, IFrontEnd *fe) const override
    {
        return new Class(name, parent, fe);
    }
};
