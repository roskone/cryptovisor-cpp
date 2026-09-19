// Нижняя панель: вкладки Шаг / Журнал / Клавиши, управление воспроизведением.
#pragma once
#include <QFrame>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>

class Transport : public QFrame {
    Q_OBJECT
public:
    explicit Transport(QWidget* parent = nullptr);

    int intervalMs() const;
    void nudgeSpeed(int delta);
    void setState(int pos, int total, const QString& text, bool error = false);
    void setJournal(const QStringList& texts, int pos);
    void clearJournal() { journal_->clear(); }
    void setPlaying(bool playing);

signals:
    void first();
    void prev();
    void next();
    void last();
    void playToggled(bool on);
    void speedChanged(int ms);
    void jump(int pos);

private:
    QStackedWidget* pages_;
    QLabel* stepText_;
    QListWidget* journal_;
    QPushButton *btnFirst_, *btnPrev_, *btnPlay_, *btnNext_, *btnLast_;
    QLabel* counter_;
    QSlider* speed_;

    static QPushButton* button(const QString& text, const QString& tip);
};
