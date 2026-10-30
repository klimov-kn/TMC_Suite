/*
**  TAMIC_soft(R) 1991 v1.00
**
**  run_flag.h
**
**  FILE : a:\include\run_flag.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

/// @file RUN_FLAG.H
/// @brief Флаги этапов расчёта TAMIC (загрузка, топология, ядро, вывод) и номер шага задания.
///
/// Наследие DOS-версии. Включается препроцессором и sfile95; функции в Qt6-версии
/// не определены (вызов `get_tpl_step` есть только в неиспользуемой функции sfile95).
/// @see https://www.tamic.ru/docs/code/shared-headers.html#legacy-api
#ifndef _RUN_FLAG_H
 #define _RUN_FLAG_H



/// @brief Возвращает номер текущего шага задания.
/// @return Номер шага.
/// @note Определения в исходниках пакета нет (объявление TAMIC); вызов есть только в неиспользуемой функции sfile95 `make_S_matrix_name`.
int  get_tpl_step( void );

#endif
