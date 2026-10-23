// Общий запуск FieldView и средства снимков: имена приложения, оформление, выбор
// графического интерфейса.
/// @file startup.h
/// @brief Общий запуск FieldView и программы снимков: выбор графического интерфейса, имена, оформление.
///
/// Часть модуля FieldView (слой `app`).
/// @see https://www.tamic.ru/docs/code/fieldview.html#rhi
#pragma once

#include <QString>
#include <QStringList>

class QApplication;

namespace fieldview {

/// @brief Выбирает графический интерфейс Qt RHI для всех видов.
///
/// Разобрать ключи --rhi d3d11|d3d12|opengl|vulkan и --warp (удаляются из списка),
/// проверить, что выбранный интерфейс создаётся; при неудаче — запасной вариант:
/// Direct3D 11 → Direct3D 11 программный (WARP) → OpenGL. Возвращает имя выбранного.
///
/// Источник выбора — переменная окружения `FIELDVIEW_RHI` или ключи командной строки
/// (`--rhi X`, `--rhi=X`, `--warp`). «opengl»/«gl» и «vulkan» принимаются без проверки;
/// «d3d12» — если создаётся; иначе пробуются Direct3D 11, затем программный Direct3D 11
/// (WARP), затем OpenGL.
/// @param[in,out] args  аргументы программы без имени; распознанные ключи удаляются
/// @return Имя выбранного интерфейса («D3D11», «D3D11 (WARP)», «OpenGL» …).
/// @see https://www.tamic.ru/docs/code/fieldview.html#rhi
QString chooseGraphicsApi(QStringList *args);

/// @brief Задаёт имена приложения, место хранения настроек и оформление.
///
/// Имена приложения (раздел настроек), оформление.
///
/// На Windows настройки — INI-файл `%APPDATA%\TMC_Suite_Qt\FieldView.ini`; при первом
/// запуске в него переносятся настройки из реестра прежних версий.
/// @param[in,out] app  приложение
void setupApplication(QApplication &app);

} // namespace fieldview
