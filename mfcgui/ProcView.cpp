// ProcView.cpp : implementation of the CProcView class
//

#include "stdafx.h"
#include "mfcgui.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "cfg.h"
#include "prog.h"
#include "util.h"
#include <assert.h>

#include "hllcode.h"
#include "chllcode.h"
#include "dataflow.h"

#include "ChildFrm.h"
#include "ProcDoc.h"
#include "ProcView.h"
#include "RenameVariableDialog.h"
#include "EditBBDialog.h"
#include "NewSymbolDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcView

IMPLEMENT_DYNCREATE(CProcView, CEditView)

BEGIN_MESSAGE_MAP(CProcView, CEditView)
	//{{AFX_MSG_MAP(CProcView)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_RENAME_VARIABLE, OnRenameVariable)
	ON_COMMAND(ID_EDIT_BB, OnEditBb)
	ON_COMMAND(ID_HIDE_UN_LABELS, OnHideUnLabels)
	ON_COMMAND(ID_SHOW_ALL_LABELS, OnShowAllLabels)
	ON_COMMAND(ID_NEW_SYMBOL, OnNewSymbol)
	ON_COMMAND(ID_DELETE_SYMBOL, OnDeleteSymbol)
	ON_COMMAND(ID_EDIT_PROC, OnEditProc)
	ON_CONTROL_REFLECT(EN_CHANGE, OnChange)
	ON_COMMAND(ID_REMOVE_USELESS, OnRemoveUseless)
	ON_COMMAND(ID_MAKE_SSA, OnMakeSsa)
	ON_COMMAND(ID_PROPOGATE_FORWARD, OnPropogateForward)
	ON_COMMAND(ID_DEBUG_EXP, OnDebugExp)
	ON_COMMAND(ID_SIMPLIFY_EXP, OnSimplifyExp)
	ON_COMMAND(ID_REV_SSA_FORM, OnRevSsaForm)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcView construction/destruction

CProcView::CProcView() : hll(NULL)
{

}

CProcView::~CProcView()
{
}

BOOL CProcView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	BOOL bPreCreated = CEditView::PreCreateWindow(cs);
	//cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping

	return bPreCreated;
}

/////////////////////////////////////////////////////////////////////////////
// CProcView drawing

void CProcView::OnDraw(CDC* pDC)
{
}

/////////////////////////////////////////////////////////////////////////////
// CProcView diagnostics

#ifdef _DEBUG
void CProcView::AssertValid() const
{
	CEditView::AssertValid();
}

void CProcView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CProcDoc* CProcView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CProcDoc)));
	return (CProcDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CProcView message handlers

void CProcView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	int pos = GetEditCtrl().CharFromPos(point);
	int nLineIndex = HIWORD(pos);
	nCharIndex = LOWORD(pos);
	
	BasicBlock *bb = hll->getBlockAt(nCharIndex);
	Exp *e = hll->getExpAt(nCharIndex);
	Proc *call = hll->getCallAt(nCharIndex);

	CMenu popup;
	popup.CreatePopupMenu();	
	popup.AppendMenu(0, ID_EDIT_BB, "Edit basic block");
	if (call && !call->isLib())
		popup.AppendMenu(0, ID_EDIT_PROC, "Goto procedure");
	if (hll->showAllLabels)
		popup.AppendMenu(0, ID_HIDE_UN_LABELS, "Hide unneeded labels");
	else
		popup.AppendMenu(0, ID_SHOW_ALL_LABELS, "Show all labels");
	popup.AppendMenu(MFT_SEPARATOR);
	popup.AppendMenu(0, ID_NEW_SYMBOL, "New symbol");
	popup.AppendMenu(0, ID_NEW_SYMBOL, "Edit symbol");
	popup.AppendMenu(0, ID_DELETE_SYMBOL, "Delete symbol");
	popup.AppendMenu(MFT_SEPARATOR);
	if (!hll->getProc()->isSSAForm()) {
		popup.AppendMenu(0, ID_MAKE_SSA, "Make SSA form");
		popup.AppendMenu(0, ID_PROPOGATE_FORWARD, "Simple propogation");
	} else {
		popup.AppendMenu(0, ID_REV_SSA_FORM, "Reverse SSA form");
		popup.AppendMenu(0, ID_REMOVE_USELESS, "Remove useless code");
		if (e && e->getOper() == opSubscript && !hll->getProc()->getCFG()->isUsedInPhi(e))
			popup.AppendMenu(0, ID_PROPOGATE_FORWARD, "Propogate forward");
	}
	popup.AppendMenu(MFT_SEPARATOR);
	popup.AppendMenu(0, ID_DEBUG_EXP, "Debug expression");
	popup.AppendMenu(0, ID_SIMPLIFY_EXP, "Simplify expression");
	POINT r;
	r.x = point.x;
	r.y = point.y;
	this->ClientToScreen(&r);
	popup.TrackPopupMenu(0, r.x, r.y, this);
	
//	CEditView::OnRButtonDown(nFlags, point);
}

void CProcView::OnRenameVariable() 
{
	CRenameVariableDialog d;	
	if (d.DoModal() == IDOK) {

	}
	
}

void CProcView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CProcDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	if (hll == NULL) {
		Proc *p = pDoc->getProc();
		if (p)
			if (p->isLib())
				SetWindowText("This is a library procedure, you cannot view it.");
			else
				hll = new CHLLCode((UserProc*)p);
	}
	if (hll) {
		hll->reset();
		if (!hll->getProc()->generateCode(*hll)) {
			SetWindowText("Cannot establish reverse DFT ordering.  Eep!");
		} else {
			std::string s;
			hll->toString(s);
			const char *str = s.c_str();
			SetWindowText(str);
		}
	}
}


void CProcView::OnEditBb() 
{
	BasicBlock *bb = hll->getBlockAt(nCharIndex);
	CEditBBDialog d(theApp.m_pMainWnd, hll->getProc()->getCFG(), bb);
	if (d.DoModal() == IDOK) {
		int sp2 = GetScrollPos(1);
		CPoint curpos = GetCaretPos();
		OnUpdate(NULL, 0, NULL);
		GetEditCtrl().LineScroll(sp2);	
		SetCaretPos(curpos);
	}
}

void CProcView::OnHideUnLabels() 
{
	hll->showAllLabels = false;
	int sp2 = GetScrollPos(1);
	CPoint curpos = GetCaretPos();
	OnUpdate(NULL, 0, NULL);
	GetEditCtrl().LineScroll(sp2);	
	SetCaretPos(curpos);
}

void CProcView::OnShowAllLabels() 
{
	hll->showAllLabels = true;	
	int sp2 = GetScrollPos(1);
	CPoint curpos = GetCaretPos();
	OnUpdate(NULL, 0, NULL);
	GetEditCtrl().LineScroll(sp2);	
	SetCaretPos(curpos);
}

void CProcView::OnNewSymbol() 
{
	Exp *e = hll->getExpAt(nCharIndex);
	CNewSymbolDialog d(theApp.m_pMainWnd, hll->getProc(), e);
	if (d.DoModal() == IDOK) {
		int sp2 = GetScrollPos(1);
		CPoint curpos = GetCaretPos();
		OnUpdate(NULL, 0, NULL);
		GetEditCtrl().LineScroll(sp2);	
		SetCaretPos(curpos);
	}
}

void CProcView::OnDeleteSymbol() 
{
	Exp *e = hll->getExpAt(nCharIndex);
	std::string s;
	TypedExp *s_exp;
	if (e && hll->getProc()->findSymbolFor(e, s, s_exp)) {
		hll->getProc()->symbols.erase(s);
		prog.symbols.erase(s);
		int sp2 = GetScrollPos(1);
		CPoint curpos = GetCaretPos();
		OnUpdate(NULL, 0, NULL);
		GetEditCtrl().LineScroll(sp2);	
		SetCaretPos(curpos);
	}	
}

void CProcView::OnEditProc() 
{
	Proc *p = hll->getCallAt(nCharIndex);
	if (p) {
		CProcDoc *d = (CProcDoc*)theApp.m_pDocManager->OpenDocumentFile(p->getName());		
		d->setProc(p);
		d->UpdateAllViews(NULL);
	}
}

void CProcView::OnChange() 
{
	int sp2 = GetScrollPos(1);
	CPoint curpos = GetCaretPos();
	std::string s;
	hll->toString(s);
	const char *str = s.c_str();
	CString s_new;
	GetWindowText(s_new);
	// find where the change occurred
	int i;
	for (i = 0; i < s_new.GetLength(); i++)
		if (s_new[i] != str[i]) break;
	assert(i < s_new.GetLength());
	// find how big the change is
	int len;	
	for (len = 0; s_new[i+len] != str[i]; len++)
		;
	assert(len > 0);
	CString s_change = s_new.Mid(i, len);
	hll->addFormating(i, s_change);
	// update the view
	hll->toString(s);
	SetWindowText(s.c_str());
	GetEditCtrl().LineScroll(sp2);	
	SetCaretPos(curpos);
}

void CProcView::OnRemoveUseless() 
{
	hll->getProc()->removeUselessCode();
	int sp2 = GetScrollPos(1);
	CPoint curpos = GetCaretPos();
	OnUpdate(NULL, 0, NULL);
	GetEditCtrl().LineScroll(sp2);	
	SetCaretPos(curpos);
}

void CProcView::OnMakeSsa() 
{
	hll->getProc()->transformToSSAForm();	
	int sp2 = GetScrollPos(1);
	CPoint curpos = GetCaretPos();
	OnUpdate(NULL, 0, NULL);
	GetEditCtrl().LineScroll(sp2);	
	SetCaretPos(curpos);
}

void CProcView::OnPropogateForward() 
{
	if (hll->getProc()->isSSAForm()) {
		Exp *e = hll->getExpAt(nCharIndex);
		assert(e && e->getOper() == opSubscript);
		hll->getProc()->getCFG()->propogateForward(e);
	} else {
		TypedExp *a = getAssign();
		hll->getProc()->getCFG()->simplePropogate(a);
	}
	int sp2 = GetScrollPos(1);
	CPoint curpos = GetCaretPos();
	OnUpdate(NULL, 0, NULL);
	GetEditCtrl().LineScroll(sp2);	
	SetCaretPos(curpos);
}

TypedExp *CProcView::getAssign()
{
	Exp *e = hll->getExpAt(nCharIndex);
	BasicBlock *bb = hll->getBlockAt(nCharIndex);
	for (std::list<RTL*>::iterator rit = bb->getRTLs()->begin(); rit != bb->getRTLs()->end(); rit++) {
		for (std::list<Exp*>::iterator it = (*rit)->getList().begin(); it != (*rit)->getList().end(); it++) {
			if ((*it)->getSubExp1()->getSubExp1() == e) {
				assert((*it)->getOper() == opTypedExp);
				return (TypedExp*)*it;
			}
		}
	}
	return false;
}

void CProcView::OnDebugExp() 
{
	Exp *e = hll->getExpAt(nCharIndex);
	BasicBlock *bb = hll->getBlockAt(nCharIndex);
	Exp *assign = getAssign();
	assert(false);
}

void CProcView::OnSimplifyExp() 
{
	Exp *e = hll->getExpAt(nCharIndex);
	BasicBlock *bb = hll->getBlockAt(nCharIndex);
	bool found = false;
	for (std::list<RTL*>::iterator rit = bb->getRTLs()->begin(); rit != bb->getRTLs()->end(); rit++) {
		for (std::list<Exp*>::iterator it = (*rit)->getList().begin(); it != (*rit)->getList().end(); it++) {
			if ((*it)->getSubExp1()->getSubExp1() == e) {
				*it = (*it)->simplify();
				found = true;
				break;
			}
		}
		if (found) break;
	}
	int sp2 = GetScrollPos(1);
	CPoint curpos = GetCaretPos();
	OnUpdate(NULL, 0, NULL);
	GetEditCtrl().LineScroll(sp2);	
	SetCaretPos(curpos);
}

void CProcView::OnRevSsaForm() 
{
	hll->getProc()->transformFromSSAForm();
	int sp2 = GetScrollPos(1);
	CPoint curpos = GetCaretPos();
	OnUpdate(NULL, 0, NULL);
	GetEditCtrl().LineScroll(sp2);	
	SetCaretPos(curpos);	
}
