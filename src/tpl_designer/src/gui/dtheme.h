/// @file dtheme.h
/// @brief Оформление TPL Designer: цвета палитры, таблица стилей Qt и ужатие окна до экрана.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Палитра совпадает с
/// программами пакета (`gui_common/theme`); цвета нужны и таблице стилей, и холсту.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-theme
#pragma once

#include <QColor>
#include <QString>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

/// Оформление в стиле сайта проекта (website/assets/css/main.css).
/// Цвета держим здесь, чтобы окно и холст не разъезжались друг с другом.
namespace dtheme {

/// @name Цвета палитры (тёмная тема пакета)
/// Возвращают цвет для рисования на холсте и в собственных виджетах; те же значения
/// записаны в таблице стилей `styleSheet()`.
///@{
/// @brief Фон окна, `#0a0e16`.
/// @return Цвет палитры.
inline QColor bg()       { return QColor(0x0a, 0x0e, 0x16); }
/// @brief Фон поверхностей (панелей, меню, карточек), `#131a28`.
/// @return Цвет палитры.
inline QColor surface()  { return QColor(0x13, 0x1a, 0x28); }
/// @brief Цвет линий и рамок, `#243049`.
/// @return Цвет палитры.
inline QColor line()     { return QColor(0x24, 0x30, 0x49); }
/// @brief Основной цвет текста, `#e7eef7`.
/// @return Цвет палитры.
inline QColor text()     { return QColor(0xe7, 0xee, 0xf7); }
/// @brief Приглушённый текст (пояснения), `#9fb0c8`.
/// @return Цвет палитры.
inline QColor textDim()  { return QColor(0x9f, 0xb0, 0xc8); }
/// @brief Самый тусклый текст (подписи, неактивное), `#6c7d97`.
/// @return Цвет палитры.
inline QColor textMute() { return QColor(0x6c, 0x7d, 0x97); }
/// @brief Акцентный бирюзовый (выделение, активные элементы), `#34e3c4`.
/// @return Цвет палитры.
inline QColor accent()   { return QColor(0x34, 0xe3, 0xc4); }
/// @brief Красный для ошибок, `#ff6b6b`.
/// @return Цвет палитры.
inline QColor danger()   { return QColor(0xff, 0x6b, 0x6b); }
/// @brief Зелёный для успешного состояния, `#5ce08a`.
/// @return Цвет палитры.
inline QColor ok()       { return QColor(0x5c, 0xe0, 0x8a); }

/// Жёлтый для предупреждений: в палитре сайта его нет, берём между accent и danger.
/// @brief Жёлтый для предупреждений, `#e3b341`.
/// @return Цвет палитры.
inline QColor warn()     { return QColor(0xe3, 0xb3, 0x41); }
///@}

/// Таблица стилей приложения.
/// @brief Возвращает таблицу стилей Qt для всего приложения (ставится в `main.cpp`).
///
/// Общие элементы — как в программах пакета; свои для редактора — свойства виджетов
/// `accent` (акцентная кнопка), `tool` (кнопка инструмента холста), `card` (карточка),
/// `hline` (разделитель), `dim`/`mute`/`h1`/`h2` (виды подписей).
/// @return Текст таблицы стилей (CSS Qt).
QString styleSheet();

/// Ужать окно до экрана. Содержимое при этом уезжает в полосу прокрутки, а не
/// за нижний край: диалог, у которого не видно кнопок, — это неработающий диалог.
/// @brief Ужимает окно до 95 % ширины и 88 % высоты доступной области экрана и ставит его в центр.
/// @param[in,out] w  окно (диалог); `nullptr` — ничего не делает. Минимальная высота обнуляется,
///                   максимальная — высота экрана.
void fitToScreen(QWidget *w);

} // namespace dtheme
