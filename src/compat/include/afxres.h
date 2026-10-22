// afxres.h — стандартные идентификаторы команд MFC.
//
// Ресурсы и код Windows-версии ссылаются на команды каркаса (открыть файл,
// печать, окна, справка) по этим номерам. Значения обязаны совпадать с MFC:
// они стоят в .rc, и по ним же собирается меню и панель в Qt-версии.

/// @file afxres.h
/// @brief Стандартные идентификаторы команд каркаса MFC (`ID_FILE_OPEN` …), стили панелей и окон.
///
/// Часть модуля «Слой совместимости». Значения совпадают с MFC: на них
/// ссылаются ресурсы `.rc` и оболочки программ при построении меню.
/// @see https://www.tamic.ru/docs/code/compat.html#headers
#pragma once

#if !defined(_WIN32) || defined(TMC_QT_WIN)

// --- Файл ---------------------------------------------------------------------
/// Команды меню «Файл»; `ID_FILE_MRU_FILE1…4` — пункты последних файлов.
#define ID_FILE_NEW             0xE100
/// Команда «Файл -> Открыть».
#define ID_FILE_OPEN            0xE101
/// Команда «Файл -> Закрыть».
#define ID_FILE_CLOSE           0xE102
/// Команда «Файл -> Сохранить».
#define ID_FILE_SAVE            0xE103
/// Команда «Файл -> Сохранить как».
#define ID_FILE_SAVE_AS         0xE104
/// Команда «Файл -> Параметры страницы».
#define ID_FILE_PAGE_SETUP      0xE105
/// Команда «Файл -> Настройка печати».
#define ID_FILE_PRINT_SETUP     0xE106
/// Команда «Файл -> Печать».
#define ID_FILE_PRINT           0xE107
/// Команда печати без диалога.
#define ID_FILE_PRINT_DIRECT    0xE108
/// Команда «Файл -> Предварительный просмотр».
#define ID_FILE_PRINT_PREVIEW   0xE109
/// Команда «Файл -> Обновить» (OLE).
#define ID_FILE_UPDATE          0xE10A
/// Команда «Файл -> Сохранить копию как».
#define ID_FILE_SAVE_COPY_AS    0xE10B
/// Команда «Файл -> Отправить».
#define ID_FILE_SEND_MAIL       0xE10C
/// Первый номер пунктов последних файлов.
#define ID_FILE_MRU_FIRST       0xE110
/// Пункт последнего файла № 1.
#define ID_FILE_MRU_FILE1       0xE110
/// Пункт последнего файла № 2.
#define ID_FILE_MRU_FILE2       0xE111
/// Пункт последнего файла № 3.
#define ID_FILE_MRU_FILE3       0xE112
/// Пункт последнего файла № 4.
#define ID_FILE_MRU_FILE4       0xE113
/// Последний номер пунктов последних файлов.
#define ID_FILE_MRU_LAST        0xE11F

// --- Правка -------------------------------------------------------------------
/// Команды меню «Правка».
#define ID_EDIT_CLEAR           0xE120
/// Команда «Правка -> Очистить всё».
#define ID_EDIT_CLEAR_ALL       0xE121
/// Команда «Правка -> Копировать».
#define ID_EDIT_COPY            0xE122
/// Команда «Правка -> Вырезать».
#define ID_EDIT_CUT             0xE123
/// Команда «Правка -> Найти».
#define ID_EDIT_FIND            0xE124
/// Команда «Правка -> Вставить».
#define ID_EDIT_PASTE           0xE125
/// Команда «Правка -> Повторить поиск».
#define ID_EDIT_REPEAT          0xE128
/// Команда «Правка -> Заменить».
#define ID_EDIT_REPLACE         0xE129
/// Команда «Правка -> Выделить всё».
#define ID_EDIT_SELECT_ALL      0xE12A
/// Команда «Правка -> Отменить».
#define ID_EDIT_UNDO            0xE12B
/// Команда «Правка -> Вернуть».
#define ID_EDIT_REDO            0xE12C

// --- Окна ---------------------------------------------------------------------
/// Команды меню «Окно».
#define ID_WINDOW_NEW           0xE130
/// Команда «Окно -> Упорядочить значки».
#define ID_WINDOW_ARRANGE       0xE131
/// Команда «Окно -> Каскадом».
#define ID_WINDOW_CASCADE       0xE132
/// Команда «Окно -> Сверху вниз».
#define ID_WINDOW_TILE_HORZ     0xE133
/// Команда «Окно -> Слева направо».
#define ID_WINDOW_TILE_VERT     0xE134
/// Команда «Окно -> Разделить».
#define ID_WINDOW_SPLIT         0xE135

// --- Программа и справка ------------------------------------------------------
/// Команды «О программе», «Выход» и справки.
#define ID_APP_ABOUT            0xE140
/// Команда «Выход».
#define ID_APP_EXIT             0xE141
/// Команда «Справка -> Содержание».
#define ID_HELP_INDEX           0xE142
/// Команда «Справка -> Как пользоваться справкой».
#define ID_HELP_USING           0xE144
/// Команда контекстной справки (Shift+F1).
#define ID_CONTEXT_HELP         0xE145
/// Команда справки (F1).
#define ID_HELP                 0xE146
/// Команда справки по умолчанию.
#define ID_DEFAULT_HELP         0xE143

// --- Вид ----------------------------------------------------------------------
/// Команды показа панели инструментов и строки состояния.
#define ID_VIEW_TOOLBAR         0xE800
/// Команда «Вид -> Строка состояния».
#define ID_VIEW_STATUS_BAR      0xE801

// --- Строка состояния ---------------------------------------------------------
/// Идентификаторы полей строки состояния; `ID_SEPARATOR` — разделитель.
#define ID_INDICATOR_EXT        0xE700
/// Индикатор Caps Lock.
#define ID_INDICATOR_CAPS       0xE701
/// Индикатор Num Lock.
#define ID_INDICATOR_NUM        0xE702
/// Индикатор Scroll Lock.
#define ID_INDICATOR_SCRL       0xE703
/// Индикатор режима замены.
#define ID_INDICATOR_OVR        0xE704
/// Индикатор записи макроса.
#define ID_INDICATOR_REC        0xE705
/// Разделитель (номер 0).
#define ID_SEPARATOR            0

/// Строковые ресурсы каркаса: заголовок программы и текст строки состояния в покое.
#define AFX_IDS_APP_TITLE       0xE000
/// Строка «Готово» в строке состояния.
#define AFX_IDS_IDLEMESSAGE     0xE001

// --- Стили и флаги, встречающиеся в коде --------------------------------------
/// Стили панелей (`CBRS_*`), окон (`WS_*`), показа (`SW_*`) и заголовка рамки (`FWS_*`) — значения MFC/Windows.
#define CBRS_TOP            0x0001
/// Панель у нижнего края.
#define CBRS_BOTTOM         0x0002
/// Панель у любого края.
#define CBRS_ALIGN_ANY      0x000F
/// Подсказки у кнопок панели.
#define CBRS_TOOLTIPS       0x0010
/// Текст в строке состояния при наведении на кнопку.
#define CBRS_FLYBY          0x0020
/// Ручка перетаскивания панели.
#define CBRS_GRIPPER        0x00400000
/// Панель меняет размер.
#define CBRS_SIZE_DYNAMIC   0x00800000
/// Стиль окна: дочернее.
#ifndef WS_CHILD
#define WS_CHILD            0x40000000L
#endif
/// Стиль окна: видимое.
#ifndef WS_VISIBLE
#define WS_VISIBLE          0x10000000L
#endif
/// Стиль окна: не рисовать поверх соседей.
#ifndef WS_CLIPSIBLINGS
#define WS_CLIPSIBLINGS     0x04000000L
#endif
/// Стиль окна: не рисовать поверх дочерних.
#ifndef WS_CLIPCHILDREN
#define WS_CLIPCHILDREN     0x02000000L
#endif
/// Стиль окна: обычное окно верхнего уровня.
#ifndef WS_OVERLAPPEDWINDOW
#define WS_OVERLAPPEDWINDOW 0x00CF0000L
#endif
/// Показ окна: показать.
#define SW_SHOW             5
/// Показ окна: скрыть.
#define SW_HIDE             0
/// Показ окна: в обычном размере.
#define SW_SHOWNORMAL       1
/// Показ окна: свёрнутым.
#define SW_SHOWMINIMIZED    2
/// Показ окна: развёрнутым.
#define SW_SHOWMAXIMIZED    3
/// Стиль окна: перекрываемое.
#ifndef WS_OVERLAPPED
#define WS_OVERLAPPED       0x00000000L
#endif
/// Стиль окна: с заголовком.
#ifndef WS_CAPTION
#define WS_CAPTION          0x00C00000L
#endif
/// Стиль окна: рамка изменения размера.
#ifndef WS_THICKFRAME
#define WS_THICKFRAME       0x00040000L
#endif
/// Стиль окна: системное меню.
#ifndef WS_SYSMENU
#define WS_SYSMENU          0x00080000L
#endif
/// Стиль окна: кнопка «Свернуть».
#ifndef WS_MINIMIZEBOX
#define WS_MINIMIZEBOX      0x00020000L
#endif
/// Стиль окна: кнопка «Развернуть».
#ifndef WS_MAXIMIZEBOX
#define WS_MAXIMIZEBOX      0x00010000L
#endif
/// Стиль рамки: добавлять имя документа к заголовку.
#define FWS_ADDTOTITLE      0x00008000L
/// Стиль рамки: имя документа перед названием программы.
#define FWS_PREFIXTITLE     0x00004000L

// Отладочный вывод MFC. В рабочей программе он не нужен.
/// Отладочный вывод MFC: пустые макросы.
#define TRACE0(s)
/// Отладочный вывод MFC с одним значением: пустой.
#define TRACE1(s, a)
/// Отладочный вывод MFC с двумя значениями: пустой.
#define TRACE2(s, a, b)


/// Сведения о классе времени выполнения MFC: пустые макросы.
#define DECLARE_DYNAMIC(cls)
/// Реализация сведений о классе MFC: пустая.
#define IMPLEMENT_DYNAMIC(cls, base)

#endif // !_WIN32
