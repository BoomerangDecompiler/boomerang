// NewSymbolDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mfcgui.h"
#include "NewSymbolDialog.h"
#include <assert.h>
#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "prog.h"
#include "proc.h"
#include "exp.h"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewSymbolDialog dialog


CNewSymbolDialog::CNewSymbolDialog(CWnd* pParent /*=NULL*/, UserProc *p, Exp *e)
	: CDialog(CNewSymbolDialog::IDD, pParent), proc(p), exp(e)
{
	//{{AFX_DATA_INIT(CNewSymbolDialog)
	//}}AFX_DATA_INIT
}


void CNewSymbolDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewSymbolDialog)
	DDX_Control(pDX, IDC_TYPE, m_type);
	DDX_Control(pDX, IDC_NAME, m_name);
	DDX_Control(pDX, IDC_EXP, m_exp);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewSymbolDialog, CDialog)
	//{{AFX_MSG_MAP(CNewSymbolDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewSymbolDialog message handlers

void CNewSymbolDialog::OnOK() 
{
	// ignore the changes to the exp for the moment
	Exp *e = exp;
	CString name;
	m_name.GetWindowText(name);
	std::string nam(name);
	std::string oldname;
	CString typestr;
	m_type.GetWindowText(typestr);
	Type *type = Type::parseType(typestr);

	TypedExp *old_exp;
	if (GetCheckedRadioButton(IDC_LOCAL, IDC_GLOBAL) == IDC_LOCAL) {
		assert(proc);
		if (proc->findSymbolFor(exp, oldname, old_exp)) {
			proc->symbols.erase(oldname);
		}
		proc->symbols[nam] = new TypedExp(type, exp);
	} else {
		if (prog.findSymbolFor(exp, oldname, old_exp)) {
			prog.symbols.erase(oldname);
		}
		prog.symbols[nam] = new TypedExp(type, exp);
	}
	
	CDialog::OnOK();
}

BOOL CNewSymbolDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (exp) {
		std::stringstream os;
		exp->print(os);
		std::string s = os.str();
		TypedExp *s_exp;
		m_exp.SetWindowText(s.c_str());
		s = "";
		if (proc) proc->findSymbolFor(exp, s, s_exp);
		else prog.findSymbolFor(exp, s, s_exp);
		m_name.SetWindowText(s.c_str());		
	}

	if (proc) {
		CheckRadioButton(IDC_LOCAL, IDC_GLOBAL, IDC_LOCAL);
	} else {
		CheckRadioButton(IDC_LOCAL, IDC_GLOBAL, IDC_GLOBAL);
		CWnd *local = this->GetDlgItem(IDC_LOCAL);
		local->ShowWindow(SW_HIDE);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
