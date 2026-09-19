#include "theme.h"

namespace theme {

QString qss() {
    const QString mono = QStringLiteral("Menlo, Consolas, \"DejaVu Sans Mono\", monospace");
    QString s = QStringLiteral(R"(
QWidget { color: %TEXT%; font-family: "%FONT%"; font-size: 12px; font-weight: 400; }
QMainWindow { background: %BG%; }

QFrame#Sidebar { background: %PANEL%; border-right: 1px solid %BORDER%; }
QScrollArea, QScrollArea > QWidget > QWidget { background: transparent; }
QLabel#PanelTitle { font-size: 12px; font-weight: 500; color: %TEXT%; }
QLabel#SectionHead { font-size: 9px; font-weight: 500; color: %DIM%; }
QFrame#SectionLine { background: %BORDER%; max-height: 1px; min-height: 1px; border: none; }
QLabel#Hint { color: %DIM%; font-size: 10px; font-weight: 300; }
QLabel#Kbd { color: %MUTED%; font-size: 10px; }
QLabel#KbdDesc { color: %DIM%; font-size: 10px; font-weight: 300; }
QLabel#FieldLabel { color: %MUTED%; font-size: 10px; }

QPushButton#Item {
    background: transparent; border: none; border-radius: 6px; text-align: left;
    padding: 7px 10px; font-size: 11px; color: %MUTED%;
}
QPushButton#Item:hover { background: %BORDER_SOFT%; color: %TEXT%; }
QPushButton#Item:checked { background: #1f2026; color: #ffffff; }

QLineEdit, QSpinBox {
    background: %BG%; border: 1px solid %BORDER%; border-radius: 6px;
    padding: 7px 10px; font-size: 12px; selection-background-color: %ACCENT%;
    font-family: %MONO%;
}
QLineEdit:focus, QSpinBox:focus { border-color: %ACCENT%; }
QLineEdit:disabled, QSpinBox:disabled { color: %DIM%; border-color: %BORDER_SOFT%; }
QSpinBox::up-button, QSpinBox::down-button { width: 0; border: none; }

QCheckBox { spacing: 8px; font-size: 11px; color: %TEXT%; }
QCheckBox::indicator { width: 16px; height: 16px; border-radius: 4px; border: 1px solid %BORDER%; background: %BG%; }
QCheckBox::indicator:checked { background: %ACCENT%; border-color: %ACCENT%; }
QRadioButton { spacing: 8px; font-size: 11px; color: %TEXT%; }
QRadioButton::indicator { width: 14px; height: 14px; border-radius: 7px; border: 1px solid %BORDER%; background: %BG%; }
QRadioButton::indicator:checked { background: %ACCENT%; border: 3px solid %BG%; outline: 1px solid %ACCENT%; }

QFrame#TabBar { background: %RAIL%; border-bottom: 1px solid %BORDER%; }
QPushButton#Tab {
    background: transparent; border: none; border-right: 1px solid %BORDER%; border-radius: 0;
    padding: 0 16px; min-height: 38px; font-size: 11px; color: %MUTED%;
}
QPushButton#Tab:hover { color: %TEXT%; background: %BORDER_SOFT%; }
QPushButton#Tab:checked { color: #ffffff; background: %BG%; border-bottom: none; }
QLabel#Pill {
    background: #16171c; border: 1px solid %BORDER%; border-radius: 14px;
    padding: 5px 14px; font-size: 10px; color: %TEXT%;
}
QPushButton#Seg {
    background: #16171c; border: 1px solid %BORDER%; border-radius: 12px;
    padding: 4px 12px; font-size: 10px; color: %MUTED%;
}
QPushButton#Seg:checked { background: #2a2b32; border-color: #34363e; color: #ffffff; }
QPushButton#Seg:hover { color: %TEXT%; }

QFrame#Transport { background: %RAIL%; border-top: 1px solid %BORDER%; }
QPushButton#BottomTab {
    background: transparent; border: none; border-bottom: 2px solid transparent; border-radius: 0;
    padding: 6px 2px; margin-right: 18px; font-size: 9px; color: %DIM%;
}
QPushButton#BottomTab:hover { color: %TEXT%; }
QPushButton#BottomTab:checked { color: %TEXT%; border-bottom: 2px solid %AMBER%; }
QLabel#StepText { font-size: 15px; padding: 2px 4px; font-family: %MONO%; }
QLabel#Error { color: %RED%; font-size: 14px; }
QLabel#StepCounter { color: %MUTED%; font-size: 11px; }
QListWidget#Journal { background: transparent; border: none; font-family: %MONO%; font-size: 12px; color: %MUTED%; outline: none; }
QListWidget#Journal::item { padding: 1px 6px; border: none; }
QListWidget#Journal::item:selected { background: #1f3d22; color: %TEXT%; }
QPushButton#Transport {
    background: #16171c; border: 1px solid %BORDER%; border-radius: 6px;
    font-size: 15px; min-width: 42px; min-height: 36px; padding: 2px 8px; color: %TEXT%;
}
QPushButton#Transport:hover { background: #1f2026; }
QPushButton#Transport:disabled { color: %DIM%; border-color: %BORDER_SOFT%; }
QPushButton#Play {
    font-size: 12px; font-weight: 500; min-height: 36px; padding: 2px 18px;
    background: %WARM%; color: #ffffff; border: none; border-radius: 6px;
}
QPushButton#Play:hover { background: %ACCENT%; }
QPushButton#Play:checked { background: %AMBER%; color: #111111; }
QPushButton#Play:disabled { background: %BORDER_SOFT%; color: %DIM%; }

QSlider::groove:horizontal { height: 3px; background: %BORDER%; border-radius: 1px; }
QSlider::handle:horizontal { width: 14px; height: 14px; margin: -6px 0; background: %ACCENT%; border-radius: 7px; }
QSlider::sub-page:horizontal { background: %ACCENT%; border-radius: 1px; }

QScrollBar:vertical { width: 6px; background: transparent; }
QScrollBar::handle:vertical { background: %BORDER%; border-radius: 3px; min-height: 30px; }
QScrollBar::add-line, QScrollBar::sub-line { height: 0; }
QToolTip { background: %PANEL%; color: %TEXT%; border: 1px solid %BORDER%; padding: 4px 8px; font-size: 10px; }
)");
    const std::pair<const char*, QString> subs[] = {
        {"%FONT%", FONT}, {"%MONO%", mono},
        {"%BG%", BG.name()}, {"%PANEL%", PANEL.name()}, {"%RAIL%", RAIL.name()},
        {"%BORDER_SOFT%", BORDER_SOFT.name()}, {"%BORDER%", BORDER.name()},
        {"%TEXT%", TEXT.name()}, {"%MUTED%", MUTED.name()}, {"%DIM%", DIM.name()},
        {"%ACCENT%", ACCENT.name()}, {"%WARM%", WARM.name()}, {"%RED%", RED.name()}, {"%AMBER%", AMBER.name()},
    };
    for (const auto& [k, v] : subs) s.replace(QLatin1String(k), v);
    return s;
}

}  // namespace theme
