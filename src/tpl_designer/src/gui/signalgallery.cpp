/// @file signalgallery.cpp
/// @brief Реализация галереи форм сигнала (`SignalGallery`) и показательных параметров
///        форм для значков (`demoWaveshape`).
///
/// Часть интерфейса редактора заданий.
/// @see https://www.tamic.ru/docs/code/tpl-designer.html#gui-signals
#include "signalgallery.h"

#include "sigfile.h"
#include "signalpreview.h"
#include "dtheme.h"

// Подключаем заголовок того, чем пользуемся, и не надеемся на транзитивность:
// QListWidget тянет QListView и QIcon на MinGW, но полагаться на это нельзя — ровно
// на этом собранный под Windows код не собирался под Linux (правила проекта TPL Designer).
#include <QIcon>
#include <QLabel>
#include <QListView>
#include <QListWidget>
#include <QListWidgetItem>
#include <QSize>
#include <QVBoxLayout>

#include <cmath>

using namespace tpl;

namespace {

/// @name Размер значка формы сигнала в галерее, логические пиксели
///@{
/// Ширина значка.
constexpr int kIconW = 104;
/// Высота значка.
constexpr int kIconH = 46;
///@}

} // namespace

Waveshape demoWaveshape(Waveform k)
{
    Waveshape w;
    w.kind = k;
    w.amplitude = 1.0;
    w.start = 0.0;

    switch (k) {
    case Waveform::MultiTone:
        // Четырёх тонов хватает, чтобы на значке читались биения — признак гребёнки.
        w.f1 = 2.0;
        w.f2 = 3.2;
        w.tones = 4;
        w.duration = 3.0;
        break;
    case Waveform::Chirp:
        w.f1 = 0.7;
        w.f2 = 4.0;
        w.duration = 3.0;
        break;
    case Waveform::SineBurst:
        w.f1 = 1.5;
        w.periods = 3;
        break;
    case Waveform::GaussRf:
        w.f1 = 3.0;
        w.width = 0.35;
        break;
    case Waveform::GaussVideo:
    case Waveform::Monocycle:
        w.width = 0.35;
        break;
    case Waveform::Trapezoid:
        w.rise = 0.4;
        w.duration = 1.2;
        w.fall = 0.4;
        break;
    case Waveform::Sawtooth:
    case Waveform::Triangle:
    case Waveform::Square:
        w.f1 = 1.2;
        w.duration = 2.5;
        break;
    case Waveform::Expression:
        // Затухающий синус: сразу понятно, что формулу можно написать любую.
        w.expression = QStringLiteral("exp(-t)*sin(2*3.14159265*2.5*t)");
        w.duration = 3.0;
        break;
    case Waveform::Table: {
        // Ломаная «от руки»: показывает, что точки задаёт человек.
        const double ys[] = {0.0, 0.9, 0.2, 0.55, -0.7, -0.15, -0.5, 0.0};
        for (int i = 0; i < 8; ++i)
            w.points << QPointF(i * 0.4, ys[i]);
        break;
    }
    }
    return w;
}

SignalGallery::SignalGallery(QWidget *parent)
    : QWidget(parent)
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(6);

    m_list = new QListWidget;
    m_list->setViewMode(QListView::IconMode);
    m_list->setFlow(QListView::LeftToRight);
    m_list->setWrapping(true);
    m_list->setResizeMode(QListView::Adjust);
    m_list->setMovement(QListView::Static);
    m_list->setIconSize(QSize(kIconW, kIconH));
    m_list->setGridSize(QSize(kIconW + 16, kIconH + 34));
    m_list->setSpacing(2);
    m_list->setUniformItemSizes(true);
    m_list->setWordWrap(true);
    // Четыре плитки в ряд: три ряда на двенадцать форм — влезает и в узкое окно.
    m_list->setMinimumHeight(3 * (kIconH + 34) + 12);
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    for (int k = int(Waveform::MultiTone); k <= int(Waveform::Table); ++k) {
        const Waveform kind = Waveform(k);
        auto *it = new QListWidgetItem(waveformName(kind));
        it->setIcon(QIcon(waveformIcon(demoWaveshape(kind), QSize(kIconW, kIconH))));
        it->setData(Qt::UserRole, k);
        it->setToolTip(waveformHint(kind));
        it->setTextAlignment(Qt::AlignHCenter | Qt::AlignTop);
        m_list->addItem(it);
    }
    root->addWidget(m_list);

    m_hint = new QLabel;
    m_hint->setWordWrap(true);
    m_hint->setProperty("mute", true);
    m_hint->setMinimumHeight(34);
    root->addWidget(m_hint);

    connect(m_list, &QListWidget::currentRowChanged, this, [this] {
        syncHint();
        emit currentChanged();
    });

    m_list->setCurrentRow(0);
    syncHint();
}

Waveform SignalGallery::current() const
{
    const QListWidgetItem *it = m_list->currentItem();
    if (!it)
        return Waveform::MultiTone;
    return Waveform(it->data(Qt::UserRole).toInt());
}

void SignalGallery::setCurrent(Waveform k)
{
    for (int i = 0; i < m_list->count(); ++i) {
        if (m_list->item(i)->data(Qt::UserRole).toInt() == int(k)) {
            m_list->setCurrentRow(i);
            return;
        }
    }
}

void SignalGallery::syncHint()
{
    m_hint->setText(waveformHint(current()));
}
