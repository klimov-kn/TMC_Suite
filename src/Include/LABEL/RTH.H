/*
**  TAMIC_soft(R) 1990 v1.00
**
** Declarations of labels for E-programms
**
** FILE: a\include\label_e.h
**
*/

/// @file RTH.H
/// @brief Строки-метки программы Rt-H для заголовка файла S-матрицы и окон TAMIC.
///
/// Общий заголовок наследия TAMIC (каталог `include/label`). В Qt6-версии
/// подключается библиотекой SFILE95 (`write_s.cpp`): `MAIN_LABEL` и `TWP_LABEL`
/// записываются в заголовок файла `.s`. Строки — данные формата, их текст
/// не переводится и не меняется.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#limits

#ifndef _LABEL_E_H
  #define _LABEL_E_H

/// Строка авторских прав TAMIC; используется библиотекой SFILE95.
#define COPYRIGHT_TITLE  "Copyright  (C) TAMIC-soft.   All rights reserved."

/// Метка программы Rt-H, записываемая в заголовок файла `.s`.
#define TWP_LABEL        "TAMIC_soft group. TAMIC Rt-H Planar Analyzer. 2000"

/// Ветка `_WINDOWS` — метки старой Windows-программы E-plane; иначе — метки Rt-H.
/// `MAIN_LABEL` записывается в заголовок файла `.s`, `TITLE_STR` используется SFILE95,
/// остальные строки в Qt6-версии не используются.
#ifdef _WINDOWS
   #define TITLE_STR "TAMIC-soft (R) E-plane.             Version 2.10W."
   #define MAIN_LABEL "TAMIC-E V 1.10W"
   #define LST_TITLE "TAMIC_soft(R) E-plane waveguide analyzer. Version 2.10W 1990,1991,1992"
 #else
   #define TITLE_STR "TAMIC-soft (R) Rt-H Planar Analyzer.             Version 2.00."
   /// Метка программы Rt-H, записываемая в заголовок файла `.s` (`write_s.cpp`).
   #define MAIN_LABEL "TAMIC-RTH V 2.00"
   /// Заголовок листинга программы Rt-H (в Qt6-версии не используется).
   #define LST_TITLE "TAMIC_soft(R) Rt-H Planar Analyzer. Version 2.00 2000"
#endif


#endif
