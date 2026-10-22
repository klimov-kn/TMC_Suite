// dllsafety.h — безопасный поиск DLL, загружаемых во время работы (Т-33).
//
// По умолчанию LoadLibrary ищет и в ТЕКУЩЕЙ папке, а ею у ядер становится папка
// задания: подложенная туда DLL с именем системной загрузилась бы в программу.
// После SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS) поиск идёт только
// в папке программы и в System32. На библиотеки, которые программа импортирует при
// запуске (Qt6*.dll), это не влияет: они уже загружены до main.

/// @file dllsafety.h
/// @brief Ограничение поиска DLL, загружаемых во время работы, папкой программы и System32.
///
/// Часть модуля «Общий слой интерфейса» (только заголовок). Вне Windows функция пустая.
/// @see https://www.tamic.ru/docs/code/gui-common.html#dll-safety
#pragma once

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

/// @brief Мелкие средства надёжности интерфейса.
namespace tmcui {

/// @brief Вызывает `SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS)`: текущая папка исключается из поиска DLL.
/// @note Вызывать в начале `main`, до первой динамической загрузки (например, OpenCL.dll).
inline void secureDllSearch()
{
    ::SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
}

} // namespace tmcui
#else
namespace tmcui {
/// @brief Пустая заглушка вне Windows.
inline void secureDllSearch() {}
} // namespace tmcui
#endif
