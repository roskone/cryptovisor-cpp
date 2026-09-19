// Сцена столбцовой перестановки: исходная строка, таблица с ключом, порядок чтения, результат.
#pragma once
#include "../../core/transposition.h"
#include "scene.h"

class TranspositionScene : public Scene {
    Q_OBJECT
public:
    using Scene::Scene;
    void setContext(const transposition::Context& ctx);

protected:
    void paintScene(QPainter& p, const QRectF& rect) override;

private:
    transposition::Context ctx_;
    bool alreadyRead(const Cell& cur, int r, int c) const;
};
