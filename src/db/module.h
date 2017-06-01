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

#include "include/memo.h"
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


class Module : public QObject
{
	Q_OBJECT

protected:
	friend class XMLProgParser;

public:
	/// The type for the list of functions.
	typedef std::list<Function *>           FunctionList;
	typedef std::map<ADDRESS, Function *>   FunctionMap;

	typedef FunctionList::iterator          iterator;
	typedef FunctionList::const_iterator    const_iterator;

private:
	FunctionList m_functionList; ///< The Functions in the module
	FrontEnd *m_currentFrontend;
	FunctionMap m_labelsToProcs;

protected:
	QString m_name;
	std::vector<Module *> m_children;
	Prog *m_parent     = nullptr;
	Module *m_upstream = nullptr;
	QFile m_out;
	QTextStream m_strm;
	QString m_stream_ext;

public slots:
	void onLibrarySignaturesChanged();

signals:
	void newFunction(Function *);

public:
	Module();
	Module(const QString& name, Prog *parent, FrontEnd *fe);
	virtual ~Module();

	QString getName() const { return m_name; }
	void setName(const QString& nam) { m_name = nam; }

	size_t getNumChildren();
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
	QString makeDirs();
	QString getOutPath(const char *ext);

	/**
	 * Prints a tree graph.
	 */
	void printTree(QTextStream& out) const;

	/// Record the \a fnc location in the ADDRESS->Function map
	/// If \a fnc is nullptr - remove given function from the map.
	void setLocationMap(ADDRESS loc, Function *fnc);

	Prog *getParent() { return m_parent; }
	void eraseFromParent();

	// Function list management
	const FunctionList& getFunctionList() const { return m_functionList; }
	FunctionList& getFunctionList()       { return m_functionList; }

	iterator begin()       { return m_functionList.begin(); }
	const_iterator begin() const { return m_functionList.begin(); }
	iterator end()         { return m_functionList.end(); }
	const_iterator end()   const { return m_functionList.end(); }
	size_t size()  const { return m_functionList.size(); }
	bool empty() const { return m_functionList.empty(); }

	/***************************************************************************/ /**
	 * \brief    Creates a new Function object, adds it to the list of procs in this Module,
	 * and adds the address to the list
	 * \param name - Name for the proc
	 * \param uNative - Native address of the entry point of the proc
	 * \param bLib - If true, this will be a libProc; else a UserProc
	 * \returns        A pointer to the new Function object
	 ******************************************************************************/
	Function *getOrInsertFunction(const QString& name, ADDRESS uNative, bool bLib = false);
	Function *getFunction(const QString& name) const;
	Function *getFunction(ADDRESS loc) const;

	/// Get a library signature for a given name (used when creating a new library proc).
	std::shared_ptr<Signature> getLibSignature(const QString& name);
};


class Class : public Module
{
protected:
	std::shared_ptr<CompoundType> m_type;

public:
	Class(const QString& name, Prog *parent, FrontEnd *fe)
		: Module(name, parent, fe)
		, m_type(CompoundType::get())
	{
	}

	/// A Class tends to be aggregated into the parent Module,
	/// this isn't the case with Java, but hey, we're not doing that yet.
	virtual bool isAggregate() const override { return true; }
};


struct ModuleFactory
{
	virtual Module *create(const QString& name, Prog *parent, FrontEnd *fe) const = 0;
};


struct DefaultModFactory : public ModuleFactory
{
	Module *create(const QString& name, Prog *parent, FrontEnd *fe) const
	{
		return new Module(name, parent, fe);
	}
};


struct ClassModFactory : public ModuleFactory
{
	Module *create(const QString& name, Prog *parent, FrontEnd *fe) const
	{
		return new Class(name, parent, fe);
	}
};
