// SaveLoadDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mfcgui.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "prog.h"
#include "util.h"
#include <assert.h>

#include "SaveLoadDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


DWORD WINAPI dosaveload(LPVOID vp)
{	
	bool bsave = (bool)vp;

	if (bsave)
		save(prog.location);
	else
		load(prog.location);

	ProgWatcher *p = prog.getWatcher();
	prog.setWatcher(NULL);
	p->alert_complete();

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CSaveLoadDialog dialog

CSaveLoadDialog::CSaveLoadDialog(bool save, CWnd* pParent /*=NULL*/)
	: CDialog(CSaveLoadDialog::IDD, pParent), m_save(save)
{
	//{{AFX_DATA_INIT(CSaveLoadDialog)
		// NOTE: the ClassWizard will add member initialization here	
	//}}AFX_DATA_INIT

}


void CSaveLoadDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSaveLoadDialog)
	DDX_Control(pDX, IDC_SAVELOADPROGRESS, m_progress);
	DDX_Control(pDX, IDC_MESSAGE, m_message);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveLoadDialog, CDialog)
	//{{AFX_MSG_MAP(CSaveLoadDialog)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveLoadDialog message handlers

void CSaveLoadDialog::OnCancel() 
{
	prog.setWatcher(NULL);
	TerminateThread(m_thread, 1);
	m_message.SetWindowText("cancelling..");
	if (!m_save)
		prog.clear();
	CDialog::OnCancel();
}

void CSaveLoadDialog::alert_new(Proc *p)
{
}

void CSaveLoadDialog::alert_decode(ADDRESS pc, int nBytes)
{
}

void CSaveLoadDialog::alert_done(Proc *p, ADDRESS pc, ADDRESS last, int nBytes)
{
}

void CSaveLoadDialog::alert_baddecode(ADDRESS pc)
{
}

void CSaveLoadDialog::alert_progress(unsigned long off, unsigned long size)
{
	m_progress.SetPos(off*100 / size);
}

void CSaveLoadDialog::alert_complete()
{
	SetTimer(WM_TIMER, 100, NULL);
}

void CSaveLoadDialog::setMessage(std::string &str)
{
	m_message.SetWindowText(str.c_str());
}

BOOL CSaveLoadDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_save)
		m_message.SetWindowText("Saving...");
	else
		m_message.SetWindowText("Loading...");

	prog.setWatcher(this);

	if ((m_thread = CreateThread(NULL, 0, dosaveload, (LPVOID)m_save, 0, &m_thread_id)) == NULL) {
		MessageBox("Cannot create new thread to load/save file", "Error", MB_ICONEXCLAMATION|MB_OK);
		return IDCANCEL;
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSaveLoadDialog::OnTimer(UINT nIDEvent) 
{
	SetTimer(WM_TIMER, -1, NULL);
	EndDialog(IDOK);
	
	CDialog::OnTimer(nIDEvent);
}
