# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=TMCGROUT - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to TMCGROUT - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "TMCGROUT - Win32 Release" && "$(CFG)" !=\
 "TMCGROUT - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "Tmcgrout.mak" CFG="TMCGROUT - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "TMCGROUT - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "TMCGROUT - Win32 Debug" (based on "Win32 (x86) Application")
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
# PROP Target_Last_Scanned "TMCGROUT - Win32 Debug"
RSC=rc.exe
MTL=mktyplib.exe
CPP=cl.exe

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"

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

ALL : "$(OUTDIR)\Tmcgrout.exe"

CLEAN : 
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\DialogDoc.obj"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\TMCGrExpression.obj"
	-@erase "$(INTDIR)\TMCGROUT.obj"
	-@erase "$(INTDIR)\Tmcgrout.pch"
	-@erase "$(INTDIR)\TMCGROUT.res"
	-@erase "$(INTDIR)\TMCGROUTDIALOGView.obj"
	-@erase "$(INTDIR)\TMCGROUTDoc.obj"
	-@erase "$(INTDIR)\TMCGROUTView.obj"
	-@erase "$(OUTDIR)\Tmcgrout.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D\
 "_MBCS" /Fp"$(INTDIR)/Tmcgrout.pch" /Yu"stdafx.h" /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
RSC_PROJ=/l 0x419 /fo"$(INTDIR)/TMCGROUT.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Tmcgrout.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 s_file.lib complex.lib exprint.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=s_file.lib complex.lib exprint.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/Tmcgrout.pdb" /machine:I386\
 /out:"$(OUTDIR)/Tmcgrout.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\DialogDoc.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TMCGrExpression.obj" \
	"$(INTDIR)\TMCGROUT.obj" \
	"$(INTDIR)\TMCGROUT.res" \
	"$(INTDIR)\TMCGROUTDIALOGView.obj" \
	"$(INTDIR)\TMCGROUTDoc.obj" \
	"$(INTDIR)\TMCGROUTView.obj"

"$(OUTDIR)\Tmcgrout.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"

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

ALL : "$(OUTDIR)\Tmcgrout.exe" "$(OUTDIR)\Tmcgrout.bsc"

CLEAN : 
	-@erase "$(INTDIR)\ChildFrm.obj"
	-@erase "$(INTDIR)\ChildFrm.sbr"
	-@erase "$(INTDIR)\DialogDoc.obj"
	-@erase "$(INTDIR)\DialogDoc.sbr"
	-@erase "$(INTDIR)\MainFrm.obj"
	-@erase "$(INTDIR)\MainFrm.sbr"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\StdAfx.sbr"
	-@erase "$(INTDIR)\TMCGrExpression.obj"
	-@erase "$(INTDIR)\TMCGrExpression.sbr"
	-@erase "$(INTDIR)\TMCGROUT.obj"
	-@erase "$(INTDIR)\Tmcgrout.pch"
	-@erase "$(INTDIR)\TMCGROUT.res"
	-@erase "$(INTDIR)\TMCGROUT.sbr"
	-@erase "$(INTDIR)\TMCGROUTDIALOGView.obj"
	-@erase "$(INTDIR)\TMCGROUTDIALOGView.sbr"
	-@erase "$(INTDIR)\TMCGROUTDoc.obj"
	-@erase "$(INTDIR)\TMCGROUTDoc.sbr"
	-@erase "$(INTDIR)\TMCGROUTView.obj"
	-@erase "$(INTDIR)\TMCGROUTView.sbr"
	-@erase "$(INTDIR)\vc40.idb"
	-@erase "$(INTDIR)\vc40.pdb"
	-@erase "$(OUTDIR)\Tmcgrout.bsc"
	-@erase "$(OUTDIR)\Tmcgrout.exe"
	-@erase "$(OUTDIR)\Tmcgrout.ilk"
	-@erase "$(OUTDIR)\Tmcgrout.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /Yu"stdafx.h" /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/Tmcgrout.pch" /Yu"stdafx.h"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\Debug/
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
RSC_PROJ=/l 0x419 /fo"$(INTDIR)/TMCGROUT.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/Tmcgrout.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\ChildFrm.sbr" \
	"$(INTDIR)\DialogDoc.sbr" \
	"$(INTDIR)\MainFrm.sbr" \
	"$(INTDIR)\StdAfx.sbr" \
	"$(INTDIR)\TMCGrExpression.sbr" \
	"$(INTDIR)\TMCGROUT.sbr" \
	"$(INTDIR)\TMCGROUTDIALOGView.sbr" \
	"$(INTDIR)\TMCGROUTDoc.sbr" \
	"$(INTDIR)\TMCGROUTView.sbr"

"$(OUTDIR)\Tmcgrout.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 s_file.lib complex.lib exprint.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=s_file.lib complex.lib exprint.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)/Tmcgrout.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/Tmcgrout.exe" 
LINK32_OBJS= \
	"$(INTDIR)\ChildFrm.obj" \
	"$(INTDIR)\DialogDoc.obj" \
	"$(INTDIR)\MainFrm.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\TMCGrExpression.obj" \
	"$(INTDIR)\TMCGROUT.obj" \
	"$(INTDIR)\TMCGROUT.res" \
	"$(INTDIR)\TMCGROUTDIALOGView.obj" \
	"$(INTDIR)\TMCGROUTDoc.obj" \
	"$(INTDIR)\TMCGROUTView.obj"

"$(OUTDIR)\Tmcgrout.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "TMCGROUT - Win32 Release"
# Name "TMCGROUT - Win32 Debug"

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"

!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\ReadMe.txt

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"

!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TMCGROUT.cpp
DEP_CPP_TMCGR=\
	".\ChildFrm.h"\
	".\MainFrm.h"\
	".\StdAfx.h"\
	".\TMCGROUT.h"\
	".\TMCGROUTDoc.h"\
	".\TMCGROUTView.h"\
	

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"


"$(INTDIR)\TMCGROUT.obj" : $(SOURCE) $(DEP_CPP_TMCGR) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"


"$(INTDIR)\TMCGROUT.obj" : $(SOURCE) $(DEP_CPP_TMCGR) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"

"$(INTDIR)\TMCGROUT.sbr" : $(SOURCE) $(DEP_CPP_TMCGR) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\StdAfx.cpp
DEP_CPP_STDAF=\
	".\StdAfx.h"\
	

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS"\
 /Fp"$(INTDIR)/Tmcgrout.pch" /Yc"stdafx.h" /Fo"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Tmcgrout.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

BuildCmds= \
	$(CPP) /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /D "_MBCS" /FR"$(INTDIR)/" /Fp"$(INTDIR)/Tmcgrout.pch" /Yc"stdafx.h"\
 /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c $(SOURCE) \
	

"$(INTDIR)\StdAfx.obj" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\StdAfx.sbr" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

"$(INTDIR)\Tmcgrout.pch" : $(SOURCE) $(DEP_CPP_STDAF) "$(INTDIR)"
   $(BuildCmds)

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MainFrm.cpp
DEP_CPP_MAINF=\
	".\MainFrm.h"\
	".\StdAfx.h"\
	".\TMCGROUT.h"\
	

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"


"$(INTDIR)\MainFrm.obj" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"

"$(INTDIR)\MainFrm.sbr" : $(SOURCE) $(DEP_CPP_MAINF) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ChildFrm.cpp
DEP_CPP_CHILD=\
	".\ChildFrm.h"\
	".\StdAfx.h"\
	".\TMCGROUT.h"\
	

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"


"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"


"$(INTDIR)\ChildFrm.obj" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"

"$(INTDIR)\ChildFrm.sbr" : $(SOURCE) $(DEP_CPP_CHILD) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TMCGROUTDoc.cpp
DEP_CPP_TMCGRO=\
	".\StdAfx.h"\
	".\TMCGROUT.h"\
	".\TMCGROUTDoc.h"\
	

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"


"$(INTDIR)\TMCGROUTDoc.obj" : $(SOURCE) $(DEP_CPP_TMCGRO) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"


"$(INTDIR)\TMCGROUTDoc.obj" : $(SOURCE) $(DEP_CPP_TMCGRO) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"

"$(INTDIR)\TMCGROUTDoc.sbr" : $(SOURCE) $(DEP_CPP_TMCGRO) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TMCGROUTView.cpp
DEP_CPP_TMCGROU=\
	".\DialogDoc.h"\
	".\StdAfx.h"\
	".\TMCGROUT.h"\
	".\TMCGROUTDIALOGView.h"\
	".\TMCGROUTDoc.h"\
	".\TMCGROUTView.h"\
	

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"


"$(INTDIR)\TMCGROUTView.obj" : $(SOURCE) $(DEP_CPP_TMCGROU) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"


"$(INTDIR)\TMCGROUTView.obj" : $(SOURCE) $(DEP_CPP_TMCGROU) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"

"$(INTDIR)\TMCGROUTView.sbr" : $(SOURCE) $(DEP_CPP_TMCGROU) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TMCGROUT.rc
DEP_RSC_TMCGROUT=\
	".\res\TMCGROUT.ico"\
	".\res\TMCGROUT.rc2"\
	".\res\TMCGROUTDoc.ico"\
	".\res\Toolbar.bmp"\
	

"$(INTDIR)\TMCGROUT.res" : $(SOURCE) $(DEP_RSC_TMCGROUT) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\TMCGROUTDIALOGView.cpp
DEP_CPP_TMCGROUTD=\
	".\StdAfx.h"\
	".\TMCGROUT.h"\
	".\TMCGROUTDIALOGView.h"\
	

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"


"$(INTDIR)\TMCGROUTDIALOGView.obj" : $(SOURCE) $(DEP_CPP_TMCGROUTD) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"


"$(INTDIR)\TMCGROUTDIALOGView.obj" : $(SOURCE) $(DEP_CPP_TMCGROUTD) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"

"$(INTDIR)\TMCGROUTDIALOGView.sbr" : $(SOURCE) $(DEP_CPP_TMCGROUTD) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DialogDoc.cpp
DEP_CPP_DIALO=\
	".\DialogDoc.h"\
	".\StdAfx.h"\
	".\TMCGROUT.h"\
	

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"


"$(INTDIR)\DialogDoc.obj" : $(SOURCE) $(DEP_CPP_DIALO) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"


"$(INTDIR)\DialogDoc.obj" : $(SOURCE) $(DEP_CPP_DIALO) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"

"$(INTDIR)\DialogDoc.sbr" : $(SOURCE) $(DEP_CPP_DIALO) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\TMCGrExpression.cpp
DEP_CPP_TMCGRE=\
	".\StdAfx.h"\
	".\TMCGrExpression.h"\
	".\TMCGROUT.h"\
	

!IF  "$(CFG)" == "TMCGROUT - Win32 Release"


"$(INTDIR)\TMCGrExpression.obj" : $(SOURCE) $(DEP_CPP_TMCGRE) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ELSEIF  "$(CFG)" == "TMCGROUT - Win32 Debug"


"$(INTDIR)\TMCGrExpression.obj" : $(SOURCE) $(DEP_CPP_TMCGRE) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"

"$(INTDIR)\TMCGrExpression.sbr" : $(SOURCE) $(DEP_CPP_TMCGRE) "$(INTDIR)"\
 "$(INTDIR)\Tmcgrout.pch"


!ENDIF 

# End Source File
# End Target
# End Project
################################################################################
