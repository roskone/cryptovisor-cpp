#include "tabbar.h"

#include <QButtonGroup>
#include <QHBoxLayout>

TabBar::TabBar(QWidget* parent) : QFrame(parent) {
    setObjectName("TabBar");
    setFixedHeight(40);
    auto* lay = new QHBoxLayout(this);
    lay->setContentsMargins(0, 0, 12, 0);
    lay->setSpacing(0);
    auto* group = new QButtonGroup(this);
    const struct { Algo a; const char* name; const char* glyph; } items[] = {
        {Algo::Caesar, "Цезарь", "⇄"}, {Algo::Xor, "XOR", "⊕"}, {Algo::Transposition, "Перестановка", "▦"}};
    for (const auto& it : items) {
        auto* b = new QPushButton(QStringLiteral("%1   %2      ×").arg(QString::fromUtf8(it.glyph), QString::fromUtf8(it.name)));
        b->setObjectName("Tab");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        group->addButton(b);
        tabs_[it.a] = b;
        lay->addWidget(b);
        connect(b, &QPushButton::clicked, this, [this, a = it.a] { emit algoSelected(a); });
    }
    lay->addStretch(1);

    pill_ = new QLabel(QStringLiteral("Шаг 0 / 0"));
    pill_->setObjectName("Pill");
    lay->addWidget(pill_);
    lay->addSpacing(12);

    auto* segGroup = new QButtonGroup(this);
    btnEnc_ = new QPushButton(QStringLiteral("Шифрование"));
    btnDec_ = new QPushButton(QStringLiteral("Дешифрование"));
    for (auto* b : {btnEnc_, btnDec_}) {
        b->setObjectName("Seg");
        b->setCheckable(true);
        b->setCursor(Qt::PointingHandCursor);
        segGroup->addButton(b);
        lay->addWidget(b);
        lay->addSpacing(4);
    }
    connect(btnEnc_, &QPushButton::clicked, this, [this] { emit modeSelected(false); });
    connect(btnDec_, &QPushButton::clicked, this, [this] { emit modeSelected(true); });
    btnEnc_->setChecked(true);
}

void TabBar::sync(Algo algo, bool decrypt) {
    tabs_[algo]->setChecked(true);
    (decrypt ? btnDec_ : btnEnc_)->setChecked(true);
}

void TabBar::setStep(int pos, int total) { pill_->setText(QStringLiteral("Шаг %1 / %2").arg(pos).arg(total)); }
