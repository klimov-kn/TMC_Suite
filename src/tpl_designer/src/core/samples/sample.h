/// @file sample.h
/// @brief Каталог образцов задач: описание образца, его параметров, паспорта и рецепта.
///
/// Часть модуля «Ядро TPL Designer». Объявляет типы каталога (`Param`, `Values`, `Row`,
/// `Build`, `Section`, `Sample`, `Baked`), доступ к каталогу (`sections`, `all`, `find`,
/// `inSection`), сборку задания по рецепту (`make`) и функции регистрации разделов,
/// которые реализованы в `waveguides.cpp`, `antennas.cpp`, `microstrip*.cpp`, `media.cpp`,
/// `lessons.cpp`. Таблица чисел ядра — `baked.cpp`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#samples
#pragma once

#include "project.h"

#include <QHash>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

#include <functional>

/// Библиотека образцов: готовые СВЧ-устройства и задачи, которые строятся по параметрам.
///
/// Образец — это РЕЦЕПТ, а не файл: функция, которая по выбранному сечению, частоте и
/// размерам собирает `tpl::Project`. Так любой выбор даёт гарантированно корректное
/// задание (его проверяет тот же `validate()`, что и мастер), а тридцать сечений
/// волновода — это тридцать строк таблицы, а не тридцать файлов.
///
/// Здесь нет ни одного виджета: рецепты тестируются без GUI, как и весь `tpl_core`.
namespace tpl::samples {

/// Поле карточки образца.
///
/// @brief Описание одного настраиваемого параметра рецепта (поле ввода на карточке).
///
/// Создаётся фабричными функциями `number`, `integer`, `choice`, `flag` при регистрации
/// образца и хранится в `Sample::params`. Значение параметра всегда передаётся рецепту
/// числом `double` в `Values` под ключом `key`.
struct Param {
    /// @brief Вид поля ввода.
    ///
    /// `Number` — вещественное число с единицей `unit` и `decimals` знаками;
    /// `Integer` — целое (при сборке округляется); `Choice` — выбор из списка `choices`,
    /// значение — номер строки от 0; `Flag` — флажок, значение 0 или 1.
    enum Kind { Number, Integer, Choice, Flag };
    /// @var Kind Number
    /// Вещественное число с единицей `unit`.
    /// @var Kind Integer
    /// Целое число (при сборке округляется).
    /// @var Kind Choice
    /// Выбор из списка `choices`; значение — номер строки от 0.
    /// @var Kind Flag
    /// Флажок; значение 0 или 1.

    /// Ключ параметра в `Values` (латиницей; по нему рецепт читает значение).
    QString key;
    /// Подпись поля на карточке (переведённая строка `tpl::tr`).
    QString label;
    QString unit;   ///< «мм», «ГГц», «град.» — показывается рядом с полем
    QString hint;   ///< подсказка под курсором
    /// Вид поля.
    Kind kind = Number;
    /// Значение по умолчанию; для `Choice` — номер строки, для `Flag` — 0 или 1.
    double def = 0.0;
    /// Нижняя граница; `make` прижимает значение к [min; max], если max > min.
    double min = 0.0;
    /// Верхняя граница; для `Choice` — число строк минус 1, для `Flag` — 1.
    double max = 0.0;
    /// Шаг изменения поля стрелками (в единицах `unit`).
    double step = 1.0;
    /// Число знаков после точки в поле (у `Integer` — 0).
    int decimals = 2;
    QStringList choices; ///< для Choice; значение — номер строки

    /// @brief Создаёт вещественный параметр.
    /// @param[in] key       ключ в `Values`
    /// @param[in] label     подпись поля
    /// @param[in] unit      единица, показываемая рядом с полем («мм», «ГГц»)
    /// @param[in] def       значение по умолчанию, в единицах `unit`
    /// @param[in] min       нижняя граница
    /// @param[in] max       верхняя граница
    /// @param[in] step      шаг изменения стрелками
    /// @param[in] decimals  знаков после точки
    /// @param[in] hint      всплывающая подсказка (может быть пустой)
    /// @return Заполненный `Param` вида `Number`.
    static Param number(const QString &key, const QString &label, const QString &unit,
                        double def, double min, double max, double step, int decimals,
                        const QString &hint = {});
    /// @brief Создаёт целочисленный параметр (шаг 1, без знаков после точки).
    /// @param[in] key    ключ в `Values`
    /// @param[in] label  подпись поля
    /// @param[in] def    значение по умолчанию
    /// @param[in] min    нижняя граница
    /// @param[in] max    верхняя граница
    /// @param[in] hint   всплывающая подсказка
    /// @return Заполненный `Param` вида `Integer`.
    static Param integer(const QString &key, const QString &label, int def, int min, int max,
                         const QString &hint = {});
    /// @brief Создаёт параметр-выбор из списка; границы — 0 и `items.size()-1`.
    /// @param[in] key    ключ в `Values`
    /// @param[in] label  подпись поля
    /// @param[in] items  строки списка (переведённые)
    /// @param[in] def    номер строки по умолчанию, от 0
    /// @param[in] hint   всплывающая подсказка
    /// @return Заполненный `Param` вида `Choice`.
    static Param choice(const QString &key, const QString &label, const QStringList &items,
                        int def, const QString &hint = {});
    /// @brief Создаёт параметр-флажок (значения 0 и 1).
    /// @param[in] key    ключ в `Values`
    /// @param[in] label  подпись флажка
    /// @param[in] def    состояние по умолчанию
    /// @param[in] hint   всплывающая подсказка
    /// @return Заполненный `Param` вида `Flag`.
    static Param flag(const QString &key, const QString &label, bool def,
                      const QString &hint = {});
};

/// Значения параметров. Выбор — номер строки, флаг — 0 или 1.
using Values = QHash<QString, double>;

/// Строка паспорта: «Отсечка H10 — 6.56 ГГц».
struct Row {
    /// Название величины («Отсечка H10»).
    QString label;
    /// Значение с единицей («6.56 ГГц»), уже отформатированное для показа.
    QString value;
};

/// Что даёт рецепт.
///
/// @brief Результат рецепта: готовое задание, паспорт теории, пояснения и файлы-спутники.
///
/// Возвращается `make()`; задание пишется на диск только через `tpl::writeTask`, который
/// кладёт рядом и `companions`.
struct Build {
    /// Собранное задание; при умолчаниях обязано проходить `validate()` без ошибок
    /// и без потерь CP1251.
    Project project;
    /// Теория: то, что считается формулами при ЛЮБЫХ параметрах.
    QVector<Row> theory;
    /// Пояснения к постановке — что именно сделано и на что смотреть после счёта.
    QStringList notes;
    /// Файлы, которые обязаны лечь РЯДОМ с заданием (готовое распределение `.eps`).
    /// Имя — без пути; содержимое — байты как есть.
    QMap<QString, QByteArray> companions;
};

/// Раздел каталога.
struct Section {
    /// Идентификатор раздела латиницей: `waveguide`, `antenna`, `microstrip`, `media`,
    /// `lesson`; на него ссылается `Sample::section`.
    QString id;
    /// Название раздела (переведённое).
    QString title;
    /// Одна строка о содержимом раздела.
    QString summary;
};

/// Образец.
///
/// @brief Запись каталога: описание образца, его параметры и рецепт сборки задания.
///
/// Записи создают функции `add*()` разделов; живут в статических таблицах каталога
/// (по одной на язык), указатели на них, выданные `find()`/`inSection()`, действительны
/// до конца работы программы.
struct Sample {
    QString id;       ///< латиницей, уникален: по нему живут числа ядра и тесты
    QString section;  ///< id раздела
    /// Название образца для плитки и карточки.
    QString title;
    QString summary;  ///< одна строка для плитки каталога
    QString standard; ///< стандарт или источник: «ГОСТ 20900-75, IEC 60153-2, EIA RS-261»
    /// Справка: что это, как устроено, что смотреть в результатах. Пишется так, чтобы
    /// стать сценарием урока.
    QString help;
    /// Ядро, которым считается образец (H или X).
    Kernel kernel = Kernel::H;
    /// Параметры карточки в порядке показа.
    QVector<Param> params;
    /// Рецепт: по полному набору значений (с умолчаниями) собирает задание.
    /// Вызывается только через `make()`, который дополняет и прижимает значения.
    std::function<Build(const Values &)> build;
};

/// Посчитанное ядром для параметров по умолчанию.
struct Baked {
    /// `Sample::id` образца, к которому относятся числа.
    QString sampleId;
    QString when;          ///< дата прогона
    QString setup;         ///< сетка и время: «сетка 263x48, шаг 0.57 мм, 21 частота»
    QVector<Row> rows;     ///< числа из .s
};

/// @brief Возвращает разделы каталога на текущем языке интерфейса.
/// @return Ссылка на статическую таблицу (своя на каждый язык; живёт до конца программы).
const QVector<Section> &sections();
/// @brief Возвращает все образцы каталога на текущем языке интерфейса.
/// @return Ссылка на статическую таблицу всех разделов подряд (своя на каждый язык).
const QVector<Sample> &all();
/// @brief Ищет образец по идентификатору.
/// @param[in] id  `Sample::id`
/// @return Указатель в таблицу `all()`; `nullptr` — такого образца нет.
const Sample *find(const QString &id);
/// @brief Возвращает образцы одного раздела в порядке регистрации.
/// @param[in] sectionId  `Section::id`
/// @return Указатели в таблицу `all()`; пустой список — раздела нет или он пуст.
QVector<const Sample *> inSection(const QString &sectionId);

/// Значения по умолчанию.
/// @param[in] s  образец
/// @return Набор «ключ -> `Param::def`» для всех параметров образца.
Values defaults(const Sample &s);
/// Равны ли значения умолчаниям — только тогда числа ядра относятся к карточке.
/// @param[in] s  образец
/// @param[in] v  значения карточки; отсутствующий ключ считается равным умолчанию
/// @return true, если каждое значение совпадает с умолчанием с относительной
///         точностью 1e-9.
bool isDefault(const Sample &s, const Values &v);

/// Числа ядра для образца; nullptr — не считали.
/// @param[in] sampleId  `Sample::id`
/// @return Указатель в таблицу `bakedTable()` или `nullptr`.
const Baked *baked(const QString &sampleId);

/// Собрать образец. Параметры, которых нет в \p v, берутся по умолчанию.
///
/// Значения прижимаются к [min; max] параметра, у `Integer`/`Choice`/`Flag`
/// округляются; пустое описание задания заменяется названием образца.
/// @param[in] s  образец
/// @param[in] v  значения карточки (единицы — как у `Param::unit`)
/// @return Результат рецепта `s.build`.
Build make(const Sample &s, const Values &v);

// --- внутреннее: разделы регистрируют свои рецепты ---
/// @name Регистрация разделов каталога
/// Каждая функция дописывает в \p out образцы своего раздела; вызываются один раз
/// при построении таблицы `all()` (на каждый язык).
///@{
/// @brief Раздел «Волноводы и узлы» (`waveguides.cpp`).
/// @param[in,out] out  таблица каталога
void addWaveguides(QVector<Sample> *out);
/// @brief Раздел «Антенны» (`antennas.cpp`).
/// @param[in,out] out  таблица каталога
void addAntennas(QVector<Sample> *out);
/// @brief Раздел «Микрополосковые», первая половина (`microstrip.cpp`); вызывает
/// `addMicrostrip2`.
/// @param[in,out] out  таблица каталога
void addMicrostrip(QVector<Sample> *out);
/// Вторая половина раздела «Микрополосковые»: изгиб, делитель, нагрузка, разомкнутый
/// конец, импульсная рефлектометрия, линия задержки, тейпер, патч, ПЗФ на шлейфах.
/// Отдельным файлом только из-за размера — раздел один, и регистрирует его
/// \ref addMicrostrip.
/// @param[in,out] out  таблица каталога
void addMicrostrip2(QVector<Sample> *out);
/// @brief Раздел «Среды, плазма, ЭПР» (`media.cpp`).
/// @param[in,out] out  таблица каталога
void addMedia(QVector<Sample> *out);
/// @brief Раздел «Учебные задания»: авторские задания из ресурсов (`lessons.cpp`).
/// @param[in,out] out  таблица каталога
void addLessons(QVector<Sample> *out);
///@}
/// @brief Возвращает таблицу чисел, посчитанных ядром для образцов при умолчаниях.
///
/// Реализована в сгенерированном файле `baked.cpp`; таблица своя на каждый язык.
/// @return Ссылка на статическую таблицу; порядок — по `Sample::id`.
const QVector<Baked> &bakedTable();

} // namespace tpl::samples
