// NewProjectDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mfcgui.h"
#include "NewProjectDialog.h"
#include <assert.h>

#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "prog.h"
#include "Win32BinaryFile.h"
#include "decoder.h"
#include "frontend.h"
#include "sparcfrontend.h"
#include "pentiumfrontend.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewProjectDialog dialog


CNewProjectDialog::CNewProjectDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CNewProjectDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewProjectDialog)
	//}}AFX_DATA_INIT
}


void CNewProjectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewProjectDialog)
	DDX_Control(pDX, IDC_LOCATION, m_location);
	DDX_Control(pDX, IDC_NAME, m_name);
	DDX_Control(pDX, IDC_FILENAME, m_filename);
	DDX_Control(pDX, IDC_LOADER, m_loader);
	DDX_Control(pDX, IDC_FRONTEND, m_frontend);
	//}}AFX_DATA_MAP

	m_loader_n = m_loader.GetCurSel();
	m_frontend_n = m_frontend.GetCurSel();
	m_name.GetWindowText(m_name_str);
	m_filename.GetWindowText(m_filename_str);
	m_location.GetWindowText(m_location_str);
}


BEGIN_MESSAGE_MAP(CNewProjectDialog, CDialog)
	//{{AFX_MSG_MAP(CNewProjectDialog)
	ON_BN_CLICKED(IDC_BROWSE_LOCATION, OnBrowseLocation)
	ON_BN_CLICKED(IDC_BROWSE_FILENAME, OnBrowseFilename)
	ON_EN_CHANGE(IDC_FILENAME, OnChangeFilename)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewProjectDialog message handlers

void CNewProjectDialog::OnBrowseLocation() 
{
	CString name;
	m_name.GetWindowText(name);
	CFileDialog f(false, "bpf", name, 0, "Boomerang project files (*.bpf)|*.bpf|All files (*.*)|*.*|");	
	if (f.DoModal() != IDCANCEL) {
		CString location = f.GetPathName();
		CString name = f.GetFileTitle();
		int last = 0;
		do last = location.Find(name, last+1); while (location.Find(name, last+1) != -1);
		location.Delete(last, location.GetLength() - last);
		m_location.SetWindowText(location);
		m_name.SetWindowText(name);
	}
}

void CNewProjectDialog::OnBrowseFilename() 
{
	CString filename;
	m_filename.GetWindowText(filename);
	CFileDialog f(true, NULL, filename, 0, "Executables (*.exe)|*.exe|Dynamic link libraries (*.dll)|*.dll|All files (*.*)|*.*|");
	if (f.DoModal() != IDCANCEL) {
		m_filename.SetWindowText(f.GetPathName());
		// If project name not entered, set it to something reasonable.
		CString name;
		m_name.GetWindowText(name);
		if (name == "") {
			m_name.SetWindowText(f.GetFileTitle());
		}
		m_loader.SetCurSel(0);
		m_frontend.SetCurSel(0);
	}	
}

void CNewProjectDialog::OnChangeFilename() 
{
	CString name;
	m_name.GetWindowText(name);
	CString filename;
	m_filename.GetWindowText(filename);
	// make project name and filename the same
	if (name == filename.Left(filename.GetLength() - 1)) {
		m_name.SetWindowText(filename);
	}	
}

BOOL CNewProjectDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString s(prog.getProgPath().c_str());
	int r = s.ReverseFind('\\');
	if (r == s.GetLength() - 1) {
		s.Delete(s.GetLength() - 1);
		r = s.ReverseFind('\\');
	}
	s.Delete(r, s.GetLength() - r);	
	m_location.SetWindowText(s + "\\projects\\");
	
	// list the available loaders, TODO: look for plugins	
	m_loader.InsertString(0, "UQBT - WIN32 EXE/DLL");
	m_loader.InsertString(1, "UQBT - DOS/286 EXE");
	m_loader.InsertString(2, "UQBT - HpSom binary");
	m_loader.InsertString(3, "UQBT - PalmOS binary");
	m_loader.InsertString(4, "UQBT - ELF binary (standard unix)");

	// list the available frontends
	m_frontend.InsertString(0, "NJMC - Intel Pentium/80x86 (32 bit only)");
	m_frontend.InsertString(1, "NJMC - Sun SPARC v8/v9");
	m_frontend.InsertString(2, "NJMC - Hp RISC");
	m_frontend.InsertString(3, "NJMC - Motorolla 68000");
	m_frontend.InsertString(4, "NJMC - ARM");
	m_frontend.InsertString(5, "UQBT - Intel 80286 (16 bit only)");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewProjectDialog::OnOK() 
{
	if (m_loader.GetCurSel() != 0) {
		// using a loader other than win32.. eep
		if (MessageBox("You have selected a loader that is not yet implemented, your selection will be ignored. Continue?", 
			"Unimplemented loader", MB_YESNO) == IDYES) {
			m_loader.SetCurSel(0);
		} else return;
	}
	if (m_frontend.GetCurSel() != 0) {
		// using a decoder other than pentium.. eep
		if (m_frontend.GetCurSel() == 1) {
			if (MessageBox("The SPARC front end is functional, however, there are no loaders currently implemented that can load binaries "
				       "that would conceivably contain SPARC code.  It is therefore recommended that you dont select the SPARC front end, "
					   "however, if you know what you are doing, feel free to give it a go.  Continue?",
					   "No good loader ok?", MB_YESNO) == IDNO) return;
		} else if (MessageBox("You have selected a front end that is not yet implemented, your selection will be ignored. Continue?", 
			"Unimplemented front end", MB_YESNO) == IDYES) {
			m_frontend.SetCurSel(0);
		} else return;
	}

	CString filename;
	m_filename.GetWindowText(filename);
	prog.filename = std::string(filename);
	CString name;
	m_name.GetWindowText(name);
	prog.project = std::string(name);
	CString location;
	m_location.GetWindowText(location);
	prog.location = std::string(location);

	if (prog.location == "" || prog.filename == "" || prog.project == "") {
		MessageBox("You must enter a name, location, filename to decompile!");
		return;
	}

	assert(SetCurrentDirectory(prog.location.c_str()));

	if (prog.location.at(prog.location.length()-1) != '\\') {
		prog.location += "\\";
	}
	prog.location += prog.project;
	prog.location += ".bpf";

	assert(m_loader.GetCurSel() == 0);
	prog.pBF = new Win32BinaryFile();
	if (!prog.pBF->RealLoad(filename)) {
		MessageBox("The loader you have selected is unable to load the requested file.  Make sure you have both the filename and the correct loader selected.",
				   "Bad loader", MB_OK);
		delete prog.pBF;
		prog.pBF = NULL;
		return;
	}
	prog.getTextLimits();
	assert(m_frontend.GetCurSel() == 0 || m_frontend.GetCurSel() == 1);	
	if (m_frontend_n == 1) {
	    bool readResult = prog.RTLDict.readSSLFile(prog.getProgPath() + "..\\specs\\sparc.ssl", false);
		assert(readResult);
		prog.pFE = new SparcFrontEnd(prog.textDelta, prog.limitTextHigh);
	} else {
		bool readResult = prog.RTLDict.readSSLFile(prog.getProgPath() + "..\\specs\\pentium.ssl", false);
		assert(readResult);
	    prog.pFE = new PentiumFrontEnd(prog.textDelta, prog.limitTextHigh);
	}

	prog.readLibParams();

	theApp.GetMainWnd()->GetMenu()->EnableMenuItem(ID_FILE_CLOSE, MF_ENABLED);
	theApp.GetMainWnd()->GetMenu()->EnableMenuItem(ID_FILE_SAVE, MF_ENABLED);
	theApp.GetMainWnd()->GetMenu()->EnableMenuItem(ID_FILE_SAVE_AS, MF_ENABLED);
	
	CDialog::OnOK();
}

BinaryFile * CNewProjectDialog::getLoader()
{
	return prog.pBF;
}

FrontEnd * CNewProjectDialog::getFrontEnd()
{	
	return prog.pFE;
}

