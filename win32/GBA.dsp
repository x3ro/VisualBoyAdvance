# Microsoft Developer Studio Project File - Name="GBA" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=GBA - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GBA.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GBA.mak" CFG="GBA - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GBA - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "GBA - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "GBA - Win32 ReleaseNoDev" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GBA - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gi /Zi /O2 /Ob2 /I "include\zlib" /I "include\png" /D "NDEBUG" /D "FINAL_VERSION" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MMX" /D "BKPT_SUPPORT" /D "DEV_VERSION" /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dxguid.lib dinput.lib winmm.lib comctl32.lib wsock32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"Release/VisualBoyAdvance.exe" /MAPINFO:EXPORTS /MAPINFO:LINES /OPT:ref

!ELSEIF  "$(CFG)" == "GBA - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /Gi /GX /ZI /Od /I "include\zlib" /I "include\png" /D "_DEBUG" /D "DEV_VERSION" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MMX" /D "BKPT_SUPPORT" /Fr /YX"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dxguid.lib dinput.lib winmm.lib comctl32.lib wsock32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /nodefaultlib:"LIBC" /out:"Debug/VisualBoyAdvance.exe" /MAPINFO:EXPORTS /MAPINFO:LINES

!ELSEIF  "$(CFG)" == "GBA - Win32 ReleaseNoDev"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "GBA___Win32_ReleaseNoDev"
# PROP BASE Intermediate_Dir "GBA___Win32_ReleaseNoDev"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseNoDev"
# PROP Intermediate_Dir "ReleaseNoDev"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gi /Zi /O2 /Ob2 /I "include\zlib" /I "include\png" /D "NDEBUG" /D "FINAL_VERSION" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MMX" /D "BKPT_SUPPORT" /D "DEV_VERSION" /YX"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /Gi /Zi /O2 /Ob2 /I "include\zlib" /I "include\png" /D "NDEBUG" /D "FINAL_VERSION" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "MMX" /D "BKPT_SUPPORT" /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dxguid.lib dinput.lib winmm.lib comctl32.lib wsock32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"Release/VisualBoyAdvance.exe" /MAPINFO:EXPORTS /MAPINFO:LINES /OPT:ref
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib dxguid.lib dinput.lib winmm.lib comctl32.lib wsock32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"ReleaseNoDev/VisualBoyAdvance.exe" /MAPINFO:EXPORTS /MAPINFO:LINES /OPT:ref

!ENDIF 

# Begin Target

# Name "GBA - Win32 Release"
# Name "GBA - Win32 Debug"
# Name "GBA - Win32 ReleaseNoDev"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "GBA"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\2xSaI.cpp
# End Source File
# Begin Source File

SOURCE=..\src\admame.cpp
# End Source File
# Begin Source File

SOURCE=..\src\armdis.cpp
# End Source File
# Begin Source File

SOURCE=..\src\bios.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Cheats.cpp
# End Source File
# Begin Source File

SOURCE=..\src\EEprom.cpp
# End Source File
# Begin Source File

SOURCE=..\src\elf.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Flash.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GBA.cpp

!IF  "$(CFG)" == "GBA - Win32 Release"

!ELSEIF  "$(CFG)" == "GBA - Win32 Debug"

# ADD CPP /W3

!ELSEIF  "$(CFG)" == "GBA - Win32 ReleaseNoDev"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\Gfx.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Globals.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Mode0.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Mode1.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Mode2.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Mode3.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Mode4.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Mode5.cpp
# End Source File
# Begin Source File

SOURCE=..\src\motionblur.cpp
# End Source File
# Begin Source File

SOURCE=..\src\pixel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\remote.cpp
# End Source File
# Begin Source File

SOURCE=..\src\simple2x.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Sound.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Sram.cpp
# End Source File
# Begin Source File

SOURCE=..\src\tvmode.cpp
# End Source File
# Begin Source File

SOURCE=..\src\unzip.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Util.cpp
# End Source File
# End Group
# Begin Group "GB"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\gb\GB.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gb\gbCheats.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gb\gbGfx.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gb\gbGlobals.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gb\gbMemory.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gb\gbPrinter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gb\gbSGB.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gb\gbSound.cpp
# End Source File
# End Group
# Begin Group "Windows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\win32\AboutDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\AccelEditor.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\AcceleratorManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\Associate.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\CmdAccelOb.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\Commands.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\CommDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\Controls.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\DirectoriesDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\Disassemble.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\ExportGSASnapshot.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\GBACheats.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\GBCheatsDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\GBColorDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\GBPrinterDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\GDBConnection.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\GSACodeSelect.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\Joypad.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\KeyboardEdit.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\LangSelect.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\Logging.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\MapView.cpp

!IF  "$(CFG)" == "GBA - Win32 Release"

!ELSEIF  "$(CFG)" == "GBA - Win32 Debug"

# ADD CPP /Gm /Gi

!ELSEIF  "$(CFG)" == "GBA - Win32 ReleaseNoDev"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\win32\MemoryViewer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\OamView.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\PaletteView.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\Reg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\ResizeDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\RomInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\ScrollWnd.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\stdafx.cpp

!IF  "$(CFG)" == "GBA - Win32 Release"

!ELSEIF  "$(CFG)" == "GBA - Win32 Debug"

# ADD CPP /Yc

!ELSEIF  "$(CFG)" == "GBA - Win32 ReleaseNoDev"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\win32\TileViewer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\vba.rc

!IF  "$(CFG)" == "GBA - Win32 Release"

!ELSEIF  "$(CFG)" == "GBA - Win32 Debug"

!ELSEIF  "$(CFG)" == "GBA - Win32 ReleaseNoDev"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\win32\wavwrite.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\Win32.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\WinResUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\src\win32\Wnd.cpp
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\win32\AccelEditor.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\AcceleratorManager.h
# End Source File
# Begin Source File

SOURCE=..\src\armdis.h
# End Source File
# Begin Source File

SOURCE=..\src\AutoBuild.h
# End Source File
# Begin Source File

SOURCE=..\src\Cheats.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\CmdAccelOb.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\Controls.h
# End Source File
# Begin Source File

SOURCE=..\src\debugger.h
# End Source File
# Begin Source File

SOURCE=..\src\EEprom.h
# End Source File
# Begin Source File

SOURCE=..\src\elf.h
# End Source File
# Begin Source File

SOURCE=..\src\expr.cpp.h
# End Source File
# Begin Source File

SOURCE=..\src\exprNode.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\FileDlg.h
# End Source File
# Begin Source File

SOURCE=..\src\Flash.h
# End Source File
# Begin Source File

SOURCE=..\src\GBA.h
# End Source File
# Begin Source File

SOURCE=..\src\GBAinline.h
# End Source File
# Begin Source File

SOURCE=..\src\Gfx.h
# End Source File
# Begin Source File

SOURCE=..\src\Globals.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\KeyboardEdit.h
# End Source File
# Begin Source File

SOURCE=..\src\NLS.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\Reg.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\ResizeDlg.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\resource.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\ScrollWnd.h
# End Source File
# Begin Source File

SOURCE=..\src\Sound.h
# End Source File
# Begin Source File

SOURCE=..\src\Sram.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\StdString.h
# End Source File
# Begin Source File

SOURCE=..\src\System.h
# End Source File
# Begin Source File

SOURCE=..\src\unzip.h
# End Source File
# Begin Source File

SOURCE=..\src\Util.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\wavwrite.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\WinResUtil.h
# End Source File
# Begin Source File

SOURCE=..\src\win32\Wnd.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\src\win32\gbadvance.ico
# End Source File
# End Group
# Begin Source File

SOURCE=..\src\i386\2xSaImmx.asm

!IF  "$(CFG)" == "GBA - Win32 Release"

# Begin Custom Build
OutDir=.\Release
InputPath=..\src\i386\2xSaImmx.asm
InputName=2xSaImmx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"c:\Program Files\Nasm\nasmw.exe" -D__DJGPP__ -f win32 -o $(OutDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "GBA - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
InputPath=..\src\i386\2xSaImmx.asm
InputName=2xSaImmx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"c:\Program Files\Nasm\nasmw.exe" -D__DJGPP__ -f win32 -o $(OutDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "GBA - Win32 ReleaseNoDev"

# Begin Custom Build
OutDir=.\ReleaseNoDev
InputPath=..\src\i386\2xSaImmx.asm
InputName=2xSaImmx

"$(OutDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	"c:\Program Files\Nasm\nasmw.exe" -D__DJGPP__ -f win32 -o $(OutDir)\$(InputName).obj $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\VisualBoyAdvance.exe.manifest
# End Source File
# Begin Source File

SOURCE=..\src\win32\VisualBoyAdvance.exe.manifest
# End Source File
# Begin Source File

SOURCE=lib\win32\libpng.lib
# End Source File
# Begin Source File

SOURCE=lib\win32\zlib.lib
# End Source File
# End Target
# End Project
