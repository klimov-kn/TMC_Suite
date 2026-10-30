// TmcIdx.h : type of the global node number and of the grid array size (INT64 patch)
//
// win32: TmcIdx = int (32 bit, as before), win64: 64 bit.
// The grid of nX*nY nodes can be bigger than 2^31 nodes in win64.

/// @file TmcIdx.h
/// @brief Тип глобального номера узла сетки `TmcIdx` и чтение/запись номера в 4 байта.
///
/// Часть библиотеки TMCIndan (общий заголовок ядер). Номер узла `iy*nX + ix`
/// на больших сетках превышает 2^31, поэтому все глобальные номера узлов
/// и размеры массивов узлов в ядрах и в TMCIndan имеют тип `TmcIdx`.
/// Формат файлов `.eps` при этом не меняется: номер узла в них хранится
/// в 4 байтах, для чего служат `TmcReadIdx32` и `TmcWriteIdx32`.
/// @see https://www.tamic.ru/docs/code/lib-tmcindan.html#node-index

#ifndef _TMCIDX_H__
#define _TMCIDX_H__

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/// Глобальный номер узла сетки и размер массивов узлов: `ptrdiff_t`
/// (32 бита в сборке win32, 64 бита в сборке win64). Отрицательное значение
/// в вызывающем коде означает «узла нет» (например, `-1` у `CalcnNodeGlobalNorm`).
typedef ptrdiff_t TmcIdx;
/// Наибольшее значение `TmcIdx`. Ядра сравнивают с ним `nX*nY` (с запасом
/// на число значений в узле), чтобы отказаться от сетки, не помещающейся в индекс.
#define TMC_IDX_MAX PTRDIFF_MAX

// node number in .eps files is stored in 4 bytes (file format is not changed)
/// @brief Читает номер узла из файла `.eps`, где он записан 4-байтовым `int`.
///
/// @param[out] p   приёмник номера узла; заполняется и при неудачном чтении
///                 (тогда значение 0)
/// @param[in]  fp  открытый на чтение двоичный файл, позиция — на номере узла
/// @return Число прочитанных элементов `fread`: 1 — успех, 0 — конец файла или ошибка.
/// @warning Номер больше 2^31 - 1 в этот формат не помещается: файлы `.eps`
///          ограничены сетками, номера узлов которых укладываются в `int`.
inline size_t TmcReadIdx32( TmcIdx *p, FILE *fp )
{
	int n = 0;
	size_t r = fread( &n, sizeof(int), 1, fp );
	*p = (TmcIdx)n;
	return r;
}
/// @brief Записывает номер узла в файл `.eps` 4-байтовым `int` (усекая `TmcIdx`).
///
/// @param[in] p   номер узла; должен укладываться в `int`, иначе в файл
///                попадёт усечённое значение
/// @param[in] fp  открытый на запись двоичный файл
/// @return Число записанных элементов `fwrite`: 1 — успех, 0 — ошибка записи.
inline size_t TmcWriteIdx32( const TmcIdx *p, FILE *fp )
{
	int n = (int)(*p);
	return fwrite( &n, sizeof(int), 1, fp );
}

#endif
