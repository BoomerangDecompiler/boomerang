#if !defined(AFX_NEWPROJECTDIALOG_H__ABF8E455_3F98_4CB1_BC5F_2346069FD686__INCLUDED_)
#define AFX_NEWPROJECTDIALOG_H__ABF8E455_3F98_4CB1_BC5F_2346069FD686__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NewProjectDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CNewProjectDialog dialog

class BinaryFile;
class FrontEnd;

class CNewProjectDialog : public CDialog
{
// Construction
public:
	FrontEnd * getFrontEnd();
	BinaryFile * getLoader();
	CNewProjectDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNewProjectDialog)
	enum { IDD = IDD_NEW_PROJECT };
	CEdit	m_location;
	CEdit	m_name;
	CEdit	m_filename;
	CListBox	m_loader;
	CListBox	m_frontend;
	//}}AFX_DATA
	CString m_filename_str;
	CString m_name_str;
	CString m_location_str;
	int m_loader_n;
	int m_frontend_n;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewProjectDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewProjectDialog)
	afx_msg void OnBrowseLocation();
	afx_msg void OnBrowseFilename();
	afx_msg void OnChangeFilename();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWPROJECTDIALOG_H__ABF8E455_3F98_4CB1_BC5F_2346069FD686__INCLUDED_)
