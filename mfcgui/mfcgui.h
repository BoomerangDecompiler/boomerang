// mfcgui.h : main header file for the MFCGUI application
//

#if !defined(AFX_MFCGUI_H__9AD87D93_F6FA_436A_8A1B_1FB73FA2A220__INCLUDED_)
#define AFX_MFCGUI_H__9AD87D93_F6FA_436A_8A1B_1FB73FA2A220__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CBoomerangApp:
// See mfcgui.cpp for the implementation of this class
//

class CBoomerangApp : public CWinApp
{
public:
	CBoomerangApp();

	char path[1024];

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBoomerangApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CBoomerangApp)
	afx_msg void OnAppAbout();
	afx_msg void OnViewProcedures();
	afx_msg void OnViewControlflow();
	afx_msg void OnViewSymbols();
	afx_msg void OnProjectNew();
	afx_msg void OnProjectOpen();
	afx_msg void OnProjectClose();
	afx_msg void OnProjectSave();
	afx_msg void OnProjectSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CBoomerangApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MFCGUI_H__9AD87D93_F6FA_436A_8A1B_1FB73FA2A220__INCLUDED_)
