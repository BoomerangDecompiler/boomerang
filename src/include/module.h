#pragma once
/*
 * Copyright (C) 2004, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/***************************************************************************/ /**
  * \file       module.h
  *    Definition of the classes that describe a Module, a grouping
  * of functions irrespective of relationship.  For example, the
  * Object Oriented Programming concept of a Class is a Module.
  * Modules can contain other Modules to form a tree.
  ******************************************************************************/

#include "memo.h"
#include "include/type.h"

#include <list>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <QtCore/QTextStream>
#include <QtCore/QFile>

class XMLProgParser;
class Function;
class Prog;
class FrontEnd;
class Module : public QObject {
    Q_OBJECT
public:
    /// The type for the list of functions.
    typedef std::list<Function *>             FunctionListType;
    typedef FunctionListType::iterator                iterator;
    typedef FunctionListType::const_iterator    const_iterator;

private:
    FunctionListType FunctionList;  ///< The Functions in the module
    QString Target;           ///< Target platform Module was compiled on
    FrontEnd * CurrentFrontend;
    std::map<ADDRESS,Function *> LabelsToProcs;
protected:
    QString Name;
    std::vector<Module *> Children;
    Prog *Parent = nullptr;
    Module *Upstream = nullptr;
    QFile out;
    QTextStream strm;
    QString stream_ext;
public slots:
    void onLibrarySignaturesChanged();
signals:
    void newFunction(Function *);
public:
    Module();
    Module(const QString &_name,Prog *_parent,FrontEnd *fe);
    virtual ~Module();
    QString getName() { return Name; }
    void setName(const QString &nam) { Name = nam; }
    size_t getNumChildren();
    Module *getChild(size_t n);
    void addChild(Module *n);
    void removeChild(Module *n);
    Module *getUpstream();
    bool hasChildren();
    Module *find(const QString &nam);
    virtual bool isAggregate() { return false; }

    void openStream(const char *ext);
    void openStreams(const char *ext);
    void closeStreams();
    QTextStream &getStream() { return strm; }
    QString makeDirs();
    QString getOutPath(const char *ext);

    void printTree(QTextStream &out);
    void setLocationMap(ADDRESS loc, Function *fnc);

    Prog * getParent() { return Parent; }
    void eraseFromParent();
    // Function list management
    const FunctionListType &getFunctionList() const { return FunctionList; }
    FunctionListType       &getFunctionList()       { return FunctionList; }

    iterator                begin()       { return FunctionList.begin(); }
    const_iterator          begin() const { return FunctionList.begin(); }
    iterator                end  ()       { return FunctionList.end();   }
    const_iterator          end  () const { return FunctionList.end();   }
    size_t                  size()  const { return FunctionList.size(); }
    bool                    empty() const { return FunctionList.empty(); }
    Function *              getOrInsertFunction(const QString &name, ADDRESS uNative, bool bLib = false);
    Function *              getFunction(const QString &name);
    Function *              getFunction(ADDRESS loc);

    std::shared_ptr<Signature> getLibSignature(const QString &name);
protected:
    friend class XMLProgParser;
};

class Class : public Module {
protected:
    std::shared_ptr<CompoundType> Type;

public:
    Class(const QString &name,Prog *_parent,FrontEnd *fe) :
        Module(name,_parent,fe), Type(CompoundType::get()){
    }

    // A Class tends to be aggregated into the parent Module,
    // this isn't the case with Java, but hey, we're not doing that yet.
    virtual bool isAggregate() { return true; }
};

struct ModuleFactory {
    virtual Module *create(const QString &name,Prog *_parent,FrontEnd *fe) const =0;
};
struct DefaultModFactory : public ModuleFactory {
    Module *create(const QString &name,Prog *_parent,FrontEnd *fe) const {
        return new Module(name,_parent,fe);
    }
};
struct ClassModFactory : public ModuleFactory {
    Module *create(const QString &name,Prog *_parent,FrontEnd *fe) const {
        return new Class(name,_parent,fe);
    }
};
