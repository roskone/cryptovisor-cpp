// Служебная утилита: рендерит окно на заданных шагах в PNG (для проверки без запуска GUI).
// cryptovisor_shots <out_dir>  — сохраняет caesar.png, xor.png, trans.png
#include <QApplication>
#include <QDirIterator>
#include <QFontDatabase>
#include <QThread>

#include "ui/main_window.h"
#include "ui/theme.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QDirIterator it(QStringLiteral(":/fonts"), {QStringLiteral("*.ttf")});
    while (it.hasNext()) QFontDatabase::addApplicationFont(it.next());
    app.setStyle(QStringLiteral("Fusion"));
    QFont f(theme::FONT);
    f.setPixelSize(12);
    app.setFont(f);
    app.setStyleSheet(theme::qss());
    const QString out = argc > 1 ? argv[1] : ".";
    MainWindow w;
    w.resize(1440, 900);
    w.show();
    app.processEvents();
    struct { const char* name; Algo algo; int pos; bool dec; } shots[] = {
        {"caesar", Algo::Caesar, 4, false}, {"xor", Algo::Xor, 16, false}, {"trans", Algo::Transposition, 20, false},
        {"trans_dec", Algo::Transposition, 10, true}};
    for (const auto& s : shots) {
        w.sidebar()->setDecrypt(s.dec);
        w.sidebar()->setAlgo(s.algo);
        w.goTo(s.pos);
        app.processEvents();
        // дождаться конца анимации
        for (int i = 0; i < 60; ++i) { QThread::msleep(10); app.processEvents(); }
        w.grab().save(out + "/" + s.name + ".png");
    }
    // прогон всех шагов всех режимов — не должно падать
    for (Algo a : {Algo::Caesar, Algo::Xor, Algo::Transposition})
        for (bool dec : {false, true}) {
            w.sidebar()->setDecrypt(dec);
            w.sidebar()->setAlgo(a);
            for (int i = 0; i <= w.total(); ++i) { w.goTo(i); w.grab(); }
        }
    return 0;
}
