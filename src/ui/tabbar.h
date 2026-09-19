// Панель вкладок над сценой: алгоритмы, пилюля с номером шага, переключатель режима.
#pragma once
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <map>

#include "sidebar.h"

class TabBar : public QFrame {
    Q_OBJECT
public:
    explicit TabBar(QWidget* parent = nullptr);
    void sync(Algo algo, bool decrypt);
    void setStep(int pos, int total);

signals:
    void algoSelected(Algo a);
    void modeSelected(bool decrypt);

private:
    std::map<Algo, QPushButton*> tabs_;
    QPushButton *btnEnc_, *btnDec_;
    QLabel* pill_;
};
