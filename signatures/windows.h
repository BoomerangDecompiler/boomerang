HLOCAL LocalFree(HLOCAL hMem);
DWORD FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPTSTR lpBuffer, DWORD nSize, va_list* Arguments);
int _write(int fd, char *buf, int size);
short *CharNextW(short *lpsz);
short *GetCommandLineW();

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
    LPCTSTR lpszMenuName;
    LPCTSTR lpszClassName;
    HICON hIconSm;
} WNDCLASSEX;
typedef WNDCLASSEX *PWNDCLASSEX;

ATOM RegisterClassExW(WNDCLASSEX *lpwcx);
ATOM RegisterClassExA(WNDCLASSEX *lpwcx);

int LoadStringA(      
    HINSTANCE hInstance,
    UINT uID,
    LPTSTR lpBuffer,
    int nBufferMax
);

HACCEL LoadAcceleratorsA(      
    HINSTANCE hInstance,
    LPCTSTR lpTableName
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
    LPCTSTR lpIconName
);

HCURSOR LoadCursorA(      
    HINSTANCE hInstance,
    LPCTSTR lpCursorName
);

HWND CreateWindowExA(      
    DWORD dwExStyle,
    LPCTSTR lpClassName,
    LPCTSTR lpWindowName,
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
