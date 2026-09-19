// Шифр Цезаря с поддержкой латиницы и кириллицы.
#pragma once
#include "steps.h"

namespace caesar {

inline const QString LATIN = QStringLiteral("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
inline const QString CYRILLIC = QStringLiteral("АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ");

const QString* alphabetFor(QChar ch);
std::vector<Step> steps(const QString& text, int shift, bool decrypt = false);
QString apply(const QString& text, int shift, bool decrypt = false);

}  // namespace caesar
