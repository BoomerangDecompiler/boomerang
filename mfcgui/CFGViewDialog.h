#if !defined(AFX_CFGVIEWDIALOG_H__2178B903_9599_42C0_8A35_7122F6F18C01__INCLUDED_)
#define AFX_CFGVIEWDIALOG_H__2178B903_9599_42C0_8A35_7122F6F18C01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CFGViewDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCFGViewDialog dialog

class Cfg;
class BasicBlock;
class CEditBBDialog;
class CProcDoc;

class Node {
protected:
	BasicBlock *bb;
	CPoint p;
	int fx, fy;
	int level;
	std::map<BasicBlock *, Node*> &nodes;

	Node(std::map<BasicBlock *, Node*> &n) : bb(NULL), nodes(n) { }

public:
	Node(BasicBlock *pbb, RECT &r, std::map<BasicBlock *, Node*> &n) : bb(pbb), nodes(n) { 
		p.x = r.left;
		p.y = r.top;
		nodes[bb] = this;
	}

	CPoint &getPoint() { return p; }
virtual CSize getSize() { return CSize(10, 10); }

	void jiggleAway(Node *other, RECT &bound);
	void jiggleStatic(RECT &bound);
	void clearForce() { fx = fy = 0; }
	bool applyForce(RECT &bound);
	int getLevel() { return level; }
	void setLevel(int i) { level = i; }

virtual bool hasBackEdgeTo(Node *d);	
virtual void setTraversed(bool b);
virtual bool isTraversed();
virtual int getNumInEdges();
virtual Node *getInEdge(int j);
virtual int getNumOutEdges();
virtual Node *getOutEdge(int j); 

virtual void mergeWith(Node *other);
virtual PBB getBB() { return bb; }  // dont use this, it's for derived classes only
};

// represents a number of nodes
class MultiNode : public Node {
protected:
	std::vector<PBB> bbs;
	bool traversed;

public:
	MultiNode(Node &n) : Node(n), traversed(false) { 
		bbs.push_back(bb);
		nodes[bb] = this;
		bb = NULL;
	}

	CPoint &getPoint() { return p; }
virtual CSize getSize() { return CSize(10, 20); }

virtual bool hasBackEdgeTo(Node *d);	
virtual void setTraversed(bool b);
virtual bool isTraversed();
virtual int getNumInEdges();
virtual Node *getInEdge(int j);
virtual int getNumOutEdges();
virtual Node *getOutEdge(int j); 
virtual void mergeWith(Node *other);
virtual PBB getBB() { return bbs.front(); }
};

class CCFGViewDialog : public CDialog
{
// Construction
public:
	CCFGViewDialog(CWnd* pParent = NULL, Cfg *c = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCFGViewDialog)
	enum { IDD = IDD_CFGVIEW };
	CButton	m_autoarrange;
	CTreeCtrl	m_tree;
	CStatic	m_graph;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCFGViewDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	Cfg *cfg;
	std::map<BasicBlock *, Node *> nodes;
	std::map<BasicBlock *, HTREEITEM> items;
	Node *curdrag;
	CEditBBDialog *edit;
	CProcDoc *doc;
	bool indicatingFollow;
	bool indicatingLatch;
	HCURSOR hIndicateFollow;
	HCURSOR hIndicateLatch;

	void indicateFollow();
	void indicateLatch();
	void setLevels();
	void compressNodes();

	// Generated message map functions
	//{{AFX_MSG(CCFGViewDialog)
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnEditBb();
	afx_msg void OnMakeIfThen();
	afx_msg void OnMakeIfThenElse();
	afx_msg void OnChangeFollow();
	afx_msg void OnClickTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeLatch();
	afx_msg void OnMakePretestedLoop();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CFGVIEWDIALOG_H__2178B903_9599_42C0_8A35_7122F6F18C01__INCLUDED_)
