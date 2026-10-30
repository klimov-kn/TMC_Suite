/*
**  TAMIC_soft(R) 1992 vR1.02
**
**  Matrix functions declarations
**
**  FILE : mic\matrix.h
**
*/

/// @file MATRIX.H
/// @brief Матричные подпрограммы TAMIC: обращение, разложение, собственные значения, переходы Z/Y/S.
///
/// Включается sfile95 (read_s1.cpp, sadd.cpp). В исходниках пакета определена только
/// `Check_S_Unitary` (read_s1.cpp, вне сборки); остальные объявления — наследие
/// TAMIC без определений. Параметр `nStorage_flag` в TAMIC задаёт способ хранения
/// матрицы (полная или треугольная).
/// @see https://www.tamic.ru/docs/code/shared-headers.html#legacy-api
#ifndef _MATRIX_H
 #define _MATRIX_H

/// @brief Проверяет унитарность матрицы рассеяния (для схемы без потерь S^H*S = I).
/// @param[in] lpxS  матрица S
/// @param[in] nDim  порядок
/// @param[out] rMax  наибольшее отклонение
/// @return Код результата проверки.
/// @note Определена в sfile95/read_s1.cpp, который в сборку Qt6-версии не входит.
int Check_S_Unitary( _complex *lpxS, int nDim, _real *rMax );


#endif

