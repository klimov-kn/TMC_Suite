/*
 *  TAMIC_soft(R) 1990,1991,1992 vR1.02
 *
 *  put_S_table - print out multimode scattering matrix to listing
 *                in the form of table
 *
 *  Usage -  put_S_table( lpxS, int inpnb, int _far *modnb );
 *
 *  Arguments - lpxS - scattering matrix
 *             inpnb - number of inputs of multiport
 *             modnb - array of number of modes at all inputs
 *
 *  Remark - scattering matrix is assumed to be stored
 *           rowwise in general storage mode
 *
 *  Reqd. routines - cabsv, cphase ( in cmplx library )
 *
 *  FILE : \source\tamic\post_pro\s_table.c
 *
 *  LIB  : ooutpXY.lib
 *
 *  OVERLAY: YES
 */

/// @file S_TABLE.CPP
/// @brief Печать многомодовой S-матрицы текущей частоты в листинг таблицей (`put_S_table`).
///
/// Часть модуля «Библиотека sfile95». Наследие TAMIC: в программах Qt6-версии
/// не вызывается, функции листинга в сборке не определены. Функция объявлена
/// и описана в `proc_s.h`.
/// @see https://www.tamic.ru/docs/code/lib-sfile95.html#legacy

#include <string.h>
#include <stdio.h>
#include <math.h>

#include <Typedef.h>
#include <FRQDATA.H>
#include <LISTING.H>
#include <COMPLEX1.H>
#include <COMPFILE.H>
#include <MAINWNDW.H>


/// Единица для сравнения модулей.
#define ONE       1.0f
/// Ноль для сравнения модулей.
#define ZERO      0.0f
/// Порог: модуль в [1, GEN_THR) считается равным 1 (ошибка округления).
#define GEN_THR   1.000001f

/// Формат заголовка таблицы (частота, Гц).
static char  title[] = "\n      S-matrix table in form \"magnitude-phase\" at frequency: %#10.4g Hz.";
/// @name Строки рамки таблицы
/// Строки рамки и шапки таблицы (псевдографика кодовой страницы DOS).
///@{
/** Строка рамки или шапки таблицы. */
static char  fmt1[] = "     ЙНННННННННННННННННННСНННННННННННННННННННННННННННННННСННННННННННННННННН»";
static char  fmt2[] = "     є     Number of     і       S-matrix elements       і   L or VSWR     є";
static char  fmt3[] = "     ЗДДДДДДДДДВДДДДДДДДДЕДДДДДДДДДДДДДДДВДДДДДДДДДДДДДДДЕДДДДДДДДДДДДДДДДД¶";
static char  fmt4[] = "     є  inputs і  modes  і 'S'-magnitude і'S'-phase(rad.)і   L in  dB      є";
static char  fmt5[] = "     МННННСННННШННННСННННШНННННННННННННННШНННННННННННННННШННННННННННННННННН№";
///@}
/// Формат строки таблицы: номера входов и мод, модуль, фаза (рад), КСВН или потери (дБ).
static char  fmt6[] = "     є %2d і %2d і %2d і %2d і  %#12.6g і %#13.6g і %5s%-11.4gє";
/// Разделитель строк таблицы.
static char  fmt7[] = "     ЗДДДДЕДДДДЕДДДДЕДДДДЕДДДДДДДДДДДДДДДЕДДДДДДДДДДДДДДДЕДДДДДДДДДДДДДДДДД¶";
/// Нижняя рамка таблицы.
static char  fmt8[] = "     ИННННПННННПННННПННННПНННННННННННННННПНННННННННННННННПНННННННННННННННННј";
/// Пять строк шапки таблицы по порядку.
static char * hood[] = { fmt1, fmt2, fmt3, fmt4, fmt5 };
/// Текст строки состояния (через `put_trace`).
static char trace[] = "Prt. S-matr.";

