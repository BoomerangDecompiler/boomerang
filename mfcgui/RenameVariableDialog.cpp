// RenameVariableDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mfcgui.h"
#include "RenameVariableDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRenameVariableDialog dialog


CRenameVariableDialog::CRenameVariableDialog(RenameDir pDir, CWnd* pParent /*=NULL*/)
	: CDialog(CRenameVariableDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRenameVariableDialog)
	m_name = _T("");
	m_newname = _T("");
	//}}AFX_DATA_INIT
	m_dir = pDir;
}


void CRenameVariableDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRenameVariableDialog)
	DDX_CBString(pDX, IDC_NAME, m_name);
	DDX_CBString(pDX, IDC_NEWNAME, m_newname);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRenameVariableDialog, CDialog)
	//{{AFX_MSG_MAP(CRenameVariableDialog)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	ON_BN_CLICKED(IDC_BOTH, OnBoth)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRenameVariableDialog message handlers

void CRenameVariableDialog::OnUp() 
{
	m_dir = RENAME_UP;	
}

void CRenameVariableDialog::OnDown() 
{
	m_dir = RENAME_DOWN;
}

void CRenameVariableDialog::OnBoth() 
{
	m_dir = RENAME_BOTH;	
}

int CRenameVariableDialog::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (m_dir == RENAME_UP)
		SetDlgItemInt(IDC_UP, 1);
	else if (m_dir == RENAME_DOWN)
		SetDlgItemInt(IDC_DOWN, 1);
	else
		SetDlgItemInt(IDC_BOTH, 1);
	return 0;
}
