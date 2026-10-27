/// @file calculatordialog.h
/// @brief Справочный калькулятор TPL Designer: отсечки волновода, длина волны в диэлектрике,
///        плазменная и циклотронная частоты, отсечки X-моды.
///
/// Часть интерфейса редактора заданий. Окно ничего не меняет в задании; формулы берёт
/// из `core/physics.h` — те же, что использует проверка задания.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-dialogs
#pragma once

#include <QDialog>

QT_BEGIN_NAMESPACE
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
QT_END_NAMESPACE

/// Калькулятор среды и волновода.
///
/// Считает то, что иначе приходится держать в голове или в чужой табличке: отсечки
/// волновода, длину волны в диэлектрике, плазменную и циклотронную частоты, отсечки
/// X-моды под подмагничиванием. Все пары двусторонние: ввели N — получили f_p,
/// ввели f_p — получили нужное N.
///
/// Ничего не меняет в задании: это справочный инструмент.
/// @brief Модальное справочное окно «Калькулятор среды и волновода».
///
/// Три группы полей (волновод, диэлектрик, плазма), каждая пара значений связана в обе
/// стороны. Открывается кнопкой калькулятора в левой колонке мастера (`MainWindow`,
/// `exec()`); объект живёт на стеке вызывающего.
/// Потоки: только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-dialogs
/// @see https://www.tamic.ru/docs/theory/plasma.html#cutoffs
class CalculatorDialog : public QDialog
{
    Q_OBJECT
public:
    /// @brief Строит три группы полей, связывает их и выполняет первый пересчёт.
    /// @param[in] parent  родительское окно (владелец диалога), может быть `nullptr`.
    explicit CalculatorDialog(QWidget *parent = nullptr);

private:
    /// @brief Пересчитывает пару «ширина волновода — отсечки H10/H20».
    ///
    /// Отсечка H10 `fc10 = c / (2 a sqrt(eps))`, H20 `fc20 = c / (a sqrt(eps))`; по ширине
    /// считаются обе отсечки, по отсечке H10 — ширина и отсечка H20.
    /// @param[in] fromWidth  true — исходные данные ширина `a` (мм) и eps заполнения;
    ///                       false — исходные данные отсечка H10 (ГГц).
    void recalcWaveguide(bool fromWidth);
    /// @brief Пересчитывает подсказку по диэлектрику: добавку eps-1, длину волны в вакууме и в среде.
    ///
    /// Входные поля — проницаемость eps (может быть <= 0, тогда выводится предупреждение
    /// о запредельной среде) и частота (ГГц); длины волн выводятся в мм.
    void recalcDielectric();
    /// @brief Пересчитывает пару «концентрация N, поле B — частоты f_p, f_ce» и отсечки X-моды.
    ///
    /// Отсечки `f_L`, `f_R` и верхний гибридный резонанс считает `tpl::plasmaBands` —
    /// тот же код, что у проверки задания.
    /// @param[in] fromValues  true — исходные данные N (м^-3) и B (Тл);
    ///                        false — исходные данные частоты f_p и f_ce (ГГц), по ним
    ///                        восстанавливаются N и B.
    /// @see https://www.tamic.ru/docs/theory/plasma.html#plasma-frequency
    /// @see https://www.tamic.ru/docs/theory/plasma.html#cyclotron
    void recalcPlasma(bool fromValues);

    /// Защита от повторного входа: true, пока идёт пересчёт (программная установка полей
    /// снова вызывает сигналы `valueChanged`).
    bool m_busy = false;

    // --- волновод ---
    /// Ширина волновода a, мм.
    QDoubleSpinBox *m_wgWidth = nullptr;
    /// Проницаемость заполнения волновода eps (сама eps, не добавка).
    QDoubleSpinBox *m_wgEps = nullptr;
    /// Отсечка волны H10, ГГц (поле двустороннее: ввод пересчитывает ширину).
    QDoubleSpinBox *m_fc10 = nullptr;
    /// Отсечка волны H20, ГГц.
    QDoubleSpinBox *m_fc20 = nullptr;
    /// Подсказка: держать свип между отсечками; во сколько раз заполнение их опускает.
    QLabel *m_wgHint = nullptr;

    // --- диэлектрик ---
    /// Проницаемость диэлектрика eps (сама eps; допускаются значения <= 0).
    QDoubleSpinBox *m_eps = nullptr;
    /// Частота для расчёта длины волны, ГГц.
    QDoubleSpinBox *m_freq = nullptr;
    /// Итог по диэлектрику: добавка eps-1 для задания, длины волн в вакууме и в среде, мм.
    QLabel *m_epsOut = nullptr;

    // --- плазма ---
    /// Концентрация электронов N, м^-3 (СИ), текстом — допускается запись с экспонентой.
    QLineEdit *m_n = nullptr;      ///< концентрация: порядки величины, спинбоксом мучение
    /// Плазменная частота f_p, ГГц.
    QDoubleSpinBox *m_fp = nullptr;
    /// Индукция подмагничивающего поля B, Тл.
    QDoubleSpinBox *m_b = nullptr;
    /// Циклотронная частота f_ce, ГГц.
    QDoubleSpinBox *m_fce = nullptr;
    /// Итог по плазме: отсечки X-моды f_L, f_R, верхний гибридный резонанс, f_p, ГГц.
    QLabel *m_plasmaOut = nullptr;
};
