// Файл создан rc2ui из TMCGROUT.rc.
//
// Что здесь. Слой совместимости (src/compat/include/mfc_dialog.h) при вызове
// CDialog::DoModal() ищет по числовому идентификатору диалога функцию разметки,
// а элементы внутри — по символьному имени. Обе таблицы заполняются здесь:
//   * функция разметки — вызов setupUi() из заголовка, который uic делает
//     из соответствующего .ui;
//   * имена элементов — все определения из resource.h.
// Если один номер описан несколькими именами, берётся первое по resource.h:
// именно оно стоит в objectName виджетов.
//
// Почему номера записаны числами, а resource.h не включён. objectName виджета
// обязан совпадать с именем элемента (IDC_GRPAR_XMAX), и uic делает из этого
// имени имя переменной в ui_*.h. Но в resource.h то же самое имя — макрос
// (#define IDC_GRPAR_XMAX 1001), и препроцессор превратил бы объявление
// переменной в «QLineEdit *1001». Поэтому resource.h сюда не включается, а
// макросы, приходящие из слоя совместимости (IDOK, IDCANCEL), снимаются ниже.
// Числовые значения взяты из resource.h на момент порождения файла; повторный
// запуск rc2ui обновит их вместе с разметкой.

/// @file dialog_registry.cpp
/// @brief Таблица диалогов TMCGROUT для слоя совместимости: номер окна -> функция разметки, номер элемента -> имя.
///
/// Часть модуля «Вьювер TMCGROUT». Порождён скриптом из ресурса Windows-версии вместе с
/// файлами `.ui`.
/// @see https://www.tamic.ru/docs/code/compat.html#rc-dialogs
#include <QDialog>

#include "afxwin.h"     // MfcDialogRegistry

// Снять макросы-идентификаторы: дальше эти имена — имена переменных в ui_*.h.
#undef IDCANCEL
#undef IDC_ADD_CHARACTERISTICS
#undef IDC_GREDIT_DOCFILENAME
#undef IDC_GREDIT_GRAPHNAME1
#undef IDC_GREDIT_GRAPHNAME10
#undef IDC_GREDIT_GRAPHNAME11
#undef IDC_GREDIT_GRAPHNAME12
#undef IDC_GREDIT_GRAPHNAME13
#undef IDC_GREDIT_GRAPHNAME14
#undef IDC_GREDIT_GRAPHNAME15
#undef IDC_GREDIT_GRAPHNAME16
#undef IDC_GREDIT_GRAPHNAME17
#undef IDC_GREDIT_GRAPHNAME18
#undef IDC_GREDIT_GRAPHNAME19
#undef IDC_GREDIT_GRAPHNAME2
#undef IDC_GREDIT_GRAPHNAME20
#undef IDC_GREDIT_GRAPHNAME3
#undef IDC_GREDIT_GRAPHNAME4
#undef IDC_GREDIT_GRAPHNAME5
#undef IDC_GREDIT_GRAPHNAME6
#undef IDC_GREDIT_GRAPHNAME7
#undef IDC_GREDIT_GRAPHNAME8
#undef IDC_GREDIT_GRAPHNAME9
#undef IDC_GREDIT_INP1_1
#undef IDC_GREDIT_INP1_10
#undef IDC_GREDIT_INP1_11
#undef IDC_GREDIT_INP1_12
#undef IDC_GREDIT_INP1_13
#undef IDC_GREDIT_INP1_14
#undef IDC_GREDIT_INP1_15
#undef IDC_GREDIT_INP1_16
#undef IDC_GREDIT_INP1_17
#undef IDC_GREDIT_INP1_18
#undef IDC_GREDIT_INP1_19
#undef IDC_GREDIT_INP1_2
#undef IDC_GREDIT_INP1_20
#undef IDC_GREDIT_INP1_3
#undef IDC_GREDIT_INP1_4
#undef IDC_GREDIT_INP1_5
#undef IDC_GREDIT_INP1_6
#undef IDC_GREDIT_INP1_7
#undef IDC_GREDIT_INP1_8
#undef IDC_GREDIT_INP1_9
#undef IDC_GREDIT_INP2_1
#undef IDC_GREDIT_INP2_10
#undef IDC_GREDIT_INP2_11
#undef IDC_GREDIT_INP2_12
#undef IDC_GREDIT_INP2_13
#undef IDC_GREDIT_INP2_14
#undef IDC_GREDIT_INP2_15
#undef IDC_GREDIT_INP2_16
#undef IDC_GREDIT_INP2_17
#undef IDC_GREDIT_INP2_18
#undef IDC_GREDIT_INP2_19
#undef IDC_GREDIT_INP2_2
#undef IDC_GREDIT_INP2_20
#undef IDC_GREDIT_INP2_3
#undef IDC_GREDIT_INP2_4
#undef IDC_GREDIT_INP2_5
#undef IDC_GREDIT_INP2_6
#undef IDC_GREDIT_INP2_7
#undef IDC_GREDIT_INP2_8
#undef IDC_GREDIT_INP2_9
#undef IDC_GREDIT_MOD1_1
#undef IDC_GREDIT_MOD1_10
#undef IDC_GREDIT_MOD1_11
#undef IDC_GREDIT_MOD1_12
#undef IDC_GREDIT_MOD1_13
#undef IDC_GREDIT_MOD1_14
#undef IDC_GREDIT_MOD1_15
#undef IDC_GREDIT_MOD1_16
#undef IDC_GREDIT_MOD1_17
#undef IDC_GREDIT_MOD1_18
#undef IDC_GREDIT_MOD1_19
#undef IDC_GREDIT_MOD1_2
#undef IDC_GREDIT_MOD1_20
#undef IDC_GREDIT_MOD1_3
#undef IDC_GREDIT_MOD1_4
#undef IDC_GREDIT_MOD1_5
#undef IDC_GREDIT_MOD1_6
#undef IDC_GREDIT_MOD1_7
#undef IDC_GREDIT_MOD1_8
#undef IDC_GREDIT_MOD1_9
#undef IDC_GREDIT_MOD2_1
#undef IDC_GREDIT_MOD2_10
#undef IDC_GREDIT_MOD2_11
#undef IDC_GREDIT_MOD2_12
#undef IDC_GREDIT_MOD2_13
#undef IDC_GREDIT_MOD2_14
#undef IDC_GREDIT_MOD2_15
#undef IDC_GREDIT_MOD2_16
#undef IDC_GREDIT_MOD2_17
#undef IDC_GREDIT_MOD2_18
#undef IDC_GREDIT_MOD2_19
#undef IDC_GREDIT_MOD2_2
#undef IDC_GREDIT_MOD2_20
#undef IDC_GREDIT_MOD2_3
#undef IDC_GREDIT_MOD2_4
#undef IDC_GREDIT_MOD2_5
#undef IDC_GREDIT_MOD2_6
#undef IDC_GREDIT_MOD2_7
#undef IDC_GREDIT_MOD2_8
#undef IDC_GREDIT_MOD2_9
#undef IDC_GREDIT_OUTFLAG1
#undef IDC_GREDIT_OUTFLAG10
#undef IDC_GREDIT_OUTFLAG11
#undef IDC_GREDIT_OUTFLAG12
#undef IDC_GREDIT_OUTFLAG13
#undef IDC_GREDIT_OUTFLAG14
#undef IDC_GREDIT_OUTFLAG15
#undef IDC_GREDIT_OUTFLAG16
#undef IDC_GREDIT_OUTFLAG17
#undef IDC_GREDIT_OUTFLAG18
#undef IDC_GREDIT_OUTFLAG19
#undef IDC_GREDIT_OUTFLAG2
#undef IDC_GREDIT_OUTFLAG20
#undef IDC_GREDIT_OUTFLAG3
#undef IDC_GREDIT_OUTFLAG4
#undef IDC_GREDIT_OUTFLAG5
#undef IDC_GREDIT_OUTFLAG6
#undef IDC_GREDIT_OUTFLAG7
#undef IDC_GREDIT_OUTFLAG8
#undef IDC_GREDIT_OUTFLAG9
#undef IDC_GREDIT_SFILENAME1
#undef IDC_GREDIT_SFILENAME10
#undef IDC_GREDIT_SFILENAME11
#undef IDC_GREDIT_SFILENAME12
#undef IDC_GREDIT_SFILENAME13
#undef IDC_GREDIT_SFILENAME14
#undef IDC_GREDIT_SFILENAME15
#undef IDC_GREDIT_SFILENAME16
#undef IDC_GREDIT_SFILENAME17
#undef IDC_GREDIT_SFILENAME18
#undef IDC_GREDIT_SFILENAME19
#undef IDC_GREDIT_SFILENAME2
#undef IDC_GREDIT_SFILENAME20
#undef IDC_GREDIT_SFILENAME3
#undef IDC_GREDIT_SFILENAME4
#undef IDC_GREDIT_SFILENAME5
#undef IDC_GREDIT_SFILENAME6
#undef IDC_GREDIT_SFILENAME7
#undef IDC_GREDIT_SFILENAME8
#undef IDC_GREDIT_SFILENAME9
#undef IDC_GRPAR_AXSFLAG
#undef IDC_GRPAR_AYSFLAG
#undef IDC_GRPAR_DATFILENAME
#undef IDC_GRPAR_POINTFLAG
#undef IDC_GRPAR_POINTFLAG2
#undef IDC_GRPAR_POINTSIZE__
#undef IDC_GRPAR_XFORMAT
#undef IDC_GRPAR_XINGHZ
#undef IDC_GRPAR_XINHZ
#undef IDC_GRPAR_XINHZ2
#undef IDC_GRPAR_XINKHZ
#undef IDC_GRPAR_XINMHZ
#undef IDC_GRPAR_XMAX
#undef IDC_GRPAR_XMIN
#undef IDC_GRPAR_XUSERAXIESNAME
#undef IDC_GRPAR_XUSERUNIT
#undef IDC_GRPAR_YFORMAT
#undef IDC_GRPAR_YMAX
#undef IDC_GRPAR_YMIN
#undef IDC_GRPAR_YSKSV1
#undef IDC_STATIC
#undef IDC_STATICLEVEL1
#undef IDC_STATICLEVEL13
#undef IDC_STATICLEVEL14
#undef IDC_STATICLEVEL6
#undef IDC_STATICLEVEL7
#undef IDC_STATICLEVEL8
#undef IDC_TMC_DOCLINK
#undef IDD_ABOUTBOX
#undef IDD_DIALOG1
#undef IDD_DIALOG2
#undef IDD_DIALOGBARCOLORH
#undef IDD_DIALOGGRPARALL
#undef IDOK
#undef IDOK2
#undef IDOK3
#undef IDOK4
#undef IDOK5
#undef IDOK6
#undef IDOK7
#undef ID_ColorLevelAutoSet8
#undef ID_DEFAULT_GRID_FORMAT
#include "ui_IDD_DIALOG1.h"
#include "ui_IDD_DIALOG2.h"
#include "ui_IDD_ABOUTBOX.h"
#include "ui_IDD_DIALOGBARCOLORH.h"
#include "ui_IDD_DIALOGGRPARALL.h"

namespace {

/// @name Функции разметки диалогов
/// Строят виджеты окна по `.ui`: `IDD_DIALOG1` (Viewport), `IDD_DIALOG2` (Characteristics),
/// `IDD_ABOUTBOX`, `IDD_DIALOGBARCOLORH` (16 стилей линий), `IDD_DIALOGGRPARALL` (стиль линии).
///@{
/// Строит виджеты окна по его `.ui`.
/// @param[in,out] qDialog пустой `QDialog`, созданный `CDialog::DoModal()` слоя совместимости.
void tmcSetup_IDD_DIALOG1(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_DIALOG1 ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

void tmcSetup_IDD_DIALOG2(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_DIALOG2 ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

void tmcSetup_IDD_ABOUTBOX(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_ABOUTBOX ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

void tmcSetup_IDD_DIALOGBARCOLORH(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_DIALOGBARCOLORH ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

void tmcSetup_IDD_DIALOGGRPARALL(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_DIALOGGRPARALL ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

// Соответствие «номер -> символьное имя» для всех элементов из resource.h.
///@}

/// @brief Пара «номер элемента — имя объекта виджета».
struct TmcControlName
{
    /// Номер элемента из resource.h.
    UINT        id;
    /// Символьное имя элемента (имя объекта виджета в разметке).
    const char* name;
};

/// Все номера resource.h с именами; при совпадении номеров первым идёт имя, стоящее в разметке.
const TmcControlName kControlNames[] = {
    { 3, "IDOK2" },
    { 4, "IDOK3" },
    { 5, "IDOK4" },
    { 6, "IDOK5" },
    { 7, "IDOK6" },
    { 8, "ID_DEFAULT_GRID_FORMAT" },
    { 9, "IDOK7" },
    { 100, "IDD_ABOUTBOX" },
    { 3000, "IDC_TMC_DOCLINK" },
    { 107, "IDD_DIALOGBARCOLORH" },
    { 128, "IDR_MAINFRAME" },
    { 129, "IDR_TMCGROTYPE" },
    { 130, "IDD_DIALOG1" },
    { 131, "IDD_DIALOG2" },
    { 132, "IDD_DIALOGGRPARALL" },
    { 1000, "IDC_GRPAR_XMIN" },
    { 1001, "IDC_GRPAR_XMAX" },
    { 1002, "IDC_GRPAR_YMIN" },
    { 1003, "IDC_GRPAR_YMAX" },
    { 1004, "IDC_GRPAR_AXSFLAG" },
    { 1005, "IDC_GRPAR_AYSFLAG" },
    { 1006, "IDC_GRPAR_XUSERUNIT" },
    { 1007, "IDC_GRPAR_XUSERAXIESNAME" },
    { 1008, "IDC_GRPAR_YFORMAT" },
    { 1009, "IDC_GRPAR_XFORMAT" },
    { 1010, "IDC_GRPAR_POINTSIZE__" },
    { 1015, "IDC_GRPAR_DATFILENAME" },
    { 1029, "IDC_GRPAR_POINTFLAG" },
    { 1030, "IDC_GRPAR_POINTFLAG2" },
    { 1031, "IDC_STATICLEVEL1" },
    { 1032, "IDC_GREDIT_SFILENAME1" },
    { 1033, "IDC_GREDIT_OUTFLAG1" },
    { 1034, "IDC_GREDIT_DOCFILENAME" },
    { 1035, "IDC_ADD_CHARACTERISTICS" },
    { 1036, "IDC_STATICLEVEL6" },
    { 1037, "IDC_STATICLEVEL7" },
    { 1038, "IDC_STATICLEVEL8" },
    { 1039, "IDC_GRPAR_XINGHZ" },
    { 1040, "IDC_GRPAR_XINMHZ" },
    { 1041, "IDC_GRPAR_XINKHZ" },
    { 1042, "IDC_GRPAR_XINHZ" },
    { 1043, "IDC_GRPAR_XINHZ2" },
    { 1044, "IDC_STATICLEVEL13" },
    { 1046, "IDC_STATICLEVEL14" },
    { 1048, "IDC_GRPAR_YSKSV1" },
    { 1049, "IDC_GREDIT_SFILENAME2" },
    { 1050, "IDC_GREDIT_SFILENAME3" },
    { 1051, "IDC_GREDIT_SFILENAME4" },
    { 1052, "IDC_GREDIT_SFILENAME5" },
    { 1053, "IDC_GREDIT_SFILENAME6" },
    { 1054, "IDC_GREDIT_SFILENAME7" },
    { 1055, "IDC_GREDIT_SFILENAME8" },
    { 1056, "IDC_GREDIT_SFILENAME9" },
    { 1057, "IDC_GREDIT_SFILENAME10" },
    { 1058, "IDC_GREDIT_SFILENAME11" },
    { 1059, "IDC_GREDIT_SFILENAME12" },
    { 1060, "IDC_GREDIT_SFILENAME13" },
    { 1061, "IDC_GREDIT_SFILENAME14" },
    { 1062, "IDC_GREDIT_SFILENAME15" },
    { 1063, "IDC_GREDIT_SFILENAME16" },
    { 1064, "IDC_GREDIT_SFILENAME17" },
    { 1065, "IDC_GREDIT_SFILENAME18" },
    { 1066, "IDC_GREDIT_SFILENAME19" },
    { 1067, "IDC_GREDIT_SFILENAME20" },
    { 1068, "IDC_GREDIT_INP1_1" },
    { 1069, "IDC_GREDIT_INP1_2" },
    { 1070, "IDC_GREDIT_INP1_3" },
    { 1071, "IDC_GREDIT_INP1_4" },
    { 1072, "IDC_GREDIT_INP1_5" },
    { 1073, "IDC_GREDIT_INP1_6" },
    { 1074, "IDC_GREDIT_INP1_7" },
    { 1075, "IDC_GREDIT_INP1_8" },
    { 1076, "IDC_GREDIT_INP1_9" },
    { 1077, "IDC_GREDIT_INP1_10" },
    { 1078, "IDC_GREDIT_INP1_11" },
    { 1079, "IDC_GREDIT_INP1_12" },
    { 1080, "IDC_GREDIT_INP1_13" },
    { 1081, "IDC_GREDIT_INP1_14" },
    { 1082, "IDC_GREDIT_INP1_15" },
    { 1083, "IDC_GREDIT_INP1_16" },
    { 1084, "IDC_GREDIT_INP1_17" },
    { 1085, "IDC_GREDIT_INP1_18" },
    { 1086, "IDC_GREDIT_INP1_19" },
    { 1087, "IDC_GREDIT_INP1_20" },
    { 1089, "IDC_GREDIT_MOD1_1" },
    { 1090, "IDC_GREDIT_MOD1_2" },
    { 1091, "IDC_GREDIT_MOD1_3" },
    { 1092, "IDC_GREDIT_MOD1_4" },
    { 1093, "IDC_GREDIT_MOD1_5" },
    { 1094, "IDC_GREDIT_MOD1_6" },
    { 1095, "IDC_GREDIT_MOD1_7" },
    { 1096, "IDC_GREDIT_MOD1_8" },
    { 1097, "IDC_GREDIT_MOD1_9" },
    { 1098, "IDC_GREDIT_MOD1_10" },
    { 1099, "IDC_GREDIT_MOD1_11" },
    { 1100, "IDC_GREDIT_MOD1_12" },
    { 1101, "IDC_GREDIT_MOD1_13" },
    { 1102, "IDC_GREDIT_MOD1_14" },
    { 1103, "IDC_GREDIT_MOD1_15" },
    { 1104, "IDC_GREDIT_MOD1_16" },
    { 1105, "IDC_GREDIT_MOD1_17" },
    { 1106, "IDC_GREDIT_MOD1_18" },
    { 1107, "IDC_GREDIT_MOD1_19" },
    { 1108, "IDC_GREDIT_MOD1_20" },
    { 1109, "ID_ColorLevelAutoSet8" },
    { 1110, "IDC_GREDIT_INP2_1" },
    { 1111, "IDC_GREDIT_INP2_2" },
    { 1112, "IDC_GREDIT_INP2_3" },
    { 1113, "IDC_GREDIT_INP2_4" },
    { 1114, "IDC_GREDIT_INP2_5" },
    { 1115, "IDC_GREDIT_INP2_6" },
    { 1116, "IDC_GREDIT_INP2_7" },
    { 1117, "IDC_GREDIT_INP2_8" },
    { 1118, "IDC_GREDIT_INP2_9" },
    { 1119, "IDC_GREDIT_INP2_10" },
    { 1120, "IDC_GREDIT_INP2_11" },
    { 1121, "IDC_GREDIT_INP2_12" },
    { 1122, "IDC_GREDIT_INP2_13" },
    { 1123, "IDC_GREDIT_INP2_14" },
    { 1124, "IDC_GREDIT_INP2_15" },
    { 1125, "IDC_GREDIT_INP2_16" },
    { 1126, "IDC_GREDIT_INP2_17" },
    { 1127, "IDC_GREDIT_INP2_18" },
    { 1128, "IDC_GREDIT_INP2_19" },
    { 1129, "IDC_GREDIT_INP2_20" },
    { 1131, "IDC_GREDIT_MOD2_1" },
    { 1132, "IDC_GREDIT_MOD2_2" },
    { 1133, "IDC_GREDIT_MOD2_3" },
    { 1134, "IDC_GREDIT_MOD2_4" },
    { 1135, "IDC_GREDIT_MOD2_5" },
    { 1136, "IDC_GREDIT_MOD2_6" },
    { 1137, "IDC_GREDIT_MOD2_7" },
    { 1138, "IDC_GREDIT_MOD2_8" },
    { 1139, "IDC_GREDIT_MOD2_9" },
    { 1140, "IDC_GREDIT_MOD2_10" },
    { 1141, "IDC_GREDIT_MOD2_11" },
    { 1142, "IDC_GREDIT_MOD2_12" },
    { 1143, "IDC_GREDIT_MOD2_13" },
    { 1144, "IDC_GREDIT_MOD2_14" },
    { 1145, "IDC_GREDIT_MOD2_15" },
    { 1146, "IDC_GREDIT_MOD2_16" },
    { 1147, "IDC_GREDIT_MOD2_17" },
    { 1148, "IDC_GREDIT_MOD2_18" },
    { 1149, "IDC_GREDIT_MOD2_19" },
    { 1150, "IDC_GREDIT_MOD2_20" },
    { 1153, "IDC_GREDIT_GRAPHNAME1" },
    { 1154, "IDC_GREDIT_GRAPHNAME2" },
    { 1155, "IDC_GREDIT_GRAPHNAME3" },
    { 1156, "IDC_GREDIT_GRAPHNAME4" },
    { 1157, "IDC_GREDIT_GRAPHNAME5" },
    { 1158, "IDC_GREDIT_GRAPHNAME6" },
    { 1159, "IDC_GREDIT_GRAPHNAME7" },
    { 1160, "IDC_GREDIT_GRAPHNAME8" },
    { 1161, "IDC_GREDIT_GRAPHNAME9" },
    { 1162, "IDC_GREDIT_GRAPHNAME10" },
    { 1164, "IDC_GREDIT_GRAPHNAME11" },
    { 1165, "IDC_GREDIT_GRAPHNAME12" },
    { 1166, "IDC_GREDIT_GRAPHNAME13" },
    { 1167, "IDC_GREDIT_GRAPHNAME14" },
    { 1168, "IDC_GREDIT_GRAPHNAME15" },
    { 1169, "IDC_GREDIT_GRAPHNAME16" },
    { 1170, "IDC_GREDIT_GRAPHNAME17" },
    { 1171, "IDC_GREDIT_GRAPHNAME18" },
    { 1172, "IDC_GREDIT_GRAPHNAME19" },
    { 1173, "IDC_GREDIT_GRAPHNAME20" },
    { 1174, "IDC_GREDIT_OUTFLAG2" },
    { 1175, "IDC_GREDIT_OUTFLAG3" },
    { 1176, "IDC_GREDIT_OUTFLAG4" },
    { 1177, "IDC_GREDIT_OUTFLAG5" },
    { 1178, "IDC_GREDIT_OUTFLAG6" },
    { 1179, "IDC_GREDIT_OUTFLAG7" },
    { 1180, "IDC_GREDIT_OUTFLAG8" },
    { 1181, "IDC_GREDIT_OUTFLAG9" },
    { 1182, "IDC_GREDIT_OUTFLAG10" },
    { 1183, "IDC_GREDIT_OUTFLAG11" },
    { 1184, "IDC_GREDIT_OUTFLAG12" },
    { 1185, "IDC_GREDIT_OUTFLAG13" },
    { 1186, "IDC_GREDIT_OUTFLAG14" },
    { 1187, "IDC_GREDIT_OUTFLAG15" },
    { 1188, "IDC_GREDIT_OUTFLAG16" },
    { 1189, "IDC_GREDIT_OUTFLAG17" },
    { 1190, "IDC_GREDIT_OUTFLAG18" },
    { 1191, "IDC_GREDIT_OUTFLAG19" },
    { 1192, "IDC_GREDIT_OUTFLAG20" },
    { 32771, "ID_EDIT_EDIT" },
    { 32772, "ID_EDIT_GRAPHICSPARAMETERS" },
    { 32773, "ID_VIEW_GRAPHICS" },
    { 32774, "ID_EDIT_DOCUMENT" },
    { 32775, "ID_EDIT_ADDCHARACTERISTICS" },
    { 32778, "ID_VIEW_RESIZECTRLR" },
    { 32779, "ID_VIEW_RESIZEWINDOW" },
    { 32780, "ID_CONFIG_EDITOR" },
    { 32781, "ID_CONFIG_FONT" },
    { 32784, "ID_CONFIG_COLORPOINT" },
    { 32785, "ID_CONFIG_COLOR_AXIS" },
    { 32786, "ID_CONFIG_COLOR_GRID" },
    { 32787, "ID_CONFIG_COLOR_BACKGROUND" },
    { 32798, "ID_VIEW_CHANGEXMAXXMIN_DECRIMENT" },
    { 32799, "ID_VIEW_CHANGEXMAXXMIN_INCREMENT" },
    { 32800, "ID_VIEW_CHANGEYMAXYMIN_DECREMENT" },
    { 32801, "ID_VIEW_CHANGEYMAXYMIN_INCREMENT" },
    { 32806, "ID_VIEW_AUTOXSIZE" },
    { 32807, "ID_CONFIG_COLOR_GRAPHICS" },
    { 32808, "ID_VIEW_TRANSLATE_HOME" },
    { 32809, "ID_VIEW_AUTOYSIZE" },
    { 32810, "ID_VIEW_ZOOM_ZOOM" },
    { 32811, "ID_VIEW_ZOOM_ZOOMX" },
    { 32812, "ID_VIEW_ZOOM_ZOOMY" },
    { 32813, "ID_VIEW_TRANSLATE_END" },
    { 32822, "ID_VIEW_ZOOM_ZOOMP" },
    { 32823, "ID_VIEW_ZOOM_ZOOMXP" },
    { 32824, "ID_VIEW_ZOOM_ZOOMYP" },
    { 1, "IDOK" },
    { 2, "IDCANCEL" },
};

/// @brief Регистрирует функции разметки и имена элементов в `MfcDialogRegistry`.
/// @return true (значение нужно для однократного вызова через статическую переменную).
bool tmcInstall()
{
    MfcDialogRegistry::registerDialog(130 /* IDD_DIALOG1 */, &tmcSetup_IDD_DIALOG1);
    MfcDialogRegistry::registerDialog(131 /* IDD_DIALOG2 */, &tmcSetup_IDD_DIALOG2);
    MfcDialogRegistry::registerDialog(100 /* IDD_ABOUTBOX */, &tmcSetup_IDD_ABOUTBOX);
    MfcDialogRegistry::registerDialog(107 /* IDD_DIALOGBARCOLORH */, &tmcSetup_IDD_DIALOGBARCOLORH);
    MfcDialogRegistry::registerDialog(132 /* IDD_DIALOGGRPARALL */, &tmcSetup_IDD_DIALOGGRPARALL);

    const int count = int(sizeof(kControlNames) / sizeof(kControlNames[0]));
    for (int i = 0; i < count; ++i)
        MfcDialogRegistry::registerControlName(kControlNames[i].id,
                                               kControlNames[i].name);
    return true;
}

} // namespace

// Вызывается из кода программы один раз при запуске. Не полагаемся на
// инициализацию статических объектов: файл лежит в статической библиотеке, и
// без явной ссылки компоновщик мог бы его не подключить.
/// @brief Однократно регистрирует диалоги TMCGROUT (повторные вызовы ничего не делают).
void TmcInstallDialogs_tmcgrout()
{
    static const bool done = tmcInstall();
    (void)done;
}
