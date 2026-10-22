// Реализация слоя совместимости для классов документа (см. tmc_mfc_doc.h).
//
// Здесь живут четыре вещи, которых на Linux нет в готовом виде:
//   * настройки программы вместо реестра Windows — ini-файл в ~/.config;
//   * время последней записи файла (FILETIME) — из stat();
//   * запуск внешнего редактора вместо CreateProcess — fork + execvp;
//   * пустой описатель файла, нужный только ради времени записи.
//
// Вычислений здесь нет.

/// @file tmc_mfc_doc.cpp
/// @brief Реализация каркаса документа: настройки в ini-файле, обработчик нехватки памяти, время файла, запуск программ.
///
/// Часть модуля «Слой совместимости». Входит в библиотеку `tmc_platform`
/// (обе сборки — Windows/MinGW и Linux). Вычислений здесь нет.
/// @see https://www.tamic.ru/docs/code/compat.html#settings
#if !defined(_WIN32) || defined(TMC_QT_WIN)

#include "tmc_mfc_doc.h"

#include <cstdlib>
#include <new>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <sys/stat.h>
#include <sys/types.h>
#include <cctype>
#ifdef _WIN32
#include <direct.h>      // _mkdir
#include <filesystem>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace {

/// Имя программы: имя файла настроек без расширения.
std::string g_appName = "TMC_Suite";

#ifdef _WIN32
/// Тип пути к файлу настроек: Unicode-путь в Windows, строка вне Windows.
typedef std::filesystem::path ConfigPath;
#else
typedef std::string ConfigPath;
#endif

/// @brief Возвращает путь к файлу настроек программы, при необходимости создаёт каталог.
///
/// Windows: `%APPDATA%\TMC_Suite_Qt\<программа>.conf`; если файла там нет, но есть
/// файл по старому ошибочному пути `<APPDATA>TMC_Suite_Qt\…`, он копируется.
/// Linux: `~/.config/TMC_Suite/<программа>.conf`.
/// @return Путь к файлу (сам файл может не существовать).
ConfigPath configPath()
{
#ifdef _WIN32
    // Windows (Qt-версия): %APPDATA%\TMC_Suite_Qt\<программа>.conf — отдельно от
    // реестра MFC-версии, чтобы две версии на одной машине не мешали друг другу.
    // Путь берётся в Unicode: профиль пользователя может содержать буквы вне
    // кодовой страницы Windows (Т-20).
    // GetEnvironmentVariableW, а не _wgetenv: у программы с main() рантайм MinGW
    // широкое окружение не заводит, и _wgetenv возвращает NULL.
    std::wstring appdata(32768, L'\0');
    const DWORD n = ::GetEnvironmentVariableW(L"APPDATA", &appdata[0], DWORD(appdata.size()));
    appdata.resize(n < appdata.size() ? n : 0);
    const std::filesystem::path base = appdata.empty() ? std::wstring(L".") : appdata;
    const std::filesystem::path dir = base / L"TMC_Suite_Qt";
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    const std::filesystem::path file = dir / std::filesystem::path(g_appName + ".conf");
    // До исправления L-1 в имени папки не было разделителя ("\T" вместо "\\T"), и
    // настройки лежали рядом с APPDATA: "<APPDATA>TMC_Suite_Qt\<программа>.conf".
    // Если на новом месте файла ещё нет — переносим копию старого.
    if (!std::filesystem::exists(file, ec)) {
        const std::filesystem::path old =
            std::filesystem::path(base.native() + L"TMC_Suite_Qt") / file.filename();
        if (std::filesystem::is_regular_file(old, ec))
            std::filesystem::copy_file(old, file, ec);
    }
    return file;
#else
    const char* home = std::getenv("HOME");
    std::string base = home ? home : ".";
    // Каталоги создаём по одному: на свежей системе ~/.config может ещё не быть,
    // а mkdir создаёт только последний уровень.
    const std::string config = base + "/.config";
    ::mkdir(config.c_str(), 0755);
    const std::string dir = config + "/TMC_Suite";
    ::mkdir(dir.c_str(), 0755);
    return dir + "/" + g_appName + ".conf";
#endif
}

// Разделы ini держим в памяти: файл маленький, а обращений к нему немного.
/// Содержимое ini-файла: раздел -> (ключ -> значение).
typedef std::map<std::string, std::map<std::string, std::string> > Ini;

/// @brief Читает ini-файл настроек целиком.
///
/// Пропускает пустые строки и строки с `;`; `\r` в конце строки отбрасывается;
/// строка без `=` вне заголовка раздела игнорируется.
/// @return Разделы и ключи; пусто, если файла нет.
Ini iniRead()
{
    Ini ini;
    std::ifstream in(configPath());
    std::string line, section;
    while (std::getline(in, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty() || line[0] == ';')
            continue;
        if (line[0] == '[' && line[line.size() - 1] == ']') {
            section = line.substr(1, line.size() - 2);
            continue;
        }
        std::string::size_type eq = line.find('=');
        if (eq == std::string::npos)
            continue;
        ini[section][line.substr(0, eq)] = line.substr(eq + 1);
    }
    return ini;
}

/// @brief Возвращает копию настроек в памяти (читается из файла при первом обращении).
/// @return Ссылка на статическую копию настроек «раздел -> ключ -> значение».
Ini& iniCache()
{
    static Ini ini;
    static bool loaded = false;
    if (!loaded) {
        loaded = true;
        ini = iniRead();
    }
    return ini;
}

/// @brief Перезаписывает файл настроек целиком.
/// @param[in] ini  содержимое; при ошибке открытия файла ничего не происходит
void iniSave(const Ini& ini)
{
    std::ofstream out(configPath(), std::ios::trunc);
    if (!out)
        return;
    for (Ini::const_iterator s = ini.begin(); s != ini.end(); ++s) {
        out << "[" << s->first << "]\n";
        for (std::map<std::string, std::string>::const_iterator e = s->second.begin();
             e != s->second.end(); ++e)
            out << e->first << "=" << e->second << "\n";
        out << "\n";
    }
}

// Время файла Windows считает в интервалах по 100 нс от 1 января 1601 года,
// Unix — в секундах от 1 января 1970. Код документа сравнивает эти значения
// только между собой (не изменился ли файл), но пересчёт всё равно делаем
// правильный: так значение остаётся осмысленным и при переносе.
/// Разница эпох Windows (1601) и Unix (1970) в интервалах по 100 нс.
const unsigned long long EPOCH_DIFF_100NS = 116444736000000000ULL;

} // namespace

// --- Настройки ---------------------------------------------------------------

void CWinApp::SetAppName(const char* name)
{
    if (name && *name)
        g_appName = name;
    AfxGetApp()->m_pszExeName = g_appName.c_str();
}

// Нехватка памяти в new — исключение CMemoryException*, как в MFC: код расчёта
// ловит его (catch( CException *e )) и завершает шаг с кодом ошибки.
namespace {
/// @brief Статический объект, при запуске программы ставящий обработчик `new`, который бросает `new CMemoryException`.
struct TmcNewHandlerInstaller
{
    TmcNewHandlerInstaller()
    {
        std::set_new_handler([]() { throw new CMemoryException; });
    }
} s_tmcNewHandlerInstaller;
/// @var s_tmcNewHandlerInstaller
/// Единственный экземпляр `TmcNewHandlerInstaller`: его конструктор при запуске программы ставит обработчик `new`.
} // namespace


CString CWinApp::GetProfileString(const char* section, const char* entry,
                                        const char* def)
{
    Ini& ini = iniCache();
    Ini::const_iterator s = ini.find(section ? section : "");
    if (s != ini.end()) {
        std::map<std::string, std::string>::const_iterator e =
            s->second.find(entry ? entry : "");
        if (e != s->second.end())
            return CString(e->second.c_str());
    }
    return CString(def ? def : "");
}

BOOL CWinApp::WriteProfileString(const char* section, const char* entry,
                                       const char* value)
{
    // Файл перечитывается перед записью: значения, которые записал другой
    // экземпляр программы, не затираются копией из памяти этого (Т-26), — как в
    // реестре MFC-версии, где каждое значение пишется отдельно.
    Ini ini = iniRead();
    ini[section ? section : ""][entry ? entry : ""] = value ? value : "";
    iniSave(ini);
    iniCache() = ini;
    return TRUE;
}

int CWinApp::GetProfileInt(const char* section, const char* entry, int def)
{
    CString s = GetProfileString(section, entry, "");
    if (s.IsEmpty())
        return def;
    return std::atoi(s.GetString());
}

BOOL CWinApp::WriteProfileInt(const char* section, const char* entry, int value)
{
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%d", value);
    return WriteProfileString(section, entry, buf);
}

CWinApp* AfxGetApp()
{
    static CWinApp app;
    return &app;
}

#ifndef _WIN32 // Windows: CompareFileTime, CreateFile, GetFileTime, CloseHandle настоящие
int CompareFileTime(const FILETIME* a, const FILETIME* b)
{
    if (!a || !b)
        return 0;
    unsigned long long ta =
        (static_cast<unsigned long long>(a->dwHighDateTime) << 32) | a->dwLowDateTime;
    unsigned long long tb =
        (static_cast<unsigned long long>(b->dwHighDateTime) << 32) | b->dwLowDateTime;
    if (ta < tb) return -1;
    if (ta > tb) return 1;
    return 0;
}

// --- Время записи файла ------------------------------------------------------
// CreateFile здесь не открывает файл по-настоящему: код документа использует
// описатель только чтобы спросить время записи и сразу закрыть. Поэтому в
// описателе лежит само время, снятое через stat().

namespace {
struct FileTimeHandle
{
    FILETIME write;
    bool     valid;
};
} // namespace

HANDLE CreateFile(const char* fileName, DWORD /*access*/, DWORD /*shareMode*/,
                  void* /*security*/, DWORD /*creation*/, DWORD /*flags*/,
                  HANDLE /*templ*/)
{
    FileTimeHandle* h = new FileTimeHandle();
    h->valid = false;
    h->write.dwLowDateTime = 0;
    h->write.dwHighDateTime = 0;

    struct stat st;
    if (fileName && ::stat(fileName, &st) == 0) {
        unsigned long long t =
            static_cast<unsigned long long>(st.st_mtime) * 10000000ULL + EPOCH_DIFF_100NS;
        h->write.dwLowDateTime  = static_cast<DWORD>(t & 0xffffffffULL);
        h->write.dwHighDateTime = static_cast<DWORD>(t >> 32);
        h->valid = true;
    }
    return static_cast<HANDLE>(h);
}

BOOL GetFileTime(HANDLE handle, FILETIME* creation, FILETIME* access, FILETIME* write)
{
    FileTimeHandle* h = static_cast<FileTimeHandle*>(handle);
    if (!h)
        return FALSE;
    if (creation) *creation = h->write;
    if (access)   *access   = h->write;
    if (write)    *write    = h->write;
    return h->valid ? TRUE : FALSE;
}

BOOL CloseHandle(HANDLE handle)
{
    delete static_cast<FileTimeHandle*>(handle);
    return TRUE;
}
#endif // !_WIN32

// --- Имя запускаемой программы ------------------------------------------------
//
// Код Windows-версии зовёт соседние программы пакета по их windows-именам:
// «TMCROS.EXE», «FLDVIEW.EXE», «TMCGROUT.EXE», «TMC_DN.EXE», а внешний
// редактор — «NOTEPAD.EXE». На Linux эти файлы называются иначе и лежат рядом
// с самой программой. Поэтому имя переводится здесь: пользователь нажимает ту
// же кнопку и получает тот же результат.
//
// Настройка пользователя (если он указал свой путь в разделе Config) имеет
// приоритет: перевод включается только тогда, когда указанного файла нет.
namespace {

#ifdef _WIN32
/// @brief Возвращает каталог текущей программы с завершающим разделителем (пусто при ошибке).
/// @return Каталог с завершающим разделителем пути; пустая строка — путь не получен.
std::string executableDirectory()
{
    char buf[MAX_PATH * 2];
    const DWORD n = ::GetModuleFileNameA(NULL, buf, sizeof(buf));
    if (n == 0 || n >= sizeof(buf))
        return std::string();
    std::string path(buf, n);
    const std::string::size_type slash = path.find_last_of("\\/");   // L-2: оба разделителя
    return slash == std::string::npos ? std::string() : path.substr(0, slash + 1);
}

/// @brief Проверяет, что по пути лежит файл (Windows) или исполняемый файл (Linux).
/// @param[in] path  путь
/// @return true — файл есть.
bool isExecutable(const std::string& path)
{
    const DWORD a = path.empty() ? INVALID_FILE_ATTRIBUTES : ::GetFileAttributesA(path.c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}
#else
/// @brief Возвращает каталог текущей программы (`/proc/self/exe`) с завершающим `/`; пусто при ошибке.
/// @return Каталог с завершающим `/`; пустая строка — путь не получен.
std::string executableDirectory()
{
    char buf[4096];
    const ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n <= 0)
        return std::string();
    buf[n] = 0;
    std::string path(buf);
    const std::string::size_type slash = path.find_last_of('/');
    return slash == std::string::npos ? std::string() : path.substr(0, slash + 1);
}

/// @brief Проверяет, что файл существует и исполняем владельцем.
/// @param[in] path  путь
/// @return true — исполняемый файл.
bool isExecutable(const std::string& path)
{
    struct stat st;
    return !path.empty() && ::stat(path.c_str(), &st) == 0 && (st.st_mode & S_IXUSR);
}
#endif

/// @brief Переводит имя программы Windows-версии в имя Qt-версии (см. `TmcResolveProgramName`).
///
/// Шаги: существующий путь возвращается как есть; отбрасываются каталог и `.exe`;
/// имя приводится к нижнему регистру; `fldview` -> `fieldview`, `planrt_h/x` ->
/// `planarrt_h/x`; ищется рядом с текущей программой. Вне Windows редакторы
/// (`notepad`, `write`, `wordpad`, `winword`) заменяются на `xdg-open`.
/// @param[in] requested  запрошенное имя или путь
/// @return Путь или имя для запуска.
std::string resolveProgramImpl(const std::string& requested)
{
    if (requested.empty())
        return requested;

    // Путь указан и существует — ничего не меняем.
    if (requested.find_first_of("/\\") != std::string::npos && isExecutable(requested))
        return requested;

    // Берём только имя файла и убираем расширение Windows.
    std::string name = requested;
    const std::string::size_type slash = name.find_last_of("/\\");
    if (slash != std::string::npos)
        name = name.substr(slash + 1);
    const std::string::size_type dot = name.find_last_of('.');
    if (dot != std::string::npos) {
        std::string ext = name.substr(dot);
        for (size_t i = 0; i < ext.size(); ++i)
            ext[i] = char(::tolower(ext[i]));
        if (ext == ".exe")
            name = name.substr(0, dot);
    }

    std::string lower = name;
    for (size_t i = 0; i < lower.size(); ++i)
        lower[i] = char(::tolower(lower[i]));

    // Соответствие программ пакета.
    if (lower == "fldview")      lower = "fieldview";
    else if (lower == "planrt_h") lower = "planarrt_h";
    else if (lower == "planrt_x") lower = "planarrt_x";

#ifdef _WIN32
    // Windows: редактор и прочие системные программы — как есть, по PATH.
    const std::string dir = executableDirectory();
    if (!dir.empty() && isExecutable(dir + lower + ".exe"))
        return dir + lower + ".exe"; // рядом с текущей программой
    return requested;
#else
    // Текстовый редактор: на Linux открываем файл тем, что назначено в системе.
    if (lower == "notepad" || lower == "write" || lower == "wordpad"
        || lower == "winword")
        return std::string("xdg-open");

    const std::string dir = executableDirectory();
    if (!dir.empty() && isExecutable(dir + lower))
        return dir + lower;          // рядом с текущей программой
#endif

    return lower;                    // пусть ищет по PATH
}

} // namespace

std::string TmcResolveProgramName(const char* windowsName)
{
    return resolveProgramImpl(windowsName ? windowsName : "");
}

// --- Запуск внешней программы ------------------------------------------------
#ifdef _WIN32
// Windows: настоящий CreateProcessA; меняется только имя программы пакета.
BOOL TmcCreateProcess(const char* appName, char* commandLine,
                      void* processAttrs, void* threadAttrs, BOOL inheritHandles,
                      DWORD creationFlags, void* environment, const char* currentDir,
                      STARTUPINFOA* startupInfo, PROCESS_INFORMATION* processInfo)
{
    std::string cmd = commandLine ? commandLine : "";
    if (!appName || !*appName) {
        // Первое слово командной строки — имя программы.
        std::string::size_type b = cmd.find_first_not_of(" 	");
        if (b != std::string::npos) {
            std::string::size_type e;
            std::string first;
            if (cmd[b] == '"') { e = cmd.find('"', b + 1); first = cmd.substr(b + 1, e == std::string::npos ? std::string::npos : e - b - 1); if (e != std::string::npos) ++e; }
            else { e = cmd.find_first_of(" 	", b); first = cmd.substr(b, e == std::string::npos ? std::string::npos : e - b); }
            const std::string res = resolveProgramImpl(first);
            if (res != first)
                cmd = "\"" + res + "\"" + (e == std::string::npos ? std::string() : cmd.substr(e));
        }
    }
    std::vector<char> buf(cmd.begin(), cmd.end());
    buf.push_back(0);
    STARTUPINFOA si;
    if (!startupInfo) { ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si); startupInfo = &si; }
    return ::CreateProcessA(appName, buf.data(), (LPSECURITY_ATTRIBUTES)processAttrs,
                            (LPSECURITY_ATTRIBUTES)threadAttrs, inheritHandles, creationFlags,
                            environment, currentDir, startupInfo, processInfo);
}
#else

BOOL CreateProcess(const char* appName, char* commandLine,
                   void* /*processAttrs*/, void* /*threadAttrs*/, BOOL /*inherit*/,
                   DWORD /*creationFlags*/, void* /*environment*/, const char* currentDir,
                   STARTUPINFO* /*startupInfo*/, PROCESS_INFORMATION* processInfo)
{
    if (processInfo) {
        processInfo->hProcess = 0;
        processInfo->hThread = 0;
        processInfo->dwProcessId = 0;
        processInfo->dwThreadId = 0;
    }

    // Разбор командной строки по пробелам, кавычки сохраняют аргумент целиком —
    // так же ведёт себя Windows при разборе lpCommandLine.
    std::vector<std::string> args;
    if (appName && *appName)
        args.push_back(appName);
    if (commandLine) {
        std::string cur;
        bool quoted = false;
        for (const char* p = commandLine; *p; ++p) {
            if (*p == '"') { quoted = !quoted; continue; }
            if (!quoted && (*p == ' ' || *p == '\t')) {
                if (!cur.empty()) { args.push_back(cur); cur.clear(); }
                continue;
            }
            cur += *p;
        }
        if (!cur.empty())
            args.push_back(cur);
    }
    if (args.empty())
        return FALSE;

    // Первое слово — имя программы: переводим windows-имя в наше.
    args[0] = resolveProgramImpl(args[0]);

    std::vector<char*> argv;
    for (size_t i = 0; i < args.size(); ++i)
        argv.push_back(const_cast<char*>(args[i].c_str()));
    argv.push_back(0);

    pid_t pid = ::fork();
    if (pid < 0)
        return FALSE;
    if (pid == 0) {
        if (currentDir && *currentDir)
            if (::chdir(currentDir) != 0)
                ::_exit(127);
        ::execvp(argv[0], &argv[0]);
        ::_exit(127);               // execvp вернулся — программы нет
    }

    if (processInfo) {
        processInfo->hProcess = reinterpret_cast<HANDLE>(static_cast<long>(pid));
        processInfo->dwProcessId = static_cast<DWORD>(pid);
    }
    return TRUE;
}

#endif

#endif // !_WIN32
