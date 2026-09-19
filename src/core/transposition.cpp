#include "transposition.h"

#include <algorithm>
#include <numeric>

namespace transposition {

void parseKey(const QString& keyIn, QVector<QChar>& headers, QVector<int>& ranks) {
    const QString key = keyIn.trimmed();
    if (key.size() < 2) throw CipherError(QStringLiteral("Ключ должен содержать минимум 2 символа"));
    headers.clear();
    ranks.clear();
    bool allDigits = std::all_of(key.begin(), key.end(), [](QChar c) { return c.isDigit(); });
    if (allDigits) {
        QVector<int> nums;
        for (const QChar c : key) nums.push_back(c.digitValue());
        QVector<int> sorted = nums;
        std::sort(sorted.begin(), sorted.end());
        for (int i = 0; i < sorted.size(); ++i)
            if (sorted[i] != i + 1)
                throw CipherError(QStringLiteral("Числовой ключ должен быть перестановкой 1..n, например 3142"));
        for (const QChar c : key) headers.push_back(c);
        for (const int x : nums) ranks.push_back(x - 1);
        return;
    }
    if (std::any_of(key.begin(), key.end(), [](QChar c) { return c.isSpace(); }))
        throw CipherError(QStringLiteral("Ключ не должен содержать пробелов"));
    const QString up = key.toUpper();
    // ранг буквы: сортируем пары (буква, позиция) — одинаковые буквы идут слева направо
    QVector<int> order(up.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return up[a] != up[b] ? up[a] < up[b] : a < b;
    });
    ranks.resize(up.size());
    for (int rank = 0; rank < order.size(); ++rank) ranks[order[rank]] = rank;
    for (const QChar c : up) headers.push_back(c);
}

QVector<int> readOrder(const QVector<int>& ranks) {
    QVector<int> order(ranks.size());
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b) { return ranks[a] < ranks[b]; });
    return order;
}

namespace {

QVector<QVector<QChar>> emptyGrid(int rows, int cols) {
    return QVector<QVector<QChar>>(rows, QVector<QChar>(cols, QChar()));
}

TransStep base(Phase phase, const QVector<QVector<QChar>>& grid) {
    TransStep s;
    s.phase = phase;
    s.grid = grid;
    return s;
}

}  // namespace

std::vector<Step> encryptSteps(const QString& text, const QString& key, Context* ctxOut) {
    if (text.isEmpty()) throw CipherError(QStringLiteral("Введите текст"));
    Context ctx;
    parseKey(key, ctx.headers, ctx.ranks);
    ctx.cols = ctx.headers.size();
    ctx.rows = (text.size() + ctx.cols - 1) / ctx.cols;
    ctx.source = text.leftJustified(ctx.rows * ctx.cols, PAD);
    ctx.order = readOrder(ctx.ranks);
    ctx.decrypt = false;
    const bool numericKey = std::all_of(key.begin(), key.end(), [](QChar c) { return c.isDigit(); });
    auto grid = emptyGrid(ctx.rows, ctx.cols);
    std::vector<Step> steps;

    // Фаза 1: заполняем таблицу по строкам
    for (int i = 0; i < ctx.source.size(); ++i) {
        const int r = i / ctx.cols, c = i % ctx.cols;
        const QChar ch = ctx.source[i];
        grid[r][c] = ch;
        const bool isPad = i >= text.size();
        const QString desc = !isPad
            ? QStringLiteral("Записываем <b>‘%1’</b> в строку %2, столбец %3").arg(ch).arg(r + 1).arg(c + 1)
            : QStringLiteral("Дополняем таблицу символом <b>‘%1’</b> (строка %2, столбец %3)").arg(PAD).arg(r + 1).arg(c + 1);
        TransStep d = base(Phase::Fill, grid);
        d.srcIndex = i;
        d.cell = Cell{r, c};
        d.ranksShown = 0;
        steps.push_back(Step{desc, c == ctx.cols - 1, d});
    }
    // Фаза 2: нумеруем столбцы по ключу
    for (int k = 0; k < ctx.order.size(); ++k) {
        const int c = ctx.order[k];
        const QString desc = QStringLiteral("Столбец <b>‘%1’</b> получает номер <b>%2</b> — %3")
                                 .arg(ctx.headers[c]).arg(k + 1).arg(numericKey ? "по ключу" : "по алфавиту");
        TransStep d = base(Phase::Order, grid);
        d.col = c;
        d.ranksShown = k + 1;
        steps.push_back(Step{desc, true, d});
    }
    // Фаза 3: читаем столбцы в порядке ключа
    QString output;
    for (int k = 0; k < ctx.order.size(); ++k) {
        const int c = ctx.order[k];
        for (int r = 0; r < ctx.rows; ++r) {
            output += grid[r][c];
            const QString desc = QStringLiteral("Читаем столбец <b>%1</b> (‘%2’), строка %3: <b>‘%4’</b>")
                                     .arg(k + 1).arg(ctx.headers[c]).arg(r + 1).arg(grid[r][c]);
            TransStep d = base(Phase::Read, grid);
            d.cell = Cell{r, c};
            d.col = c;
            d.output = output;
            d.ranksShown = ctx.cols;
            d.outIndex = output.size() - 1;
            steps.push_back(Step{desc, r == ctx.rows - 1, d});
        }
    }
    if (ctxOut) *ctxOut = ctx;
    return steps;
}

std::vector<Step> decryptSteps(const QString& cipher, const QString& key, Context* ctxOut) {
    if (cipher.isEmpty()) throw CipherError(QStringLiteral("Введите текст"));
    Context ctx;
    parseKey(key, ctx.headers, ctx.ranks);
    ctx.cols = ctx.headers.size();
    if (cipher.size() % ctx.cols != 0)
        throw CipherError(QStringLiteral("Длина шифртекста (%1) должна делиться на длину ключа (%2)")
                              .arg(cipher.size()).arg(ctx.cols));
    ctx.rows = cipher.size() / ctx.cols;
    ctx.order = readOrder(ctx.ranks);
    ctx.source = cipher;
    ctx.decrypt = true;
    auto grid = emptyGrid(ctx.rows, ctx.cols);
    std::vector<Step> steps;

    // Фаза 1: нумеруем столбцы
    for (int k = 0; k < ctx.order.size(); ++k) {
        const int c = ctx.order[k];
        TransStep d = base(Phase::Order, grid);
        d.col = c;
        d.ranksShown = k + 1;
        steps.push_back(Step{QStringLiteral("Столбец <b>‘%1’</b> получает номер <b>%2</b>").arg(ctx.headers[c]).arg(k + 1),
                             true, d});
    }
    // Фаза 2: заполняем столбцы в порядке ключа
    int i = 0;
    for (int k = 0; k < ctx.order.size(); ++k) {
        const int c = ctx.order[k];
        for (int r = 0; r < ctx.rows; ++r) {
            grid[r][c] = cipher[i];
            const QString desc = QStringLiteral("Символ <b>‘%1’</b> → столбец <b>%2</b> (‘%3’), строка %4")
                                     .arg(cipher[i]).arg(k + 1).arg(ctx.headers[c]).arg(r + 1);
            TransStep d = base(Phase::Fill, grid);
            d.srcIndex = i;
            d.cell = Cell{r, c};
            d.col = c;
            d.ranksShown = ctx.cols;
            steps.push_back(Step{desc, r == ctx.rows - 1, d});
            ++i;
        }
    }
    // Фаза 3: читаем по строкам
    QString output;
    for (int r = 0; r < ctx.rows; ++r) {
        for (int c = 0; c < ctx.cols; ++c) {
            output += grid[r][c];
            TransStep d = base(Phase::Read, grid);
            d.cell = Cell{r, c};
            d.output = output;
            d.ranksShown = ctx.cols;
            d.outIndex = output.size() - 1;
            steps.push_back(Step{QStringLiteral("Читаем строку %1, столбец %2: <b>‘%3’</b>").arg(r + 1).arg(c + 1).arg(grid[r][c]),
                                 c == ctx.cols - 1, d});
        }
    }
    if (ctxOut) *ctxOut = ctx;
    return steps;
}

QString encrypt(const QString& text, const QString& key) {
    return std::get<TransStep>(encryptSteps(text, key).back().data).output;
}

QString decrypt(const QString& cipher, const QString& key) {
    return std::get<TransStep>(decryptSteps(cipher, key).back().data).output;
}

}  // namespace transposition
