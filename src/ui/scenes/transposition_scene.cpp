#include "transposition_scene.h"

#include <QPen>
#include <algorithm>

#include "../draw.h"

using namespace draw;
using namespace theme;

void TranspositionScene::setContext(const transposition::Context& ctx) {
    ctx_ = ctx;
    setStep(nullptr, false);
}

// Была ли ячейка (r, c) уже прочитана до текущей ячейки cur.
bool TranspositionScene::alreadyRead(const Cell& cur, int r, int c) const {
    if (!ctx_.decrypt) {
        const int ic = ctx_.order.indexOf(c), icc = ctx_.order.indexOf(cur.c);
        return ic < icc || (c == cur.c && r < cur.r);
    }
    return r < cur.r || (r == cur.r && c < cur.c);
}

void TranspositionScene::paintScene(QPainter& p, const QRectF& rect) {
    const TransStep* d = step_ ? std::get_if<TransStep>(&step_->data) : nullptr;
    const auto& headers = ctx_.headers;
    const auto& ranks = ctx_.ranks;
    const auto& order = ctx_.order;
    const int rows = ctx_.rows, cols = ctx_.cols;
    const QString& source = ctx_.source;
    const bool dec = ctx_.decrypt;
    const double t = ease(progress_);
    const double m = 40, W = rect.width() - 2 * m;
    double y = 34;

    // ── Исходная строка ──
    const int srcIndex = d ? d->srcIndex : -1;
    int nSrcDone = 0;
    if (d) {
        if (d->phase == Phase::Fill) nSrcDone = srcIndex;  // текущий ещё «летит»
        else if (d->phase == Phase::Read) nSrcDone = source.size();
        else nSrcDone = dec ? 0 : source.size();
    }
    label(p, m, y, dec ? QStringLiteral("Шифртекст") : QStringLiteral("Исходный текст"));
    y += 12;
    auto srcCells = layoutRow(m, y, source.size(), W);
    for (int i = 0; i < srcCells.size(); ++i) {
        if (i < nSrcDone) cell(p, srcCells[i], QString(source[i]), CELL_DONE);
        else if (i == srcIndex) cell(p, srcCells[i], QString(source[i]), CELL_CURRENT, -1, 1 - 0.6 * t);
        else cell(p, srcCells[i], QString(source[i]), CELL_FUTURE);
    }
    const double cellH = srcCells.isEmpty() ? 60 : srcCells[0].height();
    y += cellH + 34;

    // ── Таблица ──
    const double availH = rect.height() - y - 150, sideW = 300;
    const QRectF tableArea(m, y, W - sideW - 20, availH);
    const QRectF side(tableArea.right() + 20, y, sideW, availH);
    card(p, tableArea, QStringLiteral("Таблица %1 × %2").arg(rows).arg(cols));
    card(p, side, QStringLiteral("Порядок столбцов по ключу"));

    const double gap = 8, headerH = 74;
    double c = std::min({88.0, (tableArea.width() - 40 - gap * (cols - 1)) / cols,
                         (tableArea.height() - headerH - 60 - gap * (rows - 1)) / std::max(rows, 1)});
    c = std::max(c, 18.0);
    const double gridW = cols * c + (cols - 1) * gap;
    const double gx = tableArea.center().x() - gridW / 2, gy = tableArea.y() + 36 + headerH;
    const int ranksShown = d ? d->ranksShown : 0;
    const int activeCol = d ? d->col : -1;
    const bool hasCur = d && d->cell.has_value();
    const Cell cur = hasCur ? *d->cell : Cell{-1, -1};
    auto cellRect = [&](int r, int cc) { return QRectF(gx + cc * (c + gap), gy + r * (c + gap), c, c); };
    const Phase phase = d ? d->phase : Phase::Fill;

    // заголовки: буква ключа + номер (если уже присвоен)
    for (int cc = 0; cc < cols; ++cc) {
        const QRectF hr(gx + cc * (c + gap), gy - headerH, c, headerH - 10);
        const bool rankVisible = ranks[cc] < ranksShown;
        const bool isActive = phase == Phase::Order && activeCol == cc;
        const CellState& st = (isActive || (phase == Phase::Read && activeCol == cc)) ? CELL_KEY_CURRENT : CELL_KEY;
        cell(p, QRectF(hr.x(), hr.y(), hr.width(), hr.height() * 0.62), QString(headers[cc]), st, int(c * 0.42));
        if (rankVisible)
            badge(p, QPointF(hr.center().x(), hr.bottom() - 4), QString::number(ranks[cc] + 1),
                  withAlpha(AMBER, isActive ? t : 1.0), BG, 12);
    }
    // подсветка активного столбца
    if (activeCol >= 0 && (phase == Phase::Read || (phase == Phase::Fill && dec))) {
        const QColor col = phase == Phase::Read ? GREEN : AMBER;
        const QRectF band = cellRect(0, activeCol).united(cellRect(rows - 1, activeCol)).adjusted(-5, -5, 5, 5);
        p.setPen(QPen(withAlpha(col, 0.5), 1));
        p.setBrush(withAlpha(col, phase == Phase::Read ? 0.07 : 0.06));
        p.drawRoundedRect(band, 10, 10);
    }

    const QString out = d ? d->output : QString();
    for (int r = 0; r < rows; ++r) {
        for (int cc = 0; cc < cols; ++cc) {
            const QRectF rr = cellRect(r, cc);
            const QChar ch = d ? d->grid[r][cc] : QChar();
            if (ch.isNull()) { cell(p, rr, {}, CELL_EMPTY); continue; }
            const bool isCur = hasCur && cur.r == r && cur.c == cc;
            if (phase == Phase::Fill && isCur) {  // символ прилетает из исходной строки
                const QPointF pos = lerp(srcCells[srcIndex].center(), rr.center(), t);
                cell(p, rr, {}, CELL_EMPTY);
                cell(p, QRectF(pos.x() - rr.width() / 2, pos.y() - rr.height() / 2, rr.width(), rr.height()), QString(ch), CELL_CURRENT);
            } else if (phase == Phase::Read && isCur) {
                cell(p, rr, QString(ch), CELL_CURRENT);
            } else if (phase == Phase::Read && hasCur && alreadyRead(cur, r, cc)) {
                cell(p, rr, QString(ch), CELL_DONE, -1, 0.55);
            } else {
                cell(p, rr, QString(ch), CELL_DONE);
            }
        }
    }

    // ── Боковая панель: порядок ──
    const double sx = side.x() + 18;
    double sy = side.y() + 42;
    QString keyStr;
    for (const QChar h : headers) keyStr += h;
    text(p, QRectF(sx, sy, side.width() - 36, 20), QStringLiteral("ключ: %1").arg(keyStr), 14, MUTED, QFont::Normal, leftV(), true);
    sy += 30;
    for (int k = 0; k < order.size(); ++k) {
        const int cc = order[k];
        const bool visible = k < ranksShown;
        const double ry = sy + k * 36;
        if (activeCol == cc && d) {
            p.setPen(Qt::NoPen);
            p.setBrush(withAlpha(AMBER, 0.12));
            p.drawRoundedRect(QRectF(sx - 8, ry - 3, side.width() - 20, 32), 8, 8);
        }
        badge(p, QPointF(sx + 12, ry + 13), QString::number(k + 1), visible ? AMBER : BORDER, visible ? BG : MUTED, 12);
        text(p, QRectF(sx + 34, ry, side.width() - 60, 26),
             visible ? QStringLiteral("столбец ‘%1’  (№%2 слева)").arg(headers[cc]).arg(cc + 1) : QStringLiteral("?"),
             15, visible ? TEXT : MUTED, QFont::Normal, leftV());
    }
    text(p, QRectF(side.x() + 14, side.bottom() - 62, side.width() - 28, 50),
         dec ? QStringLiteral("Шифртекст заполняет столбцы\nв порядке номеров, читаем по строкам")
             : QStringLiteral("Столбцы читаются сверху вниз\nв порядке номеров"), 12, MUTED);
    y = tableArea.bottom() + 34;

    // ── Результат ──
    label(p, m, y, QStringLiteral("Результат"));
    y += 12;
    auto outCells = layoutRow(m, y, source.size(), W);
    for (int i = 0; i < outCells.size(); ++i) {
        const QRectF& r = outCells[i];
        if (i < out.size() - 1) {
            cell(p, r, QString(out[i]), CELL_DONE);
        } else if (i == out.size() - 1 && phase == Phase::Read && hasCur) {
            const QPointF pos = lerp(cellRect(cur.r, cur.c).center(), r.center(), t);
            cell(p, r, {}, CELL_EMPTY);
            cell(p, QRectF(pos.x() - r.width() / 2, pos.y() - r.height() / 2, r.width(), r.height()), QString(out[i]), CELL_CURRENT);
        } else {
            cell(p, r, {}, CELL_EMPTY);
        }
    }
}
