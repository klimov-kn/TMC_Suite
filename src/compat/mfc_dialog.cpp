// Реализация диалогов MFC поверх Qt (см. mfc_dialog.h).
//
// Разметку строит функция, полученная из .rc (см. rc2ui); элементы
// находятся по символьному имени, которое стало objectName виджета. Поэтому
// GetDlgItem(IDC_EDIT1) в коде Windows-версии продолжает работать.

/// @file mfc_dialog.cpp
/// @brief Реализация диалогов MFC на `QDialog`: построение по реестру, подгонка разметки, DDX, элементы, страницы свойств, стандартные диалоги.
///
/// Часть модуля «Слой совместимости», входит в библиотеку `tmc_gui_common`.
/// В Windows-сборке использует перевод интерфейса (`gui_common/i18n`) и окно
/// выбора цвета (`gui_common/colorpicker`).
/// @see https://www.tamic.ru/docs/code/compat.html#rc-dialogs
#if !defined(_WIN32) || defined(TMC_QT_WIN)

#include "mfc_dialog.h"

#ifdef TMC_QT_WIN
#include "colorpicker.h"
#include "i18n/tmci18n.h"
#endif
#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDialog>
#include <QAbstractButton>
#include <QEvent>
#include <QGroupBox>
#include <QGuiApplication>
#include <QScreen>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QPaintEvent>
#include <QShowEvent>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFontDialog>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QListWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QTabWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QString>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTextDocument>
#include <QProgressBar>
#include <QApplication>
#include <QCoreApplication>
#include <QMetaObject>
#include <QThread>
#include <QTimer>
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QFontMetrics>

#include <cstdio>
#include <limits>
#include <cstdlib>
#include <map>

// --- Перекодировка ------------------------------------------------------------
// Подписи и данные приходят из файлов и ресурсов Windows-версии в CP1251.
namespace {

/// @brief Перекодирует строку CP1251 в Unicode.
/// @param[in] s  строка или NULL
/// @return Строка Qt; пустая для NULL.
QString cp1251(const char* s)
{
    if (!s)
        return QString();
    static const ushort table[128] = {
        0x0402,0x0403,0x201A,0x0453,0x201E,0x2026,0x2020,0x2021,
        0x20AC,0x2030,0x0409,0x2039,0x040A,0x040C,0x040B,0x040F,
        0x0452,0x2018,0x2019,0x201C,0x201D,0x2022,0x2013,0x2014,
        0x0098,0x2122,0x0459,0x203A,0x045A,0x045C,0x045B,0x045F,
        0x00A0,0x040E,0x045E,0x0408,0x00A4,0x0490,0x00A6,0x00A7,
        0x0401,0x00A9,0x0404,0x00AB,0x00AC,0x00AD,0x00AE,0x0407,
        0x00B0,0x00B1,0x0406,0x0456,0x0491,0x00B5,0x00B6,0x00B7,
        0x0451,0x2116,0x0454,0x00BB,0x0458,0x0405,0x0455,0x0457,
        0x0410,0x0411,0x0412,0x0413,0x0414,0x0415,0x0416,0x0417,
        0x0418,0x0419,0x041A,0x041B,0x041C,0x041D,0x041E,0x041F,
        0x0420,0x0421,0x0422,0x0423,0x0424,0x0425,0x0426,0x0427,
        0x0428,0x0429,0x042A,0x042B,0x042C,0x042D,0x042E,0x042F,
        0x0430,0x0431,0x0432,0x0433,0x0434,0x0435,0x0436,0x0437,
        0x0438,0x0439,0x043A,0x043B,0x043C,0x043D,0x043E,0x043F,
        0x0440,0x0441,0x0442,0x0443,0x0444,0x0445,0x0446,0x0447,
        0x0448,0x0449,0x044A,0x044B,0x044C,0x044D,0x044E,0x044F,
    };
    QString out;
    for (const char* p = s; *p; ++p) {
        unsigned char ch = static_cast<unsigned char>(*p);
        out.append(ch < 0x80 ? QChar(ch) : QChar(table[ch - 0x80]));
    }
    return out;
}

// Обратный перевод: данные, введённые пользователем, должны лечь в файл задания
// в той же кодировке, что и на Windows.
/// @brief Перекодирует строку Unicode в CP1251 (ASCII, кириллица А–я, Ё, ё; прочее -> `?`).
/// @param[in] s  строка Qt
/// @return Строка в CP1251.
std::string toCp1251(const QString& s)
{
    std::string out;
    out.reserve(size_t(s.size()));
    for (int i = 0; i < s.size(); ++i) {
        const ushort u = s.at(i).unicode();
        if (u < 0x80) {
            out.push_back(char(u));
        } else if (u >= 0x0410 && u <= 0x044F) {
            out.push_back(char(u - 0x0410 + 0xC0));
        } else if (u == 0x0401) {
            out.push_back(char(0xA8));
        } else if (u == 0x0451) {
            out.push_back(char(0xB8));
        } else {
            out.push_back('?');
        }
    }
    return out;
}

/// @brief Возвращает таблицу «номер диалога -> функция разметки».
/// @return Ссылка на статическую таблицу (живёт до конца программы).
std::map<UINT, MfcDialogRegistry::SetupFn>& dialogTable()
{
    static std::map<UINT, MfcDialogRegistry::SetupFn> t;
    return t;
}

/// @brief Возвращает таблицу «номер элемента -> символьное имя».
/// @return Ссылка на статическую таблицу (живёт до конца программы).
std::map<UINT, std::string>& controlTable()
{
    static std::map<UINT, std::string> t;
    return t;
}

/// @brief Находит виджет элемента в окне диалога по символьному имени номера.
/// @param[in] qtDialog  окно `QDialog*` или NULL
/// @param[in] id        номер элемента
/// @return Виджет или NULL (нет окна, имени или виджета).
QWidget* findControl(void* qtDialog, UINT id)
{
    QDialog* d = static_cast<QDialog*>(qtDialog);
    if (!d)
        return 0;
    const char* name = MfcDialogRegistry::controlName(id);
    if (!name)
        return 0;
    return d->findChild<QWidget*>(QString::fromLatin1(name));
}

/// @brief Возвращает текст виджета (поле, подпись, кнопка, список, многострочное поле).
/// @param[in] w  виджет или NULL
/// @return Текст; пустой для неподдерживаемых виджетов.
QString widgetText(QWidget* w)
{
    if (QLineEdit* e = qobject_cast<QLineEdit*>(w))    return e->text();
    if (QLabel* l = qobject_cast<QLabel*>(w))          return l->text();
    if (QAbstractButton* b = qobject_cast<QAbstractButton*>(w)) return b->text();
    if (QComboBox* c = qobject_cast<QComboBox*>(w))    return c->currentText();
    if (QPlainTextEdit* t = qobject_cast<QPlainTextEdit*>(w)) return t->toPlainText();
    if (QTextEdit* t = qobject_cast<QTextEdit*>(w))    return t->toPlainText();
    return QString();
}

/// @brief Задаёт текст виджета; в Windows-сборке переводит подписи и неизменяемые поля.
/// @param[in] w     виджет
/// @param[in] text  текст
/// Пустой `w` (например, `SetDlgItemText` для элемента, которого нет в разметке) — ничего не делает
/// (I-15; раньше Windows-сборка разыменовывала NULL и падала).
void setWidgetText(QWidget* w, const QString& text)
{
    if (!w)     // I-15
        return;
#ifdef TMC_QT_WIN
    // Поле, которое пользователь не правит (недоступное или только для чтения),
    // программа использует как подпись — его текст тоже переводим.
    const bool fixedText = !w->isEnabled()
        || (qobject_cast<QLineEdit*>(w) && qobject_cast<QLineEdit*>(w)->isReadOnly())
        || (qobject_cast<QPlainTextEdit*>(w) && qobject_cast<QPlainTextEdit*>(w)->isReadOnly());
    if (QLineEdit* e = qobject_cast<QLineEdit*>(w))    { e->setText(fixedText ? tmci18n::translate(text) : text); return; }
    if (QPlainTextEdit* t = qobject_cast<QPlainTextEdit*>(w)) {
        if (fixedText) { t->setPlainText(tmci18n::translate(text)); return; }
    }
#else
    if (QLineEdit* e = qobject_cast<QLineEdit*>(w))    { e->setText(text); return; }
#endif
#ifdef TMC_QT_WIN
    // Подпись, которую программа ставит сама («Step finish calculation»),
    // приходит уже после перевода окна — переводим её здесь. Только подписи:
    // поля ввода хранят данные пользователя и не переводятся.
    if (QLabel* l = qobject_cast<QLabel*>(w))          { l->setText(tmci18n::translate(text)); return; }
#else
    if (QLabel* l = qobject_cast<QLabel*>(w))          { l->setText(text); return; }
#endif
    if (QAbstractButton* b = qobject_cast<QAbstractButton*>(w)) { b->setText(text); return; }
    if (QComboBox* c = qobject_cast<QComboBox*>(w))    { c->setCurrentText(text); return; }
    if (QPlainTextEdit* t = qobject_cast<QPlainTextEdit*>(w)) { t->setPlainText(text); return; }
    if (QTextEdit* t = qobject_cast<QTextEdit*>(w))    { t->setPlainText(text); return; }
}

} // namespace

// --- Реестр диалогов ----------------------------------------------------------

void MfcDialogRegistry::registerDialog(UINT idd, SetupFn setup)
{
    dialogTable()[idd] = setup;
}

void MfcDialogRegistry::registerControlName(UINT id, const char* name)
{
    if (name)
        controlTable()[id] = name;
}

MfcDialogRegistry::SetupFn MfcDialogRegistry::setupFor(UINT idd)
{
    std::map<UINT, SetupFn>::const_iterator it = dialogTable().find(idd);
    return it == dialogTable().end() ? 0 : it->second;
}


const char* MfcDialogRegistry::controlName(UINT id)
{
    std::map<UINT, std::string>::const_iterator it = controlTable().find(id);
    return it == controlTable().end() ? 0 : it->second.c_str();
}

// --- Обёртки элементов --------------------------------------------------------

void CWndCtrl::SetWindowText(const char* text)
{
    setWidgetText(static_cast<QWidget*>(m_widget), cp1251(text));
}

int CWndCtrl::GetWindowText(char* buffer, int maxLen) const
{
    if (!buffer || maxLen <= 0)
        return 0;
    const std::string s = toCp1251(widgetText(static_cast<QWidget*>(m_widget)));
    int n = int(s.size());
    if (n > maxLen - 1)
        n = maxLen - 1;
    std::memcpy(buffer, s.c_str(), size_t(n));
    buffer[n] = 0;
    return n;
}


void CWndCtrl::EnableWindow(BOOL enable)
{
    if (QWidget* w = static_cast<QWidget*>(m_widget))
        w->setEnabled(enable != FALSE);
}



void CWndCtrl::SetFocus()
{
    if (QWidget* w = static_cast<QWidget*>(m_widget))
        w->setFocus();
}

// Шрифт элемента из LOGFONT (CFont::CreatePointFont / CreateFontIndirect).
void CWndCtrl::SetFont(CFont* font, BOOL /*redraw*/)
{
    QWidget* w = static_cast<QWidget*>(m_widget);
    if (!w || !font)
        return;
    QFont f = w->font();
    const QString face = QString::fromLocal8Bit(font->m_lf.lfFaceName);
    if (face.contains(QLatin1String("Courier"), Qt::CaseInsensitive)) {
        f.setFamilies({QStringLiteral("JetBrains Mono"), QStringLiteral("Consolas"), face});
        f.setStyleHint(QFont::Monospace);
    } else if (!face.isEmpty()) {
        f.setFamily(face);
    }
    if (font->m_lf.lfHeight != 0)
        f.setPixelSize(int(font->m_lf.lfHeight < 0 ? -font->m_lf.lfHeight : font->m_lf.lfHeight));
    if (font->m_lf.lfWeight >= 700)
        f.setBold(true);
    w->setFont(f);
}

// Холст поверх элемента. Qt не позволяет рисовать по виджету вне обработчика
// перерисовки, поэтому рисуем в картинку, а в ReleaseDC показываем её в
// элементе. Для пользователя результат тот же, что в Windows.
namespace {

/// @brief Рисование поверх элемента: картинка, рисовальщик по ней и сам элемент.
struct CtrlPaint
{
    /// Картинка размером с элемент, залитая цветом фона окна.
    QPixmap  pixmap;
    /// Рисовальщик по картинке.
    QPainter painter;
    /// Элемент, которому отдаётся картинка.
    QWidget* widget;
};

/// @brief Возвращает таблицу «контекст `GetDC` -> рисование поверх элемента».
/// @return Ссылка на статическую таблицу; записи добавляет `CWndCtrl::GetDC`, удаляет `ReleaseDC`.
std::map<CDC*, CtrlPaint*>& ctrlPaints()
{
    static std::map<CDC*, CtrlPaint*> t;
    return t;
}

} // namespace

CDC* CWndCtrl::GetDC()
{
    QWidget* w = static_cast<QWidget*>(m_widget);
    if (!w)
        return 0;

    CtrlPaint* p = new CtrlPaint;
    p->widget = w;
    p->pixmap = QPixmap(w->size().isEmpty() ? QSize(1, 1) : w->size());
    p->pixmap.fill(w->palette().color(QPalette::Window));
    p->painter.begin(&p->pixmap);

    CDC* dc = new CDC();
    dc->Attach(&p->painter, &p->pixmap);
    ctrlPaints()[dc] = p;
    return dc;
}

void CWndCtrl::ReleaseDC(CDC* dc)
{
    std::map<CDC*, CtrlPaint*>::iterator it = ctrlPaints().find(dc);
    if (it == ctrlPaints().end()) {
        delete dc;
        return;
    }
    CtrlPaint* p = it->second;
    p->painter.end();
    if (QLabel* label = qobject_cast<QLabel*>(p->widget))
        label->setPixmap(p->pixmap);
    ctrlPaints().erase(it);
    delete p;
    delete dc;
}



// --- Индикатор выполнения -----------------------------------------------------



int CProgressCtrl::SetPos(int pos)
{
    QProgressBar* b = qobject_cast<QProgressBar*>(static_cast<QWidget*>(m_widget));
    if (!b)
        return 0;
    const int old = b->value();
    b->setValue(pos);
    return old;
}












// --- Сообщения окну -----------------------------------------------------------
// Реализованы здесь, а не в mfc_view.cpp: нужны карта сообщений (mfc_dialog.h)
// и очередь событий Qt, а mfc_view.cpp намеренно собирается без Qt.

LRESULT CWnd::SendMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    // В Windows сообщение шло в оконную процедуру и попадало в обработчик из
    // карты (ON_MESSAGE). Здесь та же карта строится на месте.
    MfcMessageMap map;
    TmcBuildMessageMap(map);
    LRESULT result = 0;
    map.callMessage(message, wParam, lParam, &result);
    return result;
}

BOOL CWnd::PostMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    // Отложенная доставка, как в Windows: обработчик выполнится, когда
    // управление вернётся в цикл событий. Это важно — сообщение обычно шлёт
    // окно, которое просит себя закрыть, и разрушать его посреди его же
    // обработчика нельзя.
    //
    // Получатель обязан дожить до следующего шага цикла событий. В TMC это
    // всегда вид документа, который живёт, пока открыто окно.
    // Отправитель может быть и расчётным потоком: в пакетном режиме счётный
    // узел просит окно закрыться, досчитав последний шаг. У такого потока нет
    // своего цикла событий, поэтому сообщение кладём в очередь ГЛАВНОГО потока
    // — туда же, куда его клала оконная очередь Windows.
    CWnd* target = this;
    QObject* context = QCoreApplication::instance();
    if (!context)
        return FALSE;
    QMetaObject::invokeMethod(context, [target, message, wParam, lParam]() {
        target->SendMessage(message, wParam, lParam);
    }, Qt::QueuedConnection);
    return TRUE;
}

// --- Окно сообщения с кнопками ------------------------------------------------
// Windows-версия спрашивает подтверждение (например, «идёт расчёт, остановить и
// закрыть?») через AfxMessageBox с набором кнопок и сравнивает ответ с IDYES.
int AfxMessageBox(const char* text, UINT type, UINT /*helpId*/)
{
    // Без графики (прогон из консоли, средства проверки) спрашивать некого:
    // ведём себя как пакетный режим Windows-версии — сообщаем в поток ошибок и
    // отвечаем утвердительно.
    if (!qobject_cast<QApplication*>(QCoreApplication::instance())) {
        std::fprintf(stderr, "%s\n", text ? text : "");
        return ((type & 0x0F) == MB_YESNO) ? IDYES : IDOK;
    }

    const QString message = cp1251(text);
    const QString title = QCoreApplication::applicationName();

    QMessageBox::StandardButtons buttons = QMessageBox::Ok;
    if ((type & 0x0F) == MB_OKCANCEL)      buttons = QMessageBox::Ok | QMessageBox::Cancel;
    else if ((type & 0x0F) == MB_YESNO)    buttons = QMessageBox::Yes | QMessageBox::No;

    QMessageBox::Icon icon = QMessageBox::Information;
    const UINT iconBits = type & 0xF0;
    if (iconBits == MB_ICONERROR)            icon = QMessageBox::Critical;
    else if (iconBits == MB_ICONQUESTION)    icon = QMessageBox::Question;
    else if (iconBits == MB_ICONEXCLAMATION) icon = QMessageBox::Warning;

    QMessageBox box(icon, title, message, buttons);
    const int answer = box.exec();

    switch (answer) {
    case QMessageBox::Yes:    return IDYES;
    case QMessageBox::No:     return IDNO;
    case QMessageBox::Cancel: return IDCANCEL;
    default:                  return IDOK;
    }
}

// Сообщение с одним доводом (AfxMessageBox(text), tmc_mfc.h) по умолчанию
// писало в поток ошибок, а окно ставили только счётные ядра. Во вьюверах и
// FieldView так пропадали все сообщения об ошибках: например, «ОК» в окне
// «Отклонения по исходному файлу» без выбранного файла выглядело как
// неработающая кнопка. Теперь по умолчанию — обычное окно сообщения (из
// главного потока); программа может поставить свой обработчик поверх.
namespace {
/// @brief Обработчик `AfxMessageBox(text)` по умолчанию: окно сообщения в потоке интерфейса, иначе stderr.
/// @param[in] text  текст в CP1251
void tmcDefaultMessage(const char* text)
{
    QCoreApplication* app = QCoreApplication::instance();
    if (!qobject_cast<QApplication*>(app) || QThread::currentThread() != app->thread()) {
        std::fprintf(stderr, "%s\n", text ? text : "");
        return;
    }
    QMessageBox box(QMessageBox::Information, QCoreApplication::applicationName(), cp1251(text),
                    QMessageBox::Ok, QApplication::activeWindow());
    box.exec();
}
/// Устанавливает `tmcDefaultMessage` при загрузке программы (статическая инициализация).
const bool g_defaultMessageInstalled = (tmc_set_message_handler(&tmcDefaultMessage), true);
} // namespace

// --- CDialog ------------------------------------------------------------------

namespace {

/// Окно Qt, знающее свой класс-диалог MFC. Нужно ради одного: закрытие окна
/// (крестик) в MFC приходило сообщением WM_CLOSE, и код Windows-версии на него
/// рассчитывает — окно уровней цвета FieldView так сообщает виду, что закрылось.
/// @note Используется и для модальных (`DoModal`), и для немодальных (`Create`) окон.
class TmcModalDialog : public QDialog
{
public:
    /// @brief Создаёт окно, связанное с диалогом MFC.
    /// @param[in] owner  диалог MFC (не владеет)
    explicit TmcModalDialog(CDialog* owner) : m_owner(owner) {}

protected:
    /// @brief При показе поднимает окно наверх и активирует его.
    /// @param[in] event  событие показа
    void showEvent(QShowEvent* event) override
    {
        QDialog::showEvent(event);
        // Модальное окно обязано быть видно: если оно окажется позади главного,
        // программа выглядит зависшей — панель не отвечает, потому что ждёт
        // ответа на невидимое окно.
        raise();
        activateWindow();
    }

    /// @brief После обычной отрисовки вызывает `CDialog::OnPaint`.
    /// @param[in] event  событие перерисовки
    void paintEvent(QPaintEvent* event) override
    {
        QDialog::paintEvent(event);
        // В Windows диалог получал WM_PAINT и дорисовывал образцы цвета и линий
        // поверх своих элементов. Повторяем: после обычной отрисовки окна зовём
        // тот же обработчик Windows-версии.
        if (m_owner)
            m_owner->OnPaint();
    }

    /// @brief Отменяет закрытие средствами Qt и вызывает `CDialog::OnClose`.
    /// @param[in,out] event  событие закрытия (помечается как отклонённое)
    void closeEvent(QCloseEvent* event) override
    {
        // Окно закрывает не Qt, а сам диалог: обработчик может, например,
        // попросить вид разрушить окно (DestroyWindow).
        event->ignore();
        if (m_owner)
            m_owner->OnClose();
    }

    /// @brief Клавиша Esc: вызывает `CDialog::OnCancel` вместо закрытия окна.
    void reject() override
    {
        // Esc в MFC — команда IDCANCEL, то есть OnCancel диалога; Qt закрыл бы окно
        // сам, в обход обработчика (окно хода TMC_DN исчезало при идущем расчёте).
        // OnCancel по умолчанию — EndDialog -> done(), повторного вызова нет.
        if (m_owner)
            m_owner->OnCancel();
        else
            QDialog::reject();
    }

private:
    /// Диалог MFC.
    CDialog* m_owner;
};

} // namespace

CDialog::CDialog(UINT idd, CWnd* parent)
    : m_nIDD(idd), m_qtDialog(0), m_parent(parent)
{
}

CDialog::~CDialog()
{
    // Немодальное окно живёт дольше вызова Create; если его не закрыли,
    // разрушаем сами — оно ссылается на этот объект.
    if (m_qtDialog && m_qtDialogOwned) {
        QDialog* dlg = static_cast<QDialog*>(m_qtDialog);
        m_qtDialog = 0;
        delete dlg;
    }
    for (std::map<UINT, CWndCtrl*>::iterator it = m_ctrlCache.begin();
         it != m_ctrlCache.end(); ++it)
        delete it->second;
}

// Главное окно программы — родитель для модальных окон.
//
// Берём активное окно, а если его нет (например, фокус ушёл в другую программу)
// — первое окно верхнего уровня самой программы. Так диалог всегда появляется
// на своём месте и поверх, а не где решит система окон.
/// @brief Возвращает главное окно программы — родителя для модальных окон.
/// @return Активное окно (если это не диалог) или первое видимое окно верхнего уровня, не диалог; NULL — нет.
static QWidget* tmcOwnerWindow()
{
    if (QWidget* active = QApplication::activeWindow()) {
        if (!qobject_cast<QDialog*>(active))
            return active;
    }
    for (QWidget* w : QApplication::topLevelWidgets()) {
        if (w->isVisible() && !qobject_cast<QDialog*>(w))
            return w;
    }
    return 0;
}

// Расширение элементов под их подписи.
//
// Координаты в ресурсах рассчитаны на шрифт Windows; наш шрифт может оказаться
// чуть шире, и тогда текст обрезается («X ma» вместо «X max»). Делать это можно
// только ПОСЛЕ показа окна: до этого оформление ещё не применено и размеры
// текста считаются по другому шрифту.
/// @brief Расширяет вправо подписи и кнопки, текст которых не помещается, — только до ближайшего соседа справа.
/// @param[in] root  содержимое окна; NULL — ничего
static void tmcGrowLabels(QWidget* root)
{
    if (!root)
        return;

    for (QWidget* w : root->findChildren<QWidget*>()) {
        const bool hasText = qobject_cast<QLabel*>(w) || qobject_cast<QAbstractButton*>(w);
        if (!hasText || qobject_cast<QGroupBox*>(w))
            continue;

        const int needed = w->sizeHint().width();
        if (needed <= w->width())
            continue;

        // Расширяем только вправо и только до ближайшего соседа: в ресурсах
        // Windows поле ввода часто стоит вплотную к подписи, и растянутая
        // подпись ушла бы под него.
        QWidget* parent = w->parentWidget();
        if (!parent)
            continue;

        // Ограничивают только соседи: у правого края окна расти можно —
        // содержимое и само окно после этого пересчитываются.
        int limit = std::numeric_limits<int>::max();
        for (QObject* obj : parent->children()) {
            QWidget* other = qobject_cast<QWidget*>(obj);
            if (!other || other == w)
                continue;
            const QRect r = other->geometry();
            const QRect mine = w->geometry();
            const bool sameRow = r.bottom() > mine.top() && r.top() < mine.bottom();
            if (sameRow && r.left() >= mine.right())
                limit = qMin(limit, r.left() - 2);
        }

        const int allowed = limit - w->x();
        if (allowed > w->width())
            w->resize(qMin(needed, allowed), w->height());
    }
}

// Равномерное растяжение разметки под текст.
//
// Координаты из `.rc` рассчитаны на английские подписи и шрифт Windows. Русские
// подписи длиннее, а у полей ввода и кнопок в оформлении есть внутренние отступы,
// поэтому текст обрезался («едыдущий», «чения по умолчан»), а кнопки выходили
// мелкими. Растягивать отдельные элементы нельзя — они наедут на соседей. Поэтому
// находим, во сколько раз не хватает места самому тесному элементу, и растягиваем
// всю разметку целиком: взаимное расположение сохраняется, окно становится больше.
/// @brief Равномерно растягивает всю разметку так, чтобы поместился самый тесный элемент.
///
/// Для каждого элемента считает нужную ширину/высоту по шрифту и тексту,
/// находит наибольшие коэффициенты `sx`, `sy` (не более 1,7) и умножает на них
/// координаты всех элементов. Переменная окружения `TMC_DIALOG_DEBUG` печатает коэффициенты.
/// @param[in] content  виджет с элементами диалога
/// @see https://www.tamic.ru/docs/code/compat.html#dialog-fit
static void tmcScaleForText(QWidget* content)
{
    const double maxScale = 1.7;            // не раздувать окно сверх меры
    const int minButtonHeight = 28;         // кнопки OK/Отмена — не мельче этого

    const QList<QWidget*> items =
        content->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    double sx = 1.0, sy = 1.0;
    for (QWidget* w : items) {
        if (w->isHidden() || w->width() <= 0 || w->height() <= 0)
            continue;
        const QFontMetrics fm(w->font());
        int needW = 0, needH = 0;
        if (QGroupBox* box = qobject_cast<QGroupBox*>(w)) {
            needW = fm.horizontalAdvance(box->title()) + 24;
        } else if (QLineEdit* edit = qobject_cast<QLineEdit*>(w)) {
            needW = fm.horizontalAdvance(edit->text()) + 16;    // отступы, рамка, курсор
            needH = edit->sizeHint().height();
        } else if (QComboBox* combo = qobject_cast<QComboBox*>(w)) {
            needW = fm.horizontalAdvance(combo->currentText()) + 34;  // плюс стрелка
            needH = combo->sizeHint().height();
        } else if (QPushButton* button = qobject_cast<QPushButton*>(w)) {
            needW = button->sizeHint().width();
            needH = qMax(button->sizeHint().height(), minButtonHeight);
        } else if (qobject_cast<QLabel*>(w) || qobject_cast<QAbstractButton*>(w)) {
            QLabel* label = qobject_cast<QLabel*>(w);
            if (label && (label->wordWrap() || !label->pixmap().isNull()))
                continue;               // многострочный текст и картинки переносятся сами
            needW = w->sizeHint().width();
            needH = w->sizeHint().height();
        } else {
            continue;
        }
        if (needW > w->width())
            sx = qMax(sx, double(needW) / w->width());
        if (needH > w->height())
            sy = qMax(sy, double(needH) / w->height());
    }
    sx = qMin(sx, maxScale);
    sy = qMin(sy, maxScale);
    if (sx <= 1.0 && sy <= 1.0)
        return;

    for (QWidget* w : items) {
        const QRect r = w->geometry();
        const int left = qRound(r.x() * sx), top = qRound(r.y() * sy);
        const int right = qRound((r.x() + r.width()) * sx);
        const int bottom = qRound((r.y() + r.height()) * sy);
        w->setGeometry(left, top, qMax(1, right - left), qMax(1, bottom - top));
    }
    if (std::getenv("TMC_DIALOG_DEBUG"))
        std::fprintf(stderr, "диалог: растяжение под текст %.2f x %.2f\n", sx, sy);
}

// Рамки групп и подписи — под остальными элементами.
//
// В Windows рамка GROUPBOX и статическая подпись прозрачны для мыши: щелчок по
// месту, где кнопка лежит внутри рамки, попадает в кнопку, в каком бы порядке
// элементы ни были записаны в .rc. В Qt щелчок получает верхний виджет, и рамка,
// созданная позже кнопки, перехватывала его — «Графики», «Нормировка»,
// «Прореживать» в окне параметров TMC_DN не нажимались. Поэтому рамки и подписи
// опускаются вниз, а рамки ещё и пропускают мышь насквозь.
/// @brief Опускает подписи и рамки групп под остальные элементы; рамки (кроме отмечаемых) пропускают мышь.
/// @param[in] content  виджет с элементами диалога
/// @see https://www.tamic.ru/docs/code/compat.html#pitfalls
static void tmcFramesBelowControls(QWidget* content)
{
    const QList<QWidget*> items =
        content->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    for (QWidget* w : items) {
        if (qobject_cast<QLabel*>(w))
            w->lower();
    }
    for (QWidget* w : items) {
        if (QGroupBox* box = qobject_cast<QGroupBox*>(w)) {
            if (box->isCheckable())
                continue;
            box->lower();
            box->setAttribute(Qt::WA_TransparentForMouseEvents, true);
        }
    }
}

// Пересчёт размеров окна после показа: элементы уже получили окончательный
// шрифт, поэтому видно, сколько места им нужно на самом деле.
/// @brief Окончательная подгонка окна при первом показе (после перевода и оформления).
///
/// Шаги: рамки под элементы; растяжение под текст; однострочные многострочные
/// поля без отступов и полос; значок программы в пустой подписи 16–64 пикселя;
/// расширение подписей; размер содержимого и окна (не более 95 % x 92 % экрана).
/// @param[in] dlg      окно
/// @param[in] content  виджет с элементами
/// @param[in] area     область прокрутки
static void tmcRefitAfterShow(QDialog* dlg, QWidget* content, QScrollArea* area)
{
    if (!dlg || !content || !area)
        return;

    tmcFramesBelowControls(content);

    // Подписи к этому моменту уже переведены (перевод делает фильтр приложения
    // раньше этого обработчика), поэтому меряем окончательный текст.
    tmcScaleForText(content);

    // Многострочное поле высотой в одну строку: убираем внутренние отступы
    // документа и полосы прокрутки, иначе текста не видно совсем.
    for (QPlainTextEdit* edit : dlg->findChildren<QPlainTextEdit*>()) {
        if (edit->height() > 40)
            continue;
        edit->document()->setDocumentMargin(0);
        edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        edit->setLineWrapMode(QPlainTextEdit::NoWrap);
    }
    for (QTextEdit* edit : dlg->findChildren<QTextEdit*>()) {
        if (edit->height() > 40)
            continue;
        edit->document()->setDocumentMargin(0);
        edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        edit->setLineWrapMode(QTextEdit::NoWrap);
    }

    // Значок программы в окне «О программе». В ресурсах Windows на его месте
    // стоит элемент ICON; разметка переносит его пустой подписью нужного
    // размера, поэтому здесь подставляем сам значок.
    for (QLabel* label : dlg->findChildren<QLabel*>()) {
        if (!label->text().isEmpty() || !label->pixmap().isNull())
            continue;
        const QSize size = label->size();
        if (size.width() < 16 || size.height() < 16 || size.width() > 64 || size.height() > 64)
            continue;
        const QPixmap icon(QStringLiteral(":/icons/appicon.png"));
        if (!icon.isNull())
            label->setPixmap(icon.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }

    // Подписи помещаются за счёт того, что шрифт диалогов совпадает по размеру
    // с windows-версией (см. оформление). Если всё же не хватает пары пикселей,
    // элемент расширяется вправо — но только в свободное место.
    tmcGrowLabels(content);

    QRect box;
    for (QWidget* w : content->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly))
        box = box.united(w->geometry());
    if (box.isEmpty())
        return;

    const int margin = 8;
    QSize wanted(box.right() + margin, box.bottom() + margin);
    content->setMinimumSize(wanted);
    content->resize(wanted);

    QSize windowSize = wanted + QSize(2, 2);
    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        const QRect avail = screen->availableGeometry();
        windowSize.setWidth(qMin(windowSize.width(), int(avail.width() * 0.95)));
        windowSize.setHeight(qMin(windowSize.height(), int(avail.height() * 0.92)));
    }
    area->setMinimumSize(windowSize);
    dlg->setMinimumSize(windowSize);
    if (dlg->size().width() < windowSize.width() || dlg->size().height() < windowSize.height())
        dlg->resize(windowSize);
}

// Следит за первым показом окна и в этот момент подгоняет размеры.
/// @brief Фильтр событий окна: при первом `QEvent::Show` вызывает `tmcRefitAfterShow`.
class TmcDialogFitter : public QObject
{
public:
    /// @brief Создаёт фильтр (дочерний объект окна).
    /// @param[in] dlg      окно
    /// @param[in] content  виджет с элементами
    /// @param[in] area     область прокрутки
    TmcDialogFitter(QDialog* dlg, QWidget* content, QScrollArea* area)
        : QObject(dlg), m_dlg(dlg), m_content(content), m_area(area) {}

protected:
    /// @brief Ловит первый показ окна.
    /// @param[in] watched  объект события
    /// @param[in] event    событие
    /// @return Результат базового фильтра (событие не поглощается).
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (watched == m_dlg && event->type() == QEvent::Show && !m_done) {
            m_done = true;
            tmcRefitAfterShow(m_dlg, m_content, m_area);
        }
        return QObject::eventFilter(watched, event);
    }

private:
    /// Окно диалога.
    QDialog* m_dlg;
    /// Виджет с элементами.
    QWidget* m_content;
    /// Область прокрутки.
    QScrollArea* m_area;
    /// Подгонка уже выполнена.
    bool m_done = false;
};

// Подгонка окна диалога под содержимое.
//
// Разметка перенесена из ресурсов Windows: элементы стоят на своих местах с
// точными координатами, а окно имеет размер из `.rc`. Этого мало по двум
// причинам: подписи в другом шрифте занимают чуть больше места, а сам размер
// окна на другом экране может не подойти. Поэтому:
//   * содержимое кладётся в область с прокруткой — ничего не обрезается;
//   * окно получает размер по содержимому, но не больше экрана;
//   * окно можно растягивать (в MFC диалоги были фиксированными, но там и
//     обрезаться было нечему).
/// @brief Переносит элементы диалога в область прокрутки и задаёт размер окна по содержимому.
///
/// Сохраняет порядок Tab (цепочку фокуса, по которой работает `DDX_Radio`),
/// ставит фильтр `TmcDialogFitter` для подгонки при показе. Окна с уже заданной
/// раскладкой не трогает.
/// @param[in] dlg  окно; NULL — ничего
/// @see https://www.tamic.ru/docs/code/compat.html#dialog-fit
static void tmcFitDialogContent(QDialog* dlg)
{
    if (!dlg || dlg->layout())
        return;

    const QList<QWidget*> children =
        dlg->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
    if (children.isEmpty())
        return;

    QRect box;
    for (QWidget* w : children)
        box = box.united(w->geometry());
    if (box.isEmpty())
        return;

    // Порядок обхода клавишей Tab (он же порядок записи в .rc) запоминаем до
    // переноса: setParent перестраивает цепочку фокуса в порядке создания
    // виджетов, а rc2ui создаёт их в обратном порядке. По этой же цепочке
    // DDX_Radio узнаёт номер переключателя в группе.
    QList<QWidget*> tabOrder;
    for (QWidget* w = dlg->nextInFocusChain(); w && w != dlg; w = w->nextInFocusChain()) {
        if (children.contains(w) && !tabOrder.contains(w))
            tabOrder.append(w);
        if (tabOrder.size() > 4096)
            break;
    }

    const int margin = 8;
    QWidget* content = new QWidget;
    for (QWidget* w : children)
        w->setParent(content);              // положение сохраняется
    for (int i = 1; i < tabOrder.size(); ++i)
        QWidget::setTabOrder(tabOrder.at(i - 1), tabOrder.at(i));
    content->setMinimumSize(box.right() + margin, box.bottom() + margin);
    content->resize(content->minimumSize());

    QScrollArea* area = new QScrollArea(dlg);
    area->setFrameShape(QFrame::NoFrame);
    area->setWidgetResizable(true);
    area->setWidget(content);

    QVBoxLayout* layout = new QVBoxLayout(dlg);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(area);

    // Размер окна: всё содержимое целиком, но в пределах экрана.
    //
    // Задать его через resize() недостаточно: у окна появилась раскладка, и при
    // показе Qt пересчитает размер по ней, а у области с прокруткой собственный
    // размер маленький. Поэтому нижнюю границу задаём самой области — тогда окно
    // открывается с полным содержимым и его можно только увеличивать.
    QSize wanted = content->minimumSize() + QSize(2, 2);
    if (QScreen* screen = QGuiApplication::primaryScreen()) {
        const QRect avail = screen->availableGeometry();
        wanted.setWidth(qMin(wanted.width(), int(avail.width() * 0.95)));
        wanted.setHeight(qMin(wanted.height(), int(avail.height() * 0.92)));
    }
    // Нижнюю границу задаём и области, и самому окну: иначе при показе Qt
    // пересчитает размер по раскладке и окно снова станет маленьким.
    area->setMinimumSize(wanted);
    dlg->setMinimumSize(wanted);
    dlg->resize(wanted);
    dlg->setSizeGripEnabled(true);

    // Окончательная подгонка — при показе, когда оформление уже применено.
    dlg->installEventFilter(new TmcDialogFitter(dlg, content, area));

    if (std::getenv("TMC_DIALOG_DEBUG"))
        std::fprintf(stderr, "диалог: элементов %d, содержимое %dx%d, окно %dx%d\n",
                     int(children.size()), box.right() + margin, box.bottom() + margin,
                     wanted.width(), wanted.height());
}

// Порядок: разметка из реестра -> OK/Отмена -> кнопки из карты сообщений ->
// OnInitDialog -> UpdateData(FALSE) -> подгонка окна.
void CDialog::TmcBuildIn(void* qDialog)
{
    QDialog* dlg = static_cast<QDialog*>(qDialog);
    if (!dlg)
        return;

    MfcDialogRegistry::SetupFn setup = MfcDialogRegistry::setupFor(m_nIDD);
    m_qtDialog = dlg;
    if (setup)
        setup(dlg);

    // Кнопки OK и Отмена работают как в MFC.
    if (QWidget* ok = findControl(dlg, IDOK)) {
        if (QAbstractButton* b = qobject_cast<QAbstractButton*>(ok))
            QObject::connect(b, &QAbstractButton::clicked, dlg, [this]() { this->OnOK(); });
    }
    if (QWidget* cancel = findControl(dlg, IDCANCEL)) {
        if (QAbstractButton* b = qobject_cast<QAbstractButton*>(cancel))
            QObject::connect(b, &QAbstractButton::clicked, dlg, [this]() { this->OnCancel(); });
    }

    // Обработчики из карты сообщений: каждой команде — свой элемент.
    MfcMessageMap map;
    TmcBuildMessageMap(map);
    for (std::map<UINT, MfcMessageMap::Handler>::const_iterator it = map.handlers().begin();
         it != map.handlers().end(); ++it) {
        if (it->first == IDOK || it->first == IDCANCEL)
            continue;
        QWidget* w = findControl(dlg, it->first);
        MfcMessageMap::Handler h = it->second;
        if (QAbstractButton* b = qobject_cast<QAbstractButton*>(w)) {
            QObject::connect(b, &QAbstractButton::clicked, dlg, [h]() { h(); });
            continue;
        }
        // I-18: ON_EN_CHANGE / ON_CBN_SELCHANGE / ON_LBN_SELCHANGE (карта хранит только номер элемента) —
        // по виду элемента; сигналы — только от действий пользователя, как уведомления MFC о выборе
        // (правка поля программой обработчик не вызывает). Раньше подключались только кнопки.
        if (QLineEdit* e = qobject_cast<QLineEdit*>(w))
            QObject::connect(e, &QLineEdit::textEdited, dlg, [h]() { h(); });
        else if (QComboBox* c = qobject_cast<QComboBox*>(w))
            QObject::connect(c, QOverload<int>::of(&QComboBox::activated), dlg, [h]() { h(); });
        else if (QListWidget* l = qobject_cast<QListWidget*>(w))
            QObject::connect(l, &QListWidget::itemClicked, dlg, [h]() { h(); });
    }

    OnInitDialog();
    UpdateData(FALSE);          // значения полей -> элементы, как в MFC

    tmcFitDialogContent(dlg);
}

void CDialog::TmcDetach()
{
    // I-16: таймеры окна — дети QDialog и умирают вместе с ним; остановить их здесь (окно ещё живо)
    // и забыть, чтобы KillTimer после DoModal/DestroyWindow не обращался к удалённым QTimer.
    // Останавливается только таймер, который ещё числится ребёнком окна (указатель сравнивается, не
    // разыменовывается, — окно могло уже удалить его само).
    if (QObject* d = static_cast<QObject*>(m_qtDialog)) {
        const QObjectList& kids = d->children();
        for (std::map<UINT, void*>::iterator it = m_timers.begin(); it != m_timers.end(); ++it)
            if (kids.contains(static_cast<QObject*>(it->second)))
                static_cast<QTimer*>(it->second)->stop();
    }
    m_timers.clear();
    m_qtDialog = 0;
    m_qtDialogOwned = FALSE;
    m_ctrlCache.clear();
}

// Хранилище оформителя окон (tmc_set_dialog_decorator / tmc_dialog_decorator) живёт в
// библиотеке оформления tmc_ui_base (gui_common/aboutbox.cpp): её берёт и TPL Designer,
// у которого слоя MFC нет (27.10.2026). Здесь оформитель только вызывается.

INT_PTR CDialog::DoModal()
{
    if (!MfcDialogRegistry::setupFor(m_nIDD)) {
        // Разметки нет — значит, .rc ещё не переведён в .ui для этого диалога.
        // Молча «нажать отмену» правильнее, чем показать пустое окно.
        std::fprintf(stderr, "TMC: разметка диалога %u не найдена\n", unsigned(m_nIDD));
        return IDCANCEL;
    }

    // Родителем ставим главное окно программы: тогда диалог открывается по
    // центру него, как в Windows, а не там, где решит система окон.
    TmcModalDialog dlg(this);
    if (QWidget* owner = tmcOwnerWindow())
        dlg.setParent(owner, dlg.windowFlags());
    TmcBuildIn(&dlg);
    if (TmcDialogDecorator decorate = tmc_dialog_decorator())
        decorate(&dlg);

    const int result = dlg.exec();
    TmcDetach();
    return result == QDialog::Accepted ? IDOK : IDCANCEL;
}

BOOL CDialog::Create(UINT idd, CWnd* parent)
{
    if (idd)
        m_nIDD = idd;
    if (parent)
        m_parent = parent;

    if (!MfcDialogRegistry::setupFor(m_nIDD)) {
        std::fprintf(stderr, "TMC: разметка диалога %u не найдена\n", unsigned(m_nIDD));
        return FALSE;
    }
    if (m_qtDialog)             // окно уже открыто — как в MFC, второго не делаем
        return TRUE;

    // Немодальное окно: создаётся в куче и живёт до DestroyWindow, поэтому при
    // открытом окне можно работать с остальной программой (вращать сцену).
    TmcModalDialog* dlg = new TmcModalDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose, false);
    TmcBuildIn(dlg);
    if (TmcDialogDecorator decorate = tmc_dialog_decorator())
        decorate(dlg);
    m_qtDialogOwned = TRUE;
    dlg->show();
    return TRUE;
}

BOOL CDialog::Create(const char* /*templateName*/, CWnd* parent)
{
    return Create(UINT(0), parent);
}

void CDialog::OnClose()
{
    // Как в MFC: закрытие окна крестиком равносильно отмене.
    EndDialog(IDCANCEL);
}

BOOL CDialog::DestroyWindow()
{
    QDialog* dlg = static_cast<QDialog*>(m_qtDialog);
    const BOOL owned = m_qtDialogOwned;
    OnDestroy();
    TmcDetach();
    if (dlg && owned) {
        dlg->hide();
        // Разрушаем не сразу: вызов часто приходит из обработчика самого окна.
        dlg->deleteLater();
    }
    return TRUE;
}

UINT CDialog::SetTimer(UINT id, UINT elapseMs, void*)
{
    QDialog* d = static_cast<QDialog*>(m_qtDialog);
    if (!d)
        return 0;
    KillTimer(id);
    QTimer* t = new QTimer(d);
    t->setInterval(int(elapseMs));
    QObject::connect(t, &QTimer::timeout, d, [this, id]() { this->OnTimer(UINT_PTR(id)); });
    t->start();
    m_timers[id] = t;
    return id;
}

BOOL CDialog::KillTimer(UINT id)
{
    std::map<UINT, void*>::iterator it = m_timers.find(id);
    if (it == m_timers.end())
        return FALSE;
    QTimer* t = static_cast<QTimer*>(it->second);
    m_timers.erase(it);
    // I-16: таймер трогаем, только если он ещё ребёнок текущего окна (иначе он удалён вместе с окном).
    QObject* d = static_cast<QObject*>(m_qtDialog);
    if (!d || !d->children().contains(static_cast<QObject*>(t)))
        return FALSE;
    t->stop();
    t->deleteLater();
    return TRUE;
}

BOOL CDialog::OnInitDialog()
{
    return TRUE;
}

void CDialog::DoDataExchange(CDataExchange*)
{
}

void CDialog::OnOK()
{
    if (!UpdateData(TRUE))
        return;
    EndDialog(IDOK);
}

void CDialog::OnCancel()
{
    EndDialog(IDCANCEL);
}

BOOL CDialog::UpdateData(BOOL saveAndValidate)
{
    CDataExchange dx(this, saveAndValidate);
    DoDataExchange(&dx);
    return TRUE;
}

void CDialog::EndDialog(int result)
{
    if (QDialog* d = static_cast<QDialog*>(m_qtDialog))
        d->done(result == IDOK ? QDialog::Accepted : QDialog::Rejected);
}

CWndCtrl* CDialog::GetDlgItem(UINT id) const
{
    std::map<UINT, CWndCtrl*>::const_iterator it = m_ctrlCache.find(id);
    if (it != m_ctrlCache.end())
        return it->second;
    QWidget* w = findControl(m_qtDialog, id);
    CWndCtrl* ctrl = new CWndCtrl(w);
    m_ctrlCache[id] = ctrl;
    return ctrl;
}

void CDialog::SetDlgItemText(UINT id, const char* text)
{
    setWidgetText(findControl(m_qtDialog, id), cp1251(text));
}

int CDialog::GetDlgItemText(UINT id, char* buffer, int maxLen) const
{
    if (!buffer || maxLen <= 0)
        return 0;
    const std::string s = toCp1251(widgetText(findControl(m_qtDialog, id)));
    int n = int(s.size());
    if (n > maxLen - 1)
        n = maxLen - 1;
    std::memcpy(buffer, s.c_str(), size_t(n));
    buffer[n] = 0;
    return n;
}






void CDialog::SetWindowText(const char* text)
{
    if (QDialog* d = static_cast<QDialog*>(m_qtDialog))
        d->setWindowTitle(cp1251(text));
}

// --- DDX / DDV ----------------------------------------------------------------

// Доступ к виджету элемента: CDialog::GetDlgItem уже умеет искать его по
// символьному имени, поэтому обмен данными пользуется тем же путём.
namespace {
/// @brief Возвращает виджет элемента для обмена данными (через `GetDlgItem`).
/// @param[in] pDX  контекст обмена
/// @param[in] id   номер элемента
/// @return Виджет или NULL.
QWidget* ctrl(CDataExchange* pDX, int id)
{
    if (!pDX || !pDX->m_pDlgWnd)
        return 0;
    CWndCtrl* c = pDX->m_pDlgWnd->GetDlgItem(UINT(id));
    return c ? static_cast<QWidget*>(c->widget()) : 0;
}
} // namespace

void DDX_Text(CDataExchange* pDX, int id, CString& value)
{
    QWidget* w = ctrl(pDX, id);
    if (!w)
        return;
    if (pDX->m_bSaveAndValidate)
        value = toCp1251(widgetText(w)).c_str();
    else
        setWidgetText(w, cp1251(value.GetString()));
}

void DDX_Text(CDataExchange* pDX, int id, char* value, int maxLen)
{
    QWidget* w = ctrl(pDX, id);
    if (!w || !value)
        return;
    if (pDX->m_bSaveAndValidate) {
        const std::string s = toCp1251(widgetText(w));
        int n = int(s.size());
        if (n > maxLen - 1)
            n = maxLen - 1;
        std::memcpy(value, s.c_str(), size_t(n));
        value[n] = 0;
    } else {
        setWidgetText(w, cp1251(value));
    }
}

void DDX_Text(CDataExchange* pDX, int id, int& value)
{
    QWidget* w = ctrl(pDX, id);
    if (!w)
        return;
    if (pDX->m_bSaveAndValidate) {
        value = widgetText(w).trimmed().toInt();
    } else {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%d", value);
        setWidgetText(w, QString::fromLatin1(buf));
    }
}

void DDX_Text(CDataExchange* pDX, int id, UINT& value)
{
    int v = int(value);
    DDX_Text(pDX, id, v);
    value = UINT(v);
}

void DDX_Text(CDataExchange* pDX, int id, long& value)
{
    int v = int(value);
    DDX_Text(pDX, id, v);
    value = v;
}

void DDX_Text(CDataExchange* pDX, int id, double& value)
{
    QWidget* w = ctrl(pDX, id);
    if (!w)
        return;
    if (pDX->m_bSaveAndValidate) {
        value = widgetText(w).trimmed().toDouble();
    } else {
        // Формат тот же, что в Windows-версии: %g не теряет значащих цифр и не
        // добавляет лишних нулей.
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%g", value);
        setWidgetText(w, QString::fromLatin1(buf));
    }
}

void DDX_Text(CDataExchange* pDX, int id, float& value)
{
    double v = value;
    DDX_Text(pDX, id, v);
    value = float(v);
}

void DDX_Check(CDataExchange* pDX, int id, int& value)
{
    QAbstractButton* b = qobject_cast<QAbstractButton*>(ctrl(pDX, id));
    if (!b)
        return;
    if (pDX->m_bSaveAndValidate)
        value = b->isChecked() ? 1 : 0;
    else
        b->setChecked(value != 0);
}

void DDX_Radio(CDataExchange* pDX, int id, int& value)
{
    // В MFC переключатели идут подряд от первого с признаком WS_GROUP в порядке
    // записи в .rc; номер в группе и есть значение. Номера элементов при этом
    // идти подряд не обязаны («ДН» = 1048, «ДН, дБ» = 1053 в TMC_DN), поэтому
    // группу берём из разметки (QButtonGroup, см. rc2ui), а порядок —
    // из цепочки фокуса: она выстроена по порядку записи в .rc.
    if (!pDX || !pDX->m_pDlgWnd)
        return;
    QAbstractButton* first = qobject_cast<QAbstractButton*>(ctrl(pDX, id));
    if (first && first->group()) {
        QButtonGroup* group = first->group();
        QList<QAbstractButton*> order;
        QWidget* w = first;
        for (int guard = 0; w && guard < 4096; ++guard) {
            QAbstractButton* b = qobject_cast<QAbstractButton*>(w);
            if (b && b->group() == group && !order.contains(b))
                order.append(b);
            w = w->nextInFocusChain();
            if (w == first)
                break;
        }
        for (QAbstractButton* b : group->buttons()) {    // не попавшие в цепочку — в конец
            if (!order.contains(b))
                order.append(b);
        }
        if (pDX->m_bSaveAndValidate) {
            value = -1;
            for (int i = 0; i < order.size(); ++i) {
                if (order.at(i)->isChecked())
                    value = i;
            }
        } else {
            // Снять все, затем отметить нужный: в группе с исключением снять
            // отметку напрямую нельзя.
            const bool exclusive = group->exclusive();
            group->setExclusive(false);
            for (int i = 0; i < order.size(); ++i)
                order.at(i)->setChecked(i == value);
            group->setExclusive(exclusive);
        }
        return;
    }
    int index = 0;
    for (UINT cur = UINT(id); ; ++cur, ++index) {
        QAbstractButton* b = qobject_cast<QAbstractButton*>(ctrl(pDX, int(cur)));
        if (!b)
            break;
        if (pDX->m_bSaveAndValidate) {
            if (b->isChecked())
                value = index;
        } else {
            b->setChecked(index == value);
        }
        if (index > 64)         // защита от бесконечного перебора
            break;
    }
}

void DDX_Control(CDataExchange* pDX, int id, CWndCtrl& widget)
{
    // В MFC связывает переменную-обёртку с элементом окна. Здесь достаточно
    // запомнить виджет: дальше обёртка обращается к нему напрямую.
    widget.attach(ctrl(pDX, id));
}



void DDV_MaxChars(CDataExchange* pDX, CString& value, int maxChars)
{
    // I-20: обрезаем копией (Left), а не записью в буфер: буфер CString может быть общим с другими
    // строками (было: const_cast<char*>(value.GetString())[maxChars] = 0 — портились и копии).
    if (pDX && pDX->m_bSaveAndValidate && value.GetLength() > maxChars)
        value = value.Left(maxChars);
}

void DDV_MaxChars(CDataExchange* pDX, char* value, int maxChars)
{
    if (pDX && pDX->m_bSaveAndValidate && value && int(std::strlen(value)) > maxChars)
        value[maxChars] = 0;
}

void DDV_MinMaxInt(CDataExchange*, int, int, int) {}
void DDV_MinMaxDouble(CDataExchange*, double, double, double) {}

// --- Стандартные диалоги ------------------------------------------------------

CFileDialog::CFileDialog(BOOL openFileDialog, const char* defExt, const char* fileName,
                         DWORD, const char* filter, CWnd* parent)
    : m_open(openFileDialog),
      m_defExt(defExt ? defExt : ""),
      m_fileName(fileName ? fileName : ""),
      m_filter(filter ? filter : ""),
      m_parent(parent)
{
}

INT_PTR CFileDialog::DoModal()
{
    // Фильтр MFC записан как "Описание|шаблон||"; переводим в вид Qt.
    QString filter;
    if (!m_filter.empty()) {
        const QStringList parts = cp1251(m_filter.c_str()).split(QLatin1Char('|'));
        for (int i = 0; i + 1 < parts.size(); i += 2) {
            if (parts[i].isEmpty())
                break;
            if (!filter.isEmpty())
                filter += QStringLiteral(";;");
            filter += parts[i] + QStringLiteral(" (") + parts[i + 1] + QLatin1Char(')');
        }
    }

    const QString start = cp1251(m_fileName.c_str());
    const QString path = m_open
        ? QFileDialog::getOpenFileName(0, QString(), start, filter)
        : QFileDialog::getSaveFileName(0, QString(), start, filter);
    if (path.isEmpty())
        return IDCANCEL;
    m_path = path.toLocal8Bit().constData();
    return IDOK;
}


CColorDialog::CColorDialog(COLORREF initial, DWORD, CWnd* parent)
    : m_color(initial), m_parent(parent)
{
}

INT_PTR CColorDialog::DoModal()
{
    const QColor initial(int(GetRValue(m_color)), int(GetGValue(m_color)), int(GetBValue(m_color)));
#ifdef TMC_QT_WIN
    // Новое окно выбора цвета (src/gui_common/colorpicker.cpp).
    QColor c;
    if (!tmcPickColor(tmcOwnerWindow(), initial, &c))
        return IDCANCEL;
#else
    const QColor c = QColorDialog::getColor(initial, 0);
    if (!c.isValid())
        return IDCANCEL;
#endif
    m_color = RGB(c.red(), c.green(), c.blue());
    return IDOK;
}

CFontDialog::CFontDialog(LOGFONT* initial, DWORD, void*, CWnd* parent)
    : m_parent(parent)
{
    std::memset(&m_lf, 0, sizeof(m_lf));
    if (initial)
        m_lf = *initial;
}

INT_PTR CFontDialog::DoModal()
{
    QFont start;
    if (m_lf.lfFaceName[0])
        start.setFamily(QString::fromLatin1(m_lf.lfFaceName));
    if (m_lf.lfHeight)
        start.setPixelSize(int(m_lf.lfHeight < 0 ? -m_lf.lfHeight : m_lf.lfHeight));
    start.setBold(m_lf.lfWeight >= FW_BOLD);
    start.setItalic(m_lf.lfItalic != 0);

    bool ok = false;
    const QFont f = QFontDialog::getFont(&ok, start, 0);
    if (!ok)
        return IDCANCEL;

    std::memset(&m_lf, 0, sizeof(m_lf));
    const std::string family = f.family().toLatin1().constData();
    std::strncpy(m_lf.lfFaceName, family.c_str(), LF_FACESIZE - 1);
    m_lf.lfHeight = -(f.pixelSize() > 0 ? f.pixelSize() : f.pointSize());
    m_lf.lfWeight = f.bold() ? FW_BOLD : FW_NORMAL;
    m_lf.lfItalic = f.italic() ? 1 : 0;
    m_lf.lfUnderline = f.underline() ? 1 : 0;
    m_lf.lfCharSet = DEFAULT_CHARSET;
    return IDOK;
}


// --- CImageList ---------------------------------------------------------------
// Склад значков: номер картинки в наборе -> QIcon.

CImageList::CImageList()
    : m_icons(0), m_cx(0), m_cy(0)
{
}

CImageList::~CImageList()
{
    delete static_cast<QList<QIcon>*>(m_icons);
}








// --- CListCtrl ----------------------------------------------------------------
// Список с колонками поверх QTreeWidget: строка списка — элемент верхнего
// уровня, колонка — колонка дерева.

namespace {

/// @brief Приводит виджет обёртки к `QTreeWidget`.
/// @param[in] widget  `QWidget*` или NULL
/// @return `QTreeWidget*` или NULL.
QTreeWidget* asTree(void* widget)
{
    return qobject_cast<QTreeWidget*>(static_cast<QWidget*>(widget));
}

} // namespace














// --- CPropertyPage ------------------------------------------------------------

void CPropertyPage::SetModified(BOOL changed)
{
    m_bModified = changed ? TRUE : FALSE;
    if (m_sheet)
        m_sheet->UpdateApplyButton();
}

// --- CPropertySheet -----------------------------------------------------------
// Окно с закладками: QTabWidget сверху, кнопки OK / Cancel / Apply снизу — тот
// же порядок, что в Windows. Каждая страница остаётся своим диалогом, её окно
// просто становится содержимым вкладки.

CPropertySheet::CPropertySheet(UINT /*nIDCaption*/, CWnd* /*parent*/, UINT iSelectPage)
    : m_active(int(iSelectPage)), m_qtSheet(0), m_qtTabs(0), m_qtApply(0)
{
}

CPropertySheet::CPropertySheet(const char* pszCaption, CWnd* /*parent*/, UINT iSelectPage)
    : m_caption(pszCaption ? pszCaption : ""), m_active(int(iSelectPage)),
      m_qtSheet(0), m_qtTabs(0), m_qtApply(0)
{
}

CPropertySheet::~CPropertySheet()
{
}

void CPropertySheet::AddPage(CPropertyPage* page)
{
    if (!page)
        return;
    page->m_sheet = this;
    m_pages.push_back(page);
}


CPropertyPage* CPropertySheet::GetPage(int index) const
{
    if (index < 0 || index >= int(m_pages.size()))
        return 0;
    return m_pages[size_t(index)];
}

CPropertyPage* CPropertySheet::GetActivePage() const
{
    return GetPage(m_active);
}



BOOL CPropertySheet::OnInitDialog()
{
    return TRUE;
}

void CPropertySheet::UpdateApplyButton()
{
    QPushButton* apply = static_cast<QPushButton*>(m_qtApply);
    if (!apply)
        return;
    bool anyModified = false;
    for (size_t i = 0; i < m_pages.size(); ++i) {
        if (m_pages[i]->IsModified()) {
            anyModified = true;
            break;
        }
    }
    apply->setEnabled(anyModified);
}

void CPropertySheet::ApplyPages()
{
    // Как в MFC: сначала значения элементов переносятся в переменные страницы
    // (UpdateData), затем страница применяет их (OnApply шлёт виду сообщение).
    for (size_t i = 0; i < m_pages.size(); ++i) {
        CPropertyPage* page = m_pages[i];
        page->UpdateData(TRUE);
        page->OnApply();
        page->SetModified(FALSE);
    }
}

INT_PTR CPropertySheet::DoModal()
{
    if (m_pages.empty())
        return IDCANCEL;

    QDialog sheet;
    m_qtSheet = &sheet;
    sheet.setWindowTitle(m_caption.empty() ? QStringLiteral("Properties")
                                           : cp1251(m_caption.c_str()));

    QVBoxLayout* layout = new QVBoxLayout(&sheet);
    QTabWidget* tabs = new QTabWidget(&sheet);
    m_qtTabs = tabs;
    layout->addWidget(tabs);

    // Каждая страница строится в своём окне Qt, а окно становится вкладкой.
    // Подпись корешка — заголовок диалога из ресурсов (Place, Topology, ...),
    // как в Windows.
    for (size_t i = 0; i < m_pages.size(); ++i) {
        CPropertyPage* page = m_pages[i];
        QDialog* pageWindow = new QDialog(tabs);
        page->TmcBuildIn(pageWindow);
        QString caption = pageWindow->windowTitle();
        if (caption.isEmpty() && page->m_strCaption.GetLength() > 0)
            caption = cp1251(page->m_strCaption.GetString());
        if (caption.isEmpty())
            caption = QStringLiteral("Page %1").arg(int(i) + 1);
        // Две страницы из одного шаблона (мелодии ядра — обе «Melody») иначе
        // неотличимы по корешку; нумеруем повторяющиеся: «Melody 1», «Melody 2».
        int same = 0, mine = 0;
        for (size_t j = 0; j < m_pages.size(); ++j) {
            if (m_pages[j]->m_nIDD == page->m_nIDD) {
                ++same;
                if (j <= i)
                    ++mine;
            }
        }
        if (same > 1)
            caption += QStringLiteral(" %1").arg(mine);
        tabs->addTab(pageWindow, caption);
    }

    // Кнопки внизу окна, как в Windows: OK, Cancel, Apply.
    QDialogButtonBox* buttons = new QDialogButtonBox(&sheet);
    QPushButton* okButton = buttons->addButton(QDialogButtonBox::Ok);
    QPushButton* cancelButton = buttons->addButton(QDialogButtonBox::Cancel);
    QPushButton* applyButton = buttons->addButton(QDialogButtonBox::Apply);
    m_qtApply = applyButton;
    applyButton->setEnabled(false);
    layout->addWidget(buttons);

    QObject::connect(tabs, &QTabWidget::currentChanged, &sheet, [this](int index) {
        // Переход на другую закладку: MFC спрашивает у прежней страницы
        // разрешение уйти и сообщает новой, что её показали.
        CPropertyPage* prev = GetActivePage();
        if (prev && prev != GetPage(index))
            prev->OnKillActive();
        m_active = index;
        if (CPropertyPage* page = GetActivePage())
            page->OnSetActive();
    });

    QObject::connect(applyButton, &QPushButton::clicked, &sheet, [this]() { this->ApplyPages(); });
    QObject::connect(okButton, &QPushButton::clicked, &sheet, [this, &sheet]() {
        this->ApplyPages();
        sheet.accept();
    });
    QObject::connect(cancelButton, &QPushButton::clicked, &sheet, [&sheet]() { sheet.reject(); });

    if (m_active < 0 || m_active >= int(m_pages.size()))
        m_active = 0;
    tabs->setCurrentIndex(m_active);

    OnInitDialog();
    if (CPropertyPage* page = GetActivePage())
        page->OnSetActive();

    const int result = sheet.exec();
    if (result != QDialog::Accepted)
        OnClose();

    // Окна страниц разрушаются вместе с окном набора; страницы об этом должны
    // узнать, иначе будут держать указатель на несуществующее окно.
    for (size_t i = 0; i < m_pages.size(); ++i)
        m_pages[i]->TmcDetach();
    m_qtApply = 0;
    m_qtTabs = 0;
    m_qtSheet = 0;
    return result == QDialog::Accepted ? IDOK : IDCANCEL;
}

#endif // !_WIN32
