#include "transport.h"

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QTextDocumentFragment>
#include <QVBoxLayout>

#include "sidebar.h"

QPushButton* Transport::button(const QString& text, const QString& tip) {
    auto* b = new QPushButton(text);
    b->setObjectName("Transport");
    b->setToolTip(tip);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

Transport::Transport(QWidget* parent) : QFrame(parent) {
    setObjectName("Transport");
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(22, 0, 22, 14);
    lay->setSpacing(6);

    // ── вкладки нижней панели ──
    auto* tabs = new QHBoxLayout;
    tabs->setContentsMargins(0, 0, 0, 0);
    tabs->setSpacing(0);
    auto* tabGroup = new QButtonGroup(this);
    int i = 0;
    for (const char* name : {"ШАГ", "ЖУРНАЛ", "КЛАВИШИ"}) {
        auto* b = new QPushButton(QString::fromUtf8(name));
        b->setObjectName("BottomTab");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        tabGroup->addButton(b, i++);
        tabs->addWidget(b);
    }
    tabs->addStretch(1);
    lay->addLayout(tabs);
    tabGroup->button(0)->setChecked(true);

    pages_ = new QStackedWidget;
    pages_->setFixedHeight(84);
    lay->addWidget(pages_);
    connect(tabGroup, &QButtonGroup::idClicked, pages_, &QStackedWidget::setCurrentIndex);

    stepText_ = new QLabel(QStringLiteral("Введите данные слева, затем нажмите «Шаг» или пробел"));
    stepText_->setObjectName("StepText");
    stepText_->setTextFormat(Qt::RichText);
    stepText_->setWordWrap(true);
    stepText_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    pages_->addWidget(stepText_);

    journal_ = new QListWidget;
    journal_->setObjectName("Journal");
    journal_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    journal_->setFocusPolicy(Qt::NoFocus);
    journal_->setFrameShape(QFrame::NoFrame);
    connect(journal_, &QListWidget::itemClicked, this, [this](QListWidgetItem* it) { emit jump(journal_->row(it) + 1); });
    pages_->addWidget(journal_);

    auto* keys = new QWidget;
    auto* grid = new QGridLayout(keys);
    grid->setContentsMargins(4, 4, 4, 0);
    grid->setHorizontalSpacing(18);
    grid->setVerticalSpacing(2);
    const int cols = 4;
    int k = 0;
    for (const auto& hk : HOTKEYS) {
        const int r = k / cols, c = k % cols;
        auto* kl = new QLabel(QString::fromUtf8(hk.keys));
        kl->setObjectName("Kbd");
        auto* dl = new QLabel(QString::fromUtf8(hk.desc));
        dl->setObjectName("KbdDesc");
        grid->addWidget(kl, r, c * 2);
        grid->addWidget(dl, r, c * 2 + 1);
        ++k;
    }
    grid->setColumnStretch(cols * 2, 1);
    pages_->addWidget(keys);

    // ── кнопки ──
    auto* row = new QHBoxLayout;
    row->setSpacing(8);
    btnFirst_ = button("⏮", QStringLiteral("В начало (Home)"));
    btnPrev_ = button("◀", QStringLiteral("Шаг назад (←)"));
    btnPlay_ = new QPushButton(QStringLiteral("▶  Авто"));
    btnPlay_->setObjectName("Play");
    btnPlay_->setCheckable(true);
    btnPlay_->setToolTip(QStringLiteral("Авто-воспроизведение (Enter)"));
    btnPlay_->setCursor(Qt::PointingHandCursor);
    btnNext_ = button("▶", QStringLiteral("Шаг вперёд (пробел / →)"));
    btnLast_ = button("⏭", QStringLiteral("В конец (End)"));
    for (auto* b : {btnFirst_, btnPrev_, btnPlay_, btnNext_, btnLast_}) row->addWidget(b);
    connect(btnFirst_, &QPushButton::clicked, this, &Transport::first);
    connect(btnPrev_, &QPushButton::clicked, this, &Transport::prev);
    connect(btnNext_, &QPushButton::clicked, this, &Transport::next);
    connect(btnLast_, &QPushButton::clicked, this, &Transport::last);
    connect(btnPlay_, &QPushButton::toggled, this, &Transport::playToggled);

    counter_ = new QLabel;
    counter_->setObjectName("StepCounter");
    row->addSpacing(14);
    row->addWidget(counter_);
    row->addStretch(1);

    auto* speedLab = new QLabel(QStringLiteral("Скорость"));
    speedLab->setObjectName("StepCounter");
    row->addWidget(speedLab);
    speed_ = new QSlider(Qt::Horizontal);
    speed_->setRange(1, 10);
    speed_->setValue(5);
    speed_->setFixedWidth(160);
    speed_->setToolTip(QStringLiteral("+ / − на клавиатуре"));
    connect(speed_, &QSlider::valueChanged, this, [this] { emit speedChanged(intervalMs()); });
    row->addWidget(speed_);
    lay->addLayout(row);
}

int Transport::intervalMs() const {
    // 1 → 2000 мс, 10 → 150 мс
    return int(2000 - (speed_->value() - 1) * (1850.0 / 9));
}

void Transport::nudgeSpeed(int delta) { speed_->setValue(speed_->value() + delta); }

void Transport::setState(int pos, int total, const QString& text, bool error) {
    counter_->setText(QStringLiteral("%1 / %2").arg(pos).arg(total));
    stepText_->setObjectName(error ? "Error" : "StepText");
    stepText_->style()->unpolish(stepText_);
    stepText_->style()->polish(stepText_);
    stepText_->setText(text);
    btnFirst_->setEnabled(pos > 0);
    btnPrev_->setEnabled(pos > 0);
    btnNext_->setEnabled(pos < total);
    btnLast_->setEnabled(pos < total);
    btnPlay_->setEnabled(total > 0);
}

void Transport::setJournal(const QStringList& texts, int pos) {
    // Журнал: все шаги моноширинным списком, текущий — выделен.
    if (journal_->count() != texts.size()) {
        journal_->clear();
        const int width = QString::number(texts.size()).size();
        for (int i = 0; i < texts.size(); ++i)
            journal_->addItem(QStringLiteral("%1   %2").arg(i + 1, width)
                                  .arg(QTextDocumentFragment::fromHtml(texts[i]).toPlainText()));
    }
    journal_->blockSignals(true);
    journal_->clearSelection();
    if (pos > 0) {
        journal_->setCurrentRow(pos - 1);
        journal_->scrollToItem(journal_->item(pos - 1));
    }
    journal_->blockSignals(false);
}

void Transport::setPlaying(bool playing) {
    btnPlay_->blockSignals(true);
    btnPlay_->setChecked(playing);
    btnPlay_->setText(playing ? QStringLiteral("⏸  Пауза") : QStringLiteral("▶  Авто"));
    btnPlay_->blockSignals(false);
}
