HLOCAL LocalFree(HLOCAL hMem);
DWORD FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments);
int _write(int fd, char *buf, int size);
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

HACCEL LoadAcceleratorsA(      
    HINSTANCE hInstance,
    LPCSTR lpTableName
);

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

BOOL TranslateMessage(      
    const MSG *lpMsg
);

LRESULT DispatchMessageA(      
    const MSG *lpmsg
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

BOOL ShowWindow(      
    HWND hWnd,
    int nCmdShow
);

BOOL UpdateWindow(
  HWND hWnd
);
