#include "Plugin.h"

#include <cassert>

#ifdef _MSC_VER
#  include "Windows.h"
#else
#  include <dlfcn.h>
#endif


PluginHandle::PluginHandle(const QString& filePath)
{
#ifdef _MSC_VER
	m_handle = LoadLibrary(qPrintable(filePath));
	if (m_handle == nullptr) {
		throw "Loading plugin failed!";
	}
#else
	m_handle = dlopen(qPrintable(filePath), RTLD_NOW);
	if (m_handle == nullptr) {
		throw dlerror();
	}
#endif
}


PluginHandle::~PluginHandle()
{
#ifdef _MSC_VER
	FreeLibrary((HMODULE)m_handle);
#else
	dlclose(m_handle);
#endif
}


PluginHandle::Symbol PluginHandle::getSymbol(const char* name) const
{
#ifdef _MSC_VER
	return GetProcAddress((HMODULE)m_handle, name);
#else
	return dlsym(m_handle, name);
#endif
}
