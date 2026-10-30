/*
**  TAMIC_soft(R) 1990,1991 v1.00
**
**  data_const_list.h
**
**  FILE : a:\include\datacnst.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

/// @file DATACNST.H
/// @brief Общие константы задания TAMIC: шаг, высота, точность, eps и mu среды.
///
/// Структура и функции — наследие TAMIC; в коде Qt6-версии структура не используется,
/// а функции не определены (вызов `get_data_const_delta` есть только в неиспользуемой
/// функции sfile95). Параметры сетки ядер хранит `CTmcRTH_IndanParam` (TMCIndan).
/// @see https://www.tamic.ru/docs/code/shared-headers.html#legacy-api
#ifndef _DATACNST_H
 #define _DATACNST_H


/// @brief Общие константы шага задания TAMIC. В коде Qt6-версии не используется.
struct data_const_list
         {
          /// Номер шага задания.
          int num_step;
          /// Шаг сетки, единицы длины задания.
          _real delta;
          /// Относительные eps и mu среды.
          _real eps, mu;
          /// Высота (ширина) структуры, единицы длины задания.
          _real height;
          /// Требуемая точность расчёта.
          _real acuracy;
          /// @var data_const_list::mu
          /// Относительная mu среды.
          };
/// @brief Возвращает шаг сетки.
/// @param[out] delta  шаг (может быть NULL)
/// @return Шаг сетки.
/// @note Определения в исходниках пакета нет (объявление TAMIC); вызов есть только в неиспользуемой функции sfile95 `make_S_matrix_name`.
_real get_data_const_delta( _real *delta );

#endif
