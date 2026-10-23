// Точка входа TMCGROUT — вьювера выходных характеристик S-матриц пакета
// TMC Suite (графики модуля, фазы и т. п. по частоте).
//
// Программа собрана из двух слоёв: расчёт и работа с файлами взяты из
// Windows-версии без изменений, интерфейс — Qt 6.

/// @file main.cpp
/// @brief Точка входа TMCGROUT: приложение Qt, имя настроек, оформление, главное окно, файл из командной строки.
///
/// Часть модуля «Вьювер TMCGROUT».
/// @see https://www.tamic.ru/docs/code/tmcgrout.html#qt-shell
#include <QApplication>

#include "mainwindow.h"
#include "theme.h"
#include "tmc_mfc_doc.h"

#include "dllsafety.h"

/// @brief Запускает программу TMCGROUT.
/// @param[in] argc число аргументов
/// @param[in] argv аргументы; первый (необязательный) — файл документа `.soc` или файл `.s`
/// @return Код завершения цикла событий Qt (0 — нормальное завершение).
int main(int argc, char *argv[])
{
    tmcui::secureDllSearch();   // DLL не из папки задания (Т-33)
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("TMCGROUT"));
    QApplication::setOrganizationName(QStringLiteral("TMC Suite"));

    // Настройки программы (цвета графиков, формат осей) на Windows лежали в
    // реестре; здесь — в ~/.config/TMC_Suite/TMCGROUT.conf.
    CWinApp::SetAppName("TMCGROUT");

    theme::setProgram(QStringLiteral("tmcgrout"));   // пиктограммы нового набора
    theme::apply();

    MainWindow window;
    window.show();

    // Файл задания можно передать в командной строке — как в Windows-версии.
    const QStringList args = QApplication::arguments();
    if (args.size() > 1)
        window.openDocument(args.at(1));

    return app.exec();
}
