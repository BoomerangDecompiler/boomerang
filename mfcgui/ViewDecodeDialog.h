#if !defined(AFX_VIEWDECODEDIALOG_H__95CD0297_D143_48A4_882F_A88BB8205910__INCLUDED_)
#define AFX_VIEWDECODEDIALOG_H__95CD0297_D143_48A4_882F_A88BB8205910__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewDecodeDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CViewDecodeDialog dialog

class ProgWatcher;

class CViewDecodeDialog : public CDialog, public ProgWatcher
{
// Construction
public:
	CViewDecodeDialog(CWnd* pParent = NULL);   // standard constructor
	~CViewDecodeDialog();

// Dialog Data
	//{{AFX_DATA(CViewDecodeDialog)
	enum { IDD = IDD_VIEWDECODE };
	CButton	m_cancel;
	CStatic	m_undecoded;
	CTreeCtrl	m_proctree;
	CStatic	m_procedure;
	CStatic	m_problem;
	CStatic	m_instruction;
	CStatic	m_complete;
	CStatic	m_status;
	CProgressCtrl	m_progress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewDecodeDialog)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual void alert_complete();
	virtual void alert_new(Proc *p);
	virtual void alert_decode(ADDRESS pc, int nBytes);
	virtual void alert_done(Proc *p, ADDRESS pc, ADDRESS last, int nBytes);
	virtual void alert_baddecode(ADDRESS pc);
	virtual void alert_progress(unsigned long off, unsigned long size);

private:
	std::list<std::pair<ADDRESS, int>* > instructions;
	std::list<std::pair<ADDRESS, int>* > procs;
	std::list<ADDRESS>					 problems;
	DWORD m_thread_id;
	HANDLE m_thread;
	bool m_locked;

	int linesize;
	int unitsize;
	int box;
	int nlines;
	int oneline;
	double scaleamt;
	void getUpdateRect(RECT &r1, int line, ADDRESS a, int nBytes);
	CDC buf;


protected:

	// Generated message map functions
	//{{AFX_MSG(CViewDecodeDialog)
	virtual void OnCancel();
	afx_msg void OnSelchangedProctree(NMHDR* pNMHDR, LRESULT* pResult);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRclickProctree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclkProctree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWDECODEDIALOG_H__95CD0297_D143_48A4_882F_A88BB8205910__INCLUDED_)
