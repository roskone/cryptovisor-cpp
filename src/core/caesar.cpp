#include "caesar.h"

namespace caesar {

const QString* alphabetFor(QChar ch) {
    const QChar up = ch.toUpper();
    if (LATIN.contains(up)) return &LATIN;
    if (CYRILLIC.contains(up)) return &CYRILLIC;
    return nullptr;
}

namespace {

// Сдвигает один символ и заполняет подробности для сцены.
QChar shiftChar(QChar ch, int shift, CaesarStep& info) {
    const QString* alphabet = alphabetFor(ch);
    if (!alphabet) {
        info.isLetter = false;
        return ch;
    }
    const int n = alphabet->size();
    const bool upper = ch.isUpper();
    const int pos = alphabet->indexOf(ch.toUpper());
    const int newPos = ((pos + shift) % n + n) % n;
    QChar out = alphabet->at(newPos);
    if (!upper) out = out.toLower();
    info.isLetter = true;
    info.alphabet = *alphabet;
    info.upper = upper;
    info.posSrc = pos;
    info.posDst = newPos;
    info.wrapped = !(0 <= pos + shift && pos + shift < n);
    return out;
}

}  // namespace

std::vector<Step> steps(const QString& text, int shift, bool decrypt) {
    if (text.isEmpty()) throw CipherError(QStringLiteral("Введите текст"));
    const int eff = decrypt ? -shift : shift;
    std::vector<Step> result;
    QString output;
    for (int i = 0; i < text.size(); ++i) {
        const QChar ch = text[i];
        CaesarStep d;
        const QChar out = shiftChar(ch, eff, d);
        output += out;
        d.index = i;
        d.src = ch;
        d.dst = out;
        d.shift = eff;
        d.codeSrc = ch.unicode();
        d.codeDst = out.unicode();
        d.output = output;
        QString desc;
        if (d.isLetter) {
            const int n = d.alphabet.size();
            const QString sign = eff >= 0 ? "+" : "−";
            desc = QStringLiteral("<b>‘%1’</b> — позиция %2 &nbsp;→&nbsp; (%2 %3 %4) mod %5 = %6 &nbsp;→&nbsp; <b>‘%7’</b>")
                       .arg(ch).arg(d.posSrc).arg(sign).arg(std::abs(eff)).arg(n).arg(d.posDst).arg(out);
            if (d.wrapped)
                desc += QStringLiteral(" &nbsp;<span style='color:#e0af68'>(переход через край алфавита)</span>");
        } else {
            desc = QStringLiteral("<b>‘%1’</b> — не буква, остаётся без изменений").arg(ch);
        }
        result.push_back(Step{desc, true, d});
    }
    return result;
}

QString apply(const QString& text, int shift, bool decrypt) {
    const int eff = decrypt ? -shift : shift;
    QString out;
    for (const QChar ch : text) {
        CaesarStep tmp;
        out += shiftChar(ch, eff, tmp);
    }
    return out;
}

}  // namespace caesar
