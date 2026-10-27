/// @file startwindow.h
/// @brief Стартовое окно TPL Designer: четыре карточки действий.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Страница стека окна
/// из `main.cpp`; сама ничего не открывает — только испускает сигналы.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-windows
#pragma once

#include <QWidget>

/// Стартовый экран: создать задание, открыть существующее, взять готовое.
///
/// Библиотека образцов открывается третьей карточкой (см. SamplesPage).
/// @brief Страница выбора действия: «Создать», «Открыть», «Взять образец», «Вспомогательные файлы».
///
/// Создаётся в `main.cpp` (заново при смене языка); владелец — стек страниц оболочки.
/// Связи сигналов с мастером и библиотекой ставит `main.cpp`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-windows
class StartWindow : public QWidget
{
    Q_OBJECT
public:
    /// @brief Строит заголовок, четыре карточки и подпись об авторстве ядер.
    /// @param[in] parent  родительский виджет; `nullptr` — без родителя.
    explicit StartWindow(QWidget *parent = nullptr);

signals:
    /// Начать новое задание в мастере (кнопка «Создать»).
    void createRequested();

    /// Открыть готовый .tpl и продолжить работу в тех же вкладках мастера.
    void openRequested();

    /// Открыть библиотеку образцов.
    void samplesRequested();

    /// Открыть мастерскую вспомогательных файлов (.sig и .eps).
    void filesRequested();
};
