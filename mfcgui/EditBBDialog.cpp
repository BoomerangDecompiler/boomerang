// EditBBDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mfcgui.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "prog.h"
#include "util.h"
#include "cfg.h"
#include <assert.h>

#include "EditBBDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditBBDialog dialog


CEditBBDialog::CEditBBDialog(CWnd* pParent /*=NULL*/, Cfg *pcfg, PBB pbb)
	: CDialog(CEditBBDialog::IDD, pParent), cfg(pcfg), bb(pbb), lastfocus(NULL)
{
	//{{AFX_DATA_INIT(CEditBBDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditBBDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditBBDialog)
	DDX_Control(pDX, IDC_TYPE, m_type);
	DDX_Control(pDX, IDC_LOOPHEAD, m_loophead);
	DDX_Control(pDX, IDC_LOOPFOLLOW, m_loopfollow);
	DDX_Control(pDX, IDC_LATCHNODE, m_latchnode);
	DDX_Control(pDX, IDC_CONDTYPE, m_condtype);
	DDX_Control(pDX, IDC_CONDFOLLOW, m_condfollow);
	DDX_Control(pDX, IDC_CASEHEAD, m_casehead);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditBBDialog, CDialog)
	//{{AFX_MSG_MAP(CEditBBDialog)
	ON_CBN_KILLFOCUS(IDC_CONDFOLLOW, OnKillfocusCondfollow)
	ON_CBN_SELCHANGE(IDC_CASEHEAD, OnSelchangeCasehead)
	ON_CBN_SELCHANGE(IDC_CONDFOLLOW, OnSelchangeCondfollow)
	ON_CBN_SELCHANGE(IDC_LATCHNODE, OnSelchangeLatchnode)
	ON_CBN_SELCHANGE(IDC_LOOPFOLLOW, OnSelchangeLoopfollow)
	ON_CBN_SELCHANGE(IDC_LOOPHEAD, OnSelchangeLoophead)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditBBDialog message handlers

BOOL CEditBBDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CString s;
	GetWindowText(s);
	std::stringstream a;
	a << std::hex << bb->getLowAddr() << std::ends;
	s += a.str().c_str();
	SetWindowText(s);

	BB_IT it;
	BasicBlock *a_bb;
	int nbbs = 0;
	m_loophead.AddString("NONE");		
	m_loopfollow.AddString("NONE");
	m_latchnode.AddString("NONE");
	m_condfollow.AddString("NONE");
	m_casehead.AddString("NONE");		
	ntobb[nbbs] = NULL;
	bbton[NULL] = nbbs++;
	for (a_bb = cfg->getFirstBB(it); a_bb; a_bb = cfg->getNextBB(it)) {
		std::stringstream os;
		os << std::hex << a_bb->getLowAddr() << std::ends;
		ntobb[nbbs] = a_bb;
		bbton[a_bb] = nbbs++;
		m_loophead.AddString(os.str().c_str());		
		m_loopfollow.AddString(os.str().c_str());
		m_latchnode.AddString(os.str().c_str());
		m_condfollow.AddString(os.str().c_str());
		m_casehead.AddString(os.str().c_str());		
	}
	
	m_type.SetCurSel((int)bb->m_structType);
	m_condtype.SetCurSel((int)bb->m_loopCondType);
	m_loophead.SetCurSel(bbton[bb->m_loopHead]);
	m_casehead.SetCurSel(bbton[bb->m_caseHead]);
	m_condfollow.SetCurSel(bbton[bb->m_condFollow]);
	m_loopfollow.SetCurSel(bbton[bb->m_loopFollow]);
	m_latchnode.SetCurSel(bbton[bb->m_latchNode]);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditBBDialog::OnOK() 
{
	bb->m_structType = (SBBTYPE)(m_type.GetCurSel());
	bb->m_loopCondType = (SBBTYPE)(m_condtype.GetCurSel());
	bb->m_loopHead = ntobb[m_loophead.GetCurSel()];
	bb->m_caseHead = ntobb[m_casehead.GetCurSel()];
	bb->m_condFollow = ntobb[m_condfollow.GetCurSel()];
	bb->m_loopFollow = ntobb[m_loopfollow.GetCurSel()];
	bb->m_latchNode = ntobb[m_latchnode.GetCurSel()];
	
	CDialog::OnOK();
}

void CEditBBDialog::graphicalSelect(PBB pbb)
{
	SetFocus();
	if (lastfocus)
		lastfocus->SetCurSel(bbton[pbb]);
}

void CEditBBDialog::OnKillfocusCondfollow() 
{
	lastfocus = &m_condfollow;	
}

void CEditBBDialog::OnSelchangeCasehead() 
{
	lastsel = ntobb[m_casehead.GetCurSel()];
}

void CEditBBDialog::OnSelchangeCondfollow() 
{
	lastsel = ntobb[m_condfollow.GetCurSel()];
}

void CEditBBDialog::OnSelchangeLatchnode() 
{
	lastsel = ntobb[m_latchnode.GetCurSel()];
}

void CEditBBDialog::OnSelchangeLoopfollow() 
{
	lastsel = ntobb[m_loopfollow.GetCurSel()];
}

void CEditBBDialog::OnSelchangeLoophead() 
{
	lastsel = ntobb[m_loophead.GetCurSel()];	
}
