// mainframe.cpp — рамка главного окна (CMainFrame) для PlanarRT_H и PlanarRT_X.
//
// Почему файл не взят из MFC-версии. MainFrm.cpp Windows-версии — это оконная
// обвязка MFC: создание панели инструментов и строки состояния, стыковка
// панелей, поиск активного дочернего окна MDI, чтение и запись положения окна.
// Всё это в Qt-версии делает сам интерфейс (см. mainwindow.cpp), а в слое
// совместимости соответствующих методов MFC (MDIGetActive, RecalcLayout,
// CMDIFrameWnd::OnClose) нет.
//
// Коду, перенесённому из Windows-версии, от рамки нужно ровно две вещи:
//   * строка состояния m_wndStatusBar — через неё работают PutTrace и
//     PutStatistics из PL_GLFUN.CPP;
//   * указатель на рамку в SetMainFramePointer__ — по нему расчётный узел
//     закрывает программу, досчитав последний шаг в пакетном режиме.
// Здесь определено только это. Ни одной формулы и ни одного формата файла.
//
// Диалог «Computation in progress. Stop and close?» при закрытии окна во время
// счёта перенесён в Qt-слой (MainWindow::closeEvent) — с тем же текстом и тем
// же порядком действий, что в CMainFrame::OnClose Windows-версии.

/// @file mainframe.cpp
/// @brief Рамка главного окна (`CMainFrame`) для кода ядра: строка состояния, указатель на рамку, закрытие по `WM_CLOSE`.
///
/// Часть модуля «Оболочка счётного ядра». Определяет те методы `CMainFrame`
/// (заголовок `mainfrm.h` из Windows-версии), которые нужны коду ядра в Qt-версии.
/// @see https://www.tamic.ru/docs/code/kernel-shell.html#status-bar
#include "resource.h"        // номера диалогов: их ждут заголовки Windows-версии
#include "MainFrm.h"
#include "PlanRT_HDoc.h"
#include "Pl_iofor.h"

// Карта сообщений рамки. В Windows-версии в ней два пункта — показ и отметка
// строки состояния (ID_VIEW_STATUS_BAR); в Qt-версии этим распоряжается само
// главное окно (пункт View -> Status Bar).
//
// Остаётся одно сообщение: WM_CLOSE. Его шлёт расчётный узел, досчитав
// последний шаг в пакетном режиме (`HwndMainFrame->PostMessage( WM_CLOSE )` в
// TmcRTHRectNode.cpp). Обработчик записан прямо здесь, а не отдельным методом:
// добавить метод в CMainFrame нельзя — заголовок MainFrm.h принадлежит снимку
// Windows-версии и не правится.
/// @cond TMC_MESSAGE_MAP
BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
    // Обработчик `WM_CLOSE` рамки: просьба ядра закрыть программу -> `TmcPostFrameClose`.
    // (тело карты сообщений скрыто от Doxygen: вызов addMessage он принимал за объявление функции)
    tmcMap.addMessage(UINT(WM_CLOSE), [](WPARAM, LPARAM) -> LRESULT {
        TmcPostFrameClose(WM_CLOSE);
        return 0;
    });
END_MESSAGE_MAP()
/// @endcond

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    // Стиль окна — тот же, что в Windows-версии: обычное окно с рамкой,
    // системным меню и кнопками свёртки/развёртки, открывается развёрнутым.
    cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE
             | WS_THICKFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
             | WS_MAXIMIZE;

    return CMDIFrameWnd::PreCreateWindow(cs);
}

int CMainFrame::OnCreate(LPCREATESTRUCT /*lpCreateStruct*/)
{
    // Панель инструментов и строка состояния в Qt-версии создаются главным
    // окном (mainwindow.cpp). Остаётся то, ради чего код Windows-версии зовёт
    // OnCreate: запомнить рамку, чтобы расчётный узел мог её закрыть.
    SetMainFramePointer__(this);
    return 0;
}

void CMainFrame::OnDestroy()
{
    // Windows-версия сохраняла положение окна в реестре (в Qt-версии — в
    // ~/.config/TMC_Suite/<программа>.conf, тот же раздел и тот же ключ).
    // Формат строки прежний: left right top bottom.
    WINDOWPLACEMENT sWndPl;
    sWndPl.length = sizeof(WINDOWPLACEMENT);
    if (GetWindowPlacement(&sWndPl))
    {
        if (sWndPl.showCmd == SW_SHOWNORMAL)
        {
            char szBuffer[N_BUF_MAX];
            sprintf(szBuffer, "%ld %ld %ld %ld",
                    sWndPl.rcNormalPosition.left, sWndPl.rcNormalPosition.right,
                    sWndPl.rcNormalPosition.top, sWndPl.rcNormalPosition.bottom);
            AfxGetApp()->WriteProfileString(PLANRT_H_RAZDEL_INI,
                                            PLANRT_H_MNWNDSIZEPLACE_INI, szBuffer);
        };
    };

    SetMainFramePointer__(0);
    CMDIFrameWnd::OnDestroy();
}
