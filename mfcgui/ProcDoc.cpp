// ProcDoc.cpp : implementation of the CProcDoc class
//

#include "stdafx.h"
#include "mfcgui.h"

#if defined(_MSC_VER) && _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif

#include "prog.h"
#include "util.h"
#include <assert.h>

#include "ProcDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcDoc

IMPLEMENT_DYNCREATE(CProcDoc, CDocument)

BEGIN_MESSAGE_MAP(CProcDoc, CDocument)
	//{{AFX_MSG_MAP(CProcDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcDoc construction/destruction

CProcDoc::CProcDoc() : m_proc(NULL)
{
	// TODO: add one-time construction code here
}

CProcDoc::~CProcDoc()
{
}

BOOL CProcDoc::OnNewDocument()
{
	return FALSE;

	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CProcDoc serialization

void CProcDoc::Serialize(CArchive& ar)
{
	// CEditView contains an edit control which handles all serialization
	((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CProcDoc diagnostics

#ifdef _DEBUG
void CProcDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CProcDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CProcDoc commands

BOOL CProcDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	return TRUE; // we dont actually open procs

	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	
	return TRUE;
}

void CProcDoc::setProc(Proc *p)
{
	m_proc = p;
}