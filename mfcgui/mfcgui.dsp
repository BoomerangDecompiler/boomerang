# Microsoft Developer Studio Project File - Name="mfcgui" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=mfcgui - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mfcgui.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mfcgui.mak" CFG="mfcgui - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mfcgui - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "mfcgui - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mfcgui - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "../include" /I "../loader" /I "../frontend" /I "../codegen" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "mfcgui - Win32 Release"
# Name "mfcgui - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "util"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\util\util.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Group "loader"

# PROP Default_Filter ""
# Begin Group "loader headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\loader\ExeBinaryFile.h
# End Source File
# Begin Source File

SOURCE=..\loader\HpSomBinaryFile.h
# End Source File
# Begin Source File

SOURCE=..\loader\PalmBinaryFile.h
# End Source File
# Begin Source File

SOURCE=..\loader\palmsystraps.h
# End Source File
# Begin Source File

SOURCE=..\loader\SymTab.h
# End Source File
# Begin Source File

SOURCE=..\loader\Win32BinaryFile.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\loader\BinaryFile.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\loader\ExeBinaryFile.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\loader\HpSomBinaryFile.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\loader\microX86dis.c

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\loader\PalmBinaryFile.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\loader\SymTab.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\loader\Win32BinaryFile.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Group "frontend"

# PROP Default_Filter ""
# Begin Group "frontend headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\frontend\pentiumdecoder.h
# End Source File
# Begin Source File

SOURCE=..\frontend\pentiumfrontend.h
# End Source File
# Begin Source File

SOURCE=..\frontend\sparcdecoder.h
# End Source File
# Begin Source File

SOURCE=..\frontend\sparcfrontend.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\frontend\frontend.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\frontend\njmcDecoder.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\frontend\pentiumdecoder.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\frontend\pentiumfrontend.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\frontend\sparcdecoder.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\frontend\sparcfrontend.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Group "db"

# PROP Default_Filter ""
# Begin Group "db headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\db\insnameelem.h
# End Source File
# Begin Source File

SOURCE=..\db\operstrings.h
# End Source File
# Begin Source File

SOURCE=..\db\sslparser.h
# End Source File
# Begin Source File

SOURCE=..\db\sslscanner.h
# End Source File
# Begin Source File

SOURCE=..\db\table.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\db\basicblock.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\cfg.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\dataflow.cpp
# End Source File
# Begin Source File

SOURCE=..\db\exp.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\hrtl.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\insnameelem.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\proc.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\prog.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\register.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\rtl.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\signature.cpp
# End Source File
# Begin Source File

SOURCE=..\db\sslinst.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\sslparser.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\sslscanner.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\table.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\db\type.cpp
# End Source File
# End Group
# Begin Group "codegen"

# PROP Default_Filter ""
# Begin Group "codegen headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\codegen\chllcode.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\codegen\chllcode.cpp

!IF  "$(CFG)" == "mfcgui - Win32 Release"

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# SUBTRACT CPP /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Group "analysis"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\analysis\analysis.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\CFGViewDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\EditBBDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\mfcgui.cpp
# End Source File
# Begin Source File

SOURCE=.\hlp\mfcgui.hpj

!IF  "$(CFG)" == "mfcgui - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__MFCGU="hlp\AfxCore.rtf"	"hlp\$(TargetName).hm"	
# Begin Custom Build - Making help file...
OutDir=.\Release
TargetName=mfcgui
InputPath=.\hlp\mfcgui.hpj
InputName=mfcgui

"$(OutDir)\$(InputName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	start /wait hcw /C /E /M "hlp\$(InputName).hpj" 
	if errorlevel 1 goto :Error 
	if not exist "hlp\$(InputName).hlp" goto :Error 
	copy "hlp\$(InputName).hlp" $(OutDir) 
	goto :done 
	:Error 
	echo hlp\$(InputName).hpj(1) : error: 
	type "hlp\$(InputName).log" 
	:done 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__MFCGU="hlp\AfxCore.rtf"	"hlp\$(TargetName).hm"	
# Begin Custom Build - Making help file...
OutDir=.\Debug
TargetName=mfcgui
InputPath=.\hlp\mfcgui.hpj
InputName=mfcgui

"$(OutDir)\$(InputName).hlp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	start /wait hcw /C /E /M "hlp\$(InputName).hpj" 
	if errorlevel 1 goto :Error 
	if not exist "hlp\$(InputName).hlp" goto :Error 
	copy "hlp\$(InputName).hlp" $(OutDir) 
	goto :done 
	:Error 
	echo hlp\$(InputName).hpj(1) : error: 
	type "hlp\$(InputName).log" 
	:done 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mfcgui.rc
# End Source File
# Begin Source File

SOURCE=.\NewProjectDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\NewSymbolDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\ProceduresDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcPropertiesDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ProcView.cpp
# End Source File
# Begin Source File

SOURCE=.\RenameVariableDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\SaveLoadDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\SymbolsDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewDecodeDialog.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "backend headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\BinaryFile.h
# End Source File
# Begin Source File

SOURCE=..\include\cfg.h
# End Source File
# Begin Source File

SOURCE=..\include\config.h
# End Source File
# Begin Source File

SOURCE=..\include\coverage.h
# End Source File
# Begin Source File

SOURCE=..\include\dataflow.h
# End Source File
# Begin Source File

SOURCE=..\include\decoder.h
# End Source File
# Begin Source File

SOURCE=..\include\exp.h
# End Source File
# Begin Source File

SOURCE=..\include\frontend.h
# End Source File
# Begin Source File

SOURCE=..\include\hllcode.h
# End Source File
# Begin Source File

SOURCE=..\include\operator.h
# End Source File
# Begin Source File

SOURCE=..\include\osfcn.h
# End Source File
# Begin Source File

SOURCE=..\include\proc.h
# End Source File
# Begin Source File

SOURCE=..\include\prog.h
# End Source File
# Begin Source File

SOURCE=..\include\register.h
# End Source File
# Begin Source File

SOURCE=..\include\rtl.h
# End Source File
# Begin Source File

SOURCE=..\include\signature.h
# End Source File
# Begin Source File

SOURCE=..\include\type.h
# End Source File
# Begin Source File

SOURCE=..\include\types.h
# End Source File
# Begin Source File

SOURCE=..\include\util.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\CFGViewDialog.h
# End Source File
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\EditBBDialog.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\mfcgui.h
# End Source File
# Begin Source File

SOURCE=.\NewProjectDialog.h
# End Source File
# Begin Source File

SOURCE=.\NewSymbolDialog.h
# End Source File
# Begin Source File

SOURCE=.\ProcDoc.h
# End Source File
# Begin Source File

SOURCE=.\ProceduresDialog.h
# End Source File
# Begin Source File

SOURCE=.\ProcPropertiesDialog.h
# End Source File
# Begin Source File

SOURCE=.\ProcView.h
# End Source File
# Begin Source File

SOURCE=.\RenameVariableDialog.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h

!IF  "$(CFG)" == "mfcgui - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Making help include file...
TargetName=mfcgui
InputPath=.\Resource.h

"hlp\$(TargetName).hm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo. >"hlp\$(TargetName).hm" 
	echo // Commands (ID_* and IDM_*) >>"hlp\$(TargetName).hm" 
	makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Prompts (IDP_*) >>"hlp\$(TargetName).hm" 
	makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Resources (IDR_*) >>"hlp\$(TargetName).hm" 
	makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Dialogs (IDD_*) >>"hlp\$(TargetName).hm" 
	makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Frame Controls (IDW_*) >>"hlp\$(TargetName).hm" 
	makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\$(TargetName).hm" 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Making help include file...
TargetName=mfcgui
InputPath=.\Resource.h

"hlp\$(TargetName).hm" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	echo. >"hlp\$(TargetName).hm" 
	echo // Commands (ID_* and IDM_*) >>"hlp\$(TargetName).hm" 
	makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Prompts (IDP_*) >>"hlp\$(TargetName).hm" 
	makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Resources (IDR_*) >>"hlp\$(TargetName).hm" 
	makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Dialogs (IDD_*) >>"hlp\$(TargetName).hm" 
	makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\$(TargetName).hm" 
	echo. >>"hlp\$(TargetName).hm" 
	echo // Frame Controls (IDW_*) >>"hlp\$(TargetName).hm" 
	makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\$(TargetName).hm" 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\SaveLoadDialog.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\SymbolsDialog.h
# End Source File
# Begin Source File

SOURCE=.\ViewDecodeDialog.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\arrow.cur
# End Source File
# Begin Source File

SOURCE=.\res\cursor1.cur
# End Source File
# Begin Source File

SOURCE=.\res\indicate.cur
# End Source File
# Begin Source File

SOURCE=".\res\mebright-small.bmp"
# End Source File
# Begin Source File

SOURCE=.\res\mfcgui.ico
# End Source File
# Begin Source File

SOURCE=.\res\mfcgui.rc2
# End Source File
# Begin Source File

SOURCE=.\res\ProcDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Group "Help Files"

# PROP Default_Filter "cnt;rtf"
# Begin Source File

SOURCE=.\hlp\AfxCore.rtf
# End Source File
# Begin Source File

SOURCE=.\hlp\AppExit.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Bullet.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw2.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurArw4.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\CurHelp.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCopy.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditCut.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditPast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\EditUndo.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileNew.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileOpen.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FilePrnt.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\FileSave.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpSBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\HlpTBar.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\mfcgui.cnt

!IF  "$(CFG)" == "mfcgui - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying contents file...
OutDir=.\Release
InputPath=.\hlp\mfcgui.cnt
InputName=mfcgui

"$(OutDir)\$(InputName).cnt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "hlp\$(InputName).cnt" $(OutDir)

# End Custom Build

!ELSEIF  "$(CFG)" == "mfcgui - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Copying contents file...
OutDir=.\Debug
InputPath=.\hlp\mfcgui.cnt
InputName=mfcgui

"$(OutDir)\$(InputName).cnt" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy "hlp\$(InputName).cnt" $(OutDir)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hlp\RecFirst.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecLast.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecNext.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\RecPrev.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmax.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\ScMenu.bmp
# End Source File
# Begin Source File

SOURCE=.\hlp\Scmin.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\mfcgui.reg
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\TODO.txt
# End Source File
# End Target
# End Project
