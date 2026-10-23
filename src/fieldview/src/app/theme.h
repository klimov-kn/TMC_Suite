// Оформление FieldView: палитра, шрифты и таблица стилей виджетов.
//
// Значения — из единого оформления пакета (палитра сайта проекта и TPL Designer,
// описание — TMC_Suite_Mobile/docs/style-guide.md, токены Theme.qml). Здесь они
// записаны константами: каталог программы собирается сам по себе.
/// @file theme.h
/// @brief Оформление FieldView: цвета, шрифты, таблица стилей виджетов.
///
/// Часть модуля FieldView (слой `app`). Пространство имён `theme` — своё у FieldView
/// (не `gui_common/theme.h`).
/// @see https://www.tamic.ru/docs/code/fieldview.html#theme
#pragma once

#include <QColor>
#include <QFont>
#include <QString>

class QApplication;

/// @brief Цвета, шрифты и таблица стилей оформления FieldView.
namespace theme {

/// @name Цвета оформления
/// Фон, поверхности, линии, текст, акценты, предупреждение, опасность, «включено».
///@{
/// @brief Возвращает цвет оформления, названный именем функции.
/// @return Цвет sRGB (значение — в комментарии справа).
inline QColor bg()        { return QColor(0x0a, 0x0e, 0x16); }   // #0a0e16
inline QColor bg2()       { return QColor(0x0e, 0x13, 0x20); }   // #0e1320
inline QColor surface()   { return QColor(0x13, 0x1a, 0x28); }   // #131a28
inline QColor surface2()  { return QColor(0x1a, 0x23, 0x35); }   // #1a2335
inline QColor line()      { return QColor(0x24, 0x30, 0x49); }   // #243049
inline QColor text()      { return QColor(0xe7, 0xee, 0xf7); }   // #e7eef7
inline QColor textDim()   { return QColor(0x9f, 0xb0, 0xc8); }   // #9fb0c8
inline QColor textMute()  { return QColor(0x6c, 0x7d, 0x97); }   // #6c7d97
inline QColor accent()    { return QColor(0x34, 0xe3, 0xc4); }   // #34e3c4
inline QColor accentPressed() { return QColor(0x5c, 0xeb, 0xd1); }
inline QColor textOnAccent()  { return QColor(0x06, 0x23, 0x1e); }
inline QColor accent2()   { return QColor(0xb0, 0x7c, 0xff); }   // #b07cff
inline QColor accent3()   { return QColor(0xff, 0xcf, 0x5c); }   // #ffcf5c
inline QColor warn()      { return QColor(0xe3, 0xb3, 0x41); }   // #e3b341
inline QColor danger()    { return QColor(0xff, 0x6b, 0x6b); }   // #ff6b6b
inline QColor ok()        { return QColor(0x5c, 0xe0, 0x8a); }   // #5ce08a
///@}

/// @brief Возвращает имя семейства шрифта интерфейса.
/// @return «Inter».
inline QString fontUi()   { return QStringLiteral("Inter"); }
/// @brief Возвращает имя моноширинного семейства шрифта.
/// @return «JetBrains Mono».
inline QString fontMono() { return QStringLiteral("JetBrains Mono"); }

/// @brief Возвращает шрифт интерфейса заданного размера.
/// @param[in] pixelSize  размер, пиксели
/// @param[in] weight     насыщенность `QFont::Weight`
/// @return Шрифт Inter.
QFont uiFont(int pixelSize, int weight = QFont::Normal);
/// @brief Возвращает моноширинный шрифт заданного размера.
/// @param[in] pixelSize  размер, пиксели
/// @param[in] weight     насыщенность `QFont::Weight`
/// @return Шрифт JetBrains Mono.
QFont monoFont(int pixelSize, int weight = QFont::Normal);

/// @brief Применяет оформление к приложению.
///
/// Регистрирует встроенные шрифты и ставит таблицу стилей приложения.
/// @param[in,out] app  приложение
void apply(QApplication &app);

} // namespace theme
