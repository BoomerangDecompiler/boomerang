/* NOTE: All functions in this file are assumed to be PASCAL calling convention
  (hard coded in FrontEnd::readLibraryCatalog()) */

typedef unsigned int UINT;
typedef int INT;
typedef unsigned int SIZE_T;
typedef unsigned int LONG;
typedef unsigned char BYTE;
typedef BYTE *LPBYTE;
typedef const void *LPCVOID;
typedef void *LPVOID;
typedef void *PVOID;
typedef const char *LPCSTR;
typedef char *LPSTR;
typedef const short *LPCWSTR;
typedef short *LPWSTR;
typedef short wchar_t;
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef int HWND;
typedef int HMENU;
typedef int HFONT;
typedef int HLOCAL;
typedef int HINSTANCE;
typedef int HICON;
typedef int HCURSOR;
typedef int HBRUSH;
typedef int HACCEL;
typedef int HDC;
typedef int HGDIOBJ;
typedef int WPARAM;
typedef int LPARAM;
typedef int LRESULT;
typedef int ATOM;
typedef int BOOL;
typedef int HRSRC;
typedef int HMODULE;
typedef int HGLOBAL;
typedef unsigned int ULONG;
typedef unsigned char BYTE;
typedef char CHAR;
typedef int HRESULT;
typedef LRESULT WndProc(	  
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
);
typedef WndProc *WNDPROC;
typedef int CRITICAL_SECTION;
typedef CRITICAL_SECTION *LPCRITICAL_SECTION;

typedef int WinMain(	  
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
);

HLOCAL LocalFree(HLOCAL hMem);
DWORD FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments);
LPSTR CharNextA(	  
	LPCSTR lpsz
);
LPWSTR CharNextW(	   
	LPCWSTR lpsz
);
LPSTR GetCommandLineA(void);
LPWSTR GetCommandLineW(void);

typedef struct {
	UINT cbSize;
	UINT style;
	WNDPROC lpfnWndProc;
	int cbClsExtra;
	int cbWndExtra;
	HINSTANCE hInstance;
	HICON hIcon;
	HCURSOR hCursor;
	HBRUSH hbrBackground;
	LPCSTR lpszMenuName;
	LPCSTR lpszClassName;
	HICON hIconSm;
} WNDCLASSEX;
typedef struct {
	UINT cbSize;
	UINT style;
	WNDPROC lpfnWndProc;
	int cbClsExtra;
	int cbWndExtra;
	HINSTANCE hInstance;
	HICON hIcon;
	HCURSOR hCursor;
	HBRUSH hbrBackground;
	LPCWSTR lpszMenuName;
	LPCWSTR lpszClassName;
	HICON hIconSm;
} WNDCLASSEXw;


ATOM RegisterClassExW(WNDCLASSEXw *lpwcx);
ATOM RegisterClassExA(WNDCLASSEX *lpwcx);

int LoadStringA(	  
	HINSTANCE hInstance,
	UINT uID,
	LPSTR lpBuffer,
	int nBufferMax
);

int LoadStringW(	  
	HINSTANCE hInstance,
	UINT uID,
	LPWSTR lpBuffer,
	int nBufferMax
);

HACCEL LoadAcceleratorsA(	   
	HINSTANCE hInstance,
	LPCSTR lpTableName
);

HACCEL LoadAcceleratorsW(	   
	HINSTANCE hInstance,
	LPCWSTR lpTableName
);

typedef struct {
	int x;
	int y;
} POINT;
typedef POINT *LPPOINT;

typedef struct {
	int cx;
	int cy;
} SIZE;

typedef struct {
	HWND hwnd;
	UINT message;
	WPARAM wParam;
	LPARAM lParam;
	DWORD time;
	POINT pt;
} MSG;
typedef MSG *LPMSG;

BOOL GetMessageA(	   
	LPMSG lpMsg,
	HWND hWnd,
	UINT wMsgFilterMin,
	UINT wMsgFilterMax
);
BOOL GetMessageW(
	LPMSG lpMsg,
	HWND hWnd,
	UINT wMsgFilterMin,
	UINT wMsgFilterMax
);

int TranslateAcceleratorA(		
	HWND hWnd,
	HACCEL hAccTable,
	LPMSG lpMsg
);

int TranslateAcceleratorW(		
	HWND hWnd,
	HACCEL hAccTable,
	LPMSG lpMsg
);

BOOL TranslateMessage(		
	const MSG *lpMsg
);

LRESULT DispatchMessageA(	   
	const MSG *lpmsg
);

LRESULT DispatchMessageW(	   
	const MSG *lpmsg
);

HICON LoadIconW(	  
	HINSTANCE hInstance,
	LPCWSTR lpIconName
);

HCURSOR LoadCursorW(	  
	HINSTANCE hInstance,
	LPCWSTR lpCursorName
);

HICON LoadIconA(	  
	HINSTANCE hInstance,
	LPCSTR lpIconName
);

HCURSOR LoadCursorA(	  
	HINSTANCE hInstance,
	LPCSTR lpCursorName
);

HWND CreateWindowExA(	   
	DWORD dwExStyle,
	LPCSTR lpClassName,
	LPCSTR lpWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam
);

HWND CreateWindowExW(	   
	DWORD dwExStyle,
	LPCWSTR lpClassName,
	LPCWSTR lpWindowName,
	DWORD dwStyle,
	int x,
	int y,
	int nWidth,
	int nHeight,
	HWND hWndParent,
	HMENU hMenu,
	HINSTANCE hInstance,
	LPVOID lpParam
);

BOOL ShowWindow(	  
	HWND hWnd,
	int nCmdShow
);

BOOL UpdateWindow(
  HWND hWnd
);

int MessageBoxA(	  
	HWND hWnd,
	LPCSTR lpText,
	LPCSTR lpCaption,
	UINT uType
);

int MessageBoxW(	  
	HWND hWnd,
	LPCWSTR lpText,
	LPCWSTR lpCaption,
	UINT uType
);

BOOL GetProcessDefaultLayout(DWORD *pdwDefaultLayout);
BOOL SetProcessDefaultLayout(DWORD dwDefaultLayout);

HLOCAL LocalAlloc(
  UINT uFlags,
  SIZE_T uBytes
);

BOOL IsDialogMessageA(HWND hDlg, LPMSG lpMsg);
BOOL IsDialogMessageW(HWND hDlg, LPMSG lpMsg);

BOOL IsChild(HWND hWndParent, HWND hWnd);

HBRUSH GetSysColorBrush(int nIndex);

BOOL SystemParametersInfoA(
  UINT uiAction,
  UINT uiParam,
  PVOID pvParam,
  UINT fWinIni
);

BOOL SystemParametersInfoW(
  UINT uiAction,
  UINT uiParam,
  PVOID pvParam,
  UINT fWinIni
);

BOOL SetMenu(HWND hWnd, HMENU hMenu);
BOOL DestroyWindow(HWND hWnd);
BOOL DestroyMenu(HMENU hMenu);

typedef int INT_PTR;
typedef INT_PTR DlgProc(	  
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
);
typedef DlgProc *DLGPROC;

HWND CreateDialogParamA(	  
	HINSTANCE hInstance,
	LPCSTR lpTemplateName,
	HWND hWndParent,
	DLGPROC lpDialogFunc,
	LPARAM dwInitParam
);

HWND CreateDialogParamW(	  
	HINSTANCE hInstance,
	LPCWSTR lpTemplateName,
	HWND hWndParent,
	DLGPROC lpDialogFunc,
	LPARAM dwInitParam
);

LRESULT DefWindowProcA(		 
	HWND hWnd,
	UINT Msg,
	WPARAM wParam,
	LPARAM lParam
);

void PostQuitMessage(	   
	int nExitCode
);

typedef struct { 
  LONG left; 
  LONG top; 
  LONG right; 
  LONG bottom; 
} RECT;
typedef RECT *LPRECT;

typedef struct { 
  HDC  hdc; 
  BOOL fErase; 
  RECT rcPaint; 
  BOOL fRestore; 
  BOOL fIncUpdate; 
  BYTE rgbReserved[32]; 
} PAINTSTRUCT;
typedef PAINTSTRUCT *LPPAINTSTRUCT;

HDC BeginPaint(
  HWND hwnd,
  LPPAINTSTRUCT lpPaint
);

BOOL EndPaint(
  HWND hWnd,
  LPPAINTSTRUCT lpPaint
);

BOOL GetClientRect(		 
	HWND hWnd,
	LPRECT lpRect
);

int DrawTextA(
  HDC hDC,
  LPCSTR lpString,
  int nCount,
  LPRECT lpRect,
  UINT uFormat
);

int DrawTextW(
  HDC hDC,
  LPCWSTR lpString,
  int nCount,
  LPRECT lpRect,
  UINT uFormat
);

BOOL GetTextExtentPointA(
	HDC		hDC,
	LPCSTR	lpStr,
	int		len,
	SIZE*	sz
);

HGDIOBJ SelectObject(
	HDC hDC,
	HGDIOBJ hObj
);

INT_PTR DialogBoxParamA(	  
	HINSTANCE hInstance,
	LPCSTR lpTemplateName,
	HWND hWndParent,
	DLGPROC lpDialogFunc,
	LPARAM dwInitParam
);

INT_PTR DialogBoxParamW(	  
	HINSTANCE hInstance,
	LPCWSTR lpTemplateName,
	HWND hWndParent,
	DLGPROC lpDialogFunc,
	LPARAM dwInitParam
);

BOOL EndDialog(		 
	HWND hDlg,
	INT_PTR nResult
);

int wsprintfA(
	LPSTR lpOut,
	LPCSTR lpFmt,
	...
);

char *_gcvt(double value, int digits, char *buffer);  /* Convert flt to str */
int MulDiv(int number, int numerator, int denominator);
HFONT CreateFontIndirectA(void* lf);
HFONT CreateFontA(
  int nHeight,				 // height of font
  int nWidth,				 // average character width
  int nEscapement,			 // angle of escapement
  int nOrientation,			 // base-line orientation angle
  int fnWeight,				 // font weight
  DWORD fdwItalic,			 // italic attribute option
  DWORD fdwUnderline,		 // underline attribute option
  DWORD fdwStrikeOut,		 // strikeout attribute option
  DWORD fdwCharSet,			 // character set identifier
  DWORD fdwOutputPrecision,	 // output precision
  DWORD fdwClipPrecision,	 // clipping precision
  DWORD fdwQuality,			 // output quality
  DWORD fdwPitchAndFamily,	 // pitch and family
  LPCTSTR lpszFace			 // typeface name
);

typedef DWORD LCID;
LCID GetThreadLocale();
HLOCAL LocalReAlloc(
  HLOCAL hMem,
  SIZE_T uBytes,
  UINT uFlags
);
UINT GetProfileIntA(
  LPCSTR lpAppName,
  LPCSTR lpKeyName,
  INT nDefault
);
UINT GetProfileIntW(
  LPCWSTR lpAppName,
  LPCWSTR lpKeyName,
  INT nDefault
);
DWORD GetProfileStringA(
  LPCSTR lpAppName,
  LPCSTR lpKeyName,
  LPCSTR lpDefault,
  LPSTR lpReturnedString,
  DWORD nSize
);
DWORD GetProfileStringW(
  LPCWSTR lpAppName,
  LPCWSTR lpKeyName,
  LPCWSTR lpDefault,
  LPWSTR lpReturnedString,
  DWORD nSize
);
DWORD GetSysColor(
  int nIndex
);
BOOL GetWindowRect(		 
	HWND hWnd,
	LPRECT lpRect
);
LONG GetWindowLongA(
    HWND hWnd,
    int nIndex
);
LONG GetWindowLongW(
    HWND hWnd,
    int nIndex
);
LONG SetWindowLongA(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong
);
LONG SetWindowLongW(
    HWND hWnd,
    int nIndex,
    LONG dwNewLong
);
HMENU LoadMenuA(
    HINSTANCE hInstance,
    LPCSTR lpMenuName
);
HMENU LoadMenuW(
    HINSTANCE hInstance,
    LPCWSTR lpMenuName
);
LRESULT SendMessageA(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
);
LRESULT SendMessageW(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
);
BOOL InvalidateRect(
  HWND hWnd,
  LPRECT lpRect,
  BOOL bErase
);
HWND GetDlgItem(	  
	HWND hDlg,
	int nIDDlgItem
);
BOOL EnableWindow(		
	HWND hWnd,
	BOOL bEnable
);
int MapWindowPoints(
  HWND hWndFrom,
  HWND hWndTo,
  LPPOINT lpPoints,
  UINT cPoints
);
BOOL OffsetRect(
  LPRECT lprc,
  int dx,
  int dy
);
BOOL SetWindowPos(		
	HWND hWnd,
	HWND hWndInsertAfter,
	int X,
	int Y,
	int cx,
	int cy,
	UINT uFlags
);
HMENU GetMenu(		
	HWND hWnd
);
HMENU GetSubMenu(	   
	HMENU hMenu,
	int nPos
);
DWORD CheckMenuItem(	  
	HMENU hmenu,
	UINT uIDCheckItem,
	UINT uCheck
);
BOOL SetDlgItemTextA(
	HWND hDlg,
	int nIDDlgItem,
	LPCSTR lpString
);
BOOL SetDlgItemTextW(
	HWND hDlg,
	int nIDDlgItem,
	LPCWSTR lpString
);
BOOL CheckRadioButton(		
	HWND hDlg,
	int nIDFirstButton,
	int nIDLastButton,
	int nIDCheckButton
);
HCURSOR SetCursor(		
	HCURSOR hCursor
);
BOOL TextOutA(
	HDC hDC,
	int x, int y,
	char* sz, int len);

int FillRect(
  HDC hDC,			// handle to DC
  RECT *lprc,		// rectangle
  HBRUSH hbr		// handle to brush
);

struct SLIST_ENTRY {
  SLIST_ENTRY *Next;
};

typedef SLIST_ENTRY *PSLIST_ENTRY;

PSLIST_ENTRY InterlockedPushEntrySList(
  PSLIST_HEADER ListHead,
  PSLIST_ENTRY ListEntry
);

PSLIST_ENTRY InterlockedPopEntrySList(
  PSLIST_HEADER ListHead
);

ULONGLONG VerSetConditionMask(
  ULONGLONG dwlConditionMask,
  DWORD dwTypeBitMask,
  BYTE dwConditionMask
);

int FreeLibrary(unsigned int hModule);
int VirtualFree(void* lpAddress, int dwSize, unsigned int dwFreeType);

HRSRC FindResourceA(HMODULE hModule, LPCSTR lpName, LPCSTR lpType);
HGLOBAL LoadResource(HMODULE hModule, HRSRC hResInfo);
LPVOID LockResource(HGLOBAL hResData);
DWORD GetTempPathA(DWORD nBufferLength, LPSTR lpBuffer);
DWORD SizeofResource(HMODULE hModule, HRSRC hResInfo);
HINSTANCE ShellExecuteA(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd);

typedef struct {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  CHAR  szCSDVersion[128];
} OSVERSIONINFOA;

typedef struct {
  DWORD dwOSVersionInfoSize;
  DWORD dwMajorVersion;
  DWORD dwMinorVersion;
  DWORD dwBuildNumber;
  DWORD dwPlatformId;
  WCHAR szCSDVersion[128];
} OSVERSIONINFOW;

BOOL GetVersionExA(OSVERSIONINFOA *v);
BOOL GetVersionExW(OSVERSIONINFOW *v);

typedef struct {
  DWORD  dwOemId;
  DWORD  dwPageSize;
  LPVOID lpMinimumApplicationAddress;
  LPVOID lpMaximumApplicationAddress;
  DWORD  dwActiveProcessorMask;
  DWORD  dwNumberOfProcessors;
  DWORD  dwProcessorType;
  DWORD  dwReserved1;
  DWORD  dwReserved2;
} SYSTEM_INFO;

void GetSystemInfo(SYSTEM_INFO *sinf);

typedef struct {
  DWORD dwLength;      // sizeof(MEMORYSTATUS)
  DWORD dwMemoryLoad;  // % in use
  DWORD dwTotalPhys;
  DWORD dwAvailPhys;
  DWORD dwTotalPageFile;
  DWORD dwAvailPageFile;
  DWORD dwTotalVirtual;
  DWORD dwAvailVirtual;
} MEMORYSTATUS;

void GlobalMemoryStatus(MEMORYSTATUS *mst);

typedef struct {
  UINT MaxCharSize;
  BYTE DefaultChar[2]; // MAX_DEFAULTCHAR
  BYTE LeadByte[12];   // MAX_LEADBYTES
} CPINFO;

BOOL GetCPInfo(UINT uCodePage, CPINFO *lpCPInfo);

typedef void *HANDLE;

HANDLE RegisterEventSourceA(char *uncServerName, char *sourceName);
BOOL DeregisterEventSource(HANDLE eventLog);

typedef void *FARPROC;

FARPROC GetProcAddress(HANDLE module, char *procName);

LONG InterlockedExchange(LONG *target, LONG *val);

HINSTANCE LoadLibraryExA(LPCSTR lib, HANDLE file, DWORD flags);
HINSTANCE LoadLibraryExW(LPCWSTR lib, HANDLE file, DWORD flags);

int MultiByteToWideChar(UINT CodePage, DWORD dwFlags,
                        LPCSTR lpMultiByteStr, int cbMultiByte,
                        LPWSTR lpWideCharStr, int cchWideChar);

// long name causes assertion failure
// typedef DWORD ACCESS_MASK;
typedef DWORD ACC_MSK;

LONG RegOpenKeyExA(HANDLE hKey, LPCTSTR lpSubKey, DWORD ulOptions,
                  ACC_MSK regsam, HANDLE *phkResult);
LONG RegOpenKeyExW(HANDLE hKey, LPCTSTR lpSubKey, DWORD ulOptions,
                  ACC_MSK regsam, HANDLE *phkResult);
LONG RegCloseKey(HANDLE hKey);

void SetLastError(DWORD error);

void Sleep(DWORD milliseconds);

LPVOID HeapAlloc(HANDLE hHeap, DWORD dwFlags, DWORD dwBytes);

BOOL HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);

DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
DWORD GetModuleFileNameW(HMODULE hModule, LPWSTR lpFilename, DWORD nSize);

HANDLE GetStdHandle(DWORD nStdHandle);
BOOL   SetStdHandle(DWORD nStdHandle, HANDLE hHandle);

BOOL WriteFile(
    HANDLE hFile,
    LPCVOID lpBuffer,
    DWORD nNumberOfBytesToWrite,
    LPDWORD lpNumberOfBytesWritten,
    LPOVERLAPPED lpOverlapped
    );

LPVOID VirtualAlloc(
    LPVOID lpAddress,
    DWORD dwSize,
    DWORD flAllocationType,
    DWORD flProtect
    );

BOOL FlushFileBuffers(HANDLE hFile);

DWORD GetLastError(void);

DWORD
SetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod
    );

BOOL CloseHandle(HANDLE hObject);
BOOL CheckMenuRadioItem(
    HMENU hmenu,
    UINT idFirst,
    UINT idLast,
    UINT idCheck,
    UINT uFlags
);

BOOL SetWindowTextA(
    HWND hWnd,
    LPCSTR lpString
);

BOOL SetWindowTextW(
    HWND hWnd,
    LPCWSTR lpString
);

HWND SetFocus(
    HWND hWnd
);

LPSTR lstrcpynA(
    LPSTR lpString1,
    LPCSTR lpString2,
    int iMaxLength
);

LPWSTR lstrcpynW(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    int iMaxLength
);

LPSTR lstrcatA(
    LPSTR lpString1,
    LPSTR lpString2
);

LPWSTR lstrcatW(
    LPWSTR lpString1,
    LPWSTR lpString2
);

wchar_t *lstrcpyW(wchar_t *dst, wchar_t *src);

int lstrlenW( const wchar_t *string );

void _CxxThrowException(int a, int b);

BOOL DisableThreadLibraryCalls(
  HMODULE hModule
);

BOOL HeapDestroy(
  HANDLE hHeap
);

void DeleteCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
);

void InitializeCriticalSection(
  LPCRITICAL_SECTION lpCriticalSection
);

typedef struct {
  DWORD cb;
  LPSTR lpReserved;
  LPSTR lpDesktop;
  LPSTR lpTitle;
  DWORD dwX;
  DWORD dwY;
  DWORD dwXSize;
  DWORD dwYSize;
  DWORD dwXCountChars;
  DWORD dwYCountChars;
  DWORD dwFillAttribute;
  DWORD dwFlags;
  WORD wShowWindow;
  WORD cbReserved2;
  LPBYTE lpReserved2;
  HANDLE hStdInput;
  HANDLE hStdOutput;
  HANDLE hStdError;
} STARTUPINFO;
typedef STARTUPINFO *LPSTARTUPINFO;

typedef struct {
  DWORD cb;
  LPWSTR lpReserved;
  LPWSTR lpDesktop;
  LPWSTR lpTitle;
  DWORD dwX;
  DWORD dwY;
  DWORD dwXSize;
  DWORD dwYSize;
  DWORD dwXCountChars;
  DWORD dwYCountChars;
  DWORD dwFillAttribute;
  DWORD dwFlags;
  WORD wShowWindow;
  WORD cbReserved2;
  LPBYTE lpReserved2;
  HANDLE hStdInput;
  HANDLE hStdOutput;
  HANDLE hStdError;
} STARTUPINFOW;
typedef STARTUPINFOW *LPSTARTUPINFOW;

void GetStartupInfo(LPSTARTUPINFO lpStartupInfo);
void GetStartupInfoW(LPSTARTUPINFOW lpStartupInfo);

HMODULE GetModuleHandle(LPCSTR lpModuleName);
HMODULE GetModuleHandleW(LPCWSTR lpModuleName);

void ExitProcess(UINT uExitCode);

BOOL PostMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL PostMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

typedef void *REFCLSID;
HRESULT SHLoadInProc(REFCLSID rclsid);

HWND FindWindowA(LPCSTR lpClassName, LPCSTR lpWindowName);
HWND FindWindowW(LPCWSTR lpClassName, LPCWSTR lpWindowName);

void Sleep(DWORD dwMilliseconds);

HMODULE LoadLibraryA(LPCSTR lpFileName);
HMODULE LoadLibraryW(LPCWSTR lpFileName);


// guessed LPR functions

void CloseLPR(int a);
int GetShortQueue(int a, int b);
int GetLongQueue(int a, int b);
int OpenLPR(int a, int b);

