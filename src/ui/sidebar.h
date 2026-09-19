// Левая панель в стиле дерева проекта: секции с «▾», строки-элементы, поля ввода.
#pragma once
#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <map>

enum class Algo { Caesar, Xor, Transposition };

struct Params {
    Algo algo = Algo::Caesar;
    QString text;
    bool decrypt = false;
    int caesarShift = 3;
    bool xorNumeric = false;
    int xorKeyNum = 42;
    QString xorKeyStr;
    QString transKey;
    bool majorSteps = false;
};

struct Hotkey { const char* keys; const char* desc; };
inline const Hotkey HOTKEYS[] = {
    {"Пробел / →", "шаг вперёд"}, {"←", "шаг назад"}, {"Home / End", "в начало / в конец"},
    {"Enter", "авто-режим"}, {"+ / −", "быстрее / медленнее"}, {"1 · 2 · 3", "алгоритм"},
    {"E / D", "шифр / дешифр"}, {"M", "крупные шаги"}, {"F", "полный экран"}, {"Esc", "выйти из поля"},
};

// Секция дерева: заголовок «▾ НАЗВАНИЕ» + содержимое с отступом, линия-разделитель снизу.
class Section : public QWidget {
    Q_OBJECT
public:
    explicit Section(const QString& title, QWidget* parent = nullptr);
    void add(QWidget* w);
    void add(QLayout* l);

private:
    QVBoxLayout* body_;
};

class Sidebar : public QFrame {
    Q_OBJECT
public:
    explicit Sidebar(QWidget* parent = nullptr);

    Params params() const;
    Algo algo() const;
    void setAlgo(Algo a);
    void setDecrypt(bool dec);
    void toggleMajor();
    void setTextHint(const QString& text);

    QLineEdit* textEdit() const { return text_; }

signals:
    void changed();

private:
    std::map<Algo, QPushButton*> algoButtons_;
    QPushButton *btnEnc_, *btnDec_;
    QLineEdit* text_;
    QLabel* textHint_;
    QWidget* pages_[3];
    QSpinBox* caesarShift_;
    QRadioButton *xorNum_, *xorStr_;
    QSpinBox* xorKeyNum_;
    QLineEdit* xorKeyStr_;
    QLineEdit* transKey_;
    QCheckBox* major_;

    void showPage(Algo a);
    void syncXor();
};
