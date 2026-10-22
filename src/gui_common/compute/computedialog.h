// computedialog.h — окно «Compute» (меню Settings → Compute…).
//
// При открытии в фоне ищет видеокарты и проверяет каждую карту с double
// самотестом (tmccompute::selfTest); окно при этом не замирает. Флажок
// «Use the video card for calculations» доступен, только когда есть карта,
// прошедшая все проверки. OK сохраняет выбор в Compute.ini и сразу применяет его
// (tmcApplyComputeSettings), так что следующий расчёт в этом же процессе уже
// пойдёт по новой настройке.
//
// Использование:
//     ComputeDialog dlg(this);
//     if (dlg.exec() == QDialog::Accepted)
//         statusBar()->showMessage(tmcComputeStatusText());
//
// Глобальную тему окно не трогает: собственная таблица стилей задаёт только
// шрифты окна, цвета отметок и рамки своих панелей.

/// @file computedialog.h
/// @brief Окно «Compute» ядер H и X (меню Config -> Compute (GPU)...): видеокарты, их проверка и выбор расчёта.
///
/// Часть модуля «Расчёт на видеокарте» (библиотека `tmc_compute`, Qt6::Widgets).
/// @see https://www.tamic.ru/docs/code/gpu.html#compute-dialog

#pragma once

#include "tmccompute.h"

#include <QDialog>
#include <QFutureWatcher>
#include <QList>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLabel;
class QProgressBar;
class QPushButton;
class QTreeWidget;
QT_END_NAMESPACE

/// @brief Модальное окно настройки расчёта на видеокарте.
///
/// При создании читает `Compute.ini` и сразу запускает проверку в фоне: перечень карт
/// (`enumerateDevices`), затем самотест каждой карты с двойной точностью (`selfTest`) —
/// через QtConcurrent, окно не замирает. Флажок «Use the video card for calculations»
/// доступен, только если хоть одна карта прошла проверку; список карт виден, если
/// таких карт две и больше. Если карту ещё не выбирали, выбирается самая быстрая по замеру.
/// OK сохраняет выбор и итог проверки и вызывает `tmcApplyComputeSettings`.
/// Создаётся на стеке обработчиком меню `MainWindow::onComputeSettings`.
/// @note Потоки: виджеты — только в потоке интерфейса; фоновая проверка не обращается к окну.
/// @see https://www.tamic.ru/docs/code/gpu.html#compute-dialog
class ComputeDialog : public QDialog
{
    Q_OBJECT
public:
    /// @brief Строит окно, читает настройки и запускает проверку.
    /// @param[in] parent  родительское окно (обычно главное окно ядра)
    explicit ComputeDialog(QWidget *parent = nullptr);
    /// @brief Закрывает окно; незаконченная фоновая проверка дорабатывает сама.
    ~ComputeDialog() override;

    /// @brief Сообщает, идёт ли проверка карт.
    /// @return `true` — проверка идёт.
    bool isChecking() const { return m_checking; }

    // Итог последней проверки в этом окне (пустой, пока проверка идёт).
    /// @brief Возвращает итог последней проверки в этом окне.
    /// @return Ссылка на отчёт (пустой, пока проверка идёт); действительна, пока живо окно.
    const tmccompute::CheckReport &report() const { return m_report; }

signals:
    /// Проверка закончена (успешно или нет); окно уже обновлено.
    void checkFinished();

public slots:
    /// @brief Запускает поиск карт и самотесты в фоне (кнопка «Check again»; при открытии — сама). Во время проверки ничего не делает.
    void startCheck();     // «Check again»; при открытии вызывается сам
    /// @brief Сохраняет выбор (флажок, карта, итог проверки) в Compute.ini, применяет его и закрывает окно. Во время проверки не срабатывает.
    void accept() override;

private slots:
    /// @brief Принимает перечень карт; при неполадке показывает диагноз и заканчивает проверку, иначе запускает самотесты.
    void onInventoryReady();
    /// @brief Принимает результаты самотестов, пишет итог в строку хода, выбирает карту, обновляет таблицу и элементы.
    void onTestsReady();
    /// @brief Показывает подробности выбранной в таблице карты и выбирает её в списке, если она годна.
    void onCurrentRowChanged();
    /// @brief Запоминает карту, выбранную в списке, и выделяет её строку в таблице.
    /// @param[in] index  номер элемента списка; < 0 — ничего не выбрано
    void onDeviceComboChanged(int index);
    /// @brief Включает и показывает элементы по состоянию проверки (флажок, список карт, кнопки).
    void updateControls();

private:
    /// @brief Заполняет таблицу карт (имя, память, двойная точность, самотест, драйвер) и подбирает её высоту.
    void fillTable();
    /// @brief Заполняет список карт, прошедших проверку, и выбирает в нём сохранённую карту.
    void fillDeviceCombo();
    /// @brief Заполняет панель подробностей для карты, выбранной в таблице.
    void updateDetails();
    /// @brief Показывает строку «Saved: ...» — что сохранено в Compute.ini и итог последней проверки.
    void updateLastCheckLabel();
    /// @brief Собирает номера карт, прошедших самотест.
    /// @return Номера карт среди карт с двойной точностью; пусто, пока проверка не окончена.
    QList<int> passedGpuIndices() const;
    /// @brief Выбирает самую быструю по замеру карту среди прошедших проверку.
    /// @return Номер карты с наименьшим временем на узел (без замера — первая прошедшая); -1 — таких нет.
    int fastestPassedGpu() const;   // карта, прошедшая проверку, с наименьшим временем на узел; -1 — нет

    /// Настройки, прочитанные при открытии; `device` меняется выбором в окне.
    tmccompute::Settings  m_settings;
    /// Итог проверки в этом окне.
    tmccompute::CheckReport m_report;
    /// Проверка идёт.
    bool m_checking = false;
    /// Самотесты закончены (результаты в `m_report.tests` окончательные).
    bool m_testsDone = false;

    /// Ожидание фонового перечня карт.
    QFutureWatcher<tmccompute::Inventory> m_invWatch;
    /// Ожидание фоновых самотестов.
    QFutureWatcher<QList<tmccompute::SelfTestResult>> m_testWatch;

    /// Строка хода проверки (элемент окна, принадлежит окну).
    QLabel       *m_status = nullptr;
    /// Бегущая полоса, видна во время проверки.
    QProgressBar *m_busy = nullptr;
    /// Панель диагноза (красная рамка), видна при неполадке.
    QLabel       *m_diag = nullptr;
    /// Таблица видеокарт.
    QTreeWidget  *m_table = nullptr;
    /// Панель подробностей о выбранной карте.
    QLabel       *m_details = nullptr;
    /// Флажок «Use the video card for calculations».
    QCheckBox    *m_useGpu = nullptr;
    /// Подпись «Video card:» списка карт.
    QLabel       *m_deviceLabel = nullptr;
    /// Список карт, прошедших проверку (данные элемента — `gpuIndex`).
    QComboBox    *m_device = nullptr;
    /// Строка «Saved: ...» о сохранённой настройке.
    QLabel       *m_lastCheck = nullptr;
    /// Кнопка «Check again».
    QPushButton  *m_recheck = nullptr;
    /// Кнопки OK и Cancel.
    QDialogButtonBox *m_buttons = nullptr;
};
