# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=PlanRT_H - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to PlanRT_H - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PlanRT_H - Win32 Release" && "$(CFG)" !=\
 "PlanRT_H - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "PlanRT_H.mak" CFG="PlanRT_H - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PlanRT_H - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "PlanRT_H - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "PlanRT_H - Win32 Debug"
CPP=cl.exe
MTL=mktyplib.exe
RSC=rc.exe

!IF  "$(CFG)" == "PlanRT_H - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "c:\tmc\exe\tmc_rth.exe"

CLEAN : 
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\PL_GLFUN.OBJ"
	-@erase "$(INTDIR)\PL_IOFOR.OBJ"
	-@erase "$(INTDIR)\PlanRT_H.obj"
	-@erase "$(INTDIR)\PlanRT_H.pch"
	-@erase "$(INTDIR)\PlanRT_H.res"
	-@erase "$(INTDIR)\PlanRT_HDoc.obj"
	-@erase "$(INTDIR)\PlanRT_HView.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TmcDialogStatistics.obj"
	-@erase "$(INTDIR)\TmcLibError.obj"
	-@erase "$(INTDIR)\TmcRTH_BolckList.obj"
	-@erase "$(INTDIR)\TmcRTH_DialogBlock.obj"
	-@erase "$(INTDIR)\TmcRTH_DialogFormatOutFile.obj"
	-@erase "$(INTDIR)\TmcRTH_Indan.obj"
	-@erase "$(INTDIR)\TmcRTH_IndanOutput.obj"
	-@erase "$(INTDIR)\TmcRTH_IndanParam.obj"
	-@erase "$(INTDIR)\TmcRTH_IndanTopology.obj"
	-@erase "$(INTDIR)\TmcRTH_Input.obj"
	-@erase "$(INTDIR)\TmcRTH_InputNode.obj"
	-@erase "$(INTDIR)\TmcRTHNodeDiel.obj"
	-@erase "$(INTDIR)\TmcRTHRectNode.obj"
	-@erase "c:\tmc\exe\tmc_rth.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /G5 /MT /W4 /GX /O2 /Oy- /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DEBUG__1" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /G5 /MT /W4 /GX /O2 /Oy- /D "NDEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_MBCS" /D "DEBUG__1" /Fp"$(INTDIR)/PlanRT_H.pch" /Yu"stdafx.h"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
RSC_PROJ=/l 0x419 /fo"$(INTDIR)/PlanRT_H.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PlanRT_H.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 prepr.lib exprint.lib /nologo /subsystem:windows /machine:I386 /out:"c:\tmc\exe\tmc_rth.exe"
LINK32_FLAGS=prepr.lib exprint.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/tmc_rth.pdb" /machine:I386 /out:"c:\tmc\exe\tmc_rth.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\PL_GLFUN.OBJ" \
	"$(INTDIR)\PL_IOFOR.OBJ" \
	"$(INTDIR)\PlanRT_H.obj" \
	"$(INTDIR)\PlanRT_H.res" \
	"$(INTDIR)\PlanRT_HDoc.obj" \
	"$(INTDIR)\PlanRT_HView.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TmcDialogStatistics.obj" \
	"$(INTDIR)\TmcLibError.obj" \
	"$(INTDIR)\TmcRTH_BolckList.obj" \
	"$(INTDIR)\TmcRTH_DialogBlock.obj" \
	"$(INTDIR)\TmcRTH_DialogFormatOutFile.obj" \
	"$(INTDIR)\TmcRTH_Indan.obj" \
	"$(INTDIR)\TmcRTH_IndanOutput.obj" \
	"$(INTDIR)\TmcRTH_IndanParam.obj" \
	"$(INTDIR)\TmcRTH_IndanTopology.obj" \
	"$(INTDIR)\TmcRTH_Input.obj" \
	"$(INTDIR)\TmcRTH_InputNode.obj" \
	"$(INTDIR)\TmcRTHNodeDiel.obj" \
	"$(INTDIR)\TmcRTHRectNode.obj"

"c:\tmc\exe\tmc_rth.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PlanRT_H - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "c:\tmc\exe\tmc_rth.exe"

CLEAN : 
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\PL_GLFUN.OBJ"
	-@erase "$(INTDIR)\PL_IOFOR.OBJ"
	-@erase "$(INTDIR)\PlanRT_H.obj"
	-@erase "$(INTDIR)\PlanRT_H.pch"
	-@erase "$(INTDIR)\PlanRT_H.res"
	-@erase "$(INTDIR)\PlanRT_HDoc.obj"
	-@erase "$(INTDIR)\PlanRT_HView.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TmcDialogStatistics.obj"
	-@erase "$(INTDIR)\TmcLibError.obj"
	-@erase "$(INTDIR)\TmcRTH_BolckList.obj"
	-@erase "$(INTDIR)\TmcRTH_DialogBlock.obj"
	-@erase "$(INTDIR)\TmcRTH_DialogFormatOutFile.obj"
	-@erase "$(INTDIR)\TmcRTH_Indan.obj"
	-@erase "$(INTDIR)\TmcRTH_IndanOutput.obj"
	-@erase "$(INTDIR)\TmcRTH_IndanParam.obj"
	-@erase "$(INTDIR)\TmcRTH_IndanTopology.obj"
	-@erase "$(INTDIR)\TmcRTH_Input.obj"
	-@erase "$(INTDIR)\TmcRTH_InputNode.obj"
	-@erase "$(INTDIR)\TmcRTHNodeDiel.obj"
	-@erase "$(INTDIR)\TmcRTHRectNode.obj"
	-@erase "$(OUTDIR)\tmc_rth.pdb"
	-@erase "c:\tmc\exe\tmc_rth.exe"
	-@erase "c:\tmc\exe\tmc_rth.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /G5 /MTd /W4 /GX /O2 /Oy- /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "DEBUG__1" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /G5 /MTd /W4 /GX /O2 /Oy- /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_MBCS" /D "DEBUG__1" /Fp"$(INTDIR)/PlanRT_H.pch" /Yu"stdafx.h"\
 /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
RSC_PROJ=/l 0x419 /fo"$(INTDIR)/PlanRT_H.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PlanRT_H.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 prepr.lib exprint.lib /nologo /subsystem:windows /debug /machine:I386 /out:"c:\tmc\exe\tmc_rth.exe"
LINK32_FLAGS=prepr.lib exprint.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/tmc_rth.pdb" /debug /machine:I386 /out:"c:\tmc\exe\tmc_rth.exe"\
 
LINK32_OBJS= \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\PL_GLFUN.OBJ" \
	"$(INTDIR)\PL_IOFOR.OBJ" \
	"$(INTDIR)\PlanRT_H.obj" \
	"$(INTDIR)\PlanRT_H.res" \
	"$(INTDIR)\PlanRT_HDoc.obj" \
	"$(INTDIR)\PlanRT_HView.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TmcDialogStatistics.obj" \
	"$(INTDIR)\TmcLibError.obj" \
	"$(INTDIR)\TmcRTH_BolckList.obj" \
	"$(INTDIR)\TmcRTH_DialogBlock.obj" \
	"$(INTDIR)\TmcRTH_DialogFormatOutFile.obj" \
	"$(INTDIR)\TmcRTH_Indan.obj" \
	"$(INTDIR)\TmcRTH_IndanOutput.obj" \
	"$(INTDIR)\TmcRTH_IndanParam.obj" \
	"$(INTDIR)\TmcRTH_IndanTopology.obj" \
	"$(INTDIR)\TmcRTH_Input.obj" \
	"$(INTDIR)\TmcRTH_InputNode.obj" \
	"$(INTDIR)\TmcRTHNodeDiel.obj" \
	"$(INTDIR)\TmcRTHRectNode.obj"

"c:\tmc\exe\tmc_rth.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "PlanRT_H - Win32 Release"
# Name "PlanRT_H - Win32 Debug"

!IF  "$(CFG)" == "PlanRT_H - Win32 Release"

!ELSEIF  "$(CFG)" == "PlanRT_H - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "PlanRT_H - Win32 Release"

!ELSEIF  "$(CFG)" == "PlanRT_H - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\PlanRT_H.cpp
DEP_CPP_PLANR=\
	".\ChildFrm.h"\
	".\MainFrm.h"\
	".\PlanRT_H.h"\
	".\PlanRT_HDoc.h"\
	".\PlanRT_HView.h"\
	".\StdAfx.h"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TmcRTHNodeDiel.h"\
	".\TmcRTHRectNode.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\PlanRT_H.obj" : $(SOURCE) $(DEP_CPP_PLANR) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "PlanRT_H - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /G5 /MT /W4 /GX /O2 /Oy- /D "NDEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_MBCS" /D "DEBUG__1" /Fp"$(INTDIR)/PlanRT_H.pch" /Yc"stdafx.h"\
 /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\PlanRT_H.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "PlanRT_H - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /G5 /MTd /W4 /GX /O2 /Oy- /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /D "_MBCS" /D "DEBUG__1" /Fp"$(INTDIR)/PlanRT_H.pch" /Yc"stdafx.h"\
 /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\PlanRT_H.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
	".\MainFrm.h"\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ChildFrm.cpp
DEP_CPP_CHILD=\
	".\ChildFrm.h"\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	

"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PlanRT_HDoc.cpp
DEP_CPP_PLANRT=\
	".\MainFrm.h"\
	".\PlanRT_H.h"\
	".\PlanRT_HDoc.h"\
	".\StdAfx.h"\
	".\TMCGrExpression.h"\
	".\TMCGRVIW.H"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TmcRTHNodeDiel.h"\
	".\TmcRTHRectNode.h"\
	".\TYPEDEF.H"\
	".\TYPERTH.H"\
	

"$(INTDIR)\PlanRT_HDoc.obj" : $(SOURCE) $(DEP_CPP_PLANRT) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PlanRT_HView.cpp
DEP_CPP_PLANRT_=\
	".\MainFrm.h"\
	".\Pl_iofor.h"\
	".\PlanRT_H.h"\
	".\PlanRT_HDoc.h"\
	".\PlanRT_HView.h"\
	".\StdAfx.h"\
	".\TmcDialogStatistics.h"\
	".\TMCGrExpression.h"\
	".\TMCGRVIW.H"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_DialogFormatOutFile.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TmcRTHNodeDiel.h"\
	".\TmcRTHRectNode.h"\
	".\TYPEDEF.H"\
	".\TYPERTH.H"\
	

"$(INTDIR)\PlanRT_HView.obj" : $(SOURCE) $(DEP_CPP_PLANRT_) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PlanRT_H.rc
DEP_RSC_PLANRT_H=\
	".\res\PlanRT_H.ico"\
	".\res\PlanRT_H.rc2"\
	".\res\PlanRT_HDoc.ico"\
	".\res\Toolbar.bmp"\
	

"$(INTDIR)\PlanRT_H.res" : $(SOURCE) $(DEP_RSC_PLANRT_H) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTHRectNode.cpp

!IF  "$(CFG)" == "PlanRT_H - Win32 Release"

DEP_CPP_TMCRT=\
	".\Pl_iofor.h"\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TmcRTHNodeDiel.h"\
	".\TmcRTHRectNode.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	{$(INCLUDE)}"\typerth.h"\
	

"$(INTDIR)\TmcRTHRectNode.obj" : $(SOURCE) $(DEP_CPP_TMCRT) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


!ELSEIF  "$(CFG)" == "PlanRT_H - Win32 Debug"

DEP_CPP_TMCRT=\
	".\Pl_iofor.h"\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TmcRTHNodeDiel.h"\
	".\TmcRTHRectNode.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	{$(INCLUDE)}"\typerth.h"\
	

"$(INTDIR)\TmcRTHRectNode.obj" : $(SOURCE) $(DEP_CPP_TMCRT) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTHNodeDiel.cpp
DEP_CPP_TMCRTH=\
	".\expr.h"\
	".\Pl_iofor.h"\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TmcRTHNodeDiel.h"\
	".\TmcRTHRectNode.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\TmcRTHNodeDiel.obj" : $(SOURCE) $(DEP_CPP_TMCRTH) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTH_Input.cpp
DEP_CPP_TMCRTH_=\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcRTH_Input.h"\
	".\TmcRTH_InputNode.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\TmcRTH_Input.obj" : $(SOURCE) $(DEP_CPP_TMCRTH_) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTH_InputNode.cpp
DEP_CPP_TMCRTH_I=\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcRTH_InputNode.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\TmcRTH_InputNode.obj" : $(SOURCE) $(DEP_CPP_TMCRTH_I) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcLibError.cpp
DEP_CPP_TMCLI=\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcLibError.h"\
	

"$(INTDIR)\TmcLibError.obj" : $(SOURCE) $(DEP_CPP_TMCLI) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTH_Indan.cpp
DEP_CPP_TMCRTH_IN=\
	".\PlanRT_H.h"\
	".\prepr1.h"\
	".\StdAfx.h"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\TmcRTH_Indan.obj" : $(SOURCE) $(DEP_CPP_TMCRTH_IN) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTH_IndanParam.cpp
DEP_CPP_TMCRTH_IND=\
	".\expr.h"\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\TmcRTH_IndanParam.obj" : $(SOURCE) $(DEP_CPP_TMCRTH_IND) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTH_IndanOutput.cpp
DEP_CPP_TMCRTH_INDA=\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\TmcRTH_IndanOutput.obj" : $(SOURCE) $(DEP_CPP_TMCRTH_INDA)\
 "$(INTDIR)" "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTH_IndanTopology.cpp
DEP_CPP_TMCRTH_INDAN=\
	".\expr.h"\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\TmcRTH_IndanTopology.obj" : $(SOURCE) $(DEP_CPP_TMCRTH_INDAN)\
 "$(INTDIR)" "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTH_BolckList.cpp
DEP_CPP_TMCRTH_B=\
	".\expr.h"\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\TmcRTH_BolckList.obj" : $(SOURCE) $(DEP_CPP_TMCRTH_B) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcDialogStatistics.cpp
DEP_CPP_TMCDI=\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcDialogStatistics.h"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_DialogBlock.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TmcRTHNodeDiel.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\TmcDialogStatistics.obj" : $(SOURCE) $(DEP_CPP_TMCDI) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTH_DialogBlock.cpp
DEP_CPP_TMCRTH_D=\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_DialogBlock.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TYPERTH.H"\
	

"$(INTDIR)\TmcRTH_DialogBlock.obj" : $(SOURCE) $(DEP_CPP_TMCRTH_D) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TmcRTH_DialogFormatOutFile.cpp
DEP_CPP_TMCRTH_DI=\
	".\PlanRT_H.h"\
	".\StdAfx.h"\
	".\TmcRTH_DialogFormatOutFile.h"\
	

"$(INTDIR)\TmcRTH_DialogFormatOutFile.obj" : $(SOURCE) $(DEP_CPP_TMCRTH_DI)\
 "$(INTDIR)" "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PL_GLFUN.CPP
DEP_CPP_PL_GL=\
	".\MainFrm.h"\
	".\PlanRT_H.h"\
	".\PlanRT_HDoc.h"\
	".\PlanRT_HView.h"\
	".\StdAfx.h"\
	".\TmcDialogStatistics.h"\
	".\TMCGrExpression.h"\
	".\TMCGRVIW.H"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_DialogFormatOutFile.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TmcRTHNodeDiel.h"\
	".\TmcRTHRectNode.h"\
	".\TYPEDEF.H"\
	".\TYPERTH.H"\
	

"$(INTDIR)\PL_GLFUN.OBJ" : $(SOURCE) $(DEP_CPP_PL_GL) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PL_IOFOR.CPP
DEP_CPP_PL_IO=\
	".\MainFrm.h"\
	".\Pl_iofor.h"\
	".\PlanRT_H.h"\
	".\PlanRT_HDoc.h"\
	".\PlanRT_HView.h"\
	".\StdAfx.h"\
	".\TmcDialogStatistics.h"\
	".\TMCGrExpression.h"\
	".\TMCGRVIW.H"\
	".\TmcLibError.h"\
	".\TmcRTH_BolckList.h"\
	".\TmcRTH_DialogFormatOutFile.h"\
	".\TmcRTH_Indan.h"\
	".\TmcRTH_IndanOutput.h"\
	".\TmcRTH_IndanParam.h"\
	".\TmcRTH_IndanTopology.h"\
	".\TmcRTHNodeDiel.h"\
	".\TmcRTHRectNode.h"\
	".\TYPEDEF.H"\
	".\TYPERTH.H"\
	

"$(INTDIR)\PL_IOFOR.OBJ" : $(SOURCE) $(DEP_CPP_PL_IO) "$(INTDIR)"\
 "$(INTDIR)\PlanRT_H.pch"


# End Source File
# End Target
# End Project
################################################################################
