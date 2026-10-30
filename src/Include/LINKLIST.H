/*
**  TAMIC_soft(R) 1990 v1.00
**
**  linklist.h
**
**  FILE : a:\include\linklist.h
**
**  LIB  :
**
**  OVERLAY:
**
*/

/// @file LINKLIST.H
/// @brief Список связей между модулями (блоками) схемы TAMIC.
///
/// Наследие TAMIC: в коде Qt6-версии структуры не используются, функции не определены
/// (одноимённая `SetLink` есть только как метод в TMCIndan). Включается из run.h.
/// @see https://www.tamic.ru/docs/code/shared-headers.html#legacy-api
#ifndef _LINKLIST_H
 #define _LINKLIST_H

/// @brief Одна связь: вход одного модуля соединён с входом другого.
typedef struct
	 {
          /// Номер связи.
          int num;     /* link number*/
          /// Номер первого модуля.
          int m1;      /* first modul number*/
          /// Номер входа (окна) первого модуля.
          int in1;     /* window's first modul number*/
          /// Номер второго модуля.
          int m2;      /* second number*/
          /// Номер входа (окна) второго модуля.
          int in2;     /* window's second modul number*/
	  } LINK;

/// @brief Элемент односвязного списка связей.
struct tagLinkList
	 {
	  /// Связь.
	  LINK                     Link;
	  /// Следующий элемент; NULL — конец списка.
	  struct tagLinkList *lpNext;
	  };

/// Синоним `struct tagLinkList`.
typedef struct tagLinkList LINK_LIST;
/// @brief Добавляет связь в список.
/// @param[in] lplnkBuffer  связь
/// @return 0 — успех.
/// @note Определения в исходниках пакета нет (объявление TAMIC); в Qt6-версии не вызывается.
int       SetLink( LINK *lplnkBuffer );
#endif
