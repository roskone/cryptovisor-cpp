// Сцена шифра Цезаря: строка текста, лента алфавита со стрелкой сдвига, формула, результат.
#pragma once
#include "scene.h"

class CaesarScene : public Scene {
    Q_OBJECT
public:
    using Scene::Scene;
    void setContext(const QString& text, int shift);

protected:
    void paintScene(QPainter& p, const QRectF& rect) override;

private:
    QString text_;
    int shift_ = 0;
};
