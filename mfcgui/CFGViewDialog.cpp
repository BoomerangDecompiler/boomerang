// CFGViewDialog.cpp : implementation file
//

#include "stdafx.h"
#include "mfcgui.h"
#include "math.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "prog.h"
#include "cfg.h"
#include "util.h"
#include <assert.h>

#include "CFGViewDialog.h"
#include "EditBBDialog.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "ProcDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCFGViewDialog dialog


CCFGViewDialog::CCFGViewDialog(CWnd* pParent /*=NULL*/, Cfg *c)
	: CDialog(CCFGViewDialog::IDD, pParent), curdrag(NULL), edit(NULL), cfg(c), doc(NULL), indicatingFollow(false),
	indicatingLatch(false)
{
	//{{AFX_DATA_INIT(CCFGViewDialog)
	//}}AFX_DATA_INIT
}


void CCFGViewDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCFGViewDialog)
	DDX_Control(pDX, IDC_AUTOARRANGE, m_autoarrange);
	DDX_Control(pDX, IDC_TREE, m_tree);
	DDX_Control(pDX, IDC_GRAPH, m_graph);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCFGViewDialog, CDialog)
	//{{AFX_MSG_MAP(CCFGViewDialog)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY(NM_RCLICK, IDC_TREE, OnRclickTree)
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	ON_COMMAND(ID_EDIT_BB, OnEditBb)
	ON_COMMAND(ID_MAKE_IF_THEN, OnMakeIfThen)
	ON_COMMAND(ID_MAKE_IF_THEN_ELSE, OnMakeIfThenElse)
	ON_COMMAND(ID_CHANGE_FOLLOW, OnChangeFollow)
	ON_NOTIFY(NM_CLICK, IDC_TREE, OnClickTree)
	ON_COMMAND(ID_CHANGE_LATCH, OnChangeLatch)
	ON_COMMAND(ID_MAKE_PRETESTED_LOOP, OnMakePretestedLoop)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCFGViewDialog message handlers

void CCFGViewDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CPen pen(PS_SOLID, 1, (COLORREF)0);
	CPen redp(PS_SOLID, 1, (COLORREF)0x0000ff);
	CPen dot(PS_DOT, 1, (COLORREF)0);
	CPen dash(PS_DASH, 1, (COLORREF)0x00ff00);

	for (std::map<BasicBlock *, Node *>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		POINT b = (*it).second->getPoint();
		SIZE sz = (*it).second->getSize();
		m_graph.ClientToScreen(&b);
		ScreenToClient(&b);
		PBB bb = (*it).first;
		PBB cursel = (PBB)m_tree.GetItemData(m_tree.GetSelectedItem());
		if (cursel == bb) {
			RECT b1 = CRect(b.x, b.y, b.x + sz.cx, b.y + sz.cy);
			b1.top -= 2;
			b1.bottom += 2;
			b1.left -= 2;
			b1.right += 2;
			dc.SelectObject(&dot);
			dc.Rectangle(&b1);
		}
		if (edit && edit->lastsel == bb) {
			RECT b1 = CRect(b.x, b.y, b.x + sz.cx, b.y + sz.cy);
			b1.top -= 2;
			b1.bottom += 2;
			b1.left -= 2;
			b1.right += 2;
			dc.SelectObject(&dash);
			dc.Rectangle(&b1);
		}
		dc.SelectObject(&pen);
		switch(bb->m_structType) {
			case IFTHEN:
			case IFTHENELSE:
			case IFELSE:
				dc.MoveTo(b.x + sz.cx / 2, b.y);
				dc.LineTo(b.x + sz.cx, b.y + sz.cy / 2);
				dc.LineTo(b.x + sz.cx / 2, b.y + sz.cy);
				dc.LineTo(b.x, b.y + sz.cy / 2);
				dc.LineTo(b.x + sz.cx / 2, b.y);
				break;
			case PRETESTLOOP:
			case POSTTESTLOOP:
			case ENDLESSLOOP:
				dc.MoveTo(b.x, b.y);
				dc.LineTo(b.x + sz.cx, b.y);
				dc.LineTo(b.x + sz.cx, b.y + sz.cy);
				dc.LineTo(b.x, b.y + sz.cy);
				dc.LineTo(b.x, b.y);
				break;
			default:
				{
					RECT b1 = CRect(b.x, b.y, b.x + sz.cx, b.y + sz.cy);
					dc.Ellipse(&b1);
				}
		}
		Node *n = (*it).second;
		for (int i = 0; i < n->getNumOutEdges(); i++) {
			Node *d = n->getOutEdge(i);
			POINT t = d->getPoint();
			SIZE tsz = d->getSize();
			m_graph.ClientToScreen(&t);
			ScreenToClient(&t);
//			if (d->getBB()->isAncestorOf(bb) || bb == d->getBB()) {
//				dc.SelectObject(&redp);
//			}
			dc.MoveTo(b.x + sz.cx / 2, b.y + sz.cy);
			dc.LineTo(t.x + tsz.cx / 2, t.y);
		}
	}

	// Do not call CDialog::OnPaint() for painting messages
}

BOOL CCFGViewDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (cfg == NULL) {
		CMainFrame *m = (CMainFrame*)theApp.GetMainWnd();
		CChildFrame *w = (CChildFrame*)m->GetActiveFrame();
		CView *v = w->GetActiveView();	
		doc = (CProcDoc*)v->GetDocument();
		UserProc *p = (UserProc*)doc->getProc();
		cfg = p->getCFG();
	}
	
	RECT r;
	m_graph.GetClientRect(&r);

	RECT r1;
	r1.top = r.top + (r.bottom - r.top) / 2 - 5;
	r1.bottom = r.top + (r.bottom - r.top) / 2 + 5;
	r1.left = r.left + (r.right - r.left) / 2 - 5;
	r1.right = r.left + (r.right - r.left) / 2 + 5;

	BB_IT it;
	BasicBlock *bb;

	int nbbs = 0;
	for (bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it))
		nbbs++;
	for (bb = cfg->getFirstBB(it); bb; bb = cfg->getNextBB(it)) {
		std::ostringstream os;
		os << std::hex << bb->getLowAddr() << std::ends;
		items[bb] = m_tree.InsertItem(os.str().c_str());
		m_tree.SetItemData(items[bb], (DWORD)bb);
		r1.left = bb->m_DFTfirst * (r.right - r.left) / nbbs;
		if (r1.left > (r.right - 15))
			r1.left = r.right - 15;
		r1.right = r1.left + 10;
		r1.top = r.bottom - 30;
		r1.bottom = r.bottom - 20;
		nodes[bb] = new Node(bb, r1, nodes);
	}

	//compressNodes();
	setLevels();

	m_autoarrange.SetCheck(1);

	SetTimer(WM_TIMER, 100, NULL);

	hIndicateFollow = theApp.LoadCursor(IDC_INDICATE_FOLLOW);
	hIndicateLatch = theApp.LoadCursor(IDC_INDICATE_LATCH);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// nodes should be repelled by all other nodes, but slightly less to nodes
// that they have edges to, they should try to move below nodes they have
// in edges from
void Node::jiggleAway(Node *other, RECT &bound)
{
	for (int i = 0; i < getNumInEdges(); i++)
		if (getInEdge(i) == other) return;

	CPoint &o = other->getPoint();

	int dx = o.x - p.x;
	int dy = o.y - p.y;
	int d = (int)sqrt((double)(dx*dx + dy*dy));
	if (d <= 15) {
		if (dx == 0) 
			fx += (rand()%11) - 5;		
		else {
			if (dx < 0)
				fx += (int)(15/sqrt((double)abs(dx)));
			else
				fx -= (int)(15/sqrt((double)abs(dx)));
		}
		if (dy == 0) 
			fy += (rand()%11) - 5;
		else {
			if (dy < 0)
				fy += (int)(15/sqrt((double)abs(dy)));
			else
				fy -= (int)(15/sqrt((double)abs(dy)));
		}
	}
}

bool Node::applyForce(RECT &bound)
{
	bool change = false;

	if (getNumInEdges() == 0) {
		p.x = bound.left + (bound.right - bound.left) / 2 - 5;
		p.y = bound.top;
		return false;
	}

	if (getNumOutEdges() == 0) {
		p.y = bound.bottom - 10;
		return false;
	}

	// move horizontal if ok
	if (fx && (p.x+fx) > bound.left && ((p.x + getSize().cx)+fx) < bound.right) {
		p.x += fx;		
		change = true;
	}
	// move vertical if ok
	if (fy && (p.y+fy) > bound.top && ((p.y + getSize().cy)+fy) < bound.bottom) {
		p.y += fy;
		change = true;
	}

	return change;
}

void Node::jiggleStatic(RECT &bound)
{
	// bounce off walls
	if (p.x < bound.left + 15) {
		fx += (int)(15/sqrt((double)p.x - bound.left));
	}
	if ((p.x + getSize().cx) > bound.right - 15) {
		fx -= (int)(15/sqrt((double)bound.right - (p.x + getSize().cx)));
	}
	if (p.y < bound.top + 15) {
		fy += (int)(15/sqrt((double)p.y - bound.top));
	}
	if ((p.y + getSize().cy) > bound.bottom - 15) {
		fy -= (int)(15/sqrt((double)bound.bottom - (p.y + getSize().cy)));
	}

	// move up if you're below one of your out edges (unless it's a backedge)
	int i;
	for (i = 0; i < getNumOutEdges(); i++) 
		if (!hasBackEdgeTo(getOutEdge(i))) {
			Node *other = getOutEdge(i);
			assert(other);
			CPoint &o = other->getPoint();
			if (p.y > o.y || (o.y - p.y) < 10)
				fy -= 5;
		}

	// move down if you're above one of your in edges (unless it's a backedge)
	for (i = 0; i < getNumInEdges(); i++) 
		if (!getInEdge(i)->hasBackEdgeTo(this)) {
			Node *other = getInEdge(i);
			assert(other);
			CPoint &o = other->getPoint();
			if (p.y < o.y || (p.y - o.y) < 10)
				fy += 5;
		}

	// strafe so that you are vertically below a single in edge
	if (getNumInEdges() == 1 && getInEdge(0)->getNumOutEdges() == 1) {
		Node *other = getInEdge(0);
		assert(other);
		CPoint &o = other->getPoint();
		if (o.x < p.x)
			fx--;
		else if (o.x > p.x)
			fx++;
	}

	// strafe so that you are in the middle of your two in edges
	if (getNumInEdges() == 2) {
		Node *parent1 = getInEdge(0);
		assert(parent1);
		Node *parent2 = getInEdge(1);
		assert(parent2);
		CPoint &p1 = parent1->getPoint();
		CPoint &p2 = parent2->getPoint();
		if (p1.x > p.x && p2.x > p.x && ((p1.x - p.x) > 15 || (p2.x - p.x) > 15))
			fx += 5;
		else if (p1.x < p.x && p2.x < p.x && ((p.x - p1.x) > 15 || (p.x - p2.x) > 15))
			fx -= 5;
		else {
			int middle = p1.x + (p2.x - p1.x) / 2;
			if (p.x < middle)
				fx++;
			else if (p.x > middle)
				fx--;	
		}
	}

	// strafe so that you and a single sibling are the same distance from the parent
	if (getNumInEdges() == 1 && getInEdge(0)->getNumOutEdges() == 2) {
		Node *other = getInEdge(0);
		assert(other);
		Node *sibling = other->getOutEdge(0);
		if (sibling == this)
			sibling = other->getOutEdge(1);
		CPoint &o = other->getPoint();
		CPoint &s = sibling->getPoint();
		if (s.x < o.x && p.x < o.x) {
			fx += 5;
		} else
		if (s.x > o.x && p.x > o.x) {
			fx -= 5;
		} else {
			int d = abs(s.x - o.x);
			int md = abs(p.x - o.x);
			if (abs(md - d) > 20) {
				if (md < d) {
					if (p.x < o.x)
						fx--;
					else 
						fx++;
				} else {
					if (p.x < o.x)
						fx++;
					else 
						fx--;
				}
			} else
			// make a diamond if both siblings have the same out edge
			if (getNumOutEdges() == 1 && sibling->getNumOutEdges() == 1 &&
				getOutEdge(0) == sibling->getOutEdge(0)) {
				Node *child = getOutEdge(0);
				assert(child);
				CPoint &c = child->getPoint();
				int vd = abs(c.y - o.y);
				if (abs(md - vd) > 2) {
					if (md < vd) {
						if (p.x < o.x)
							fx--;
						else
							fx++;
					} else {
						if (p.x < o.x)
							fx++;
						else
							fx--;
					}					
				}
				int middle = c.y + (o.y - c.y) / 2;
				if (p.y > middle)
					fy--;
				else if (p.y < middle)
					fy++;
			}
		}
		// vertically siblings want to be at the same level
		if (abs(s.y - p.y) > 2) {
			if (s.y > p.y)
				fy++;
			else
				fy--;
		}
	}
}

void Node::setTraversed(bool b) 
{ 
	bb->setTraversed(b); 
}

bool Node::isTraversed() 
{ 
	return bb->isTraversed(); 
}

int Node::getNumInEdges() 
{ 
	return bb->getInEdges().size();
}

Node *Node::getInEdge(int j)
{ 
	return nodes[bb->getInEdges()[j]]; 
}

int Node::getNumOutEdges() 
{ 
	return bb->getOutEdges().size();
}

Node *Node::getOutEdge(int j) 
{ 
	return nodes[bb->getOutEdge(j)];
}

bool Node::hasBackEdgeTo(Node *d) 
{ 
	return bb->hasBackEdgeTo(d->getBB()); 
}

void Node::mergeWith(Node *other)
{
	Node *n = new MultiNode(*this);
	n->mergeWith(other);
	delete this;
}

void MultiNode::setTraversed(bool b)
{
	traversed = b;
}

bool MultiNode::isTraversed()
{
	return traversed;
}

int MultiNode::getNumInEdges()
{
	return bbs[0]->getInEdges().size();
}

Node *MultiNode::getInEdge(int j)
{
	return nodes[bbs[0]->getInEdges()[j]];
}

int MultiNode::getNumOutEdges()
{
	return bbs.back()->getOutEdges().size();
}

Node *MultiNode::getOutEdge(int j)
{
	return nodes[bbs.back()->getOutEdges()[j]];
}

void MultiNode::mergeWith(Node *other)
{
	assert(other != this);
	if (other->getBB() == NULL) {
		// multi with multi
		MultiNode *n = (MultiNode*)other;
		for (int i = 0; i < n->bbs.size(); i++) {
			bbs.push_back(n->bbs[i]);
			nodes[n->bbs[i]] = this;
		}
	} else {
		// multi with single
		bbs.push_back(other->getBB());
		nodes[other->getBB()] = this;
	}
	delete other;
}

bool MultiNode::hasBackEdgeTo(Node *d) 
{ 
	return bbs.back()->hasBackEdgeTo(d->getBB()); 
}

void CCFGViewDialog::OnTimer(UINT nIDEvent) 
{
	if (m_autoarrange.GetCheck()) {
		RECT r;
		m_graph.GetClientRect(&r);

		for (std::map<BasicBlock *, Node *>::iterator itc = nodes.begin(); itc != nodes.end(); itc++)		
			(*itc).second->clearForce();

		// jiggle the nodes
		for (std::map<BasicBlock *, Node *>::iterator it = nodes.begin(); it != nodes.end(); it++) {
			(*it).second->jiggleStatic(r);
			for (std::map<BasicBlock *, Node *>::iterator it1 = nodes.begin(); it1 != nodes.end(); it1++)
					if ((*it).second != (*it1).second)
						(*it).second->jiggleAway((*it1).second, r);
		}

		bool change = false;
		for (std::map<BasicBlock *, Node *>::iterator ita = nodes.begin(); ita != nodes.end(); ita++)
			change |= (*ita).second->applyForce(r);

//		for (std::map<BasicBlock *, Node *>::iterator itf = nodes.begin(); itf != nodes.end(); itf++)
//			(*itf).second->makePretty(r);

		if (change) {
			m_graph.ClientToScreen(&r);
			ScreenToClient(&r);
			InvalidateRect(&r);
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}


void CCFGViewDialog::OnLButtonDown(UINT nFlags, CPoint point) 
{
	POINT p = point;
	ClientToScreen(&p);
	m_graph.ScreenToClient(&p);
	for (std::map<BasicBlock *, Node *>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		CRect r;
		r.left = (*it).second->getPoint().x;
		r.top = (*it).second->getPoint().y;
		r.right = r.left + (*it).second->getSize().cx;
		r.bottom = r.top + (*it).second->getSize().cy;
		if (r.PtInRect(p)) {
			PBB bb = (*it).first;
			if (indicatingLatch) {
				PBB cursel = (PBB)m_tree.GetItemData(m_tree.GetSelectedItem());
				cursel->m_latchNode = bb;
				bb->m_loopHead = cursel;
				indicatingLatch = false;
				break;
			}
			if (indicatingFollow) {
				PBB cursel = (PBB)m_tree.GetItemData(m_tree.GetSelectedItem());
				if (cursel->m_structType == IFTHEN || cursel->m_structType == IFTHENELSE)
					cursel->m_condFollow = bb;
				else
					cursel->m_loopFollow = bb;
				indicatingFollow = false;
				break;
			}
			m_tree.SelectItem(items[bb]);			
			m_tree.EnsureVisible(items[bb]);
			m_tree.SetFocus();
			RECT r;
			m_graph.GetClientRect(&r);
			m_graph.ClientToScreen(&r);
			ScreenToClient(&r);
			InvalidateRect(&r);
			if (edit != NULL) {
				edit->graphicalSelect(bb);
			} else
				curdrag = (*it).second;
			break;			
		}
	}
	
	CDialog::OnLButtonDown(nFlags, point);
}

void CCFGViewDialog::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (curdrag) {
		POINT p = point;
		ClientToScreen(&p);
		m_graph.ScreenToClient(&p);
		CPoint &r = curdrag->getPoint();
		r.x = p.x;
		r.y = p.y;
		RECT v;
		m_graph.GetClientRect(&v);
		m_graph.ClientToScreen(&v);
		ScreenToClient(&v);
		InvalidateRect(&v);
	}
	
	CDialog::OnMouseMove(nFlags, point);

	if (indicatingLatch)
		::SetCursor(hIndicateLatch);
	else if (indicatingFollow)
		::SetCursor(hIndicateFollow);
}

void CCFGViewDialog::OnLButtonUp(UINT nFlags, CPoint point) 
{
	curdrag = NULL;

	CDialog::OnLButtonUp(nFlags, point);
}

void CCFGViewDialog::OnRclickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if (edit) {
		delete edit;
		edit = NULL;
	}
	edit = new CEditBBDialog(theApp.m_pMainWnd, cfg, (PBB)m_tree.GetItemData(m_tree.GetSelectedItem()));
	edit->DoModal();
	delete edit;
	edit = NULL;
	
	*pResult = 0;
}

void CCFGViewDialog::OnRButtonDown(UINT nFlags, CPoint point) 
{
	OnLButtonDown(nFlags, point);
	curdrag = NULL;
	PBB cursel = (PBB)m_tree.GetItemData(m_tree.GetSelectedItem());
	CMenu popup;
	popup.CreatePopupMenu();
	if (cursel) {
		if (cursel->getOutEdges().size() == 2) {
			if (cursel->m_structType != IFTHEN)
				popup.AppendMenu(0, ID_MAKE_IF_THEN, "Make if/then");
			if (cursel->m_structType != IFTHENELSE)
				popup.AppendMenu(0, ID_MAKE_IF_THEN_ELSE, "Make if/then/else");
			if (cursel->m_structType != PRETESTLOOP)
				popup.AppendMenu(0, ID_MAKE_PRETESTED_LOOP, "Make pretested loop");
		}
		if (cursel->m_structType != NONE)
			popup.AppendMenu(0, ID_CHANGE_FOLLOW, "Change follow node");
		if (cursel->m_structType == PRETESTLOOP ||
			cursel->m_structType == POSTTESTLOOP ||
			cursel->m_structType == ENDLESSLOOP)
			popup.AppendMenu(0, ID_CHANGE_LATCH, "Change latch node");
		popup.AppendMenu(MFT_SEPARATOR);
	}
	popup.AppendMenu(0, ID_EDIT_BB, "Edit selection");
	POINT p = point;
	ClientToScreen(&p);
	popup.TrackPopupMenu(0, p.x, p.y, this);	
	CDialog::OnRButtonDown(nFlags, point);
}

void CCFGViewDialog::OnDestroy() 
{
	CDialog::OnDestroy();

	if (doc)
		doc->UpdateAllViews(NULL);	
}

void CCFGViewDialog::OnEditBb() 
{
	if (edit) {
		delete edit;
		edit = NULL;
	}
	edit = new CEditBBDialog(theApp.m_pMainWnd, cfg, (PBB)m_tree.GetItemData(m_tree.GetSelectedItem()));
	edit->DoModal();
	delete edit;
	edit = NULL;	
}

void CCFGViewDialog::OnMakeIfThen() 
{
	PBB cursel = (PBB)m_tree.GetItemData(m_tree.GetSelectedItem());
	assert(cursel);
	cursel->m_structType = IFTHEN;
	indicateFollow();
}

void CCFGViewDialog::OnMakeIfThenElse() 
{
	PBB cursel = (PBB)m_tree.GetItemData(m_tree.GetSelectedItem());
	assert(cursel);
	cursel->m_structType = IFTHENELSE;
	indicateFollow();
}

void CCFGViewDialog::indicateLatch()
{	
	indicatingLatch = true;
	RECT v;
	m_graph.GetClientRect(&v);
	m_graph.ClientToScreen(&v);
	ScreenToClient(&v);
	InvalidateRect(&v);
}

void CCFGViewDialog::indicateFollow()
{	
	indicatingFollow = true;
	RECT v;
	m_graph.GetClientRect(&v);
	m_graph.ClientToScreen(&v);
	ScreenToClient(&v);
	InvalidateRect(&v);
}

void CCFGViewDialog::OnChangeFollow() 
{
	indicateFollow();	
}

void CCFGViewDialog::OnClickTree(NMHDR* pNMHDR, LRESULT* pResult) 
{
	RECT v;
	m_graph.GetClientRect(&v);
	m_graph.ClientToScreen(&v);
	ScreenToClient(&v);
	InvalidateRect(&v);	
	*pResult = 0;
}

void CCFGViewDialog::OnChangeLatch() 
{
	indicateLatch();	
}

void CCFGViewDialog::setLevels() 
{
	std::vector<std::vector<Node*> > levels;
	int level = 0;
	levels.resize(1);
	levels[0].push_back(nodes[cfg->getEntryBB()]);
	cfg->unTraverse();
	while (levels[level].size() != 0) {
		for (int i = 0; i < levels[level].size(); i++) {
			Node *n = levels[level][i];
			n->setLevel(level);			
			n->setTraversed(true);
			levels.resize(level+2);
			for (int j = 0; j < n->getNumOutEdges(); j++) 
				if (!n->getOutEdge(j)->isTraversed()) {
					levels[level+1].push_back(n->getOutEdge(j));
				}
		}
		level++;
	}
	RECT r;
	m_graph.GetClientRect(&r);
	int totalh = r.bottom - r.top;
	int totalw = r.right - r.left;
	// now arrange the nodes on their levels
	for (int i = 0; i < level; i++) {
		for (int j = 0; j < levels[i].size(); j++) {
			CPoint &n = levels[i][j]->getPoint();
			n.y = i * totalh / level;
			n.x = j * totalw / levels[i].size();
		}
	}
}

void CCFGViewDialog::compressNodes()
{
	// look for any node with a single out edge
	// if the successor has only 1 in edge then
	// the two can be merged into a multinode
	bool changed = true;
	while (changed) {
		changed = false;
		for (std::map<BasicBlock *, Node *>::iterator it = nodes.begin(); it != nodes.end(); it++) {
			BasicBlock *bb = (*it).first;
			Node *n = (*it).second;
			if (n->getNumOutEdges() == 1 && n->getOutEdge(0)->getNumInEdges() == 1) {
				n->mergeWith(n->getOutEdge(0));
				changed = true;
				break;
			}
		}
	}
}

void CCFGViewDialog::OnMakePretestedLoop() 
{
	PBB cursel = (PBB)m_tree.GetItemData(m_tree.GetSelectedItem());
	assert(cursel);
	cursel->m_structType = PRETESTLOOP;
	indicateFollow();
	indicateLatch();
}

