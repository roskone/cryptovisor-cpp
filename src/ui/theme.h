// Палитра и QSS: плоский тёмный редактор кода.
#pragma once
#include <QColor>
#include <QString>

namespace theme {

inline const QString FONT = QStringLiteral("Unbounded");

inline const QColor BG("#131418"), PANEL("#0f1013"), RAIL("#0c0d10"), CARD("#16171c");
inline const QColor BORDER("#262830"), BORDER_SOFT("#1e2026"), TEXT("#e8e8ea"), MUTED("#8b8b94");
inline const QColor DIM("#5a5b64"), DOT("#1a1b20");
inline const QColor ACCENT("#7aa2f7"), ACCENT_SOFT("#1c2745"), WARM("#f0527a"), GREEN("#9ece6a");
inline const QColor RED("#f7768e"), AMBER("#e0af68");

// Состояние ячейки: фон, рамка, текст
struct CellState { QColor bg, border, fg; };
inline const CellState CELL_FUTURE{QColor("#1a1b21"), QColor("#2c2e37"), QColor("#767880")};
inline const CellState CELL_DONE{QColor("#182618"), QColor("#4a7a3a"), QColor("#d9f0c8")};
inline const CellState CELL_CURRENT{QColor("#18264a"), QColor("#7aa2f7"), QColor("#ffffff")};
inline const CellState CELL_KEY{QColor("#2c161f"), QColor("#7a2a44"), QColor("#f5c6d4")};
inline const CellState CELL_KEY_CURRENT{QColor("#3a1628"), QColor("#f0527a"), QColor("#ffffff")};
inline const CellState CELL_EMPTY{QColor("#111216"), QColor("#22232a"), QColor("#44454d")};

QString qss();

}  // namespace theme
