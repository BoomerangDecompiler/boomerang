/*
 * Boomerang main gui class implementation.
 *  
 * This file contains the main routine for the Boomerang graphical
 * user interface.  It is based on the MDI demo from the wxwindows
 * toolkit.
 *
 * 19 Mar 02 - Mike: Minor changes for Linux compile
 */

// For compilers that support precompilation, includes "wx/wx.h".

#include "stdincs.h"

#include <wx/toolbar.h>

#if defined(__WXGTK__) || defined(__WXMOTIF__)
    #include "boomerang.xpm"
    #include "bitmaps/new.xpm"
    #include "bitmaps/open.xpm"
    #include "bitmaps/save.xpm"
    #include "bitmaps/copy.xpm"
    #include "bitmaps/cut.xpm"
    #include "bitmaps/paste.xpm"
    #include "bitmaps/print.xpm"
    #include "bitmaps/help.xpm"
#endif

#include "boomerang.h"
#include "Preferences.h"
#include "Project.h"
#include "Plugin.h"

IMPLEMENT_APP(Boomerang)

// global variables

MyFrame *frame = (MyFrame *) NULL;
wxMenu *plugin_menu;

// event tables

BEGIN_EVENT_TABLE(MyFrame, wxMDIParentFrame)
    EVT_MENU(MDI_ABOUT, MyFrame::OnAbout)
    EVT_MENU(MDI_NEW_PROJECT, MyFrame::OnNewProject)
    EVT_MENU(MDI_OPEN_PROJECT, MyFrame::OnOpenProject)
    EVT_MENU(MDI_SAVE_PROJECT, MyFrame::OnSaveProject)
    EVT_MENU(MDI_CLOSE_PROJECT, MyFrame::OnCloseProject)
    EVT_MENU(MDI_PREFERENCES, MyFrame::OnPreferences)
    EVT_MENU(MDI_QUIT, MyFrame::OnQuit)

    EVT_CLOSE(MyFrame::OnClose)

    EVT_SIZE(MyFrame::OnSize)
END_EVENT_TABLE()

// implementation Boomerang App class

// Initialise this in OnInit, not statically
bool Boomerang::OnInit()
{
	plugin_menu = new wxMenu;

    // Create the main frame window

    frame = new MyFrame((wxFrame *)NULL, -1, "Boomerang",
                        wxPoint(-1, -1), wxSize(1000, 800),
                        wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL);

    // Give it an icon
#ifdef __WXMSW__
    frame->SetIcon(wxIcon("boomerang_icn"));
#else
    frame->SetIcon(wxIcon("boomerang_xpm"));
#endif

	SetVendorName("Aussie");
	SetAppName("Boomerang");

    // Make a menubar
    wxMenu *file_menu = new wxMenu;

	file_menu->Append(MDI_NEW_PROJECT, "&New Project\tCtrl-N", "Creates a new project");
    file_menu->Append(MDI_OPEN_PROJECT, "&Open Project\tCtrl-O", "Opens an existing project");
    file_menu->Append(MDI_SAVE_PROJECT, "&Save Project\tCtrl-S", "Saves the current project");
    file_menu->Append(MDI_CLOSE_PROJECT, "&Close Project", "Closes the current project");
	file_menu->AppendSeparator();
	file_menu->Append(MDI_PREFERENCES, "&Preferences", "Edits the application preferences");
	file_menu->AppendSeparator();
    file_menu->Append(MDI_QUIT, "&Exit\tAlt-X", "Quits the application; prompts to save changes");

    wxMenu *help_menu = new wxMenu;
    help_menu->Append(MDI_ABOUT, "&About\tF1");

	wxMenu *project_menu = new wxMenu;
	project_menu->Append(MDI_PROJECT_SETTINGS, "&Settings", "Edit this projects settings");

	wxMenu *view_menu = new wxMenu;
	
	view_menu->Append(MDI_VIEW_PLUGINS, "P&lugins", plugin_menu, "View the plugins that have been loaded");
	view_menu->AppendSeparator();
	view_menu->Append(MDI_VIEW_PROPERTIES, "&Properties", "Edits the current selections properties");

	wxMenu *edit_menu = new wxMenu;
	edit_menu->Append(MDI_EDIT_UNDO, "&Undo\tCtrl-Z", "Undoes the last action");
	edit_menu->Append(MDI_EDIT_REDO, "&Redo\tCtrl-Y", "Redoes the previously undone action");	
	edit_menu->AppendSeparator();
	edit_menu->Append(MDI_EDIT_CUT, "Cu&t\tCtrl-X", "Cuts the selection and moves it to the Clipboard");
	edit_menu->Append(MDI_EDIT_COPY, "&Copy\tCtrl-C", "Copies the selection to the Clipboard");	
	edit_menu->Append(MDI_EDIT_PASTE, "&Paste\tCtrl-V", "Inserts the Clipboard contents at the insertion point");
	edit_menu->Append(MDI_EDIT_DELETE, "&Delete\tDel", "Deletes the current selection");
	edit_menu->AppendSeparator();
	edit_menu->Append(MDI_EDIT_SELECTALL, "Select A&ll\tCtrl-A", "Selects the entire document");
	edit_menu->AppendSeparator();
	edit_menu->Append(MDI_EDIT_FIND, "&Find\tCtrl-F", "Find the specified text");
	edit_menu->Append(MDI_EDIT_REPLACE, "R&eplace\tCtrl-E", "Replaces the specified text with different text");
	edit_menu->AppendSeparator();
	edit_menu->Append(MDI_EDIT_GOTO, "&Goto\tCtrl-G", "Moves to a specified location");

    wxMenuBar *menu_bar = new wxMenuBar;

    menu_bar->Append(file_menu, "&File");
    menu_bar->Append(edit_menu, "&Edit");
    menu_bar->Append(view_menu, "&View");
	menu_bar->Append(project_menu, "&Project");
    menu_bar->Append(help_menu, "&Help");

    // Associate the menu bar with the frame
    frame->SetMenuBar(menu_bar);	

    frame->CreateStatusBar();

    frame->Show(TRUE);

    SetTopWindow(frame);

	frame->IndicateNotImplemented();

	// disabled because we dont have a project to save/close
	menu_bar->FindItem(MDI_SAVE_PROJECT)->Enable(false);
	menu_bar->FindItem(MDI_CLOSE_PROJECT)->Enable(false);
	menu_bar->FindItem(MDI_PROJECT_SETTINGS)->Enable(false);

	// disabled until we have a view
	menu_bar->FindItem(MDI_VIEW_PROPERTIES)->Enable(false);

    return TRUE;
}

// MyFrame

// Define my frame constructor
MyFrame::MyFrame(wxWindow *parent,
                 const wxWindowID id,
                 const wxString& title,
                 const wxPoint& pos,
                 const wxSize& size,
                 const long style)
       : wxMDIParentFrame(parent, id, title, pos, size, style), curProject(NULL)
{
    CreateToolBar(wxNO_BORDER | wxTB_FLAT | wxTB_HORIZONTAL);
    InitToolBar(GetToolBar());

    // Accelerators
    wxAcceleratorEntry entries[5];
    entries[0].Set(wxACCEL_CTRL, (int) 'N', MDI_NEW_PROJECT);
    entries[1].Set(wxACCEL_CTRL, (int) 'O', MDI_OPEN_PROJECT);
	entries[2].Set(wxACCEL_CTRL, (int) 'S', MDI_SAVE_PROJECT);
    entries[3].Set(wxACCEL_CTRL, (int) 'X', MDI_QUIT);
    entries[4].Set(wxACCEL_CTRL, (int) 'A', MDI_ABOUT);
    wxAcceleratorTable accel(4, entries);
    SetAcceleratorTable(accel);

	// setup preferences
	prefs = new Preferences(this);

	// load plugins
	LoadPlugins();
}

void MyFrame::IndicateNotImplemented(void)
{
	int notimplemented[] = { //MDI_NEW_PROJECT, MDI_OPEN_PROJECT, MDI_SAVE_PROJECT, MDI_CLOSE_PROJECT,
		                     MDI_EDIT_UNDO, MDI_EDIT_REDO, MDI_EDIT_CUT, MDI_EDIT_COPY, MDI_EDIT_PASTE, MDI_EDIT_DELETE,
							 MDI_EDIT_SELECTALL, MDI_EDIT_FIND, MDI_EDIT_REPLACE, MDI_EDIT_GOTO,
							 0 };

	for (int i = 0; notimplemented[i]; i++) {
		wxMenuItem *item = GetMenuBar()->FindItem(notimplemented[i]);

		item->SetHelp(item->GetHelp() + " [NOT IMPLEMENTED]");
		item->Enable(false);
	}
}

void MyFrame::LoadPlugins(void)
{
	wxDir dir(prefs->m_PluginDir);

	if (!dir.IsOpened()) 
	{
		// no plugins
		return;
	}

	wxString filename;
	bool cont = dir.GetFirst(&filename);
	while (cont) 
	{
		IPlugin *p = IPlugin::load(prefs->m_PluginDir + "\\" + filename);
		if (p)
		{
			plugins.push_back(p);
			plugin_menu->Append(MDI_VIEW_PLUGINS, p->GetName(), "View this plugin");
		}
		cont = dir.GetNext(&filename);
	}
}

void MyFrame::OnClose(wxCloseEvent& event)
{
    if ( event.CanVeto() && curProject )
    {
        wxString msg;
        if ( wxMessageBox("Changes will be lost, close anyhow?", "Please confirm",
                          wxICON_QUESTION | wxYES_NO) != wxYES )
        {
            event.Veto();

            return;
        }
    }

    event.Skip();
}

void MyFrame::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    Close();
}

void MyFrame::OnAbout(wxCommandEvent& WXUNUSED(event) )
{
    (void)wxMessageBox("The Boomerang Decompiler.  See http://boomerang.sourceforge.net/ for more information.\n", "About Boomerang");
}

void MyFrame::OnNewProject(wxCommandEvent& WXUNUSED(event) )
{
    if ( curProject && curProject->hasUnsaved() )
    {
		int savechanges = wxNO;        

        if ( (savechanges = wxMessageBox("There are outstanding changes to this project.  Save changes now?", "Please confirm",
                          wxICON_QUESTION | wxYES_NO | wxCANCEL)) == wxCANCEL )
        {
            return;
        }

		if (savechanges) 
		{
			curProject->saveChanges(this);
		}

		delete curProject;
    }

	// display new project dialog
	curProject = Project::createProjectUsingDialog(this);
	if (curProject) 
	{
		GetMenuBar()->FindItem(MDI_SAVE_PROJECT)->Enable(true);
		GetMenuBar()->FindItem(MDI_CLOSE_PROJECT)->Enable(true);
	}
}

void MyFrame::OnOpenProject(wxCommandEvent& WXUNUSED(event) )
{
    if ( curProject && curProject->hasUnsaved() )
    {
		int savechanges = wxNO;        

        if ( (savechanges = wxMessageBox("There are outstanding changes to this project.  Save changes now?", "Please confirm",
                          wxICON_QUESTION | wxYES_NO | wxCANCEL)) == wxCANCEL )
        {
            return;
        }

		if (savechanges) 
		{
			curProject->saveChanges(this);
		}

		delete curProject;
    }

	// display open project dialog
	curProject = Project::openProjectUsingDialog(this);
	if (curProject) 
	{
		GetMenuBar()->FindItem(MDI_SAVE_PROJECT)->Enable(true);
		GetMenuBar()->FindItem(MDI_CLOSE_PROJECT)->Enable(true);
	}
}

void MyFrame::OnSaveProject(wxCommandEvent& WXUNUSED(event) )
{
	if (curProject)
		curProject->saveChanges(this);
}

void MyFrame::OnCloseProject(wxCommandEvent& WXUNUSED(event) )
{
    if ( curProject && curProject->hasUnsaved() )
    {
		int savechanges = wxNO;        

        if ( (savechanges = wxMessageBox("There are outstanding changes to this project.  Save changes now?", "Please confirm",
                          wxICON_QUESTION | wxYES_NO | wxCANCEL)) == wxCANCEL )
        {
            return;
        }

		if (savechanges) 
		{
			curProject->saveChanges(this);
		}

    }

	delete curProject;
	curProject = NULL;

	GetMenuBar()->FindItem(MDI_SAVE_PROJECT)->Enable(false);
	GetMenuBar()->FindItem(MDI_CLOSE_PROJECT)->Enable(false);
}

void MyFrame::OnPreferences(wxCommandEvent& WXUNUSED(event))
{
	prefs->ShowModal();
}

void MyFrame::OnProjectSettings(wxCommandEvent& WXUNUSED(event))
{
	if (curProject) 
	{
		curProject->displaySettingsDialog(this);
	}
}

void MyFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
    int w, h;
    GetClientSize(&w, &h);

    GetClientWindow()->SetSize(0, 0, w, h);
}

void MyFrame::InitToolBar(wxToolBar* toolBar)
{
    wxBitmap* bitmaps[8];

#ifdef __WXMSW__
    bitmaps[0] = new wxBitmap("icon1", wxBITMAP_TYPE_RESOURCE);
    bitmaps[1] = new wxBitmap("icon2", wxBITMAP_TYPE_RESOURCE);
    bitmaps[2] = new wxBitmap("icon3", wxBITMAP_TYPE_RESOURCE);
    bitmaps[3] = new wxBitmap("icon4", wxBITMAP_TYPE_RESOURCE);
    bitmaps[4] = new wxBitmap("icon5", wxBITMAP_TYPE_RESOURCE);
    bitmaps[5] = new wxBitmap("icon6", wxBITMAP_TYPE_RESOURCE);
    bitmaps[6] = new wxBitmap("icon7", wxBITMAP_TYPE_RESOURCE);
    bitmaps[7] = new wxBitmap("icon8", wxBITMAP_TYPE_RESOURCE);
#else
    bitmaps[0] = new wxBitmap( new_xpm );
    bitmaps[1] = new wxBitmap( open_xpm );
    bitmaps[2] = new wxBitmap( save_xpm );
    bitmaps[3] = new wxBitmap( copy_xpm );
    bitmaps[4] = new wxBitmap( cut_xpm );
    bitmaps[5] = new wxBitmap( paste_xpm );
    bitmaps[6] = new wxBitmap( print_xpm );
    bitmaps[7] = new wxBitmap( help_xpm );
#endif

#ifdef __WXMSW__
    int width = 24;
#else
    int width = 16;
#endif
    int currentX = 5;

    toolBar->AddTool( MDI_NEW_PROJECT, *(bitmaps[0]), wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "New project");
//	GetMenuBar()->FindItem(MDI_NEW_PROJECT)->SetBitmap(*(bitmaps[0]));   // this should work!
    currentX += width + 5;
    toolBar->AddTool( MDI_OPEN_PROJECT, *bitmaps[1], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Open project");
    currentX += width + 5;
    toolBar->AddTool( MDI_SAVE_PROJECT, *bitmaps[2], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Save project");
    currentX += width + 5;
    toolBar->AddSeparator();
    toolBar->AddTool(3, *bitmaps[3], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Copy");
    currentX += width + 5;
    toolBar->AddTool(4, *bitmaps[4], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Cut");
    currentX += width + 5;
    toolBar->AddTool(5, *bitmaps[5], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Paste");
    currentX += width + 5;
    toolBar->AddSeparator();
    toolBar->AddTool(6, *bitmaps[6], wxNullBitmap, FALSE, currentX, -1, (wxObject *) NULL, "Print");
    currentX += width + 5;
    toolBar->AddSeparator();
    toolBar->AddTool(7, *bitmaps[7], wxNullBitmap, TRUE, currentX, -1, (wxObject *) NULL, "Help");

    toolBar->Realize();

    int i;
    for (i = 0; i < 8; i++)
        delete bitmaps[i];
}
