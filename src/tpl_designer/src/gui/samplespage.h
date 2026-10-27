/// @file samplespage.h
/// @brief Страница библиотеки образцов: разделы, плитки, карточка образца с параметрами и паспортом.
///
/// Часть модуля «Редактор заданий TPL Designer», слой интерфейса. Показывает рецепты
/// каталога `tpl::samples` и «Мои образцы»; задание строит рецепт (`tpl::samples::make`),
/// записывает — `tpl::writeTask`.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-samples
#pragma once

#include "project.h"
#include "sample.h"
#include "usersamples.h"

#include <QByteArray>
#include <QHash>
#include <QMap>
#include <QPixmap>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QFormLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QPushButton;
class QStackedWidget;
class QTimer;
QT_END_NAMESPACE

class GeometryView;

/// Библиотека образцов: каталог готовых устройств и задач (docs/samples-concept.md).
///
/// Слева — разделы и поиск, в середине — плитки с картинками, справа — карточка:
/// картинка, параметры, паспорт (теория и числа, посчитанные ядром), проверка и
/// справка. Образец можно открыть в редакторе или сразу сохранить `.tpl`.
///
/// Экран не знает синтаксиса `.tpl` и физики устройств: задания строят рецепты в
/// tpl_core, здесь их только показывают.
/// @brief Страница стека окна: каталог образцов и карточка выбранного образца.
///
/// Создаётся в `main.cpp` (заново при смене языка), владелец — стек страниц оболочки.
/// Открыть образец в мастере — сигнал `openRequested`; записать задание — прямо отсюда.
/// Работает в потоке интерфейса; картинки плиток рисуются понемногу по таймеру.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-samples
class SamplesPage : public QWidget
{
    Q_OBJECT
public:
    /// @brief Строит шапку, список разделов с поиском, плитки и карточку; читает «Мои образцы».
    /// @param[in] parent  родительский виджет; `nullptr` — без родителя.
    explicit SamplesPage(QWidget *parent = nullptr);

    /// Перечитать «Мои образцы» с диска.
    /// @brief Перечитывает «Мои образцы» (`tpl::samples::userSamples`), перестраивает разделы и плитки,
    ///        сохраняя выделение.
    void reloadUserSamples();

signals:
    /// Вернуться на стартовое окно (кнопка «Назад»).
    void backRequested();
    /// @brief Открыть образец в мастере (кнопка «Открыть в редакторе» или двойной щелчок по плитке).
    /// @param[in] p           задание, построенное рецептом при текущих параметрах.
    /// @param[in] companions  сопутствующие файлы: имя -> байты (например, `lens.eps`).
    void openRequested(const tpl::Project &p, const QMap<QString, QByteArray> &companions);

protected:
    /// @brief При каждом показе страницы перечитывает «Мои образцы» (их могли пополнить из мастера).
    /// @param[in] e  событие показа.
    void showEvent(QShowEvent *e) override;

private:
    /// Плитка каталога: встроенный образец или свой.
    /// @brief Запись каталога для плитки: откуда образец и что о нём показать.
    struct Entry {
        /// true — свой образец из «Моих образцов», false — встроенный рецепт.
        bool user = false;
        QString key;      ///< "b:<id>" или "u:<папка>"
        QString id;       ///< встроенный: id рецепта
        /// Номер в `m_user` для своего образца; -1 — встроенный.
        int userIndex = -1;
        /// Идентификатор раздела (`user` у своих образцов).
        QString section;
        /// Название на плитке и в карточке.
        QString title;
        /// Короткое описание (подсказка плитки, строка карточки).
        QString summary;
        /// Стандарт или «Мои образцы» — третья часть строки сведений.
        QString standard;
        /// Ядро задания (H или X).
        tpl::Kernel kernel = tpl::Kernel::H;
    };

    /// @brief Составляет `m_entries`: все встроенные рецепты (`tpl::samples::all`), затем свои
    ///        образцы; забывает картинки своих образцов и текущий выбор.
    void rebuildEntries();
    /// @brief Заполняет список разделов: «Все образцы», разделы каталога с числом образцов, «Мои образцы».
    void rebuildSections();
    /// @brief Заполняет плитки текущего раздела или результатами поиска (по всем словам запроса,
    ///        без учёта регистра, по всей библиотеке) и запускает дорисовку картинок.
    void rebuildGallery();
    /// @brief Рисует картинки не более чем двух плиток за проход (`GeometryView::render`
    ///        задания при параметрах по умолчанию); когда рисовать нечего — останавливает таймер.
    void renderSomeThumbs();
    /// @brief Показывает карточку записи: заголовок, сведения, параметры по умолчанию, задание.
    /// @param[in] entryIndex  номер в `m_entries`; вне диапазона — ничего не делает.
    void showEntry(int entryIndex);
    /// @brief Строит поля параметров рецепта (число, целое, выбор, флажок); правка перезапускает
    ///        таймер пересборки задания.
    void rebuildParams();
    /// @brief Пересобирает задание по текущим параметрам, подгоняет высоту картинки под пропорции
    ///        области и обновляет карточку.
    void rebuildBuild();
    /// @brief Обновляет карточку: итог `tpl::validate`, паспорт «теория», числа ядра
    ///        (`tpl::samples::baked`, только при умолчаниях), пояснения, справку, доступность кнопок.
    void updateCard();
    /// @brief Строит задание записи каталога.
    /// @param[in] e  запись каталога.
    /// @param[in] v  значения параметров (для своих образцов не используются).
    /// @return Итог рецепта `tpl::samples::make` или прочитанный свой образец
    ///         (`tpl::samples::loadUserSample`); пустой, если рецепт не найден.
    tpl::samples::Build buildFor(const Entry &e, const tpl::samples::Values &v) const;
    /// @brief Возвращает рецепт выбранного встроенного образца.
    /// @return Указатель на рецепт каталога; `nullptr` — ничего не выбрано или выбран свой образец.
    const tpl::samples::Sample *currentSample() const;

    /// @brief Записывает задание в выбранную папку через `tpl::writeTask` (с проверкой
    ///        `tpl::validate` и вопросом о перезаписи) и показывает строку запуска ядра.
    void onSave();
    /// @brief Пересобирает задание и испускает `openRequested`.
    void onOpen();
    /// @brief Удаляет свой образец вместе с папкой (`tpl::samples::removeUserSample`) после вопроса.
    void onDeleteUser();
    /// @brief Возвращает параметры рецепта к значениям по умолчанию.
    void onResetParams();

    /// Все записи каталога: встроенные, затем свои.
    QVector<Entry> m_entries;
    /// Свои образцы с диска.
    QVector<tpl::samples::UserSample> m_user;
    /// Текущий раздел: идентификатор, `all` или `user`.
    QString m_section = QStringLiteral("all");
    /// Номер выбранной записи в `m_entries`; -1 — ничего не выбрано.
    int m_current = -1;
    /// Текущие значения параметров рецепта (ключ -> число).
    tpl::samples::Values m_values;
    /// Итог рецепта при текущих параметрах: задание, паспорт, пояснения, сопутствующие файлы.
    tpl::samples::Build m_build;
    tpl::Project m_view; ///< то, что показывает картинка карточки (адрес живёт вместе с окном)
    /// Нарисованные картинки плиток по ключу записи.
    QHash<QString, QPixmap> m_thumbs;
    /// true — списки заполняет программа; сигналы выбора в это время не обрабатываются.
    bool m_loading = false;

    /// Список разделов.
    QListWidget *m_sections = nullptr;
    /// Строка поиска.
    QLineEdit *m_search = nullptr;
    /// Плитки или надпись «пусто».
    QStackedWidget *m_galleryStack = nullptr;
    /// Плитки образцов (данные строки — номер в `m_entries`).
    QListWidget *m_gallery = nullptr;
    /// Надпись на месте пустого списка плиток.
    QLabel *m_empty = nullptr;
    /// Однократный таймер пересборки задания после правки параметра, 120 мс.
    QTimer *m_rebuildTimer = nullptr;
    /// Таймер дорисовки картинок плиток (интервал 0 — в каждом проходе цикла событий).
    QTimer *m_thumbTimer = nullptr;

    // --- карточка ---
    /// Карточка: 0 — приглашение выбрать образец, 1 — карточка образца.
    QStackedWidget *m_cardStack = nullptr;
    /// Название образца.
    QLabel *m_title = nullptr;
    /// Строка сведений: раздел, ядро, стандарт.
    QLabel *m_meta = nullptr;
    /// Картинка задания: `GeometryView` в режиме предпросмотра (`setPreview(true)`).
    GeometryView *m_preview = nullptr;
    /// Короткое описание образца.
    QLabel *m_summary = nullptr;
    /// Рамка параметров рецепта (скрыта, если параметров нет).
    QGroupBox *m_paramsBox = nullptr;
    /// Форма полей параметров.
    QFormLayout *m_paramsForm = nullptr;
    /// Кнопка «Вернуть по умолчанию».
    QPushButton *m_reset = nullptr;
    /// Итог проверки задания (ошибки, предупреждения, сводка сетки).
    QLabel *m_check = nullptr;
    /// Паспорт: строки «теория».
    QLabel *m_theory = nullptr;
    /// Паспорт: числа, посчитанные ядром при умолчаниях.
    QLabel *m_kernel = nullptr;
    /// Пояснения рецепта и список сопутствующих файлов.
    QLabel *m_notes = nullptr;
    /// Справка образца.
    QLabel *m_help = nullptr;
    /// Кнопка «Открыть в редакторе».
    QPushButton *m_open = nullptr;
    /// Кнопка записи задания.
    QPushButton *m_save = nullptr;
    /// Кнопка удаления своего образца.
    QPushButton *m_delete = nullptr;
};
