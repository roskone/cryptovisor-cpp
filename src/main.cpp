// Точка входа: Криптовизор — интерактивный визуализатор шифров.
#include <QApplication>
#include <QDirIterator>
#include <QFont>
#include <QFontDatabase>
#include <QIcon>

#include "ui/main_window.h"
#include "ui/theme.h"

static void loadFonts() {
    QDirIterator it(QStringLiteral(":/fonts"), {QStringLiteral("*.ttf")});
    while (it.hasNext()) QFontDatabase::addApplicationFont(it.next());
}

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Криптовизор"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icon.png")));
    app.setStyle(QStringLiteral("Fusion"));
    loadFonts();
    QFont f(theme::FONT);
    f.setPixelSize(12);
    app.setFont(f);
    app.setStyleSheet(theme::qss());
    MainWindow win;
    win.show();
    return app.exec();
}
