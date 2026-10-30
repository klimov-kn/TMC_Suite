/*
**  TAMIC_soft(R) 1991 v1.00
**
** Macros and decloration for ys_proc programm
**
** FILE: a\tmc_norm\ys_proc.h
**
*/

/// @file YS_PROC.H
/// @brief Объявления утилиты TAMIC пересчёта S-матриц в Y-матрицы и вывода таблиц (наследие).
///
/// Часть модуля «Библиотека sfile95». Подключается файлами s_remt.cpp и s_wemt.cpp,
/// но ни одна объявленная здесь функция в сборке Qt6-версии не определена и не
/// вызывается: реализации либо в файлах вне сборки (s_db.cpp, stxtable.cpp,
/// wrtslst.cpp), либо отсутствуют в исходниках пакета.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#legacy

#include <FRQDATA.H>
#include <S_file.h>

#ifndef _YS_PROC_H
  #define _YS_PROC_H

/// Вид текстовой таблицы: действительная и мнимая части (то же значение, что в `proc_s.h`).
#define REAL_IMAG  0
/// Вид текстовой таблицы: модуль и фаза.
#define ARG_PHASE  1
/// Вид текстовой таблицы: КСВН и потери.
#define VSWR_LOSS  2

/// @brief Запоминает имя S-файла. Повторное объявление (см. `proc_s.h`); реализация (s_db.cpp) вне сборки.
/// @param[in] name  имя файла
/// @return 0 — успех; -1 — имя слишком длинное.
int  set_file_name( char *name );
/// @brief Возвращает имя S-файла. Повторное объявление (см. `proc_s.h`); определения нет.
/// @return Имя файла.
char * get_S_file_name( void );

/// @brief Задаёт флаг вывода текстовой таблицы. Повторное объявление; реализация (s_db.cpp) вне сборки.
/// @param[in] nFlag  0 — не выводить
void Set_OutTextFlag( int nFlag );
/// @brief Возвращает флаг вывода текстовой таблицы. Повторное объявление; реализация (s_db.cpp) вне сборки.
/// @return Значение флага.
int  Get_OutTextFlag( void );

/// @brief Главная процедура утилиты пересчёта S -> Y (наследие TAMIC). Определения в исходниках пакета нет.
void ys_proc( void );
/// @brief Функция утилиты S -> Y TAMIC (по имени — пересчёт одной S-матрицы в Y-матрицу). Определения нет.
/// @param[in]  sm     S-матрица `dim*dim`
/// @param[out] ymatr  Y-матрица
/// @param[in]  dim    размерность
/// @return Код завершения.
int _S_to_Y( _complex *sm, float *ymatr, int dim );

/// @brief Пишет S-матрицы всех частот в текстовый файл для других САПР. Реализация (stxtable.cpp) в сборку Qt6-версии не входит.
/// @param[in] lpxS     матрицы по частотам
/// @param[in] frq_out  частоты
/// @param[in] id       описания входов по частотам
/// @return 0 — успех, иначе ошибка.
int  put_S_text_table( _complex   **lpxS,
                                      FREQUENCY  *frq_out,
                                      INPUT_DATA *id );

/// @brief Печатает S-матрицы всех частот в листинг. Реализация (wrtslst.cpp) в сборку Qt6-версии не входит.
/// @param[in] si             индексы сетки
/// @param[in] extrapol_flag  число сеток минус один
/// @param[in] sss            матрицы по частотам
/// @param[in] frq_out        частоты
/// @param[in] id             описания входов по частотам
/// @return 0 — успех, иначе ошибка.
int  write_S_to_listing( S_INDEX    *si,
                                        int               extrapol_flag,
                                        _complex   **sss,
                                        FREQUENCY  *frq_out,
                                        INPUT_DATA *id );


//int                *build_index_table( const _OFFSET_TABLE& ot1, const _OFFSET_TABLE& ot2,
//                                       _OFFSET_TABLE& ot3,
//                                       Uint *lpIndexTable, Uint *lpPointNum );

#endif
