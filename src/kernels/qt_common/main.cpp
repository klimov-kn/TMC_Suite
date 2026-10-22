// Точка входа оболочек счётных ядер PlanarRT_H и PlanarRT_X.
//
// Программа собрана из двух слоёв: расчёт, работа с файлами задания и счёт в
// отдельном потоке взяты из Windows-версии без изменений, интерфейс — Qt 6.
//
// Файл общий для обеих программ: чем они отличаются, собрано в planrt_app.h.

/// @file main.cpp
/// @brief Точка входа PlanarRT_H и PlanarRT_X: настройки, ключи командной строки, главное окно, пакетный режим, код возврата.
///
/// Часть модуля «Оболочка счётного ядра». Заменяет `InitInstance`/`ExitInstance`
/// MFC-версии. Порядок: защита поиска DLL, `QApplication`, имя программы и файл
/// настроек, тема, программы просмотра по умолчанию, выбор CPU/GPU, разбор ключей
/// (`Set_CommandLine_Flags`), главное окно, проверка задания в пакетном режиме,
/// открытие задания, цикл событий, код возврата 0/1/2/3 (Windows).
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#startup
#include "tmccompute.h"
#include <QFileInfo>
#include <QApplication>

#include "mainwindow.h"
#include "planrt_app.h"

// Заголовки общего слоя интерфейса — при снятой подмене CScrollView
// (см. planrt_prelude.h); дальше подмена нужна снова, её ждут заголовки
// Windows-версии.
#undef CScrollView
#include "theme.h"
/// Подмена базового класса вида снова включена (см. `planrt_prelude.h`): её ждут заголовки ядра.
#define CScrollView TmcPlanRtScrollView

#include "tmc_mfc_doc.h"
#include "resource.h"
#include "Pl_iofor.h"

#include <QDir>
#include <QStandardPaths>

#include "pathcheck.h"

// Программы просмотра по умолчанию — программы пакета, как их прописывал
// установщик MFC-версии (installer/TMC_Suite.iss: ExternViewerName -> TMCROS,
// ExternFieldViewerName -> FieldView, ExternSmatrixViewerName -> TMCGROUT,
// ExternDirectPatViewerName -> TMC_DN). Без этих записей ядро берёт свои
// встроенные значения: «winword.exe» для сигнала и поля, а для S-матрицы и ДН
// склеивает папку задания с именем без разделителя («…\taskTMCGROUT.EXE»), и
// кнопки просмотра не открывают ничего (L-3, Т-18; так же сделано в Linux-версии).
//
// Запись не трогается, если в ней указана существующая программа (человек выбрал
// свою в Config -> Viewer). Заменяются пустые, «winword.exe»/«notepad.exe» и пути
// к несуществующим файлам. Имена ключей — из PlanRT_HDoc.h (раздел
// PLANRT_H_RAZDEL_INI общий для H и X).
/// @brief Записывает в настройки программы пакета вместо негодных значений просмотрщиков и редактора.
///
/// Для ключей ExternViewerName (TMCROS), ExternFieldViewerName (FieldView),
/// ExternSmatrixViewerName (TMCGROUT), ExternDirectPatViewerName (TMC_DN): значение
/// считается негодным, если оно пустое, «winword»/«notepad» или указывает на
/// несуществующий файл; тогда пишется путь к программе пакета из папки ядра (если
/// она там есть и путь переводится в кодовую страницу Windows без потерь).
/// В Windows то же для ExternEditorName: TPL Designer из папки ядра, иначе Блокнот.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#viewers
static void seedViewerDefaults()
{
#ifdef _WIN32
    static const char *const kExe = ".exe";
#else
    static const char *const kExe = "";
#endif
    static const struct { const char *key; const char *program; } kViewers[] = {
        { "ExternViewerName",          "tmcros"    },
        { "ExternFieldViewerName",     "fieldview" },
        { "ExternSmatrixViewerName",   "tmcgrout"  },
        { "ExternDirectPatViewerName", "tmc_dn"    },
    };
    const char *section = "PlanRT_H Config";
    const QString dir = QCoreApplication::applicationDirPath();
    for (const auto &v : kViewers) {
        const CString stored = AfxGetApp()->GetProfileString(section, v.key, "");
        const QString value = QString::fromLocal8Bit(static_cast<const char *>(stored));
        const QString base = QFileInfo(value).fileName().toLower();
        bool usable = !value.isEmpty() && base != QLatin1String("winword.exe")
                      && base != QLatin1String("notepad.exe")
                      && base != QLatin1String("winword") && base != QLatin1String("notepad");
        if (usable) {
            if (value.contains(QLatin1Char('/')) || value.contains(QLatin1Char('\\')))
                usable = QFileInfo(value).isFile();
            else
                usable = !QStandardPaths::findExecutable(value).isEmpty()
                         || QFileInfo(dir + QLatin1Char('/') + value).isFile();
        }
        const QString program = QDir::toNativeSeparators(
            dir + QLatin1Char('/') + QLatin1String(v.program) + QLatin1String(kExe));
        // Путь, который не переводится в кодовую страницу Windows без потерь,
        // не записываем: ядро открыло бы по нему другой файл (Т-20).
        const QByteArray local = program.toLocal8Bit();
        if (!usable && QFileInfo(program).isFile() && QString::fromLocal8Bit(local) == program)
            AfxGetApp()->WriteProfileString(section, v.key, local.constData());
    }

#ifdef _WIN32
    // Внешний редактор задания (Config -> Editor; запасной путь Alt+F7, если TPL Designer
    // не найден): по умолчанию в ядре — «winword.exe» (BUG-039). Ставим TPL Designer из
    // папки ядра, а если его нет — Блокнот из System32 (есть в любой Windows). Выбранный
    // человеком существующий редактор, указанный полным путём, не трогается.
    {
        const CString stored = AfxGetApp()->GetProfileString(section, "ExternEditorName", "");
        const QString value = QString::fromLocal8Bit(static_cast<const char *>(stored));
        const bool fullPath = value.contains(QLatin1Char('\\')) || value.contains(QLatin1Char('/'));
        if (!(fullPath && QFileInfo(value).isFile())) {
            QString editor = QDir::toNativeSeparators(dir + QStringLiteral("/TplDesigner.exe"));
            if (!QFileInfo(editor).isFile()) {
                wchar_t sys[MAX_PATH];
                const UINT n = ::GetSystemDirectoryW(sys, MAX_PATH);
                editor = QString::fromWCharArray(sys, int(n)) + QStringLiteral("\\notepad.exe");
            }
            const QByteArray local = editor.toLocal8Bit();
            if (QFileInfo(editor).isFile() && QString::fromLocal8Bit(local) == editor)
                AfxGetApp()->WriteProfileString(section, "ExternEditorName", local.constData());
        }
    }
#endif
}

#include "dllsafety.h"

/// @brief Запускает программу.
/// @param[in] argc число доводов командной строки.
/// @param[in] argv доводы: файл задания .tpl и ключи `/Ar`, `/Arb`, `/d`, `/Time`, `/GPU[:n]`, `/CPU`
///                 (в Windows — через `/` или `-`, вне Windows — только через `-`).
/// @return Windows: код расчёта 0 — всё посчитано, 1 — ошибка задания, 2 — ошибка
///         расчёта, 3 — остановлено; вне Windows — код цикла событий Qt.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#command-line
int main(int argc, char *argv[])
{
    tmcui::secureDllSearch();   // DLL не из папки задания (Т-33)
    QApplication app(argc, argv);
    QApplication::setApplicationName(QString::fromLatin1(TmcPlanRtApp().appName));
    QApplication::setOrganizationName(QStringLiteral("TMC Suite"));

    // Настройки программы (внешние просмотрщики, форматы вывода, мелодии) на
    // Windows лежали в реестре; здесь — в ~/.config/TMC_Suite/<программа>.conf.
    CWinApp::SetAppName(TmcPlanRtApp().appName);

    theme::setProgram(QString::fromLatin1(TmcPlanRtApp().appName).toLower());   // planarrt_h / planarrt_x
    theme::apply();
    seedViewerDefaults();

    // Ключи режима разбираются так же, как в Windows-версии: тем же кодом и по
    // всей командной строке целиком (в MFC это было theApp.m_lpCmdLine).
    QStringList tail = QApplication::arguments();
    if (!tail.isEmpty())
        tail.removeFirst();
    // Расчёт на видеокарте по настройке «Config -> Compute (GPU)»: ставит или
    // снимает TMC_GPU до первого расчёта (ключи /GPU и /CPU главнее настройки).
    tmcApplyComputeSettings(tail);

    // QApplication::arguments() уже без кавычек: довод с пробелами берётся в
    // кавычки снова, иначе слово из пути («k -Arb x\demo.tpl») ядро приняло бы
    // за ключ — правка А-15 считает ключом только слово вне кавычек (Т-10).
    QStringList quoted;
    for (const QString &arg : tail)
        quoted << ((arg.contains(QLatin1Char(' ')) || arg.contains(QLatin1Char('\t')))
                       ? QLatin1Char('"') + arg + QLatin1Char('"') : arg);
    QByteArray cmdLine = quoted.join(QLatin1Char(' ')).toLocal8Bit();
    cmdLine.append('\0');
    Set_CommandLine_Flags(cmdLine.data());

    MainWindow window;
    window.show();
#ifdef _WIN32
    // Главное окно для закрытия после пакетного расчёта (в MFC — InitInstance).
    SetRunMainWnd(reinterpret_cast<HWND>(&window));
#endif

    // Файл задания — первый довод, не являющийся ключом. Ключом считается
    // только довод, начинающийся с дефиса: в Windows-версии ключи писались
    // ещё и через косую черту (/Ar), но на Linux с косой черты начинается
    // любой полный путь, и файл задания принимался бы за ключ.
    QString taskFile;
    for (const QString &arg : tail) {
        if (arg.startsWith(QLatin1Char('-')))
            continue;
#ifdef _WIN32
        // Windows: ключи пишутся и через косую черту (/Arb, /Time, /GPU), как в
        // MFC-версии; путь к файлу с косой черты здесь не начинается.
        if (arg.startsWith(QLatin1Char('/')))
            continue;
#endif
        taskFile = arg;
        break;
    }

#ifdef _WIN32
    // Пакетный режим (/Ar, /Arb), как InitInstance MFC-версии: задание должно
    // быть указано и существовать, иначе выход с кодом 1 без окон.
    if (IsAutoRun()) {
        if (taskFile.isEmpty()) {
            ConsolePrint("ERROR: no .tpl file in the command line");
            SetRunExitCode(TMC_EXIT_TPL_ERROR);
            ConsoleReportResult();
            return GetRunExitCode();
        }
        if (!QFileInfo(taskFile).isFile()) {
            ConsolePrint("ERROR: can't open file %s", taskFile.toLocal8Bit().constData());
            SetRunExitCode(TMC_EXIT_TPL_ERROR);
            ConsoleReportResult();
            return GetRunExitCode();
        }
        // Путь с буквами вне кодовой страницы Windows расчётный код открыл бы не тот
        // (café -> cafe: посчитал бы чужое задание и записал результат в чужую
        // папку) — ошибка задания, код 1, без окон (Т-20).
        if (!tmcui::pathFitsCodePage(QFileInfo(taskFile).absoluteFilePath())) {
            ConsolePrint("ERROR: the task path contains characters outside the Windows code page "
                         "(only Latin and Cyrillic are supported): %s",
                         taskFile.toLocal8Bit().constData());
            SetRunExitCode(TMC_EXIT_TPL_ERROR);
            ConsoleReportResult();
            return GetRunExitCode();
        }
        ConsolePrint("File: %s", taskFile.toLocal8Bit().constData());
    }
#endif
    if (!taskFile.isEmpty())
        window.openDocument(taskFile);

    const int rc = app.exec();
#ifdef _WIN32
    // Код завершения — как у MFC-версии (ExitInstance): 0 — всё посчитано,
    // 1 — ошибка задания, 2 — ошибка расчёта, 3 — остановлено.
    (void)rc;
    return GetRunExitCode();
#else
    return rc;
#endif
}
