// windows.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <sstream>
#include "resource.h"
#undef NO_ADDRESS
#include "prog.h"
#include "signature.h"
#include "statement.h"
#include "boomerang.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND hTopWnd, hStatusBar, hTreeView, hLogView;
HBRUSH yellowBrush, buttonBrush;
HFONT hTabBarFont, hMemoryDumpFont;
static Prog *prog = NULL;
std::map<Proc*,HTREEITEM> procItems;
HTREEITEM treeDragging = NULL;
HMENU clusterMenu = NULL, procMenu = NULL, callersMenu = NULL, callsMenu = NULL;
bool someUnknown = false;

enum { V_CODE, V_RTL, V_MIXED } view_as = V_RTL;

struct my_tab {
	const char *name;
	HWND edit;
	Cluster *cluster;
};
std::map<std::string, my_tab> tabWithName;
char *selectedTab = NULL;
RECT tabBarRect;
int iCluster, iProc, iDProc, iLProc, iULProc;


class MyLParam {
public:
	Proc *p;
	Cluster *c;
	int isCluster;
	MyLParam(Cluster *c) : c(c) { isCluster = 1; }
	MyLParam(Proc *p) : p(p) { isCluster = 0; }
};

HANDLE hDecompilerThread = NULL;

void updateDecompilerMenu()
{
	if (hDecompilerThread == NULL) {
		EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_SUSPEND, MF_GRAYED);
		EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_RESUME, MF_GRAYED);
		EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_TERMINATE, MF_GRAYED);
		EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_START, MF_ENABLED);
		SendMessage(hStatusBar, SB_SETICON, 2, NULL);
	} else {
		EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_SUSPEND, MF_ENABLED);		
		EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_TERMINATE, MF_ENABLED);
		EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_START, MF_GRAYED);
		SendMessage(hStatusBar, SB_SETICON, 2, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_RESUME)));
	}
}

void updateAllTabs()
{
	for (std::map<std::string, my_tab>::iterator it = tabWithName.begin(); it != tabWithName.end(); )
		if ((*it).first != (*it).second.cluster->getName()) {
			struct my_tab t = (*it).second;
			if ((*it).first == selectedTab)
				selectedTab = strdup(t.cluster->getName());
			tabWithName.erase(it);
			t.name = strdup(t.cluster->getName());
			tabWithName[t.cluster->getName()] = t;
			it = tabWithName.begin();
		} else 
			it++;
	InvalidateRect(hTopWnd, &tabBarRect, TRUE);
}

void updateTreeView()
{
	HTREEITEM h = TreeView_GetRoot(hTreeView);
	while (h) {
		TVITEM i;
		i.hItem = h;
		i.mask = TVIF_TEXT | TVIF_HANDLE | TVIF_PARAM;
		char buf[1024];
		i.pszText = buf;
		i.cchTextMax = sizeof(buf);
		TreeView_GetItem(hTreeView, &i);
		if (strcmp(i.pszText, ((MyLParam*)i.lParam)->c->getName())) {
			i.mask = TVIF_TEXT;
			strcpy(buf, ((MyLParam*)i.lParam)->c->getName());
			TreeView_SetItem(hTreeView, &i);
		}
		h = TreeView_GetNextSibling(hTreeView, h);
	}
	for (std::map<Proc*,HTREEITEM>::iterator it = procItems.begin(); it != procItems.end(); it++) {
		TVITEM i;
		i.hItem = (*it).second;
		i.mask = TVIF_TEXT;
		i.pszText = strdup((*it).first->getName());
		TreeView_SetItem(hTreeView, &i);
	}
}

void suspendDecompiler()
{
	EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_SUSPEND, MF_GRAYED);
	EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_RESUME, MF_ENABLED);
	SendMessage(hStatusBar, SB_SETICON, 2, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_SUSPEND)));
	SuspendThread(hDecompilerThread);
}

void resumeDecompiler()
{
	EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_SUSPEND, MF_ENABLED);
	EnableMenuItem(GetMenu(hTopWnd), ID_DECOMPILER_RESUME, MF_GRAYED);
	SendMessage(hStatusBar, SB_SETICON, 2, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_RESUME)));
	ResumeThread(hDecompilerThread);
}

void generateCodeForCluster(Cluster *c);
void generateCodeForUserProc(UserProc *p);

void updateCodeView()
{
	HTREEITEM h = TreeView_GetSelection(hTreeView);
	if (h == NULL)
		return;
	TVITEM i;
	i.hItem = h;
	i.mask = TVIF_PARAM;
	TreeView_GetItem(hTreeView, &i);
	MyLParam *lp = (MyLParam*)i.lParam;
	if (lp->isCluster) {
		generateCodeForCluster(lp->c);
		if (lp->c->getNumChildren() == 0) {
			PROGMAP::const_iterator it;
			bool found = false;
			for (Proc *proc = prog->getFirstProc(it); proc; proc = prog->getNextProc(it))
				if (proc->getCluster() == lp->c) {
					found = true;
					break;
				}
			if (!found) {
				EnableMenuItem(GetMenu(hTopWnd), ID_VIEW_DELETECLUSTER, MF_ENABLED);
				if (clusterMenu)
					EnableMenuItem(clusterMenu, ID_VIEW_DELETECLUSTER, MF_ENABLED);
			}
		}
	} else {
		UserProc *u = dynamic_cast<UserProc*>(lp->p);
		if (u)
			generateCodeForUserProc(u);
	}
}

void saveUndoPoint()
{
	return;  // Possibly Mike broke it
	if (prog) {
		prog->takeMemo();
		if (prog->canRestore())
			EnableMenuItem(GetMenu(hTopWnd), ID_EDIT_UNDO, MF_ENABLED);
		else
			EnableMenuItem(GetMenu(hTopWnd), ID_EDIT_UNDO, MF_GRAYED);
		EnableMenuItem(GetMenu(hTopWnd), ID_EDIT_REDO, MF_GRAYED);
	}
}

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	StatusBar(HWND, UINT, WPARAM, LPARAM);
WNDPROC oldStatusBarProc = NULL;
LRESULT CALLBACK	TreeView(HWND, UINT, WPARAM, LPARAM);
WNDPROC oldTreeViewProc = NULL;
LRESULT CALLBACK	TabEdit(HWND, UINT, WPARAM, LPARAM);
WNDPROC oldTabEditProc = NULL;
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	NewProject(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	DebugOptions(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	DecodeOptions(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	DecompileOptions(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	Decoding(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	ProcProperties(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	SymbolTable(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	MemoryDump(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPTSTR	   lpCmdLine,
					 int	   nCmdShow)
{
	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	char filename[MAX_PATH];
	GetCurrentDirectory(sizeof(filename), filename);
	strcat(filename, "\\");
	Boomerang::get()->setProgPath(filename);
	strcat(filename, "output");
	Boomerang::get()->setOutputPath(filename);

	InitCommonControls();

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_BOOMERANG, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_BOOMERANG);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

#define LOG_SIZE 1024*92
char log[LOG_SIZE], *plog = log;

class WindowLogger : public Log {
public:
	WindowLogger() : Log() { }
	virtual Log &operator<<(const char *pstr) {
		if (pstr == NULL)
			return *this;
		char *str = new char[strlen(pstr) + 1024], *p2 = str;
		for (const char *p1 = pstr; *p1; p1++)
			if (*p1 != '\n')
				*p2++ = *p1;
			else {
				*p2++ = '\r';
				*p2++ = '\n';
			}
		*p2 = 0;

		if (strlen(str) + plog - log < LOG_SIZE) {
			strcpy(plog, str);
			plog += strlen(str);
		} else {
			memmove(log, log + 1024, LOG_SIZE - 1024);
			plog -= 1024;
			if (strlen(str) + plog - log < LOG_SIZE) {
				strcpy(plog, str);
				plog += strlen(str);
			}
		}
		SetWindowText(hLogView, log);
		SendMessage(hLogView, WM_VSCROLL, SB_BOTTOM, 0);
		return *this;
	}
	virtual ~WindowLogger() {};
};

void OpenProject();
void SaveProject();
void CloseProject();

//
//	FUNCTION: MyRegisterClass()
//
//	PURPOSE: Registers the window class.
//
//	COMMENTS:
//
//	  This function and its usage are only necessary if you want this code
//	  to be compatible with Win32 systems prior to the 'RegisterClassEx'
//	  function that was added to Windows 95. It is important to call this function
//	  so that the application will get 'well formed' small icons associated
//	  with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_BOOMERANG);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_BOOMERANG;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_BOOMERANG);

	return RegisterClassEx(&wcex);
}

void addNewTab(Cluster *cluster);

HFONT MyCreateFont( void ) 
{ 
	CHOOSEFONT cf; 
	LOGFONT lf; 
	HFONT hfont; 
 
	// Initialize members of the CHOOSEFONT structure. 
 
	cf.lStructSize = sizeof(CHOOSEFONT); 
	cf.hwndOwner = (HWND)NULL; 
	cf.hDC = (HDC)NULL; 
	cf.lpLogFont = &lf; 
	cf.iPointSize = 0; 
	cf.Flags = CF_SCREENFONTS; 
	cf.rgbColors = RGB(0,0,0); 
	cf.lCustData = 0L; 
	cf.lpfnHook = (LPCFHOOKPROC)NULL; 
	cf.lpTemplateName = (LPSTR)NULL; 
	cf.hInstance = (HINSTANCE) NULL; 
	cf.lpszStyle = (LPSTR)NULL; 
	cf.nFontType = SCREEN_FONTTYPE; 
	cf.nSizeMin = 0; 
	cf.nSizeMax = 0; 
 
	// Display the CHOOSEFONT common-dialog box. 
 
	ChooseFont(&cf); 
 
	// Create a logical font based on the user's 
	// selection and return a handle identifying 
	// that font. 
 
	OpenClipboard(hTopWnd);
	EmptyClipboard(); 
	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, 8192);
	char *buf = (char*)GlobalLock(hglbCopy); 
	buf[0] = 0;
	for (unsigned i = 0; i < sizeof(lf); i++)
		sprintf(buf + strlen(buf), "0x%02X, ", ((unsigned char*)&lf)[i]);
	GlobalUnlock(hglbCopy); 
	SetClipboardData(CF_TEXT, hglbCopy); 
	CloseClipboard();
	hfont = CreateFontIndirect(cf.lpLogFont);

	return (hfont); 
} 

void setupDecompiler();

//
//	 FUNCTION: InitInstance(HANDLE, int)
//
//	 PURPOSE: Saves instance handle and creates main window
//
//	 COMMENTS:
//
//		  In this function, we save the instance handle in a global variable and
//		  create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	hTopWnd = hWnd;

	hStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0, hWnd, NULL, hInstance, NULL);

	oldStatusBarProc = (WNDPROC)SetWindowLongPtr (hStatusBar, GWLP_WNDPROC, (LONG_PTR)StatusBar);
	SetTimer(hStatusBar, 1, 1000, NULL);

	RECT r, rsb;
	GetClientRect(hWnd, &r);
	GetClientRect(hStatusBar, &rsb);

	int parts[] = { rsb.right / 2, rsb.right / 2 + 200, rsb.right / 2 + 232, -1 };
	SendMessage(hStatusBar, SB_SETPARTS, 4, (LPARAM) parts);

	hTreeView = CreateWindowEx(0, WC_TREEVIEW, NULL, WS_CHILD | WS_DLGFRAME | WS_VISIBLE | TVS_EDITLABELS | TVS_HASLINES | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_SHOWSELALWAYS, r.right - 200, 0, 200, r.bottom - rsb.bottom, hWnd, NULL, hInstance, NULL);

	oldTreeViewProc = (WNDPROC)SetWindowLongPtr (hTreeView, GWLP_WNDPROC, (LONG_PTR)TreeView);
	HIMAGELIST himglist = ImageList_Create(16, 16, ILC_COLOR4, 5, 1);
	iCluster = ImageList_AddIcon(himglist, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLUSTER)));
	iProc = ImageList_AddIcon(himglist, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROC)));
	iDProc = ImageList_AddIcon(himglist, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DPROC)));
	iLProc = ImageList_AddIcon(himglist, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LPROC)));
	iULProc = ImageList_AddIcon(himglist, LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ULPROC)));
	TreeView_SetImageList(hTreeView, himglist, TVSIL_NORMAL);

	yellowBrush = CreateSolidBrush(RGB(255, 255, 200));
	buttonBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

	unsigned char lf_bits[] = { 0xF3, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x01, 0x22, 0x41, 0x72, 0x69, 0x61, 0x6C, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, };
	hTabBarFont = CreateFontIndirect((LOGFONT*)lf_bits);
	LOGFONT mmfont;
	memcpy(&mmfont, lf_bits, sizeof(mmfont));
	strcpy(mmfont.lfFaceName, "OEM fixed font");
	mmfont.lfPitchAndFamily &= ~3;
	mmfont.lfPitchAndFamily |= FIXED_PITCH;
	hMemoryDumpFont = CreateFontIndirect(&mmfont);

	hLogView = CreateWindowEx(0, WC_EDIT, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL	| WS_DLGFRAME
		| ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY, 
		0, r.bottom - rsb.bottom - 200, r.right - 200, 200, hTopWnd, NULL, hInst, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
  
	setupDecompiler();
	return TRUE;
}

void selectTab(const char *name)
{
	if (selectedTab) {
		if (!strcmp(selectedTab, name))
			return;
		ShowWindowAsync(tabWithName[selectedTab].edit, SW_HIDE);
	}
	selectedTab = strdup(name);
	ShowWindowAsync(tabWithName[selectedTab].edit, SW_SHOW);
	InvalidateRect(hTopWnd, &tabBarRect, TRUE);
}

void updateTab(const char *name)
{
	Cluster *c = tabWithName[name].cluster;
	const char *path = c->getOutPath((view_as == V_CODE || view_as == V_MIXED) ? "c" : "rtl");
	std::ifstream in(path, std::ios_base::binary);
	if (in.is_open()) {
		in.seekg(0, std::ios_base::end);
		int n = in.tellg();
		in.seekg(0, std::ios_base::beg);
		char *buf = new char[n];
		in.read(buf, n);
		in.close();
		SendMessage(tabWithName[c->getName()].edit, WM_SETTEXT, 0, (LPARAM)buf);
	}
}

void generateCodeForClusterThread(Cluster *c)
{
	char buf[1024];
	sprintf(buf, "Generating code for cluster %s", c->getName());
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)buf);
	if (view_as == V_CODE)
		prog->generateCode(c);
	else if (view_as == V_RTL)
		prog->generateRTL(c);
	else if (view_as == V_MIXED)
		prog->generateCode(c, NULL, true);
	selectTab(c->getName());
	updateTab(c->getName());
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Done");
	ExitThread(0);
}

void generateCodeForCluster(Cluster *c)
{
	addNewTab(c);
	DWORD id;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)generateCodeForClusterThread, (LPVOID)c, 0, &id);
}

void generateCodeForAllThread(void *n)
{
	char buf[1024];
	sprintf(buf, "Generating code...");
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)buf);
	prog->generateCode();
	if (selectedTab)
		updateTab(selectedTab);
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Done");
	ExitThread(0);
}

void generateCodeForAll()
{
	DWORD id;
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)generateCodeForAllThread, 0, 0, &id);
}

void generateCodeForUserProcThread(UserProc *proc)
{
	if (prog == NULL)
		return;
	char buf[1024];
	sprintf(buf, "Generating code for proc %s", proc->getName());
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)buf);
	if (view_as == V_CODE)
		prog->generateCode(proc->getCluster(), proc);
	else if (view_as == V_RTL)
		prog->generateRTL(proc->getCluster(), proc);
	else if (view_as == V_MIXED)
		prog->generateCode(proc->getCluster(), proc, true);
	selectTab(proc->getCluster()->getName());
	updateTab(proc->getCluster()->getName());
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Done");
	ExitThread(0);
}

void generateCodeForUserProc(UserProc *proc)
{
	addNewTab(proc->getCluster());
	DWORD id;
	HANDLE h = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)generateCodeForUserProcThread, (LPVOID)proc, 0, &id);
}

void closeTab(const char *name)
{
	for (std::map<std::string, struct my_tab>::iterator it = tabWithName.begin(); it != tabWithName.end(); it++) 
		if ((*it).first == name) {
			DestroyWindow((*it).second.edit);
			if (!strcmp(selectedTab, name))
				selectedTab = NULL;
			tabWithName.erase(it);
			break;
		}
	InvalidateRect(hTopWnd, &tabBarRect, TRUE);
}

void closeAllTabs()
{
	for (std::map<std::string, struct my_tab>::iterator it = tabWithName.begin(); it != tabWithName.end(); it++) {
		DestroyWindow((*it).second.edit);
	}
	tabWithName.clear();
	selectedTab = NULL;
	InvalidateRect(hTopWnd, &tabBarRect, TRUE);
}

void addNewTab(Cluster *cluster)
{
	const char *name = cluster->getName();
	if (tabWithName.find(name) != tabWithName.end())
		return;

	struct my_tab tab;
	tab.name = strdup(name);
	tab.cluster = cluster;
	RECT r, rsb;
	GetClientRect(hTopWnd, &r);
	GetClientRect(hStatusBar, &rsb);
	
	tab.edit = CreateWindowEx(0, WC_EDIT, NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL	| WS_DLGFRAME
		| ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 
		0, tabBarRect.bottom, r.right - 200, r.bottom - rsb.bottom - tabBarRect.bottom - 200, hTopWnd, NULL, hInst, NULL);
	oldTabEditProc = (WNDPROC)SetWindowLongPtr (tab.edit, GWLP_WNDPROC, (LONG_PTR)TabEdit);
	tabWithName[name] = tab;
	if (selectedTab == NULL)
		selectTab(name);
	InvalidateRect(hTopWnd, &tabBarRect, TRUE);
}

void drawTabBar(HDC hdc)
{
	RECT r = tabBarRect;
	FillRect(hdc, &r, yellowBrush);
	r.left += 4;
	for (std::map<std::string, struct my_tab>::iterator it = tabWithName.begin(); it != tabWithName.end(); it++) {
		const char *name = (*it).second.name;
		DrawText(hdc, name, -1, &r, DT_CALCRECT | DT_LEFT | DT_TOP);
		r.bottom++;
		r.right += 4;
		if (selectedTab && !strcmp(name, selectedTab)) {
			r.left -= 4;
			r.top++;
			FillRect(hdc, &r, buttonBrush);
			r.left += 4;
			r.top--;
			SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
			DrawText(hdc, name, -1, &r, DT_LEFT | DT_TOP);
			MoveToEx(hdc, r.right, r.top + 1, NULL);
			LineTo(hdc, r.right, r.bottom - 1);
		} else {
			SetBkColor(hdc, RGB(255, 255, 200));
			DrawText(hdc, name, -1, &r, DT_LEFT | DT_TOP);
			MoveToEx(hdc, r.right, r.top + 2, NULL);
			LineTo(hdc, r.right, r.bottom - 3);
		}
		r.left += r.right - r.left + 4;
		r.right = r.left + 10;
	}
}

void handleTabBarPress(int xPos)
{
	HDC hdc = GetDC(hTopWnd);
	RECT r = tabBarRect;
	r.left += 4;

	for (std::map<std::string, struct my_tab>::iterator it = tabWithName.begin(); it != tabWithName.end(); it++) {
		const char *name = (*it).second.name;
		DrawText(hdc, name, -1, &r, DT_CALCRECT | DT_LEFT | DT_TOP);
		r.bottom++;
		r.right += 4;
		if (xPos >= r.left && xPos < r.right) {
			selectTab(name);
			break;
		}
		r.left += r.right - r.left + 4;
		r.right = r.left + 10;
	}
	ReleaseDC(hTopWnd, hdc);
}

Proc *selectedProc()
{
	HTREEITEM h = TreeView_GetSelection(hTreeView);
	TVITEM it;
	it.hItem = h;
	it.mask = TVIF_HANDLE;
	if (!TreeView_GetItem(hTreeView, &it))
		return NULL;
	MyLParam *lp = (MyLParam*)it.lParam;
	if (lp->isCluster)
		return NULL;
	return lp->p;
}

LRESULT CALLBACK StatusBar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
	case WM_COMMAND:
		break;
	case WM_LBUTTONUP:
		{
			int parts[5];
			SendMessage(hWnd, SB_GETPARTS, 5, (LPARAM)parts);
			if (LOWORD(lParam) > parts[1] && LOWORD(lParam) < parts[2]) {
				if ((HICON)SendMessage(hWnd, SB_GETICON, 2, 0) == LoadIcon(hInst, MAKEINTRESOURCE(IDI_RESUME)) ||
					(HICON)SendMessage(hWnd, SB_GETICON, 2, 0) == LoadIcon(hInst, MAKEINTRESOURCE(IDI_RESUME1))) {
					SendMessage(hWnd, SB_SETICON, 2, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_SUSPEND)));
					suspendDecompiler();
				} else {
					SendMessage(hWnd, SB_SETICON, 2, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_RESUME)));
					resumeDecompiler();
				}
			}
		}
		break;
	case WM_TIMER:
		if ((HICON)SendMessage(hWnd, SB_GETICON, 2, 0) == LoadIcon(hInst, MAKEINTRESOURCE(IDI_RESUME))) {
			SendMessage(hWnd, SB_SETICON, 2, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_RESUME1)));
		} else if ((HICON)SendMessage(hWnd, SB_GETICON, 2, 0) == LoadIcon(hInst, MAKEINTRESOURCE(IDI_RESUME1))) {
			SendMessage(hWnd, SB_SETICON, 2, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_RESUME)));
		}
		break;
	case WM_RBUTTONDOWN:
		break;
	case WM_MOUSEMOVE:
		break;
	}
	return CallWindowProc(oldStatusBarProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK TreeView(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	TVHITTESTINFO tvhti;
	TVITEM tvi;
	MyLParam *lp;

	switch (uMsg) 
	{
	case WM_LBUTTONUP:
		if (treeDragging) {
			TreeView_SetInsertMark(hTreeView, NULL, TRUE);
			TVHITTESTINFO tvh;
			tvh.pt.x = LOWORD(lParam);
			tvh.pt.y = HIWORD(lParam);
			TreeView_HitTest(hTreeView, &tvh);
			if (tvh.hItem && tvh.hItem != treeDragging) {
				TVINSERTSTRUCT tvi;
				tvi.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvi.item.hItem = treeDragging;
				char buf[1024];
				tvi.item.pszText = buf;
				tvi.item.cchTextMax = sizeof(buf);
				TreeView_GetItem(hTreeView, &tvi.item);
				TVITEM tv;
				tv.mask = TVIF_PARAM;
				tv.hItem = tvh.hItem;
				TreeView_GetItem(hTreeView, &tv);
				MyLParam *tp = (MyLParam*)tv.lParam;
				if (tp->isCluster) {
					tvi.hParent = tvh.hItem;
					tvi.hInsertAfter = TVI_FIRST;
				} else {
					tvi.hParent = TreeView_GetParent(hTreeView, tvh.hItem);
					tvi.hInsertAfter = tvh.hItem;
				}
				MyLParam *p = (MyLParam*)tvi.item.lParam;
				if (!p->isCluster || tp->isCluster) {
					HTREEITEM h = TreeView_InsertItem(hTreeView, &tvi);
					TreeView_DeleteItem(hTreeView, treeDragging);
					if (p->isCluster) {
						p->c->getParent()->removeChild(p->c);
						tp->c->addChild(p->c);
					} else {
						procItems[p->p] = h;
						if (tp->isCluster) {
							p->p->setCluster(tp->c);
						} else {
							p->p->setCluster(tp->p->getCluster());
						}
					}
				}
			}
			treeDragging = NULL;
		}
		break;
	case WM_RBUTTONDOWN:
		tvhti.pt.x = LOWORD(lParam);
		tvhti.pt.y = HIWORD(lParam);
		TreeView_HitTest(hTreeView, &tvhti);
		tvi.hItem = tvhti.hItem;
		tvi.mask = TVIF_PARAM;
		TreeView_GetItem(hTreeView, &tvi);
		GetWindowRect(hWnd, &r);
		lp = (MyLParam*)tvi.lParam;
		if (lp->isCluster) {
			if (clusterMenu == NULL) {
				clusterMenu = CreatePopupMenu();
				AppendMenu(clusterMenu, MF_STRING, ID_VIEW_RENAME, "&Rename");
				AppendMenu(clusterMenu, MF_STRING, ID_VIEW_NEWCLUSTER, "&New Cluster");
				AppendMenu(clusterMenu, MF_STRING | MF_GRAYED, ID_VIEW_DELETECLUSTER, "&Delete Cluster");
			}
			TreeView_SelectItem(hTreeView, tvhti.hItem);
			TrackPopupMenu(clusterMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, r.left + LOWORD(lParam), r.top + HIWORD(lParam), 0, hTopWnd, NULL);
		} else {
			UserProc *u = dynamic_cast<UserProc*>(lp->p);
			if (procMenu != NULL) {
				DestroyMenu(procMenu);
			}
			callersMenu = CreatePopupMenu();
			callsMenu = CreatePopupMenu();
			procMenu = CreatePopupMenu();
			AppendMenu(procMenu, MF_STRING, ID_VIEW_RENAME, "&Rename");
			AppendMenu(procMenu, MF_SEPARATOR, 0, 0);
			AppendMenu(procMenu, MF_STRING | MF_POPUP, (UINT_PTR)callersMenu, "&Callers");
			AppendMenu(procMenu, MF_STRING | MF_POPUP, (UINT_PTR)callsMenu, "C&alls");				
			AppendMenu(procMenu, MF_STRING, ID_VIEW_PROPERTIES, "&Properties");
			while (DeleteMenu(callersMenu, 0, MF_BYPOSITION))
				;
			std::set<CallStatement*> &callers = lp->p->getCallers();
			int n = 33001;
			for (std::set<CallStatement*>::iterator it = callers.begin(); it != callers.end(); it++, n++)
				AppendMenu(callersMenu, MF_STRING, n, (*it)->getProc()->getName());
			while (DeleteMenu(callsMenu, 0, MF_BYPOSITION))
				;
			if (u) {
				std::list<Proc*> &calls = u->getCallees();
				n = 34001;
				for (std::list<Proc*>::iterator it = calls.begin(); it != calls.end(); it++, n++)
					AppendMenu(callsMenu, MF_STRING, n, (*it)->getName());
			}
			TreeView_Select(hTreeView, tvhti.hItem, TVGN_CARET);
			TrackPopupMenu(procMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, r.left + LOWORD(lParam), r.top + HIWORD(lParam), 0, hTopWnd, NULL);
		}
		break;
	case WM_MOUSEMOVE:
		if (treeDragging) {
			TVHITTESTINFO tvh;
			tvh.pt.x = LOWORD(lParam);
			tvh.pt.y = HIWORD(lParam);
			TreeView_HitTest(hTreeView, &tvh);
			if (tvh.hItem)
				TreeView_SetInsertMark(hTreeView, tvh.hItem, TRUE);
			else
				TreeView_SetInsertMark(hTreeView, NULL, TRUE);
		}
		break;
	default:
		return CallWindowProc(oldTreeViewProc, hWnd, uMsg, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK TabEdit(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static Statement *stmt;
	static Exp *exp;

	//std::map<std::string, my_tab>::iterator it;
	//for (it = tabWithName.begin(); it != tabWithName.end(); it++)
	//	if ((*it).second.edit == hWnd)
	//		break;

	switch (uMsg) 
	{
	case WM_COMMAND:
		switch(LOWORD(wParam)) {
			case ID_STMT_PROPAGATE_TO:
				StatementSet exclude;
				stmt->propagateTo(-1, exclude);
				stmt->getProc()->updateBlockVars();
				updateCodeView();
				break;
		}
		break;
	case WM_LBUTTONUP:
		break;
	case WM_RBUTTONDOWN:
		{
			DWORD d = SendMessage(hWnd, EM_CHARFROMPOS, 0, lParam);
			int ch = LOWORD(d);
			int line = HIWORD(d);
			Proc *p = selectedProc();
			if (p == NULL)
				break;  // don't currently support right click in cluster view
			UserProc *u = dynamic_cast<UserProc*>(p);
			if (u == NULL)
				break;
			stmt = u->getStmtAtLex(ch, -1);
			if (stmt == NULL)
				break;
			exp = stmt->getExpAtLex(ch, -1);
			HMENU editMenu = CreatePopupMenu();
			AppendMenu(editMenu, MF_STRING, ID_STMT_PROPAGATE_TO, "&Propagate to this statement");
			RECT r;
			GetWindowRect(hWnd, &r);
			TrackPopupMenu(editMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON, r.left + LOWORD(lParam), r.top + HIWORD(lParam), 0, hWnd, NULL);
			DestroyMenu(editMenu);
		}
		break;
	case WM_MOUSEMOVE:
		break;
	}
	return CallWindowProc(oldTabEditProc, hWnd, uMsg, wParam, lParam);
}

//
//	FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//	PURPOSE:  Processes messages for the main window.
//
//	WM_COMMAND	- process the application menu
//	WM_PAINT	- Paint the main window
//	WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	TEXTMETRIC tm;
	LPNMTREEVIEW pnmtv;
	LPNMTVDISPINFO ptvdi;

	switch (message) 
	{
	case WM_SIZE:
		{
			RECT r;
			RECT rsb;
			GetClientRect(hTopWnd, &r);
			GetClientRect(hStatusBar, &rsb);
			SetWindowPos(hTreeView, 0, r.right - 200, 0, 200, r.bottom - rsb.bottom, SWP_NOZORDER);
			SetWindowPos(hStatusBar, 0, 0, r.bottom - rsb.bottom, r.right, rsb.bottom, SWP_NOZORDER);
			SetWindowPos(hLogView, 0, 0, r.bottom - rsb.bottom - 200, r.right - 200, 200, SWP_NOZORDER);
			for (std::map<std::string, my_tab>::iterator it = tabWithName.begin(); it != tabWithName.end(); it++) {
				SetWindowPos((*it).second.edit, 0, 0, tabBarRect.bottom, r.right - 200, r.bottom - rsb.bottom - tabBarRect.bottom - 200, SWP_NOZORDER);
			}
		}
		break;
	case WM_COMMAND:
		wmId	= LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		if (selectedTab && wmEvent == EN_CHANGE && (HWND)lParam == tabWithName[selectedTab].edit) {
			updateTab(selectedTab);
		}
		if (wmId > 33000) {
			char buf[1024];
			MENUITEMINFO mi;
			mi.cbSize = sizeof(mi);
			mi.fMask = MIIM_STRING;
			mi.cch = sizeof(buf);
			mi.dwTypeData = buf;
			if (GetMenuItemInfo(procMenu, wmId, FALSE, &mi)) {
				Proc *p = prog->findProc(buf);
				if (p && procItems.find(p) != procItems.end())
					TreeView_SelectItem(hTreeView, procItems[p]);
			}
		}
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_NEW_PROJECT:
			if (DialogBox(hInst, (LPCTSTR)IDD_NEWPROJECT, hWnd, (DLGPROC)NewProject) == IDOK)
				if (DialogBox(hInst, (LPCTSTR)IDD_DECODING, hWnd, (DLGPROC)Decoding) == IDOK)
					;
			break;
		case ID_DECODER_SHOWCOVERAGE:
			DialogBox(hInst, (LPCTSTR)IDD_DECODING, hWnd, (DLGPROC)Decoding);
			break;
		case ID_OPEN_PROJECT:
			OpenProject();
			break;
		case ID_FILE_SAVE:
			SaveProject();
			break;
		case ID_FILE_CLOSE:
			CloseProject();
			break;
		case ID_HELP_BOOMERANGWEBSITE:
			ShellExecute(NULL, "open", "http://boomerang.sourceforge.net/", NULL, NULL, SW_SHOW);
			break;
		case ID_HELP_BOOMERANGFAQ:
			ShellExecute(NULL, "open", "http://boomerang.sourceforge.net/FAQ.html", NULL, NULL, SW_SHOW);
			break;
		case ID_LOADER_SYMBOLTABLE:
			DialogBox(hInst, (LPCTSTR)IDD_SYMBOLTABLE, hWnd, (DLGPROC)SymbolTable);
			break;
		case ID_LOADER_MEMORYDUMP:
			DialogBox(hInst, (LPCTSTR)IDD_MEMORYDUMP, hWnd, (DLGPROC)MemoryDump);
			break;
		case ID_DECOMPILER_START:
			if (hDecompilerThread == NULL) {
				saveUndoPoint();
				prog->decompile();
			}
			break;
		case ID_DECOMPILER_SUSPEND:
			if (hDecompilerThread) {
				suspendDecompiler();
			}
			break;
		case ID_DECOMPILER_RESUME:
			if (hDecompilerThread) {
				resumeDecompiler();
			}
			break;
		case ID_DECOMPILER_TERMINATE:
			if (hDecompilerThread) {
				TerminateThread(hDecompilerThread, 0);
				hDecompilerThread = NULL;
				updateDecompilerMenu();
			}
			break;
		case ID_SETOUTPUTPATH:
			{
				OPENFILENAME ofn;
				memset(&ofn, 0, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				char buf[MAX_PATH];
				buf[0] = 0;
				ofn.lpstrFile = buf;
				ofn.nMaxFile = MAX_PATH;
				if (GetOpenFileName(&ofn)) {
					Boomerang::get()->setOutputDirectory(buf);
				}
			}
			break;
		case ID_VIEW_CODE:
			if (view_as != V_CODE) {
				view_as = V_CODE;
				updateCodeView();
			}
			break;
		case ID_VIEW_RTL:
			if (view_as != V_RTL) {
				view_as = V_RTL;
				updateCodeView();
			}
			break;
		case ID_VIEW_MIXED:
			if (view_as != V_MIXED) {
				view_as = V_MIXED;
				updateCodeView();
			}
			break;
		case ID_VIEW_RENAME:
			TreeView_EditLabel(hTreeView, TreeView_GetSelection(hTreeView));
			break;
		case ID_TO_SSA:
			{
				UserProc *u = dynamic_cast<UserProc*>(selectedProc());
				if (u == NULL)
					break;
				Boomerang::get()->noDecompile = true;
				u->decompile();
				Boomerang::get()->noDecompile = false;
				updateCodeView();
			}
			break;
		case ID_FROM_SSA:
			{
				UserProc *u = dynamic_cast<UserProc*>(selectedProc());
				if (u == NULL)
					break;
				u->fromSSAform();
				updateCodeView();
			}
			break;
		case ID_PROP_REGS:
			{
				UserProc *u = dynamic_cast<UserProc*>(selectedProc());
				if (u == NULL)
					break;
				u->propagateAtDepth(0);
				updateCodeView();
			}
			break;
		case ID_VIEW_NEWCLUSTER:
			{
				TVINSERTSTRUCT tvi;
				tvi.hParent = TreeView_GetRoot(hTreeView);
				tvi.hInsertAfter = TVI_LAST;
				tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvi.item.state = tvi.item.stateMask = TVIS_EXPANDED;
				tvi.item.pszText = "New Cluster";
				tvi.item.iImage = tvi.item.iSelectedImage = iCluster;
				MyLParam *p = new MyLParam(new Cluster());
				tvi.item.lParam = (LPARAM)p;
				prog->getRootCluster()->addChild(p->c);
				HTREEITEM h = TreeView_InsertItem(hTreeView, &tvi);
				TreeView_EditLabel(hTreeView, h);
			}
			break;
		case ID_VIEW_DELETECLUSTER:
			{
				TVITEM tvi;
				tvi.hItem = TreeView_GetSelection(hTreeView);
				tvi.mask = TVIF_PARAM;
				TreeView_GetItem(hTreeView, &tvi);
				MyLParam *p = (MyLParam*)tvi.lParam;
				if (p->isCluster && p->c != prog->getRootCluster()) {
					p->c->getParent()->removeChild(p->c);
					TreeView_DeleteItem(hTreeView, tvi.hItem);
					closeTab(p->c->getName());
				}
			}
			break;
		case ID_VIEW_PROPERTIES:
			{
				Proc *p = selectedProc();
				if (p)
					if (p->isLib())
						DialogBox(hInst, (LPCTSTR)IDD_LPROCPROPERTIES, hWnd, (DLGPROC)ProcProperties);
					else
						DialogBox(hInst, (LPCTSTR)IDD_PROCPROPERTIES, hWnd, (DLGPROC)ProcProperties);
			}
			break;
		case ID_DECODER_OPTIONS:
			DialogBox(hInst, (LPCTSTR)IDD_DECODEOPTIONS, hWnd, (DLGPROC)DecodeOptions);
			break;
		case ID_DECOMPILER_OPTIONS:
			DialogBox(hInst, (LPCTSTR)IDD_DECOMPILEOPTIONS, hWnd, (DLGPROC)DecompileOptions);
			break;
		case ID_CODEGENERATOR_REGENERATEALL:
			generateCodeForAll();
			break;
		case ID_EDIT_UNDO:
			prog->restoreMemo();
			if (!prog->canRestore())
				EnableMenuItem(GetMenu(hTopWnd), ID_EDIT_UNDO, MF_GRAYED);
			EnableMenuItem(GetMenu(hTopWnd), ID_EDIT_REDO, MF_ENABLED);
			updateAllTabs();
			updateTreeView();
			updateCodeView();
			break;
		case ID_EDIT_REDO:
			prog->restoreMemo(true);
			if (!prog->canRestore(true))
				EnableMenuItem(GetMenu(hTopWnd), ID_EDIT_REDO, MF_GRAYED);
			EnableMenuItem(GetMenu(hTopWnd), ID_EDIT_UNDO, MF_ENABLED);
			updateAllTabs();
			updateTreeView();
			updateCodeView();
			break;
		case ID_EDIT_COPY:
			if (selectedTab)
				SendMessage(tabWithName[selectedTab].edit, WM_COPY, 0, 0);
			break;
		case ID_EDIT_PASTE:
			if (selectedTab)
				SendMessage(tabWithName[selectedTab].edit, WM_PASTE, 0, 0);
			break;
		case ID_EDIT_SELECTALL:
			if (selectedTab)
				SendMessage(tabWithName[selectedTab].edit, EM_SETSEL, 0, -1);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		SelectObject(hdc, hTabBarFont);
		GetTextMetrics(hdc, &tm);
		GetClientRect(hWnd, &tabBarRect);
		tabBarRect.right -= 200;
		tabBarRect.bottom = tm.tmHeight;
		drawTabBar(hdc);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_LBUTTONDOWN:
		if (GET_Y_LPARAM(lParam) <= tabBarRect.bottom)
			handleTabBarPress(GET_X_LPARAM(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_NOTIFY:
		pnmtv = (LPNMTREEVIEW) lParam;
		if (pnmtv->hdr.code == TVN_SELCHANGED) {
			EnableMenuItem(GetMenu(hTopWnd), ID_VIEW_DELETECLUSTER, MF_GRAYED);
			if (clusterMenu)
				EnableMenuItem(clusterMenu, ID_VIEW_DELETECLUSTER, MF_GRAYED);
			updateCodeView();
		}
		ptvdi = (LPNMTVDISPINFO) lParam;
		if (ptvdi->hdr.code == TVN_ENDLABELEDIT && ptvdi->item.pszText) {
			MyLParam *lp = (MyLParam*)ptvdi->item.lParam;
			TreeView_SetItem(hTreeView, &ptvdi->item);
			TreeView_SelectItem(hTreeView, ptvdi->item.hItem);
			if (lp->isCluster) {
				std::map<std::string, struct my_tab>::iterator it = tabWithName.find(lp->c->getName());
				if (!strcmp(selectedTab, lp->c->getName()))
					selectedTab = strdup(ptvdi->item.pszText);
				if (it != tabWithName.end()) {
					struct my_tab t = (*it).second;
					t.name = strdup(ptvdi->item.pszText);
					tabWithName.erase(it);
					tabWithName[ptvdi->item.pszText] = t;
				}				
				lp->c->setName(ptvdi->item.pszText);
				if (lp->c == prog->getRootCluster())
					prog->setName(ptvdi->item.pszText);
				generateCodeForCluster(lp->c);
			} else {
				lp->p->setName(ptvdi->item.pszText);
				UserProc *u = dynamic_cast<UserProc*>(lp->p);
				if (u)
					generateCodeForUserProc(u);
			}
			InvalidateRect(hTopWnd, &tabBarRect, TRUE);
			saveUndoPoint();
		}
		if (pnmtv->hdr.code == TVN_BEGINDRAG) {
			treeDragging = pnmtv->itemNew.hItem;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

struct decompile_params {
	char name[128];
	char target[MAX_PATH];
};



class MyWatcher : public Watcher
{
public:
	MyWatcher() { }

	virtual void alert_decode(ADDRESS pc, int nBytes);
	virtual void alert_baddecode(ADDRESS pc);
	virtual void alert_start_decode(ADDRESS start, int nBytes);
	virtual void alert_end_decode();
	virtual void alert_decode(Proc *p, ADDRESS pc, ADDRESS last, int nBytes);
	virtual void alert_start_decompile(UserProc *p);
	virtual void alert_decompile_SSADepth(UserProc *p, int depth);
	virtual void alert_decompile_beforePropagate(UserProc *p, int depth);
	virtual void alert_decompile_afterPropagate(UserProc *p, int depth);
	virtual void alert_decompile_afterRemoveStmts(UserProc *p, int depth);
	virtual void alert_end_decompile(UserProc *p);
	virtual void alert_load(Proc *p);
	virtual void alert_new(Proc *p);
	virtual void alert_update_signature(Proc *p);
};

typedef enum { UNDECODED, INSTRUCTION, BAD_INSTRUCTION, PROCEDURE } blk_color;
ADDRESS lowest;
int totalBytes;
int scale = 0;
int vsize = 10;
HWND decodeDlg;
HDC decodeDC = NULL;
RECT decodeRect;

HPEN pens[4];

int completedProcs = 0;

HANDLE hLoadThread = NULL;

void LoadProjectThread(const char *fname)
{
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Loading project..");
	prog = Boomerang::get()->loadFromXML(fname);
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Done");
	hLoadThread = NULL;
	saveUndoPoint();
	ExitThread(0);
}

void OpenProject()
{
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	char buf[MAX_PATH], title[128];
	buf[0] = 0;
	title[0] = 0;
	ofn.lpstrFile = buf;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = title;
	ofn.nMaxFileTitle = 128;
	ofn.lpstrDefExt = "xml";
	char filter[] = { 'B', 'o', 'o', 'm', 'e', 'r', 'a', 'n', 'g', ' ', 'f', 'i', 'l', 'e', 's', 0, '*', '.', 'x', 'm', 'l', 0, 0 };
	ofn.lpstrFilter = filter;
	if (hLoadThread == NULL && GetOpenFileName(&ofn)) {
		CloseProject();
		DWORD id;
		hLoadThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LoadProjectThread, strdup(buf), 0, &id);
	}
}

HANDLE hSaveThread = NULL;

void SaveProjectThread()
{
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Saving project..");
	Boomerang::get()->persistToXML(prog);
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Saved");
	hSaveThread = NULL;
	ExitThread(0);
}

void SaveProject()
{
	if (prog && hSaveThread == NULL) {
		DWORD id;
		hSaveThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)SaveProjectThread, 0, 0, &id);
	}
}

void CloseProject()
{
	closeAllTabs();
	TreeView_DeleteAllItems(hTreeView);
	log[0] = 0;
	plog = log;
	SetWindowText(hLogView, log);
	completedProcs = 0;
	scale = 0;
	prog = NULL;
	if (decodeDC) {
		DeleteDC(decodeDC);
		decodeDC = NULL;
	}
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Done");
}

void drawBlk(ADDRESS start, int nBytes, blk_color color)
{
	if (decodeDC == NULL)
		return;
	int width = decodeRect.right - decodeRect.left;
	int height = decodeRect.bottom - decodeRect.top;
	if (scale == 0) {
		do {
			scale++;
			if (totalBytes < width)
				vsize = height;
			else
				vsize = height / (totalBytes / scale / width);
		} while (vsize < 10);
	}
	SelectObject(decodeDC, pens[color]);
	int sstart = (start - lowest) / scale, send = (start - lowest + nBytes) / scale;
	if (send - sstart == 0)
		send++;
	for (int sv = sstart; sv < send; sv++) {
		int x = decodeRect.left + (sv % decodeRect.right);
		int y = decodeRect.top + (sv / decodeRect.right) * vsize;
		MoveToEx(decodeDC, x, y, NULL);
		LineTo(decodeDC, x, y + vsize);
	}
}

LRESULT CALLBACK Decoding(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	DWORD rc;

	switch (message)
	{
	case WM_INITDIALOG:
		pens[0] = CreatePen(PS_SOLID, 0, RGB(0,0,0));
		pens[1] = CreatePen(PS_SOLID, 0, RGB(0,0,255));
		pens[2] = CreatePen(PS_SOLID, 0, RGB(255,0,0));
		pens[3] = CreatePen(PS_SOLID, 0, RGB(0,255,0));
		decodeDlg = hDlg;
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		GetClientRect(hDlg, &decodeRect);
		if (decodeDC == NULL) {
			decodeDC = CreateCompatibleDC(hdc);
			HBITMAP b = CreateCompatibleBitmap(hdc, decodeRect.right, decodeRect.bottom);
			SelectObject(decodeDC, b);
		}
		rc = BitBlt(hdc, 0, 0, decodeRect.right, decodeRect.bottom, decodeDC, 0, 0, SRCCOPY);
		EndPaint(hDlg, &ps);
		break;
	}
	return FALSE;
}

void MyWatcher::alert_decode(ADDRESS pc, int nBytes)
{
	if (pc < lowest)
		lowest = pc;
	drawBlk(pc, nBytes, INSTRUCTION);
	InvalidateRect(decodeDlg, NULL, FALSE);
}

void addProcToTree(Proc *p)
{
	HTREEITEM h = TreeView_GetRoot(hTreeView);
	while (h) {
		TVITEM i;
		i.hItem = h;
		i.mask = TVIF_TEXT | TVIF_HANDLE;
		char buf[1024];
		i.pszText = buf;
		i.cchTextMax = sizeof(buf);
		TreeView_GetItem(hTreeView, &i);
		if (!strcmp(i.pszText, p->getCluster()->getName()))
			break;
		h = TreeView_GetNextSibling(hTreeView, h);
	}
	if (h == NULL) {
		TVINSERTSTRUCT tvi;
		tvi.hParent = TVI_ROOT;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.item.state = tvi.item.stateMask = TVIS_EXPANDED;
		tvi.item.pszText = (LPSTR)p->getCluster()->getName();
		tvi.item.lParam = (LPARAM)new MyLParam(p->getCluster());
		tvi.item.iImage = tvi.item.iSelectedImage = iCluster;
		h = TreeView_InsertItem(hTreeView, &tvi);
	}

	TVINSERTSTRUCT tvi;
	tvi.hParent = h;
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tvi.item.state = tvi.item.stateMask = TVIS_EXPANDED;
	tvi.item.pszText = (LPSTR)p->getName();
	tvi.item.lParam = (LPARAM)new MyLParam(p);
	tvi.item.iImage = tvi.item.iSelectedImage = iProc;
	if (p->isLib()) {
		tvi.item.iImage = tvi.item.iSelectedImage = iLProc;
		if (p->getSignature()->isUnknown()) {
			tvi.item.iImage = tvi.item.iSelectedImage = iULProc;
			someUnknown = true;
		}
	}
	HTREEITEM hi = TreeView_InsertItem(hTreeView, &tvi);
	procItems[p] = hi;
}

void MyWatcher::alert_decode(Proc *p, ADDRESS pc, ADDRESS last, int nBytes)
{
	prog = p->getProg();

	if (pc < lowest)
		lowest = pc;
	drawBlk(pc, last - pc, PROCEDURE);
}

void MyWatcher::alert_new(Proc *p)
{
	addProcToTree(p);
}

void MyWatcher::alert_update_signature(Proc *p)
{
	for (std::map<Proc*,HTREEITEM>::iterator it = procItems.begin(); it != procItems.end(); it++)
		if ((*it).first == p) {
			TVITEM tvi;
			tvi.hItem = (*it).second;
			tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tvi.pszText = (LPSTR)p->getName();
			tvi.iImage = tvi.iSelectedImage = iProc;
			if (p->isLib()) {
				tvi.iImage = tvi.iSelectedImage = iLProc;
				if (p->getSignature()->isUnknown())
					tvi.iImage = tvi.iSelectedImage = iULProc;
			} else {
				UserProc *u = dynamic_cast<UserProc*>(p);
				if (u->isDecompiled())
					tvi.iImage = tvi.iSelectedImage = iDProc;
			}
			TreeView_SetItem(hTreeView, &tvi);
			break;
		}
}

void MyWatcher::alert_load(Proc *p)
{
	prog = p->getProg();
	addProcToTree(p);
}

void MyWatcher::alert_baddecode(ADDRESS pc)
{
	if (pc < lowest)
		lowest = pc;
	drawBlk(pc, 1, BAD_INSTRUCTION);
	InvalidateRect(decodeDlg, NULL, FALSE);
}

void MyWatcher::alert_start_decode(ADDRESS start, int nBytes)
{
	lowest = start;
	totalBytes = nBytes;
	drawBlk(start, nBytes, UNDECODED);
	InvalidateRect(decodeDlg, NULL, FALSE);
}

void MyWatcher::alert_end_decode()
{
	saveUndoPoint();
	SendMessage(decodeDlg, WM_COMMAND, IDOK, 0);
	if (someUnknown) {
		MessageBox(NULL, "Some library procedures were found that do not have known signatures. "
							"The decompiler has been suspended.	 To ensure the best possible decompilation, please take "
							"this opportunity to update the signature files.  Select the Tools -> Decompiler -> Resume menu "
							"option when you are ready to continue decompiling.", "Decompile suspended", MB_OK);
		suspendDecompiler();
	}
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Analysing...");
}

void MyWatcher::alert_start_decompile(UserProc *p)
{
	char buf[1024];
	sprintf(buf, "%i of %i complete.", completedProcs, p->getProg()->getNumUserProcs());
	SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)buf);
	sprintf(buf, "Decompiling %s.", p->getName());
	SendMessage(hStatusBar, SB_SETTEXT, 3, (LPARAM)buf);
	if (selectedProc() == p) {
		updateCodeView();
		suspendDecompiler();
	}
}

void MyWatcher::alert_decompile_SSADepth(UserProc *p, int depth)
{
	char buf[1024];
	sprintf(buf, "%i of %i complete.", completedProcs, p->getProg()->getNumUserProcs());
	SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)buf);
	sprintf(buf, "Decompiling %s. SSA at depth %i.", p->getName(), depth);
	SendMessage(hStatusBar, SB_SETTEXT, 3, (LPARAM)buf);
	if (selectedProc() == p) {
		updateCodeView();
		suspendDecompiler();
	}
}

void MyWatcher::alert_decompile_beforePropagate(UserProc *p, int depth)
{
	char buf[1024];
	sprintf(buf, "%i of %i complete.", completedProcs, p->getProg()->getNumUserProcs());
	SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)buf);
	sprintf(buf, "Decompiling %s. Before propagate at depth %i.", p->getName(), depth);
	SendMessage(hStatusBar, SB_SETTEXT, 3, (LPARAM)buf);
	if (selectedProc() == p) {
		updateCodeView();
		suspendDecompiler();
	}
}

void MyWatcher::alert_decompile_afterPropagate(UserProc *p, int depth)
{
	char buf[1024];
	sprintf(buf, "%i of %i complete.", completedProcs, p->getProg()->getNumUserProcs());
	SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)buf);
	sprintf(buf, "Decompiling %s. After propagate at depth %i.", p->getName(), depth);
	SendMessage(hStatusBar, SB_SETTEXT, 3, (LPARAM)buf);
	if (selectedProc() == p) {
		updateCodeView();
		suspendDecompiler();
	}
}

void MyWatcher::alert_decompile_afterRemoveStmts(UserProc *p, int depth)
{
	char buf[1024];
	sprintf(buf, "%i of %i complete.", completedProcs, p->getProg()->getNumUserProcs());
	SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)buf);
	sprintf(buf, "Decompiling %s. After removing statements at depth %i.", p->getName(), depth);
	SendMessage(hStatusBar, SB_SETTEXT, 3, (LPARAM)buf);
	if (selectedProc() == p) {
		updateCodeView();
		suspendDecompiler();
	}
}

void MyWatcher::alert_end_decompile(UserProc *p)
{
	saveUndoPoint();
	completedProcs++;
	char buf[1024];
	sprintf(buf, "%i of %i complete.", completedProcs, p->getProg()->getNumUserProcs());
	SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)buf);
	SendMessage(hStatusBar, SB_SETTEXT, 3, (LPARAM)"Decompiled");
	TVITEM tv;
	tv.hItem = procItems[p];
	tv.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
	tv.iImage = tv.iSelectedImage = iDProc;
	TreeView_SetItem(hTreeView, &tv);
	if (selectedProc() == p) {
		updateCodeView();
		suspendDecompiler();
	}
}

void setupDecompiler()
{
	Boomerang::get()->setLogger(new WindowLogger());
	MyWatcher *w = new MyWatcher();
	Boomerang::get()->addWatcher(w);
}

void doDecompile(struct decompile_params *params)
{
	someUnknown = false;
	if (Boomerang::get()->decompile(params->target, params->name))
		MessageBox(NULL, "Load failed", "", MB_ICONSTOP);
	if (decodeDlg)
		SendMessage(decodeDlg, WM_COMMAND, IDOK, 0);
	hDecompilerThread = NULL;
	updateDecompilerMenu();
	saveUndoPoint();
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)"Done");
	ExitThread(0);
}

LRESULT CALLBACK NewProject(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_NAME, "<Enter name>");
		SetDlgItemText(hDlg, IDC_OUTPUT, Boomerang::get()->getOutputPath().c_str());
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			char output[MAX_PATH];
			struct decompile_params *params = new decompile_params;
			GetDlgItemText(hDlg, IDC_NAME, params->name, sizeof(params->name));
			GetDlgItemText(hDlg, IDC_TARGET, params->target, sizeof(params->target));
			GetDlgItemText(hDlg, IDC_OUTPUT, output, sizeof(output));
			if (!params->name[0] || params->name[0] == '<')
				MessageBox(hDlg, "Enter a name for the project.", "Error", MB_OK);
			else if (!params->target[0])
				MessageBox(hDlg, "Choose a file to decompile.", "Error", MB_OK);
			else if (!output[0])
				MessageBox(hDlg, "Specify an output directory.", "Error", MB_OK);
			else if (!Boomerang::get()->setOutputDirectory(output))
				MessageBox(hDlg, "Specified output directory is invalid.", "Error", MB_OK);
			else {
				CloseProject();
				DWORD id;
				hDecompilerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)doDecompile, params, 0, &id);
				updateDecompilerMenu();
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
		}
		if (LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_DEBUGOPTIONS) {
			DialogBox(hInst, (LPCTSTR)IDD_DEBUGOPTIONS, hDlg, (DLGPROC)DebugOptions);
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_DECODEOPTIONS) {
			DialogBox(hInst, (LPCTSTR)IDD_DECODEOPTIONS, hDlg, (DLGPROC)DecodeOptions);
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_DECOMPILEOPTIONS) {
			DialogBox(hInst, (LPCTSTR)IDD_DECOMPILEOPTIONS, hDlg, (DLGPROC)DecompileOptions);
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_BROWSEBUTTON || LOWORD(wParam) == IDC_BROWSEBUTTON2) {
			OPENFILENAME ofn;
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			char buf[MAX_PATH], title[128];
			buf[0] = 0;
			title[0] = 0;
			ofn.lpstrFile = buf;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = title;
			ofn.nMaxFileTitle = 128;
			if (GetOpenFileName(&ofn)) {			
				if (LOWORD(wParam) == IDC_BROWSEBUTTON2) {
					SetDlgItemText(hDlg, IDC_OUTPUT, buf);
				} else {
					SetDlgItemText(hDlg, IDC_TARGET, buf);
					char name[128];
					GetDlgItemText(hDlg, IDC_NAME, name, sizeof(name));
					if (name[0] == 0 || name[0] == '<') {
						if (strrchr(title, '.'))
							*strrchr(title, '.') = 0;
						SetDlgItemText(hDlg, IDC_NAME, title);
					}
				}
			}
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK DebugOptions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			Boomerang::get()->debugDecoder		= (IsDlgButtonChecked(hDlg, IDC_CHECKD)		== BST_CHECKED);
			Boomerang::get()->debugGen			= (IsDlgButtonChecked(hDlg, IDC_CHECKCG)	== BST_CHECKED);
			Boomerang::get()->debugLiveness		= (IsDlgButtonChecked(hDlg, IDC_CHECKL)		== BST_CHECKED);
			Boomerang::get()->debugSwitch		= (IsDlgButtonChecked(hDlg, IDC_CHECKSA)	== BST_CHECKED);
			Boomerang::get()->debugProof		= (IsDlgButtonChecked(hDlg, IDC_CHECKPROOF) == BST_CHECKED);
			Boomerang::get()->debugUnusedRetsAndParams=(IsDlgButtonChecked(hDlg, IDC_CHECKUR)	== BST_CHECKED);
			Boomerang::get()->debugTA			= (IsDlgButtonChecked(hDlg, IDC_CHECKTA)	== BST_CHECKED);
			Boomerang::get()->debugUnusedStmt	= (IsDlgButtonChecked(hDlg, IDC_CHECKUS)	== BST_CHECKED);
			Boomerang::get()->printRtl			= (IsDlgButtonChecked(hDlg, IDC_CHECKRTL)	== BST_CHECKED);
			Boomerang::get()->traceDecoder		= (IsDlgButtonChecked(hDlg, IDC_CHECKTD)	== BST_CHECKED);
			Boomerang::get()->printAST			= (IsDlgButtonChecked(hDlg, IDC_CHECKAST)	== BST_CHECKED);
			Boomerang::get()->vFlag				= (IsDlgButtonChecked(hDlg, IDC_CHECKV)		== BST_CHECKED);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK DecodeOptions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_RADIOMAIN, BST_CHECKED);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			Boomerang::get()->decodeMain = (IsDlgButtonChecked(hDlg, IDC_RADIOMAIN) == BST_CHECKED);
			Boomerang::get()->decodeThruIndCall = (IsDlgButtonChecked(hDlg, IDC_CHECKDECODEINDIRECT) == BST_CHECKED);
			Boomerang::get()->noDecodeChildren = (IsDlgButtonChecked(hDlg, IDC_CHECKDECODECHILDREN) == BST_CHECKED);
			int count = SendMessage(GetDlgItem(hDlg, IDC_LISTENTRYPOINT), (UINT) LB_GETCOUNT, (WPARAM)0, (LPARAM)0);
			for (int idx = 0; idx < count; idx++) {
				char buf[128];
				SendMessage(GetDlgItem(hDlg, IDC_LISTENTRYPOINT), (UINT) LB_GETTEXT, (WPARAM)idx, (LPARAM)buf);
				int i;
				sscanf(buf, "%08X", &i);
				Boomerang::get()->entrypoints.push_back(i);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_ADDBUTTON)
		{
			char entrypoint[128];
			GetDlgItemText(hDlg, IDC_COMBOENTRYPOINT, entrypoint, sizeof(entrypoint));
			if (entrypoint[0]) {
				int i;
				if (sscanf(entrypoint, "0x%x", &i) == 1 || sscanf(entrypoint, "%x", &i) == 1 || sscanf(entrypoint, "%d", &i) == 1) {
					sprintf(entrypoint, "%08X", i);
					SendMessage(GetDlgItem(hDlg, IDC_LISTENTRYPOINT), (UINT) LB_ADDSTRING, (WPARAM)0, (LPARAM)entrypoint);
				}
			}
		}
		if (LOWORD(wParam) == IDC_REMOVEBUTTON)
		{
			int idx = SendMessage(GetDlgItem(hDlg, IDC_LISTENTRYPOINT), (UINT) LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
			if (idx != LB_ERR)
				SendMessage(GetDlgItem(hDlg, IDC_LISTENTRYPOINT), (UINT) LB_DELETESTRING, (WPARAM)idx, (LPARAM)0);
		}
		if (LOWORD(wParam) == IDC_CLEARBUTTON)
		{
			SendMessage(GetDlgItem(hDlg, IDC_LISTENTRYPOINT), (UINT) LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK DecompileOptions(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		CheckDlgButton(hDlg, IDC_RADIONTA, BST_CHECKED);
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			Boomerang::get()->noBranchSimplify = (IsDlgButtonChecked(hDlg, IDC_CHECKNOSIMPBRANCHES) == BST_CHECKED);
			Boomerang::get()->noRemoveNull = (IsDlgButtonChecked(hDlg, IDC_CHECKNOREMOVENULL) == BST_CHECKED);
			Boomerang::get()->noLocals = (IsDlgButtonChecked(hDlg, IDC_CHECKNOCREATELOCAL) == BST_CHECKED);
			Boomerang::get()->noParameterNames = (IsDlgButtonChecked(hDlg, IDC_CHECKNOPARAMS) == BST_CHECKED);
			Boomerang::get()->noRemoveLabels = (IsDlgButtonChecked(hDlg, IDC_CHECKNOUNNEEDEDLABELS) == BST_CHECKED);
			Boomerang::get()->noRemoveReturns = (IsDlgButtonChecked(hDlg, IDC_CHECKNOUNNEEDEDRETURNS) == BST_CHECKED);
			Boomerang::get()->noDataflow = (IsDlgButtonChecked(hDlg, IDC_CHECKNODATAFLOW) == BST_CHECKED);
			Boomerang::get()->stopBeforeDecompile = (IsDlgButtonChecked(hDlg, IDC_CHECKNODECOMP) == BST_CHECKED);
			Boomerang::get()->noProve = (IsDlgButtonChecked(hDlg, IDC_CHECKNOPROOF) == BST_CHECKED);
			Boomerang::get()->noChangeSignatures = (IsDlgButtonChecked(hDlg, IDC_CHECKNOCHANGESIGS) == BST_CHECKED);
			Boomerang::get()->conTypeAnalysis = false;
			Boomerang::get()->dfaTypeAnalysis = false;
			if (IsDlgButtonChecked(hDlg, IDC_RADIODFTA) == BST_CHECKED)
				Boomerang::get()->dfaTypeAnalysis = true;
			if (IsDlgButtonChecked(hDlg, IDC_RADIOCTA) == BST_CHECKED)
				Boomerang::get()->conTypeAnalysis = true;			
			char file[MAX_PATH];
			GetDlgItemText(hDlg, IDC_COMBOSYMBOLFILE, file, sizeof(file));
			if (file[0])
				Boomerang::get()->symbolFiles.push_back(file);
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_BROWSEBUTTON3) {
			OPENFILENAME ofn;
			memset(&ofn, 0, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			char buf[MAX_PATH], title[128];
			buf[0] = 0;
			title[0] = 0;
			ofn.lpstrFile = buf;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = title;
			ofn.nMaxFileTitle = 128;
			if (GetOpenFileName(&ofn)) {			
				SetDlgItemText(hDlg, IDC_COMBOSYMBOLFILE, buf);
			}
		}
		if (LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

void addReturnsToList(Proc *proc, HWND hReturns)
{
	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 0;

	for (int i = 0; i < proc->getSignature()->getNumReturns(); i++) {
		lvi.iItem = i;
		lvi.pszText = (LPSTR)proc->getSignature()->getReturnType(i)->getCtype();
		ListView_InsertItem(hReturns, &lvi);
		std::ostringstream st;
		proc->getSignature()->getReturnExp(i)->print(st);
		std::string s = st.str();
		ListView_SetItemText(hReturns, i, 1, (LPSTR)s.c_str());
	}
}

void addParamsToList(Proc *proc, HWND hParams)
{
	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 0;

	for (int i = 0; i < proc->getSignature()->getNumParams(); i++) {
		lvi.iItem = i;
		lvi.pszText = (LPSTR)proc->getSignature()->getParamName(i);
		ListView_InsertItem(hParams, &lvi);
		ListView_SetItemText(hParams, i, 1, (LPSTR)proc->getSignature()->getParamType(i)->getCtype());
		std::ostringstream st;
		proc->getSignature()->getParamExp(i)->print(st);
		std::string s = st.str();
		ListView_SetItemText(hParams, i, 2, (LPSTR)s.c_str());
	}
}

void addLocalsToList(UserProc *u, HWND hLocals)
{
	ListView_DeleteAllItems(hLocals);
	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.iSubItem = 0;

	for (int i = 0; i < u->getNumLocals(); i++) {
		lvi.iItem = i;
		lvi.pszText = (LPSTR)u->getLocalName(i);
		Exp *e = u->getLocalExp(lvi.pszText);
		ListView_InsertItem(hLocals, &lvi);
		ListView_SetItemText(hLocals, i, 1, (LPSTR)u->getLocalType(lvi.pszText)->getCtype());
		std::ostringstream st;
		e->print(st);
		std::string s = st.str();
		ListView_SetItemText(hLocals, i, 2, (LPSTR)s.c_str());
	}
}

LRESULT CALLBACK ProcProperties(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hReturns, hParams, hLocals;
	static Proc *proc;
	NMLVDISPINFO *pdi;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			hReturns = GetDlgItem(hDlg, IDC_SIGNATURE_RETURNS);
			hParams = GetDlgItem(hDlg, IDC_SIGNATURE_PARAMS);
			hLocals = GetDlgItem(hDlg, IDC_LOCALS);
			proc = selectedProc();
			SetDlgItemText(hDlg, IDC_SIGNATURE_NAME, proc->getName());
			LVCOLUMN lvc;
			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			lvc.cx = 120;
			lvc.pszText = "Name";
			ListView_InsertColumn(hParams, 0, &lvc);
			ListView_InsertColumn(hLocals, 0, &lvc);
			lvc.cx = 120;
			lvc.pszText = "Type";
			ListView_InsertColumn(hReturns, 0, &lvc);
			ListView_InsertColumn(hParams, 1, &lvc);
			ListView_InsertColumn(hLocals, 1, &lvc);
			lvc.cx = 135;
			lvc.pszText = "Expression";
			ListView_InsertColumn(hReturns, 1, &lvc);
			ListView_InsertColumn(hParams, 2, &lvc);
			ListView_InsertColumn(hLocals, 2, &lvc);

			addReturnsToList(proc, hReturns);
			addParamsToList(proc, hParams);

			UserProc *u = dynamic_cast<UserProc*>(proc);
			if (u) {
				addLocalsToList(u, hLocals);
			}
		}
		return TRUE;

	case WM_COMMAND:
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDOK) && LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDCANCEL) && LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_REREADSIGS)) {
			prog->rereadLibSignatures();
			ListView_DeleteAllItems(hReturns);
			ListView_DeleteAllItems(hParams);
			addReturnsToList(proc, hReturns);
			addParamsToList(proc, hParams);
		}
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_MSDN)) {
			char buf[1024];
			sprintf(buf, "http://search.microsoft.com/search/results.aspx?qu=%s", proc->getName());
			ShellExecute(NULL, "open", buf, NULL, NULL, SW_SHOW);
		}
		if (HIWORD(wParam) == EN_KILLFOCUS && LOWORD(wParam) == IDC_SIGNATURE_NAME) {
			char str[1024];
			GetDlgItemText(hDlg, IDC_SIGNATURE_NAME, str, sizeof(str));
			proc->setName(str);
			UserProc *u = dynamic_cast<UserProc*>(proc);
			if (u) {
				HTREEITEM h = TreeView_GetSelection(hTreeView);
				TVITEM tvi;
				tvi.mask = TVIF_TEXT | TVIF_HANDLE;
				tvi.hItem = h;
				tvi.pszText = str;
				TreeView_SetItem(hTreeView, &tvi);
				generateCodeForUserProc(u);	
			}
		}
		break;
	case WM_NOTIFY:
		pdi = (NMLVDISPINFO*) lParam;
		if (pdi->hdr.code == LVN_ENDLABELEDIT && pdi->item.pszText) {
			UserProc *u = dynamic_cast<UserProc*>(proc);
			if (pdi->hdr.hwndFrom == hLocals) {
				const char *origName = u->getLocalName(pdi->item.iItem);
				u->renameLocal(origName, pdi->item.pszText);
				ListView_SetItemText(hLocals, pdi->item.iItem, 0, pdi->item.pszText);
				generateCodeForUserProc(u);
				addLocalsToList(u, hLocals);
				return TRUE;
			} else if (pdi->hdr.hwndFrom == hParams) {
				proc->renameParam(proc->getSignature()->getParamName(pdi->item.iItem), pdi->item.pszText);
				ListView_SetItemText(hParams, pdi->item.iItem, 0, pdi->item.pszText);
				if (u)
					generateCodeForUserProc(u);				
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK SymbolTable(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hSymbols = NULL;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			hSymbols = GetDlgItem(hDlg, IDC_LIST1);
			LVCOLUMN lvc;
			lvc.mask = LVCF_TEXT | LVCF_WIDTH;
			lvc.cx = 120;
			lvc.pszText = "Name";
			ListView_InsertColumn(hSymbols, 0, &lvc);
			lvc.cx = 120;
			lvc.pszText = "Address";
			ListView_InsertColumn(hSymbols, 1, &lvc);

			LVITEM lvi;
			lvi.mask = LVIF_TEXT;
			lvi.iSubItem = 0;
			std::map<ADDRESS, std::string> &symbols = prog->getSymbols();
			int i = 0;
			for (std::map<ADDRESS, std::string>::iterator it = symbols.begin(); it != symbols.end(); it++, i++) {
				lvi.iItem = i;
				lvi.pszText = (LPSTR)(*it).second.c_str();
				ListView_InsertItem(hSymbols, &lvi);
				char tmp[20];
				sprintf(tmp, "%08X", (*it).first);
				ListView_SetItemText(hSymbols, i, 1, (LPSTR)tmp);
			}
		}
		return TRUE;

	case WM_COMMAND:
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDOK) && LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDCANCEL) && LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

LRESULT CALLBACK MemoryDump(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static RECT r;
	TEXTMETRIC tm;
	HDC hdc;
	PAINTSTRUCT ps;
	static ADDRESS top;
	static HWND hSB;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			if (prog == NULL)
				return FALSE;
			top = 0;
			hSB = GetDlgItem(hDlg, IDC_SCROLLBAR1);
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_RANGE | SIF_POS;
			si.nPos = 0;
			si.nMin = 0;
			si.nMax = prog->getImageSize() / 16;
			SetScrollInfo(hSB, SB_CTL, &si, TRUE);
		}
		return TRUE;

	case WM_VSCROLL:
		if (LOWORD(wParam) != SB_THUMBTRACK) {
			SCROLLINFO si;
			si.cbSize = sizeof(si);
			si.fMask = SIF_RANGE | SIF_POS;
			GetScrollInfo(hSB, SB_CTL, &si);
			si.fMask = SIF_POS;
			if (LOWORD(wParam) == SB_LINEDOWN && si.nPos < si.nMax)
				si.nPos++;
			if (LOWORD(wParam) == SB_LINEUP && si.nPos > si.nMin)
				si.nPos--;
			if (LOWORD(wParam) == SB_PAGEDOWN && si.nPos + 20 < si.nMax)
				si.nPos += 20;
			if (LOWORD(wParam) == SB_PAGEUP && si.nPos - 20 > si.nMin)
				si.nPos -= 20;
			if (LOWORD(wParam) == SB_THUMBPOSITION) {
				si.nPos = HIWORD(wParam);
			}
			SetScrollInfo(hSB, SB_CTL, &si, TRUE);
			top = si.nPos * 16;
			InvalidateRect(hDlg, &r, TRUE);
		}		
		return TRUE;

	case WM_COMMAND:
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDOK) && LOWORD(wParam) == IDOK) {
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDCANCEL) && LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		SelectObject(hdc, hMemoryDumpFont);
		GetClientRect(hDlg, &r);
		GetTextMetrics(hdc, &tm);
		for (int j = 0; r.top + j * tm.tmHeight < r.bottom; j ++) {
			char line[1024];
			
			sprintf(line, "%08X ", prog->getImageBase() + top + j * 16);
			for (int i = 0; i < 16; i++)
				sprintf(line + 9 + i * 3, "%02X%c", (unsigned char)prog->readNative1(prog->getImageBase() + top + j * 16 + i), i == 7 ? '-' : ' ');
			for (int i = 0; i < 16; i++) {
				char ch = prog->readNative1(prog->getImageBase() + top + j * 16 + i);
				sprintf(line + 9 + 16 * 3 + i, "%c", ch ? ch : '.');
			}

			TextOut(hdc, 0, r.top + j * tm.tmHeight, line, strlen(line));
		}
		EndPaint(hDlg, &ps);
		return TRUE;
	}
	return FALSE;
}
