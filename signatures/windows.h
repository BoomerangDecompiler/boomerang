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
