#if !defined(AFX_PROCPROPERTIESDIALOG_H__823DDEC3_52EB_4136_9814_ABB2FB348B1D__INCLUDED_)
#define AFX_PROCPROPERTIESDIALOG_H__823DDEC3_52EB_4136_9814_ABB2FB348B1D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProcPropertiesDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProcPropertiesDialog dialog

class CProcPropertiesDialog : public CDialog
{
// Construction
public:
	CProcPropertiesDialog(Proc *p, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProcPropertiesDialog)
	enum { IDD = IDD_PROC_PROPERTIES };
	CListCtrl	m_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcPropertiesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	Proc *m_proc;

	void update();
	void addItem(const char *property, const char *value);

	// Generated message map functions
	//{{AFX_MSG(CProcPropertiesDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCPROPERTIESDIALOG_H__823DDEC3_52EB_4136_9814_ABB2FB348B1D__INCLUDED_)
