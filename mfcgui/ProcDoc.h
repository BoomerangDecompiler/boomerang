// ProcDoc.h : interface of the CProcDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCDOC_H__C539B954_4095_4621_ABF5_67218E5FCB31__INCLUDED_)
#define AFX_PROCDOC_H__C539B954_4095_4621_ABF5_67218E5FCB31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Proc;

class CProcDoc : public CDocument
{
protected: // create from serialization only
	CProcDoc();
	DECLARE_DYNCREATE(CProcDoc)

// Attributes
public:
	void setProc(Proc *p);
	Proc *getProc() { return m_proc; }

private:
	Proc *m_proc;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProcDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CProcDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCDOC_H__C539B954_4095_4621_ABF5_67218E5FCB31__INCLUDED_)
