#if !defined(AFX_SYMBOLSDIALOG_H__07702EA9_3323_4069_83AF_4CB96E25DF7A__INCLUDED_)
#define AFX_SYMBOLSDIALOG_H__07702EA9_3323_4069_83AF_4CB96E25DF7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SymbolsDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSymbolsDialog dialog

class CSymbolsDialog : public CDialog
{
// Construction
public:
	CSymbolsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSymbolsDialog)
	enum { IDD = IDD_SYMBOLS };
	CListCtrl	m_list;
	CTabCtrl	m_tab;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSymbolsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSymbolsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SYMBOLSDIALOG_H__07702EA9_3323_4069_83AF_4CB96E25DF7A__INCLUDED_)
