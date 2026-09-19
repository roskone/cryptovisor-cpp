// Сцена XOR: байты текста и ключа, побитовая панель 3×8, таблица истинности, результат.
#pragma once
#include "../theme.h"
#include "scene.h"

class XorScene : public Scene {
    Q_OBJECT
public:
    using Scene::Scene;
    void setContext(const QByteArray& data, const QByteArray& key, bool wasHex, bool numericKey);

protected:
    void paintScene(QPainter& p, const QRectF& rect) override;

private:
    QByteArray data_, key_;
    bool wasHex_ = false, numericKey_ = false;
    static void byteCell(QPainter& p, const QRectF& r, unsigned char b, const theme::CellState& st, double alpha = 1.0);
};
