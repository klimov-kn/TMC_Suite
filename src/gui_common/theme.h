// theme.h — оформление программ TMC Suite под Linux.
//
// Палитра и приёмы те же, что в TPL Designer: тёмная подложка, приглушённые
// рамки, бирюзовый акцент. Компоновка окон при этом остаётся такой же, как в
// Windows-версии — меняется только оформление, не расположение элементов.

/// @file theme.h
/// @brief Оформление программ пакета: палитра, таблица стилей, шрифты, пиктограммы программ и команд.
///
/// Часть модуля «Общий слой интерфейса». Входит в библиотеку `tmc_ui_base`
/// (вместе с `aboutbox.cpp`), которая не зависит от слоя MFC и используется
/// также TPL Designer. Только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/gui-common.html#theme
#pragma once

#include <QColor>
#include <QString>

QT_BEGIN_NAMESPACE
class QWidget;
class QIcon;
QT_END_NAMESPACE

/// @brief Палитра и функции оформления программ пакета.
namespace theme {

/// @brief Цвет фона окон (`#0a0e16`).
/// @return Цвет `#0a0e16`.
inline QColor bg()       { return QColor(0x0a, 0x0e, 0x16); }
/// @brief Цвет панелей, меню, строки состояния (`#131a28`).
/// @return Цвет `#131a28`.
inline QColor surface()  { return QColor(0x13, 0x1a, 0x28); }
/// @brief Цвет рамок и разделителей (`#243049`).
/// @return Цвет `#243049`.
inline QColor line()     { return QColor(0x24, 0x30, 0x49); }
/// @brief Основной цвет текста (`#e7eef7`).
/// @return Цвет `#e7eef7`.
inline QColor text()     { return QColor(0xe7, 0xee, 0xf7); }
/// @brief Приглушённый цвет текста (`#9fb0c8`).
/// @return Цвет `#9fb0c8`.
inline QColor textDim()  { return QColor(0x9f, 0xb0, 0xc8); }
/// @brief Цвет недоступного текста (`#6c7d97`).
/// @return Цвет `#6c7d97`.
inline QColor textMute() { return QColor(0x6c, 0x7d, 0x97); }
/// @brief Акцентный (бирюзовый) цвет (`#34e3c4`).
/// @return Цвет `#34e3c4`.
inline QColor accent()   { return QColor(0x34, 0xe3, 0xc4); }
/// @brief Цвет ошибки (`#ff6b6b`).
/// @return Цвет `#ff6b6b`.
inline QColor danger()   { return QColor(0xff, 0x6b, 0x6b); }
/// @brief Цвет успеха (`#5ce08a`).
/// @return Цвет `#5ce08a`.
inline QColor ok()       { return QColor(0x5c, 0xe0, 0x8a); }
/// @brief Цвет предупреждения (`#e3b341`).
/// @return Цвет `#e3b341`.
inline QColor warn()     { return QColor(0xe3, 0xb3, 0x41); }

/// Таблица стилей приложения.
/// @return Текст таблицы стилей Qt (QSS) для всего приложения.
QString styleSheet();

/// Применить оформление ко всему приложению (палитра + таблица стилей).
///
/// Также: устанавливает числовую локаль C (`LC_NUMERIC`) — разделитель
/// дробной части точка; загружает шрифты; задаёт значок приложения;
/// подключает оформитель окна «О программе» (`tmcabout::install`) и перевод
/// интерфейса (`tmci18n::install`); ставит фильтр однострочных подсказок.
/// @pre Объект `QApplication` создан; `setProgram` вызван (для значка).
void apply();

/// Имя программы для поиска пиктограмм нового набора (planarrt_h, planarrt_x,
/// tmcgrout, tmcros, tmc_dn, fieldview). Задаётся в main до создания окна.
/// @param[in] program  имя программы в нижнем регистре
void setProgram(const QString &program);
/// @brief Возвращает имя программы, заданное `setProgram` (пустое — не задано).
/// @return Имя программы в нижнем регистре; пустая строка — `setProgram` не вызывался.
QString program();

/// Пиктограмма команды. Сначала — новый набор (вариант Б «плитки»,
/// `:/tmcicons/toolbar/<программа>/<ID>.svg`); у переключателей там два файла
/// `<ID>_off.svg` (красный кружок) и `<ID>_on.svg` (зелёный) — они кладутся в
/// состояния Off/On, и отмеченное действие само показывает зелёный. Если нового
/// рисунка нет — прежний из Windows-ресурсов.
/// @param[in] commandId  имя команды (`ID_FILE_OPEN` …)
/// @return Значок; пустой, если рисунка нет ни в одном наборе.
QIcon commandIcon(const QString &commandId);

/// Пиктограмма команды в заданном состоянии (кнопки запуска вьюверов: зелёный —
/// вьювер запущен, красный — нет). Пустая, если у команды нет пары _on/_off.
/// @param[in] commandId  имя команды
/// @param[in] on         true — состояние «запущен» (`_on.svg`), false — `_off.svg`
/// @return Значок из файла `_on.svg` или `_off.svg` команды; пустой, если такого файла нет.
QIcon commandIconState(const QString &commandId, bool on);

/// Пиктограмма программы (вариант A) для заголовка окна и панели задач.
/// @return Значок из `:/tmcicons/app/<программа>[_sm].svg`; пустой, если программа не задана или рисунка нет.
QIcon appIcon();

/// Запущена ли в системе программа с таким именем файла (например "tmcros.exe").
/// Опрос списка процессов не чаще раза в секунду.
/// @param[in] exeName  имя исполняемого файла без пути; регистр не важен
/// @return true — процесс найден (только Windows; в других системах всегда false).
bool programRunning(const QString &exeName);

/// Шрифты проекта: интерфейс — Inter, числа и тексты заданий — JetBrains Mono.
/// @brief Возвращает имя гарнитуры интерфейса (Inter из ресурсов; загружает шрифты при первом вызове).
/// @return Имя семейства из загруженного шрифта; если загрузить не удалось — `Inter`.
QString uiFontFamily();
/// @brief Возвращает имя моноширинной гарнитуры (JetBrains Mono из ресурсов).
/// @return Имя семейства из загруженного шрифта; если загрузить не удалось — `JetBrains Mono`.
QString monoFontFamily();

/// Ужать окно до экрана: диалог, у которого не видно кнопок, — неработающий диалог.
/// @brief Подгоняет размер окна под экран (не более 95 % ширины и 88 % высоты) и ставит его по центру.
/// @param[in,out] w  окно; NULL — ничего
void fitToScreen(QWidget *w);

} // namespace theme
