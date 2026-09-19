// Главное окно: связывает панель ввода, сцены и транспорт; обрабатывает горячие клавиши.
#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QTimer>
#include <vector>

#include "../core/steps.h"
#include "scenes/caesar_scene.h"
#include "scenes/transposition_scene.h"
#include "scenes/xor_scene.h"
#include "sidebar.h"
#include "tabbar.h"
#include "transport.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();

    void rebuild();
    void goTo(int pos);
    void stepForward();
    void stepBack();
    void setPlaying(bool playing);
    void togglePlay();
    void toggleFullscreen();

    int pos() const { return pos_; }
    int total() const { return int(steps_.size()); }
    Sidebar* sidebar() const { return sidebar_; }

private:
    std::vector<Step> steps_;
    int pos_ = 0;
    bool majorOnly_ = false;

    Sidebar* sidebar_;
    TabBar* tabbar_;
    QStackedWidget* stack_;
    CaesarScene* caesarScene_;
    XorScene* xorScene_;
    TranspositionScene* transScene_;
    Transport* transport_;
    QTimer timer_;

    Scene* currentScene() const { return static_cast<Scene*>(stack_->currentWidget()); }
    void setupShortcuts();
    void escape();
    void build(const Params& prm);
    void showStep(bool animate = true);
};
