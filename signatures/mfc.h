/* NOTE: All functions in this file are assumed to be THISCALL calling
  convention (hard coded in FrontEnd::readLibraryCatalog()) */

/* Must include windows.h before this file */

typedef struct {
	void*	vt;
} CObject;

typedef struct {
	void	*vt;
	char	filler[28];
	void*	hWnd;
} CWnd;

typedef struct {
	CObject obj;
	HGDIOBJ m_hObject;
} CGdiObject;

typedef struct {
	CGdiObject g;
} CBrush;

typedef struct {
	CObject obj;
	HDC		m_hDC;
	HDC		m_attribDC;
	BOOL	bIsPrinting;
} CDC;

typedef struct	{
	CDC		cdc;
	HWND	m_hWnd;
	PAINTSTRUCT m_ps;
} CPaintDC;


POINT ?MoveTo@CDC@@QAE?AVCPoint@@HH@Z(	/* CDC::MoveTo */
	CDC* this,
	POINT *ret,		/* MSVC convention when returning structs is to pass a
						hidden first parameter, actually second here */
	int x,
	int y);

BOOL ?LineTo@CDC@@QAEHHH@Z(		/* CDC::LineTo */
	CDC* this,
	int x,
	int y);

UINT ?SetTextAlign@CDC@@QAEII@Z(
	CDC* this,
	UINT flags);

int ?SetBkMode@CDC@@QAEHH@Z(
	CDC* this,
	int mode);

BOOL CDC_TextOut(CDC* this, int x, int y, char* sz, int len);
void CDC_FillRect(CDC* this, RECT* r, CBrush* br);


void ??0CString@@QAE@PBD@Z(CString* this, char* sz);   /* CString
											constructor with C string */
void ??1CString@@QAE@XZ(CString* this);			 /* CString destructor */
typedef struct {
	CObject obj;		/* More... */
} CObList;
int ?FindIndex@CObList@@QBEPAU__POSITION@@H@Z(	/* CObList::FindIndex */
	CObList* this,
	int idx
);
void ?OnCreate@CView@@IAEHPAUtagCREATESTRUCTA@@@Z(
	CView* this,
	void* lpCreateStruct
);
void ?Attach@CGdiObject@@QAEHPAX@Z(
	CGdiObject* this,
	void* obj
);

