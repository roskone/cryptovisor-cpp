// Общая модель шага визуализации.
// Каждый алгоритм превращает входные данные в список Step. Шаг хранит полный снимок
// состояния, поэтому сцена может показать любой шаг напрямую, без «проигрывания» предыдущих.
#pragma once

#include <QByteArray>
#include <QChar>
#include <QString>
#include <QVector>
#include <array>
#include <optional>
#include <stdexcept>
#include <variant>
#include <vector>

struct CipherError : std::runtime_error {
    QString message;
    explicit CipherError(const QString& m) : std::runtime_error(m.toStdString()), message(m) {}
};

// ── Цезарь ──
struct CaesarStep {
    int index = 0;
    QChar src, dst;
    int shift = 0;
    int codeSrc = 0, codeDst = 0;
    QString output;
    bool isLetter = false;
    QString alphabet;
    bool upper = false;
    int posSrc = -1, posDst = -1;
    bool wrapped = false;
};

// ── XOR ──
struct XorStep {
    int byteIndex = 0, bitIndex = 0;
    int tByte = 0, kByte = 0, keyIndex = 0;
    std::array<int, 8> tBits{}, kBits{};
    std::array<int, 8> rBits{};       // -1 — бит ещё не вычислен
    QByteArray output;
    bool byteDone = false;
};

// ── Перестановка ──
enum class Phase { Fill, Order, Read };

struct Cell { int r, c; };

struct TransStep {
    Phase phase = Phase::Fill;
    QVector<QVector<QChar>> grid;     // QChar() — пустая ячейка
    int srcIndex = -1;
    std::optional<Cell> cell;
    int col = -1;
    QString output;
    int ranksShown = 0;
    int outIndex = -1;
};

struct Step {
    QString text;                      // пояснение для лектора (простой HTML допустим)
    bool major = true;                 // «крупный» шаг — граница символа/байта/столбца
    std::variant<CaesarStep, XorStep, TransStep> data;
};
