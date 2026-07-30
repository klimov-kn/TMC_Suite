; ============================================================================
;  TMC_Suite.iss — установщик программного комплекса TMC Suite
;  Компилятор: Inno Setup 6 (ISCC.exe).  Кодировка скрипта: UTF-8 с BOM.
;
;  Что упаковывается:
;    - программы win32 и/или win64 (выбор пользователя)  -> {app}\bin\winXX
;    - руководство пользователя (Sphinx HTML)            -> {app}\docs\user-manual
;    - образцы данных SAMPLE_R                            -> {app}\samples\SAMPLE_R
;  В реестр пишется путь установки (HKLM\Software\TMC Suite\InstallPath),
;  чтобы программы находили документацию из окна About независимо от того,
;  куда установлены и откуда запущены (см. src\Include\TmcDocLink.h).
;
;  Программы слинкованы статически (UseOfMfc=Static, /MT) — VC++ Redistributable
;  НЕ требуется, всё работает «из коробки».
;
;  ВНИМАНИЕ: все пути в [Setup] и [Files] заданы относительно SourceDir (=..,
;  корень проекта), поэтому ресурсы установщика начинаются с "installer\".
; ============================================================================

#define AppName "TMC Suite"
#define AppVer "1.1"
#define AppPublisher "К. Н. Климов"
#define AppURL "http://www.tamic.ru"
#define AppYear "2026"

[Setup]
AppId={{B7E9F3A2-1C4D-4E8A-9F2B-6D5A8C3E1B07}
AppName={#AppName}
AppVersion={#AppVer}
AppVerName={#AppName} {#AppVer}
AppPublisher={#AppPublisher}
AppPublisherURL={#AppURL}
AppSupportURL={#AppURL}
DefaultDirName={autopf}\TMC Suite
DefaultGroupName=TMC Suite
AllowNoIcons=yes
LicenseFile=installer\LICENSE_ru.txt
OutputDir=installer\Output
OutputBaseFilename=TMC_Suite_Setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
WizardImageFile=installer\assets\wizard-large.bmp
WizardSmallImageFile=installer\assets\wizard-small.bmp
SetupIconFile=installer\assets\setup.ico
SourceDir=..
PrivilegesRequired=admin
; Мы намеренно пишем в HKCU пути к вьюверам для ядер (см. [Registry]): счётные ядра
; на MFC читают эти значения ТОЛЬКО из HKCU. При установке «для себя» (админ
; подтверждает UAC своей учётной записью) это верный куст. Директива снимает
; предупреждение компилятора о per-user изменениях в admin-режиме — оно ожидаемо.
UsedUserAreasWarning=no
ArchitecturesInstallIn64BitMode=x64compatible
DisableWelcomePage=no
UninstallDisplayName=TMC Suite
UninstallDisplayIcon={app}\bin\{code:PrimaryBinName}\TMCGROUT.exe

[Languages]
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; --- 64-битные программы ---
Source: "dist\win64\bin\*"; DestDir: "{app}\bin\win64"; Flags: ignoreversion recursesubdirs; Check: Want64
; --- 32-битные программы ---
Source: "dist\win32\bin\*"; DestDir: "{app}\bin\win32"; Flags: ignoreversion recursesubdirs; Check: Want32
; --- документация: руководство пользователя (Sphinx HTML) ---
Source: "docs\user-manual\sphinx\_build\html\*"; DestDir: "{app}\docs\user-manual"; Flags: ignoreversion recursesubdirs createallsubdirs
; --- образцы данных ---
Source: "samples\SAMPLE_R\*"; DestDir: "{app}\samples\SAMPLE_R"; Flags: ignoreversion recursesubdirs createallsubdirs
; --- лицензионное соглашение ---
Source: "installer\LICENSE_ru.txt"; DestDir: "{app}"; DestName: "LICENSE_ru.txt"; Flags: ignoreversion
Source: "LICENSE"; DestDir: "{app}"; DestName: "LICENSE.txt"; Flags: ignoreversion
; --- иконки соцсетей и сплэш (во временную папку, не устанавливаются) ---
Source: "installer\assets\github.bmp"; Flags: dontcopy
Source: "installer\assets\researchgate.bmp"; Flags: dontcopy
Source: "installer\assets\linkedin.bmp"; Flags: dontcopy
Source: "installer\assets\elibrary.bmp"; Flags: dontcopy
Source: "installer\assets\splash.bmp"; Flags: dontcopy

[Icons]
; --- меню Пуск (папка "TMC Suite") — основная (рекомендуемая) версия ---
Name: "{group}\TMCGROUT — просмотр S-матриц";       Filename: "{app}\bin\{code:PrimaryBinName}\TMCGROUT.exe"
Name: "{group}\TMCROS — выходной сигнал";            Filename: "{app}\bin\{code:PrimaryBinName}\TMCROS.exe"
Name: "{group}\TMC_DN — диаграммы направленности";   Filename: "{app}\bin\{code:PrimaryBinName}\TMC_DN.exe"
Name: "{group}\PlanarRT H (tmc_rth)";                Filename: "{app}\bin\{code:PrimaryBinName}\tmc_rth.exe"
Name: "{group}\PlanarRT X (tmc_rtx)";                Filename: "{app}\bin\{code:PrimaryBinName}\tmc_rtx.exe"
Name: "{group}\FieldView — просмотр полей";          Filename: "{app}\bin\{code:PrimaryBinName}\FieldView.exe"
Name: "{group}\Руководство пользователя";            Filename: "{app}\docs\user-manual\index.html"
Name: "{group}\Удалить TMC Suite";                   Filename: "{uninstallexe}"
; --- рабочий стол (по галочке) ---
Name: "{autodesktop}\TMCGROUT";   Filename: "{app}\bin\{code:PrimaryBinName}\TMCGROUT.exe"; Tasks: desktopicon
Name: "{autodesktop}\TMCROS";     Filename: "{app}\bin\{code:PrimaryBinName}\TMCROS.exe";   Tasks: desktopicon
Name: "{autodesktop}\TMC_DN";     Filename: "{app}\bin\{code:PrimaryBinName}\TMC_DN.exe";   Tasks: desktopicon
Name: "{autodesktop}\FieldView";  Filename: "{app}\bin\{code:PrimaryBinName}\FieldView.exe"; Tasks: desktopicon

[Registry]
; путь установки — программы читают его из About для поиска документации
Root: HKA; Subkey: "Software\TMC Suite"; ValueType: string; ValueName: "InstallPath"; ValueData: "{app}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\TMC Suite"; ValueType: string; ValueName: "Version";     ValueData: "{#AppVer}"

; ---------------------------------------------------------------------------
;  Предварительная запись путей к вьюверам для счётных ядер (идея К.Н. Климова).
;
;  Ядра tmc_rth (PlanarRT_H) и tmc_rtx (PlanarRT_X) вызывают внешние программы
;  (FieldView, TMCROS, TMCGROUT, TMC_DN) по кнопке/меню. При первом вызове ядро
;  просит указать exe вручную и запоминает путь в реестре (MFC WriteProfileString).
;  Здесь мы прописываем эти пути заранее — тогда ядро сразу запускает вьювер и
;  ничего не спрашивает, а пользователь не сможет случайно указать вьювер другой
;  разрядности (win32-ядро + win64-FieldView и наоборот — поле бы не отобразилось).
;
;  ВАЖНО про раскладку реестра:
;   * MFC хранит эти значения ТОЛЬКО в HKEY_CURRENT_USER (не в HKLM), поэтому
;     Root: HKCU (а не HKA). Установщик под admin пишет в куст того пользователя,
;     который подтвердил UAC (обычный сценарий — установка «для себя»).
;   * Раздел один и тот же для обеих разрядностей одного ядра
;     (win32 и win64 tmc_rth читают одну и ту же ветку). Поэтому пути указываем на
;     ОСНОВНУЮ выбранную платформу {code:PrimaryBinName} — так же, как ярлыки меню
;     «Пуск». При установке «обе версии» основной считается win64.
;   * Имена веток/значений взяты 1:1 из кода ядер (PlanRT_HDoc.h):
;       раздел  PLANRT_H_RAZDEL_INI            = "PlanRT_H Config"
;       Field   PLANRT_H_EXTERNFIELDVIEWERNAME = "ExternFieldViewerName"  -> FieldView
;       Signal  PLANRT_H_EXTERNVIEWERNAME      = "ExternViewerName"       -> TMCROS
;       S-matr  PLANRT_H_EXTERNSMVIEWERNAME    = "ExternSmatrixViewerName"-> TMCGROUT
;       DirPat  PLANRT_H_EXTERNDIRPATVIEWERNAM = "ExternDirectPatViewerName" -> TMC_DN
;       Editor  PLANRT_H_EXTERNEDITORNAME      = "ExternEditorName"       -> notepad
;     ProfileName (имя приложения): "Planar RT H analyzer" / "Planar RT X analyzer".
; ---------------------------------------------------------------------------

; --- ядро H: HKCU\Software\PlanarRT_H\Planar RT H analyzer\PlanRT_H Config ---
Root: HKCU; Subkey: "Software\PlanarRT_H\Planar RT H analyzer\PlanRT_H Config"; ValueType: string; ValueName: "ExternFieldViewerName";    ValueData: "{app}\bin\{code:PrimaryBinName}\FieldView.exe"; Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\PlanarRT_H\Planar RT H analyzer\PlanRT_H Config"; ValueType: string; ValueName: "ExternViewerName";         ValueData: "{app}\bin\{code:PrimaryBinName}\TMCROS.exe";    Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\PlanarRT_H\Planar RT H analyzer\PlanRT_H Config"; ValueType: string; ValueName: "ExternSmatrixViewerName";  ValueData: "{app}\bin\{code:PrimaryBinName}\TMCGROUT.exe";  Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\PlanarRT_H\Planar RT H analyzer\PlanRT_H Config"; ValueType: string; ValueName: "ExternDirectPatViewerName"; ValueData: "{app}\bin\{code:PrimaryBinName}\TMC_DN.exe";    Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\PlanarRT_H\Planar RT H analyzer\PlanRT_H Config"; ValueType: string; ValueName: "ExternEditorName";         ValueData: "{sys}\notepad.exe";                            Flags: uninsdeletevalue

; --- ядро X: HKCU\Software\PlanarRT_X\Planar RT X analyzer\PlanRT_H Config ---
Root: HKCU; Subkey: "Software\PlanarRT_X\Planar RT X analyzer\PlanRT_H Config"; ValueType: string; ValueName: "ExternFieldViewerName";    ValueData: "{app}\bin\{code:PrimaryBinName}\FieldView.exe"; Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\PlanarRT_X\Planar RT X analyzer\PlanRT_H Config"; ValueType: string; ValueName: "ExternViewerName";         ValueData: "{app}\bin\{code:PrimaryBinName}\TMCROS.exe";    Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\PlanarRT_X\Planar RT X analyzer\PlanRT_H Config"; ValueType: string; ValueName: "ExternSmatrixViewerName";  ValueData: "{app}\bin\{code:PrimaryBinName}\TMCGROUT.exe";  Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\PlanarRT_X\Planar RT X analyzer\PlanRT_H Config"; ValueType: string; ValueName: "ExternDirectPatViewerName"; ValueData: "{app}\bin\{code:PrimaryBinName}\TMC_DN.exe";    Flags: uninsdeletevalue
Root: HKCU; Subkey: "Software\PlanarRT_X\Planar RT X analyzer\PlanRT_H Config"; ValueType: string; ValueName: "ExternEditorName";         ValueData: "{sys}\notepad.exe";                            Flags: uninsdeletevalue

[Code]
var
  VersionPage: TInputOptionWizardPage;
  AnimBar: TPanel;
  AnimDir: Integer;
  FinishBuilt: Boolean;
  AnimCb: LongWord;
  AnimTimerId: LongWord;
  AnimActive: Boolean;

function SetTimer(hWnd, nIDEvent, uElapse, lpTimerFunc: LongWord): LongWord; external 'SetTimer@user32.dll stdcall';
function KillTimer(hWnd, nIDEvent: LongWord): LongWord; external 'KillTimer@user32.dll stdcall';

{ --- стартовый сплэш-экран (логотип ~768x512, показывается ~2 секунды) --- }
procedure ShowSplash;
var
  f: TForm;
  img: TBitmapImage;
begin
  ExtractTemporaryFile('splash.bmp');
  f := TForm.Create(nil);
  f.BorderStyle := bsNone;
  f.Position := poScreenCenter;
  img := TBitmapImage.Create(f);
  img.Parent := f;
  img.AutoSize := True;
  img.Bitmap.LoadFromFile(ExpandConstant('{tmp}\splash.bmp'));
  img.Left := 0;
  img.Top := 0;
  f.ClientWidth := img.Width;
  f.ClientHeight := img.Height;
  f.Show;
  f.Update;
  Sleep(2000);
  f.Close;
  f.Free;
end;

function InitializeSetup(): Boolean;
begin
  try
    ShowSplash;
  except
  end;
  Result := True;
end;

{ --- какие версии ставить (по выбору на странице версии) --- }
function Want64: Boolean;
begin
  if not IsWin64 then
    Result := False
  else
    Result := (VersionPage.SelectedValueIndex = 0) or (VersionPage.SelectedValueIndex = 2);
end;

function Want32: Boolean;
begin
  if not IsWin64 then
    Result := True
  else
    Result := (VersionPage.SelectedValueIndex = 1) or (VersionPage.SelectedValueIndex = 2);
end;

{ имя подпапки основной версии для ярлыков и иконки удаления }
function PrimaryBinName(Param: String): String;
begin
  if Want64 then Result := 'win64' else Result := 'win32';
end;

{ --- анимация приветственной страницы: бегущая полоса --- }
procedure AnimTick(Sender: TObject);
var
  x, minX, maxX: Integer;
begin
  minX := ScaleX(176);
  maxX := WizardForm.WelcomePage.Width - AnimBar.Width - ScaleX(40);
  if maxX < minX then maxX := minX;
  x := AnimBar.Left + AnimDir * ScaleX(6);
  if x <= minX then begin x := minX; AnimDir := 1; end;
  if x >= maxX then begin x := maxX; AnimDir := -1; end;
  AnimBar.Left := x;
end;

procedure TimerProc(H, Msg, IdEvent, Time: LongWord);
begin
  AnimTick(nil);
end;

procedure StartAnim;
begin
  if not AnimActive then
  begin
    AnimTimerId := SetTimer(0, 0, 30, AnimCb);
    AnimActive := True;
  end;
end;

procedure StopAnim;
begin
  if AnimActive then
  begin
    KillTimer(0, AnimTimerId);
    AnimActive := False;
  end;
end;

{ открыть ссылку (URL хранится в свойстве Hint элемента) }
procedure IconClick(Sender: TObject);
var
  ec: Integer;
begin
  ShellExec('open', (Sender as TControl).Hint, '', '', SW_SHOW, ewNoWait, ec);
end;

{ один кликабельный логотип (без подписи) на финишной странице }
procedure AddIcon(X, Y: Integer; FileName, URL: String);
var
  img: TBitmapImage;
begin
  ExtractTemporaryFile(FileName);
  img := TBitmapImage.Create(WizardForm);
  img.Parent := WizardForm.FinishedPage;
  img.Bitmap.LoadFromFile(ExpandConstant('{tmp}\') + FileName);
  img.Cursor := crHand;
  img.Hint := URL;
  img.ShowHint := True;
  img.OnClick := @IconClick;
  img.SetBounds(X, Y, 32, 32);
end;

{ построить блок ссылок на финишной странице (один раз) }
procedure BuildFinishLinks;
var
  hdr, site, year: TNewStaticText;
  L, W, y0, iconW, gap, total, startX, step, yIcons: Integer;
begin
  if FinishBuilt then exit;
  FinishBuilt := True;

  { сократить стандартный текст и зафиксировать его высоту, чтобы не наезжал }
  WizardForm.FinishedLabel.AutoSize := False;
  WizardForm.FinishedLabel.Caption := 'Программа TMC Suite установлена на ваш компьютер.';
  WizardForm.FinishedLabel.Height := ScaleY(34);

  L := WizardForm.FinishedLabel.Left;
  W := WizardForm.FinishedLabel.Width;
  y0 := WizardForm.FinishedLabel.Top + WizardForm.FinishedLabel.Height + ScaleY(20);

  { заголовок }
  hdr := TNewStaticText.Create(WizardForm);
  hdr.Parent := WizardForm.FinishedPage;
  hdr.Caption := 'Ресурсы проекта:';
  hdr.SetBounds(L, y0, W, ScaleY(15));

  { ссылка на сайт (отдельная строка) }
  site := TNewStaticText.Create(WizardForm);
  site.Parent := WizardForm.FinishedPage;
  site.Caption := 'http://www.tamic.ru';
  site.Font.Color := $00CC6600;
  site.Font.Style := [fsUnderline];
  site.Cursor := crHand;
  site.Hint := 'http://www.tamic.ru';
  site.ShowHint := True;
  site.OnClick := @IconClick;
  site.SetBounds(L, y0 + ScaleY(20), W, ScaleY(16));

  { ряд логотипов — без подписей, по центру области, равные промежутки }
  iconW := 32;
  gap := ScaleX(26);
  step := iconW + gap;
  total := 4 * iconW + 3 * gap;
  startX := L + (W - total) div 2;
  if startX < L then startX := L;
  yIcons := y0 + ScaleY(50);
  AddIcon(startX,            yIcons, 'github.bmp',       'https://github.com/knklimov/tmcsuit');
  AddIcon(startX + step,     yIcons, 'researchgate.bmp', 'https://www.researchgate.net/profile/Konstantin-Klimov');
  AddIcon(startX + 2 * step, yIcons, 'linkedin.bmp',     'https://www.linkedin.com/in/konstantinklimov');
  AddIcon(startX + 3 * step, yIcons, 'elibrary.bmp',     'https://www.elibrary.ru/author_items.asp?authorid=121878');

  { год — снизу по центру области; сначала текст, затем год }
  year := TNewStaticText.Create(WizardForm);
  year.Parent := WizardForm.FinishedPage;
  year.AutoSize := True;
  year.Caption := 'TMC Suite — установщик   © {#AppYear}';
  year.Top := WizardForm.FinishedPage.Height - ScaleY(26);
  year.Left := L + (W - year.Width) div 2;
  if year.Left < L then year.Left := L;
end;

procedure InitializeWizard;
begin
  FinishBuilt := False;

  { страница выбора версии — после страницы выбора папки }
  VersionPage := CreateInputOptionPage(wpSelectDir,
    'Выбор версии для установки',
    'Какую версию программ установить?',
    'TMC Suite поставляется в 32- и 64-битной версиях. ' +
    'Рекомендуемая для вашей системы отмечена ниже.',
    True, False);
  if IsWin64 then
  begin
    VersionPage.Add('64-битная версия (x64)   —   рекомендуется для вашей системы');
    VersionPage.Add('32-битная версия (x86)');
    VersionPage.Add('Обе версии (32 и 64 бит)');
    VersionPage.SelectedValueIndex := 0;
  end
  else
  begin
    VersionPage.Add('32-битная версия (x86)   —   рекомендуется для вашей системы');
    VersionPage.SelectedValueIndex := 0;
  end;

  { анимированная полоса на приветственной странице }
  AnimBar := TPanel.Create(WizardForm);
  AnimBar.Parent := WizardForm.WelcomePage;
  AnimBar.BevelOuter := bvNone;
  AnimBar.Color := clHighlight;
  AnimBar.Caption := '';
  AnimBar.SetBounds(ScaleX(176), ScaleY(205), ScaleX(48), ScaleY(4));
  AnimDir := 1;

  AnimActive := False;
  AnimCb := CreateCallback(@TimerProc);
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpWelcome then StartAnim else StopAnim;
  if CurPageID = wpFinished then
    BuildFinishLinks;
end;
