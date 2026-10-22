// Заглушка <afxdisp.h> для сборки под Linux: MFC здесь нет.
// Кроме строк и win-типов коду документа нужен каркас CDocument и
// COLORREF/LOGFONT — они в tmc_mfc_doc.h.
/// @file afxdisp.h
/// @brief Заглушка заголовка MFC `<afxdisp.h>`: подключает каркас документа `tmc_mfc_doc.h`.
///
/// Часть модуля «Слой совместимости». OLE-автоматизация MFC не эмулируется;
/// заголовок нужен только для того, чтобы `stdafx.h` Windows-версии компилировался.
/// @see https://www.tamic.ru/docs/code/compat.html#headers
#pragma once
#include "tmc_mfc_doc.h"
