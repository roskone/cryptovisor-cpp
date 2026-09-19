// Примитивы рисования, общие для всех сцен.
#pragma once
#include <QFont>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QVector>

#include "theme.h"

namespace draw {

QFont font(int px, QFont::Weight weight = QFont::Normal, bool mono = false);
double ease(double t);                       // ease-out cubic
QPointF lerp(QPointF a, QPointF b, double t);
QColor withAlpha(QColor c, double a);

void background(QPainter& p, const QRectF& rect);
void card(QPainter& p, const QRectF& rect, const QString& title = {});
void label(QPainter& p, double x, double y, const QString& text, const QColor& color = theme::DIM);
void text(QPainter& p, const QRectF& rect, const QString& s, int px = 14, const QColor& color = theme::TEXT,
          QFont::Weight weight = QFont::Normal, Qt::Alignment align = Qt::AlignCenter, bool mono = false);
void cell(QPainter& p, const QRectF& rect, const QString& s, const theme::CellState& state, int px = -1,
          double alpha = 1.0, QFont::Weight weight = QFont::DemiBold);
QVector<QRectF> layoutRow(double x, double y, int n, double avail, double maxCell = 60, double gap = 6,
                          double minCell = 14);
void arrow(QPainter& p, QPointF a, QPointF b, const QColor& color, double width = 2.5, double curve = 0,
           double head = 9);
void badge(QPainter& p, QPointF center, const QString& s, const QColor& bg, const QColor& fg = theme::BG,
           double r = 11);

inline Qt::Alignment leftV() { return Qt::AlignLeft | Qt::AlignVCenter; }

}  // namespace draw
