// Столбцовая перестановка (columnar transposition).
// Ключ — слово («КЛЮЧ») или последовательность цифр («3142»). Текст записывается в таблицу
// по строкам, столбцы читаются в порядке, который задаёт ключ.
#pragma once
#include "steps.h"

namespace transposition {

inline const QChar PAD = QChar('_');

struct Context {
    QVector<QChar> headers;
    QVector<int> ranks;        // ранг каждого столбца 0..n-1
    QVector<int> order;        // индексы столбцов в порядке чтения
    int rows = 0, cols = 0;
    QString source;            // исходная строка (дополненная) или шифртекст
    bool decrypt = false;
};

void parseKey(const QString& key, QVector<QChar>& headers, QVector<int>& ranks);
QVector<int> readOrder(const QVector<int>& ranks);

std::vector<Step> encryptSteps(const QString& text, const QString& key, Context* ctx = nullptr);
std::vector<Step> decryptSteps(const QString& cipher, const QString& key, Context* ctx = nullptr);
QString encrypt(const QString& text, const QString& key);
QString decrypt(const QString& cipher, const QString& key);

}  // namespace transposition
