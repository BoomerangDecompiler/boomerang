#include "module.h"

#include "core/Log.h"

#include "db/proc.h"
#include "db/prog.h"
#include "db/signature.h"
#include "db/statements/callstatement.h"

#include <QDir>
#include <QString>

#if defined(_WIN32) && !defined(__MINGW32__)
#undef NO_ADDRESS
#include <windows.h>
namespace dbghelp
{
#include <dbghelp.h>
// dbghelp.h can define ADDRESS
#ifdef ADDRESS
#undef ADDRESS
#endif
}
#undef NO_ADDRESS
#define NO_ADDRESS    ADDRESS::g(-1)
#include <iostream>
#include "core/Log.h"
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
// From prog.cpp
BOOL CALLBACK addSymbol(dbghelp::PSYMBOL_INFO pSymInfo, ULONG SymbolSize, PVOID UserContext);
SharedType typeFromDebugInfo(int index, DWORD64 ModBase);

#endif


void Module::onLibrarySignaturesChanged()
{
	m_currentFrontend->readLibraryCatalog();

	for (Function *pProc : m_functionList) {
		if (pProc->isLib()) {
			pProc->setSignature(getLibSignature(pProc->getName()));

			for (CallStatement *call_stmt : pProc->getCallers()) {
				call_stmt->setSigArguments();
			}

			Boomerang::get()->alertUpdateSignature(pProc);
		}
	}
}


Module::Module()
{
	m_strm.setDevice(&m_out);
}


Module::Module(const QString& _name, Prog *_parent, FrontEnd *fe)
	: m_currentFrontend(fe)
	, m_name(_name)
	, m_parent(_parent)
{
	m_strm.setDevice(&m_out);
}


Module::~Module()
{
	for (Function *proc : m_functionList) {
		delete proc;
	}
}


size_t Module::getNumChildren()
{
	return m_children.size();
}


Module *Module::getChild(size_t n)
{
	return m_children[n];
}


void Module::addChild(Module *n)
{
	if (n->m_upstream) {
		n->m_upstream->removeChild(n);
	}

	m_children.push_back(n);
	n->m_upstream = this;
}


void Module::removeChild(Module *n)
{
	auto it = m_children.begin();

	for ( ; it != m_children.end(); it++) {
		if (*it == n) {
			break;
		}
	}

	assert(it != m_children.end());
	m_children.erase(it);
}


Module *Module::getUpstream() const
{
	return m_upstream;
}


bool Module::hasChildren() const
{
	return !m_children.empty();
}


void Module::openStream(const char *ext)
{
	if (m_out.isOpen()) {
		return;
	}

	m_out.setFileName(getOutPath(ext));
	m_out.open(QFile::WriteOnly | QFile::Text);
	m_stream_ext = ext;
}


void Module::openStreams(const char *ext)
{
	openStream(ext);

	for (Module *child : m_children) {
		child->openStreams(ext);
	}
}


void Module::closeStreams()
{
	if (m_out.isOpen()) {
		m_strm.flush();
		m_out.close();
	}

	for (Module *child : m_children) {
		child->closeStreams();
	}
}


QString Module::makeDirs()
{
	QString path;

	if (m_upstream) {
		path = m_upstream->makeDirs();
	}
	else {
		path = Boomerang::get()->getOutputPath();
	}

	QDir dr(path);

	if ((getNumChildren() > 0) || (m_upstream == nullptr)) {
		dr.mkpath(m_name);
		dr.cd(m_name);
	}

	return dr.absolutePath();
}


QString Module::getOutPath(const char *ext)
{
	QString basedir = makeDirs();
	QDir    dr(basedir);

	return dr.absoluteFilePath(m_name + "." + ext);
}


Module *Module::find(const QString& nam)
{
	if (m_name == nam) {
		return this;
	}

	for (Module *child : m_children) {
		Module *c = child->find(nam);

		if (c) {
			return c;
		}
	}

	return nullptr;
}


void Module::printTree(QTextStream& ostr) const
{
	ostr << "\t\t" << m_name << "\n";

	for (Module *elem : m_children) {
		elem->printTree(ostr);
	}
}


void Module::setLocationMap(ADDRESS loc, Function *fnc)
{
	if (fnc == nullptr) {
		size_t count = m_labelsToProcs.erase(loc);
		assert(count == 1);
	}
	else {
		m_labelsToProcs[loc] = fnc;
	}
}


void Module::eraseFromParent()
{
	m_parent->getModuleList().remove(this);
	delete this;
}


Function *Module::getOrInsertFunction(const QString& name, ADDRESS uNative, bool bLib)
{
	Function *pProc;

	if (bLib) {
		pProc = new LibProc(this, name, uNative);
	}
	else {
		pProc = new UserProc(this, name, uNative);
	}

	if (NO_ADDRESS != uNative) {
		assert(m_labelsToProcs.find(uNative) == m_labelsToProcs.end());
		m_labelsToProcs[uNative] = pProc;
	}

	m_functionList.push_back(pProc); // Append this to list of procs
	// alert the watchers of a new proc
	emit newFunction(pProc);
	Boomerang::get()->alertNew(pProc);
	// TODO: add platform agnostic way of using debug information, should be moved to Loaders, Prog should just collect info
	// from Loader

#if defined(_WIN32) && !defined(__MINGW32__)
	if (CurrentFrontend->isWin32()) {
		// use debugging information
		HANDLE               hProcess = GetCurrentProcess();
		dbghelp::SYMBOL_INFO *sym     = (dbghelp::SYMBOL_INFO *)malloc(sizeof(dbghelp::SYMBOL_INFO) + 1000);
		sym->SizeOfStruct = sizeof(*sym);
		sym->MaxNameLen   = 1000;
		sym->Name[0]      = 0;
		BOOL  got = dbghelp::SymFromAddr(hProcess, uNative.m_value, 0, sym);
		DWORD retType;

		if (got && *sym->Name &&
			dbghelp::SymGetTypeInfo(hProcess, sym->ModBase, sym->TypeIndex, dbghelp::TI_GET_TYPE, &retType)) {
			DWORD d;
			// get a calling convention
			got =
				dbghelp::SymGetTypeInfo(hProcess, sym->ModBase, sym->TypeIndex, dbghelp::TI_GET_CALLING_CONVENTION, &d);

			if (got) {
				std::cout << "calling convention: " << d << "\n";
				// TODO: use it
			}
			else {
				// assume we're stdc calling convention, remove r28, r24 returns
				pProc->setSignature(Signature::instantiate(PLAT_PENTIUM, CONV_C, name));
			}

			// get a return type
			SharedType rtype = typeFromDebugInfo(retType, sym->ModBase);

			if (!rtype->isVoid()) {
				pProc->getSignature()->addReturn(rtype, Location::regOf(24));
			}

			// find params and locals
			dbghelp::IMAGEHLP_STACK_FRAME stack;
			stack.InstructionOffset = uNative.m_value;
			dbghelp::SymSetContext(hProcess, &stack, 0);
			dbghelp::SymEnumSymbols(hProcess, 0, nullptr, addSymbol, pProc);

			LOG << "final signature: ";
			pProc->getSignature()->printToLog();
			LOG << "\n";
		}
	}
#endif

	return pProc;
}


Function *Module::getFunction(const QString& name) const
{
	for (Function *f : m_functionList) {
		if (f->getName() == name) {
			return f;
		}
	}

	return nullptr;
}


Function *Module::getFunction(ADDRESS loc) const
{
	auto iter = m_labelsToProcs.find(loc);

	if (iter == m_labelsToProcs.end()) {
		return nullptr;
	}

	return iter->second;
}


std::shared_ptr<Signature> Module::getLibSignature(const QString& name)
{
	return m_currentFrontend->getLibSignature(name);
}
