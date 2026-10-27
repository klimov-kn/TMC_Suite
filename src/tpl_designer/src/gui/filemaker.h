/// @file filemaker.h
/// @brief Мастерская вспомогательных файлов: файл падающей волны `.sig` и файл распределения `.eps`.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Окно пишет файлы теми же
/// функциями ядра генератора, что и мастер (`tpl::sigFileText`, `tpl::epsFileBytes`), и
/// умеет читать готовые файлы обратно (`tpl::parseSigMarker`, `tpl::readEpsHeader`).
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-files
#pragma once

#include "epsfile.h"
#include "project.h"

#include <QDialog>
#include <QHash>
#include <QPointF>
#include <QVector>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QTableWidget;
QT_END_NAMESPACE

class SignalGallery;
class SignalPreview;

/// Карта распределения eps: то же, что ляжет в файл, но глазами.
///
/// Рисуется по тем же числам, что уйдут в `.eps` (`tpl::epsValues`), — иначе картинка
/// рано или поздно разошлась бы с файлом, а проверить её человеку нечем.
/// @brief Виджет-карта добавки eps по узлам сетки (цветом, от минимума к максимуму).
///
/// Создаётся вкладкой «Файл распределения» окна `FileMakerDialog`, владелец — окно.
/// Значения хранит копией; веса узлов (0.5 на ребре) на карте не применяются.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#eps-file
class EpsMapView : public QWidget
{
    Q_OBJECT
public:
    /// @brief Создаёт пустую карту (минимальная высота 240 пикселей).
    /// @param[in] parent  родительский виджет; `nullptr` — без родителя.
    explicit EpsMapView(QWidget *parent = nullptr);

    /// \p values — добавка eps по узлам сетки, построчно (`j = iY*nX + iX`).
    /// @brief Задаёт сетку и значения, находит их минимум и максимум и перерисовывает карту.
    /// @param[in] grid    сетка: шаг (мм), начало (мм), число узлов `nX`, `nY`.
    /// @param[in] values  значения по узлам, длина `nX*nY`; пустой массив — карта пуста.
    void setField(const tpl::EpsGrid &grid, const QVector<double> &values);
    /// @brief Убирает значения; на месте карты появляется приглашение ввести формулу.
    void clear();

protected:
    /// @brief Рисует карту изображением nX x nY пикселей, растянутым с сохранением пропорций (ось Y вверх).
    /// @param[in] e  событие рисования (не используется).
    void paintEvent(QPaintEvent *e) override;

private:
    /// Сетка показываемых значений.
    tpl::EpsGrid m_grid;
    /// Значения по узлам, построчно (`j = iY*nX + iX`).
    QVector<double> m_values;
    /// Наименьшее значение (низ цветовой шкалы).
    double m_lo = 0.0;
    /// Наибольшее значение (верх цветовой шкалы).
    double m_hi = 0.0;
};

/// Мастерская вспомогательных файлов: сделать `.sig` и `.eps` отдельно от задания.
///
/// Зачем отдельное окно. Оба файла программа и так пишет рядом с заданием, когда они
/// в нём заданы. Но готовый файл часто нужен САМ ПО СЕБЕ: положить рядом с чужим
/// заданием, отдать коллеге, подставить в уже написанный `.tpl`. Делать его в
/// блокноте нельзя — `.sig` требует шага вдвое мельче шага ядра, а `.eps` вообще
/// двоичный, с весами узлов. Поэтому оба рождаются здесь, и ровно тем же кодом,
/// каким их пишет мастер.
/// @brief Модальное окно с двумя вкладками: «Файл сигнала (.sig)» и «Файл распределения (.eps)».
///
/// Открывается карточкой «Вспомогательные файлы» стартового окна (`main.cpp`) и кнопкой
/// в мастере (`MainWindow`); создаётся на стеке и живёт до закрытия. Задание, переданное
/// в конструктор, только подсказывает умолчания и не меняется.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-files
class FileMakerDialog : public QDialog
{
    Q_OBJECT
public:
    /// \p p — задание, из которого берутся умолчания (шаг сетки, время счёта).
    /// Без задания подставляются разумные числа.
    /// @brief Строит обе вкладки и сразу считает предпросмотр сигнала и карту eps.
    /// @param[in] p       задание: берутся шаг сетки `delta` (мм, иначе 1) и время счёта `timeMax` (нс, иначе 40).
    /// @param[in] parent  родительское окно; `nullptr` — без родителя.
    explicit FileMakerDialog(const tpl::Project &p, QWidget *parent = nullptr);


    /// Форма сигнала, которую человек собрал в этом окне: мастер может забрать её
    /// в порт, а не только записать на диск.
    /// @brief Собирает форму сигнала из галереи, полей и таблицы точек.
    /// @return Форма: вид, амплитуда, начало и длительность (нс), частоты (ГГц), число тонов
    ///         и периодов, ширина и фронты (нс), формула, точки таблицы и тоны `#FREQ`.
    tpl::Waveshape waveshape() const;

    /// Открыть готовый `.sig`. Путь пуст — спросить файл.
    /// @brief Читает `.sig` (CP1251): по служебной строке `@TPLD-SIG` возвращает саму форму,
    ///        иначе кладёт точки в таблицу формы «Своя таблица».
    /// @param[in] given  путь к файлу; пустая строка — спросить файл окном выбора.
    void openSignalFile(const QString &given = QString());

private:
    /// @brief Строит вкладку сигнала: галерея форм, поля параметров, таблица точек,
    ///        «Под какое задание» (DELTA, TIME, имя), предпросмотр и кнопка записи.
    /// @return Страница вкладки (с прокруткой); владельцем станет `QTabWidget`.
    QWidget *buildSignalTab();
    /// @brief Строит вкладку распределения: формула eps(x, y, r, f), шаг, область, имя, карта, кнопки.
    /// @return Страница вкладки; владельцем станет `QTabWidget`.
    QWidget *buildEpsTab();
    /// @brief Показывает поля, нужные выбранной форме, обновляет предпросмотр, сводку
    ///        (`tpl::sigPlan`: шаг записи, число точек, размер; полоса, FREQ согласования, пик)
    ///        и предупреждения (обрезка временем счёта, укрупнение шага, пустая таблица/формула).
    void refreshSignal();
    /// @brief Пересчитывает карту и сводку распределения: из прочитанного файла или по формуле
    ///        (`tpl::epsValues` от выражения `(формула)-1`), с предупреждением о eps <= 0.
    void refreshEps();
    /// @brief Записывает `.sig`: текст `tpl::sigFileText` в CP1251; при потере символов не пишет.
    void saveSignal();
    /// @brief Записывает `.eps` функцией `tpl::epsFileBytes` — из прочитанных чисел (с обратным
    ///        учётом весов узлов) или по формуле.
    void saveEps();
    /// @brief Загружает в таблицу точки из текстового файла в две колонки (`tpl::parseSigPoints`)
    ///        и тоны строки `#FREQ` (`tpl::parseSigTones`), выбирает форму «Своя таблица».
    void loadTable();
    /// Открыть готовый `.eps`: показать его сетку и значения.
    /// @brief Читает шапку и значения чужого `.eps` (`tpl::readEpsHeader`, `tpl::readEpsValues`)
    ///        и приводит поля шага и области к прочитанной сетке.
    void openEpsFile();
    /// Таблица -> m_points. Строки без двух чисел пропускаются.
    /// @brief Переносит строки таблицы в `m_points` (запятая допускается, точки сортируются по времени).
    void pullTable();
    /// m_points -> таблица.
    /// @brief Заполняет таблицу точками `m_points` (10 значащих цифр).
    void pushTable();
    /// Заполнить таблицу текущей формой: взять её точки и дальше править руками.
    /// @brief Превращает выбранную форму в точки: 20 на период верхней частоты, не меньше 400
    ///        и не больше 50000; тоны формы переходят в `m_tableTones`.
    void tableFromCurrentShape();
    /// Тонов в принесённом файле больше 99 — показать, что возьмётся. true — показано.
    /// @brief Предупреждает окном, если тонов больше `tpl::kMaxFreqs` (`tpl::toneCutNotice`).
    /// @return true — окно предупреждения было показано.
    bool warnToneCut();
    /// @brief Строит задание-носитель с одним активным портом и сигналом из файла.
    /// @return Задание: имя файла, DELTA (мм), TIME (нс), коробка, порт с формой `waveshape()`,
    ///         частота согласования `tpl::suggestedFreq` (ГГц, иначе 1).
    tpl::Project signalProject() const;
    /// @brief Возвращает сетку распределения по полям шага и области (`tpl::epsGridFor`).
    /// @return Сетка; при флажке «Начало координат в центре» область симметрична нулю.
    tpl::EpsGrid epsGrid() const;

    /// Задание, из которого взяты умолчания (копия).
    tpl::Project m_source;

    /// Вкладки окна: 0 — сигнал, 1 — распределение.
    QTabWidget *m_tabs = nullptr;

    // --- сигнал ---
    /// Галерея форм сигнала.
    SignalGallery *m_gallery = nullptr;
    /// Предпросмотр a(t) и спектра.
    SignalPreview *m_preview = nullptr;
    /// Амплитуда, относительные единицы.
    QDoubleSpinBox *m_amp = nullptr;
    /// Начало сигнала, нс.
    QDoubleSpinBox *m_start = nullptr;
    /// Длительность посылки, нс.
    QDoubleSpinBox *m_duration = nullptr;
    /// Нижняя частота полосы, несущая или частота повторения, ГГц (подпись меняется по форме).
    QDoubleSpinBox *m_f1 = nullptr;
    /// Верхняя частота полосы, ГГц.
    QDoubleSpinBox *m_f2 = nullptr;
    /// Число тонов гребёнки, 2...`tpl::kMaxFreqs`.
    QSpinBox *m_tones = nullptr;
    /// Число периодов в посылке.
    QSpinBox *m_periods = nullptr;
    /// Ширина импульса, нс.
    QDoubleSpinBox *m_width = nullptr;
    /// Длительность переднего фронта, нс.
    QDoubleSpinBox *m_rise = nullptr;
    /// Длительность заднего фронта, нс.
    QDoubleSpinBox *m_fall = nullptr;
    /// Формула a(t) для формы «выражение».
    QLineEdit *m_expr = nullptr;
    /// Рамка таблицы точек (видна только у формы «Своя таблица»).
    QWidget *m_tableBox = nullptr;
    /// Таблица точек: время (нс) и значение a.
    QTableWidget *m_table = nullptr;
    /// Сводка по таблице: число точек, интервал, тоны `#FREQ`.
    QLabel *m_tableInfo = nullptr;
    /// Кнопка «Загрузить из файла…».
    QPushButton *m_tableLoad = nullptr;
    /// Кнопка «Взять из формы».
    QPushButton *m_tableFromShape = nullptr;
    /// true — таблицу заполняет программа; правки ячеек в это время не переносятся в модель.
    bool m_fillingTable = false;
    /// Шаг сетки DELTA задания, под которое пишется файл, мм.
    QDoubleSpinBox *m_delta = nullptr;
    /// Время счёта TIME, нс.
    QDoubleSpinBox *m_time = nullptr;
    /// Имя файла `.sig` без расширения.
    QLineEdit *m_sigName = nullptr;
    /// Сводка «что получится»: шаг записи, точки, размер, полоса, FREQ, пик.
    QLabel *m_sigSummary = nullptr;
    /// Предупреждения по сигналу.
    QLabel *m_sigWarn = nullptr;
    /// Точки своей таблицы: время (нс) и значение a.
    QVector<QPointF> m_points;
    /// Строка `#FREQ` принесённого файла: многотон, посчитанный на стороне, им и остаётся.
    QVector<double> m_tableTones;
    /// Подпись строки формы по её полю: прячется вместе с полем.
    QHash<QWidget *, QWidget *> m_rowLabel;

    // --- распределение ---
    /// Формула eps(x, y, r, f) — сама eps, а не добавка; x, y — мм, f — полярный угол.
    QLineEdit *m_epsFormula = nullptr;
    /// Шаг сетки распределения, мм.
    QDoubleSpinBox *m_epsDelta = nullptr;
    /// Левая граница области распределения X, мм.
    QDoubleSpinBox *m_epsX0 = nullptr;
    /// Правая граница области распределения X, мм.
    QDoubleSpinBox *m_epsX1 = nullptr;
    /// Нижняя граница области распределения Y, мм.
    QDoubleSpinBox *m_epsY0 = nullptr;
    /// Верхняя граница области распределения Y, мм.
    QDoubleSpinBox *m_epsY1 = nullptr;
    /// Флажок «Начало координат в центре области».
    QCheckBox *m_epsCentered = nullptr;
    /// Имя файла `.eps` без расширения.
    QLineEdit *m_epsName = nullptr;
    /// Карта распределения.
    EpsMapView *m_epsMap = nullptr;
    /// Сводка: сетка, область, диапазон eps, размер файла.
    QLabel *m_epsSummary = nullptr;
    /// Предупреждения и ошибки формулы.
    QLabel *m_epsWarn = nullptr;
    /// Значения, прочитанные из чужого файла. Непусто — карта показывает ИХ, а не
    /// формулу: у готового файла формулы нет и быть не может.
    QVector<double> m_epsLoaded;
    /// Сетка прочитанного файла.
    tpl::EpsGrid m_epsLoadedGrid;
    /// Подпись «открыт ИМЯ» у прочитанного файла.
    QLabel *m_epsSource = nullptr;
    /// Кнопка «Вернуться к формуле» (видна, пока показан прочитанный файл).
    QPushButton *m_epsDrop = nullptr;
};
