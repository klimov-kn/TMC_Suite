// tmccompute.h — настройка расчёта на видеокарте (без виджетов, Qt6::Core).
//
// Что здесь есть:
//   * перечисление видеокарт через OpenCL (тот же порядок, что у ядра:
//     TmcOclListGpus, затем только карты с double);
//   * диагноз «чего не хватает» — нет OpenCL.dll, нет платформ, нет карт,
//     нет карт с double — с понятным текстом, что установить;
//   * самотест карты (~0,1–1 с): шаг H-схемы на маленькой сетке на карте и на
//     процессоре, результат должен совпасть побитно; по желанию — замер скорости;
//   * настройки в %APPDATA%\TMC_Suite_Qt\Compute.ini (общие для H и X);
//   * tmcApplyComputeSettings() — вызвать в main ядра до запуска расчёта;
//   * tmcComputeStatusText() — текст для строки состояния.
//
// Номер карты (Settings::device, DeviceInfo::gpuIndex) — это номер среди карт
// с double, ровно тот, что понимает ядро: TMC_GPU = номер + 1, ключ /GPU:номер.

/// @file tmccompute.h
/// @brief Настройка расчёта на видеокарте без виджетов: перечень карт, диагноз, самотест, Compute.ini, ключи /GPU и /CPU.
///
/// Часть модуля «Расчёт на видеокарте» (библиотека `tmc_compute`, Qt6::Core). Не
/// считает задания сама: ядро решает «карта или процессор» в `CTmcGpuH::Begin` /
/// `CTmcGpuX::Begin`, а этот модуль готовит для него переменную окружения `TMC_GPU`
/// по настройке пользователя (`tmcApplyComputeSettings`) и проверяет, годится ли
/// карта (`selfTest`). Окно — `computedialog.h`.
/// @see https://www.tamic.ru/docs/code/gpu.html#compute-settings

#pragma once

#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringList>

/// Пространство имён модуля настройки расчёта на видеокарте.
namespace tmccompute {

// Состояние OpenCL на компьютере.
/// @brief Итог поиска OpenCL и видеокарт на компьютере (что мешает считать на карте).
/// @see https://www.tamic.ru/docs/code/gpu.html#diagnosis
enum class OclState {
    /// Есть хотя бы одна видеокарта с двойной точностью.
    Ok,           // есть хотя бы одна видеокарта с double
    /// Сборка не умеет считать на карте (не Windows или 32 бита).
    Unsupported,  // сборка не умеет считать на карте (не Windows или 32 бита)
    /// В системном каталоге нет OpenCL.dll.
    NoLibrary,    // нет OpenCL.dll
    /// OpenCL.dll есть, но без функций OpenCL 1.2.
    OldLibrary,   // OpenCL.dll есть, но без функций OpenCL 1.2
    /// OpenCL.dll есть, но ни один драйвер OpenCL не зарегистрирован.
    NoPlatforms,  // OpenCL.dll есть, но ни один драйвер OpenCL не зарегистрирован
    /// Драйверы OpenCL есть, видеокарт среди устройств нет.
    NoGpu,        // драйверы OpenCL есть, видеокарт среди устройств нет
    /// Видеокарты есть, но ни одна не умеет `double` (`cl_khr_fp64`).
    NoDoubleGpu   // видеокарты есть, но ни одна не умеет double (cl_khr_fp64)
};

// Одна видеокарта.
/// @brief Сведения об одной видеокарте для окна и отчёта (копия `TmcOclDevice` в типах Qt).
struct DeviceInfo {
    /// Номер среди всех видеокарт (порядок: платформа, устройство), с 0.
    int     listIndex = -1;   // номер среди всех видеокарт (порядок платформа, устройство)
    /// Номер среди карт с двойной точностью — его понимает ядро (`/GPU:n`, `TMC_GPU` = n + 1); -1 — карта без `double`.
    int     gpuIndex  = -1;   // номер среди карт с double — его понимает ядро; -1 — карта без double
    /// Имя платформы OpenCL (драйвера производителя).
    QString platform;         // имя платформы OpenCL (NVIDIA CUDA, AMD Accelerated Parallel Processing…)
    /// Имя карты.
    QString name;             // имя карты
    /// Производитель.
    QString vendor;           // производитель
    /// Версия драйвера (CL_DRIVER_VERSION).
    QString driver;           // версия драйвера (CL_DRIVER_VERSION)
    /// Версия OpenCL устройства (CL_DEVICE_VERSION).
    QString openclVersion;    // версия OpenCL устройства (CL_DEVICE_VERSION)
    /// Память карты, байт.
    quint64 globalMemBytes = 0;  // память карты
    /// Наибольший размер одного буфера, байт.
    quint64 maxAllocBytes  = 0;  // наибольший один буфер (обычно ¼ памяти)
    /// Число вычислительных блоков.
    uint    computeUnits   = 0;  // вычислительные блоки
    /// Наибольшая частота, МГц.
    uint    clockMHz       = 0;  // частота, МГц
    /// `true` — карта умеет `double`; только такие карты годятся для расчёта.
    bool    fp64 = false;        // поддержка double (cl_khr_fp64 или CL_DEVICE_DOUBLE_FP_CONFIG)
};

// Итог перечисления.
/// @brief Итог перечисления видеокарт (`enumerateDevices`).
struct Inventory {
    /// Состояние OpenCL; `Ok` — есть карта с двойной точностью.
    OclState state = OclState::NoLibrary;
    /// Число платформ OpenCL (драйверов); 0 — нет ни одной.
    int platformCount = 0;
    /// Все видеокарты, включая карты без `double`, в порядке `TmcOclListGpus`.
    QList<DeviceInfo> devices;   // все видеокарты, включая карты без double
    /// Пусто при `Ok`; иначе — что не так и что установить (по-английски, HTML со ссылками на драйверы).
    QString diagnosis;           // пусто при Ok; иначе — что не так и что установить (англ., HTML со ссылками)

    /// @brief Считает карты с двойной точностью в `devices`.
    /// @return Число карт, годных для расчёта по признаку `fp64`.
    int usableCount() const;     // карт с double
};

// Перечислить видеокарты. Безопасно на машине без OpenCL (вернёт диагноз).
// Можно вызывать из любого потока; первый вызов может длиться до ~0,5 с
// (драйвер инициализирует платформу).
/// @brief Перечисляет видеокарты OpenCL и ставит диагноз, если считать на карте нельзя.
///
/// Безопасна на машине без OpenCL. Переменная окружения `TMC_COMPUTE_SIMULATE`
/// (`NoLibrary`, `NoPlatforms`, `NoGpu`, `NoDoubleGpu`) изображает соответствующую поломку
/// для проверки окна.
/// @return Итог: состояние, список карт, диагноз.
/// @note Потоки: можно вызывать из любого потока; загрузка OpenCL.dll защищена мьютексом.
/// Первый вызов может длиться до долей секунды.
/// @see https://www.tamic.ru/docs/code/gpu.html#diagnosis
Inventory enumerateDevices();

// Результат самотеста одной карты.
/// @brief Результат самотеста одной карты (`selfTest`).
/// @see https://www.tamic.ru/docs/code/gpu.html#selftest
struct SelfTestResult {
    /// Тест запускался (карта найдена и умеет `double`).
    bool    ran = false;          // тест запускался (карта найдена и умеет double)
    /// Все проверки пройдены: карту можно использовать для расчёта.
    bool    passed = false;       // все шаги пройдены
    /// Номер карты среди карт с двойной точностью; -1 — не задан.
    int     gpuIndex = -1;
    /// Имя карты.
    QString deviceName;
    /// Версия драйвера.
    QString driver;

    /// Созданы контекст и очередь OpenCL.
    bool    contextOk = false;    // созданы контекст и очередь
    /// Драйвер скомпилировал пробную программу OpenCL C с `double`.
    bool    compileOk = false;    // драйвер скомпилировал программу (компилятор OpenCL C на месте)
    /// Результат карты совпал с процессором побитно.
    bool    bitExact  = false;    // результат карты совпал с процессором побитно
    /// Наибольшее абсолютное расхождение карты и процессора (0 при побитном совпадении).
    double  maxAbsDiff = 0.0;     // наибольшее расхождение (0 при побитном совпадении)
    /// Что сломалось (по-английски); пусто при успехе.
    QString error;                // что сломалось (англ.), пусто при успехе
    /// Журнал компиляции (до 4000 символов), если она не удалась.
    QString buildLog;             // журнал компиляции, если она не удалась

    /// Замер скорости выполнен (поля ниже заполнены).
    bool    speedMeasured = false;
    /// Сторона квадратной сетки замера, узлов (2048); 0 — замера не было.
    int     speedGrid = 0;        // сторона квадратной сетки замера
    /// Время на карте, нс на узел за такт (такт = два подтакта схемы H).
    double  gpuNsPerNode = 0.0;   // нс на узел за такт (такт = два прохода) на карте
    /// То же на процессоре во `cpuThreads` потоках, нс на узел за такт.
    double  cpuNsPerNode = 0.0;   // то же на процессоре
    /// Число потоков процессора в замере.
    int     cpuThreads = 0;       // сколько потоков процессора в замере
    /// Пропускная способность памяти карты при копировании (чтение + запись), ГБ/с.
    double  bandwidthGBs = 0.0;   // пропускная способность памяти карты (копирование), ГБ/с

    /// Длительность всего теста, мс.
    double  elapsedMs = 0.0;      // сколько длился весь тест
};

// Самотест карты номер gpuIndex (среди карт с double).
// Сетка 301×203, 200 тактов H-схемы на карте и на процессоре — побитная сверка
// (FP_CONTRACT OFF на карте, -ffp-contract=off на процессоре). measureSpeed —
// добавить короткий замер скорости (сетка 2048×2048, ~0,2 с).
// Каждый запуск ядра на карте — доли миллисекунды: сторож драйвера (TDR, ~2 с)
// не срабатывает.
/// @brief Проверяет, годится ли карта для расчёта: контекст, компиляция, побитное совпадение с процессором, скорость.
///
/// Шаги: контекст и очередь OpenCL; компиляция пробной программы (та же формула узла
/// H, что в ядре, `FP_CONTRACT OFF`); 200 тактов схемы H на сетке 301 x 203 на карте и
/// на процессоре (процессорная часть собрана без слияния умножения и сложения) и
/// побитное сравнение; по желанию — замер скорости на сетке 2048 x 2048 и пропускной
/// способности памяти. Каждый запуск ядра на карте короче миллисекунды — сторож
/// драйвера (TDR) не срабатывает. `TMC_COMPUTE_SIMULATE=FailTest` изображает провал сверки.
/// @param[in] gpuIndex      номер карты среди карт с двойной точностью, с 0
/// @param[in] measureSpeed  `true` — добавить замер скорости (~0,2 с)
/// @return Результат; `passed` == `true` — карту можно использовать.
/// @note Потоки: можно вызывать из рабочего потока (окно так и делает).
/// @see https://www.tamic.ru/docs/code/gpu.html#selftest
SelfTestResult selfTest(int gpuIndex, bool measureSpeed = true);

// Полная проверка: перечисление + самотест каждой карты с double.
/// @brief Итог полной проверки: перечень карт и самотест каждой.
struct CheckReport {
    /// Перечень карт и диагноз.
    Inventory inventory;
    /// Результаты самотестов, по одному на элемент `inventory.devices`; у карт без `double` `ran` == `false`.
    QList<SelfTestResult> tests;   // параллельно inventory.devices; у карт без double ran == false
    /// Время окончания проверки.
    QDateTime when;

    /// @brief Ищет результат самотеста карты по её номеру среди карт с двойной точностью.
    /// @param[in] gpuIndex  номер карты
    /// @return Указатель на результат внутри `tests` (действителен, пока жив отчёт); `nullptr` — тест не запускался.
    const SelfTestResult *testFor(int gpuIndex) const;
};
/// @brief Выполняет полную проверку: `enumerateDevices` и `selfTest` каждой карты с двойной точностью.
/// @param[in] measureSpeed  `true` — замерять скорость каждой карты
/// @return Отчёт проверки.
CheckReport runFullCheck(bool measureSpeed = true);

// Текст проверки для консоли (ASCII, по-английски).
/// @brief Строит текст отчёта проверки для консоли (по-английски, ссылки развёрнуты в адреса).
/// @param[in] report  отчёт `runFullCheck`
/// @return Многострочный текст.
QString reportText(const CheckReport &report);

// Настройки (QSettings, INI, файл settingsFilePath()).
/// @brief Настройка расчёта на видеокарте, общая для ядер H и X (файл `Compute.ini`).
/// @see https://www.tamic.ru/docs/code/gpu.html#compute-settings
struct Settings {
    /// [Compute] UseGpu — пользователь включил расчёт на видеокарте.
    bool      useGpu = false;  // [Compute] UseGpu — считать на видеокарте
    /// [Compute] Device — номер карты среди карт с двойной точностью, >= 0.
    int       device = 0;      // [Compute] Device — номер карты среди карт с double
    /// `true` — Device уже записан в файл; иначе окно выбирает самую быструю карту само.
    bool      deviceSaved = false;  // Device уже записан в файл (иначе карту выбираем сами — самую быструю)
    // результат последней проверки выбранной карты
    /// [LastCheck] Name — имя карты последней сохранённой проверки; пусто — карты не было.
    QString   checkedName;     // [LastCheck] Name
    /// [LastCheck] Driver — версия драйвера при проверке.
    QString   checkedDriver;   // [LastCheck] Driver
    /// [LastCheck] Date — когда была проверка (ISO 8601); недействительна — проверки не было.
    QDateTime checkedAt;       // [LastCheck] Date (ISO 8601)
    /// [LastCheck] Ok — проверка выбранной карты пройдена. Без неё `UseGpu` не включает карту.
    bool      checkOk = false; // [LastCheck] Ok
};

/// @brief Возвращает полное имя файла настроек: `%APPDATA%\TMC_Suite_Qt\Compute.ini`.
/// @return Путь; без APPDATA — в общем каталоге настроек Qt.
QString  settingsFilePath();   // %APPDATA%\TMC_Suite_Qt\Compute.ini
/// @brief Читает настройки из `Compute.ini` (нет файла — значения по умолчанию: процессор).
/// @return Настройки.
Settings loadSettings();
/// @brief Записывает настройки в `Compute.ini`, создавая каталог при необходимости.
/// @param[in] s  настройки
/// @return `true` — записано; `false` — файл не записался.
bool     saveSettings(const Settings &s);   // false — файл не записался

// Записать в s итог проверки карты gpuIndex из report (имя, драйвер, дата, ok).
/// @brief Переносит итог проверки карты в настройки (раздел [LastCheck]).
/// @param[in,out] s         настройки
/// @param[in]     report    отчёт проверки
/// @param[in]     gpuIndex  номер выбранной карты; нет её теста — имя и драйвер очищаются, `checkOk` = `false`
void storeCheckResult(Settings &s, const CheckReport &report, int gpuIndex);

// «15.9 GB», «512 MB»
/// @brief Форматирует размер памяти: от 1 ГБ — «15.9 GB», меньше — «512 MB».
/// @param[in] bytes  размер, байт
/// @return Строка.
QString formatBytes(quint64 bytes);

} // namespace tmccompute

// Вызывается в main ядра ДО запуска расчёта (и повторно — после OK в окне Compute):
//   есть /CPU или -CPU      — снять TMC_GPU (расчёт на процессоре);
//   есть /GPU, /GPU:n, -GPU — окружение не трогать (ядро прочтёт ключ само);
//   TMC_GPU задана снаружи до запуска программы — оставить её;
//   иначе UseGpu и последняя проверка прошла — TMC_GPU = Device + 1;
//   иначе — снять TMC_GPU.
// args — как QCoreApplication::arguments() (имя программы в начале не мешает).
/// @brief Ставит или снимает переменную окружения `TMC_GPU`, по которой ядро решает, запрашивать ли карту.
///
/// Порядок старшинства: ключ `/CPU` (`-CPU`) — снять `TMC_GPU`; ключ `/GPU`, `/GPU:n`
/// (`-GPU`) — не трогать окружение (ядро прочтёт ключ само); `TMC_GPU`, заданная снаружи
/// до запуска программы, — оставить её; иначе `UseGpu` и пройденная последняя проверка —
/// `TMC_GPU = Device + 1`; иначе снять `TMC_GPU`. Внешнее значение запоминается при
/// первом вызове.
/// @param[in] args  аргументы командной строки (как `QCoreApplication::arguments()`)
/// @note Вызывается в `main` оболочки ядра до первого расчёта и после OK в окне Compute.
/// @see https://www.tamic.ru/docs/code/gpu.html#device-choice
void tmcApplyComputeSettings(const QStringList &args);

// Текст для строки состояния: «Compute: GPU — <имя карты>» или «Compute: CPU».
// Смотрит на TMC_GPU и на ключи /GPU в QCoreApplication::arguments().
/// @brief Возвращает текст для строки состояния ядра: «Расчёт: GPU — имя карты» или «Расчёт: CPU» (на языке интерфейса).
///
/// Показывает настройку, а не фактическое решение по шагу: на маленькой сетке или при
/// нехватке памяти ядро всё равно посчитает шаг на процессоре.
/// @return Текст строки состояния.
QString tmcComputeStatusText();
