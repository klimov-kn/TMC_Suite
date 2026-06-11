# Microsoft Developer Studio Generated NMAKE File, Format Version 4.20
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

!IF "$(CFG)" == ""
CFG=PREPR - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to PREPR - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "PREPR - Win32 Release" && "$(CFG)" != "PREPR - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "PREPR.mak" CFG="PREPR - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PREPR - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "PREPR - Win32 Debug" (based on "Win32 (x86) Static Library")
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
# PROP Target_Last_Scanned "PREPR - Win32 Debug"
CPP=cl.exe

!IF  "$(CFG)" == "PREPR - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "d:\tmc\lib\win32\"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=d:\tmc\lib\win32
INTDIR=.\Release

ALL : "$(OUTDIR)\PREPR.lib"

CLEAN : 
	-@erase "$(INTDIR)\DEF_DEST.OBJ"
	-@erase "$(INTDIR)\DEF_ER.OBJ"
	-@erase "$(INTDIR)\DEF_ERD.OBJ"
	-@erase "$(INTDIR)\DEF_MACR.OBJ"
	-@erase "$(INTDIR)\DEF_STR.OBJ"
	-@erase "$(INTDIR)\DEFMIS.OBJ"
	-@erase "$(INTDIR)\PREP_LD.OBJ"
	-@erase "$(INTDIR)\PREP_MN.OBJ"
	-@erase "$(INTDIR)\READ_BUF.OBJ"
	-@erase "$(OUTDIR)\PREPR.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PREPR.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PREPR.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/PREPR.lib" 
LIB32_OBJS= \
	"$(INTDIR)\DEF_DEST.OBJ" \
	"$(INTDIR)\DEF_ER.OBJ" \
	"$(INTDIR)\DEF_ERD.OBJ" \
	"$(INTDIR)\DEF_MACR.OBJ" \
	"$(INTDIR)\DEF_STR.OBJ" \
	"$(INTDIR)\DEFMIS.OBJ" \
	"$(INTDIR)\PREP_LD.OBJ" \
	"$(INTDIR)\PREP_MN.OBJ" \
	"$(INTDIR)\READ_BUF.OBJ"

"$(OUTDIR)\PREPR.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PREPR - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "d:\tmc\lib\win32\"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=d:\tmc\lib\win32
INTDIR=.\Debug

ALL : "$(OUTDIR)\PREPR.lib"

CLEAN : 
	-@erase "$(INTDIR)\DEF_DEST.OBJ"
	-@erase "$(INTDIR)\DEF_ER.OBJ"
	-@erase "$(INTDIR)\DEF_ERD.OBJ"
	-@erase "$(INTDIR)\DEF_MACR.OBJ"
	-@erase "$(INTDIR)\DEF_STR.OBJ"
	-@erase "$(INTDIR)\DEFMIS.OBJ"
	-@erase "$(INTDIR)\PREP_LD.OBJ"
	-@erase "$(INTDIR)\PREP_MN.OBJ"
	-@erase "$(INTDIR)\READ_BUF.OBJ"
	-@erase "$(OUTDIR)\PREPR.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

# ADD BASE CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /GX /Z7 /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/PREPR.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.\.
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/PREPR.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
LIB32_FLAGS=/nologo /out:"$(OUTDIR)/PREPR.lib" 
LIB32_OBJS= \
	"$(INTDIR)\DEF_DEST.OBJ" \
	"$(INTDIR)\DEF_ER.OBJ" \
	"$(INTDIR)\DEF_ERD.OBJ" \
	"$(INTDIR)\DEF_MACR.OBJ" \
	"$(INTDIR)\DEF_STR.OBJ" \
	"$(INTDIR)\DEFMIS.OBJ" \
	"$(INTDIR)\PREP_LD.OBJ" \
	"$(INTDIR)\PREP_MN.OBJ" \
	"$(INTDIR)\READ_BUF.OBJ"

"$(OUTDIR)\PREPR.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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

# Name "PREPR - Win32 Release"
# Name "PREPR - Win32 Debug"

!IF  "$(CFG)" == "PREPR - Win32 Release"

!ELSEIF  "$(CFG)" == "PREPR - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\DEF_DEST.CPP
DEP_CPP_DEF_D=\
	{$(INCLUDE)}"\prepr1.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	{$(INCLUDE)}"\typedef.h"\
	

"$(INTDIR)\DEF_DEST.OBJ" : $(SOURCE) $(DEP_CPP_DEF_D) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DEF_ER.CPP
DEP_CPP_DEF_E=\
	{$(INCLUDE)}"\prepr1.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	

"$(INTDIR)\DEF_ER.OBJ" : $(SOURCE) $(DEP_CPP_DEF_E) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DEF_ERD.CPP
DEP_CPP_DEF_ER=\
	{$(INCLUDE)}"\error1.h"\
	{$(INCLUDE)}"\prepr1.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	{$(INCLUDE)}"\typedef.h"\
	

"$(INTDIR)\DEF_ERD.OBJ" : $(SOURCE) $(DEP_CPP_DEF_ER) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DEF_MACR.CPP
DEP_CPP_DEF_M=\
	{$(INCLUDE)}"\error1.h"\
	{$(INCLUDE)}"\prepr1.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	{$(INCLUDE)}"\typedef.h"\
	

"$(INTDIR)\DEF_MACR.OBJ" : $(SOURCE) $(DEP_CPP_DEF_M) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DEF_STR.CPP
DEP_CPP_DEF_S=\
	{$(INCLUDE)}"\error1.h"\
	{$(INCLUDE)}"\prepr1.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	{$(INCLUDE)}"\typedef.h"\
	

"$(INTDIR)\DEF_STR.OBJ" : $(SOURCE) $(DEP_CPP_DEF_S) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DEFMIS.CPP
DEP_CPP_DEFMI=\
	{$(INCLUDE)}"\error1.h"\
	{$(INCLUDE)}"\indan.h"\
	{$(INCLUDE)}"\listing.h"\
	{$(INCLUDE)}"\mainwndw.h"\
	{$(INCLUDE)}"\prepr1.h"\
	{$(INCLUDE)}"\tmc_lib.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	{$(INCLUDE)}"\tpl_file.h"\
	{$(INCLUDE)}"\typedef.h"\
	

"$(INTDIR)\DEFMIS.OBJ" : $(SOURCE) $(DEP_CPP_DEFMI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PREP_LD.CPP
DEP_CPP_PREP_=\
	{$(INCLUDE)}"\error1.h"\
	{$(INCLUDE)}"\mainwndw.h"\
	{$(INCLUDE)}"\prepr1.h"\
	{$(INCLUDE)}"\run_flag.h"\
	{$(INCLUDE)}"\tmc_lib.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	{$(INCLUDE)}"\tmclimit.h"\
	{$(INCLUDE)}"\tpl_file.h"\
	{$(INCLUDE)}"\typedef.h"\
	

"$(INTDIR)\PREP_LD.OBJ" : $(SOURCE) $(DEP_CPP_PREP_) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\PREP_MN.CPP
DEP_CPP_PREP_M=\
	{$(INCLUDE)}"\error1.h"\
	{$(INCLUDE)}"\mainwndw.h"\
	{$(INCLUDE)}"\prepr1.h"\
	{$(INCLUDE)}"\run_flag.h"\
	{$(INCLUDE)}"\tmc_lib.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	{$(INCLUDE)}"\tmclimit.h"\
	{$(INCLUDE)}"\tpl_file.h"\
	{$(INCLUDE)}"\typedef.h"\
	

"$(INTDIR)\PREP_MN.OBJ" : $(SOURCE) $(DEP_CPP_PREP_M) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\READ_BUF.CPP
DEP_CPP_READ_=\
	{$(INCLUDE)}"\descrptr.h"\
	{$(INCLUDE)}"\error1.h"\
	{$(INCLUDE)}"\listing.h"\
	{$(INCLUDE)}"\mainwndw.h"\
	{$(INCLUDE)}"\prepr1.h"\
	{$(INCLUDE)}"\TmcLibError.h"\
	{$(INCLUDE)}"\tmclimit.h"\
	{$(INCLUDE)}"\tpl_file.h"\
	{$(INCLUDE)}"\typedef.h"\
	

"$(INTDIR)\READ_BUF.OBJ" : $(SOURCE) $(DEP_CPP_READ_) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
