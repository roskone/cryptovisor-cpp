#include "caesar_scene.h"

#include <cmath>

#include "../../core/caesar.h"
#include "../draw.h"

using namespace draw;
using namespace theme;

void CaesarScene::setContext(const QString& text, int shift) {
    text_ = text;
    shift_ = shift;
    setStep(nullptr, false);
}

void CaesarScene::paintScene(QPainter& p, const QRectF& rect) {
    const CaesarStep* d = step_ ? std::get_if<CaesarStep>(&step_->data) : nullptr;
    const int idx = d ? d->index : -1;
    const double t = ease(progress_);
    const double m = 40, W = rect.width() - 2 * m;
    double y = 34;

    // ── Исходный текст ──
    label(p, m, y, QStringLiteral("Исходный текст"));
    y += 12;
    auto cells = layoutRow(m, y, text_.size(), W);
    for (int i = 0; i < cells.size(); ++i)
        cell(p, cells[i], QString(text_[i]), i < idx ? CELL_DONE : i == idx ? CELL_CURRENT : CELL_FUTURE);
    const double cellH = cells.isEmpty() ? 60 : cells[0].height();
    y += cellH + 40;

    // ── Лента алфавита ──
    const bool isLetter = d && d->isLetter;
    const QString alphabet = isLetter ? d->alphabet : caesar::LATIN;
    const double cardH = 150;
    const QRectF cardR(m, y, W, cardH);
    const QString sign = shift_ >= 0 ? "+" : "−";
    card(p, cardR, QStringLiteral("Алфавит (%1 букв) · сдвиг %2%3").arg(alphabet.size()).arg(sign).arg(std::abs(shift_)));
    const double stripY = cardR.y() + 78;
    auto strip = layoutRow(cardR.x() + 18, stripY, alphabet.size(), cardR.width() - 36, 44, 4);
    const int posSrc = isLetter ? d->posSrc : -1, posDst = isLetter ? d->posDst : -1;
    for (int i = 0; i < strip.size(); ++i) {
        const CellState& st = i == posSrc ? CELL_CURRENT : (i == posDst && t > 0.85) ? CELL_DONE : CELL_FUTURE;
        cell(p, strip[i], QString(alphabet[i]), st, int(strip[i].height() * 0.48));
        text(p, QRectF(strip[i].x(), strip[i].bottom() + 2, strip[i].width(), 14), QString::number(i), 10, MUTED);
    }
    if (isLetter) {
        const int n = alphabet.size();
        const double cur = posSrc + shift_ * t;
        const bool wrappedNow = cur >= n || cur < 0;
        const double curMod = std::fmod(std::fmod(cur, n) + n, n);
        const QPointF srcC(strip[posSrc].center().x(), strip[posSrc].top() - 4);
        const double cellW = strip[0].width() + 4;
        const QPointF dstC(strip[0].center().x() + curMod * cellW, strip[0].top() - 4);
        if (wrappedNow) {
            const double edgeX = shift_ > 0 ? strip.last().right() + 6 : strip[0].left() - 6;
            arrow(p, srcC, QPointF(edgeX, srcC.y()), withAlpha(WARM, 0.5), 2.5, 36);
            const double startX = shift_ > 0 ? strip[0].left() - 6 : strip.last().right() + 6;
            arrow(p, QPointF(startX, srcC.y()), dstC, WARM, 2.5, 36);
            text(p, QRectF(cardR.x(), cardR.y() + 30, cardR.width(), 20),
                 QStringLiteral("переход через край: mod %1").arg(n), 13, WARM, QFont::DemiBold);
        } else {
            arrow(p, srcC, dstC, WARM, 2.5, 36);
        }
        const double midX = wrappedNow ? cardR.center().x() : (srcC.x() + dstC.x()) / 2;
        badge(p, QPointF(midX, stripY - 34), sign + QString::number(std::abs(shift_)), WARM, BG, 14);
    } else if (d) {
        text(p, QRectF(cardR.x(), cardR.y() + 30, cardR.width(), 24),
             QStringLiteral("символ не входит в алфавит — сдвиг не применяется"), 14, MUTED);
    }
    y += cardH + 24;

    // ── Формула и коды ──
    const QRectF left(m, y, W * 0.58 - 10, 132);
    const QRectF right(left.right() + 20, y, W - left.width() - 20, 132);
    card(p, left, QStringLiteral("Вычисление"));
    card(p, right, QStringLiteral("Коды символов"));
    if (d) {
        struct Line { QString s; QColor c; };
        QVector<Line> lines;
        if (isLetter) {
            const int n = alphabet.size();
            lines = {{QStringLiteral("‘%1’  →  позиция %2").arg(d->src).arg(posSrc), TEXT},
                     {QStringLiteral("(%1 %2 %3) mod %4  =  %5").arg(posSrc).arg(sign).arg(std::abs(shift_)).arg(n).arg(posDst), WARM},
                     {QStringLiteral("позиция %1  →  ‘%2’").arg(posDst).arg(d->dst), GREEN}};
        } else {
            lines = {{QStringLiteral("‘%1’ — не буква").arg(d->src), TEXT},
                     {QStringLiteral("остаётся как есть"), MUTED},
                     {QStringLiteral("→ ‘%1’").arg(d->dst), GREEN}};
        }
        for (int k = 0; k < lines.size(); ++k)
            text(p, QRectF(left.x() + 18, left.y() + 30 + k * 32, left.width() - 36, 30), lines[k].s, 21, lines[k].c,
                 QFont::DemiBold, leftV(), true);
        struct Row { QString lab; int code; QColor c; };
        const Row rows[] = {{QStringLiteral("‘%1’").arg(d->src), d->codeSrc, TEXT},
                            {QStringLiteral("‘%1’").arg(d->dst), d->codeDst, GREEN}};
        for (int k = 0; k < 2; ++k) {
            const double ry = right.y() + 36 + k * 42;
            text(p, QRectF(right.x() + 18, ry, 50, 30), rows[k].lab, 20, rows[k].c, QFont::DemiBold, leftV(), true);
            text(p, QRectF(right.x() + 70, ry, 90, 30), QString::number(rows[k].code).rightJustified(5), 18, MUTED,
                 QFont::Normal, leftV(), true);
            const QString bits = QString::number(rows[k].code, 2).rightJustified(rows[k].code < 256 ? 8 : 16, '0');
            text(p, QRectF(right.x() + 160, ry, right.width() - 178, 30), bits, 18, rows[k].c, QFont::Normal, leftV(), true);
        }
    } else {
        text(p, QRectF(left.x(), left.y() + 20, left.width(), left.height() - 20),
             QStringLiteral("Нажмите «Шаг» или пробел"), 16, MUTED);
    }
    y += 132 + 36;

    // ── Результат ──
    label(p, m, y, QStringLiteral("Результат"));
    y += 12;
    const QString out = d ? d->output : QString();
    cells = layoutRow(m, y, text_.size(), W);
    for (int i = 0; i < cells.size(); ++i) {
        if (i < out.size() - 1) cell(p, cells[i], QString(out[i]), CELL_DONE);
        else if (i == out.size() - 1) cell(p, cells[i], QString(out[i]), CELL_CURRENT, -1, 0.35 + 0.65 * t);
        else cell(p, cells[i], {}, CELL_EMPTY);
    }
}
