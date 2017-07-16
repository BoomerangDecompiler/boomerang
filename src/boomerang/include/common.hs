#
# Copyright (C) 1999-2000, The University of Queensland
# Copyright (C) 2000-2001, Sun Microsystems, Inc
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

################################################################################
# FILE:     common.hs
# OVERVIEW: "Simplified header" file for commonly used library functions
#   From this file, the proc object can construct a map from function name to
#   compact parameter information.
################################################################################

# $Revision$
# Format: <function name> <return type> <parameter type> ...
# where the function mame is just the name in ascii. All types are one of these:
#  v    void (return type only)
#  i    integer (includes long, time_t etc)
#  pd    pointer to data (struct*, char*, float*, char*[], int[], etc)
#  fs    floating point (note: no single precision floats are usually passed)
#  fd    floating point double (no single precision floats are usually passed)
#  pf    pointer to a function
#  ...    var args
#
# 22 Feb 00 - Cristina: added common PalmOS system function signatures
# 14-21 Mar 00 - Mike: Changed short parameters to "s"
# 31 Mar 00 - Mike: Removed "sysTrap" from Palm function names
# 13 Apr 00 - Mike: Strict return types now (including "b" for byte)
# 05 Sep 00 - Mike: Added entries for RayTracer benchmark
# 10 Jan 01 - Brian: Added atan2, fileno for perl benchmark
# 22 Apr 01 - Brian: Added X and Xt entries for Xlogo program

# <string.h>
atof fd pd
atoi i pd
atol i pd
strcat pd pd pd
strchr pd pd i
strcmp i pd pd
strcpy pd pd pd
strdup pd pd
strlen i pd
strncmp i pd pd i
strncpy pd pd pd i
strpbrk pd pd pd
strrchr pd pd i
strstr pd pd pd
strerror pd i

# <unistd.h>
_exit v i
_sysconf i i
ioctl i i i ...
readlink i pd pd i

# sys/stat.h
stat64 i pd pd
lstat64 i pd pd
fstat64 i i pd

atan fd fd
atan2 fd fd fd
atexit i pf
bsearch pd pd pd i i pf
calloc pd i i
chdir i pd
chmod i pd i
clock i
close i i
cos fd fd
ctime pd pd
dup i i
execl i pd pd ...
execv i pd pd
execvp pd pd
exit v i
exp fd fd
fclose i pd
fdopen pd i pd
fflush i pd
fgets pd pd i pd
fileno i pd
fopen pd pd pd
fprintf i pd pd ...
fputc i i pd
fputs i pd pd
fread i pd i i pd
free v pd
fseek i pd i i
fstat i pd pd
ftell i pd
fwrite i pd i i pd
getcwd pd pd i
getenv pd pd
getopt i i pd pd
getpid i
getrlimit i i pd
gmtime pd pd pd
isatty i i
localtime pd pd
log fd fd
longjmp v pd i
lseek i i i i
malloc pd i
memcmp i pd pd i
memcpy pd pd pd i
memmove pd pd pd i
memset pd pd i i
mktemp pd pd
modf fd fd pd
open i pd i ...
perror v pd
pow fd fd fd
printf i pd ...
puts i pd
qsort v pd i i pf
rand i
read i i pd i
realloc pd pd i
sbrk pd i
scanf i pd ...
setjmp i pd
setrlimit i i pd
signal pd i pf
sin fd fd
sprintf i pd pd ...
sqrt fd fd
srand v i
sscanf i pd pd ...
stat i pd pd
system i pd
tan fd fd
time i pd
times i pd
tolower i i
toupper i i
ungetc i i pd
unlink i pd
utime i pd pd
vfprintf i pd pd pd
vprintf i pd pd
write i i pd i

# <locale.h>
setlocale pd i pd
localeconv pd

# <libintl.h>
gettext pd pd
dgettext pd pd pd
dcgettext pd pd pd i
textdomain pd pd
bindtextdomain pd pd pd

# <getwidth.h>
getwidth v pd

# <stdlib.h>
mbstowcs i pd pd i
mbtowc i pd pd i
abort v
abs i i
labs i i
wcstombs i pd pd i

# <wchar.h>
wcscat pd pd pd
wscat pd pd pd
wcsncat pd pd pd i
wsncat pd pd pd
wcscmp i pd pd
wscmp i pd pd
wcsncmp i pd pd i
wsncmp i pd pd i
wcscpy pd pd pd
wscpy pd pd pd
wcsncpy pd pd pd i
wsncpy pd pd pd i
wcslen i pd
wslen i pd
wcschr pd pd i
wschr pd ps i
wcsrchr pd pd i
wsrchr pd pd i
windex pd pd i
wrindex pd pd i
wcspbrk pd pd pd
wspbrk pd pd pd
wcswcs pd pd pd
wcsspn i pd pd
wsspn i pd pd
_iswctype i i i
wstod fd pd pd
iswdigit i i
iswspace i i
fputwc i i pd
fgetwc i pd
wcwidth i i

# <stdio.h>
__flsbuf i i pd
_flsbuf i i pd
putc i i pd
__filbuf i pd
_filbuf i pd
fopen64 pd pd pd
pclose pd pd

# <libelf.h>
elf_version i i
elf_begin pd i i pd
elf_end i pd
elf_next i pd
elf_kind i pd
elf_getarhdr pd pd
elf_getbase i pd
elf32_getehdr pd pd
elf32_newehdr pd pd
elf_getscn pd pd i
elf_getdata pd pd pd
elf_nextscn pd pd pd
elf32_getshdr pd pd
elf_strptr pd pd i i

# <ctype.h>
isalpha i i
isdigit i i
isupper i i
islower i i

# time.h
cftime i pd pd pd

# pwd.h
getpwuid i i pd pd i pd

# grp.h
getgrgid pd i

# sys/acl.h
acl i pd i i pd

# <[n]curses.h>
initscr pd
noecho i
nonl i
wrefresh i pd
has_colors i
start_color i
use_default_colors i
curs_set i i
newwin pd i i i i
wmove i pd i i
move i i i
waddnstr i pd pd i
waddch i pd i
cbreak i
nodelay i pd
wclear i pd
werase i pd
wtouchln i pd i i i
overlay i pd pd
napms i i
wgetch i pd
beep i
endwin i
init_pair i i i i
wattr_off i pd i
wattr_on i pd i
mvwin i pd i i
nl i
wtimeout i pd i
mvprintw i i i pd ...
wnoutrefresh i pd
doupdate i
wprintw i pd pd ...
clearok i pd i
getmouse i pd
ungetch i i
scrollok i pd i
keypad i pd i
mousemask i i pd

# <langinfo.h>
nl_langinfo pd i

# C++ functions
# (a) GNU's g++
__ls__7ostreamPCc pd pd pd
__ls__7ostreamd pd pd fd
__ls__7ostreami pd pd i
__builtin_new pd i

# (b) Sun's CC
__0fOunsafe_ostreamGoutstrPCcTB pd pd pd pd
__0OnwUi pd i
__0oOunsafe_ostreamlsl pd pd i
__0FEendlR6Hostream pd pd
__0oOunsafe_ostreamlsd pd pd fd
__0oNIostream_initctv v pd
__0oNIostream_initdtv v pd
# Note "hidden" last parameter below
__0oKistrstreamctPCc v pd pd i
__0oHistreamrsPc pd pd pd
# Destructor takes 2nd arg (below)
__0oKistrstreamdtv pd pd i
__0oHistreamrsRf pd pd pd
__0oHistreamrsRi pd pd pd
__0oHistreamrsRUl pd pd pd
__0oHistreamrsRl pd pd pd
_vector_con_ v pd i i pf i
__ftou i fs
__dtou i fd
__0OdlPv v pd
_ex_rethrow_q v


##### The following are system functions for the PalmOS
## User interface API
# Form functions
FrmAlert s s
FrmCloseAllForms v
FrmDispatchEvent i pd
FrmDrawForm v pd
FrmGetActiveForm pd
FrmGotoForm v s
FrmInitForm pd s
FrmSetActiveForm v pd
FrmSetEventHandler v pd pf

# System manager
SysHandleEvent b pd
SysUIAppSwitch s s i s pd
SysAppExit v pd pd pd
SysAppStartup s pd pd pd
SysAppLaunch i s i s s pd pd
SysInsertionSort v pd s s pf i
MemPtrFree i pd
MemHandleUnlock i pd

# Memory manager
MemHandleLock pd i
MemPtrSize i pd

# Lists
LstSetSelection v pd s

# Controls
CtlSetLabel v pd pd

## Data and resource manager
# Data manager functions
DmGetNextDatabaseByTypeCreator i s pd i i s pd pd
DmGetResource pd i s
DmReleaseResource i pd

# Menus
MenuEraseStatus v pd
MenuHandleEvent b pd pd pd

# Error manager
ErrDisplayFileLineMsg v pd s pd

# System event manager
EvtGetEvent v pd i

# Preferences
PrefGetAppPreferences s i s pd pd s
PrefSetAppPreferences v i s s pd s s
PrefGetPreference i s
PrefSetPreference v s i

# String Manager
StrCompare i pd pd
StrCat pd pd pd
StrChr pd pd s

# Feature manager
FtrGet s i s pd
FtrSet s i s pd

# Next are guesses
AbtShowAbout v i


##### The following are library functions for X and Xt (X toolkit)
XBell                    v   pd i
XChangeWindowAttributes  v   pd i i pd
XCloseDisplay            v   pd     
XCreateGC                pd  pd i i pd
XCreatePixmap            i   pd i i i i
XFreeGC                  v   pd pd
XFreePixmap              v   pd i
XInternAtom              i   pd pd i
XSetWMProtocols          i   pd i pd i    
XShapeCombineMask        v   pd i i i i i i   
XShapeQueryExtension     i   pd pd pd

XmuDrawLogo              v   pd i pd pd i i i i 

XtAddCallback            v   pd pd pf pd
XtAppAddActions          v   pd pd i
XtAppMainLoop            v   pd          
XtCreateManagedWidget    pd  pd pd pd pd i
XtDisplay                pd  pd
XtGetGC                  pd  pd i pd
XtGetValues              v   pd pd i
XtOpenApplication        pd  pd pd pd i pd pd pd pd pd i
XtOverrideTranslations   v   pd pd
XtParseTranslationTable  pd  pd
XtRealizeWidget          v   pd
XtReleaseGC              v   pd pd
XtSetValues              v   pd pd i
XtWidgetToApplicationContext pd  pd 
XtWindow                 i   pd
XtWindowOfObject         i   pd

_XtInherit               pf  v

SmcCloseConnection       i   pd i pd
