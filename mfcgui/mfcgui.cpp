// mfcgui.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "mfcgui.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "prog.h"
#include <assert.h>

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ProcDoc.h"
#include "ProcView.h"
#include "NewProjectDialog.h"

#include "ViewDecodeDialog.h"
#include "ProceduresDialog.h"
#include "CFGViewDialog.h"
#include "SaveLoadDialog.h"
#include "SymbolsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBoomerangApp

BEGIN_MESSAGE_MAP(CBoomerangApp, CWinApp)
	//{{AFX_MSG_MAP(CBoomerangApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_VIEW_PROCEDURES, OnViewProcedures)
	ON_COMMAND(ID_VIEW_CONTROLFLOW, OnViewControlflow)
	ON_COMMAND(ID_VIEW_SYMBOLS, OnViewSymbols)
	ON_COMMAND(ID_PROJECT_NEW, OnProjectNew)
	ON_COMMAND(ID_PROJECT_OPEN, OnProjectOpen)
	ON_COMMAND(ID_PROJECT_CLOSE, OnProjectClose)
	ON_COMMAND(ID_PROJECT_SAVE, OnProjectSave)
	ON_COMMAND(ID_PROJECT_SAVE_AS, OnProjectSaveAs)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBoomerangApp construction

CBoomerangApp::CBoomerangApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBoomerangApp object

CBoomerangApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CBoomerangApp initialization

BOOL CBoomerangApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Boomerang"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// get command line for base directories
	CString cmd(GetCommandLine());
	if (cmd.Left(1) == "\"") {
		cmd.Delete(0);
		assert(cmd.Right(1) == "\"");
		cmd.Delete(cmd.GetLength() - 1);
	}
	char c[1024];
	char *p;
	GetFullPathName(cmd, 1024, c, &p);
	assert(p);
	*p = 0;
	std::string &progpath = prog.getProgPath();
	progpath = c;

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CMultiDocTemplate(
		IDR_PROCTYPE,
		RUNTIME_CLASS(CProcDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CProcView));
	AddDocTemplate(pDocTemplate);

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW))
		return FALSE;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
//	if (!ProcessShellCommand(cmdInfo))
//		return FALSE;

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindow(m_nCmdShow);
	pMainFrame->UpdateWindow();

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CBoomerangApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CBoomerangApp message handlers

void CBoomerangApp::OnViewProcedures() 
{
	CProceduresDialog d(m_pMainWnd);
	d.DoModal();	
}

void CBoomerangApp::OnViewControlflow() 
{
	CCFGViewDialog d(m_pMainWnd);
	d.DoModal();	
}

void CBoomerangApp::OnViewSymbols() 
{
	CSymbolsDialog d(m_pMainWnd);
	d.DoModal();	
}

void CBoomerangApp::OnProjectNew() 
{
	if (prog.project != std::string("")) {
		int res = m_pMainWnd->MessageBox("Do you wish to save the current project?", NULL, MB_YESNOCANCEL);
		if (res == IDYES)
			OnProjectSave();
		if (res == IDCANCEL)
			return;
	}	
	prog.clear();
	CNewProjectDialog d;
	if (d.DoModal() == IDOK) {
		CViewDecodeDialog d;
		if (d.DoModal() == IDCANCEL) {
			// impatient bastard
		}
	}	
}

void CBoomerangApp::OnProjectOpen() 
{
	if (prog.project != std::string("")) {
		int res = m_pMainWnd->MessageBox("Do you wish to save the current project?", NULL, MB_YESNOCANCEL);
		if (res == IDYES)
			OnProjectSave();
		if (res == IDCANCEL)
			return;
	}	
	prog.clear();
	CFileDialog f(true, "bpf", NULL, 0, "Boomerang project files (*.bpf)|*.bpf|All files (*.*)|*.*|");	
	if (f.DoModal() != IDCANCEL) {
		prog.location = f.GetPathName();
		CString s;
		m_pMainWnd->GetWindowText(s);
		m_pMainWnd->SetWindowText(s + " [loading]");
		CSaveLoadDialog d(false);
		if (d.DoModal() == IDCANCEL) {
		}
		m_pMainWnd->RedrawWindow();
	}	
}

void CBoomerangApp::OnProjectClose() 
{
	if (prog.project != std::string("")) {
		int res = m_pMainWnd->MessageBox("Do you wish to save the current project?", NULL, MB_YESNOCANCEL);
		if (res == IDYES)
			OnProjectSave();
		if (res == IDCANCEL)
			return;
	}	
	prog.clear();
	m_pMainWnd->RedrawWindow();	
}

void CBoomerangApp::OnProjectSave() 
{
	if (prog.project == "" && 
		m_pMainWnd->MessageBox("This will create an empty project, which is largely useless, continue?", NULL, MB_YESNOCANCEL) != IDOK)
			return;
	if (prog.location == "")
		OnProjectSaveAs();
	else {
		CString s;
		m_pMainWnd->GetWindowText(s);
		m_pMainWnd->SetWindowText(s + " [saving]");
		CSaveLoadDialog d(true);
		if (d.DoModal() == IDCANCEL) {
		}
		m_pMainWnd->RedrawWindow();
	}	
}

void CBoomerangApp::OnProjectSaveAs() 
{
	CFileDialog f(false, "bpf", prog.project.c_str(), 0, "Boomerang project files (*.bpf)|*.bpf|All files (*.*)|*.*|");	
	if (f.DoModal() != IDCANCEL) {
		prog.location = f.GetPathName();
		if (prog.project == "")
			prog.project = f.GetFileTitle();
		OnProjectSave();
	}	
}
