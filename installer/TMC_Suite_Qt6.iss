; ============================================================================
;  TMC_Suite_Qt6.iss — установщик TMC Suite 5.0 (Qt6, Windows x64)
;  Компилятор: Inno Setup 6.7 и новее (ISCC.exe). Кодировка: UTF-8 с BOM.
;
;  Что упаковывается: ВСЯ папка поставки ..\dist\TMC_Suite_Qt6_win64
;  (рекурсивно, как есть — её собирает tools\package_win.sh) -> {app}.
;  Список файлов здесь НЕ перечисляется: пересобрали поставку — пересобрали
;  установщик, ничего править не нужно.
;
;  Оформление: тёмный мастер (WizardStyle=modern dark) в палитре программ
;  пакета; картинки мастера рисует make_wizard_images.py -> assets\.
;
;  Это отдельный продукт со своим AppId: ставится рядом с MFC-версией
;  (installer\TMC_Suite.iss в корне проекта) и не мешает ей.
;
;  Все пути — относительно папки этого файла.
; ============================================================================

#define AppName      "TMC Suite"
#define AppVer       "5.0"
#define AppPublisher "К. Н. Климов"
#define AppURL       "http://www.tamic.ru"
#define AppYear      "2026"
#define DistDir      "..\dist\TMC_Suite_Qt6_win64"
#define LicenseRu    "LICENSE_ru.txt"
; английский мастер показывает английский текст: краткое изложение + полный текст Apache 2.0
#define LicenseEn    "LICENSE_en.txt"
; Для проверочной установки рядом с рабочей (ISCC /DTestBuild): свой AppId, ProgID,
; ключ реестра и группа меню — проверка не задевает установленный пакет.
#ifdef TestBuild
  #define AppIdStr   "{{5E1B7A44-0C2D-4F3B-9A61-7E2C0B9D4F11}"
  #define ProgPrefix "TMCSuiteTest"
  #define RegKey     "Software\TMC Suite Test"
  #define GroupName  "TMC Suite Test"
#else
  #define AppIdStr   "{{AA584951-850D-4CD9-AF91-20AEBBD23037}"
  #define ProgPrefix "TMCSuite5"
  #define RegKey     "Software\TMC Suite"
  #define GroupName  "TMC Suite"
#endif

[Setup]
AppId={#AppIdStr}
AppName={#AppName}
AppVersion={#AppVer}
AppVerName={#AppName} {#AppVer}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
AppUpdatesURL={#AppURL}
AppCopyright=© {#AppYear} К. Н. Климов и коллектив TAMIC
VersionInfoVersion=5.0.0.0
VersionInfoProductName={#AppName}
VersionInfoProductVersion={#AppVer}
VersionInfoDescription={#AppName} Setup
VersionInfoCompany={#AppPublisher}

; Своя папка, не общая с MFC-версией (installer\TMC_Suite.iss ставит в {autopf}\TMC Suite
; и выдаёт «Пользователи: Изменение»): файлы, унаследовавшие это право, обычный
; пользователь мог бы подменить (Т-32). Выбрать папку MFC-версии мастер не даст.
DefaultDirName={autopf}\TMC Suite Qt
DefaultGroupName={#GroupName}
AllowNoIcons=yes
DisableWelcomePage=no
; мастер шире стандартного (120): строки лицензии (до 80 знаков) помещаются без переносов
WizardSizePercent=135
LicenseFile={#LicenseRu}

; Установка для всех (Program Files, нужны права администратора) или только
; для себя (%LOCALAPPDATA%\Programs, без прав администратора) — выбор в диалоге
; при запуске или ключами /ALLUSERS, /CURRENTUSER.
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=commandline dialog
; Программы читают пути только из HKCU (QSettings HKEY_CURRENT_USER\Software\TMC Suite),
; поэтому HKCU пишется и при установке для всех — это ожидаемо.
UsedUserAreasWarning=no

ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
; Оповещение оболочки о новых ассоциациях — сами, без ожидания (CurStepChanged/
; CurUninstallStepChanged, SHCNF_FLUSHNOWAIT): встроенное ChangesAssociations=yes ждёт,
; пока Проводник перестроит значки, и мастер на несколько секунд замирает перед «Завершением».
ChangesAssociations=no

OutputDir=Output
OutputBaseFilename=TMC_Suite_{#AppVer}_Qt6_Setup
Compression=lzma2/max
SolidCompression=yes

SetupIconFile=..\src\resources\icons\app\planarrt_h.ico
UninstallDisplayIcon={app}\planarrt_h.exe
UninstallDisplayName={#AppName}

; --- оформление: только тёмная тема, палитра программ пакета ---
;   bg #0a0e16, surface #131a28, line #243049, text #e7eef7, text-dim #9fb0c8,
;   accent #34e3c4, accent-2 #b07cff, accent-3 #ffcf5c
WizardStyle=modern dark hidebevels
WizardBackColor=#0a0e16
WizardBackImageFile=assets\wizard-back-596x432.png,assets\wizard-back-796x576.png,assets\wizard-back-994x720.png,assets\wizard-back-1272x922.png
WizardImageFile=assets\wizard-large-202x386.png,assets\wizard-large-269x515.png,assets\wizard-large-336x643.png,assets\wizard-large-430x824.png,assets\wizard-large-534x1022.png
WizardSmallImageFile=assets\wizard-small-58.png,assets\wizard-small-77.png,assets\wizard-small-97.png,assets\wizard-small-124.png,assets\wizard-small-159.png
WizardImageBackColor=#0a0e16
WizardSmallImageBackColor=#0a0e16

; мастер двуязычный: в начале — выбор языка (русский / English), по умолчанию отмечен язык Windows
ShowLanguageDialog=yes
LanguageDetectionMethod=uilanguage

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"; LicenseFile: "{#LicenseEn}"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"; LicenseFile: "{#LicenseRu}"

[Messages]
; короткие заголовки страниц «Приветствие» и «Завершение» — без переноса на две строки
en.WelcomeLabel1=TMC Suite Setup
ru.WelcomeLabel1=Установка TMC Suite
en.WelcomeLabel2=This will install TMC Suite (version {#AppVer}) on your computer.%n%nIt is recommended that you close all other applications before continuing.
ru.WelcomeLabel2=Программа установит TMC Suite (версия {#AppVer}) на ваш компьютер.%n%nРекомендуется закрыть все прочие приложения перед тем, как продолжить.
en.FinishedHeadingLabel=Setup complete
ru.FinishedHeadingLabel=Установка завершена

[CustomMessages]
; --- ярлыки ---
en.IconPlanarH=PlanarRT H — solver (H-polarization)
ru.IconPlanarH=PlanarRT H — расчёт (H-поляризация)
en.IconPlanarX=PlanarRT X — solver (X-mode, plasma)
ru.IconPlanarX=PlanarRT X — расчёт (X-мода, плазма)
en.IconGrout=TMCGROUT — S-matrix viewer
ru.IconGrout=TMCGROUT — просмотр S-матриц
en.IconRos=TMCROS — output signal viewer
ru.IconRos=TMCROS — выходной сигнал
en.IconDn=TMC_DN — radiation patterns
ru.IconDn=TMC_DN — диаграммы направленности
en.IconField=FieldView — field viewer
ru.IconField=FieldView — просмотр полей
en.IconTpl=TPL Designer — task editor
ru.IconTpl=TPL Designer — редактор заданий
en.IconManual=User manual (PDF)
ru.IconManual=Руководство пользователя (PDF)
en.IconSamples=Samples
ru.IconSamples=Примеры заданий
en.IconLicense=License agreement
ru.IconLicense=Лицензионное соглашение
en.IconThirdParty=Third-party licenses
ru.IconThirdParty=Лицензии сторонних компонентов
; --- задачи ---
en.GroupAssoc=Open files with TMC Suite programs:
ru.GroupAssoc=Открывать файлы программами TMC Suite:
; --- последняя страница ---
en.FinishedText=TMC Suite has been installed on your computer.
ru.FinishedText=Программа TMC Suite установлена на ваш компьютер.
en.FinishedLinks=Project resources:
ru.FinishedLinks=Ресурсы проекта:
en.FinishedYear=TMC Suite, version {#AppVer}   © {#AppYear} K. N. Klimov
ru.FinishedYear=TMC Suite, версия {#AppVer}   © {#AppYear} К. Н. Климов
en.TaskAssocTpl=TPL Designer: .tpl
ru.TaskAssocTpl=TPL Designer: .tpl
en.TaskAssocGrout=TMCGROUT: .soc
ru.TaskAssocGrout=TMCGROUT: .soc
en.TaskAssocRos=TMCROS: .tos, .toc
ru.TaskAssocRos=TMCROS: .tos, .toc
en.TaskAssocDn=TMC_DN: .dop
ru.TaskAssocDn=TMC_DN: .dop
en.TaskAssocField=FieldView: .ex, .ey, .hz, .em, .amp, .faz, .frames
ru.TaskAssocField=FieldView: .ex, .ey, .hz, .em, .amp, .faz, .frames
en.GroupAssocCommon=Common formats that other programs also use:
ru.GroupAssocCommon=Общие форматы, которыми пользуются и другие программы:
en.AlreadyInstalled=TMC Suite %1 is already installed on this computer:%n%2%n%nSetup will update it: program files will be replaced, your settings and task files will be kept.%n%nContinue?
ru.AlreadyInstalled=TMC Suite %1 уже установлена на этом компьютере:%n%2%n%nПрограмма установки обновит её: файлы программы будут заменены, ваши настройки и файлы заданий сохранятся.%n%nПродолжить?
en.AlreadyOtherMode=TMC Suite %1 is already installed on this computer for %3:%n%2%n%nThe new installation (for %4) will be a second, separate copy. It is recommended to cancel and first uninstall the existing copy (Settings → Apps → TMC Suite → Uninstall).%n%nInstall a second copy anyway?
ru.AlreadyOtherMode=TMC Suite %1 уже установлена на этом компьютере %3:%n%2%n%nНовая установка (%4) поставит вторую, отдельную копию. Рекомендуется отменить установку и сначала удалить прежнюю копию (Параметры → Приложения → TMC Suite → Удалить).%n%nВсё равно поставить вторую копию?
en.ModeAll=all users
ru.ModeAll=для всех пользователей
en.ModeMe=the current user only
ru.ModeMe=только для текущего пользователя
en.MfcFolder=This folder contains the MFC version of TMC Suite. Install the Qt6 version into a separate folder.
ru.MfcFolder=В этой папке установлена MFC-версия TMC Suite. Установите Qt6-версию в отдельную папку.
; .s (ассемблер), .t (тесты Perl), .tf (Terraform), .tt — тоже общие: по умолчанию не
; забираем (Т-34). При удалении прежняя программа для них возвращается ([Code]).
en.TaskAssocCommon=.s, .s2p, .s4p, .s6p, .s8p, .tab → TMCGROUT;  .t → TMCROS;  .tf, .tt → FieldView;  .dat → TMC_DN
ru.TaskAssocCommon=.s, .s2p, .s4p, .s6p, .s8p, .tab → TMCGROUT;  .t → TMCROS;  .tf, .tt → FieldView;  .dat → TMC_DN
; --- описания типов файлов в проводнике ---
en.DescTpl=TMC Suite task file
ru.DescTpl=Файл задания TMC Suite
en.DescGrout=TMC Suite S-matrix (TMCGROUT)
ru.DescGrout=S-матрица TMC Suite (TMCGROUT)
en.DescRos=TMC Suite signal (TMCROS)
ru.DescRos=Сигнал TMC Suite (TMCROS)
en.DescDn=TMC Suite radiation pattern (TMC_DN)
ru.DescDn=Диаграмма направленности TMC Suite (TMC_DN)
en.DescField=TMC Suite field (FieldView)
ru.DescField=Поле TMC Suite (FieldView)

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "assoctpl";    Description: "{cm:TaskAssocTpl}";    GroupDescription: "{cm:GroupAssoc}"
Name: "assocgrout";  Description: "{cm:TaskAssocGrout}";  GroupDescription: "{cm:GroupAssoc}"
Name: "assocros";    Description: "{cm:TaskAssocRos}";    GroupDescription: "{cm:GroupAssoc}"
Name: "assocdn";     Description: "{cm:TaskAssocDn}";     GroupDescription: "{cm:GroupAssoc}"
Name: "assocfield";  Description: "{cm:TaskAssocField}";  GroupDescription: "{cm:GroupAssoc}"
Name: "assoccommon"; Description: "{cm:TaskAssocCommon}"; GroupDescription: "{cm:GroupAssocCommon}"; Flags: unchecked

[Files]
; --- значки-ссылки последней страницы (во временную папку, не устанавливаются; make_link_icons.py) ---
; Первыми в архиве: архив сплошной (SolidCompression), и ExtractTemporaryFile файла из конца
; распаковывал бы всю поставку — последняя страница несколько секунд стояла недостроенной.
Source: "assets\link-*.bmp"; Flags: dontcopy
; --- вся поставка целиком (рекурсивно); временные файлы препроцессора не берём ---
Source: "{#DistDir}\*"; DestDir: "{app}"; Excludes: "$$vr$$s*.prc,*~$Prepr*$~"; Flags: ignoreversion recursesubdirs createallsubdirs
; --- лицензионное соглашение ---
Source: "{#LicenseRu}"; DestDir: "{app}"; DestName: "LICENSE_ru.txt"; Flags: ignoreversion
Source: "{#LicenseEn}"; DestDir: "{app}"; DestName: "LICENSE_en.txt"; Flags: ignoreversion

[Icons]
; Рабочая папка ярлыков — «Документы»: препроцессор ядер пишет временный файл
; в текущую папку, а Program Files обычному пользователю на запись закрыта.
Name: "{group}\{cm:IconPlanarH}"; Filename: "{app}\planarrt_h.exe";  WorkingDir: "{userdocs}"
Name: "{group}\{cm:IconPlanarX}"; Filename: "{app}\planarrt_x.exe";  WorkingDir: "{userdocs}"
Name: "{group}\{cm:IconGrout}";   Filename: "{app}\tmcgrout.exe";    WorkingDir: "{userdocs}"
Name: "{group}\{cm:IconRos}";     Filename: "{app}\tmcros.exe";      WorkingDir: "{userdocs}"
Name: "{group}\{cm:IconDn}";      Filename: "{app}\tmc_dn.exe";      WorkingDir: "{userdocs}"
Name: "{group}\{cm:IconField}";   Filename: "{app}\fieldview.exe";   WorkingDir: "{userdocs}"
Name: "{group}\{cm:IconTpl}";     Filename: "{app}\TplDesigner.exe"; WorkingDir: "{userdocs}"
; ярлык руководства — на языке мастера (английский PDF есть в поставке — tools/package_win.sh)
Name: "{group}\{cm:IconManual}";  Filename: "{app}\TMC_Suite_5.0_Руководство_пользователя.pdf"; Languages: ru
Name: "{group}\{cm:IconManual}";  Filename: "{app}\TMC_Suite_5.0_User_Manual.pdf"; Languages: en
Name: "{group}\{cm:IconSamples}"; Filename: "{app}\samples"
Name: "{group}\{cm:IconLicense}"; Filename: "{app}\LICENSE_ru.txt"; Languages: ru
Name: "{group}\{cm:IconLicense}"; Filename: "{app}\LICENSE_en.txt"; Languages: en
Name: "{group}\{cm:IconThirdParty}"; Filename: "{app}\licenses\README.txt"
Name: "{group}\{cm:ProgramOnTheWeb,{#AppName}}"; Filename: "{#AppURL}"
Name: "{group}\{cm:UninstallProgram,{#AppName}}"; Filename: "{uninstallexe}"
; --- рабочий стол (по галочке): точки входа — ядра и TPL Designer; вьюверы ядра запускают сами ---
Name: "{autodesktop}\PlanarRT H";   Filename: "{app}\planarrt_h.exe";  WorkingDir: "{userdocs}"; Tasks: desktopicon
Name: "{autodesktop}\PlanarRT X";   Filename: "{app}\planarrt_x.exe";  WorkingDir: "{userdocs}"; Tasks: desktopicon
Name: "{autodesktop}\TPL Designer"; Filename: "{app}\TplDesigner.exe"; WorkingDir: "{userdocs}"; Tasks: desktopicon

[Registry]
; --- пути, которые читают программы: QSettings("HKEY_CURRENT_USER\Software\TMC Suite") ---
Root: HKCU; Subkey: "{#RegKey}"; ValueType: string; ValueName: "InstallPath";     ValueData: "{app}";                 Flags: uninsdeletevalue uninsdeletekeyifempty
Root: HKCU; Subkey: "{#RegKey}"; ValueType: string; ValueName: "TplDesignerPath"; ValueData: "{app}\TplDesigner.exe"; Flags: uninsdeletevalue uninsdeletekeyifempty

; --- ассоциации файлов (HKA: HKLM при установке для всех, HKCU — для себя) ---
; Каждое расширение .ext получает свой ProgID TMCSuite5.ext: описание, иконка
; программы (exe,0) и команда открытия "<exe>" "%1".
; Группы: расширения (через пробел) | программа | описание (CustomMessages) | задача.
#define AssocGroups 9
#dim GExts[AssocGroups] {"tpl", "soc", "tos toc", "dop", "ex ey hz em amp faz frames", "dat", "s s2p s4p s6p s8p tab", "t", "tf tt"}
#dim GExe[AssocGroups]  {"TplDesigner.exe", "tmcgrout.exe", "tmcros.exe", "tmc_dn.exe", "fieldview.exe", "tmc_dn.exe", "tmcgrout.exe", "tmcros.exe", "fieldview.exe"}
#dim GDesc[AssocGroups] {"DescTpl", "DescGrout", "DescRos", "DescDn", "DescField", "DescDn", "DescGrout", "DescRos", "DescField"}
#dim GTask[AssocGroups] {"assoctpl", "assocgrout", "assocros", "assocdn", "assocfield", "assoccommon", "assoccommon", "assoccommon", "assoccommon"}
#define G 0
#define P 0
#define Rest ""
#define Ext ""
; Порядок строк важен: при удалении записи обрабатываются с конца, поэтому
; OpenWithProgids идёт после ключа расширения — сначала удаляется он, затем
; опустевший ключ .ext. Значение по умолчанию .ext удаляется (uninsdeletevalue),
; ProgID — целиком (uninsdeletekey).
#sub AssocOne
  #expr Ext = Copy(Rest, 1, P - 1)
  #expr Rest = Copy(Rest, P + 1, Len(Rest))
Root: HKA; Subkey: "Software\Classes\.{#Ext}"; ValueType: string; ValueName: ""; ValueData: "{#ProgPrefix}.{#Ext}"; Flags: uninsdeletevalue uninsdeletekeyifempty; Tasks: {#GTask[G]}
Root: HKA; Subkey: "Software\Classes\.{#Ext}\OpenWithProgids"; ValueType: string; ValueName: "{#ProgPrefix}.{#Ext}"; ValueData: ""; Flags: uninsdeletevalue uninsdeletekeyifempty; Tasks: {#GTask[G]}
Root: HKA; Subkey: "Software\Classes\{#ProgPrefix}.{#Ext}"; ValueType: string; ValueName: ""; ValueData: "{cm:{#GDesc[G]}}"; Flags: uninsdeletekey; Tasks: {#GTask[G]}
Root: HKA; Subkey: "Software\Classes\{#ProgPrefix}.{#Ext}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#GExe[G]},0"; Tasks: {#GTask[G]}
Root: HKA; Subkey: "Software\Classes\{#ProgPrefix}.{#Ext}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#GExe[G]}"" ""%1"""; Tasks: {#GTask[G]}
#endsub
#sub AssocGroup
  #expr Rest = GExts[G] + " "
  #for {P = Pos(" ", Rest); P > 0; P = Pos(" ", Rest)} AssocOne
#endsub
#for {G = 0; G < AssocGroups; G++} AssocGroup
#ifdef DumpTo
#expr SaveToFile(DumpTo)
#endif

[Code]
{ Цвета палитры. Встроенный тёмный стиль мастера красит текст сам; чтобы задать
  свой цвет надписи, у неё снимается seFont из StyleElements. }
const
  ColAccent  = '#34e3c4';
  ColAccent2 = '#b07cff';
  ColText    = '#e7eef7';
  ColTextDim = '#9fb0c8';
  ColSurface = '#131a28';

procedure Paint(L: TNewStaticText; const Col: String; Bold: Boolean);
begin
  L.StyleElements := L.StyleElements - [seFont];
  L.Font.Color := StrToColor(Col);
  if Bold then
    L.Font.Style := L.Font.Style + [fsBold];
end;

{ поля ввода и текстовые окна: фон surface вместо серого фона встроенного стиля }
procedure PaintEdit(E: TNewEdit);
begin
  E.StyleElements := E.StyleElements - [seClient, seFont];
  E.Color := StrToColor(ColSurface);
  E.Font.Color := StrToColor(ColText);
end;

procedure PaintMemo(M: TNewMemo);
begin
  M.StyleElements := M.StyleElements - [seClient, seFont];
  M.Color := StrToColor(ColSurface);
  M.Font.Color := StrToColor(ColText);
end;

{ --- последняя страница: сайт и значки GitHub, LinkedIn, ResearchGate, eLibrary --- }
var
  FinishBuilt: Boolean;

procedure LinkClick(Sender: TObject);
var
  Code: Integer;
  URL: String;
begin
  if Sender is TBitmapImage then
    URL := TBitmapImage(Sender).Hint
  else
    URL := TNewStaticText(Sender).Hint;
  ShellExec('open', URL, '', '', SW_SHOWNORMAL, ewNoWait, Code);
end;

{ размер значка под масштаб экрана: 32 / 40 / 48 / 64 пикселя }
function IconSize: Integer;
var
  W: Integer;
begin
  W := ScaleX(32);
  if W >= 60 then Result := 64
  else if W >= 46 then Result := 48
  else if W >= 38 then Result := 40
  else Result := 32;
end;

procedure AddLink(X, Y, Size: Integer; const Name, Caption, URL: String);
var
  Img: TBitmapImage;
  Cap: TNewStaticText;
  F: String;
begin
  F := 'link-' + Name + '-' + IntToStr(Size) + '.bmp';
  ExtractTemporaryFile(F);
  Img := TBitmapImage.Create(WizardForm);
  Img.Parent := WizardForm.FinishedPage;
  Img.Bitmap.LoadFromFile(ExpandConstant('{tmp}\') + F);
  Img.SetBounds(X, Y, Size, Size);
  Img.Cursor := crHand;
  Img.Hint := URL;
  Img.ShowHint := True;
  Img.OnClick := @LinkClick;

  Cap := TNewStaticText.Create(WizardForm);
  Cap.Parent := WizardForm.FinishedPage;
  Cap.AutoSize := True;
  Cap.Caption := Caption;
  Paint(Cap, ColTextDim, False);
  Cap.Cursor := crHand;
  Cap.Hint := URL;
  Cap.ShowHint := True;
  Cap.OnClick := @LinkClick;
  Cap.Top := Y + Size + ScaleY(4);
  Cap.Left := X + (Size - Cap.Width) div 2;
end;

procedure BuildFinishLinks;
var
  Hdr, Site, Year: TNewStaticText;
  L, W, Y0, Size, Gap, Total, X0, Step: Integer;
begin
  if FinishBuilt then exit;
  FinishBuilt := True;

  WizardForm.FinishedLabel.AutoSize := False;
  WizardForm.FinishedLabel.Caption := CustomMessage('FinishedText');
  WizardForm.FinishedLabel.Height := ScaleY(34);
  { галочек «Запустить …» нет — список прячем, чтобы не занимал место }
  WizardForm.RunList.Visible := False;

  L := WizardForm.FinishedLabel.Left;
  W := WizardForm.FinishedLabel.Width;
  Y0 := WizardForm.FinishedLabel.Top + WizardForm.FinishedLabel.Height + ScaleY(16);

  Hdr := TNewStaticText.Create(WizardForm);
  Hdr.Parent := WizardForm.FinishedPage;
  Hdr.Caption := CustomMessage('FinishedLinks');
  Paint(Hdr, ColTextDim, False);
  Hdr.SetBounds(L, Y0, W, ScaleY(16));

  Site := TNewStaticText.Create(WizardForm);
  Site.Parent := WizardForm.FinishedPage;
  Site.Caption := 'www.tamic.ru';
  Paint(Site, ColAccent, False);
  Site.Font.Style := [fsUnderline];
  Site.Cursor := crHand;
  Site.Hint := 'https://www.tamic.ru';
  Site.ShowHint := True;
  Site.OnClick := @LinkClick;
  Site.SetBounds(L, Y0 + ScaleY(22), W, ScaleY(18));

  { ряд значков — как в окне «О программе» программ пакета }
  Size := IconSize;
  Gap := ScaleX(34);
  Step := Size + Gap;
  Total := 4 * Size + 3 * Gap;
  X0 := L + (W - Total) div 2;
  if X0 < L then X0 := L;
  AddLink(X0,            Y0 + ScaleY(56), Size, 'github',       'GitHub',       'https://github.com/klimov-kn');
  AddLink(X0 + Step,     Y0 + ScaleY(56), Size, 'linkedin',     'LinkedIn',     'https://www.linkedin.com/in/konstantinklimov');
  AddLink(X0 + 2 * Step, Y0 + ScaleY(56), Size, 'researchgate', 'ResearchGate', 'https://www.researchgate.net/profile/Konstantin-Klimov');
  AddLink(X0 + 3 * Step, Y0 + ScaleY(56), Size, 'elibrary',     'eLibrary',     'https://www.elibrary.ru/author_items.asp?authorid=121878');

  Year := TNewStaticText.Create(WizardForm);
  Year.Parent := WizardForm.FinishedPage;
  Year.AutoSize := True;
  Year.Caption := CustomMessage('FinishedYear');
  Paint(Year, ColTextDim, False);
  Year.Top := WizardForm.FinishedPage.Height - ScaleY(28);
  Year.Left := L + (W - Year.Width) div 2;
  if Year.Left < L then Year.Left := L;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  { элементы страницы построены заранее (InitializeWizard); здесь — только текст, который
    Inno Setup записывает в FinishedLabel перед показом страницы }
  if CurPageID = wpFinished then
  begin
    BuildFinishLinks;
    WizardForm.FinishedLabel.Caption := CustomMessage('FinishedText');
  end;
end;

{ --- Т-32: не ставить в папку MFC-версии ---
  MFC-установщик выдаёт на свою папку «Пользователи: Изменение»; файлы Qt6-версии там
  унаследовали бы это право. }
function NextButtonClick(CurPageID: Integer): Boolean;
begin
  Result := True;
  if (CurPageID = wpSelectDir) and
     (FileExists(AddBackslash(WizardDirValue) + 'tmc_rth.exe') or
      FileExists(AddBackslash(WizardDirValue) + 'tmc_rtx.exe')) then
  begin
    MsgBox(CustomMessage('MfcFolder'), mbError, MB_OK);
    Result := False;
  end;
end;

{ --- Т-34: прежние ассоциации расширений ---
  Перед записью своей ассоциации прежнее значение .ext запоминается
  (Software\TMC Suite Qt\PrevAssoc), при удалении — возвращается. }
const
  AssocList = 'tpl:assoctpl soc:assocgrout tos:assocros toc:assocros dop:assocdn ex:assocfield ey:assocfield hz:assocfield em:assocfield amp:assocfield faz:assocfield frames:assocfield dat:assoccommon s:assoccommon s2p:assoccommon s4p:assoccommon s6p:assoccommon s8p:assoccommon tab:assoccommon t:assoccommon tf:assoccommon tt:assoccommon ';
  PrevKey = 'Software\TMC Suite Qt\PrevAssoc';

function AssocRoot: Integer;
begin
  if IsAdminInstallMode then
    Result := HKLM
  else
    Result := HKCU;
end;

procedure SavePrevAssoc;
var
  Rest, Item, Ext, Task, Old: String;
  P, C: Integer;
begin
  Rest := AssocList;
  P := Pos(' ', Rest);
  while P > 0 do
  begin
    Item := Copy(Rest, 1, P - 1);
    Rest := Copy(Rest, P + 1, Length(Rest));
    C := Pos(':', Item);
    Ext := Copy(Item, 1, C - 1);
    Task := Copy(Item, C + 1, Length(Item));
    if WizardIsTaskSelected(Task) and
       RegQueryStringValue(AssocRoot, 'Software\Classes\.' + Ext, '', Old) and
       (Old <> '') and (Pos('{#ProgPrefix}.', Old) <> 1) then
      RegWriteStringValue(AssocRoot, PrevKey, Ext, Old);
    P := Pos(' ', Rest);
  end;
end;

procedure RestorePrevAssoc;
var
  Names: TArrayOfString;
  I: Integer;
  Old, Cur: String;
begin
  if not RegGetValueNames(AssocRoot, PrevKey, Names) then
    Exit;
  for I := 0 to GetArrayLength(Names) - 1 do
    if RegQueryStringValue(AssocRoot, PrevKey, Names[I], Old) then
      if not RegQueryStringValue(AssocRoot, 'Software\Classes\.' + Names[I], '', Cur) or (Cur = '') then
        RegWriteStringValue(AssocRoot, 'Software\Classes\.' + Names[I], '', Old);
  RegDeleteKeyIncludingSubkeys(AssocRoot, PrevKey);
  RegDeleteKeyIfEmpty(AssocRoot, 'Software\TMC Suite Qt');
end;

{ --- оповещение оболочки о смене ассоциаций без ожидания (см. ChangesAssociations=no) --- }
procedure SHChangeNotify(wEventId, uFlags: Integer; dwItem1, dwItem2: Integer);
  external 'SHChangeNotify@shell32.dll stdcall';

procedure NotifyAssocChanged;
begin
  { SHCNE_ASSOCCHANGED = $08000000; SHCNF_IDLIST = 0, SHCNF_FLUSHNOWAIT = $2000 }
  SHChangeNotify($08000000, $2000, 0, 0);
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
    SavePrevAssoc;
  if CurStep = ssPostInstall then
    NotifyAssocChanged;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    RestorePrevAssoc;
    NotifyAssocChanged;
  end;
end;

{ --- уже установленная копия ---
  Inno Setup молча ставит поверх прежней установки; предупреждаем явно. Если прежняя копия
  стоит в другом режиме (для всех / только для себя), новая будет второй копией. }
const
  UninstKey = 'Software\Microsoft\Windows\CurrentVersion\Uninstall\';

function InstalledAt(Root: Integer; var Ver, Dir: String): Boolean;
var
  Key: String;
begin
  { AppIdStr записан с удвоенной «{» (экранирование Inno) — первая лишняя }
  Key := UninstKey + Copy('{#AppIdStr}', 2, Length('{#AppIdStr}')) + '_is1';
  Result := RegQueryStringValue(Root, Key, 'InstallLocation', Dir);
  if Result then
  begin
    if not RegQueryStringValue(Root, Key, 'DisplayVersion', Ver) then
      Ver := '';
    Dir := RemoveBackslashUnlessRoot(Dir);
  end;
end;

function InitializeSetup: Boolean;
var
  Ver, Dir, Msg: String;
  SameRoot, OtherRoot: Integer;
  OtherMode, ThisMode: String;
begin
  Result := True;
  if IsAdminInstallMode then
  begin
    SameRoot := HKLM64; OtherRoot := HKCU;
    ThisMode := CustomMessage('ModeAll'); OtherMode := CustomMessage('ModeMe');
  end else
  begin
    SameRoot := HKCU; OtherRoot := HKLM64;
    ThisMode := CustomMessage('ModeMe'); OtherMode := CustomMessage('ModeAll');
  end;
  if InstalledAt(SameRoot, Ver, Dir) then
  begin
    Msg := FmtMessage(CustomMessage('AlreadyInstalled'), [Ver, Dir]);
    Result := SuppressibleMsgBox(Msg, mbConfirmation, MB_YESNO, IDYES) = IDYES;
  end
  else if InstalledAt(OtherRoot, Ver, Dir) then
  begin
    Msg := FmtMessage(CustomMessage('AlreadyOtherMode'), [Ver, Dir, OtherMode, ThisMode]);
    Result := SuppressibleMsgBox(Msg, mbConfirmation, MB_YESNO or MB_DEFBUTTON2, IDYES) = IDYES;
  end;
end;

procedure InitializeWizard;
begin
  FinishBuilt := False;
  { заголовки страниц — акцентный бирюзовый, пояснения — приглушённый текст }
  Paint(WizardForm.PageNameLabel, ColAccent, True);
  Paint(WizardForm.PageDescriptionLabel, ColTextDim, False);
  { крупные заголовки «Приветствие» / «Завершение» }
  Paint(WizardForm.WelcomeLabel1, ColAccent, False);
  Paint(WizardForm.FinishedHeadingLabel, ColAccent, False);
  { поля и текстовые окна }
  { Окно лицензии (RichEdit) оставлено встроенному стилю: при смене фона RichEdit
    приглушает текст и подсвечивает ссылки — читаемость хуже. }
  PaintMemo(WizardForm.ReadyMemo);
  PaintEdit(WizardForm.DirEdit);
  PaintEdit(WizardForm.GroupEdit);
  { последняя страница строится сразу, а не при показе — без «старой» раскладки на миг }
  BuildFinishLinks;
end;
