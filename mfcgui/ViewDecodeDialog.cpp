// ViewDecodeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mfcgui.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "frontend.h"
#include "prog.h"
#include <assert.h>

#include "ViewDecodeDialog.h"
#include "ProcDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define UNDECODED 0xFF0000
#define INSTRUCTION 0x7F7F00
#define PROCEDURE 0x00FF00
#define PROBLEM 0x0000FF

/////////////////////////////////////////////////////////////////////////////
// CViewDecodeDialog dialog

CViewDecodeDialog::CViewDecodeDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CViewDecodeDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CViewDecodeDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CViewDecodeDialog::~CViewDecodeDialog()
{
	for (std::list<std::pair<ADDRESS, int>* >::iterator it = procs.begin();
		 it != procs.end(); it++)
		delete *it;
	procs.clear();
}


void CViewDecodeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CViewDecodeDialog)
	DDX_Control(pDX, IDCANCEL, m_cancel);
	DDX_Control(pDX, IDC_UNDECODED, m_undecoded);
	DDX_Control(pDX, IDC_PROCTREE, m_proctree);
	DDX_Control(pDX, IDC_PROCEDURE, m_procedure);
	DDX_Control(pDX, IDC_PROBLEM, m_problem);
	DDX_Control(pDX, IDC_INSTRUCTION, m_instruction);
	DDX_Control(pDX, IDC_COMPLETE, m_complete);
	DDX_Control(pDX, IDC_STATUS, m_status);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CViewDecodeDialog, CDialog)
	//{{AFX_MSG_MAP(CViewDecodeDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_PROCTREE, OnSelchangedProctree)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_NOTIFY(NM_RCLICK, IDC_PROCTREE, OnRclickProctree)
	ON_NOTIFY(NM_DBLCLK, IDC_PROCTREE, OnDblclkProctree)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewDecodeDialog message handlers

void CViewDecodeDialog::OnCancel() 
{
	
	CDialog::OnCancel();
}

void CViewDecodeDialog::OnSelchangedProctree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	
	
	*pResult = 0;
}

DWORD WINAPI dodecode(LPVOID vp)
{	
	CViewDecodeDialog *d = (CViewDecodeDialog*)vp;

	bool gotMain;
	ADDRESS a = prog.pFE->getMainEntryPoint(gotMain);
	assert(a != NO_ADDRESS);

	prog.setWatcher(d);

	UserProc* pProc;
	if (gotMain)
		pProc = (UserProc *)prog.newProc("main", a);
	else
		pProc = (UserProc *)prog.newProc("_start", a);

	prog.decode();
	prog.wellForm();

	d->alert_complete();

	return 0;
}

void CViewDecodeDialog::alert_complete()
{
	m_cancel.SetWindowText("done");
}

void CViewDecodeDialog::alert_new(Proc *p)
{
	HTREEITEM h = m_proctree.InsertItem(p->getName());
	m_proctree.SetItemData(h, (DWORD)p);
	m_proctree.EnsureVisible(h);	
}

void CViewDecodeDialog::alert_decode(ADDRESS pc, int nBytes)
{
	/* wait for lock */
	while (m_locked)
		Sleep(10);
	m_locked = true;

	RECT r;
	for (int i = 0; i < nlines; i++) {
		getUpdateRect(r, i, pc, nBytes);
		buf.FillSolidRect(&r, INSTRUCTION);
	}

	m_locked = false;
}

void CViewDecodeDialog::alert_baddecode(ADDRESS pc)
{
	/* wait for lock */
	while (m_locked)
		Sleep(10);
	m_locked = true;

	problems.push_back(pc);

	RECT r;
	for (int i = 0; i < nlines; i++) {
		getUpdateRect(r, i, pc, 1);
		buf.FillSolidRect(&r, PROBLEM);
	}

	m_locked = false;
}


void CViewDecodeDialog::alert_done(Proc *p, ADDRESS pc, ADDRESS last, int nBytes)
{
	/* wait for lock */
	while (m_locked)
		Sleep(10);
	m_locked = true;

	procs.push_back(new std::pair<ADDRESS, int>(pc, nBytes));
	
	RECT r;
	for (int i = 0; i < nlines; i++) {
		getUpdateRect(r, i, pc, last - pc);
		buf.FillSolidRect(&r, PROCEDURE);
	}

	m_locked = false;
}


BOOL CViewDecodeDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	RECT r;
	m_status.GetClientRect(&r);	
	buf.CreateCompatibleDC(GetDC());
	CBitmap b;
	b.CreateCompatibleBitmap(GetDC(), r.right, r.bottom);
	buf.SelectObject(&b);	
	buf.FillSolidRect(&r, UNDECODED);
	
	//linesize = 15;
	//unitsize = 10;
	linesize = 5;
	unitsize = 10;
	box = prog.limitTextHigh - prog.limitTextLow;
	nlines = (r.bottom - r.top) / linesize;
	oneline = box / nlines;
	scaleamt = ((double)(r.right - r.left)) / oneline;

	m_locked = false;

	if ((m_thread = CreateThread(NULL, 0, dodecode, this, 0, &m_thread_id)) == NULL) {
		MessageBox("Cannot create new thread to perform decompilation", "Error", MB_ICONEXCLAMATION|MB_OK);
		return IDCANCEL;
	}

	SetTimer(WM_TIMER, 100, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CViewDecodeDialog::DoModal() 
{
	return CDialog::DoModal();
}

void CViewDecodeDialog::getUpdateRect(RECT &r1, int line, ADDRESS a, int nBytes)
{
	RECT r;
	m_status.GetClientRect(&r);

	int n = nBytes;
	int st = a - prog.limitTextLow;
	int en = a - prog.limitTextLow + n;
	int line_start = st / oneline;
	int line_end = en / oneline;
	int off_start = st % oneline;
	int off_end = en % oneline;

	if (line < line_start || line > line_end) {
		r1.top = 0;
		r1.bottom = 0;
		r1.left = 0;
		r1.right = 0;
		return;
	}

	r1.top = r.top + line*linesize;
	r1.bottom = r1.top + linesize;
	if (line == line_start)
		r1.left = r.left + off_start*scaleamt;
	else
		r1.left = r.left;
	if (line == line_end) {
		r1.right = r.left + off_end*scaleamt;
		//if ((r1.right - r1.left) < unitsize) r1.right = r1.left + unitsize;
	} else
		r1.right = r.right;

	if (r1.right > r.right)
		r1.right = r.right;
}

void CViewDecodeDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	RECT r;
	m_undecoded.GetClientRect(&r);
	m_undecoded.ClientToScreen(&r);
	ScreenToClient(&r);	
	dc.FillSolidRect(&r, UNDECODED);

	m_instruction.GetClientRect(&r);
	m_instruction.ClientToScreen(&r);
	ScreenToClient(&r);
	dc.FillSolidRect(&r, INSTRUCTION);

	m_procedure.GetClientRect(&r);
	m_procedure.ClientToScreen(&r);
	ScreenToClient(&r);
	dc.FillSolidRect(&r, PROCEDURE);

	m_problem.GetClientRect(&r);
	m_problem.ClientToScreen(&r);
	ScreenToClient(&r);
	dc.FillSolidRect(&r, PROBLEM);

	/* wait for lock */
	while (m_locked)
		Sleep(1);
	m_locked = true;

	m_status.GetClientRect(&r);
	m_status.ClientToScreen(&r);
	ScreenToClient(&r);

	dc.BitBlt(r.left, r.top, r.right - r.left, r.bottom - r.top, &buf, 0, 0, SRCCOPY);

	m_locked = false;
		 
	// Do not call CDialog::OnPaint() for painting messages
}

void CViewDecodeDialog::OnDestroy() 
{
	TerminateThread(m_thread, 1);
	SetTimer(WM_TIMER, -1, NULL);
	prog.setWatcher(NULL);

	CDialog::OnDestroy();	
}

void CViewDecodeDialog::OnTimer(UINT nIDEvent) 
{

	while (m_locked)
		Sleep(1);
	m_locked = true;

	int total = 0;
	int totaln[100];
	int textSize = prog.limitTextHigh - prog.limitTextLow;
	memset(totaln, 0, 100*sizeof(int));

	/* this loop tries to sum up all the overlapping regions in a way which will
	   not result in a % greater than 100 */
	for (std::list<std::pair<ADDRESS, int>* >::iterator it = procs.begin();
		 it != procs.end(); it++) {
		 int n = ((*it)->first - prog.limitTextLow) * 100 / textSize;
		 totaln[n] += (*it)->second;
	}

	m_locked = false;

	for (int i = 0; i < 100; i++)
		if (totaln[i] > textSize / 100)
			total += textSize / 100;
		else
			total += totaln[i];

	m_progress.SetPos(total*100 / box);
	char s[1024];
	sprintf(s, "%i%%", total * 100 / box);
	m_complete.SetWindowText(s);

	RECT r;
	m_status.GetClientRect(&r);
	m_status.ClientToScreen(&r);
	ScreenToClient(&r);
	InvalidateRect(&r, false);

	CDialog::OnTimer(nIDEvent);
}

void CViewDecodeDialog::alert_progress(unsigned long off, unsigned long size)
{
}

void CViewDecodeDialog::OnRclickProctree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CViewDecodeDialog::OnDblclkProctree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CString s;
	m_cancel.GetWindowText(s);	
	if (s == "done") {
		HTREEITEM h = m_proctree.GetSelectedItem();
		Proc *p = (Proc *)m_proctree.GetItemData(h);
		if (p && !p->isLib()) {
			UserProc *u = (UserProc *)p;
			CProcDoc *d = (CProcDoc*)theApp.m_pDocManager->OpenDocumentFile(p->getName());		
			d->setProc(p);
			d->UpdateAllViews(NULL);

			EndDialog(IDOK);				
		}
	}
	*pResult = 0;
}
