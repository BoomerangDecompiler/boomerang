#if !defined(AFX_RENAMEVARIABLEDIALOG_H__607F5420_8DB5_49A5_A650_6401E0C8CDFE__INCLUDED_)
#define AFX_RENAMEVARIABLEDIALOG_H__607F5420_8DB5_49A5_A650_6401E0C8CDFE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RenameVariableDialog.h : header file
//

typedef enum { RENAME_UP, RENAME_DOWN, RENAME_BOTH } RenameDir;

/////////////////////////////////////////////////////////////////////////////
// CRenameVariableDialog dialog

class CRenameVariableDialog : public CDialog
{
// Construction
public:

	CRenameVariableDialog(RenameDir pDir = RENAME_BOTH, CWnd* pParent = NULL);   // standard constructor
	
	RenameDir m_dir;

// Dialog Data
	//{{AFX_DATA(CRenameVariableDialog)
	enum { IDD = IDD_RENAME_VARIABLE };
	CString	m_name;
	CString	m_newname;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRenameVariableDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRenameVariableDialog)
	afx_msg void OnUp();
	afx_msg void OnDown();
	afx_msg void OnBoth();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RENAMEVARIABLEDIALOG_H__607F5420_8DB5_49A5_A650_6401E0C8CDFE__INCLUDED_)
