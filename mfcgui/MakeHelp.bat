@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by MFCGUI.HPJ. >"hlp\mfcgui.hm"
echo. >>"hlp\mfcgui.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\mfcgui.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\mfcgui.hm"
echo. >>"hlp\mfcgui.hm"
echo // Prompts (IDP_*) >>"hlp\mfcgui.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\mfcgui.hm"
echo. >>"hlp\mfcgui.hm"
echo // Resources (IDR_*) >>"hlp\mfcgui.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\mfcgui.hm"
echo. >>"hlp\mfcgui.hm"
echo // Dialogs (IDD_*) >>"hlp\mfcgui.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\mfcgui.hm"
echo. >>"hlp\mfcgui.hm"
echo // Frame Controls (IDW_*) >>"hlp\mfcgui.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\mfcgui.hm"
REM -- Make help for Project MFCGUI


echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\mfcgui.hpj"
if errorlevel 1 goto :Error
if not exist "hlp\mfcgui.hlp" goto :Error
if not exist "hlp\mfcgui.cnt" goto :Error
echo.
if exist Debug\nul copy "hlp\mfcgui.hlp" Debug
if exist Debug\nul copy "hlp\mfcgui.cnt" Debug
if exist Release\nul copy "hlp\mfcgui.hlp" Release
if exist Release\nul copy "hlp\mfcgui.cnt" Release
echo.
goto :done

:Error
echo hlp\mfcgui.hpj(1) : error: Problem encountered creating help file

:done
echo.
