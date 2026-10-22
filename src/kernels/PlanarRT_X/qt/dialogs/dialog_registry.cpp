// Файл создан rc2ui из PlanRT_H.rc.
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
/// @brief Таблица диалогов программы для слоя совместимости: номер окна -> функция разметки, номер элемента -> имя.
///
/// Часть модуля «Оболочка счётного ядра». Порождён скриптом из ресурса Windows-версии
/// вместе с файлами `.ui`; у H и X свои копии (различается имя функции установки).
/// @see https://www.tamic.ru/docs/code/compat.html#rc-dialogs
#include <QDialog>

#include "afxwin.h"     // MfcDialogRegistry

// Снять макросы-идентификаторы: дальше эти имена — имена переменных в ui_*.h.
#undef IDCANCEL
#undef IDC_STATIC
#undef IDC_TMCBLOCKDX10
#undef IDC_TMCBLOCKDX7
#undef IDC_TMCBLOCKDX8
#undef IDC_TMCBLOCKDX9
#undef IDC_TMCBLOCKDY10
#undef IDC_TMCBLOCKDY2
#undef IDC_TMCBLOCKDY3
#undef IDC_TMCBLOCKDY4
#undef IDC_TMCBLOCKDY5
#undef IDC_TMCBLOCKDY6
#undef IDC_TMCBLOCKDY7
#undef IDC_TMCBLOCKDY8
#undef IDC_TMCBLOCKDY9
#undef IDC_TMCBLOCKMEMORY1
#undef IDC_TMCBLOCKNPOINTXY
#undef IDC_TMCBLOCKNTYPE
#undef IDC_TMCBLOCKXMAXBTEXT
#undef IDC_TMCBLOCKXMINBTEXT
#undef IDC_TMCBLOCKYMAXBTEXT
#undef IDC_TMCBLOCKYMINBTEXT
#undef IDC_TMCCALCTIMEEXPORT
#undef IDC_TMCCALCTIMEREFRESH
#undef IDC_TMCCALCTIMETEXT
#undef IDC_TMCSTATISTICSANGLEUNIT
#undef IDC_TMCSTATISTICSCALCTIME
#undef IDC_TMCSTATISTICSDELTA
#undef IDC_TMCSTATISTICSERRORMESSAGE
#undef IDC_TMCSTATISTICSFIELDOUT
#undef IDC_TMCSTATISTICSFILEOUT
#undef IDC_TMCSTATISTICSFREQ
#undef IDC_TMCSTATISTICSFREQUNIT
#undef IDC_TMCSTATISTICSLONGUNIT
#undef IDC_TMCSTATISTICSNBLOCK
#undef IDC_TMCSTATISTICSNEXTSTEP
#undef IDC_TMCSTATISTICSNSTEP
#undef IDC_TMCSTATISTICSPREVSTEP
#undef IDC_TMCSTATISTICST0
#undef IDC_TMCSTATISTICST2
#undef IDC_TMCSTATISTICSTIMEUNIT
#undef IDC_TMCSTATISTICSTMAX
#undef IDC_TMCSTATISTICSTMIN
#undef IDC_TMCSTATISTICSTOPOLOGYOUT
#undef IDC_TMCSTATISTICSVIEWBLOCKLIST2
#undef IDC_TMCSTATISTICSWAVELENGHT
#undef IDC_TMCSTATISTICSWAVELENGHTDELT
#undef IDC_TMCSTATISTICSXMAX
#undef IDC_TMCSTATISTICSXMIN
#undef IDC_TMCSTATISTICSYMAX
#undef IDC_TMCSTATISTICSYMIN
#undef IDC_TMC_DOCLINK
#undef IDC_TmcSoundEffects_Sound100
#undef IDC_TmcSoundEffects_Sound101
#undef IDC_TmcSoundEffects_Sound102
#undef IDC_TmcSoundEffects_Sound103
#undef IDC_TmcSoundEffects_Sound104
#undef IDC_TmcSoundEffects_Sound105
#undef IDC_TmcSoundEffects_Sound106
#undef IDC_TmcSoundEffects_Sound107
#undef IDC_TmcSoundEffects_Sound108
#undef IDC_TmcSoundEffects_Sound109
#undef IDC_TmcSoundEffects_Sound110
#undef IDC_TmcSoundEffects_Sound111
#undef IDC_TmcSoundEffects_Sound112
#undef IDC_TmcSoundEffects_Sound113
#undef IDC_TmcSoundEffects_Sound114
#undef IDC_TmcSoundEffects_Sound115
#undef IDC_TmcSoundEffects_Sound116
#undef IDC_TmcSoundEffects_Sound117
#undef IDC_TmcSoundEffects_Sound118
#undef IDC_TmcSoundEffects_Sound119
#undef IDC_TmcSoundEffects_Sound120
#undef IDC_TmcSoundEffects_Sound121
#undef IDC_TmcSoundEffects_Sound122
#undef IDC_TmcSoundEffects_Sound123
#undef IDC_TmcSoundEffects_Sound124
#undef IDC_TmcSoundEffects_Sound125
#undef IDC_TmcSoundEffects_Sound126
#undef IDC_TmcSoundEffects_Sound127
#undef IDC_TmcSoundEffects_Sound128
#undef IDC_TmcSoundEffects_Sound129
#undef IDC_TmcSoundEffects_Sound130
#undef IDC_TmcSoundEffects_Sound131
#undef IDC_TmcSoundEffects_Sound132
#undef IDC_TmcSoundEffects_Sound133
#undef IDC_TmcSoundEffects_Sound134
#undef IDC_TmcSoundEffects_Sound135
#undef IDC_TmcSoundEffects_Sound136
#undef IDC_TmcSoundEffects_Sound137
#undef IDC_TmcSoundEffects_Sound138
#undef IDC_TmcSoundEffects_Sound139
#undef IDC_TmcSoundEffects_Sound140
#undef IDC_TmcSoundEffects_Sound141
#undef IDC_TmcSoundEffects_Sound142
#undef IDC_TmcSoundEffects_Sound143
#undef IDC_TmcSoundEffects_Sound144
#undef IDC_TmcSoundEffects_Sound145
#undef IDC_TmcSoundEffects_Sound146
#undef IDC_TmcSoundEffects_Sound147
#undef IDC_TmcSoundEffects_Sound148
#undef IDC_TmcSoundEffects_Sound149
#undef IDC_TmcSoundEffects_Sound150
#undef IDC_TmcSoundEffects_Sound151
#undef IDC_TmcSoundEffects_Sound152
#undef IDC_TmcSoundEffects_Sound153
#undef IDC_TmcSoundEffects_Sound154
#undef IDC_TmcSoundEffects_Sound155
#undef IDC_TmcSoundEffects_Sound156
#undef IDC_TmcSoundEffects_Sound157
#undef IDC_TmcSoundEffects_Sound158
#undef IDC_TmcSoundEffects_Sound159
#undef IDC_TmcSoundEffects_Sound160
#undef IDC_TmcSoundEffects_Sound161
#undef IDC_TmcSoundEffects_Sound162
#undef IDC_TmcSoundEffects_Sound163
#undef IDC_TmcSoundEffects_Sound164
#undef IDC_TmcSoundEffects_Sound165
#undef IDC_TmcSoundEffects_Sound166
#undef IDC_TmcSoundEffects_Sound167
#undef IDC_TmcSoundEffects_Sound168
#undef IDC_TmcSoundEffects_Sound169
#undef IDC_TmcSoundEffects_Sound170
#undef IDC_TmcSoundEffects_Sound171
#undef IDC_TmcSoundEffects_Sound172
#undef IDC_TmcSoundEffects_Sound173
#undef IDC_TmcSoundEffects_Sound174
#undef IDC_TmcSoundEffects_Sound175
#undef IDC_TmcSoundEffects_Sound176
#undef IDC_TmcSoundEffects_Sound177
#undef IDC_TmcSoundEffects_Sound178
#undef IDC_TmcSoundEffects_Sound179
#undef IDC_TmcSoundEffects_Sound180
#undef IDC_TmcSoundEffects_Sound181
#undef IDC_TmcSoundEffects_Sound182
#undef IDC_TmcSoundEffects_Sound183
#undef IDC_TmcSoundEffects_Sound184
#undef IDC_TmcSoundEffects_Sound185
#undef IDC_TmcSoundEffects_Sound186
#undef IDC_TmcSoundEffects_Sound187
#undef IDC_TmcSoundEffects_Sound188
#undef IDC_TmcSoundEffects_Sound189
#undef IDC_TmcSoundEffects_Sound190
#undef IDC_TmcSoundEffects_Sound191
#undef IDC_TmcSoundEffects_Sound192
#undef IDC_TmcSoundEffects_Sound193
#undef IDC_TmcSoundEffects_Sound194
#undef IDC_TmcSoundEffects_Sound195
#undef IDC_TmcSoundEffects_Sound196
#undef IDC_TmcSoundEffects_Sound197
#undef IDC_TmcSoundEffects_Sound198
#undef IDC_TmcSoundEffects_Sound199
#undef IDC_TmcSoundEffects_Sound200
#undef IDC_TmcSoundEffects_Sound201
#undef IDC_TmcSoundEffects_Sound202
#undef IDC_TmcSoundEffects_Sound203
#undef IDC_TmcSoundEffects_Sound204
#undef IDC_TmcSoundEffects_Sound205
#undef IDC_TmcSoundEffects_Sound206
#undef IDC_TmcSoundEffects_Sound207
#undef IDC_TmcSoundEffects_Sound208
#undef IDC_TmcSoundEffects_Sound209
#undef IDC_TmcSoundEffects_Sound210
#undef IDC_TmcSoundEffects_Sound211
#undef IDC_TmcSoundEffects_Sound212
#undef IDC_TmcSoundEffects_Sound213
#undef IDC_TmcSoundEffects_Sound214
#undef IDC_TmcSoundEffects_Sound215
#undef IDC_TmcSoundEffects_Sound216
#undef IDC_TmcSoundEffects_Sound217
#undef IDC_TmcSoundEffects_Sound218
#undef IDC_TmcSoundEffects_Sound219
#undef IDC_TmcSoundEffects_Sound220
#undef IDC_TmcSoundEffects_Sound221
#undef IDC_TmcSoundEffects_Sound222
#undef IDC_TmcSoundEffects_Sound223
#undef IDC_TmcSoundEffects_Sound224
#undef IDC_TmcSoundEffects_Sound225
#undef IDC_TmcSoundEffects_Sound226
#undef IDC_TmcSoundEffects_Sound227
#undef IDC_TmcSoundEffects_Sound228
#undef IDC_TmcSoundEffects_Sound229
#undef IDC_TmcSoundEffects_Sound230
#undef IDC_TmcSoundEffects_Sound231
#undef IDC_TmcSoundEffects_Sound232
#undef IDC_TmcSoundEffects_Sound233
#undef IDC_TmcSoundEffects_Sound234
#undef IDC_TmcSoundEffects_Sound235
#undef IDC_TmcSoundEffects_Sound236
#undef IDC_TmcSoundEffects_Sound237
#undef IDC_TmcSoundEffects_Sound238
#undef IDC_TmcSoundEffects_Sound239
#undef IDC_TmcSoundEffects_Sound240
#undef IDC_TmcSoundEffects_Sound97
#undef IDC_TmcSoundEffects_Sound98
#undef IDC_TmcSoundEffects_Sound99
#undef IDD_ABOUTBOX
#undef IDD_TMC_CALCTIME
#undef IDD_TMC_CONFIG_OUTFFORMAT
#undef IDD_TMC_RTH_TOPBLOCK
#undef IDD_TMC_SOUND_MELODY1
#undef IDD_TMC_STATISTICS
#undef IDOK
#undef ID_TMCBLOCKBUTTONNEXT2
#include "ui_IDD_ABOUTBOX.h"
#include "ui_IDD_TMC_CALCTIME.h"
#include "ui_IDD_TMC_STATISTICS.h"
#include "ui_IDD_TMC_RTH_TOPBLOCK.h"
#include "ui_IDD_TMC_CONFIG_OUTFFORMAT.h"
#include "ui_IDD_TMC_SOUND_MELODY1.h"

namespace {

/// @name Функции разметки диалогов
/// Строят виджеты окна по `.ui`: `IDD_ABOUTBOX`, `IDD_TMC_CALCTIME`, `IDD_TMC_STATISTICS`,
/// `IDD_TMC_RTH_TOPBLOCK`, `IDD_TMC_CONFIG_OUTFFORMAT`, `IDD_TMC_SOUND_MELODY1`.
/// Параметр `qDialog` — пустой `QDialog`, создаваемый `CDialog::DoModal()` слоя совместимости.
///@{
/// Строит виджеты окна по его `.ui`.
/// @param[in,out] qDialog пустой `QDialog`, созданный `CDialog::DoModal()` слоя совместимости.
void tmcSetup_IDD_ABOUTBOX(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_ABOUTBOX ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

void tmcSetup_IDD_TMC_CALCTIME(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_TMC_CALCTIME ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

void tmcSetup_IDD_TMC_STATISTICS(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_TMC_STATISTICS ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

void tmcSetup_IDD_TMC_RTH_TOPBLOCK(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_TMC_RTH_TOPBLOCK ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

void tmcSetup_IDD_TMC_CONFIG_OUTFFORMAT(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_TMC_CONFIG_OUTFFORMAT ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

void tmcSetup_IDD_TMC_SOUND_MELODY1(void* qDialog)
{
    // Ui_-объект хранит только указатели: после setupUi() виджеты
    // принадлежат диалогу, и держать его дальше не нужно.
    Ui_IDD_TMC_SOUND_MELODY1 ui;
    ui.setupUi(static_cast<QDialog*>(qDialog));
}

///@}
// Соответствие «номер -> символьное имя» для всех элементов из resource.h.
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
    { 2, "IDCANCEL" },
    { 4, "ID_TMCBLOCKBUTTONNEXT2" },
    { 5, "IDC_TMCSTATISTICSVIEWBLOCKLIST2" },
    { 6, "IDC_TMCSTATISTICSPREVSTEP" },
    { 7, "IDC_TMCSTATISTICSNEXTSTEP" },
    { 100, "IDD_ABOUTBOX" },
    { 3000, "IDC_TMC_DOCLINK" },
    { 101, "ID_MY_STATUS_BAR" },
    { 102, "IDD_TMC_RTH_TOPBLOCK" },
    { 103, "IDD_TMC_CONFIG_OUTFFORMAT" },
    { 104, "IDD_TMC_SOUND_MELODY1" },
    { 128, "IDR_MAINFRAME" },
    { 129, "IDR_PLANRTTYPE" },
    { 130, "IDR_MAINFRAME1" },
    { 1000, "IDC_TMCSTATISTICSERRORMESSAGE" },
    { 1001, "IDC_TMCSTATISTICSNSTEP" },
    { 1002, "IDC_TMCSTATISTICST0" },
    { 1003, "IDC_TMCSTATISTICSLONGUNIT" },
    { 1004, "IDC_TMCSTATISTICSFREQUNIT" },
    { 1005, "IDC_TMCSTATISTICSTIMEUNIT" },
    { 1006, "IDC_TMCSTATISTICSFREQ" },
    { 1007, "IDC_TMCSTATISTICSDELTA" },
    { 1008, "IDC_TMCSTATISTICSTMIN" },
    { 1009, "IDC_TMCSTATISTICSTMAX" },
    { 1010, "IDC_TMCSTATISTICSXMIN" },
    { 1011, "IDC_TMCSTATISTICSXMAX" },
    { 1012, "IDC_TMCSTATISTICSYMIN" },
    { 1013, "IDC_TMCSTATISTICSYMAX" },
    { 1014, "IDC_TMCSTATISTICSANGLEUNIT" },
    { 1015, "IDC_TMCSTATISTICSWAVELENGHT" },
    { 1016, "IDC_TMCSTATISTICSNBLOCK" },
    { 1017, "IDC_TMCSTATISTICSFILEOUT" },
    { 1018, "IDC_TMCSTATISTICSFIELDOUT" },
    { 1019, "IDC_TMCSTATISTICSTOPOLOGYOUT" },
    { 1020, "IDC_TMCSTATISTICSWAVELENGHTDELT" },
    { 1021, "IDC_TMCSTATISTICST2" },
    { 1022, "IDC_TMCBLOCKDX7" },
    { 1023, "IDC_TMCBLOCKDX8" },
    { 1024, "IDC_TMCBLOCKDX9" },
    { 1025, "IDC_TMCBLOCKDX10" },
    { 1026, "IDC_TMCBLOCKXMINBTEXT" },
    { 1027, "IDC_TMCBLOCKDY2" },
    { 1028, "IDC_TMCBLOCKDY3" },
    { 1029, "IDC_TMCBLOCKDY4" },
    { 1030, "IDC_TMCBLOCKDY5" },
    { 1031, "IDC_TMCBLOCKDY6" },
    { 170, "IDD_TMC_CALCTIME" },
    { 1900, "IDC_TMCSTATISTICSCALCTIME" },
    { 1901, "IDC_TMCCALCTIMETEXT" },
    { 1902, "IDC_TMCCALCTIMEEXPORT" },
    { 1903, "IDC_TMCCALCTIMEREFRESH" },
    { 1032, "IDC_TMCBLOCKDY7" },
    { 1033, "IDC_TMCBLOCKDY8" },
    { 1034, "IDC_TMCBLOCKDY9" },
    { 1035, "IDC_TMCBLOCKDY10" },
    { 1036, "IDC_TMCBLOCKNPOINTXY" },
    { 1037, "IDC_TMCBLOCKNTYPE" },
    { 1038, "IDC_TMCBLOCKXMAXBTEXT" },
    { 1039, "IDC_TMCBLOCKYMINBTEXT" },
    { 1040, "IDC_TMCBLOCKYMAXBTEXT" },
    { 1041, "IDC_TMCBLOCKMEMORY1" },
    { 1121, "IDC_TmcSoundEffects_Sound97" },
    { 1122, "IDC_TmcSoundEffects_Sound98" },
    { 1123, "IDC_TmcSoundEffects_Sound99" },
    { 1124, "IDC_TmcSoundEffects_Sound100" },
    { 1125, "IDC_TmcSoundEffects_Sound101" },
    { 1126, "IDC_TmcSoundEffects_Sound102" },
    { 1127, "IDC_TmcSoundEffects_Sound103" },
    { 1128, "IDC_TmcSoundEffects_Sound104" },
    { 1129, "IDC_TmcSoundEffects_Sound105" },
    { 1130, "IDC_TmcSoundEffects_Sound106" },
    { 1131, "IDC_TmcSoundEffects_Sound107" },
    { 1132, "IDC_TmcSoundEffects_Sound108" },
    { 1133, "IDC_TmcSoundEffects_Sound109" },
    { 1134, "IDC_TmcSoundEffects_Sound110" },
    { 1135, "IDC_TmcSoundEffects_Sound111" },
    { 1136, "IDC_TmcSoundEffects_Sound112" },
    { 1137, "IDC_TmcSoundEffects_Sound113" },
    { 1138, "IDC_TmcSoundEffects_Sound114" },
    { 1139, "IDC_TmcSoundEffects_Sound115" },
    { 1140, "IDC_TmcSoundEffects_Sound116" },
    { 1141, "IDC_TmcSoundEffects_Sound117" },
    { 1142, "IDC_TmcSoundEffects_Sound118" },
    { 1143, "IDC_TmcSoundEffects_Sound119" },
    { 1144, "IDC_TmcSoundEffects_Sound120" },
    { 1145, "IDC_TmcSoundEffects_Sound121" },
    { 1146, "IDC_TmcSoundEffects_Sound122" },
    { 1147, "IDC_TmcSoundEffects_Sound123" },
    { 1148, "IDC_TmcSoundEffects_Sound124" },
    { 1149, "IDC_TmcSoundEffects_Sound125" },
    { 1150, "IDC_TmcSoundEffects_Sound126" },
    { 1151, "IDC_TmcSoundEffects_Sound127" },
    { 1152, "IDC_TmcSoundEffects_Sound128" },
    { 1153, "IDC_TmcSoundEffects_Sound129" },
    { 1154, "IDC_TmcSoundEffects_Sound130" },
    { 1155, "IDC_TmcSoundEffects_Sound131" },
    { 1156, "IDC_TmcSoundEffects_Sound132" },
    { 1157, "IDC_TmcSoundEffects_Sound133" },
    { 1158, "IDC_TmcSoundEffects_Sound134" },
    { 1159, "IDC_TmcSoundEffects_Sound135" },
    { 1160, "IDC_TmcSoundEffects_Sound136" },
    { 1161, "IDC_TmcSoundEffects_Sound137" },
    { 1162, "IDC_TmcSoundEffects_Sound138" },
    { 1163, "IDC_TmcSoundEffects_Sound139" },
    { 1164, "IDC_TmcSoundEffects_Sound140" },
    { 1165, "IDC_TmcSoundEffects_Sound141" },
    { 1166, "IDC_TmcSoundEffects_Sound142" },
    { 1167, "IDC_TmcSoundEffects_Sound143" },
    { 1168, "IDC_TmcSoundEffects_Sound144" },
    { 1169, "IDC_TmcSoundEffects_Sound145" },
    { 1170, "IDC_TmcSoundEffects_Sound146" },
    { 1171, "IDC_TmcSoundEffects_Sound147" },
    { 1172, "IDC_TmcSoundEffects_Sound148" },
    { 1173, "IDC_TmcSoundEffects_Sound149" },
    { 1174, "IDC_TmcSoundEffects_Sound150" },
    { 1175, "IDC_TmcSoundEffects_Sound151" },
    { 1176, "IDC_TmcSoundEffects_Sound152" },
    { 1177, "IDC_TmcSoundEffects_Sound153" },
    { 1178, "IDC_TmcSoundEffects_Sound154" },
    { 1179, "IDC_TmcSoundEffects_Sound155" },
    { 1180, "IDC_TmcSoundEffects_Sound156" },
    { 1181, "IDC_TmcSoundEffects_Sound157" },
    { 1182, "IDC_TmcSoundEffects_Sound158" },
    { 1183, "IDC_TmcSoundEffects_Sound159" },
    { 1184, "IDC_TmcSoundEffects_Sound160" },
    { 1185, "IDC_TmcSoundEffects_Sound161" },
    { 1186, "IDC_TmcSoundEffects_Sound162" },
    { 1187, "IDC_TmcSoundEffects_Sound163" },
    { 1188, "IDC_TmcSoundEffects_Sound164" },
    { 1189, "IDC_TmcSoundEffects_Sound165" },
    { 1190, "IDC_TmcSoundEffects_Sound166" },
    { 1191, "IDC_TmcSoundEffects_Sound167" },
    { 1192, "IDC_TmcSoundEffects_Sound168" },
    { 1193, "IDC_TmcSoundEffects_Sound169" },
    { 1194, "IDC_TmcSoundEffects_Sound170" },
    { 1195, "IDC_TmcSoundEffects_Sound171" },
    { 1196, "IDC_TmcSoundEffects_Sound172" },
    { 1197, "IDC_TmcSoundEffects_Sound173" },
    { 1198, "IDC_TmcSoundEffects_Sound174" },
    { 1199, "IDC_TmcSoundEffects_Sound175" },
    { 1200, "IDC_TmcSoundEffects_Sound176" },
    { 1201, "IDC_TmcSoundEffects_Sound177" },
    { 1202, "IDC_TmcSoundEffects_Sound178" },
    { 1203, "IDC_TmcSoundEffects_Sound179" },
    { 1204, "IDC_TmcSoundEffects_Sound180" },
    { 1205, "IDC_TmcSoundEffects_Sound181" },
    { 1206, "IDC_TmcSoundEffects_Sound182" },
    { 1207, "IDC_TmcSoundEffects_Sound183" },
    { 1208, "IDC_TmcSoundEffects_Sound184" },
    { 1209, "IDC_TmcSoundEffects_Sound185" },
    { 1210, "IDC_TmcSoundEffects_Sound186" },
    { 1211, "IDC_TmcSoundEffects_Sound187" },
    { 1212, "IDC_TmcSoundEffects_Sound188" },
    { 1213, "IDC_TmcSoundEffects_Sound189" },
    { 1214, "IDC_TmcSoundEffects_Sound190" },
    { 1215, "IDC_TmcSoundEffects_Sound191" },
    { 1216, "IDC_TmcSoundEffects_Sound192" },
    { 1217, "IDC_TmcSoundEffects_Sound193" },
    { 1218, "IDC_TmcSoundEffects_Sound194" },
    { 1219, "IDC_TmcSoundEffects_Sound195" },
    { 1220, "IDC_TmcSoundEffects_Sound196" },
    { 1221, "IDC_TmcSoundEffects_Sound197" },
    { 1222, "IDC_TmcSoundEffects_Sound198" },
    { 1223, "IDC_TmcSoundEffects_Sound199" },
    { 1224, "IDC_TmcSoundEffects_Sound200" },
    { 1225, "IDC_TmcSoundEffects_Sound201" },
    { 1226, "IDC_TmcSoundEffects_Sound202" },
    { 1227, "IDC_TmcSoundEffects_Sound203" },
    { 1228, "IDC_TmcSoundEffects_Sound204" },
    { 1229, "IDC_TmcSoundEffects_Sound205" },
    { 1230, "IDC_TmcSoundEffects_Sound206" },
    { 1231, "IDC_TmcSoundEffects_Sound207" },
    { 1232, "IDC_TmcSoundEffects_Sound208" },
    { 1233, "IDC_TmcSoundEffects_Sound209" },
    { 1234, "IDC_TmcSoundEffects_Sound210" },
    { 1235, "IDC_TmcSoundEffects_Sound211" },
    { 1236, "IDC_TmcSoundEffects_Sound212" },
    { 1237, "IDC_TmcSoundEffects_Sound213" },
    { 1238, "IDC_TmcSoundEffects_Sound214" },
    { 1239, "IDC_TmcSoundEffects_Sound215" },
    { 1240, "IDC_TmcSoundEffects_Sound216" },
    { 1241, "IDC_TmcSoundEffects_Sound217" },
    { 1242, "IDC_TmcSoundEffects_Sound218" },
    { 1243, "IDC_TmcSoundEffects_Sound219" },
    { 1244, "IDC_TmcSoundEffects_Sound220" },
    { 1245, "IDC_TmcSoundEffects_Sound221" },
    { 1246, "IDC_TmcSoundEffects_Sound222" },
    { 1247, "IDC_TmcSoundEffects_Sound223" },
    { 1248, "IDC_TmcSoundEffects_Sound224" },
    { 1249, "IDC_TmcSoundEffects_Sound225" },
    { 1250, "IDC_TmcSoundEffects_Sound226" },
    { 1251, "IDC_TmcSoundEffects_Sound227" },
    { 1252, "IDC_TmcSoundEffects_Sound228" },
    { 1253, "IDC_TmcSoundEffects_Sound229" },
    { 1254, "IDC_TmcSoundEffects_Sound230" },
    { 1255, "IDC_TmcSoundEffects_Sound231" },
    { 1256, "IDC_TmcSoundEffects_Sound232" },
    { 1257, "IDC_TmcSoundEffects_Sound233" },
    { 1258, "IDC_TmcSoundEffects_Sound234" },
    { 1259, "IDC_TmcSoundEffects_Sound235" },
    { 1260, "IDC_TmcSoundEffects_Sound236" },
    { 1261, "IDC_TmcSoundEffects_Sound237" },
    { 1262, "IDC_TmcSoundEffects_Sound238" },
    { 1263, "IDC_TmcSoundEffects_Sound239" },
    { 1264, "IDC_TmcSoundEffects_Sound240" },
    { 32771, "ID_EDIT_EDIT" },
    { 32773, "ID_RUN_RUN" },
    { 32774, "ID_RUN_STOP" },
    { 32775, "ID_CONFIG_EDITOR" },
    { 32776, "ID_RUN_SKIPSTEP" },
    { 32777, "ID_RUN_BACKSTEP" },
    { 32778, "ID_RUN_RESTARTALL" },
    { 32779, "ID_RUN_STARTSTEP" },
    { 32780, "ID_RUN_RESTARTSTEP" },
    { 32781, "ID_VIEW_STATISTICS" },
    { 32783, "ID_VIEW_TOPOLOGY" },
    { 32784, "ID_VIEW_FIELD" },
    { 32785, "ID_VIEW_OUTPUT" },
    { 32793, "ID_CONFIG_COLOR_BACKGROUND" },
    { 32794, "ID_CONFIG_FORMAT_OUTPUTDATAFILE" },
    { 32795, "ID_CONFIG_VIEWER" },
    { 32796, "ID_CONFIG_VIEWER_OUTPUTSIGNAL" },
    { 32802, "ID_VIEW_FIELD_1" },
    { 32803, "ID_CONFIG_VIEWER_FIELD" },
    { 32804, "ID_CONFIG_SINCHRONIZATION" },
    { 32808, "ID_CONFIG_SOUND" },
    { 32810, "ID_CONFIG_SOUND_MELODY" },
    { 32813, "ID_CONFIG_SMATRIX" },
    { 32814, "ID_CONFIG_VIEWER_SMATRIX" },
    { 32815, "ID_CONFIG_AUTORUN" },
    { 32817, "ID_CONFIG_SETUP" },
    { 32818, "ID_VIEW_DIRECTIONALPATTERN" },
    { 32820, "ID_CONFIG_VIEWER_DIRECTIONALPATTERN" },
    { 32821, "ID_CONFIG_DIRECTIONALPATTERN" },
    { 1, "IDOK" },
    { 3, "IDABORT" },
    { 9, "IDHELP" },
};

/// @brief Регистрирует функции разметки и имена элементов в `MfcDialogRegistry`.
/// @return true (значение нужно для однократного вызова через статическую переменную).
bool tmcInstall()
{
    MfcDialogRegistry::registerDialog(100 /* IDD_ABOUTBOX */, &tmcSetup_IDD_ABOUTBOX);
    MfcDialogRegistry::registerDialog(170 /* IDD_TMC_CALCTIME */, &tmcSetup_IDD_TMC_CALCTIME);
    MfcDialogRegistry::registerDialog(101 /* IDD_TMC_STATISTICS */, &tmcSetup_IDD_TMC_STATISTICS);
    MfcDialogRegistry::registerDialog(102 /* IDD_TMC_RTH_TOPBLOCK */, &tmcSetup_IDD_TMC_RTH_TOPBLOCK);
    MfcDialogRegistry::registerDialog(103 /* IDD_TMC_CONFIG_OUTFFORMAT */, &tmcSetup_IDD_TMC_CONFIG_OUTFFORMAT);
    MfcDialogRegistry::registerDialog(104 /* IDD_TMC_SOUND_MELODY1 */, &tmcSetup_IDD_TMC_SOUND_MELODY1);

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
void TmcInstallDialogs_planarrt_x()
{
    static const bool done = tmcInstall();
    (void)done;
}
