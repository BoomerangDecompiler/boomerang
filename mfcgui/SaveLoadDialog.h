#if !defined(AFX_SAVELOADDIALOG_H__D5E0ECA5_6444_4443_AB85_2A8BB5933961__INCLUDED_)
#define AFX_SAVELOADDIALOG_H__D5E0ECA5_6444_4443_AB85_2A8BB5933961__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SaveLoadDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaveLoadDialog dialog

class ProgWatcher;

class CSaveLoadDialog : public CDialog, public ProgWatcher
{
// Construction
public:
	CSaveLoadDialog(bool save, CWnd* pParent = NULL);   // standard constructor

	virtual void alert_complete();
	virtual void alert_new(Proc *p);
	virtual void alert_decode(ADDRESS pc, int nBytes);
	virtual void alert_done(Proc *p, ADDRESS pc, ADDRESS last, int nBytes);
	virtual void alert_baddecode(ADDRESS pc);
	virtual void alert_progress(unsigned long off, unsigned long size);
	void setMessage(std::string &str);

// Dialog Data
	//{{AFX_DATA(CSaveLoadDialog)
	enum { IDD = IDD_SAVELOAD };
	CProgressCtrl	m_progress;
	CStatic	m_message;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaveLoadDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	DWORD m_thread_id;
	HANDLE m_thread;
	bool m_save;

	// Generated message map functions
	//{{AFX_MSG(CSaveLoadDialog)
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SAVELOADDIALOG_H__D5E0ECA5_6444_4443_AB85_2A8BB5933961__INCLUDED_)
