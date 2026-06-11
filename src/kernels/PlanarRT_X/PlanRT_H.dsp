# Microsoft Developer Studio Project File - Name="PlanRT_H" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=PlanRT_H - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PlanRT_H.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PlanRT_H.mak" CFG="PlanRT_H - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PlanRT_H - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PlanRT_H - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
F90=df.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PlanRT_H - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /G6 /Zp16 /MT /W3 /vd0 /Ox /Ot /Og /Oy- /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DEBUG__1" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 sfile95.lib prepr.lib exprint.lib /nologo /subsystem:windows /machine:I386 /out:"c:\tmc\exe\tmc_rthf.exe"

!ELSEIF  "$(CFG)" == "PlanRT_H - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /G5 /MTd /W4 /GX /O2 /Oy- /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DEBUG__1" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 prepr.lib exprint.lib /nologo /subsystem:windows /debug /machine:I386 /out:"c:\tmc\exe\tmc_rth.exe"

!ENDIF 

# Begin Target

# Name "PlanRT_H - Win32 Release"
# Name "PlanRT_H - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\ChildFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\FieldIntegrated.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\PL_GLFUN.CPP
# End Source File
# Begin Source File

SOURCE=.\PL_IOFOR.CPP
# End Source File
# Begin Source File

SOURCE=.\PlanRT_H.cpp
# End Source File
# Begin Source File

SOURCE=.\PlanRT_H.rc
# End Source File
# Begin Source File

SOURCE=.\PlanRT_HDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\PlanRT_HView.cpp
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\TmcDialogStatistics.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_BolckList.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_DialogBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_DialogFormatOutFile.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_Indan.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_IndanOutput.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_IndanParam.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_IndanTopology.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_Input.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_InputNode.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTHNodeDiel.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcRTHRectNode.cpp
# End Source File
# Begin Source File

SOURCE=..\Tmcrtout\TmcSMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcSoundEffProp.cpp
# End Source File
# Begin Source File

SOURCE=.\TmcSoundMel1.cpp
# End Source File
# Begin Source File

SOURCE=..\Tmcrtout\TmcTtoS.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\ChildFrm.h
# End Source File
# Begin Source File

SOURCE=.\FieldIntegrated.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\Pl_iofor.h
# End Source File
# Begin Source File

SOURCE=.\PlanRT_H.h
# End Source File
# Begin Source File

SOURCE=.\PlanRT_HDoc.h
# End Source File
# Begin Source File

SOURCE=.\PlanRT_HView.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\TmcDialogStatistics.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_BolckList.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_DialogBlock.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_DialogFormatOutFile.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_Indan.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_IndanOutput.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_IndanParam.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_IndanTopology.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_Input.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTH_InputNode.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTHNodeDiel.h
# End Source File
# Begin Source File

SOURCE=.\TmcRTHRectNode.h
# End Source File
# Begin Source File

SOURCE=..\Tmcrtout\TmcSMatrix.h
# End Source File
# Begin Source File

SOURCE=.\TmcSoundEffProp.h
# End Source File
# Begin Source File

SOURCE=.\TmcSoundMel1.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RES\idr_main.ico
# End Source File
# Begin Source File

SOURCE=.\res\PlanRT_H.ico
# End Source File
# Begin Source File

SOURCE=.\res\PlanRT_H.rc2
# End Source File
# Begin Source File

SOURCE=.\res\PlanRT_HDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# End Target
# End Project
