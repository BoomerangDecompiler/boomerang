#if !defined(AFX_NEWSYMBOLDIALOG_H__08DEFCAE_172E_4E8A_8EAC_AAF450A077DF__INCLUDED_)
#define AFX_NEWSYMBOLDIALOG_H__08DEFCAE_172E_4E8A_8EAC_AAF450A077DF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewSymbolDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewSymbolDialog dialog

class UserProc;
class Exp;

class CNewSymbolDialog : public CDialog
{
// Construction
public:
	CNewSymbolDialog(CWnd* pParent = NULL, UserProc *p = NULL, Exp *e = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewSymbolDialog)
	enum { IDD = IDD_NEWSYMBOL };
	CComboBox	m_type;
	CEdit	m_name;
	CEdit	m_exp;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewSymbolDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	UserProc *proc;
	Exp *exp;

	// Generated message map functions
	//{{AFX_MSG(CNewSymbolDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWSYMBOLDIALOG_H__08DEFCAE_172E_4E8A_8EAC_AAF450A077DF__INCLUDED_)
