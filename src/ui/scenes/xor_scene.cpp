#include "xor_scene.h"

#include <QPen>

#include "../../core/xor_cipher.h"
#include "../draw.h"

using namespace draw;
using namespace theme;

void XorScene::setContext(const QByteArray& data, const QByteArray& key, bool wasHex, bool numericKey) {
    data_ = data;
    key_ = key;
    wasHex_ = wasHex;
    numericKey_ = numericKey;
    setStep(nullptr, false);
}

void XorScene::byteCell(QPainter& p, const QRectF& r, unsigned char b, const CellState& st, double alpha) {
    // Ячейка байта: символ сверху, hex снизу
    cell(p, r, {}, st, -1, alpha);
    const double h = r.height();
    text(p, QRectF(r.x(), r.y() + h * 0.08, r.width(), h * 0.52), xorc::byteRepr(b), int(h * 0.42),
         withAlpha(st.fg, alpha), QFont::DemiBold, Qt::AlignCenter, true);
    const QColor hexColor = (&st == &CELL_CURRENT) ? st.fg : MUTED;
    text(p, QRectF(r.x(), r.y() + h * 0.58, r.width(), h * 0.36), xorc::hex2(b), int(h * 0.24),
         withAlpha(hexColor, alpha), QFont::Normal, Qt::AlignCenter, true);
}

void XorScene::paintScene(QPainter& p, const QRectF& rect) {
    const XorStep* d = step_ ? std::get_if<XorStep>(&step_->data) : nullptr;
    const int bi = d ? d->byteIndex : -1, bit = d ? d->bitIndex : -1, ki = d ? d->keyIndex : -1;
    const double t = ease(progress_);
    const double m = 40, W = rect.width() - 2 * m;
    double y = 34;

    // ── Байты текста ──
    label(p, m, y, wasHex_ ? QStringLiteral("Шифртекст (hex)") : QStringLiteral("Текст · CP1251 · 1 символ = 1 байт"));
    y += 12;
    auto cells = layoutRow(m, y, data_.size(), W);
    for (int i = 0; i < cells.size(); ++i) {
        const bool done = i < bi || (i == bi && d && d->byteDone && t > 0.9);
        byteCell(p, cells[i], data_[i], done ? CELL_DONE : i == bi ? CELL_CURRENT : CELL_FUTURE);
    }
    const double cellH = cells.isEmpty() ? 60 : cells[0].height();
    y += cellH + 34;

    // ── Байты ключа ──
    label(p, m, y, (key_.size() == 1 && numericKey_) ? QStringLiteral("Ключ · число 0–255")
                                                      : QStringLiteral("Ключ · %1 байт · повторяется циклически").arg(key_.size()));
    y += 12;
    auto kcells = layoutRow(m, y, key_.size(), W);
    for (int i = 0; i < kcells.size(); ++i) byteCell(p, kcells[i], key_[i], i == ki ? CELL_KEY_CURRENT : CELL_KEY);
    if (d && key_.size() > 1)
        text(p, QRectF(kcells.last().right() + 16, y, 300, kcells[0].height()),
             QStringLiteral("ключ[%1 mod %2 = %3]").arg(bi).arg(key_.size()).arg(ki), 15, ACCENT, QFont::Normal, leftV(), true);
    y += kcells[0].height() + 40;

    // ── Побитовая панель ──
    const double panelH = 268, truthW = 230;
    const QRectF panel(m, y, W - truthW - 20, panelH);
    const QRectF truth(panel.right() + 20, y, truthW, panelH);
    card(p, panel, QStringLiteral("Побитовая операция"));
    card(p, truth, QStringLiteral("Таблица истинности"));

    const double labelW = 240, bitsX = panel.x() + labelW, bitsAvail = panel.width() - labelW - 24, rowGap = 70;
    const double rowsY[3] = {panel.y() + 44, panel.y() + 44 + rowGap, panel.y() + 44 + 2 * rowGap};
    QVector<QRectF> rows[3];
    for (int k = 0; k < 3; ++k) rows[k] = layoutRow(bitsX, rowsY[k], 8, bitsAvail, 56, 8);

    if (d) {  // вертикальная подсветка текущего бита
        const QRectF col = rows[0][bit];
        const QRectF band(col.x() - 6, rowsY[0] - 8, col.width() + 12, rowsY[2] + rows[2][0].height() - rowsY[0] + 16);
        p.setPen(Qt::NoPen);
        p.setBrush(withAlpha(AMBER, 0.08));
        p.drawRoundedRect(band, 10, 10);
    }

    auto rowLabel = [&](int k, const QString& title, const QString& value, const QColor& c) {
        const QRectF r(panel.x() + 16, rowsY[k], labelW - 64, rows[k][0].height());
        text(p, QRectF(r.x(), r.y(), r.width(), r.height() / 2), title, 12, MUTED, QFont::Bold, leftV());
        text(p, QRectF(r.x(), r.y() + r.height() / 2, r.width(), r.height() / 2), value, 13, c, QFont::DemiBold, leftV(), true);
    };
    int rbPartial = 0;
    bool doneAll = false;
    if (d) {
        doneAll = true;
        for (int i = 0; i < 8; ++i) {
            if (d->rBits[i] < 0) doneAll = false;
            else rbPartial |= d->rBits[i] << (7 - i);
        }
        auto fmt = [](unsigned char b) {
            return QStringLiteral("‘%1’ = %2 = 0x%3").arg(xorc::byteRepr(b)).arg(b, 3).arg(xorc::hex2(b));
        };
        rowLabel(0, QStringLiteral("ТЕКСТ"), fmt(d->tByte), TEXT);
        rowLabel(1, QStringLiteral("КЛЮЧ"), fmt(d->kByte), ACCENT);
        rowLabel(2, QStringLiteral("РЕЗУЛЬТАТ"), doneAll ? fmt(rbPartial) : QStringLiteral("…"), GREEN);
    } else {
        rowLabel(0, QStringLiteral("ТЕКСТ"), "—", MUTED);
        rowLabel(1, QStringLiteral("КЛЮЧ"), "—", MUTED);
        rowLabel(2, QStringLiteral("РЕЗУЛЬТАТ"), "—", MUTED);
    }
    for (int i = 0; i < 8; ++i) {
        cell(p, rows[0][i], d ? QString::number(d->tBits[i]) : QString(),
             i == bit ? CELL_CURRENT : i < bit ? CELL_DONE : CELL_FUTURE);
        cell(p, rows[1][i], d ? QString::number(d->kBits[i]) : QString(), i == bit ? CELL_KEY_CURRENT : CELL_KEY);
        const int v = d ? d->rBits[i] : -1;
        const QRectF& r = rows[2][i];
        if (v < 0) {
            cell(p, r, {}, CELL_EMPTY);
        } else if (i == bit) {  // появление результата: масштаб + прозрачность
            const double s = 0.6 + 0.4 * t;
            const QRectF rr(r.center().x() - r.width() * s / 2, r.center().y() - r.height() * s / 2, r.width() * s, r.height() * s);
            cell(p, rr, QString::number(v), CELL_DONE, int(r.height() * 0.42), 0.3 + 0.7 * t);
        } else {
            cell(p, r, QString::number(v), CELL_DONE);
        }
    }
    const double opX = bitsX - 32;
    text(p, QRectF(opX - 14, rowsY[0] + rows[0][0].height() / 2 + rowGap / 2 - 14, 28, 28), "⊕", 22, WARM, QFont::Bold);
    text(p, QRectF(opX - 14, rowsY[1] + rows[1][0].height() / 2 + rowGap / 2 - 14, 28, 28), "=", 22, GREEN, QFont::Bold);
    const double sepY = rowsY[2] - 10;
    p.setPen(QPen(BORDER, 1));
    p.drawLine(QPointF(bitsX, sepY), QPointF(rows[2].last().right(), sepY));
    for (int i = 0; i < 8; ++i)
        text(p, QRectF(rows[2][i].x(), rows[2][i].bottom() + 4, rows[2][i].width(), 14),
             QStringLiteral("бит %1").arg(7 - i), 9, DIM);

    // таблица истинности
    const int table[4][3] = {{0, 0, 0}, {0, 1, 1}, {1, 0, 1}, {1, 1, 0}};
    const double tx = truth.x() + 18, ty = truth.y() + 40;
    text(p, QRectF(tx, ty, 60, 22), "a", 13, MUTED, QFont::Bold, Qt::AlignCenter, true);
    text(p, QRectF(tx + 64, ty, 60, 22), "b", 13, MUTED, QFont::Bold, Qt::AlignCenter, true);
    text(p, QRectF(tx + 128, ty, 70, 22), "a ⊕ b", 13, MUTED, QFont::Bold, Qt::AlignCenter, true);
    for (int k = 0; k < 4; ++k) {
        const double ry = ty + 30 + k * 46;
        const bool active = d && d->tBits[bit] == table[k][0] && d->kBits[bit] == table[k][1];
        if (active) {
            p.setPen(QPen(WARM, 1));
            p.setBrush(withAlpha(WARM, 0.12));
            p.drawRoundedRect(QRectF(tx - 6, ry - 4, truth.width() - 24, 40), 8, 8);
        }
        const QColor c = active ? TEXT : MUTED;
        text(p, QRectF(tx, ry, 60, 32), QString::number(table[k][0]), 20, c, QFont::DemiBold, Qt::AlignCenter, true);
        text(p, QRectF(tx + 64, ry, 60, 32), QString::number(table[k][1]), 20, c, QFont::DemiBold, Qt::AlignCenter, true);
        text(p, QRectF(tx + 128, ry, 70, 32), QString::number(table[k][2]), 20, active ? GREEN : c, QFont::Bold, Qt::AlignCenter, true);
    }
    text(p, QRectF(truth.x(), truth.bottom() - 34, truth.width(), 24), QStringLiteral("1, если биты различаются"), 12, MUTED);
    y += panelH + 36;

    // ── Результат ──
    label(p, m, y, QStringLiteral("Результат · hex и символ"));
    y += 12;
    const QByteArray out = d ? d->output : QByteArray();
    cells = layoutRow(m, y, data_.size(), W);
    for (int i = 0; i < cells.size(); ++i) {
        if (i < out.size() - 1) byteCell(p, cells[i], out[i], CELL_DONE);
        else if (i == out.size() - 1) byteCell(p, cells[i], out[i], CELL_CURRENT, 0.35 + 0.65 * t);
        else cell(p, cells[i], {}, CELL_EMPTY);
    }
}
