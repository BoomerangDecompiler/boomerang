#if !defined(AFX_EDITBBDIALOG_H__4195A9D2_613C_4C6E_B715_94E72ABF6A3E__INCLUDED_)
#define AFX_EDITBBDIALOG_H__4195A9D2_613C_4C6E_B715_94E72ABF6A3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditBBDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditBBDialog dialog

class CEditBBDialog : public CDialog
{
// Construction
public:
	CEditBBDialog(CWnd* pParent = NULL, Cfg *pcfg = NULL, PBB pbb = NULL);   // standard constructor

	void graphicalSelect(PBB pbb);
	BasicBlock *lastsel;

// Dialog Data
	//{{AFX_DATA(CEditBBDialog)
	enum { IDD = IDD_EDITBB };
	CComboBox	m_type;
	CComboBox	m_loophead;
	CComboBox	m_loopfollow;
	CComboBox	m_latchnode;
	CComboBox	m_condtype;
	CComboBox	m_condfollow;
	CComboBox	m_casehead;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditBBDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BasicBlock *bb;
	Cfg *cfg;
	std::map<int, BasicBlock*> ntobb;
	std::map<BasicBlock*, int> bbton;
	CComboBox *lastfocus;

	// Generated message map functions
	//{{AFX_MSG(CEditBBDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnKillfocusCondfollow();
	afx_msg void OnSelchangeCasehead();
	afx_msg void OnSelchangeCondfollow();
	afx_msg void OnSelchangeLatchnode();
	afx_msg void OnSelchangeLoopfollow();
	afx_msg void OnSelchangeLoophead();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITBBDIALOG_H__4195A9D2_613C_4C6E_B715_94E72ABF6A3E__INCLUDED_)
