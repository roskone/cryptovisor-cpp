#include "xor_cipher.h"

#include <QRegularExpression>

namespace xorc {

namespace {

// Минимальная таблица CP1251: ASCII, кириллица, Ё/ё и несколько типографских знаков.
bool toCp1251(QChar ch, unsigned char& out) {
    const ushort u = ch.unicode();
    if (u < 0x80) { out = static_cast<unsigned char>(u); return true; }
    if (u >= 0x0410 && u <= 0x044F) { out = static_cast<unsigned char>(0xC0 + (u - 0x0410)); return true; }
    switch (u) {
        case 0x0401: out = 0xA8; return true;   // Ё
        case 0x0451: out = 0xB8; return true;   // ё
        case 0x00AB: out = 0xAB; return true;   // «
        case 0x00BB: out = 0xBB; return true;   // »
        case 0x2014: out = 0x97; return true;   // —
        case 0x2013: out = 0x96; return true;   // –
        case 0x2116: out = 0xB9; return true;   // №
        case 0x2026: out = 0x85; return true;   // …
        case 0x00A0: out = 0xA0; return true;
        default: return false;
    }
}

QChar fromCp1251(unsigned char b) {
    if (b < 0x80) return QChar(b);
    if (b >= 0xC0) return QChar(0x0410 + (b - 0xC0));
    switch (b) {
        case 0xA8: return QChar(0x0401);
        case 0xB8: return QChar(0x0451);
        case 0xAB: return QChar(0x00AB);
        case 0xBB: return QChar(0x00BB);
        case 0x97: return QChar(0x2014);
        case 0x96: return QChar(0x2013);
        case 0xB9: return QChar(0x2116);
        case 0x85: return QChar(0x2026);
        case 0xA0: return QChar(0x00A0);
        default: return QChar(0xFFFD);
    }
}

}  // namespace

QByteArray encodeText(const QString& text) {
    QByteArray out;
    out.reserve(text.size());
    for (const QChar ch : text) {
        unsigned char b;
        if (!toCp1251(ch, b))
            throw CipherError(QStringLiteral("Символ «%1» нельзя закодировать в CP1251").arg(ch));
        out.append(static_cast<char>(b));
    }
    return out;
}

QString decodeText(const QByteArray& bytes) {
    QString s;
    for (const char c : bytes) s += fromCp1251(static_cast<unsigned char>(c));
    return s;
}

QString byteRepr(unsigned char b) {
    const QChar ch = fromCp1251(b);
    return (ch.isPrint() && ch.unicode() != 0xFFFD) ? QString(ch) : QStringLiteral("·");
}

QString hex2(unsigned char b) {
    return QString::number(b, 16).toUpper().rightJustified(2, QChar('0'));
}

std::array<int, 8> bits(unsigned char b) {
    std::array<int, 8> r{};
    for (int i = 0; i < 8; ++i) r[i] = (b >> (7 - i)) & 1;
    return r;
}

QByteArray parseInput(const QString& text, bool allowHex, bool* wasHex) {
    static const QRegularExpression hexRe(QStringLiteral("^[0-9a-fA-F]{2}([\\s,]*[0-9a-fA-F]{2})*$"));
    const QString stripped = text.trimmed();
    if (stripped.isEmpty()) throw CipherError(QStringLiteral("Введите текст"));
    if (allowHex && hexRe.match(stripped).hasMatch()) {
        QString clean = stripped;
        clean.remove(QRegularExpression(QStringLiteral("[\\s,]")));
        if (wasHex) *wasHex = true;
        return QByteArray::fromHex(clean.toLatin1());
    }
    if (wasHex) *wasHex = false;
    return encodeText(text);
}

QByteArray parseKey(bool numeric, int keyNum, const QString& keyStr) {
    if (numeric) {
        if (keyNum < 0 || keyNum > 255) throw CipherError(QStringLiteral("Числовой ключ должен быть от 0 до 255"));
        return QByteArray(1, static_cast<char>(keyNum));
    }
    if (keyStr.isEmpty()) throw CipherError(QStringLiteral("Введите ключ"));
    return encodeText(keyStr);
}

std::vector<Step> steps(const QByteArray& data, const QByteArray& key) {
    if (data.isEmpty()) throw CipherError(QStringLiteral("Введите текст"));
    if (key.isEmpty()) throw CipherError(QStringLiteral("Введите ключ"));
    std::vector<Step> result;
    QByteArray output;
    for (int bi = 0; bi < data.size(); ++bi) {
        const int ki = bi % key.size();
        const unsigned char tb = data[bi], kb = key[ki];
        const auto tbits = bits(tb), kbits = bits(kb);
        std::array<int, 8> rbits;
        rbits.fill(-1);
        for (int bit = 0; bit < 8; ++bit) {
            rbits[bit] = tbits[bit] ^ kbits[bit];
            const bool done = bit == 7;
            if (done) output.append(static_cast<char>(tb ^ kb));
            XorStep d;
            d.byteIndex = bi;
            d.bitIndex = bit;
            d.tByte = tb;
            d.kByte = kb;
            d.keyIndex = ki;
            d.tBits = tbits;
            d.kBits = kbits;
            d.rBits = rbits;
            d.output = output;
            d.byteDone = done;
            QString desc = QStringLiteral("Байт %1, бит %2: &nbsp;<b>%3</b> ⊕ <b>%4</b> = <b>%5</b>")
                               .arg(bi + 1).arg(bit + 1).arg(tbits[bit]).arg(kbits[bit]).arg(rbits[bit]);
            if (done) {
                const unsigned char res = tb ^ kb;
                desc += QStringLiteral(" &nbsp;→&nbsp; байт готов: 0x%1 ⊕ 0x%2 = <b>0x%3</b> (‘%4’)")
                            .arg(hex2(tb), hex2(kb), hex2(res), byteRepr(res));
            }
            result.push_back(Step{desc, done, d});
        }
    }
    return result;
}

QByteArray apply(const QByteArray& data, const QByteArray& key) {
    QByteArray out;
    for (int i = 0; i < data.size(); ++i) out.append(static_cast<char>(data[i] ^ key[i % key.size()]));
    return out;
}

}  // namespace xorc
