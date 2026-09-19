#include "main_window.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLineEdit>
#include <QShortcut>
#include <QSpinBox>
#include <QVBoxLayout>

#include "../core/caesar.h"
#include "../core/transposition.h"
#include "../core/xor_cipher.h"

MainWindow::MainWindow() {
    setWindowTitle(QStringLiteral("Криптовизор — пошаговая визуализация шифров"));
    resize(1440, 900);

    auto* central = new QWidget;
    setCentralWidget(central);
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    sidebar_ = new Sidebar;
    root->addWidget(sidebar_);

    auto* right = new QVBoxLayout;
    right->setContentsMargins(0, 0, 0, 0);
    right->setSpacing(0);
    tabbar_ = new TabBar;
    right->addWidget(tabbar_);
    stack_ = new QStackedWidget;
    caesarScene_ = new CaesarScene;
    xorScene_ = new XorScene;
    transScene_ = new TranspositionScene;
    for (Scene* s : {static_cast<Scene*>(caesarScene_), static_cast<Scene*>(xorScene_), static_cast<Scene*>(transScene_)})
        stack_->addWidget(s);
    right->addWidget(stack_, 1);
    transport_ = new Transport;
    right->addWidget(transport_);
    root->addLayout(right, 1);

    timer_.setInterval(transport_->intervalMs());
    connect(&timer_, &QTimer::timeout, this, [this] {
        if (pos_ >= total()) setPlaying(false);
        else stepForward();
    });

    connect(sidebar_, &Sidebar::changed, this, &MainWindow::rebuild);
    connect(transport_, &Transport::first, this, [this] { goTo(0); });
    connect(transport_, &Transport::prev, this, &MainWindow::stepBack);
    connect(transport_, &Transport::next, this, &MainWindow::stepForward);
    connect(transport_, &Transport::last, this, [this] { goTo(total()); });
    connect(transport_, &Transport::playToggled, this, &MainWindow::setPlaying);
    connect(transport_, &Transport::speedChanged, &timer_, qOverload<int>(&QTimer::setInterval));
    connect(transport_, &Transport::jump, this, &MainWindow::goTo);
    connect(tabbar_, &TabBar::algoSelected, sidebar_, &Sidebar::setAlgo);
    connect(tabbar_, &TabBar::modeSelected, sidebar_, &Sidebar::setDecrypt);

    setupShortcuts();
    rebuild();
    currentScene()->setFocus();
}

void MainWindow::setupShortcuts() {
    auto sc = [this](std::initializer_list<QKeySequence> keys, std::function<void()> slot) {
        for (const auto& k : keys) {
            auto* s = new QShortcut(k, this);
            s->setContext(Qt::WindowShortcut);
            connect(s, &QShortcut::activated, this, slot);
        }
    };
    sc({Qt::Key_Space, Qt::Key_Right}, [this] { stepForward(); });
    sc({Qt::Key_Left}, [this] { stepBack(); });
    sc({Qt::Key_Home}, [this] { goTo(0); });
    sc({Qt::Key_End}, [this] { goTo(total()); });
    sc({Qt::Key_Return, Qt::Key_Enter}, [this] { togglePlay(); });
    sc({Qt::Key_Plus, Qt::Key_Equal}, [this] { transport_->nudgeSpeed(1); });
    sc({Qt::Key_Minus, Qt::Key_Underscore}, [this] { transport_->nudgeSpeed(-1); });
    sc({Qt::Key_1}, [this] { sidebar_->setAlgo(Algo::Caesar); });
    sc({Qt::Key_2}, [this] { sidebar_->setAlgo(Algo::Xor); });
    sc({Qt::Key_3}, [this] { sidebar_->setAlgo(Algo::Transposition); });
    sc({Qt::Key_E}, [this] { sidebar_->setDecrypt(false); });
    sc({Qt::Key_D}, [this] { sidebar_->setDecrypt(true); });
    sc({Qt::Key_M}, [this] { sidebar_->toggleMajor(); });
    sc({Qt::Key_F, Qt::Key_F11, QKeySequence::FullScreen}, [this] { toggleFullscreen(); });
    sc({Qt::Key_Escape}, [this] { escape(); });
}

void MainWindow::escape() {
    QWidget* w = QApplication::focusWidget();
    if (qobject_cast<QLineEdit*>(w) || qobject_cast<QSpinBox*>(w)) currentScene()->setFocus();
    else if (isFullScreen()) showNormal();
}

void MainWindow::toggleFullscreen() { isFullScreen() ? showNormal() : showFullScreen(); }

void MainWindow::rebuild() {
    setPlaying(false);
    const Params prm = sidebar_->params();
    majorOnly_ = prm.majorSteps;
    tabbar_->sync(prm.algo, prm.decrypt);
    sidebar_->setTextHint({});
    try {
        build(prm);
    } catch (const CipherError& e) {
        steps_.clear();
        currentScene()->clearContext();
        transport_->setState(0, 0, e.message, true);
        tabbar_->setStep(0, 0);
        return;
    }
    pos_ = 0;
    transport_->clearJournal();
    showStep(false);
}

void MainWindow::build(const Params& prm) {
    switch (prm.algo) {
        case Algo::Caesar: {
            steps_ = caesar::steps(prm.text, prm.caesarShift, prm.decrypt);
            stack_->setCurrentWidget(caesarScene_);
            caesarScene_->setContext(prm.text, prm.decrypt ? -prm.caesarShift : prm.caesarShift);
            break;
        }
        case Algo::Xor: {
            bool wasHex = false;
            const QByteArray data = xorc::parseInput(prm.text, prm.decrypt, &wasHex);
            const QByteArray key = xorc::parseKey(prm.xorNumeric, prm.xorKeyNum, prm.xorKeyStr);
            steps_ = xorc::steps(data, key);
            const QByteArray result = xorc::apply(data, key);
            QString hint = QStringLiteral("Результат hex: ") + QString::fromLatin1(result.toHex(' ')).toUpper();
            if (prm.decrypt) hint += QStringLiteral("   → «%1»").arg(xorc::decodeText(result));
            sidebar_->setTextHint(hint);
            stack_->setCurrentWidget(xorScene_);
            xorScene_->setContext(data, key, wasHex, prm.xorNumeric);
            break;
        }
        case Algo::Transposition: {
            transposition::Context ctx;
            steps_ = prm.decrypt ? transposition::decryptSteps(prm.text, prm.transKey, &ctx)
                                 : transposition::encryptSteps(prm.text, prm.transKey, &ctx);
            stack_->setCurrentWidget(transScene_);
            transScene_->setContext(ctx);
            break;
        }
    }
}

void MainWindow::showStep(bool animate) {
    Scene* scene = currentScene();
    if (pos_ == 0) {
        scene->setStep(nullptr, false);
        transport_->setState(0, total(), QStringLiteral("Готово. Нажмите «Шаг» (пробел), чтобы начать"));
    } else {
        const Step& step = steps_[pos_ - 1];
        scene->setStep(&step, animate);
        transport_->setState(pos_, total(), step.text);
    }
    tabbar_->setStep(pos_, total());
    QStringList texts;
    for (const auto& s : steps_) texts << s.text;
    transport_->setJournal(texts, pos_);
    if (pos_ >= total()) setPlaying(false);
}

void MainWindow::goTo(int pos) {
    pos_ = std::clamp(pos, 0, total());
    showStep(true);
}

void MainWindow::stepForward() {
    if (pos_ >= total()) return;
    ++pos_;
    if (majorOnly_)
        while (pos_ < total() && !steps_[pos_ - 1].major) ++pos_;
    showStep();
}

void MainWindow::stepBack() {
    if (pos_ <= 0) return;
    --pos_;
    if (majorOnly_)
        while (pos_ > 0 && !steps_[pos_ - 1].major) --pos_;
    showStep();
}

void MainWindow::setPlaying(bool playing) {
    if (playing && pos_ >= total()) {
        pos_ = 0;
        showStep(false);
    }
    if (playing && total() > 0) {
        timer_.start();
    } else {
        timer_.stop();
        playing = false;
    }
    transport_->setPlaying(playing);
}

void MainWindow::togglePlay() { setPlaying(!timer_.isActive()); }
