#include <string>
#include <iostream>
#include <fstream>
#include <list>

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
    #include "wx/mdi.h"
#endif

#include "wx/config.h"
#include "wx/dir.h"

using namespace std;

#include "../Plugin.h"
#include "TestPlugin.h"

TestPluginFrame::TestPluginFrame(wxMDIParentFrame *parent) : wxMDIChildFrame(parent, -1, "Test Plugin")
{

}

wxMDIChildFrame *TestPlugin::GetWindow(wxMDIParentFrame *parent)
{
	if (m_window) return m_window;
	m_window = new TestPluginFrame(parent);
    return m_window;
}	

// dll main and the create function follow

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

// This is an example of an exported function.
extern "C" __declspec(dllexport) IPlugin *create(void)
{
	return new TestPlugin();
}

