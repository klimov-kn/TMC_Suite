// ucrt_libm_win.cpp — математические функции из ucrtbase.dll (Windows, MinGW).
//
// MFC-версия пакета собрана MSVC и берёт sin, cos, exp, atan2 … из системной
// библиотеки UCRT (ucrtbase.dll). MinGW по умолчанию берёт их из своей
// библиотеки, которая в последнем бите округляет иначе. Формулы те же, но числа
// в .s/.AMP/.FAZ расходились бы с MFC-версией в младших разрядах.
//
// Здесь эти функции перенаправлены в ucrtbase.dll — в ту же реализацию, что у
// MFC-версии. Формулы ядра и порядок вычислений не меняются. Приём проверен в
// TMC_Suite_Mobile (src/kernels/compat/ucrt_libm_probe.cpp, ISSUES.md О-9).
//
// Подключается в каждую программу (интерфейсный источник tmc_platform), только
// при сборке под Windows компилятором MinGW.

/// @file ucrt_libm_win.cpp
/// @brief Перенаправление `sin`, `cos`, `exp`, `pow` … в ucrtbase.dll, чтобы числа совпадали со сборкой компилятором MSVC.
///
/// Часть модуля «Слой совместимости». Только Windows-сборка компилятором MinGW;
/// компилируется в каждую программу (интерфейсный источник `tmc_platform`).
/// @see https://www.tamic.ru/docs/code/compat.html#libm
#if defined(_WIN32) && defined(__MINGW32__)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace {

/// Указатель на функцию UCRT одного аргумента.
typedef double (*TmcFn1)(double);
/// Указатель на функцию UCRT двух аргументов.
typedef double (*TmcFn2)(double, double);

/// @brief Возвращает адрес функции в ucrtbase.dll.
///
/// Библиотека загружается при первом вызове. Если её или функции нет —
/// показывается окно ошибки и процесс завершается с кодом 99.
/// @param[in] name  имя функции (`"sin"` …)
/// @return Адрес функции (никогда не NULL).
FARPROC tmcUcrt(const char* name)
{
    static HMODULE h = LoadLibraryA("ucrtbase.dll");
    FARPROC p = h ? GetProcAddress(h, name) : NULL;
    if (p == NULL) {
        // ucrtbase.dll есть в любой Windows 10/11 (и ставится обновлением на 7/8.1).
        MessageBoxA(NULL, "ucrtbase.dll (Universal C Runtime) not found.\n"
                          "Install the Windows update KB2999226 or the Visual C++ Redistributable.",
                    "TMC Suite", MB_OK | MB_ICONERROR);
        ExitProcess(99);
    }
    return p;
}

} // namespace

/// Определяет `extern "C" double fn(double)`, вызывающую одноимённую функцию UCRT (адрес кэшируется).
#define TMC_UCRT1(fn) \
    extern "C" double fn(double x) \
    { static TmcFn1 p = (TmcFn1)(void*)tmcUcrt(#fn); return p(x); }
/// Определяет `extern "C" double fn(double, double)`, вызывающую одноимённую функцию UCRT.
#define TMC_UCRT2(fn) \
    extern "C" double fn(double x, double y) \
    { static TmcFn2 p = (TmcFn2)(void*)tmcUcrt(#fn); return p(x, y); }

/// Перенаправляемые функции: тригонометрия, гиперболические, экспонента и логарифмы, atan2, pow, fmod.
TMC_UCRT1(sin)  TMC_UCRT1(cos)  TMC_UCRT1(tan)
TMC_UCRT1(asin) TMC_UCRT1(acos) TMC_UCRT1(atan)
TMC_UCRT1(sinh) TMC_UCRT1(cosh) TMC_UCRT1(tanh)
TMC_UCRT1(exp)  TMC_UCRT1(log)  TMC_UCRT1(log10)
TMC_UCRT2(atan2) TMC_UCRT2(pow) TMC_UCRT2(fmod)

// g++ объединяет sin(x) и cos(x) одного аргумента в один вызов sincos:
// направляем и его в те же функции UCRT.
/// @brief Вычисляет синус и косинус через функции UCRT (замена объединённого вызова g++).
/// @param[in]  x  аргумент, радианы
/// @param[out] s  sin(x)
/// @param[out] c  cos(x)
extern "C" void sincos(double x, double* s, double* c)
{
    *s = sin(x);
    *c = cos(x);
}

#endif
