#if !defined(AFX_PROCEDURESDIALOG_H__652588EF_CAFF_4C84_8841_18EDA8F66003__INCLUDED_)
#define AFX_PROCEDURESDIALOG_H__652588EF_CAFF_4C84_8841_18EDA8F66003__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProceduresDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProceduresDialog dialog

class CProceduresDialog : public CDialog
{
// Construction
public:
	CProceduresDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProceduresDialog)
	enum { IDD = IDD_PROCEDURES };
	CTreeCtrl	m_calls;
	CTreeCtrl	m_calledby;
	CTreeCtrl	m_procs;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProceduresDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	bool m_leaves;

	void addToProcs(Proc *p);
	void strForProc(Proc *p, CString &s);

	std::map<Proc*, HTREEITEM> m_procitem;

	void redraw();

	// Generated message map functions
	//{{AFX_MSG(CProceduresDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedProcs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedCalls(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickCalls(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkCalls(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickProcs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditCode();
	afx_msg void OnProperties();
	afx_msg void OnViewControlflow();
	afx_msg void OnShowLeaves();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCEDURESDIALOG_H__652588EF_CAFF_4C84_8841_18EDA8F66003__INCLUDED_)
