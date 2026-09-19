// Проверка горячих клавиш и авто-режима через QTest (offscreen).
#include <QApplication>
#include <QTest>
#include <cstdio>

#include "ui/main_window.h"
#include "ui/theme.h"

static int failures = 0;
#define CHECK(c) do { if (!(c)) { std::printf("FAIL %d: %s\n", __LINE__, #c); ++failures; } } while (0)

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    app.setStyleSheet(theme::qss());
    MainWindow w;
    w.show();
    app.processEvents();
    auto key = [&](Qt::Key k) { QTest::keyClick(&w, k); app.processEvents(); };
    key(Qt::Key_Space); CHECK(w.pos() == 1);
    key(Qt::Key_Right); CHECK(w.pos() == 2);
    key(Qt::Key_Left);  CHECK(w.pos() == 1);
    key(Qt::Key_End);   CHECK(w.pos() == 12);
    key(Qt::Key_Home);  CHECK(w.pos() == 0);
    key(Qt::Key_2);     CHECK(w.sidebar()->algo() == Algo::Xor); CHECK(w.total() == 96);
    key(Qt::Key_M);     key(Qt::Key_Space); CHECK(w.pos() == 8);
    key(Qt::Key_Space); CHECK(w.pos() == 16);
    key(Qt::Key_Left);  CHECK(w.pos() == 8);
    key(Qt::Key_D);     CHECK(w.sidebar()->params().decrypt);
    key(Qt::Key_Return); QTest::qWait(1400); CHECK(w.pos() > 0);  // интервал при скорости 5 ≈ 1.2 с
    key(Qt::Key_Return);
    // ввод в поле не должен срабатывать как горячие клавиши
    w.sidebar()->textEdit()->setFocus();
    w.sidebar()->textEdit()->clear();
    QTest::keyClicks(w.sidebar()->textEdit(), "ab 1"); app.processEvents();
    CHECK(w.sidebar()->textEdit()->text() == "ab 1"); CHECK(w.sidebar()->algo() == Algo::Xor);
    key(Qt::Key_Escape); CHECK(QApplication::focusWidget() != w.sidebar()->textEdit());
    key(Qt::Key_3); w.sidebar()->setDecrypt(true); w.sidebar()->textEdit()->setText("abcde"); app.processEvents();
    CHECK(w.total() == 0);  // ошибка: длина не кратна ключу
    std::printf(failures ? "FAILED\n" : "OK — клавиши работают\n");
    return failures ? 1 : 0;
}
