/////////////////////////////////////////////////////////////////////////////
// Name:        mdi.cpp
// Purpose:     MDI sample
// Author:      Julian Smart
// Modified by:
// Created:     04/01/98
// RCS-ID:      $Id$
// Copyright:   (c) Julian Smart and Markus Holzem
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#include <wx/toolbar.h>

// Define a new application
class Boomerang : public wxApp
{
public:
    bool OnInit();
};

class Project;
class Preferences;
class IPlugin;

// Define a new frame
class MyFrame : public wxMDIParentFrame
{
	Project *curProject;
	Preferences *prefs;
	list<IPlugin *> plugins;
public:

    MyFrame(wxWindow *parent, const wxWindowID id, const wxString& title,
            const wxPoint& pos, const wxSize& size, const long style);

    void InitToolBar(wxToolBar* toolBar);
	void IndicateNotImplemented(void);
	void LoadPlugins();

    void OnSize(wxSizeEvent& event);
    void OnAbout(wxCommandEvent& event);
    void OnNewProject(wxCommandEvent& event);
    void OnOpenProject(wxCommandEvent& event);
    void OnSaveProject(wxCommandEvent& event);
    void OnCloseProject(wxCommandEvent& event);
    void OnPreferences(wxCommandEvent& event);
	void OnProjectSettings(wxCommandEvent& event);
    void OnQuit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);

    DECLARE_EVENT_TABLE()
};

// menu items ids
enum
{
    MDI_QUIT = 100,
    MDI_NEW_PROJECT,
	MDI_OPEN_PROJECT,
	MDI_SAVE_PROJECT,
	MDI_CLOSE_PROJECT,
	MDI_PREFERENCES,
	MDI_PROJECT_SETTINGS,
	MDI_VIEW_PROPERTIES,
	MDI_VIEW_PLUGINS,
	MDI_EDIT_UNDO,
	MDI_EDIT_REDO,
	MDI_EDIT_CUT,
	MDI_EDIT_COPY,
	MDI_EDIT_PASTE,
	MDI_EDIT_DELETE,
	MDI_EDIT_SELECTALL,
	MDI_EDIT_FIND,
	MDI_EDIT_REPLACE,
	MDI_EDIT_GOTO,
    MDI_ABOUT
};
