// SymbolsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mfcgui.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "prog.h"
#include <assert.h>
#include <sstream>

#include "SymbolsDialog.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "ProcDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSymbolsDialog dialog


CSymbolsDialog::CSymbolsDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSymbolsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSymbolsDialog)
	//}}AFX_DATA_INIT
}


void CSymbolsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSymbolsDialog)
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_TAB, m_tab);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSymbolsDialog, CDialog)
	//{{AFX_MSG_MAP(CSymbolsDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnSelchangeTab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSymbolsDialog message handlers

BOOL CSymbolsDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_tab.DeleteAllItems();
	m_tab.InsertItem(0, "Globals");
	m_tab.InsertItem(1, "Locals");
	m_tab.SetCurSel(0);

	RECT r;
	m_list.GetClientRect(&r);
	int w = (r.right - r.left) / 3;
	m_list.InsertColumn(0, "Expression");
	m_list.SetColumnWidth(0, w);
	m_list.InsertColumn(1, "Symbol");
	m_list.SetColumnWidth(1, w);
	m_list.InsertColumn(2, "Type");
	m_list.SetColumnWidth(2, r.right - w*2);

	LRESULT r1;
	OnSelchangeTab(NULL, &r1);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSymbolsDialog::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_list.DeleteAllItems();
	if (m_tab.GetCurSel() == 1) {
		// locals
		CMainFrame *m = (CMainFrame*)theApp.GetMainWnd();
		CChildFrame *w = (CChildFrame*)m->GetActiveFrame();
		if (w) {
			CView *v = w->GetActiveView();	
			CProcDoc *doc = (CProcDoc*)v->GetDocument();
			UserProc *p = (UserProc*)doc->getProc();
			for (std::map<std::string, TypedExp *>::iterator it = p->symbols.begin(); it != p->symbols.end(); it++)
			{
				TypedExp *t = (*it).second;
				std::stringstream os;
				t->getSubExp1()->print(os);
				LVITEM i;
				i.mask = LVIF_TEXT;
				i.stateMask = 0;
				i.iItem = 0;
				i.iSubItem = 0;
				std::string s = os.str();
				i.pszText = (char *)s.c_str();			
				m_list.InsertItem(&i);
				s = (*it).first;
				m_list.SetItem(0, 1, LVIF_TEXT, s.c_str(), 0, 0, 0, 0);
				m_list.SetItem(0, 2, LVIF_TEXT, t->getType()->getCtype().c_str(), 0, 0, 0, 0);			
			}			
		}
	} else {
		// globals
		for (std::map<std::string, TypedExp *>::iterator it = prog.symbols.begin(); it != prog.symbols.end(); it++)
		{
			TypedExp *t = (*it).second;
			std::stringstream os;
			t->getSubExp1()->print(os);
			LVITEM i;
			i.mask = LVIF_TEXT;
			i.stateMask = 0;
			i.iItem = 0;
			i.iSubItem = 0;
			std::string s = os.str();
			i.pszText = (char *)s.c_str();			
			m_list.InsertItem(&i);
			s = (*it).first;
			m_list.SetItem(0, 1, LVIF_TEXT, s.c_str(), 0, 0, 0, 0);
			m_list.SetItem(0, 2, LVIF_TEXT, t->getType()->getCtype().c_str(), 0, 0, 0, 0);			
		}
	}
	*pResult = 0;
}
