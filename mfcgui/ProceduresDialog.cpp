// ProceduresDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mfcgui.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "prog.h"
#include "cfg.h"
#include "util.h"
#include <assert.h>
#include <sstream>

#include "ProceduresDialog.h"
#include "ProcPropertiesDialog.h"
#include "ProcDoc.h"
#include "CFGViewDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProceduresDialog dialog


CProceduresDialog::CProceduresDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CProceduresDialog::IDD, pParent), m_leaves(false)
{
	//{{AFX_DATA_INIT(CProceduresDialog)
	//}}AFX_DATA_INIT
}


void CProceduresDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProceduresDialog)
	DDX_Control(pDX, IDC_CALLS, m_calls);
	DDX_Control(pDX, IDC_CALLEDBY, m_calledby);
	DDX_Control(pDX, IDC_PROCS, m_procs);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProceduresDialog, CDialog)
	//{{AFX_MSG_MAP(CProceduresDialog)
	ON_NOTIFY(TVN_SELCHANGED, IDC_PROCS, OnSelchangedProcs)
	ON_NOTIFY(TVN_SELCHANGED, IDC_CALLS, OnSelchangedCalls)
	ON_NOTIFY(NM_RCLICK, IDC_CALLS, OnRclickCalls)
	ON_NOTIFY(NM_DBLCLK, IDC_CALLS, OnDblclkCalls)
	ON_NOTIFY(NM_RCLICK, IDC_PROCS, OnRclickProcs)
	ON_COMMAND(ID_EDIT_CODE, OnEditCode)
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_VIEW_CONTROLFLOW, OnViewControlflow)
	ON_COMMAND(ID_SHOW_LEAVES, OnShowLeaves)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CProceduresDialog::strForProc(Proc *p, CString &s)
{
	std::ostringstream os;
	os << p->getName() << " <0x" << std::hex << p->getNativeAddress() << std::dec << ">";
	if (p->isLib())
		os << " library" << std::ends;
	else {
		UserProc *up = (UserProc*)p;
		if (!up->getCFG()->isWellFormed())
			os << " ill-formed";
		if (!up->getCFG()->establishDFTOrder())
			os << " disconnected";
		if (!up->getCFG()->establishRevDFTOrder())
			os << " irregular";
		os << std::ends;
	}
	s = os.str().c_str();
}

void CProceduresDialog::addToProcs(Proc *p)
{
	if (m_procitem.find(p) != m_procitem.end())
		return;
	
	CString s;
	strForProc(p, s);

	bool leaf = true;
	if (p->isLib()) 
		leaf = false;
	else {
		UserProc *u = (UserProc*)p;
		std::set<Proc*>::iterator it;		
		for (it = u->getCallees().begin(); it != u->getCallees().end(); it++)
			if (!(*it)->isLib()) leaf = false;
	}

	if (p->getFirstCaller()) {
		addToProcs(p->getFirstCaller());
		if (!m_leaves || leaf)
			m_procitem[p] = m_procs.InsertItem(s, m_procitem[p->getFirstCaller()]);
	} else if (!m_leaves || leaf)
		m_procitem[p] = m_procs.InsertItem(s);
	m_procs.SetItemData(m_procitem[p], (DWORD)p);
}

/////////////////////////////////////////////////////////////////////////////
// CProceduresDialog message handlers

BOOL CProceduresDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	redraw();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProceduresDialog::redraw()
{
	m_procs.DeleteAllItems();
	m_procitem.clear();
	PROGMAP::const_iterator it;
	Proc *p;
	for (p = prog.getFirstProc(it); p; p = prog.getNextProc(it)) {
		addToProcs(p);
	}
	for (p = prog.getFirstProc(it); p; p = prog.getNextProc(it)) {
		m_procs.Expand(m_procitem[p], TVE_EXPAND);
	}
}

void CProceduresDialog::OnSelchangedProcs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	UserProc *p = (UserProc *)m_procs.GetItemData(m_procs.GetSelectedItem());
	m_calls.DeleteAllItems();		
	if (!p->isLib()) {
		for (std::set<Proc*>::iterator it = p->getCallees().begin(); it != p->getCallees().end(); it++) {
			m_calls.SetItemData(m_calls.InsertItem((*it)->getName()), (DWORD)*it);
		}
	}
	*pResult = 0;
}

void CProceduresDialog::OnSelchangedCalls(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

	if (m_calls.GetSelectedItem()) {
		Proc *p = (Proc *)m_calls.GetItemData(m_calls.GetSelectedItem());
		m_procs.EnsureVisible(m_procitem[p]);
	}
	
	*pResult = 0;
}

void CProceduresDialog::OnRclickCalls(NMHDR* pNMHDR, LRESULT* pResult) 
{
	Proc *p = (Proc *)m_calls.GetItemData(m_calls.GetSelectedItem());
	m_procs.SelectItem(m_procitem[p]);
	m_procs.EnsureVisible(m_procitem[p]);
	m_procs.SetFocus();
	
	*pResult = 0;
}

void CProceduresDialog::OnDblclkCalls(NMHDR* pNMHDR, LRESULT* pResult) 
{
	
	*pResult = 0;
}

void CProceduresDialog::OnRclickProcs(NMHDR* pNMHDR, LRESULT* pResult) 
{
	POINT p;
	::GetCursorPos(&p);
	m_procs.ScreenToClient(&p);
	HTREEITEM h = m_procs.HitTest(CPoint(p));
	if (h) m_procs.SelectItem(h);

	h = m_procs.GetSelectedItem();
	if (h) {
		Proc *pr = (Proc *)m_procs.GetItemData(h);
		UserProc *up = NULL;
		if (!pr->isLib()) up = (UserProc *)pr;
		CMenu popup;
		popup.CreatePopupMenu();
		if (m_leaves)
			popup.AppendMenu(0, ID_SHOW_LEAVES, "Show all");
		else
			popup.AppendMenu(0, ID_SHOW_LEAVES, "Show only leaves");
		popup.AppendMenu(MFT_SEPARATOR);
		if (up) {
			popup.AppendMenu(0, ID_VIEW_CONTROLFLOW, "View control flow");
			popup.AppendMenu(0, ID_EDIT_CODE, "Edit code");
			popup.AppendMenu(MFT_SEPARATOR);
		}
		popup.AppendMenu(0, ID_PROPERTIES, "Properties");
		m_procs.ClientToScreen(&p);
		popup.TrackPopupMenu(0, p.x, p.y, this);
	}
	
	*pResult = 0;
}

void CProceduresDialog::OnEditCode() 
{
	HTREEITEM h = m_procs.GetSelectedItem();
	if (h) {
		Proc *p = (Proc *)m_procs.GetItemData(h);
		CProcDoc *d = (CProcDoc*)theApp.m_pDocManager->OpenDocumentFile(p->getName());		
		d->setProc(p);
		d->UpdateAllViews(NULL);

		EndDialog(IDOK);
	}	
}

void CProceduresDialog::OnProperties() 
{
	HTREEITEM h = m_procs.GetSelectedItem();
	if (h) {
		Proc *p = (Proc*)m_procs.GetItemData(h);
		CProcPropertiesDialog d(p, theApp.GetMainWnd());
		d.DoModal();
	}	
}

void CProceduresDialog::OnViewControlflow() 
{
	HTREEITEM h = m_procs.GetSelectedItem();
	if (h) {
		UserProc *p = (UserProc *)m_procs.GetItemData(h);		
		CCFGViewDialog d(theApp.GetMainWnd(), p->getCFG());
		d.DoModal();
	}	
}

void CProceduresDialog::OnShowLeaves() 
{
	m_leaves ^= true;
	redraw();
}
