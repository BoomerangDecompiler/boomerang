// 19 Mar 02 - Mike: Added dlopen/dlsym code for opening Unix .so libraries

#include "stdincs.h"

#include "Plugin.h"

typedef IPlugin *create_func(void);

IPlugin *IPlugin::load(wxString filename)
{
#if defined(__WXGTK__) || defined(__WXMOTIF__)
    void* h = dlopen(filename.c_str(), RTLD_LAZY);
    if (h == NULL) {
        return NULL;        // No error message or number as yet
    }
    create_func* create = (create_func*)dlsym(h, "create");
    if (create == NULL) {
        return NULL;
    }
    // Problem: we have not closed this library with dlclose, and we
    // have not stored h anywhere.

#else
	HMODULE h = LoadLibrary(filename.c_str());
	if (h == NULL) {
		DWORD error = GetLastError();	
		return NULL;
	}
	create_func *create = (create_func *)GetProcAddress(h, "create");
	if (create == NULL) {
		DWORD error = GetLastError();	
		return NULL;
	}
#endif

	return (*create)();
}
