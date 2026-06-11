//============================================================================
// TmcDocLink.h  —  единая логика ссылки на локальную документацию TMC Suite
//----------------------------------------------------------------------------
// Подключается во ВСЕ 6 программ (вьюверы, ядра, FieldView) из окна About.
//
// Назначение:
//   - найти HTML-файл руководства пользователя, НЕ завися от жёсткого пути;
//   - открыть его в браузере по умолчанию (ShellExecute);
//   - если документация не найдена — предложить онлайн-версию на сайте.
//
// ВАЖНО про кодировку:
//   Этот файл сохранён в UTF-8 С BOM специально для того, чтобы компилятор
//   MSVC корректно сконвертировал русские строковые литералы в кодировку
//   выполнения (на русской Windows — CP1251), как и в остальном коде проекта.
//   Весь русский текст интерфейса сосредоточен ЗДЕСЬ, чтобы файлы .cpp/.rc
//   (сохранённые в Windows-1251 без BOM) оставались только-ASCII и не
//   повреждались редакторами. См. историю с багом #8 (порча символов UTF-8).
//
// Поиск пути (по порядку, до первого попадания):
//   1) относительно самого .exe (GetModuleFileName), с подъёмом вверх по
//      дереву каталогов — обычный случай запуска из папки установки;
//   2) из реестра HKLM/HKCU\Software\TMC Suite, значение "InstallPath" —
//      его пишет установщик Inno Setup. Работает, даже если .exe перенесли;
//   3) не нашли -> сообщение и предложение открыть сайт.
//============================================================================
#ifndef TMC_DOCLINK_H__
#define TMC_DOCLINK_H__

#include <windows.h>
#include <shellapi.h>

//----------------------------------------------------------------------------
// Настраиваемые константы
//----------------------------------------------------------------------------

// Подпись кликабельной ссылки в окне About.
static const TCHAR TMC_DOCLINK_CAPTION[] = _T("Открыть руководство пользователя");

// Адрес сайта с онлайн-документацией (фолбэк, если локальная не найдена).
// При необходимости поменять на актуальный раздел документации сайта.
static const TCHAR TMC_DOC_SITE_URL[] = _T("http://www.tamic.ru");

// Ключ реестра, в который установщик записывает путь установки.
static const TCHAR TMC_DOC_REG_KEY[]   = _T("Software\\TMC Suite");
static const TCHAR TMC_DOC_REG_VALUE[] = _T("InstallPath");

//----------------------------------------------------------------------------
// Вспомогательное: существует ли файл
//----------------------------------------------------------------------------
inline bool TmcDoc_FileExists(LPCTSTR path)
{
    DWORD a = ::GetFileAttributes(path);
    return (a != INVALID_FILE_ATTRIBUTES) && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

//----------------------------------------------------------------------------
// Проверить набор типовых раскладок документации внутри базовой папки.
// relPage — относительный путь html, напр. "viewers\\tmcgrout.html".
//----------------------------------------------------------------------------
inline bool TmcDoc_CheckBase(const CString& base, LPCTSTR relPage, CString& out)
{
    static const LPCTSTR subs[] = {
        _T("docs\\user-manual\\"),                       // раскладка установщика
        _T("docs\\user-manual\\sphinx\\_build\\html\\"), // дерево разработки
        _T("docs\\"),
        _T("")
    };
    CString root = base;
    root.TrimRight(_T("\\"));
    for (int i = 0; i < (int)(sizeof(subs) / sizeof(subs[0])); ++i)
    {
        CString cand;
        cand.Format(_T("%s\\%s%s"), (LPCTSTR)root, subs[i], relPage);
        if (TmcDoc_FileExists(cand)) { out = cand; return true; }
    }
    return false;
}

//----------------------------------------------------------------------------
// Прочитать путь установки из реестра (HKLM или HKCU).
//----------------------------------------------------------------------------
inline bool TmcDoc_ReadRegInstallPath(CString& out)
{
    HKEY roots[2] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };
    for (int r = 0; r < 2; ++r)
    {
        REGSAM sam = KEY_READ;
#ifdef KEY_WOW64_64KEY
        sam |= KEY_WOW64_64KEY; // читаем 64-битную ветку из 32-битного процесса
#endif
        HKEY h = NULL;
        if (::RegOpenKeyEx(roots[r], TMC_DOC_REG_KEY, 0, sam, &h) == ERROR_SUCCESS)
        {
            TCHAR  val[MAX_PATH] = { 0 };
            DWORD  cb   = sizeof(val);
            DWORD  type = 0;
            LONG   rc = ::RegQueryValueEx(h, TMC_DOC_REG_VALUE, NULL, &type,
                                          (LPBYTE)val, &cb);
            ::RegCloseKey(h);
            if (rc == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ)
                && val[0] != 0)
            {
                out = val;
                return true;
            }
        }
    }
    return false;
}

//----------------------------------------------------------------------------
// Главный поиск: найти полный путь к html-странице руководства.
//----------------------------------------------------------------------------
inline bool TmcDoc_Find(LPCTSTR relPage, CString& out)
{
    // 1) относительно .exe, поднимаясь вверх по дереву
    TCHAR mod[MAX_PATH] = { 0 };
    if (::GetModuleFileName(NULL, mod, MAX_PATH))
    {
        CString dir = mod;
        int p = dir.ReverseFind(_T('\\'));
        if (p > 0) dir = dir.Left(p);

        CString base = dir;
        for (int up = 0; up < 6; ++up)
        {
            if (TmcDoc_CheckBase(base, relPage, out)) return true;
            int q = base.ReverseFind(_T('\\'));
            if (q <= 2) break;            // дошли до корня диска
            base = base.Left(q);
        }
    }

    // 2) из реестра (путь установки)
    CString reg;
    if (TmcDoc_ReadRegInstallPath(reg))
    {
        if (TmcDoc_CheckBase(reg, relPage, out)) return true;
    }

    return false;
}

//----------------------------------------------------------------------------
// Открыть документацию: локально, либо предложить сайт.
//----------------------------------------------------------------------------
inline void TmcDoc_Open(CWnd* pParent, LPCTSTR relPage)
{
    HWND hWnd = (pParent != NULL) ? pParent->GetSafeHwnd() : NULL;

    CString path;
    if (TmcDoc_Find(relPage, path))
    {
        ::ShellExecute(hWnd, _T("open"), path, NULL, NULL, SW_SHOWNORMAL);
        return;
    }

    CString msg =
        _T("Локальная документация не найдена.\n")
        _T("Возможно, она была удалена или перемещена после установки.\n\n")
        _T("Открыть онлайн-версию документации на сайте?");

    int r = ::MessageBox(hWnd, msg, _T("Документация TMC Suite"),
                         MB_ICONQUESTION | MB_YESNO);
    if (r == IDYES)
    {
        ::ShellExecute(hWnd, _T("open"), TMC_DOC_SITE_URL, NULL, NULL, SW_SHOWNORMAL);
    }
}

//----------------------------------------------------------------------------
// Оформить статик-контрол как гиперссылку (синий + подчёркнутый).
// Вызывать из OnInitDialog. linkFont — член диалога (CFont), хранит шрифт.
//----------------------------------------------------------------------------
inline void TmcDoc_InitLink(CWnd* pDlg, UINT idLink, CFont& linkFont)
{
    if (pDlg == NULL) return;
    CWnd* pCtl = pDlg->GetDlgItem(idLink);
    if (pCtl == NULL) return;

    pCtl->SetWindowText(TMC_DOCLINK_CAPTION);

    LOGFONT lf;
    ::ZeroMemory(&lf, sizeof(lf));
    CFont* cur = pCtl->GetFont();
    if (cur != NULL && cur->GetLogFont(&lf) == 0)
    {
        // запасной вариант, если шрифт получить не удалось
        cur = pDlg->GetFont();
        if (cur != NULL) cur->GetLogFont(&lf);
    }
    lf.lfUnderline = TRUE;

    linkFont.DeleteObject();
    if (linkFont.CreateFontIndirect(&lf))
        pCtl->SetFont(&linkFont);
}

//----------------------------------------------------------------------------
// Покрасить ссылку в синий. Вызывать из OnCtlColor диалога.
//----------------------------------------------------------------------------
inline void TmcDoc_ColorLink(CDC* pDC, CWnd* pWnd, UINT idLink)
{
    if (pDC != NULL && pWnd != NULL && pWnd->GetDlgCtrlID() == (int)idLink)
    {
        pDC->SetTextColor(RGB(0, 0, 238)); // классический «ссылочный» синий
        pDC->SetBkMode(TRANSPARENT);
    }
}

#endif // TMC_DOCLINK_H__
