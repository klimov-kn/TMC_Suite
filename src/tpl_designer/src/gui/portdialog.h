/// @file portdialog.h
/// @brief Диалог «Свойства порта»: размещение входа, среда в сечении, активность, тип волны
///        и сигнал падающей волны (формулой ядра или файлом `.sig`).
///
/// Часть интерфейса редактора заданий. Диалог правит копию `tpl::Port` и возвращает её
/// методом `port()`; в задание её кладёт вызывающий (главное окно).
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-dialogs
#pragma once

#include "project.h"

#include <QDialog>
#include <QHash>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QStackedWidget;
class QWidget;
QT_END_NAMESPACE

class SignalGallery;
class SignalPreview;

/// Свойства порта: сторона, апертура, активность, форма сигнала.
/// График рисуется по той же формуле, что уйдёт в файл, — чтобы не врал.
///
/// Сигнал задаётся двумя разными способами, и выбор между ними не вкус:
///  * ФОРМУЛОЙ — ядро считает `Amp(t,f)*sin(2*pi*FREQ*t + Faza)` само. Дешево и точно,
///    но выразить этим можно только несущую под огибающей.
///  * ФАЙЛОМ `.sig` — файл задаёт всю `a(t)`, и форма может быть любой: гребёнка
///    тонов (вся полоса за один проход), ЛЧМ, пила, видеоимпульс, своя формула, свои
///    точки. Файл пишет генератор, человек выбирает форму картинкой.
/// @brief Модальный диалог свойств одного порта.
///
/// Открывается главным окном (`MainWindow::onEditRequested`, запрос правки порта от
/// холста, панели свойств или списка объектов), `exec()`; объект живёт на стеке вызывающего. Задание (`Project`) держится ссылкой
/// только для чтения: из него берутся время счёта, частоты и фигуры (eps среды под
/// входом). Любая правка поля вызывает `refresh()`, который пересчитывает предпросмотр
/// и подсказки. Потоки: только поток интерфейса.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-dialogs
/// @see https://www.tamic.ru/docs/theory/ports.html#signal
/// @see https://www.tamic.ru/docs/theory/ports.html#incident-file
class PortDialog : public QDialog
{
    Q_OBJECT
public:
    /// @brief Строит группы «Размещение», «Возбуждение», «Что придёт на вход» и кнопки
    ///        «Применить»/«Отмена».
    /// @param[in] p        исходный порт (копируется; точки своей таблицы и тоны `#FREQ` — тоже).
    /// @param[in] project  задание, в котором стоит порт; должно жить дольше диалога.
    /// @param[in] parent   родительское окно, может быть `nullptr`.
    PortDialog(const tpl::Port &p, const tpl::Project &project, QWidget *parent = nullptr);

    /// @brief Собирает порт из полей диалога.
    /// @return Копия исходного порта с полями окна: имя, сторона, апертура `from`..`to` (мм),
    ///         активность, мода, eps входа (сама eps) и признак `epsAuto`, сигнал формулой
    ///         (амплитуда, фаза в радианах, центр и длительность в нс) и форма сигнала из файла
    ///         (`Waveshape`, имя файла `.sig` без расширения).
    tpl::Port port() const;

    /// Частота согласования, которую просит выбранная форма сигнала, ГГц.
    /// 0 — форма ничего не советует (своя формула, своя таблица) или сигнал формулой.
    ///
    /// Окно само частоту задания не меняет: `FREQ` живёт в проекте, и ставит её тот,
    /// кто проектом владеет. Но оставлять это на память человеку нельзя — уехавшая
    /// `FREQ` портит согласование порта молча.
    /// @brief Возвращает частоту согласования порта, которую советует форма сигнала из файла.
    /// @return Частота, ГГц (`tpl::suggestedFreq`); 0 — порт пассивный, сигнал формулой или
    ///         форма частоты не советует.
    double wantedFreq() const;

private slots:
    /// @brief Пересчитывает видимость полей формы, предпросмотр, строку «что уйдёт в файл»,
    ///        eps входа, одномодовый диапазон апертуры и подсказку.
    ///
    /// Вызывает ядро генератора: `tpl::waveFields` (какие поля есть у формы), `tpl::sigPlan`
    /// (число точек и шаг файла `.sig`), `tpl::portWindow`, `tpl::signalFileName`,
    /// `tpl::ampExpression`, `tpl::portMediumEps` (eps среды под входом), `tpl::cutoffH10`,
    /// `tpl::cutoffH20`, `tpl::waveBand`, `tpl::waveSupport`. Предупреждения — только о том,
    /// что ядро посчитает не так, как задумано (полоса за отсечкой H20 или ниже H10, сигнал
    /// длиннее счёта, резкое окно).
    /// @see https://www.tamic.ru/docs/theory/ports.html#port-w
    void refresh();

private:
    /// @brief Строит группу «Размещение»: имя, сторона, апертура, eps входа и подсказки к ним.
    /// @param[in] parent  виджет-содержимое, в компоновку которого добавляется группа.
    void buildPlacement(QWidget *parent);
    /// @brief Строит группу «Возбуждение»: активность, тип волны, источник сигнала и две страницы
    ///        полей (формулой ядра; файлом — галерея форм и параметры формы).
    /// @param[in] parent  виджет-содержимое, в компоновку которого добавляется группа.
    void buildExcitation(QWidget *parent);
    /// @brief Строит группу «Что придёт на вход»: предпросмотр сигнала и две строки пояснений.
    /// @param[in] parent  виджет-содержимое, в компоновку которого добавляется группа.
    void buildPreview(QWidget *parent);
    /// @brief Загружает точки своей таблицы из текстового файла (`.sig`, `.txt`, `.csv`, `.dat`).
    ///
    /// Читает файл `tpl::readTplFileText`, точки — `tpl::parseSigPoints` (время, нс; значение),
    /// тоны строки `#FREQ` — `tpl::parseSigTones`; выбирает форму «своя таблица». Меньше двух
    /// точек — окно с причиной; больше 99 тонов — окно `tpl::toneCutNotice`.
    void loadTableFromFile();

    /// Редактируемая копия порта (поля, которых нет в окне, переносятся в `port()` как есть).
    tpl::Port m_port;
    /// Задание, в котором стоит порт (только чтение).
    const tpl::Project &m_project;

    /// Название порта.
    QLineEdit *m_name = nullptr;
    /// Сторона области: левая, правая, нижняя, верхняя (`tpl::PortSide`).
    QComboBox *m_side = nullptr;
    /// Начало апертуры, мм.
    QDoubleSpinBox *m_from = nullptr;
    /// Конец апертуры, мм.
    QDoubleSpinBox *m_to = nullptr;
    QCheckBox *m_epsAuto = nullptr; ///< eps входа — по среде под ним (Port::epsAuto)
    /// Проницаемость в сечении входа (сама eps, не меньше 0.001); при `m_epsAuto` — только показ.
    QDoubleSpinBox *m_eps = nullptr;
    /// Переключатель «Активный (излучает)».
    QRadioButton *m_active = nullptr;
    /// Переключатель «Пассивный (только принимает)».
    QRadioButton *m_passive = nullptr;
    /// Тип волны: H10 или плоская (`tpl::PortMode`).
    QComboBox *m_mode = nullptr;

    // --- источник сигнала ---
    /// Источник сигнала: формула ядра.
    QRadioButton *m_byFormula = nullptr;
    /// Источник сигнала: файл `.sig`.
    QRadioButton *m_byFile = nullptr;
    /// Две страницы полей: 0 — формулой, 1 — файлом.
    QStackedWidget *m_sourceStack = nullptr;

    // --- сигнал формулой ---
    /// Огибающая сигнала формулой: непрерывный, гауссов, прямоугольный (`tpl::SignalShape`).
    QComboBox *m_shape = nullptr;
    /// Амплитуда сигнала формулой (безразмерная).
    QDoubleSpinBox *m_amp = nullptr;
    /// Фаза сигнала формулой, градусы (в модели — радианы).
    QDoubleSpinBox *m_phase = nullptr;
    /// Центр импульса, нс.
    QDoubleSpinBox *m_center = nullptr;
    /// Длительность импульса, нс.
    QDoubleSpinBox *m_duration = nullptr;

    // --- сигнал файлом ---
    /// Галерея форм сигнала из файла.
    SignalGallery *m_gallery = nullptr;
    /// Амплитуда формы (безразмерная).
    QDoubleSpinBox *m_wAmp = nullptr;
    /// Начало сигнала, нс.
    QDoubleSpinBox *m_wStart = nullptr;
    /// Длительность посылки, нс.
    QDoubleSpinBox *m_wDuration = nullptr;
    /// Нижняя частота полосы, несущая или частота повторения (подпись меняется по форме), ГГц.
    QDoubleSpinBox *m_wF1 = nullptr;
    /// Верхняя частота полосы, ГГц.
    QDoubleSpinBox *m_wF2 = nullptr;
    /// Число тонов гребёнки (2...99).
    QSpinBox *m_wTones = nullptr;
    /// Число периодов синуса в посылке.
    QSpinBox *m_wPeriods = nullptr;
    /// Параметр tau гауссовой шапки и моноцикла, нс.
    QDoubleSpinBox *m_wWidth = nullptr;
    /// Фронт трапеции, нс.
    QDoubleSpinBox *m_wRise = nullptr;
    /// Спад трапеции, нс.
    QDoubleSpinBox *m_wFall = nullptr;
    /// Формула a(t) своей формы, t в нс.
    QLineEdit *m_wExpr = nullptr;
    /// Имя файла `.sig` без расширения; пусто — имя даст программа.
    QLineEdit *m_wFile = nullptr;
    /// Кнопка «Загрузить точки из файла».
    QPushButton *m_wLoad = nullptr;
    /// Счётчик точек своей таблицы и тонов `#FREQ`.
    QLabel *m_wTable = nullptr;
    /// Пояснение к файлу `.sig`: имя, число точек, шаг, пик сигнала.
    QLabel *m_wFileInfo = nullptr;
    /// Поле -> его метка в форме. Нужна, чтобы прятать СТРОКУ целиком: спрятанное
    /// поле с висящей рядом подписью выглядит поломкой, а не «этого у формы нет».
    /// Сам QFormLayout не в состоянии этого сделать за нас: он живёт внутри
    /// компоновки страницы, и по виджету его не найти.
    QHash<QWidget *, QWidget *> m_rowLabel;

    /// Предпросмотр сигнала и спектра.
    SignalPreview *m_preview = nullptr;
    /// Строка «что уйдёт в файл задания» (Amp и окно).
    QLabel *m_formula = nullptr;
    /// Одномодовый диапазон апертуры (отсечки H10/H20 с учётом eps).
    QLabel *m_cutoff = nullptr;
    /// Пояснение к eps входа: добавка eps-1, которая уйдёт в файл.
    QLabel *m_epsHint = nullptr;
    /// Подсказка внизу окна (цвет предупреждения — при ошибочном замысле).
    QLabel *m_hint = nullptr;

    /// Точки своей таблицы: живут здесь, потому что в поля их не положить.
    QVector<QPointF> m_points;
    /// Строка `#FREQ` принесённого файла: многотон, посчитанный на стороне, им и остаётся.
    QVector<double> m_tableTones;
};
