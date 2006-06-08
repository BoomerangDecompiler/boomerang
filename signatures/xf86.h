
typedef int OptionValueType;
typedef struct {
  double freq;
  int units;
} ValueUnion;        // this is actually a union of other stuff and this :(
typedef unsigned char CARD8;
typedef unsigned short CARD16;
typedef unsigned int CARD32;
typedef unsigned long IOADDRESS;
typedef void *pointer;
typedef int Bool;
typedef int INT32;

typedef struct {
    const char * modname;
    const char * vendor;
    CARD32       _modinfo1_;
    CARD32       _modinfo2_;
    CARD32       xf86version;
    CARD8        majorversion;
    CARD8        minorversion;
    CARD16       patchlevel;
    const char * abiclass;
    CARD32       abiversion;
    const char * moduleclass;
    CARD32       checksum[4];
} XF86ModuleVersionInfo;

typedef pointer ModuleSetupProc(pointer module, pointer opts, int *errmaj, int *errmin);
typedef void ModuleTearDownProc(pointer module);

typedef struct {
    XF86ModuleVersionInfo 	*vers;
    ModuleSetupProc             *setup;
    ModuleTearDownProc          *teardown;
} XF86ModuleData;


typedef struct {
    int                 token;
    const char*         name;
    OptionValueType     type;
    ValueUnion          value;
    Bool                found;
} OptionInfoRec;
typedef OptionInfoRec *OptionInfoPtr;
typedef OptionInfoRec OptionInfoRecs[];

typedef void IdentifyFunc(int flags);
typedef OptionInfoRecs *AvailableOptionsFunc(int chipid, int bustype);

typedef struct {
    int driverVersion;
    char *driverName;
    IdentifyFunc *Identify;
    ProbeFunc *Probe;
    AvailableOptionsFunc *AvailableOptions;
    pointer module;
    int refCount;
} DriverRec;
typedef DriverRec *DriverPtr;

typedef Bool ProbeFunc(DriverRec *drv, int flags);

void xf86AddDriver(DriverPtr driver, pointer module, int flags);

typedef const char *Sym;
typedef Sym SymList[];

void xf86LoaderRefSymLists(SymList *p, ...);
void xf86LoaderRefSymbols(const char *p, ...);
void LoaderRefSymLists(SymList *p, ...);
void LoaderRefSymbols(const char *p, ...);

typedef struct {
   char *                       identifier;
   char *                       driver;
   pointer                      commonOptions;
   pointer                      extraOptions;
} IDevRec;
typedef IDevRec *IDevPtr;

typedef struct {
    int                 vendor;
    int                 chipType;
    int                 chipRev;
    int                 subsysVendor;
    int                 subsysCard;
    int                 bus;
    int                 device;
    int                 func;
    int                 class;
    int                 subclass;
    int                 interface;
    memType             memBase[6];
    memType             ioBase[6];
    int                 size[6];
    unsigned char       type[6];
    memType             biosBase;
    int                 biosSize;
    pointer             thisCard;
    Bool                validSize;
    Bool                validate;
    CARD32              listed_class;
} pciVideoRec;
typedef pciVideoRec *pciVideoPtr;

typedef struct {
    int                 frameX0;
    int                 frameY0;
    int                 virtualX;
    int                 virtualY;
    int                 depth;
    int                 fbbpp;
    rgb                 weight;
    rgb                 blackColour;
    rgb                 whiteColour;
    int                 defaultVisual;
    char **             modes;
    pointer             options;
} DispRec;
typedef DispRec *DispPtr;

typedef struct {
    char *              identifier;
    pointer             options;
} confXvPortRec;
typedef confXvPortRec *confXvPortPtr;

typedef struct {
    char *              identifier;
    int                 numports;
    confXvPortPtr       ports;
    pointer             options;
} confXvAdaptorRec;
typedef confXvAdaptorRec *confXvAdaptorPtr;

typedef struct { 
	float hi;
	float lo; 
} range;

typedef struct {
    char *              id;
    char *              vendor;
    char *              model;
    int                 nHsync;
    range               hsync[8];
    int                 nVrefresh;
    range               vrefresh[8];
    DisplayModePtr      Modes;          /* Start of the monitor's mode list */
    DisplayModePtr      Last;           /* End of the monitor's mode list */
    Gamma               gamma;          /* Gamma of the monitor */
    int                 widthmm;
    int                 heightmm;
    pointer             options;
    pointer             DDC;
} MonRec;
typedef MonRec *MonPtr;

typedef struct {
    char *              id;
    int                 screennum;
    int                 defaultdepth;
    int                 defaultbpp;
    int                 defaultfbbpp;
    MonPtr              monitor;
    GDevPtr             device;
    int                 numdisplays;
    DispPtr             displays;
    int                 numxvadaptors;
    confXvAdaptorPtr    xvadaptors;
    pointer             options;
} confScreenRec;
typedef confScreenRec *confScreenPtr;


typedef struct {
   char *                       identifier;
   char *                       vendor;
   char *                       board;
   char *                       chipset;
   char *                       ramdac;
   char *                       driver;
   confScreenPtr      		myScreenSection;
   Bool                         claimed;
   int                          dacSpeeds[4];
   int                          numclocks;
   int                          clock[128];
   char *                       clockchip;
   char *                       busID;
   Bool                         active;
   Bool                         inUse;
   int                          videoRam;
   int                          textClockFreq;
   unsigned long                BiosBase;
   unsigned long                MemBase;
   unsigned long                IOBase;
   int                          chipID;
   int                          chipRev;
   pointer                      options;
   int                          irq;
   int                          screen;
} GDevRec;
typedef GDevRec *GDevPtr;

int xf86MatchDevice(const char *drivername, GDevPtr **driversectlist);

typedef unsigned long memType;

pciVideoPtr *xf86GetPciVideoInfo(void);

typedef unsigned int uint;

pointer Xrealloc(pointer p, uint n);

typedef struct {
    int                 token;
    const char *        name;
} SymTabRec;
typedef SymTabRec *SymTabPtr;

typedef struct {
    int numChipset;
    int PCIid;
    resRange *resList;
} PciChipsets;

typedef struct {
    unsigned long type;
    memType a;
    memType b;
} resRange;
typedef resRange *resList;

int xf86MatchPciInstances(const char *driverName, int vendorID,
                      SymTabPtr chipsets, PciChipsets *PCIchipsets,
                      GDevPtr *devList, int numDevs, DriverPtr drvp,
                      int **foundEntities);
void Xfree(pointer p);

typedef struct {
    unsigned char       depth;
    unsigned char       bitsPerPixel;
    unsigned char       scanlinePad;
} PixmapFormatRec;

typedef struct {
    int                 myNum;
    ATOM                id;
    short               width;
	short				height;
    short               mmWidth;
	short				mmHeight;
    short               numDepths;
    unsigned char       rootDepth;
    DepthPtr            allowedDepths;
    unsigned long       rootVisual;
    unsigned long       defColormap;
    short               minInstalledCmaps;
	short				maxInstalledCmaps;
    char                backingStoreSupport;
	char				saveUnderSupport;
    unsigned long       whitePixel;
	unsigned long		blackPixel;
    unsigned long       rgf;
    GCPtr               GCperDepth[9];
    PixmapPtr           PixmapPerDepth[1];
    pointer             devPrivate;
    short               numVisuals;
    VisualPtr           visuals;
    int                 WindowPrivateLen;
    unsigned            *WindowPrivateSizes;
    unsigned            totalWindowSize;
    int                 GCPrivateLen;
    unsigned            *GCPrivateSizes;
    unsigned            totalGCSize;

    CloseScreenProcPtr          CloseScreen;
    QueryBestSizeProcPtr        QueryBestSize;
    SaveScreenProcPtr           SaveScreen;
    GetImageProcPtr             GetImage;
    GetSpansProcPtr             GetSpans;
    PointerNonInterestBoxProcPtr PointerNonInterestBox;
    SourceValidateProcPtr       SourceValidate;

    CreateWindowProcPtr         CreateWindow;
    DestroyWindowProcPtr        DestroyWindow;
    PositionWindowProcPtr       PositionWindow;
    ChangeWindowAttributesProcPtr ChangeWindowAttributes;
    RealizeWindowProcPtr        RealizeWindow;
    UnrealizeWindowProcPtr      UnrealizeWindow;
    ValidateTreeProcPtr         ValidateTree;
    PostValidateTreeProcPtr     PostValidateTree;
    WindowExposuresProcPtr      WindowExposures;
    PaintWindowBackgroundProcPtr PaintWindowBackground;
    PaintWindowBorderProcPtr    PaintWindowBorder;
    CopyWindowProcPtr           CopyWindow;
    ClearToBackgroundProcPtr    ClearToBackground;
    ClipNotifyProcPtr           ClipNotify;
    RestackWindowProcPtr        RestackWindow;

    CreatePixmapProcPtr         CreatePixmap;
    DestroyPixmapProcPtr        DestroyPixmap;

    SaveDoomedAreasProcPtr      SaveDoomedAreas;
    RestoreAreasProcPtr         RestoreAreas;
    ExposeCopyProcPtr           ExposeCopy;
    TranslateBackingStoreProcPtr TranslateBackingStore;
    ClearBackingStoreProcPtr    ClearBackingStore;
    DrawGuaranteeProcPtr        DrawGuarantee;
    BSFuncRec                   BackingStoreFuncs;

    RealizeFontProcPtr          RealizeFont;
    UnrealizeFontProcPtr        UnrealizeFont;

    ConstrainCursorProcPtr      ConstrainCursor;
    CursorLimitsProcPtr         CursorLimits;
    DisplayCursorProcPtr        DisplayCursor;
    RealizeCursorProcPtr        RealizeCursor;
    UnrealizeCursorProcPtr      UnrealizeCursor;
    RecolorCursorProcPtr        RecolorCursor;
    SetCursorPositionProcPtr    SetCursorPosition;

    CreateGCProcPtr             CreateGC;

    CreateColormapProcPtr       CreateColormap;
    DestroyColormapProcPtr      DestroyColormap;
    InstallColormapProcPtr      InstallColormap;
    UninstallColormapProcPtr    UninstallColormap;
    ListInstalledColormapsProcPtr ListInstalledColormaps;
    StoreColorsProcPtr          StoreColors;
    ResolveColorProcPtr         ResolveColor;

// this just keeps going and going

} ScreenRec;
typedef ScreenRec *ScreenPtr;

typedef int Pix24Flags;
typedef int MessageType;

typedef struct { 
	CARD32 red;
	CARD32 green;
	CARD32 blue; 
} rgb;
typedef struct { 
	float red;
	float green;
	float blue; 
} Gamma;

typedef struct {
    DisplayModeRec *    prev;
    DisplayModeRec *    next;
    char *                      name;
    ModeStatus                  status;
    int                         type;

    int                         Clock;
    int                         HDisplay;
    int                         HSyncStart;
    int                         HSyncEnd;
    int                         HTotal;
    int                         HSkew;
    int                         VDisplay;
    int                         VSyncStart;
    int                         VSyncEnd;
    int                         VTotal;
    int                         VScan;
    int                         Flags;

    int                         ClockIndex;
    int                         SynthClock;
    int                         CrtcHDisplay;
    int                         CrtcHBlankStart;
    int                         CrtcHSyncStart;
    int                         CrtcHSyncEnd;
    int                         CrtcHBlankEnd;
    int                         CrtcHTotal;
    int                         CrtcHSkew;
    int                         CrtcVDisplay;
    int                         CrtcVBlankStart;
    int                         CrtcVSyncStart;
    int                         CrtcVSyncEnd;
    int                         CrtcVBlankEnd;
    int                         CrtcVTotal;
    Bool                        CrtcHAdjusted;
    Bool                        CrtcVAdjusted;
    int                         PrivSize;
    INT32 *                     Private;
    int                         PrivFlags;

    float                       HSync;
	float						VRefresh;
} DisplayModeRec;
typedef DisplayModeRec *DisplayModePtr;


typedef struct {
    int                 driverVersion;
    char *              driverName;
                                   
    ScreenPtr           pScreen;
    int                 scrnIndex;
    Bool                configured;
    int                 origIndex;

    int                 imageByteOrder;
    int                 bitmapScanlineUnit;
    int                 bitmapScanlinePad;
    int                 bitmapBitOrder;
    int                 numFormats;
    PixmapFormatRec     formats[8];
    PixmapFormatRec     fbFormat;
    unsigned char pad;

    int                 bitsPerPixel;
    Pix24Flags          pixmap24;
    int                 depth;    
    MessageType         depthFrom;
    MessageType         bitsPerPixelFrom;
    rgb                 weight;
    rgb                 mask;
    rgb                 offset;
    int                 rgbBits;
    Gamma               gamma;
    int                 defaultVisual;
    int                 maxHValue;
    int                 maxVValue;
    int                 virtualX;
    int                 virtualY;
    int                 xInc;

    MessageType         virtualFrom;
    int                 displayWidth;
    int                 frameX0;
    int                 frameY0;
    int                 frameX1;
    int                 frameY1;
    int                 zoomLocked;
    DisplayModePtr      modePool;
    DisplayModePtr      modes;
    DisplayModePtr      currentMode;
    confScreenPtr       confScreen;
    MonPtr              monitor;
    DispPtr             display;
    int *               entityList;
    int                 numEntities;
    int                 widthmm;
    int                 heightmm;
    int                 xDpi;
    int                 yDpi;
    char *              name;
    pointer             driverPrivate;
    DevUnion *          privates;
    DriverPtr           drv;
    pointer             module;
    int                 colorKey;
    int                 overlayFlags;

    char *              chipset;
    char *              ramdac;
    char *              clockchip;
    Bool                progClock;
    int                 numClocks;
    int                 clock[128];
    int                 videoRam;
    unsigned long       biosBase;
    unsigned long       memPhysBase;
    unsigned long       fbOffset;
    IOADDRESS           domainIOBase;
    int                 memClk;
    int                 textClockFreq;
    Bool                flipPixels;
    pointer             options;

    int                 chipID;
    int                 chipRev;
    int                 racMemFlags;
    int                 racIoFlags;
    pointer             access;
    xf86CurrentAccessPtr CurrentAccess;
    resType             resourceType;
    pointer             busAccess;

    Bool                vtSema;
    DevUnion            pixmapPrivate;

    Bool                silkenMouse;

    ClockRangesPtr      clockRanges;
    int                 adjustFlags;

    int                 reservedInt[16];

    int *               entityInstanceList;
    pointer             reservedPtr[15];

    xf86ProbeProc                       *Probe;
    xf86PreInitProc                     *PreInit;
    xf86ScreenInitProc                  *ScreenInit;
    xf86SwitchModeProc                  *SwitchMode;
    xf86AdjustFrameProc                 *AdjustFrame;
    xf86EnterVTProc                     *EnterVT;
    xf86LeaveVTProc                     *LeaveVT;
    xf86FreeScreenProc                  *FreeScreen;
    xf86ValidModeProc                   *ValidMode;
    xf86EnableDisableFBAccessProc       *EnableDisableFBAccess;
    xf86SetDGAModeProc                  *SetDGAMode;
    xf86ChangeGammaProc                 *ChangeGamma;
    xf86PointerMovedProc                *PointerMoved;
    xf86PMEventProc                     *PMEvent;
    xf86HandleMessageProc               *HandleMessage;
    xf86DPMSSetProc                     *DPMSSet;
    xf86LoadPaletteProc                 *LoadPalette;
    xf86SetOverscanProc                 *SetOverscan;
    xorgRRFuncProc                      *RRFunc;

    funcPointer         reservedFuncs[11];
} ScrnInfoRec;
typedef ScrnInfoRec *ScrnInfoPtr;

ScrnInfoPtr xf86ConfigPciEntity(ScrnInfoPtr pScrn, int scrnFlag,
                                int entityIndex,PciChipsets *p_chip,
                                resList res, EntityProc init,
                                EntityProc enter, EntityProc leave,
                                pointer private);

int xf86GetLastScrnFlag(int entityIndex);
void xf86SetLastScrnFlag(int entityIndex, int scrnIndex);
Bool xf86IsEntityShared(int entityIndex);
void xf86SetEntityShared(int entityIndex);
Bool xf86IsEntitySharable(int entityIndex);
void xf86SetEntitySharable(int entityIndex);
Bool xf86IsPrimInitDone(int entityIndex);
void xf86SetPrimInitDone(int entityIndex);
void xf86ClearPrimInitDone(int entityIndex);
int xf86AllocateEntityPrivateIndex(void);
DevUnion *xf86GetEntityPrivate(int entityIndex, int privIndex);
void xf86SetEntityInstanceForScreen(ScrnInfoPtr pScrn, int entityIndex, int instance);
int xf86GetVerbosity(void);
void xf86DrvMsgVerb(int scrnIndex, MessageType type, int verb, const char *format, ...);
void xf86Msg(MessageType type, const char *format, ...);
pointer XNFcalloc(unsigned int n);
pointer Xcalloc(unsigned int n);
pointer Xalloc(unsigned int n);

// libc wrapper stuff
typedef int xf86size_t;
xf86size_t xf86strlen(const char *s);
int xf86vsnprintf(char *s, xf86size_t len, const char *format, va_list ap);
char *xf86strncpy(char *dest, const char *src, xf86size_t n);
int xf86isspace(int c);
