; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CPlanRT_HView
LastTemplate=CPropertySheet
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "PlanRT_H.h"
LastPage=0

ClassCount=25
Class1=CPlanRT_HApp
Class2=CPlanRT_HDoc
Class3=CPlanRT_HView
Class4=CMainFrame

ResourceCount=17
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDR_PLANRTTYPE
Class5=CAboutDlg
Class6=CChildFrame
Class7=CTmcRTHRectNode
Class8=CTmcRTHNodeDiel
Resource4=IDD_TMC_STATISTICS (English (U.S.))
Resource5=IDD_PROPPAGE_LARGE (English (U.S.))
Class9=CTmcRTHNodeDielOne
Class10=CTmcRTH_DielNodeList
Class11=CTmcRTH_Input
Class12=CTmcRTH_InputNode
Class13=CTmcLibError
Resource6=IDR_PLANRTTYPE (English (U.S.))
Class14=CTmcRTH_Indan
Class15=CTmcRTH_IndanParam
Class16=CTmcRTH_IndanOutput
Class17=CTmcRTH_IndanTopology
Class18=CTmcRTH_BolckList
Class19=CTmcErrorMessage
Resource7=IDD_TMC_ERROR_BOX (English (U.S.))
Class20=CTmcDialogStatistics
Resource8=IDR_MAINFRAME (English (U.S.))
Class21=CTmcRTH_DialogBlock
Resource9=IDD_TMC_RTH_TOPBLOCK (English (U.S.))
Resource10=IDD_DIALOGBAR (English (U.S.))
Resource11=IDD_FORMVIEW (English (U.S.))
Resource12=IDD_OLE_PROPPAGE_LARGE (English (U.S.))
Resource13=IDD_PROPPAGE_SMALL (English (U.S.))
Resource14=IDD_TMC_SOUND_MELODY2 (English (U.S.))
Class22=CTmcRTH_DialogFormatOutFile
Resource15=IDD_ABOUTBOX (English (U.S.))
Class23=CTmcSoundMel1
Resource16=IDD_TMC_CONFIG_OUTFFORMAT (English (U.S.))
Class24=CTmcSoundMelugy2
Class25=CTmcSoundEffProp
Resource17=IDD_TMC_SOUND_MELODY1 (English (U.S.))

[CLS:CPlanRT_HApp]
Type=0
HeaderFile=PlanRT_H.h
ImplementationFile=PlanRT_H.cpp
Filter=N
LastObject=CPlanRT_HApp

[CLS:CPlanRT_HDoc]
Type=0
HeaderFile=PlanRT_HDoc.h
ImplementationFile=PlanRT_HDoc.cpp
Filter=N
LastObject=CPlanRT_HDoc
BaseClass=CDocument
VirtualFilter=DC

[CLS:CPlanRT_HView]
Type=0
HeaderFile=PlanRT_HView.h
ImplementationFile=PlanRT_HView.cpp
Filter=C
LastObject=ID_CONFIG_DIRECTIONALPATTERN
BaseClass=CScrollView
VirtualFilter=VWC

[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CMDIFrameWnd
VirtualFilter=fWC
LastObject=CMainFrame


[CLS:CChildFrame]
Type=0
HeaderFile=ChildFrm.h
ImplementationFile=ChildFrm.cpp
Filter=M
LastObject=CChildFrame
BaseClass=CMDIChildWnd
VirtualFilter=mfWC

[CLS:CAboutDlg]
Type=0
HeaderFile=PlanRT_H.cpp
ImplementationFile=PlanRT_H.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=CAboutDlg

[DLG:IDD_ABOUTBOX]
Type=1
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308352
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Class=CTmcErrorMessage

[MNU:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
CommandCount=8
Command8=ID_APP_ABOUT

[TB:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
CommandCount=8
Command8=ID_APP_ABOUT

[MNU:IDR_PLANRTTYPE]
Type=1
Class=CPlanRT_HView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_UNDO
Command12=ID_EDIT_CUT
Command13=ID_EDIT_COPY
Command14=ID_EDIT_PASTE
CommandCount=21
Command15=ID_VIEW_TOOLBAR
Command16=ID_VIEW_STATUS_BAR
Command17=ID_WINDOW_NEW
Command18=ID_WINDOW_CASCADE
Command19=ID_WINDOW_TILE_HORZ
Command20=ID_WINDOW_ARRANGE
Command21=ID_APP_ABOUT

[ACL:IDR_MAINFRAME]
Type=1
Class=CMainFrame
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
CommandCount=14
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE


[CLS:CTmcRTHRectNode]
Type=0
HeaderFile=TmcRTHRectNode.h
ImplementationFile=TmcRTHRectNode.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcRTHRectNode

[CLS:CTmcRTHNodeDiel]
Type=0
HeaderFile=TmcRTHNodeDiel.h
ImplementationFile=TmcRTHNodeDiel.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcRTHNodeDiel

[TB:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_EDIT_EDIT
Command4=ID_VIEW_OUTPUT
Command5=ID_VIEW_FIELD_1
Command6=ID_CONFIG_SMATRIX
Command7=ID_VIEW_DIRECTIONALPATTERN
Command8=ID_VIEW_STATISTICS
Command9=ID_RUN_RESTARTALL
Command10=ID_RUN_RUN
Command11=ID_RUN_STOP
Command12=ID_RUN_STARTSTEP
Command13=ID_RUN_RESTARTSTEP
Command14=ID_RUN_SKIPSTEP
Command15=ID_RUN_BACKSTEP
Command16=ID_VIEW_TOPOLOGY
Command17=ID_VIEW_FIELD
Command18=ID_CONFIG_SINCHRONIZATION
Command19=ID_CONFIG_DIRECTIONALPATTERN
Command20=ID_CONFIG_SOUND
Command21=ID_CONFIG_AUTORUN
Command22=ID_FILE_PRINT
Command23=ID_APP_ABOUT
CommandCount=23

[MNU:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_PRINT_SETUP
Command4=ID_FILE_MRU_FILE1
Command5=ID_APP_EXIT
Command6=ID_VIEW_TOOLBAR
Command7=ID_VIEW_STATUS_BAR
Command8=ID_APP_ABOUT
CommandCount=8

[MNU:IDR_PLANRTTYPE (English (U.S.))]
Type=1
Class=CPlanRT_HView
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_CLOSE
Command4=ID_FILE_SAVE
Command5=ID_FILE_SAVE_AS
Command6=ID_FILE_PRINT
Command7=ID_FILE_PRINT_PREVIEW
Command8=ID_FILE_PRINT_SETUP
Command9=ID_FILE_MRU_FILE1
Command10=ID_APP_EXIT
Command11=ID_EDIT_EDIT
Command12=ID_EDIT_UNDO
Command13=ID_EDIT_CUT
Command14=ID_EDIT_COPY
Command15=ID_EDIT_PASTE
Command16=ID_VIEW_OUTPUT
Command17=ID_VIEW_FIELD_1
Command18=ID_VIEW_STATISTICS
Command19=ID_CONFIG_SMATRIX
Command20=ID_VIEW_DIRECTIONALPATTERN
Command21=ID_VIEW_TOOLBAR
Command22=ID_VIEW_STATUS_BAR
Command23=ID_RUN_RUN
Command24=ID_RUN_RESTARTALL
Command25=ID_RUN_STARTSTEP
Command26=ID_RUN_RESTARTSTEP
Command27=ID_RUN_SKIPSTEP
Command28=ID_RUN_BACKSTEP
Command29=ID_RUN_STOP
Command30=ID_CONFIG_EDITOR
Command31=ID_CONFIG_VIEWER_OUTPUTSIGNAL
Command32=ID_CONFIG_VIEWER_FIELD
Command33=ID_CONFIG_VIEWER_SMATRIX
Command34=ID_CONFIG_VIEWER_DIRECTIONALPATTERN
Command35=ID_CONFIG_COLOR_BACKGROUND
Command36=ID_CONFIG_FORMAT_OUTPUTDATAFILE
Command37=ID_VIEW_TOPOLOGY
Command38=ID_VIEW_FIELD
Command39=ID_CONFIG_SINCHRONIZATION
Command40=ID_CONFIG_DIRECTIONALPATTERN
Command41=ID_CONFIG_SOUND
Command42=ID_CONFIG_SOUND_MELODY
Command43=ID_CONFIG_AUTORUN
Command44=ID_CONFIG_SETUP
Command45=ID_WINDOW_CASCADE
Command46=ID_WINDOW_TILE_HORZ
Command47=ID_WINDOW_ARRANGE
Command48=ID_APP_ABOUT
CommandCount=48

[ACL:IDR_MAINFRAME (English (U.S.))]
Type=1
Class=?
Command1=ID_EDIT_COPY
Command2=ID_FILE_NEW
Command3=ID_FILE_OPEN
Command4=ID_FILE_PRINT
Command5=ID_FILE_SAVE
Command6=ID_EDIT_PASTE
Command7=ID_EDIT_UNDO
Command8=ID_EDIT_CUT
Command9=ID_VIEW_TOPOLOGY
Command10=ID_VIEW_FIELD
Command11=ID_VIEW_DIRECTIONALPATTERN
Command12=ID_VIEW_STATISTICS
Command13=ID_CONFIG_SMATRIX
Command14=ID_VIEW_OUTPUT
Command15=ID_RUN_RUN
Command16=ID_RUN_STOP
Command17=ID_RUN_RESTARTALL
Command18=ID_NEXT_PANE
Command19=ID_RUN_SKIPSTEP
Command20=ID_RUN_BACKSTEP
Command21=ID_PREV_PANE
Command22=ID_EDIT_EDIT
Command23=ID_RUN_STARTSTEP
Command24=ID_RUN_RESTARTSTEP
Command25=ID_EDIT_COPY
Command26=ID_EDIT_PASTE
Command27=ID_EDIT_CUT
Command28=ID_EDIT_UNDO
CommandCount=28

[DLG:IDD_ABOUTBOX (English (U.S.))]
Type=1
Class=CTmcErrorMessage
ControlCount=6
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_RTHTMCABOUTBOXMODEL,edit,1342244992
Control6=IDC_STATIC,static,1342308352

[CLS:CTmcRTHNodeDielOne]
Type=0
HeaderFile=TmcRTHNodeDielOne.h
ImplementationFile=TmcRTHNodeDielOne.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcRTHNodeDielOne

[CLS:CTmcRTH_DielNodeList]
Type=0
HeaderFile=TmcRTH_DielNodeList.h
ImplementationFile=TmcRTH_DielNodeList.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcRTH_DielNodeList

[CLS:CTmcRTH_Input]
Type=0
HeaderFile=TmcRTH_Input.h
ImplementationFile=TmcRTH_Input.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcRTH_Input

[CLS:CTmcRTH_InputNode]
Type=0
HeaderFile=TmcRTH_InputNode.h
ImplementationFile=TmcRTH_InputNode.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcRTH_InputNode

[CLS:CTmcLibError]
Type=0
HeaderFile=TmcLibError.h
ImplementationFile=TmcLibError.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcLibError

[CLS:CTmcRTH_Indan]
Type=0
HeaderFile=TmcRTH_Indan.h
ImplementationFile=TmcRTH_Indan.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcRTH_Indan

[CLS:CTmcRTH_IndanParam]
Type=0
HeaderFile=TmcRTH_IndanParam.h
ImplementationFile=TmcRTH_IndanParam.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcRTH_IndanParam

[CLS:CTmcRTH_IndanOutput]
Type=0
HeaderFile=TmcRTH_IndanOutput.h
ImplementationFile=TmcRTH_IndanOutput.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcRTH_IndanOutput

[CLS:CTmcRTH_IndanTopology]
Type=0
HeaderFile=TmcRTH_IndanTopology.h
ImplementationFile=TmcRTH_IndanTopology.cpp
BaseClass=generic CWnd
Filter=W
LastObject=CTmcRTH_IndanTopology

[CLS:CTmcRTH_BolckList]
Type=0
HeaderFile=TmcRTH_BolckList.h
ImplementationFile=TmcRTH_BolckList.cpp
BaseClass=generic CWnd
Filter=W
LastObject=IDC_EDIT1

[DLG:IDD_TMC_ERROR_BOX (English (U.S.))]
Type=1
ControlCount=34
Control1=IDOK,button,1342373889
Control2=IDC_TMCSTATISTICSERRORMESSAGE,edit,1350631556
Control3=IDC_STATIC,button,1342177287
Control4=IDC_STATIC,button,1342177287
Control5=IDC_TMCSTATISTICSNSTEP,edit,1350641792
Control6=IDC_STATIC,button,1342177287
Control7=IDC_TMCSTATISTICST0,edit,1350641792
Control8=IDC_STATIC,button,1342177287
Control9=IDC_TMCSTATISTICSLONGUNIT,edit,1350633604
Control10=IDC_STATIC,button,1342177287
Control11=IDC_STATIC,button,1342177287
Control12=IDC_TMCSTATISTICSFREQUNIT,edit,1350633604
Control13=IDC_STATIC,button,1342177287
Control14=IDC_TMCSTATISTICSTIMEUNIT,edit,1350633604
Control15=IDC_STATIC,button,1342177287
Control16=IDC_TMCSTATISTICSFREQ,edit,1350641792
Control17=IDC_STATIC,button,1342177287
Control18=IDC_TMCSTATISTICSDELTA,edit,1350641792
Control19=IDC_STATIC,button,1342177287
Control20=IDC_STATIC,button,1342177287
Control21=IDC_TMCSTATISTICSTMIN,edit,1350641792
Control22=IDC_STATIC,static,1342308352
Control23=IDC_STATIC,static,1342308352
Control24=IDC_TMCSTATISTICSTMAX,edit,1350641792
Control25=IDC_STATIC,button,1342177287
Control26=IDC_TMCSTATISTICSXMIN,edit,1350641792
Control27=IDC_STATIC,static,1342308352
Control28=IDC_STATIC,static,1342308352
Control29=IDC_TMCSTATISTICSXMAX,edit,1350641792
Control30=IDC_STATIC,button,1342177287
Control31=IDC_TMCSTATISTICSYMIN,edit,1350641792
Control32=IDC_STATIC,static,1342308352
Control33=IDC_STATIC,static,1342308352
Control34=IDC_TMCSTATISTICSYMAX,edit,1350641792

[CLS:CTmcErrorMessage]
Type=0
HeaderFile=TmcErrorMessage.h
ImplementationFile=TmcErrorMessage.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_RTHTMCABOUTBOXMODEL

[DLG:IDD_TMC_STATISTICS (English (U.S.))]
Type=1
Class=?
ControlCount=76
Control1=IDOK,button,1342373889
Control2=IDC_TMCSTATISTICSVIEWBLOCKLIST,button,1342373888
Control3=IDC_TMCSTATISTICSPREVSTEP,button,1342373888
Control4=IDC_TMCSTATISTICSNEXTSTEP,button,1342373888
Control5=IDC_TMCSTATISTICSVIEWBLOCKLIST2,button,1342373888
Control6=IDC_TMCSTATISTICSLONGUNIT,edit,1350633604
Control7=IDC_TMCSTATISTICSTIMEUNIT,edit,1350633604
Control8=IDC_TMCSTATISTICSFREQUNIT,edit,1350633604
Control9=IDC_TMCSTATISTICSANGLEUNIT,edit,1350633604
Control10=IDC_TMCSTATISTICSDELTA,edit,1350641792
Control11=IDC_TMCSTATISTICST0,edit,1350641792
Control12=IDC_TMCSTATISTICSFREQ,edit,1350641792
Control13=IDC_TMCSTATISTICSWAVELENGHT,edit,1350641792
Control14=IDC_TMCSTATISTICSTMIN,edit,1350641792
Control15=IDC_TMCSTATISTICSTMAX,edit,1350641792
Control16=IDC_TMCSTATISTICSXMIN,edit,1350641792
Control17=IDC_TMCSTATISTICSXMAX,edit,1350641792
Control18=IDC_TMCSTATISTICSYMIN,edit,1350641792
Control19=IDC_TMCSTATISTICSYMAX,edit,1350641792
Control20=IDC_TMCSTATISTICSWAVELENGHTDELT,edit,1350641792
Control21=IDC_TMCSTATISTICST2,edit,1350641792
Control22=IDC_TMCSTATISTICSFILEOUT,edit,1350641792
Control23=IDC_TMCSTATISTICSNSTEP,edit,1350641792
Control24=IDC_TMCSTATISTICSERRORMESSAGE,edit,1350633540
Control25=IDC_TMCSTATISTICSNBLOCK,edit,1350641792
Control26=IDC_TMCSTATISTICSFIELDOUT,button,1476460547
Control27=IDC_TMCSTATISTICSTOPOLOGYOUT,button,1476460547
Control28=IDC_STATIC,button,1342177287
Control29=IDC_STATIC,button,1342177287
Control30=IDC_STATIC,button,1342177287
Control31=IDC_STATIC,button,1342177287
Control32=IDC_STATIC,button,1342177287
Control33=IDC_STATIC,button,1342177287
Control34=IDC_STATIC,button,1342177287
Control35=IDC_STATIC,button,1342177287
Control36=IDC_STATIC,button,1342177287
Control37=IDC_STATIC,button,1342177287
Control38=IDC_STATIC,button,1342177287
Control39=IDC_STATIC,static,1342308352
Control40=IDC_STATIC,static,1342308352
Control41=IDC_STATIC,button,1342177287
Control42=IDC_STATIC,static,1342308352
Control43=IDC_STATIC,static,1342308352
Control44=IDC_STATIC,button,1342177287
Control45=IDC_STATIC,static,1342308352
Control46=IDC_STATIC,static,1342308352
Control47=IDC_STATIC,button,1342177287
Control48=IDC_STATIC,button,1342177287
Control49=IDC_STATIC,button,1342177287
Control50=IDC_STATIC,static,1342308352
Control51=IDC_STATIC,button,1342177287
Control52=IDC_STATIC,static,1342308352
Control53=IDC_STATIC,static,1342308352
Control54=IDC_STATIC,static,1342308352
Control55=IDC_TMCSTATISTICSXSIZEWL,edit,1350641792
Control56=IDC_TMCSTATISTICSYSIZEWL,edit,1350641792
Control57=IDC_TMCSTATISTICSTSIZEWL,edit,1350641792
Control58=IDC_STATIC,static,1342308352
Control59=IDC_STATIC,static,1342308352
Control60=IDC_STATIC,static,1342308352
Control61=IDC_STATIC,static,1342308352
Control62=IDC_TMCSTATISTICSTSIZEWL1,edit,1350641792
Control63=IDC_STATIC,button,1342177287
Control64=IDC_STATIC,button,1342177287
Control65=IDC_TMCSTATISTICSNUMNODE,edit,1350641792
Control66=IDC_TMCSTATISTICSNUMMEMORYMB,edit,1350641792
Control67=IDC_STATIC,button,1342177287
Control68=IDC_STATIC,static,1342308352
Control69=IDC_TMCSTATISTICSFILEOUT2,edit,1350641792
Control70=IDC_STATIC,button,1342177287
Control71=IDC_TMCSTATISTICSACCURACY,edit,1342253184
Control72=IDC_STATIC,static,1342308352
Control73=IDC_TMCSTATISTICSFILEOUT3,edit,1350641792
Control74=IDC_TMCSTATISTICSTOLERANCE,edit,1342253184
Control75=IDC_STATIC,static,1342308352
Control76=IDC_STATIC,button,1342177287

[CLS:CTmcDialogStatistics]
Type=0
HeaderFile=TmcDialogStatistics.h
ImplementationFile=TmcDialogStatistics.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_TMCSTATISTICSDELTA

[DLG:IDD_TMC_RTH_TOPBLOCK (English (U.S.))]
Type=1
Class=?
ControlCount=65
Control1=IDOK,button,1342373889
Control2=ID_TMCBLOCKBUTTONNEXT2,button,1342373888
Control3=IDC_TMCBLOCKNUMBER,edit,1350641792
Control4=IDC_TMCBLOCKX0,edit,1350641792
Control5=IDC_TMCBLOCKY0,edit,1350641792
Control6=IDC_TMCBLOCKLONGUNIT,edit,1350633600
Control7=IDC_TMCBLOCKSTRING,edit,1350633540
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,button,1342177287
Control10=IDC_STATIC,button,1342177287
Control11=IDC_STATIC,button,1342177287
Control12=IDC_STATIC,button,1342177287
Control13=IDC_STATIC,button,1342177287
Control14=ID_TMCBLOCKBUTTONPREV,button,1342373888
Control15=ID_TMCBLOCKBUTTONNEXT,button,1342373888
Control16=IDC_STATIC,button,1342177287
Control17=IDC_TMCBLOCKEXPREPS,edit,1350633540
Control18=IDC_STATIC,button,1342177287
Control19=IDC_TMCBLOCKXMINB,edit,1350641792
Control20=IDC_TMCBLOCKXMAXB,edit,1350641792
Control21=IDC_TMCBLOCKYMINB,edit,1350641792
Control22=IDC_TMCBLOCKYMAXB,edit,1350641792
Control23=IDC_TMCBLOCKVXSPEED,edit,1350641792
Control24=IDC_STATIC,button,1342177287
Control25=IDC_TMCBLOCKWSPEED,edit,1350641792
Control26=IDC_STATIC,button,1342177287
Control27=IDC_TMCBLOCKVYSPEED,edit,1350641792
Control28=IDC_TMCBLOCKDX1,edit,1350641792
Control29=IDC_STATIC,button,1342177287
Control30=IDC_TMCBLOCKDY1,edit,1350641792
Control31=IDC_STATIC,button,1342177287
Control32=IDC_TMCBLOCKDX2,edit,1350641792
Control33=IDC_TMCBLOCKDX3,edit,1350641792
Control34=IDC_TMCBLOCKDX4,edit,1350641792
Control35=IDC_TMCBLOCKDX5,edit,1350641792
Control36=IDC_TMCBLOCKDX6,edit,1350641792
Control37=IDC_TMCBLOCKDX7,edit,1350641792
Control38=IDC_TMCBLOCKDX8,edit,1350641792
Control39=IDC_TMCBLOCKDX9,edit,1350641792
Control40=IDC_TMCBLOCKDX10,edit,1350641792
Control41=IDC_TMCBLOCKDY2,edit,1350641792
Control42=IDC_TMCBLOCKDY3,edit,1350641792
Control43=IDC_TMCBLOCKDY4,edit,1350641792
Control44=IDC_TMCBLOCKDY5,edit,1350641792
Control45=IDC_TMCBLOCKDY6,edit,1350641792
Control46=IDC_TMCBLOCKDY7,edit,1350641792
Control47=IDC_TMCBLOCKDY8,edit,1350641792
Control48=IDC_TMCBLOCKDY9,edit,1350641792
Control49=IDC_TMCBLOCKDY10,edit,1350641792
Control50=IDC_TMCBLOCKNPOINTXY,edit,1350641792
Control51=IDC_STATIC,button,1342177287
Control52=IDC_STATIC,button,1342177287
Control53=IDC_TMCBLOCKNTYPE,edit,1350641792
Control54=IDC_STATIC,static,1342308352
Control55=IDC_STATIC,static,1342308352
Control56=IDC_TMCBLOCKXMINBTEXT,edit,1342253184
Control57=IDC_TMCBLOCKXMAXBTEXT,edit,1342253184
Control58=IDC_TMCBLOCKYMINBTEXT,edit,1342253184
Control59=IDC_TMCBLOCKYMAXBTEXT,edit,1342253184
Control60=IDC_STATIC,button,1342177287
Control61=IDC_STATIC,button,1342177287
Control62=IDC_STATIC,button,1342177287
Control63=IDC_STATIC,button,1342177287
Control64=IDC_STATIC,button,1342177287
Control65=IDC_TMCBLOCKMEMORY1,edit,1350633540

[CLS:CTmcRTH_DialogBlock]
Type=0
HeaderFile=TmcRTH_DialogBlock.h
ImplementationFile=TmcRTH_DialogBlock.cpp
BaseClass=CDialog
Filter=D
LastObject=IDC_TMCBLOCKLONGUNIT
VirtualFilter=dWC

[DLG:IDD_DIALOGBAR (English (U.S.))]
Type=1
ControlCount=1
Control1=IDC_STATIC,static,1342308352

[DLG:IDD_FORMVIEW (English (U.S.))]
Type=1
ControlCount=1
Control1=IDC_STATIC,static,1342308352

[DLG:IDD_OLE_PROPPAGE_LARGE (English (U.S.))]
Type=1
ControlCount=1
Control1=IDC_STATIC,static,1342308352

[DLG:IDD_PROPPAGE_SMALL (English (U.S.))]
Type=1
ControlCount=1
Control1=IDC_STATIC,static,1342308352

[DLG:IDD_PROPPAGE_LARGE (English (U.S.))]
Type=1
ControlCount=1
Control1=IDC_STATIC,static,1342308352

[DLG:IDD_TMC_CONFIG_OUTFFORMAT (English (U.S.))]
Type=1
Class=?
ControlCount=15
Control1=IDC_TMCRTHFORMUOTF_NT,edit,1350631552
Control2=IDC_TMCRTHFORMUOTF_DT,edit,1350631552
Control3=IDC_TMCRTHFORMUOTF_NBL,edit,1350631552
Control4=IDC_TMCRTHFORMUOTF_INP,edit,1350631552
Control5=IDC_TMCRTHFORMUOTF_OUT,edit,1350631552
Control6=IDOK,button,1342373889
Control7=IDC_STATIC,button,1342177287
Control8=IDC_STATIC,button,1342177287
Control9=IDC_STATIC,button,1342177287
Control10=IDC_STATIC,button,1342177287
Control11=IDC_STATIC,button,1342177287
Control12=IDOKDefaultFormat,button,1342373888
Control13=IDC_STATIC,button,1342177287
Control14=IDC_STATIC,button,1342177287
Control15=IDC_TMCRTHFORMUOTFIELD,edit,1350631552

[CLS:CTmcRTH_DialogFormatOutFile]
Type=0
HeaderFile=TmcRTH_DialogFormatOutFile.h
ImplementationFile=TmcRTH_DialogFormatOutFile.cpp
BaseClass=CDialog
Filter=D
LastObject=CTmcRTH_DialogFormatOutFile

[DLG:IDD_TMC_SOUND_MELODY1 (English (U.S.))]
Type=1
Class=CTmcSoundMel1
ControlCount=171
Control1=IDC_TmcSoundEffects_TimeInterval,edit,1350631552
Control2=IDC_STATIC,button,1342177287
Control3=IDC_TmcSoundEffects_Sound,button,1342373897
Control4=IDC_TmcSoundEffects_Sound2,button,1342242825
Control5=IDC_TmcSoundEffects_Sound3,button,1342242825
Control6=IDC_TmcSoundEffects_Sound4,button,1342242825
Control7=IDC_TmcSoundEffects_Sound5,button,1342242825
Control8=IDC_TmcSoundEffects_Sound6,button,1342242825
Control9=IDC_TmcSoundEffects_Sound7,button,1342242825
Control10=IDC_TmcSoundEffects_Sound8,button,1342242825
Control11=IDC_TmcSoundEffects_Sound9,button,1342242825
Control12=IDC_TmcSoundEffects_Sound10,button,1342242825
Control13=IDC_TmcSoundEffects_Sound11,button,1342242825
Control14=IDC_TmcSoundEffects_Sound12,button,1342242825
Control15=IDC_TmcSoundEffects_Sound97,button,1342242825
Control16=IDC_STATIC,button,1342373895
Control17=IDC_TmcSoundEffects_Sound98,button,1342373897
Control18=IDC_TmcSoundEffects_Sound99,button,1342242825
Control19=IDC_TmcSoundEffects_Sound100,button,1342242825
Control20=IDC_TmcSoundEffects_Sound101,button,1342242825
Control21=IDC_TmcSoundEffects_Sound102,button,1342242825
Control22=IDC_TmcSoundEffects_Sound103,button,1342242825
Control23=IDC_TmcSoundEffects_Sound104,button,1342242825
Control24=IDC_TmcSoundEffects_Sound105,button,1342242825
Control25=IDC_TmcSoundEffects_Sound106,button,1342242825
Control26=IDC_TmcSoundEffects_Sound107,button,1342242825
Control27=IDC_TmcSoundEffects_Sound108,button,1342242825
Control28=IDC_TmcSoundEffects_Sound109,button,1342242825
Control29=IDC_TmcSoundEffects_Sound110,button,1342242825
Control30=IDC_STATIC,button,1342373895
Control31=IDC_TmcSoundEffects_Sound111,button,1342373897
Control32=IDC_TmcSoundEffects_Sound112,button,1342242825
Control33=IDC_TmcSoundEffects_Sound113,button,1342242825
Control34=IDC_TmcSoundEffects_Sound114,button,1342242825
Control35=IDC_TmcSoundEffects_Sound115,button,1342242825
Control36=IDC_TmcSoundEffects_Sound116,button,1342242825
Control37=IDC_TmcSoundEffects_Sound117,button,1342242825
Control38=IDC_TmcSoundEffects_Sound118,button,1342242825
Control39=IDC_TmcSoundEffects_Sound119,button,1342242825
Control40=IDC_TmcSoundEffects_Sound120,button,1342242825
Control41=IDC_TmcSoundEffects_Sound121,button,1342242825
Control42=IDC_TmcSoundEffects_Sound122,button,1342242825
Control43=IDC_TmcSoundEffects_Sound123,button,1342242825
Control44=IDC_STATIC,button,1342373895
Control45=IDC_TmcSoundEffects_Sound124,button,1342373897
Control46=IDC_TmcSoundEffects_Sound125,button,1342242825
Control47=IDC_TmcSoundEffects_Sound126,button,1342242825
Control48=IDC_TmcSoundEffects_Sound127,button,1342242825
Control49=IDC_TmcSoundEffects_Sound128,button,1342242825
Control50=IDC_TmcSoundEffects_Sound129,button,1342242825
Control51=IDC_TmcSoundEffects_Sound130,button,1342242825
Control52=IDC_TmcSoundEffects_Sound131,button,1342242825
Control53=IDC_TmcSoundEffects_Sound132,button,1342242825
Control54=IDC_TmcSoundEffects_Sound133,button,1342242825
Control55=IDC_TmcSoundEffects_Sound134,button,1342242825
Control56=IDC_TmcSoundEffects_Sound135,button,1342242825
Control57=IDC_TmcSoundEffects_Sound136,button,1342242825
Control58=IDC_STATIC,button,1342373895
Control59=IDC_TmcSoundEffects_Sound137,button,1342373897
Control60=IDC_TmcSoundEffects_Sound138,button,1342242825
Control61=IDC_TmcSoundEffects_Sound139,button,1342242825
Control62=IDC_TmcSoundEffects_Sound140,button,1342242825
Control63=IDC_TmcSoundEffects_Sound141,button,1342242825
Control64=IDC_TmcSoundEffects_Sound142,button,1342242825
Control65=IDC_TmcSoundEffects_Sound143,button,1342242825
Control66=IDC_TmcSoundEffects_Sound144,button,1342242825
Control67=IDC_TmcSoundEffects_Sound145,button,1342242825
Control68=IDC_TmcSoundEffects_Sound146,button,1342242825
Control69=IDC_TmcSoundEffects_Sound147,button,1342242825
Control70=IDC_TmcSoundEffects_Sound148,button,1342242825
Control71=IDC_TmcSoundEffects_Sound149,button,1342242825
Control72=IDC_STATIC,button,1342373895
Control73=IDC_TmcSoundEffects_Sound150,button,1342373897
Control74=IDC_TmcSoundEffects_Sound151,button,1342242825
Control75=IDC_TmcSoundEffects_Sound152,button,1342242825
Control76=IDC_TmcSoundEffects_Sound153,button,1342242825
Control77=IDC_TmcSoundEffects_Sound154,button,1342242825
Control78=IDC_TmcSoundEffects_Sound155,button,1342242825
Control79=IDC_TmcSoundEffects_Sound156,button,1342242825
Control80=IDC_TmcSoundEffects_Sound157,button,1342242825
Control81=IDC_TmcSoundEffects_Sound158,button,1342242825
Control82=IDC_TmcSoundEffects_Sound159,button,1342242825
Control83=IDC_TmcSoundEffects_Sound160,button,1342242825
Control84=IDC_TmcSoundEffects_Sound161,button,1342242825
Control85=IDC_TmcSoundEffects_Sound162,button,1342242825
Control86=IDC_STATIC,button,1342373895
Control87=IDC_TmcSoundEffects_Sound163,button,1342373897
Control88=IDC_TmcSoundEffects_Sound164,button,1342242825
Control89=IDC_TmcSoundEffects_Sound165,button,1342242825
Control90=IDC_TmcSoundEffects_Sound166,button,1342242825
Control91=IDC_TmcSoundEffects_Sound167,button,1342242825
Control92=IDC_TmcSoundEffects_Sound168,button,1342242825
Control93=IDC_TmcSoundEffects_Sound169,button,1342242825
Control94=IDC_TmcSoundEffects_Sound170,button,1342242825
Control95=IDC_TmcSoundEffects_Sound171,button,1342242825
Control96=IDC_TmcSoundEffects_Sound172,button,1342242825
Control97=IDC_TmcSoundEffects_Sound173,button,1342242825
Control98=IDC_TmcSoundEffects_Sound174,button,1342242825
Control99=IDC_TmcSoundEffects_Sound175,button,1342242825
Control100=IDC_STATIC,button,1342373895
Control101=IDC_TmcSoundEffects_Sound176,button,1342373897
Control102=IDC_TmcSoundEffects_Sound177,button,1342242825
Control103=IDC_TmcSoundEffects_Sound178,button,1342242825
Control104=IDC_TmcSoundEffects_Sound179,button,1342242825
Control105=IDC_TmcSoundEffects_Sound180,button,1342242825
Control106=IDC_TmcSoundEffects_Sound181,button,1342242825
Control107=IDC_TmcSoundEffects_Sound182,button,1342242825
Control108=IDC_TmcSoundEffects_Sound183,button,1342242825
Control109=IDC_TmcSoundEffects_Sound184,button,1342242825
Control110=IDC_TmcSoundEffects_Sound185,button,1342242825
Control111=IDC_TmcSoundEffects_Sound186,button,1342242825
Control112=IDC_TmcSoundEffects_Sound187,button,1342242825
Control113=IDC_TmcSoundEffects_Sound188,button,1342242825
Control114=IDC_STATIC,button,1342373895
Control115=IDC_TmcSoundEffects_Sound189,button,1342373897
Control116=IDC_TmcSoundEffects_Sound190,button,1342242825
Control117=IDC_TmcSoundEffects_Sound191,button,1342242825
Control118=IDC_TmcSoundEffects_Sound192,button,1342242825
Control119=IDC_TmcSoundEffects_Sound193,button,1342242825
Control120=IDC_TmcSoundEffects_Sound194,button,1342242825
Control121=IDC_TmcSoundEffects_Sound195,button,1342242825
Control122=IDC_TmcSoundEffects_Sound196,button,1342242825
Control123=IDC_TmcSoundEffects_Sound197,button,1342242825
Control124=IDC_TmcSoundEffects_Sound198,button,1342242825
Control125=IDC_TmcSoundEffects_Sound199,button,1342242825
Control126=IDC_TmcSoundEffects_Sound200,button,1342242825
Control127=IDC_TmcSoundEffects_Sound201,button,1342242825
Control128=IDC_STATIC,button,1342373895
Control129=IDC_TmcSoundEffects_Sound202,button,1342373897
Control130=IDC_TmcSoundEffects_Sound203,button,1342242825
Control131=IDC_TmcSoundEffects_Sound204,button,1342242825
Control132=IDC_TmcSoundEffects_Sound205,button,1342242825
Control133=IDC_TmcSoundEffects_Sound206,button,1342242825
Control134=IDC_TmcSoundEffects_Sound207,button,1342242825
Control135=IDC_TmcSoundEffects_Sound208,button,1342242825
Control136=IDC_TmcSoundEffects_Sound209,button,1342242825
Control137=IDC_TmcSoundEffects_Sound210,button,1342242825
Control138=IDC_TmcSoundEffects_Sound211,button,1342242825
Control139=IDC_TmcSoundEffects_Sound212,button,1342242825
Control140=IDC_TmcSoundEffects_Sound213,button,1342242825
Control141=IDC_TmcSoundEffects_Sound214,button,1342242825
Control142=IDC_STATIC,button,1342373895
Control143=IDC_TmcSoundEffects_Sound215,button,1342373897
Control144=IDC_TmcSoundEffects_Sound216,button,1342242825
Control145=IDC_TmcSoundEffects_Sound217,button,1342242825
Control146=IDC_TmcSoundEffects_Sound218,button,1342242825
Control147=IDC_TmcSoundEffects_Sound219,button,1342242825
Control148=IDC_TmcSoundEffects_Sound220,button,1342242825
Control149=IDC_TmcSoundEffects_Sound221,button,1342242825
Control150=IDC_TmcSoundEffects_Sound222,button,1342242825
Control151=IDC_TmcSoundEffects_Sound223,button,1342242825
Control152=IDC_TmcSoundEffects_Sound224,button,1342242825
Control153=IDC_TmcSoundEffects_Sound225,button,1342242825
Control154=IDC_TmcSoundEffects_Sound226,button,1342242825
Control155=IDC_TmcSoundEffects_Sound227,button,1342242825
Control156=IDC_STATIC,button,1342373895
Control157=IDC_TmcSoundEffects_Sound228,button,1342373897
Control158=IDC_TmcSoundEffects_Sound229,button,1342242825
Control159=IDC_TmcSoundEffects_Sound230,button,1342242825
Control160=IDC_TmcSoundEffects_Sound231,button,1342242825
Control161=IDC_TmcSoundEffects_Sound232,button,1342242825
Control162=IDC_TmcSoundEffects_Sound233,button,1342242825
Control163=IDC_TmcSoundEffects_Sound234,button,1342242825
Control164=IDC_TmcSoundEffects_Sound235,button,1342242825
Control165=IDC_TmcSoundEffects_Sound236,button,1342242825
Control166=IDC_TmcSoundEffects_Sound237,button,1342242825
Control167=IDC_TmcSoundEffects_Sound238,button,1342242825
Control168=IDC_TmcSoundEffects_Sound239,button,1342242825
Control169=IDC_TmcSoundEffects_Sound240,button,1342242825
Control170=IDC_STATIC,button,1342373895
Control171=IDC_EDIT1,edit,1476460672

[CLS:CTmcSoundMel1]
Type=0
HeaderFile=TmcSoundMel1.h
ImplementationFile=TmcSoundMel1.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=CTmcSoundMel1

[DLG:IDD_TMC_SOUND_MELODY2 (English (U.S.))]
Type=1
Class=CTmcSoundMelugy2
ControlCount=2
Control1=IDC_TmcSoundEffects_TimeInterval,edit,1350631552
Control2=65535,button,1342177287

[CLS:CTmcSoundMelugy2]
Type=0
HeaderFile=TmcSoundMelugy2.h
ImplementationFile=TmcSoundMelugy2.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC

[CLS:CTmcSoundEffProp]
Type=0
HeaderFile=TmcSoundEffProp.h
ImplementationFile=TmcSoundEffProp.cpp
BaseClass=CPropertySheet
Filter=W
LastObject=CTmcSoundEffProp

