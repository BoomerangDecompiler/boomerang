
typedef unsigned int UINT;
typedef unsigned int SIZE_T;
typedef unsigned int LONG;
typedef const void *LPCVOID;
typedef void *LPVOID;
typedef void *PVOID;
typedef const char LPCSTR[];
typedef char LPSTR[];
typedef const short LPCWSTR[];
typedef short LPWSTR[];
typedef int DWORD;
typedef int HWND;
typedef int HLOCAL;
typedef int HINSTANCE;
typedef int HICON;
typedef int HCURSOR;
typedef int HBRUSH;
typedef int HACCEL;
typedef int WPARAM;
typedef int LPARAM;
typedef int LRESULT;
typedef int ATOM;
typedef int BOOL;
typedef unsigned char BYTE;
typedef LRESULT WndProc(      
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
);
typedef WndProc *WNDPROC;

typedef int WinMain(      
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow
);

HLOCAL LocalFree(HLOCAL hMem);
DWORD FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments);
int _write(int fd, char buf[], int size);
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
typedef MSG *PMSG;

BOOL GetMessageA(      
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

BOOL EndPaint(
  HWND hWnd,
  LPPAINTSTRUCT lpPaint
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

POINT ?MoveTo@CDC@@QAE?AVCPoint@@HH@Z(  /* CDC::MoveTo */
    POINT *ret,     /* MSVC convention when returning structs is to pass a
                        hidden first parameter */
    int x,
    int y
);

BOOL ?LineTo@CDC@@QAEHHH@Z(     /* CDC::LineTo */
    int x,
    int y
);

UINT ?SetTextAlign@CDC@@QAEII@Z(UINT flags);
void ??0CString@@QAE@PBD@Z(char* sz);   /* CString constructor with C string */
void ??1CString@@QAE@XZ();          /* CString destructor */
int ?FindIndex@CObList@@QBEPAU__POSITION@@H@Z(  /* CObList::FindIndex */
    int idx
);
int wsprintfA(
    LPTSTR lpOut,
    LPCTSTR lpFmt,
    ...
);

char *_gcvt(double value, int digits, char *buffer);  /* Convert flt to str */
int MulDiv(int number, int numerator, int denominator);
