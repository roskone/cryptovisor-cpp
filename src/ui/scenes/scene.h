// Базовый класс сцены: хранит текущий шаг и анимирует переход между шагами.
#pragma once
#include <QVariantAnimation>
#include <QWidget>

#include "../../core/steps.h"

class Scene : public QWidget {
    Q_OBJECT
public:
    explicit Scene(QWidget* parent = nullptr);

    void clearContext();                                    // нет данных — показать подсказку
    void setStep(const Step* step, bool animate = true);    // nullptr — начальное состояние

protected:
    bool hasContext_ = false;
    const Step* step_ = nullptr;
    double progress_ = 1.0;                                 // 0..1, ход анимации текущего шага

    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent* e) override;
    virtual void paintScene(QPainter& p, const QRectF& rect) = 0;

private:
    QVariantAnimation anim_;
};
