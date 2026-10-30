/*
**  TAMIC_soft(R) 1990,1991 v1.20
**
** Macros and decloration for run  programms.
**
** FILE: a:\include\run.h
*/

/// @file RUN.H
/// @brief Расчётная часть TAMIC: Y-матрица, входы, модули схемы, подпрограммы МКЭ и сборки.
///
/// Включается из indandec.h (и далее sfile95). Все структуры и функции — наследие
/// TAMIC (расчёт методом конечных элементов и сборка схемы по связям); в коде
/// Qt6-версии не используются, функции не определены. Ядра пакета считают
/// Rt-сеткой во временной области (библиотека TMCIndan и ядра H/X).
/// @see https://www.tamic.ru/docs/code/shared-headers.html#legacy-api
#ifndef _RUN_H
 #define _RUN_H

 /// Тип элемента Y-матрицы, если не задан в typedef.h (там он уже задан как `_real`).
 #ifndef _ELEM_VAL
  #ifdef _ELEM_COMPLEX
   #define _ELEM_VAL _complex
  #else
   #define _ELEM_VAL _real
  #endif
 #endif

#ifndef _INP_ADRESS
 /// Признак того, что `INP_ADRESS` уже определён (защита от повторного определения).
 #define _INP_ADRESS

 /// @brief Адрес входа: номер блока и номер входа блока. В коде Qt6-версии не используется.
 typedef struct
         {
          /// Номер блока.
          short block;
          /// Номер входа блока.
          short input;
          } INP_ADRESS;

#endif

#ifndef _Y_MATRIX_DEF
 /// Признак того, что `INPUT` и `Y_MATRIX` уже определены (защита от повторного определения).
 #define _Y_MATRIX_DEF

/// @brief Описание входов Y-матрицы: число, моды, нормировка, адреса. В коде Qt6-версии не используется.
typedef struct
        {
         /// Число входов.
         short        num;                                       /* number of input*/
         /// Число мод на каждом входе.
         short        norm_ptr[MAX_INPUT_NUM];                   /* number of mode in input*/
         /// Коэффициенты нормировки.
         _real      norm_array[MAX_INPUT_NUM][MAX_INPUT_NUM];  /* norm. coeff.*/
         /// Нормировка входа: +1 — нет, -1 — да.
         short        flag[MAX_INPUT_NUM];                       /* +1 - no norm., -1 yes norm.*/
         /// Адреса входов.
         INP_ADRESS ia[MAX_INPUT_NUM];                         /* input adress*/
         } INPUT;

/// @brief Y-матрица модуля или схемы с описанием входов. В коде Qt6-версии не используется.
typedef struct
         {
          /// Буфер элементов Y-матрицы.
          _ELEM_VAL *matr;               /* pointer to Y-matrix buffer*/
//          float       _far *matr;               /* pointer to Y-matrix buffer*/
          /// Нормировка: 0 — недоступна, 1 — доступна, но не выполнена, 2 — выполнена.
          short             norm_flag;          /* norm_flag: 0 - norm. not available*/
                                              /*            1 - available but not norm*/
                                              /*            2 - norm*/
          /// Описание входов.
          INPUT           inp;                /* input data*/
          } Y_MATRIX;
#endif
#include <LINKLIST.H>
/// @brief Указатели на массивы триангуляции модуля МКЭ. В коде Qt6-версии не используется.
struct point_trn_adress
         {
          /// Координаты узлов x и y.
          float *skx,   *sky;
          /// Типы границ, номера узлов, наибольшее число узлов.
          int   *ktpgr, *nomuz, maxnuz;
          /// Число треугольников и номера их вершин.
          int   koltr,       *nadr1, *nadr2, *nadr3;
          /// eps и mu треугольников.
          float *epstr, *rmutr;
          };
/// @var point_trn_adress::sky
/// Координаты y узлов.
/// @var point_trn_adress::nomuz
/// Номера узлов.
/// @var point_trn_adress::maxnuz
/// Наибольшее число узлов.
/// @var point_trn_adress::nadr1
/// Номера первых вершин треугольников.
/// @var point_trn_adress::nadr2
/// Номера вторых вершин треугольников.
/// @var point_trn_adress::nadr3
/// Номера третьих вершин треугольников.
/// @var point_trn_adress::rmutr
/// mu треугольников.

/// @brief Константы расчёта на одной частоте. В коде Qt6-версии не используется.
typedef struct
          { _real omga;       /* 2.0*PI*F(i)*/
            /// Число pi. (Поле `omga` — круговая частота 2*pi*F, рад/с.)
            _real pi;         /* PI = 3.14....*/
            /// Магнитная постоянная mu0, Гн/м.
            _real um0;        /* mu0 constant*/
            /// Электрическая постоянная eps0, Ф/м.
            _real eps0;       /* eps0 constant*/
           }CONST_BLOCK;
/// @var CONST_BLOCK::omga
/// Круговая частота 2*pi*F, рад/с.

#ifndef _MODUL_GP
 /// Признак того, что `struct modul_GP` уже определена (защита от повторного определения).
 #define _MODUL_GP

/// @brief Элемент списка геометрических примитивов (сущностей) модуля. В коде Qt6-версии не используется.
struct modul_GP
	{
         /// Код сущности.
         int                   code;        /* entity's code*/
         /// Номер блока.
         int                   num;         /* block number*/
         /// Указатель на сущность.
         void            *gp;          /* pointer to this entity*/
         /// Следующий элемент.
         struct modul_GP *next;        /* pointer to next entity*/
	 };
/// Синоним `struct modul_GP`.
typedef struct modul_GP MODUL_GP;
#endif


/// @brief Рассчитывает Y-матрицу модуля.
/// @param[in] modul_num  номер модуля
/// @param[out] matr  Y-матрица
/// @param[in] ptr  список сущностей модуля
/// @return 0 — успех.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int  modul( int modul_num, Y_MATRIX *matr, MODUL_GP * ptr );

#endif
