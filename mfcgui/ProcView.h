// ProcView.h : interface of the CProcView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROCVIEW_H__ECF13700_4F54_424C_9CD6_95881FFC23FC__INCLUDED_)
#define AFX_PROCVIEW_H__ECF13700_4F54_424C_9CD6_95881FFC23FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class HLLCode;

class CProcView : public CEditView
{
private:
	HLLCode *hll;

protected: // create from serialization only
	CProcView();
	DECLARE_DYNCREATE(CProcView)

// Attributes
public:
	CProcDoc* GetDocument();	

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProcView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	int nCharIndex;

	TypedExp *getAssign();

// Generated message map functions
protected:
	//{{AFX_MSG(CProcView)
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRenameVariable();
	afx_msg void OnEditBb();
	afx_msg void OnHideUnLabels();
	afx_msg void OnShowAllLabels();
	afx_msg void OnNewSymbol();
	afx_msg void OnDeleteSymbol();
	afx_msg void OnEditProc();
	afx_msg void OnChange();
	afx_msg void OnRemoveUseless();
	afx_msg void OnMakeSsa();
	afx_msg void OnPropogateForward();
	afx_msg void OnDebugExp();
	afx_msg void OnSimplifyExp();
	afx_msg void OnRevSsaForm();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in ProcView.cpp
inline CProcDoc* CProcView::GetDocument()
   { return (CProcDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROCVIEW_H__ECF13700_4F54_424C_9CD6_95881FFC23FC__INCLUDED_)
