// printsupport.h — печать вида, перенесённого из Windows-версии.
//
// В MFC печать шла тем же кодом, что и рисование на экране: каркас давал виду
// контекст устройства принтера, и `OnDraw` рисовал в него. Здесь ровно то же:
// контекст рисования подключается к принтеру Qt, а вид рисует себя не зная,
// куда именно.
//
// Благодаря этому на бумаге получается та же картинка, что на экране, и
// отдельного «кода печати» не появляется.

/// @file printsupport.h
/// @brief Печать, предпросмотр, печать в PDF и настройка страницы для видов MFC.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`.
/// Параметры принтера общие на всё время работы программы. Только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/gui-common.html#print
#pragma once

#include <QString>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

/// Вид MFC (`compat/include/mfc_view.h`).
class CScrollView;

/// @brief Печать видов MFC средствами Qt.
namespace tmcprint {

/// Печать вида. Показывает обычный диалог печати; возвращает false, если
/// пользователь отказался или печать не удалась.
/// @param[in] parent        окно-родитель диалога
/// @param[in] view          вид; NULL — false
/// @param[in] documentName  имя задания печати
/// @return true — страница отправлена на печать; false — `view == NULL` или пользователь отказался в диалоге печати.
bool printView(QWidget *parent, CScrollView *view, const QString &documentName);

/// Предварительный просмотр печати того же вида.
/// @param[in] parent        окно-родитель
/// @param[in] view          вид; NULL — false
/// @param[in] documentName  имя задания печати
/// @return true — окно показано (результат печати из окна не возвращается); false — `view == NULL`.
bool previewView(QWidget *parent, CScrollView *view, const QString &documentName);

/// Печать вида в файл PDF — тем же кодом, что и на принтер. Нужна, чтобы
/// проверять печать без участия человека (в том числе печать окна FieldView,
/// которое рисуется через OpenGL).
/// @param[in] view  вид; NULL — false
/// @param[in] path  путь к файлу PDF; пустой — false
/// @return true — страница нарисована (ошибка записи файла не проверяется).
bool printViewToPdf(CScrollView *view, const QString &path);

/// Настройка страницы (в Windows-версии — пункт «Print Setup»).
/// @param[in] parent  окно-родитель
/// @return true — пользователь нажал OK.
bool pageSetup(QWidget *parent);

} // namespace tmcprint
