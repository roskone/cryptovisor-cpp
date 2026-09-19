#include "sidebar.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QScrollArea>

namespace {

QIcon dotIcon(const QColor& color, int size = 10) {
    QPixmap pm(size * 2, size * 2);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawEllipse(size / 2, size / 2, size, size);
    return QIcon(pm);
}

// Строка-элемент с цветной точкой-статусом.
QPushButton* itemButton(const QString& text, const QColor& dot) {
    auto* b = new QPushButton(text);
    b->setIcon(dotIcon(dot));
    b->setIconSize(QSize(20, 20));
    b->setObjectName("Item");
    b->setCheckable(true);
    b->setCursor(Qt::PointingHandCursor);
    return b;
}

QLabel* hint(const QString& text) {
    auto* h = new QLabel(text);
    h->setObjectName("Hint");
    h->setWordWrap(true);
    return h;
}

QLabel* fieldLabel(const QString& text) {
    auto* l = new QLabel(text);
    l->setObjectName("FieldLabel");
    return l;
}

QFrame* line() {
    auto* l = new QFrame;
    l->setObjectName("SectionLine");
    return l;
}

}  // namespace

Section::Section(const QString& title, QWidget* parent) : QWidget(parent) {
    auto* lay = new QVBoxLayout(this);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);
    auto* head = new QLabel(QStringLiteral("▾   ") + title.toUpper());
    head->setObjectName("SectionHead");
    head->setContentsMargins(12, 10, 12, 8);
    lay->addWidget(head);
    auto* body = new QWidget;
    body_ = new QVBoxLayout(body);
    body_->setContentsMargins(14, 0, 12, 12);
    body_->setSpacing(6);
    lay->addWidget(body);
    lay->addWidget(line());
}

void Section::add(QWidget* w) { body_->addWidget(w); }
void Section::add(QLayout* l) { body_->addLayout(l); }

Sidebar::Sidebar(QWidget* parent) : QFrame(parent) {
    setObjectName("Sidebar");
    setFixedWidth(250);
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(0);

    auto* head = new QWidget;
    auto* hl = new QHBoxLayout(head);
    hl->setContentsMargins(14, 12, 12, 10);
    auto* title = new QLabel(QStringLiteral("Параметры"));
    title->setObjectName("PanelTitle");
    auto* info = new QLabel(QStringLiteral("ⓘ"));
    info->setObjectName("Hint");
    info->setToolTip(QStringLiteral("Всё пересчитывается автоматически при вводе"));
    hl->addWidget(title);
    hl->addStretch(1);
    hl->addWidget(info);
    outer->addWidget(head);
    outer->addWidget(line());

    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setFrameShape(QFrame::NoFrame);
    auto* inner = new QWidget;
    scroll->setWidget(inner);
    outer->addWidget(scroll, 1);
    auto* lay = new QVBoxLayout(inner);
    lay->setContentsMargins(0, 0, 0, 0);
    lay->setSpacing(0);

    // ── Алгоритм ──
    auto* sec = new Section(QStringLiteral("Алгоритм"));
    auto* algoGroup = new QButtonGroup(this);
    const struct { Algo a; const char* name; const char* dot; } algos[] = {
        {Algo::Caesar, "Шифр Цезаря", "#9ece6a"}, {Algo::Xor, "XOR", "#bb9af7"}, {Algo::Transposition, "Перестановка", "#ff9e64"}};
    for (const auto& a : algos) {
        auto* b = itemButton(QString::fromUtf8(a.name), QColor(a.dot));
        algoGroup->addButton(b);
        algoButtons_[a.a] = b;
        sec->add(b);
        connect(b, &QPushButton::clicked, this, [this, algo = a.a] { showPage(algo); emit changed(); });
    }
    algoButtons_[Algo::Caesar]->setChecked(true);
    lay->addWidget(sec);

    // ── Режим ──
    sec = new Section(QStringLiteral("Режим"));
    btnEnc_ = itemButton(QStringLiteral("Шифровать"), QColor("#7aa2f7"));
    btnDec_ = itemButton(QStringLiteral("Дешифровать"), QColor("#f0527a"));
    auto* modeGroup = new QButtonGroup(this);
    modeGroup->addButton(btnEnc_);
    modeGroup->addButton(btnDec_);
    btnEnc_->setChecked(true);
    connect(modeGroup, &QButtonGroup::buttonClicked, this, [this] { emit changed(); });
    sec->add(btnEnc_);
    sec->add(btnDec_);
    lay->addWidget(sec);

    // ── Текст ──
    sec = new Section(QStringLiteral("Текст"));
    text_ = new QLineEdit(QStringLiteral("Привет, мир!"));
    text_->setMaxLength(40);
    text_->setPlaceholderText(QStringLiteral("до 40 символов"));
    connect(text_, &QLineEdit::textChanged, this, &Sidebar::changed);
    sec->add(text_);
    textHint_ = hint({});
    textHint_->hide();
    sec->add(textHint_);
    lay->addWidget(sec);

    // ── Ключ ──
    sec = new Section(QStringLiteral("Ключ"));
    // Цезарь
    auto* w = new QWidget;
    auto* v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(6);
    auto* row = new QHBoxLayout;
    row->addWidget(fieldLabel(QStringLiteral("Сдвиг")));
    caesarShift_ = new QSpinBox;
    caesarShift_->setRange(-32, 32);
    caesarShift_->setValue(3);
    connect(caesarShift_, &QSpinBox::valueChanged, this, &Sidebar::changed);
    row->addWidget(caesarShift_, 1);
    v->addLayout(row);
    v->addWidget(hint(QStringLiteral("Латиница — 26 букв, кириллица — 33 (с Ё). Регистр сохраняется.")));
    pages_[0] = w;
    // XOR
    w = new QWidget;
    v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(6);
    xorNum_ = new QRadioButton(QStringLiteral("Число 0–255"));
    xorStr_ = new QRadioButton(QStringLiteral("Строка (циклически)"));
    xorStr_->setChecked(true);
    v->addWidget(xorNum_);
    xorKeyNum_ = new QSpinBox;
    xorKeyNum_->setRange(0, 255);
    xorKeyNum_->setValue(42);
    v->addWidget(xorKeyNum_);
    v->addWidget(xorStr_);
    xorKeyStr_ = new QLineEdit(QStringLiteral("ключ"));
    xorKeyStr_->setMaxLength(16);
    v->addWidget(xorKeyStr_);
    v->addWidget(hint(QStringLiteral("CP1251: 1 символ = 1 байт. При дешифровании можно ввести hex «4A 2F …».")));
    connect(xorNum_, &QRadioButton::toggled, this, [this] { syncXor(); });
    connect(xorKeyNum_, &QSpinBox::valueChanged, this, &Sidebar::changed);
    connect(xorKeyStr_, &QLineEdit::textChanged, this, &Sidebar::changed);
    pages_[1] = w;
    // Перестановка
    w = new QWidget;
    v = new QVBoxLayout(w);
    v->setContentsMargins(0, 0, 0, 0);
    v->setSpacing(6);
    transKey_ = new QLineEdit(QStringLiteral("КЛЮЧ"));
    transKey_->setMaxLength(10);
    transKey_->setPlaceholderText(QStringLiteral("слово или цифры, напр. 3142"));
    connect(transKey_, &QLineEdit::textChanged, this, &Sidebar::changed);
    v->addWidget(transKey_);
    v->addWidget(hint(QStringLiteral("Слово — столбцы нумеруются по алфавиту букв ключа. Цифры — явный порядок чтения.")));
    pages_[2] = w;
    for (auto* page : pages_) sec->add(page);
    lay->addWidget(sec);

    // ── Показ ──
    sec = new Section(QStringLiteral("Показ"));
    major_ = new QCheckBox(QStringLiteral("Крупные шаги"));
    major_->setToolTip(QStringLiteral("По символу / байту / столбцу вместо бита и ячейки (M)"));
    connect(major_, &QCheckBox::toggled, this, &Sidebar::changed);
    sec->add(major_);
    lay->addWidget(sec);
    lay->addStretch(1);

    syncXor();
    showPage(Algo::Caesar);
}

void Sidebar::showPage(Algo a) {
    for (int i = 0; i < 3; ++i) pages_[i]->setVisible(i == static_cast<int>(a));
}

void Sidebar::syncXor() {
    const bool num = xorNum_->isChecked();
    xorKeyNum_->setEnabled(num);
    xorKeyStr_->setEnabled(!num);
    emit changed();
}

Algo Sidebar::algo() const {
    for (const auto& [a, b] : algoButtons_)
        if (b->isChecked()) return a;
    return Algo::Caesar;
}

void Sidebar::setAlgo(Algo a) {
    algoButtons_[a]->setChecked(true);
    showPage(a);
    emit changed();
}

void Sidebar::setDecrypt(bool dec) {
    (dec ? btnDec_ : btnEnc_)->setChecked(true);
    emit changed();
}

void Sidebar::toggleMajor() { major_->setChecked(!major_->isChecked()); }

void Sidebar::setTextHint(const QString& text) {
    textHint_->setText(text);
    textHint_->setVisible(!text.isEmpty());
}

Params Sidebar::params() const {
    Params p;
    p.algo = algo();
    p.text = text_->text();
    p.decrypt = btnDec_->isChecked();
    p.caesarShift = caesarShift_->value();
    p.xorNumeric = xorNum_->isChecked();
    p.xorKeyNum = xorKeyNum_->value();
    p.xorKeyStr = xorKeyStr_->text();
    p.transKey = transKey_->text();
    p.majorSteps = major_->isChecked();
    return p;
}
