
#include "stdincs.h"

#include "Plugin.h"

typedef IPlugin *create_func(void);

IPlugin *IPlugin::load(wxString filename)
{
#if defined(__WXGTK__) || defined(__WXMOTIF__)

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