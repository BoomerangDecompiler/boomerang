# Microsoft Developer Studio Project File - Name="boomerang" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=boomerang - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "boomerang_VC5.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "boomerang_VC5.mak" CFG="boomerang - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "boomerang - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "boomerang - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "boomerang - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "boomerang - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /I "include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "boomerang - Win32 Release"
# Name "boomerang - Win32 Debug"
# Begin Group "source files"

# PROP Default_Filter "*.c*"
# Begin Source File

SOURCE=.\analysis\analysis.cpp
# ADD CPP /GR-
# End Source File
# Begin Source File

SOURCE=".\c\ansi-c-parser.cpp"
# ADD CPP /D "_MSDOS"
# SUBTRACT CPP /YX
# End Source File
# Begin Source File

SOURCE=".\c\ansi-c-scanner.cpp"
# End Source File
# Begin Source File

SOURCE=.\db\basicblock.cpp
# End Source File
# Begin Source File

SOURCE=.\loader\BinaryFile.cpp
# End Source File
# Begin Source File

SOURCE=.\boomerang.cpp
# ADD CPP /GR
# End Source File
# Begin Source File

SOURCE=.\db\cfg.cpp
# ADD CPP /GR
# End Source File
# Begin Source File

SOURCE=.\codegen\chllcode.cpp
# ADD CPP /GR
# End Source File
# Begin Source File

SOURCE=.\type\constraint.cpp
# End Source File
# Begin Source File

SOURCE=.\include\constraint.h
# End Source File
# Begin Source File

SOURCE=.\driver.cpp
# End Source File
# Begin Source File

SOURCE=.\loader\ElfBinaryFile.cpp
# End Source File
# Begin Source File

SOURCE=.\loader\ExeBinaryFile.cpp
# End Source File
# Begin Source File

SOURCE=.\db\exp.cpp
# End Source File
# Begin Source File

SOURCE=.\frontend\frontend.cpp
# ADD CPP /GR /I "c"
# End Source File
# Begin Source File

SOURCE=.\loader\HpSomBinaryFile.cpp
# End Source File
# Begin Source File

SOURCE=.\db\insnameelem.cpp
# End Source File
# Begin Source File

SOURCE=.\log.cpp
# End Source File
# Begin Source File

SOURCE=.\db\managed.cpp
# End Source File
# Begin Source File

SOURCE=.\loader\microX86dis.c
# End Source File
# Begin Source File

SOURCE=.\frontend\njmcDecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\loader\PalmBinaryFile.cpp
# End Source File
# Begin Source File

SOURCE=.\frontend\pentiumdecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\frontend\pentiumfrontend.cpp
# End Source File
# Begin Source File

SOURCE=.\db\proc.cpp
# End Source File
# Begin Source File

SOURCE=.\db\prog.cpp
# End Source File
# Begin Source File

SOURCE=.\db\register.cpp
# End Source File
# Begin Source File

SOURCE=.\db\rtl.cpp
# End Source File
# Begin Source File

SOURCE=.\db\signature.cpp
# End Source File
# Begin Source File

SOURCE=.\frontend\sparcdecoder.cpp
# End Source File
# Begin Source File

SOURCE=.\frontend\sparcfrontend.cpp
# End Source File
# Begin Source File

SOURCE=.\db\sslinst.cpp
# End Source File
# Begin Source File

SOURCE=.\db\sslparser.cpp
# End Source File
# Begin Source File

SOURCE=.\db\sslscanner.cpp
# End Source File
# Begin Source File

SOURCE=.\db\statement.cpp
# End Source File
# Begin Source File

SOURCE=.\loader\SymTab.cpp
# End Source File
# Begin Source File

SOURCE=.\db\table.cpp
# End Source File
# Begin Source File

SOURCE=.\type\type.cpp
# End Source File
# Begin Source File

SOURCE=.\util\util.cpp
# End Source File
# Begin Source File

SOURCE=.\loader\Win32BinaryFile.cpp
# End Source File
# End Group
# Begin Group "header files"

# PROP Default_Filter "*.h"
# Begin Source File

SOURCE=.\include\analysis.h
# End Source File
# Begin Source File

SOURCE=".\c\ansi-c-parser.h"
# End Source File
# Begin Source File

SOURCE=".\c\ansi-c-scanner.h"
# End Source File
# Begin Source File

SOURCE=.\include\BinaryFile.h
# End Source File
# Begin Source File

SOURCE=.\include\BinaryFileStub.h
# End Source File
# Begin Source File

SOURCE=.\include\boomerang.h
# End Source File
# Begin Source File

SOURCE=.\include\cfg.h
# End Source File
# Begin Source File

SOURCE=.\codegen\chllcode.h
# End Source File
# Begin Source File

SOURCE=.\include\config.h
# End Source File
# Begin Source File

SOURCE=.\include\decoder.h
# End Source File
# Begin Source File

SOURCE=.\loader\ElfBinaryFile.h
# End Source File
# Begin Source File

SOURCE=.\loader\ExeBinaryFile.h
# End Source File
# Begin Source File

SOURCE=.\include\exp.h
# End Source File
# Begin Source File

SOURCE=.\include\exphelp.h
# End Source File
# Begin Source File

SOURCE=.\include\frontend.h
# End Source File
# Begin Source File

SOURCE=.\include\hllcode.h
# End Source File
# Begin Source File

SOURCE=.\loader\HpSomBinaryFile.h
# End Source File
# Begin Source File

SOURCE=.\include\log.h
# End Source File
# Begin Source File

SOURCE=.\include\managed.h
# End Source File
# Begin Source File

SOURCE=.\include\operator.h
# End Source File
# Begin Source File

SOURCE=.\db\operstrings.h
# End Source File
# Begin Source File

SOURCE=.\include\osfcn.h
# End Source File
# Begin Source File

SOURCE=.\loader\PalmBinaryFile.h
# End Source File
# Begin Source File

SOURCE=.\frontend\pentiumdecoder.h
# End Source File
# Begin Source File

SOURCE=.\frontend\pentiumfrontend.h
# End Source File
# Begin Source File

SOURCE=.\include\proc.h
# End Source File
# Begin Source File

SOURCE=.\include\prog.h
# End Source File
# Begin Source File

SOURCE=.\include\register.h
# End Source File
# Begin Source File

SOURCE=.\include\rtl.h
# End Source File
# Begin Source File

SOURCE=.\include\sigenum.h
# End Source File
# Begin Source File

SOURCE=.\include\signature.h
# End Source File
# Begin Source File

SOURCE=.\frontend\sparcdecoder.h
# End Source File
# Begin Source File

SOURCE=.\frontend\sparcfrontend.h
# End Source File
# Begin Source File

SOURCE=.\db\sslparser.h
# End Source File
# Begin Source File

SOURCE=.\db\sslscanner.h
# End Source File
# Begin Source File

SOURCE=.\include\statement.h
# End Source File
# Begin Source File

SOURCE=.\loader\SymTab.h
# End Source File
# Begin Source File

SOURCE=.\db\table.h
# End Source File
# Begin Source File

SOURCE=.\include\type.h
# End Source File
# Begin Source File

SOURCE=.\include\types.h
# End Source File
# Begin Source File

SOURCE=.\include\util.h
# End Source File
# Begin Source File

SOURCE=.\loader\Win32BinaryFile.h
# End Source File
# End Group
# End Target
# End Project
