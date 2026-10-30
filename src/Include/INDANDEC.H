/*
**  TAMIC_soft(R) 1990 v1.00
**
**  indandec.h
**
**  FILE : a:\include\indandec.c
**
**  LIB  :
**
**  OVERLAY:
**
*/

/// @file INDANDEC.H
/// @brief Структуры описания модулей схемы TAMIC: входы, волноводы, шлейфы, многоугольники.
///
/// Включается sfile95 (read_s.cpp, write_s.cpp, wrrd_s.cpp) ради цепочки заголовков
/// (подключает run.h). Сами структуры — наследие TAMIC; в коде Qt6-версии не используются.
/// @see https://www.tamic.ru/docs/code/shared-headers.html#legacy-api
#ifndef _INDANDEC_H
 #define _INDANDEC_H


#ifndef _MODUL_FL
  #define _MODUL_FL

#endif

#ifndef _GP_POLYGON_DEF
 #define _GP_POLYGON_DEF

#endif


#ifndef _INP_ADRESS
 #include <RUN.H>
#endif

#endif
