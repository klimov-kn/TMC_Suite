/// @file printsupport.cpp
/// @brief Реализация печати видов: `CDC` подключается к `QPrinter`, вид OpenGL печатается снимком кадра.
///
/// Часть модуля «Общий слой интерфейса», библиотека `tmc_gui_common`.
/// @see https://www.tamic.ru/docs/code/gui-common.html#print
#include "printsupport.h"

#include <QPageSetupDialog>
#include <QPainter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>

#include "mfc_view.h"

namespace {

// Настройки принтера живут между вызовами, как в Windows-версии, где параметры
// страницы запоминались на время работы программы.
/// @brief Возвращает общий объект принтера (высокое разрешение), живущий до конца программы.
/// @return Ссылка на статический `QPrinter`; настройки страницы сохраняются между вызовами.
QPrinter &sharedPrinter()
{
    static QPrinter printer(QPrinter::HighResolution);
    return printer;
}

// Рисование одной страницы: контекст подключается к принтеру, и вид рисует
// себя тем же кодом, что и на экране. Масштаб получается сам: режим отображения
// MM_HIMETRIC считает пиксели из разрешения устройства, а у принтера оно своё.
/// @brief Рисует одну страницу вида на принтере.
///
/// Вид рисует себя через `CDC` в режиме `MM_HIMETRIC` с началом в левом верхнем углу листа.
/// @param[in] printer  принтер
/// @param[in] view     вид; NULL — ничего
void drawPage(QPrinter *printer, CScrollView *view)
{
    if (!view)
        return;
    QPainter painter(printer);

    CDC dc;
    dc.Attach(&painter, printer);
    dc.SetPrinting(TRUE);

    // На бумаге прокрутки нет: начало координат — левый верхний угол листа.
    dc.SetMapMode(MM_HIMETRIC);
    dc.SetViewportOrg(0, 0);
    view->OnDraw(&dc);
}

} // namespace

namespace tmcprint {

bool printView(QWidget *parent, CScrollView *view, const QString &documentName)
{
    if (!view)
        return false;

    QPrinter &printer = sharedPrinter();
    printer.setDocName(documentName);

    QPrintDialog dialog(&printer, parent);
    dialog.setWindowTitle(QStringLiteral("Print"));
    if (dialog.exec() != QDialog::Accepted)
        return false;

    drawPage(&printer, view);
    return true;
}

bool previewView(QWidget *parent, CScrollView *view, const QString &documentName)
{
    if (!view)
        return false;

    QPrinter &printer = sharedPrinter();
    printer.setDocName(documentName);

    QPrintPreviewDialog dialog(&printer, parent);
    dialog.setWindowTitle(QStringLiteral("Print Preview"));
    QObject::connect(&dialog, &QPrintPreviewDialog::paintRequested,
                     [view](QPrinter *p) { drawPage(p, view); });
    dialog.exec();
    return true;
}

bool printViewToPdf(CScrollView *view, const QString &path)
{
    if (!view || path.isEmpty())
        return false;

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    drawPage(&printer, view);
    return true;
}

bool pageSetup(QWidget *parent)
{
    QPageSetupDialog dialog(&sharedPrinter(), parent);
    dialog.setWindowTitle(QStringLiteral("Print Setup"));
    return dialog.exec() == QDialog::Accepted;
}

} // namespace tmcprint
