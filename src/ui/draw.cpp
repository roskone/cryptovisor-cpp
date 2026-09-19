#include "draw.h"

#include <QFontDatabase>
#include <QPainterPath>
#include <QPen>
#include <algorithm>
#include <cmath>

namespace draw {

QFont font(int px, QFont::Weight weight, bool mono) {
    QFont f;
    if (mono) {
        f.setFamilies({QStringLiteral("Menlo"), QStringLiteral("Consolas"), QStringLiteral("DejaVu Sans Mono")});
        f.setStyleHint(QFont::Monospace);
    } else {
        f.setFamily(theme::FONT);
    }
    f.setPixelSize(px);
    f.setWeight(weight);
    return f;
}

double ease(double t) {
    t = std::clamp(t, 0.0, 1.0);
    return 1 - std::pow(1 - t, 3);
}

QPointF lerp(QPointF a, QPointF b, double t) {
    return {a.x() + (b.x() - a.x()) * t, a.y() + (b.y() - a.y()) * t};
}

QColor withAlpha(QColor c, double a) {
    c.setAlphaF(std::clamp(a, 0.0, 1.0));
    return c;
}

void background(QPainter& p, const QRectF& rect) { p.fillRect(rect, theme::BG); }

void card(QPainter& p, const QRectF& rect, const QString& title) {
    // плоский блок: тонкая рамка без заливки, заголовок как комментарий кода
    p.setPen(QPen(theme::BORDER_SOFT, 1));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(rect, 6, 6);
    if (!title.isEmpty()) {
        p.setFont(font(11, QFont::Normal, true));
        p.setPen(theme::DIM);
        p.drawText(QRectF(rect.x() + 14, rect.y() + 8, rect.width() - 28, 18), leftV(), "// " + title.toLower());
    }
}

void label(QPainter& p, double x, double y, const QString& s, const QColor& color) {
    p.setFont(font(11, QFont::Normal, true));
    p.setPen(color);
    p.drawText(QPointF(x, y), "// " + s.toLower());
}

void text(QPainter& p, const QRectF& rect, const QString& s, int px, const QColor& color, QFont::Weight weight,
          Qt::Alignment align, bool mono) {
    p.setFont(font(px, weight, mono));
    p.setPen(color);
    p.drawText(rect, align, s);
}

void cell(QPainter& p, const QRectF& rect, const QString& s, const theme::CellState& state, int px, double alpha,
          QFont::Weight weight) {
    p.setPen(QPen(withAlpha(state.border, alpha), 1));
    p.setBrush(withAlpha(state.bg, alpha));
    p.drawRoundedRect(rect, 6, 6);
    if (s.isEmpty()) return;
    if (px < 0) px = int(rect.height() * 0.42);
    QString shown = s;
    QColor fg = state.fg;
    if (shown == QStringLiteral(" ")) { shown = QStringLiteral("␣"); fg = theme::MUTED; }
    text(p, rect, shown, px, withAlpha(fg, alpha), weight, Qt::AlignCenter, true);
}

QVector<QRectF> layoutRow(double x, double y, int n, double avail, double maxCell, double gap, double minCell) {
    QVector<QRectF> out;
    if (n <= 0) return out;
    double c = std::min(maxCell, (avail - gap * (n - 1)) / n);
    c = std::max(minCell, c);
    for (int i = 0; i < n; ++i) out.push_back(QRectF(x + i * (c + gap), y, c, c));
    return out;
}

void arrow(QPainter& p, QPointF a, QPointF b, const QColor& color, double width, double curve, double head) {
    QPainterPath path(a);
    QPointF tangent;
    if (curve != 0) {
        const QPointF c1(a.x(), a.y() - curve), c2(b.x(), b.y() - curve);
        path.cubicTo(c1, c2, b);
        tangent = b - c2;
    } else {
        path.lineTo(b);
        tangent = b - a;
    }
    p.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
    const double len = std::hypot(tangent.x(), tangent.y());
    const double ux = len > 0 ? tangent.x() / len : 1, uy = len > 0 ? tangent.y() / len : 0;
    const QPointF left(b.x() - ux * head - uy * head * 0.6, b.y() - uy * head + ux * head * 0.6);
    const QPointF right(b.x() - ux * head + uy * head * 0.6, b.y() - uy * head - ux * head * 0.6);
    QPainterPath tri(b);
    tri.lineTo(left);
    tri.lineTo(right);
    tri.closeSubpath();
    p.setBrush(color);
    p.setPen(Qt::NoPen);
    p.drawPath(tri);
}

void badge(QPainter& p, QPointF center, const QString& s, const QColor& bg, const QColor& fg, double r) {
    p.setPen(Qt::NoPen);
    p.setBrush(bg);
    p.drawEllipse(center, r, r);
    text(p, QRectF(center.x() - r, center.y() - r, 2 * r, 2 * r), s, int(r * 1.1), fg, QFont::Bold);
}

}  // namespace draw
