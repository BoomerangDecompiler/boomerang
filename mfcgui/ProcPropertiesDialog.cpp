// ProcPropertiesDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mfcgui.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "prog.h"
#include "util.h"
#include <assert.h>
#include <sstream>

#include "ProcPropertiesDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcPropertiesDialog dialog


CProcPropertiesDialog::CProcPropertiesDialog(Proc *p, CWnd* pParent /*=NULL*/)
	: CDialog(CProcPropertiesDialog::IDD, pParent), m_proc(p)
{
	//{{AFX_DATA_INIT(CProcPropertiesDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProcPropertiesDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcPropertiesDialog)
	DDX_Control(pDX, IDC_LIST, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcPropertiesDialog, CDialog)
	//{{AFX_MSG_MAP(CProcPropertiesDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcPropertiesDialog message handlers

BOOL CProcPropertiesDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	RECT r;
	m_list.GetClientRect(&r);
	int w = (r.right - r.left) / 3;
	m_list.InsertColumn(0, "Value");
	m_list.SetColumnWidth(0, r.right - w);
	m_list.InsertColumn(1, "Property");
	m_list.SetColumnWidth(1, w);

	int ord[2] = { 1, 0 };
	m_list.SetColumnOrderArray(2, ord);	

	update();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProcPropertiesDialog::addItem(const char *property, const char *value)
{
	LVITEM i;
	i.mask = LVIF_TEXT;
	i.stateMask = 0;
	i.iItem = 0;
	i.iSubItem = 0;
	i.pszText = (char *)value;
	m_list.InsertItem(&i);
	m_list.SetItem(0, 1, LVIF_TEXT, property, 0, 0, 0, 0);
}

void CProcPropertiesDialog::update()
{
	m_list.DeleteAllItems();

	addItem("Name", m_proc->getName());
}