// aboutbox.h — единое окно «О программе» для всех программ TMC Suite.
//
// Окно одинаковое в PlanarRT_H, PlanarRT_X, TMCGROUT, TMCROS, TMC_DN и
// FieldView: значок и название программы, версия пакета 5.1, год 2026, автор
// К. Н. Климов, значки-ссылки на GitHub, LinkedIn, ResearchGate и eLibrary,
// ссылки на сайт и на руководство пользователя (PDF рядом с программой, если
// его нет — руководство на сайте).
//
// Какая программа запущена, окно узнаёт из theme::program() (задаётся в main),
// поэтому вызывающему коду достаточно прежнего вызова tmcabout::show().
// Окно IDD_ABOUTBOX из ресурсов Windows, если его откроет перенесённый код,
// тоже подменяется этим содержимым (decorate(), ставится install()).

/// @file aboutbox.h
/// @brief Единое окно «О программе» для всех программ пакета и подмена окна `IDD_ABOUTBOX` из ресурсов.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_ui_base` (без слоя MFC).
/// Только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/gui-common.html#about
#pragma once

#include <QtGlobal>
#include <QString>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

/// @brief Окно «О программе».
namespace tmcabout {

/// @brief Путь к PDF руководства рядом с программой — на языке интерфейса, если есть
/// оба (`TMC_Suite_*_User_Manual.pdf` — английское, остальные `TMC_Suite_*.pdf` — русское).
/// @return Абсолютный путь или пустая строка (тогда открывать руководство на сайте).
QString manualPath();

/// Краткое содержимое окна «О программе» в виде HTML (название, описание,
/// версия, автор, ссылки) — для мест, где нужен текст, а не окно.
/// @param[in] title        название программы (если `theme::program()` пуст)
/// @param[in] description  описание от программы
/// @return HTML-текст.
QString aboutText(const QString &title, const QString &description);

/// Показать окно «О программе».
///   windowTitle — прежний заголовок («About TMCGROUT»); окно ставит свой
///                 «About `<программа>`», параметр нужен, если программа не задана;
///   title       — название программы (если theme::program() пуст);
///   description — описание от программы. Строки «Copyright …» отбрасываются,
///                 остальные (например «Model : H») показываются под описанием.
/// @param[in] parent       окно-родитель или NULL
/// @param[in] windowTitle  прежний заголовок окна
/// @param[in] title        название программы
/// @param[in] description  описание; строки разделяются `<br>` или переводом строки
void show(QWidget *parent, const QString &windowTitle, const QString &title,
          const QString &description);

/// Подменить содержимое окна IDD_ABOUTBOX (из ресурсов Windows) единым окном.
/// Для остальных окон ничего не делает.
/// @param[in,out] dialog  построенное окно; подменяется только при `objectName() == "IDD_ABOUTBOX"`, один раз
void decorate(QWidget *dialog);

/// Включить подмену для всех окон программы (зовёт theme::apply()).
/// @brief Регистрирует `decorate` как оформитель окон слоя MFC (`tmc_set_dialog_decorator`).
void install();

} // namespace tmcabout
