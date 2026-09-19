#include "scene.h"

#include <QEasingCurve>
#include <QPainter>

#include "../draw.h"

Scene::Scene(QWidget* parent) : QWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);
    setMinimumSize(640, 420);
    anim_.setStartValue(0.0);
    anim_.setEndValue(1.0);
    anim_.setDuration(380);
    anim_.setEasingCurve(QEasingCurve::OutCubic);
    connect(&anim_, &QVariantAnimation::valueChanged, this, [this](const QVariant& v) {
        progress_ = v.toDouble();
        update();
    });
}

void Scene::clearContext() {
    hasContext_ = false;
    step_ = nullptr;
    anim_.stop();
    progress_ = 1.0;
    update();
}

void Scene::setStep(const Step* step, bool animate) {
    hasContext_ = true;
    step_ = step;
    anim_.stop();
    if (animate && step) {
        progress_ = 0.0;
        anim_.start();
    } else {
        progress_ = 1.0;
    }
    update();
}

void Scene::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    const QRectF r(rect());
    draw::background(p, r);
    if (!hasContext_) {
        draw::text(p, r, QStringLiteral("Введите данные слева, чтобы начать"), 18, theme::MUTED);
        return;
    }
    paintScene(p, r);
}

void Scene::mousePressEvent(QMouseEvent* e) {
    setFocus();
    QWidget::mousePressEvent(e);
}
