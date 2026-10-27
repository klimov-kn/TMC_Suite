/// @file baked.cpp
/// @brief Паспорт образцов: числа, посчитанные ядром при параметрах по умолчанию.
///
/// Часть модуля «Ядро TPL Designer», каталог образцов. Файл порождён скриптом
/// bake_samples (команда `cpp`) по результатам прогонов ядер; его
/// комментарии заданы в шаблоне скрипта (`write_cpp`).
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#include "sample.h"

#include "lang.h"

// Числа, посчитанные ядром для образцов с параметрами по умолчанию.
// Файл СГЕНЕРИРОВАН bake_samples cpp — руками не править:
// пересчитайте образцы (sample_gen + bake_samples) и соберите заново.

namespace tpl::samples {

namespace {

// Тексты паспорта зависят от языка (lang.h): таблица строится функцией, а хранится
// своя на каждый язык — переключение языка не «замораживает» надписи, а указатели
// из baked() остаются живыми.
/// @brief Строит таблицу паспортов на текущем языке.
/// @return Записи `Baked`, упорядоченные по `Sample::id`.
QVector<Baked> makeTable()
{
    return {
        {QStringLiteral("ant_horn"), QStringLiteral("2026-10-26"), tpl::tr("сетка 203x168, шаг 1.42875 мм, TIME 20 нс, шагов 1", "grid 203x168, step 1.42875 mm, TIME 20 ns, steps 1"), {
            {tpl::tr("10 ГГц", "10 GHz"), tpl::tr("|S11| -31.5 дБ", "|S11| -31.5 dB")},
        }},
        {QStringLiteral("ant_lens_horn"), QStringLiteral("2026-10-26"), tpl::tr("сетка 209x189, шаг 1.42875 мм, TIME 20 нс, шагов 2", "grid 209x189, step 1.42875 mm, TIME 20 ns, steps 2"), {
            {tpl::tr("10 ГГц (без линзы)", "10 GHz (without lens)"), tpl::tr("|S11| -28.9 дБ", "|S11| -28.9 dB")},
            {tpl::tr("10 ГГц (с линзой)", "10 GHz (with lens)"), tpl::tr("|S11| -14.7 дБ", "|S11| -14.7 dB")},
        }},
        {QStringLiteral("ant_open_wg"), QStringLiteral("2026-10-26"), tpl::tr("сетка 147x168, шаг 1.42875 мм, TIME 20 нс, шагов 1", "grid 147x168, step 1.42875 mm, TIME 20 ns, steps 1"), {
            {tpl::tr("10 ГГц", "10 GHz"), tpl::tr("|S11| -24.4 дБ", "|S11| -24.4 dB")},
        }},
        {QStringLiteral("ant_reflector"), QStringLiteral("2026-10-26"), tpl::tr("сетка 210x294, шаг 1.42875 мм, TIME 25 нс, шагов 1", "grid 210x294, step 1.42875 mm, TIME 25 ns, steps 1"), {
            {tpl::tr("10 ГГц", "10 GHz"), tpl::tr("|S11| -7.6 дБ", "|S11| -7.6 dB")},
        }},
        {QStringLiteral("les_horn_demo"), QStringLiteral("2026-10-27"), tpl::tr("сетка 240x200, шаг 1 мм, TIME 3 нс, шагов 1", "grid 240x200, step 1 mm, TIME 3 ns, steps 1"), {
            {tpl::tr("10 ГГц", "10 GHz"), tpl::tr("|S11| -22.3 дБ", "|S11| -22.3 dB")},
        }},
        {QStringLiteral("les_lr1_horn"), QStringLiteral("2026-10-27"), tpl::tr("сетка 240x200, шаг 1 мм, TIME 60 нс, шагов 1", "grid 240x200, step 1 mm, TIME 60 ns, steps 1"), {
            {tpl::tr("6 ГГц", "6 GHz"), tpl::tr("|S11| -30.4 дБ", "|S11| -30.4 dB")},
        }},
        {QStringLiteral("les_lr1_wg"), QStringLiteral("2026-10-27"), tpl::tr("сетка 155x78, шаг 1 мм, TIME 115 нс, шагов 17", "grid 155x78, step 1 mm, TIME 115 ns, steps 17"), {
            {tpl::tr("3.3 ГГц", "3.3 GHz"), tpl::tr("|S11| -32.4 дБ, |S21| -0.0 дБ", "|S11| -32.4 dB, |S21| -0.0 dB")},
            {tpl::tr("4.1 ГГц", "4.1 GHz"), tpl::tr("|S11| -7.7 дБ, |S21| -0.8 дБ", "|S11| -7.7 dB, |S21| -0.8 dB")},
            {tpl::tr("4.9 ГГц", "4.9 GHz"), tpl::tr("|S11| -8.9 дБ, |S21| -0.6 дБ", "|S11| -8.9 dB, |S21| -0.6 dB")},
        }},
        {QStringLiteral("les_lr2_horn"), QStringLiteral("2026-10-27"), tpl::tr("сетка 240x170, шаг 1 мм, TIME 60 нс, шагов 1", "grid 240x170, step 1 mm, TIME 60 ns, steps 1"), {
            {tpl::tr("8 ГГц", "8 GHz"), tpl::tr("|S11| -33.4 дБ", "|S11| -33.4 dB")},
        }},
        {QStringLiteral("les_lr2_plasma"), QStringLiteral("2026-10-27"), tpl::tr("сетка 240x80, шаг 1 мм, TIME 115 нс, шагов 31", "grid 240x80, step 1 mm, TIME 115 ns, steps 31"), {
            {tpl::tr("3 ГГц", "3 GHz"), tpl::tr("|S11| -0.0 дБ, |S21| -30.7 дБ", "|S11| -0.0 dB, |S21| -30.7 dB")},
            {tpl::tr("4.5 ГГц", "4.5 GHz"), tpl::tr("|S11| -0.1 дБ, |S21| -16.3 дБ", "|S11| -0.1 dB, |S21| -16.3 dB")},
            {tpl::tr("6 ГГц", "6 GHz"), tpl::tr("|S11| -5.1 дБ, |S21| -1.6 дБ", "|S11| -5.1 dB, |S21| -1.6 dB")},
        }},
        {QStringLiteral("med_eps_file"), QStringLiteral("2026-10-27"), tpl::tr("сетка 299x262, шаг 0.8 мм, TIME 20 нс, шагов 1", "grid 299x262, step 0.8 mm, TIME 20 ns, steps 1"), {
            {tpl::tr("10 ГГц", "10 GHz"), tpl::tr("|S11| -32.5 дБ", "|S11| -32.5 dB")},
        }},
        {QStringLiteral("med_grad"), QStringLiteral("2026-10-27"), tpl::tr("сетка 180x36, шаг 1 мм, TIME 30 нс, шагов 32", "grid 180x36, step 1 mm, TIME 30 ns, steps 32"), {
            {tpl::tr("2.5 ГГц (резкая граница)", "2.5 GHz (sharp boundary)"), tpl::tr("|S11| -7.2 дБ, |S21| -1.0 дБ", "|S11| -7.2 dB, |S21| -1.0 dB")},
            {tpl::tr("6.5 ГГц (резкая граница)", "6.5 GHz (sharp boundary)"), tpl::tr("|S11| -8.2 дБ, |S21| -1.1 дБ", "|S11| -8.2 dB, |S21| -1.1 dB")},
            {tpl::tr("9 ГГц (резкая граница)", "9 GHz (sharp boundary)"), tpl::tr("|S11| -8.3 дБ, |S21| -1.1 дБ", "|S11| -8.3 dB, |S21| -1.1 dB")},
            {tpl::tr("10 ГГц (резкая граница)", "10 GHz (sharp boundary)"), tpl::tr("|S11| -7.6 дБ, |S21| -1.2 дБ", "|S11| -7.6 dB, |S21| -1.2 dB")},
            {tpl::tr("2.5 ГГц (плавный профиль)", "2.5 GHz (smooth profile)"), tpl::tr("|S11| -16.0 дБ, |S21| -0.3 дБ", "|S11| -16.0 dB, |S21| -0.3 dB")},
            {tpl::tr("6.5 ГГц (плавный профиль)", "6.5 GHz (smooth profile)"), tpl::tr("|S11| -24.3 дБ, |S21| -0.2 дБ", "|S11| -24.3 dB, |S21| -0.2 dB")},
            {tpl::tr("8.5 ГГц (плавный профиль)", "8.5 GHz (smooth profile)"), tpl::tr("|S11| -30.7 дБ, |S21| -0.3 дБ", "|S11| -30.7 dB, |S21| -0.3 dB")},
            {tpl::tr("10 ГГц (плавный профиль)", "10 GHz (smooth profile)"), tpl::tr("|S11| -23.0 дБ, |S21| -0.3 дБ", "|S11| -23.0 dB, |S21| -0.3 dB")},
        }},
        {QStringLiteral("med_luneburg"), QStringLiteral("2026-10-27"), tpl::tr("сетка 300x270, шаг 1 мм, TIME 25 нс, шагов 1", "grid 300x270, step 1 mm, TIME 25 ns, steps 1"), {
            {tpl::tr("10 ГГц", "10 GHz"), tpl::tr("|S11| -33.3 дБ", "|S11| -33.3 dB")},
        }},
        {QStringLiteral("med_slab"), QStringLiteral("2026-10-27"), tpl::tr("сетка 517x27, шаг 0.6 мм, TIME 20 нс, шагов 33", "grid 517x27, step 0.6 mm, TIME 20 ns, steps 33"), {
            {tpl::tr("2 ГГц", "2 GHz"), tpl::tr("|S11| -8.0 дБ, |S21| -0.9 дБ", "|S11| -8.0 dB, |S21| -0.9 dB")},
            {tpl::tr("7 ГГц", "7 GHz"), tpl::tr("|S11| -14.9 дБ, |S21| -0.3 дБ", "|S11| -14.9 dB, |S21| -0.3 dB")},
            {tpl::tr("7.313 ГГц", "7.313 GHz"), tpl::tr("|S11| -33.7 дБ, |S21| -0.2 дБ", "|S11| -33.7 dB, |S21| -0.2 dB")},
            {tpl::tr("12 ГГц", "12 GHz"), tpl::tr("|S11| -4.8 дБ, |S21| -1.9 дБ", "|S11| -4.8 dB, |S21| -1.9 dB")},
        }},
        {QStringLiteral("ms_bend"), QStringLiteral("2026-10-26"), tpl::tr("сетка 111x111, шаг 0.56284 мм, TIME 15 нс, шагов 21", "grid 111x111, step 0.56284 mm, TIME 15 ns, steps 21"), {
            {tpl::tr("1.8 ГГц", "1.8 GHz"), tpl::tr("|S11| -28.6 дБ, |S21| -0.3 дБ", "|S11| -28.6 dB, |S21| -0.3 dB")},
            {tpl::tr("3.3 ГГц", "3.3 GHz"), tpl::tr("|S11| -19.6 дБ, |S21| -0.3 дБ", "|S11| -19.6 dB, |S21| -0.3 dB")},
            {tpl::tr("4.2 ГГц", "4.2 GHz"), tpl::tr("|S11| -34.6 дБ, |S21| -0.3 дБ", "|S11| -34.6 dB, |S21| -0.3 dB")},
            {tpl::tr("4.8 ГГц", "4.8 GHz"), tpl::tr("|S11| -16.0 дБ, |S21| -0.4 дБ", "|S11| -16.0 dB, |S21| -0.4 dB")},
        }},
        {QStringLiteral("ms_branchline"), QStringLiteral("2026-10-26"), tpl::tr("сетка 128x47, шаг 0.6 мм, TIME 20 нс, шагов 21", "grid 128x47, step 0.6 mm, TIME 20 ns, steps 21"), {
            {tpl::tr("1.8 ГГц", "1.8 GHz"), tpl::tr("|S11| -5.9 дБ, |S21| -8.1 дБ, |S31| -4.7 дБ, |S41| -8.4 дБ", "|S11| -5.9 dB, |S21| -8.1 dB, |S31| -4.7 dB, |S41| -8.4 dB")},
            {tpl::tr("3 ГГц", "3 GHz"), tpl::tr("|S11| -21.4 дБ, |S21| -3.3 дБ, |S31| -3.6 дБ, |S41| -20.7 дБ", "|S11| -21.4 dB, |S21| -3.3 dB, |S31| -3.6 dB, |S41| -20.7 dB")},
            {tpl::tr("4.2 ГГц", "4.2 GHz"), tpl::tr("|S11| -7.9 дБ, |S21| -5.6 дБ, |S31| -5.5 дБ, |S41| -7.6 дБ", "|S11| -7.9 dB, |S21| -5.6 dB, |S31| -5.5 dB, |S41| -7.6 dB")},
        }},
        {QStringLiteral("ms_bsf"), QStringLiteral("2026-10-27"), tpl::tr("сетка 302x56, шаг 0.377427 мм, TIME 25 нс, шагов 41", "grid 302x56, step 0.377427 mm, TIME 25 ns, steps 41"), {
            {tpl::tr("1.5 ГГц", "1.5 GHz"), tpl::tr("|S11| -10.4 дБ, |S21| -0.8 дБ", "|S11| -10.4 dB, |S21| -0.8 dB")},
            {tpl::tr("3 ГГц", "3 GHz"), tpl::tr("|S11| -0.5 дБ, |S21| -59.2 дБ", "|S11| -0.5 dB, |S21| -59.2 dB")},
            {tpl::tr("4.275 ГГц", "4.275 GHz"), tpl::tr("|S11| -33.6 дБ, |S21| -0.3 дБ", "|S11| -33.6 dB, |S21| -0.3 dB")},
            {tpl::tr("4.5 ГГц", "4.5 GHz"), tpl::tr("|S11| -8.0 дБ, |S21| -1.0 дБ", "|S11| -8.0 dB, |S21| -1.0 dB")},
        }},
        {QStringLiteral("ms_line"), QStringLiteral("2026-10-26"), tpl::tr("сетка 195x17, шаг 0.56284 мм, TIME 15 нс, шагов 21", "grid 195x17, step 0.56284 mm, TIME 15 ns, steps 21"), {
            {tpl::tr("1.8 ГГц", "1.8 GHz"), tpl::tr("|S11| -19.0 дБ, |S21| -0.4 дБ", "|S11| -19.0 dB, |S21| -0.4 dB")},
            {tpl::tr("3.3 ГГц", "3.3 GHz"), tpl::tr("|S11| -24.6 дБ, |S21| -0.4 дБ", "|S11| -24.6 dB, |S21| -0.4 dB")},
            {tpl::tr("4.2 ГГц", "4.2 GHz"), tpl::tr("|S11| -27.7 дБ, |S21| -0.3 дБ", "|S11| -27.7 dB, |S21| -0.3 dB")},
            {tpl::tr("4.8 ГГц", "4.8 GHz"), tpl::tr("|S11| -26.6 дБ, |S21| -0.4 дБ", "|S11| -26.6 dB, |S21| -0.4 dB")},
        }},
        {QStringLiteral("ms_load"), QStringLiteral("2026-10-27"), tpl::tr("сетка 195x17, шаг 0.56284 мм, TIME 20 нс, шагов 21", "grid 195x17, step 0.56284 mm, TIME 20 ns, steps 21"), {
            {tpl::tr("1.5 ГГц", "1.5 GHz"), tpl::tr("|S11| -1.7 дБ", "|S11| -1.7 dB")},
            {tpl::tr("3 ГГц", "3 GHz"), tpl::tr("|S11| -4.3 дБ", "|S11| -4.3 dB")},
            {tpl::tr("4.05 ГГц", "4.05 GHz"), tpl::tr("|S11| -8.2 дБ", "|S11| -8.2 dB")},
            {tpl::tr("4.5 ГГц", "4.5 GHz"), tpl::tr("|S11| -6.3 дБ", "|S11| -6.3 dB")},
        }},
        {QStringLiteral("ms_lpf"), QStringLiteral("2026-10-26"), tpl::tr("сетка 281x42, шаг 0.4 мм, TIME 20 нс, шагов 37", "grid 281x42, step 0.4 mm, TIME 20 ns, steps 37"), {
            {tpl::tr("0.4 ГГц", "0.4 GHz"), tpl::tr("|S11| -10.9 дБ, |S21| -0.7 дБ", "|S11| -10.9 dB, |S21| -0.7 dB")},
            {tpl::tr("1.5 ГГц", "1.5 GHz"), tpl::tr("|S11| -28.2 дБ, |S21| -0.3 дБ", "|S11| -28.2 dB, |S21| -0.3 dB")},
            {tpl::tr("2.2 ГГц", "2.2 GHz"), tpl::tr("|S11| -2.8 дБ, |S21| -4.4 дБ", "|S11| -2.8 dB, |S21| -4.4 dB")},
            {tpl::tr("4 ГГц", "4 GHz"), tpl::tr("|S11| -0.4 дБ, |S21| -19.0 дБ", "|S11| -0.4 dB, |S21| -19.0 dB")},
        }},
        {QStringLiteral("ms_openend"), QStringLiteral("2026-10-27"), tpl::tr("сетка 100x17, шаг 0.56284 мм, TIME 15 нс, шагов 21", "grid 100x17, step 0.56284 mm, TIME 15 ns, steps 21"), {
            {tpl::tr("1.5 ГГц", "1.5 GHz"), tpl::tr("|S11| -0.1 дБ", "|S11| -0.1 dB")},
            {tpl::tr("2.1 ГГц", "2.1 GHz"), tpl::tr("|S11| -0.8 дБ", "|S11| -0.8 dB")},
            {tpl::tr("3 ГГц", "3 GHz"), tpl::tr("|S11| -0.3 дБ", "|S11| -0.3 dB")},
            {tpl::tr("4.5 ГГц", "4.5 GHz"), tpl::tr("|S11| -0.3 дБ", "|S11| -0.3 dB")},
        }},
        {QStringLiteral("ms_patch"), QStringLiteral("2026-10-27"), tpl::tr("сетка 235x160, шаг 0.5 мм, TIME 35 нс, шагов 25", "grid 235x160, step 0.5 mm, TIME 35 ns, steps 25"), {
            {tpl::tr("2.1 ГГц", "2.1 GHz"), tpl::tr("|S11| -0.5 дБ", "|S11| -0.5 dB")},
            {tpl::tr("3 ГГц", "3 GHz"), tpl::tr("|S11| -0.6 дБ", "|S11| -0.6 dB")},
            {tpl::tr("3.3 ГГц", "3.3 GHz"), tpl::tr("|S11| -12.8 дБ", "|S11| -12.8 dB")},
            {tpl::tr("3.9 ГГц", "3.9 GHz"), tpl::tr("|S11| -1.6 дБ", "|S11| -1.6 dB")},
        }},
        {QStringLiteral("ms_qwt"), QStringLiteral("2026-10-26"), tpl::tr("сетка 420x26, шаг 0.3 мм, TIME 15 нс, шагов 25", "grid 420x26, step 0.3 mm, TIME 15 ns, steps 25"), {
            {tpl::tr("1.2 ГГц", "1.2 GHz"), tpl::tr("|S11| -9.3 дБ, |S21| -0.5 дБ", "|S11| -9.3 dB, |S21| -0.5 dB")},
            {tpl::tr("2.4 ГГц", "2.4 GHz"), tpl::tr("|S11| -27.3 дБ, |S21| 0.0 дБ", "|S11| -27.3 dB, |S21| 0.0 dB")},
            {tpl::tr("3 ГГц", "3 GHz"), tpl::tr("|S11| -22.7 дБ, |S21| -0.0 дБ", "|S11| -22.7 dB, |S21| -0.0 dB")},
            {tpl::tr("4.8 ГГц", "4.8 GHz"), tpl::tr("|S11| -9.7 дБ, |S21| -0.6 дБ", "|S11| -9.7 dB, |S21| -0.6 dB")},
        }},
        {QStringLiteral("ms_stub"), QStringLiteral("2026-10-26"), tpl::tr("сетка 206x40, шаг 0.56284 мм, TIME 20 нс, шагов 41", "grid 206x40, step 0.56284 mm, TIME 20 ns, steps 41"), {
            {tpl::tr("1.5 ГГц", "1.5 GHz"), tpl::tr("|S11| -12.0 дБ, |S21| -0.6 дБ", "|S11| -12.0 dB, |S21| -0.6 dB")},
            {tpl::tr("3 ГГц", "3 GHz"), tpl::tr("|S11| -0.0 дБ, |S21| -21.0 дБ", "|S11| -0.0 dB, |S21| -21.0 dB")},
            {tpl::tr("4.5 ГГц", "4.5 GHz"), tpl::tr("|S11| -5.8 дБ, |S21| -1.8 дБ", "|S11| -5.8 dB, |S21| -1.8 dB")},
        }},
        {QStringLiteral("ms_taper"), QStringLiteral("2026-10-27"), tpl::tr("сетка 364x26, шаг 0.3 мм, TIME 15 нс, шагов 35", "grid 364x26, step 0.3 mm, TIME 15 ns, steps 35"), {
            {tpl::tr("0.9 ГГц", "0.9 GHz"), tpl::tr("|S11| -3.0 дБ, |S21| -3.1 дБ", "|S11| -3.0 dB, |S21| -3.1 dB")},
            {tpl::tr("2.7 ГГц", "2.7 GHz"), tpl::tr("|S11| -22.8 дБ, |S21| -0.0 дБ", "|S11| -22.8 dB, |S21| -0.0 dB")},
            {tpl::tr("3.45 ГГц", "3.45 GHz"), tpl::tr("|S11| -22.5 дБ, |S21| -0.0 дБ", "|S11| -22.5 dB, |S21| -0.0 dB")},
            {tpl::tr("6 ГГц", "6 GHz"), tpl::tr("|S11| -15.6 дБ, |S21| -0.1 дБ", "|S11| -15.6 dB, |S21| -0.1 dB")},
        }},
        {QStringLiteral("ms_tee"), QStringLiteral("2026-10-27"), tpl::tr("сетка 381x219, шаг 0.29459 мм, TIME 25 нс, шагов 21", "grid 381x219, step 0.29459 mm, TIME 25 ns, steps 21"), {
            {tpl::tr("1.8 ГГц", "1.8 GHz"), tpl::tr("|S11| -14.6 дБ, |S21| -3.3 дБ, |S31| -3.2 дБ", "|S11| -14.6 dB, |S21| -3.3 dB, |S31| -3.2 dB")},
            {tpl::tr("3 ГГц", "3 GHz"), tpl::tr("|S11| -15.2 дБ, |S21| -3.1 дБ, |S31| -3.2 дБ", "|S11| -15.2 dB, |S21| -3.1 dB, |S31| -3.2 dB")},
            {tpl::tr("4.2 ГГц", "4.2 GHz"), tpl::tr("|S11| -29.9 дБ, |S21| -2.9 дБ, |S31| -3.1 дБ", "|S11| -29.9 dB, |S21| -2.9 dB, |S31| -3.1 dB")},
        }},
        {QStringLiteral("pl_column"), QStringLiteral("2026-10-27"), tpl::tr("сетка 428x46, шаг 1.8035 мм, TIME 45 нс, шагов 21", "grid 428x46, step 1.8035 mm, TIME 45 ns, steps 21"), {
            {tpl::tr("2.6 ГГц", "2.6 GHz"), tpl::tr("|S11| -3.3 дБ, |S21| -2.7 дБ", "|S11| -3.3 dB, |S21| -2.7 dB")},
            {tpl::tr("3.275 ГГц", "3.275 GHz"), tpl::tr("|S11| -7.0 дБ, |S21| -1.0 дБ", "|S11| -7.0 dB, |S21| -1.0 dB")},
            {tpl::tr("3.95 ГГц", "3.95 GHz"), tpl::tr("|S11| -10.4 дБ, |S21| -0.4 дБ", "|S11| -10.4 dB, |S21| -0.4 dB")},
        }},
        {QStringLiteral("pl_magnetized"), QStringLiteral("2026-10-27"), tpl::tr("сетка 437x46, шаг 1.8035 мм, TIME 45 нс, шагов 21", "grid 437x46, step 1.8035 mm, TIME 45 ns, steps 21"), {
            {tpl::tr("2.6 ГГц", "2.6 GHz"), tpl::tr("|S11| -4.5 дБ, |S21| -1.9 дБ", "|S11| -4.5 dB, |S21| -1.9 dB")},
            {tpl::tr("3.005 ГГц", "3.005 GHz"), tpl::tr("|S11| -26.9 дБ, |S21| -0.0 дБ", "|S11| -26.9 dB, |S21| -0.0 dB")},
            {tpl::tr("3.275 ГГц", "3.275 GHz"), tpl::tr("|S11| -4.3 дБ, |S21| -2.1 дБ", "|S11| -4.3 dB, |S21| -2.1 dB")},
            {tpl::tr("3.95 ГГц", "3.95 GHz"), tpl::tr("|S11| -3.7 дБ, |S21| -2.4 дБ", "|S11| -3.7 dB, |S21| -2.4 dB")},
        }},
        {QStringLiteral("pl_slab"), QStringLiteral("2026-10-27"), tpl::tr("сетка 437x46, шаг 1.8035 мм, TIME 40 нс, шагов 21", "grid 437x46, step 1.8035 mm, TIME 40 ns, steps 21"), {
            {tpl::tr("2.6 ГГц", "2.6 GHz"), tpl::tr("|S11| -1.7 дБ, |S21| -4.9 дБ", "|S11| -1.7 dB, |S21| -4.9 dB")},
            {tpl::tr("3.275 ГГц", "3.275 GHz"), tpl::tr("|S11| -4.1 дБ, |S21| -2.1 дБ", "|S11| -4.1 dB, |S21| -2.1 dB")},
            {tpl::tr("3.95 ГГц", "3.95 GHz"), tpl::tr("|S11| -6.7 дБ, |S21| -1.0 дБ", "|S11| -6.7 dB, |S21| -1.0 dB")},
        }},
        {QStringLiteral("rcs_airfoil"), QStringLiteral("2026-10-27"), tpl::tr("сетка 280x280, шаг 1.5 мм, TIME 30 нс, шагов 38", "grid 280x280, step 1.5 mm, TIME 30 ns, steps 38"), {
            {tpl::tr("5 ГГц (без объекта)", "5 GHz (no object)"), tpl::tr("|S11| -38.6 дБ", "|S11| -38.6 dB")},
            {tpl::tr("угол 5°", "angle 5 deg"), tpl::tr("|S11| -28.9 дБ", "|S11| -28.9 dB")},
            {tpl::tr("угол 95°", "angle 95 deg"), tpl::tr("|S11| -10.1 дБ", "|S11| -10.1 dB")},
            {tpl::tr("угол 110°", "angle 110 deg"), tpl::tr("|S11| -37.8 дБ", "|S11| -37.8 dB")},
            {tpl::tr("угол 0°", "angle 0 deg"), tpl::tr("|S11| -29.2 дБ", "|S11| -29.2 dB")},
        }},
        {QStringLiteral("rcs_coated"), QStringLiteral("2026-10-27"), tpl::tr("сетка 340x340, шаг 1.5 мм, TIME 35 нс, шагов 27", "grid 340x340, step 1.5 mm, TIME 35 ns, steps 27"), {
            {tpl::tr("2 ГГц (без объекта)", "2 GHz (no object)"), tpl::tr("|S11| -32.5 дБ", "|S11| -32.5 dB")},
            {tpl::tr("5 ГГц (без объекта)", "5 GHz (no object)"), tpl::tr("|S11| -38.1 дБ", "|S11| -38.1 dB")},
            {tpl::tr("5.75 ГГц (без объекта)", "5.75 GHz (no object)"), tpl::tr("|S11| -40.7 дБ", "|S11| -40.7 dB")},
            {tpl::tr("8 ГГц (без объекта)", "8 GHz (no object)"), tpl::tr("|S11| -40.3 дБ", "|S11| -40.3 dB")},
            {tpl::tr("2 ГГц (металл)", "2 GHz (metal)"), tpl::tr("|S11| -19.5 дБ", "|S11| -19.5 dB")},
            {tpl::tr("5 ГГц (металл)", "5 GHz (metal)"), tpl::tr("|S11| -27.5 дБ", "|S11| -27.5 dB")},
            {tpl::tr("8 ГГц (металл)", "8 GHz (metal)"), tpl::tr("|S11| -29.8 дБ", "|S11| -29.8 dB")},
            {tpl::tr("2 ГГц (металл + плазма)", "2 GHz (metal + plasma)"), tpl::tr("|S11| -19.2 дБ", "|S11| -19.2 dB")},
            {tpl::tr("5 ГГц (металл + плазма)", "5 GHz (metal + plasma)"), tpl::tr("|S11| -28.0 дБ", "|S11| -28.0 dB")},
            {tpl::tr("8 ГГц (металл + плазма)", "8 GHz (metal + plasma)"), tpl::tr("|S11| -29.0 дБ", "|S11| -29.0 dB")},
        }},
        {QStringLiteral("rcs_cylinder"), QStringLiteral("2026-10-27"), tpl::tr("сетка 326x326, шаг 1.5 мм, TIME 30 нс, шагов 26", "grid 326x326, step 1.5 mm, TIME 30 ns, steps 26"), {
            {tpl::tr("2 ГГц (без объекта)", "2 GHz (no object)"), tpl::tr("|S11| -30.8 дБ", "|S11| -30.8 dB")},
            {tpl::tr("5 ГГц (без объекта)", "5 GHz (no object)"), tpl::tr("|S11| -38.7 дБ", "|S11| -38.7 dB")},
            {tpl::tr("8 ГГц (без объекта)", "8 GHz (no object)"), tpl::tr("|S11| -43.3 дБ", "|S11| -43.3 dB")},
            {tpl::tr("2 ГГц (цилиндр)", "2 GHz (cylinder)"), tpl::tr("|S11| -18.2 дБ", "|S11| -18.2 dB")},
            {tpl::tr("2.5 ГГц (цилиндр)", "2.5 GHz (cylinder)"), tpl::tr("|S11| -27.5 дБ", "|S11| -27.5 dB")},
            {tpl::tr("5 ГГц (цилиндр)", "5 GHz (cylinder)"), tpl::tr("|S11| -26.2 дБ", "|S11| -26.2 dB")},
            {tpl::tr("8 ГГц (цилиндр)", "8 GHz (cylinder)"), tpl::tr("|S11| -25.9 дБ", "|S11| -25.9 dB")},
        }},
        {QStringLiteral("rcs_strip"), QStringLiteral("2026-10-27"), tpl::tr("сетка 240x240, шаг 1.5 мм, TIME 25 нс, шагов 38", "grid 240x240, step 1.5 mm, TIME 25 ns, steps 38"), {
            {tpl::tr("5 ГГц (без объекта)", "5 GHz (no object)"), tpl::tr("|S11| -36.3 дБ", "|S11| -36.3 dB")},
            {tpl::tr("угол 5°", "angle 5 deg"), tpl::tr("|S11| -11.2 дБ", "|S11| -11.2 dB")},
            {tpl::tr("угол 30°", "angle 30 deg"), tpl::tr("|S11| -42.5 дБ", "|S11| -42.5 dB")},
            {tpl::tr("угол 95°", "angle 95 deg"), tpl::tr("|S11| -25.3 дБ", "|S11| -25.3 dB")},
            {tpl::tr("угол 0°", "angle 0 deg"), tpl::tr("|S11| -10.0 дБ", "|S11| -10.0 dB")},
        }},
        {QStringLiteral("wg_bend"), QStringLiteral("2026-10-26"), tpl::tr("сетка 175x175, шаг 0.5715 мм, TIME 20 нс, шагов 21", "grid 175x175, step 0.5715 mm, TIME 20 ns, steps 21"), {
            {tpl::tr("8.2 ГГц", "8.2 GHz"), tpl::tr("|S11| -18.5 дБ, |S21| -0.1 дБ", "|S11| -18.5 dB, |S21| -0.1 dB")},
            {tpl::tr("10.3 ГГц", "10.3 GHz"), tpl::tr("|S11| -39.2 дБ, |S21| -0.0 дБ", "|S11| -39.2 dB, |S21| -0.0 dB")},
            {tpl::tr("10.51 ГГц", "10.51 GHz"), tpl::tr("|S11| -52.8 дБ, |S21| 0.0 дБ", "|S11| -52.8 dB, |S21| 0.0 dB")},
            {tpl::tr("12.4 ГГц", "12.4 GHz"), tpl::tr("|S11| -24.0 дБ, |S21| -0.0 дБ", "|S11| -24.0 dB, |S21| -0.0 dB")},
        }},
        {QStringLiteral("wg_filter"), QStringLiteral("2026-10-26"), tpl::tr("сетка 359x54, шаг 0.47625 мм, TIME 25 нс, шагов 31", "grid 359x54, step 0.47625 mm, TIME 25 ns, steps 31"), {
            {tpl::tr("9 ГГц", "9 GHz"), tpl::tr("|S11| 0.0 дБ, |S21| -54.1 дБ", "|S11| 0.0 dB, |S21| -54.1 dB")},
            {tpl::tr("10 ГГц", "10 GHz"), tpl::tr("|S11| -0.8 дБ, |S21| -7.6 дБ", "|S11| -0.8 dB, |S21| -7.6 dB")},
            {tpl::tr("10.13 ГГц", "10.13 GHz"), tpl::tr("|S11| -8.6 дБ, |S21| -0.6 дБ", "|S11| -8.6 dB, |S21| -0.6 dB")},
            {tpl::tr("11 ГГц", "11 GHz"), tpl::tr("|S11| -0.0 дБ, |S21| -22.9 дБ", "|S11| -0.0 dB, |S21| -22.9 dB")},
        }},
        {QStringLiteral("wg_iris"), QStringLiteral("2026-10-26"), tpl::tr("сетка 264x46, шаг 0.5715 мм, TIME 20 нс, шагов 21", "grid 264x46, step 0.5715 mm, TIME 20 ns, steps 21"), {
            {tpl::tr("8.2 ГГц", "8.2 GHz"), tpl::tr("|S11| -1.0 дБ, |S21| -6.9 дБ", "|S11| -1.0 dB, |S21| -6.9 dB")},
            {tpl::tr("10.3 ГГц", "10.3 GHz"), tpl::tr("|S11| -2.5 дБ, |S21| -3.6 дБ", "|S11| -2.5 dB, |S21| -3.6 dB")},
            {tpl::tr("12.4 ГГц", "12.4 GHz"), tpl::tr("|S11| -4.4 дБ, |S21| -2.0 дБ", "|S11| -4.4 dB, |S21| -2.0 dB")},
        }},
        {QStringLiteral("wg_load"), QStringLiteral("2026-10-26"), tpl::tr("сетка 333x46, шаг 0.5715 мм, TIME 20 нс, шагов 33", "grid 333x46, step 0.5715 mm, TIME 20 ns, steps 33"), {
            {tpl::tr("8.2 ГГц (короткое замыкание)", "8.2 GHz (short circuit)"), tpl::tr("|S11| -0.0 дБ", "|S11| -0.0 dB")},
            {tpl::tr("10.3 ГГц (короткое замыкание)", "10.3 GHz (short circuit)"), tpl::tr("|S11| -0.0 дБ", "|S11| -0.0 dB")},
            {tpl::tr("12.4 ГГц (короткое замыкание)", "12.4 GHz (short circuit)"), tpl::tr("|S11| -0.0 дБ", "|S11| -0.0 dB")},
            {tpl::tr("8.2 ГГц (плоский поглотитель)", "8.2 GHz (flat absorber)"), tpl::tr("|S11| -12.1 дБ", "|S11| -12.1 dB")},
            {tpl::tr("10.3 ГГц (плоский поглотитель)", "10.3 GHz (flat absorber)"), tpl::tr("|S11| -17.8 дБ", "|S11| -17.8 dB")},
            {tpl::tr("12.4 ГГц (плоский поглотитель)", "12.4 GHz (flat absorber)"), tpl::tr("|S11| -21.8 дБ", "|S11| -21.8 dB")},
            {tpl::tr("8.2 ГГц (клин)", "8.2 GHz (wedge)"), tpl::tr("|S11| -7.2 дБ", "|S11| -7.2 dB")},
            {tpl::tr("10.3 ГГц (клин)", "10.3 GHz (wedge)"), tpl::tr("|S11| -11.1 дБ", "|S11| -11.1 dB")},
            {tpl::tr("12.4 ГГц (клин)", "12.4 GHz (wedge)"), tpl::tr("|S11| -14.5 дБ", "|S11| -14.5 dB")},
        }},
        {QStringLiteral("wg_post"), QStringLiteral("2026-10-26"), tpl::tr("сетка 396x66, шаг 0.381 мм, TIME 20 нс, шагов 21", "grid 396x66, step 0.381 mm, TIME 20 ns, steps 21"), {
            {tpl::tr("8.2 ГГц", "8.2 GHz"), tpl::tr("|S11| -0.3 дБ, |S21| -12.4 дБ", "|S11| -0.3 dB, |S21| -12.4 dB")},
            {tpl::tr("10.3 ГГц", "10.3 GHz"), tpl::tr("|S11| -0.8 дБ, |S21| -7.9 дБ", "|S11| -0.8 dB, |S21| -7.9 dB")},
            {tpl::tr("12.4 ГГц", "12.4 GHz"), tpl::tr("|S11| -1.6 дБ, |S21| -5.1 дБ", "|S11| -1.6 dB, |S21| -5.1 dB")},
        }},
        {QStringLiteral("wg_section"), QStringLiteral("2026-10-26"), tpl::tr("сетка 198x46, шаг 0.5715 мм, TIME 15 нс, шагов 21", "grid 198x46, step 0.5715 mm, TIME 15 ns, steps 21"), {
            {tpl::tr("8.2 ГГц", "8.2 GHz"), tpl::tr("|S11| -91.1 дБ, |S21| 0.0 дБ", "|S11| -91.1 dB, |S21| 0.0 dB")},
            {tpl::tr("10.3 ГГц", "10.3 GHz"), tpl::tr("|S11| -100.3 дБ, |S21| 0.0 дБ", "|S11| -100.3 dB, |S21| 0.0 dB")},
            {tpl::tr("11.77 ГГц", "11.77 GHz"), tpl::tr("|S11| -111.5 дБ, |S21| 0.0 дБ", "|S11| -111.5 dB, |S21| 0.0 dB")},
            {tpl::tr("12.4 ГГц", "12.4 GHz"), tpl::tr("|S11| -107.1 дБ, |S21| 0.0 дБ", "|S11| -107.1 dB, |S21| 0.0 dB")},
        }},
        {QStringLiteral("wg_step"), QStringLiteral("2026-10-26"), tpl::tr("сетка 258x46, шаг 0.5715 мм, TIME 20 нс, шагов 42", "grid 258x46, step 0.5715 mm, TIME 20 ns, steps 42"), {
            {tpl::tr("8.49 ГГц", "8.49 GHz"), tpl::tr("|S11| -13.7 дБ, |S21| -0.2 дБ", "|S11| -13.7 dB, |S21| -0.2 dB")},
            {tpl::tr("10.44 ГГц", "10.44 GHz"), tpl::tr("|S11| -22.9 дБ, |S21| -0.0 дБ", "|S11| -22.9 dB, |S21| -0.0 dB")},
            {tpl::tr("12.4 ГГц", "12.4 GHz"), tpl::tr("|S11| -27.9 дБ, |S21| -0.0 дБ", "|S11| -27.9 dB, |S21| -0.0 dB")},
        }},
        {QStringLiteral("wg_tee"), QStringLiteral("2026-10-26"), tpl::tr("сетка 304x175, шаг 0.5715 мм, TIME 20 нс, шагов 33", "grid 304x175, step 0.5715 mm, TIME 20 ns, steps 33"), {
            {tpl::tr("8.2 ГГц", "8.2 GHz"), tpl::tr("|S11| -9.1 дБ, |S21| -2.6 дБ, |S31| -4.8 дБ", "|S11| -9.1 dB, |S21| -2.6 dB, |S31| -4.8 dB")},
            {tpl::tr("10.3 ГГц", "10.3 GHz"), tpl::tr("|S11| -13.6 дБ, |S21| -1.9 дБ, |S31| -5.0 дБ", "|S11| -13.6 dB, |S21| -1.9 dB, |S31| -5.0 dB")},
            {tpl::tr("12.4 ГГц", "12.4 GHz"), tpl::tr("|S11| -11.0 дБ, |S21| -0.5 дБ, |S31| -14.1 дБ", "|S11| -11.0 dB, |S21| -0.5 dB, |S31| -14.1 dB")},
        }},
        {QStringLiteral("wg_window"), QStringLiteral("2026-10-26"), tpl::tr("сетка 284x46, шаг 0.5715 мм, TIME 20 нс, шагов 21", "grid 284x46, step 0.5715 mm, TIME 20 ns, steps 21"), {
            {tpl::tr("8.2 ГГц", "8.2 GHz"), tpl::tr("|S11| -6.7 дБ, |S21| -1.1 дБ", "|S11| -6.7 dB, |S21| -1.1 dB")},
            {tpl::tr("10.09 ГГц", "10.09 GHz"), tpl::tr("|S11| -46.4 дБ, |S21| -0.0 дБ", "|S11| -46.4 dB, |S21| -0.0 dB")},
            {tpl::tr("10.3 ГГц", "10.3 GHz"), tpl::tr("|S11| -28.0 дБ, |S21| -0.0 дБ", "|S11| -28.0 dB, |S21| -0.0 dB")},
            {tpl::tr("12.4 ГГц", "12.4 GHz"), tpl::tr("|S11| -9.3 дБ, |S21| -0.5 дБ", "|S11| -9.3 dB, |S21| -0.5 dB")},
        }},
    };
}

} // namespace

const QVector<Baked> &bakedTable()
{
    if (tpl::english()) {
        static const QVector<Baked> en = makeTable();
        return en;
    }
    static const QVector<Baked> ru = makeTable();
    return ru;
}

} // namespace tpl::samples
