// Побитовый XOR с ключом-числом или ключом-строкой.
// Текст кодируется в CP1251, чтобы каждый символ (в т.ч. кириллица) занимал ровно один байт.
#pragma once
#include "steps.h"

namespace xorc {

QByteArray encodeText(const QString& text);            // бросает CipherError
QString decodeText(const QByteArray& bytes);
QString byteRepr(unsigned char b);                      // печатный символ или «·»
QString hex2(unsigned char b);                          // "4A"
std::array<int, 8> bits(unsigned char b);

// Возвращает байты; в режиме дешифрования принимает hex («4A 2F …»). wasHex — распознан ли hex.
QByteArray parseInput(const QString& text, bool allowHex, bool* wasHex);
QByteArray parseKey(bool numeric, int keyNum, const QString& keyStr);

std::vector<Step> steps(const QByteArray& data, const QByteArray& key);
QByteArray apply(const QByteArray& data, const QByteArray& key);

}  // namespace xorc
