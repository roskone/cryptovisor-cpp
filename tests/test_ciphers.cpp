// Тесты ядра. Запуск: ctest --test-dir build  (или ./build/test_ciphers)
#include <QCoreApplication>
#include <cstdio>
#include <cstdlib>

#include "core/caesar.h"
#include "core/transposition.h"
#include "core/xor_cipher.h"

static int failures = 0;
#define CHECK(cond)                                                                       \
    do {                                                                                  \
        if (!(cond)) { std::printf("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++failures; } \
    } while (0)
#define CHECK_THROWS(expr)                                                                \
    do {                                                                                  \
        bool thrown = false;                                                              \
        try { (void)(expr); } catch (const CipherError&) { thrown = true; }               \
        CHECK(thrown);                                                                    \
    } while (0)

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    using namespace Qt::StringLiterals;

    // Цезарь
    CHECK(caesar::apply(u"Hello, World!"_s, 3) == u"Khoor, Zruog!"_s);
    CHECK(caesar::apply(u"Khoor, Zruog!"_s, 3, true) == u"Hello, World!"_s);
    CHECK(caesar::apply(u"Ёж"_s, 1) == u"Жз"_s);
    CHECK(caesar::apply(u"Я"_s, 1) == u"А"_s);
    CHECK(caesar::steps(u"abc"_s, 1).size() == 3);
    CHECK_THROWS(caesar::steps(u""_s, 1));

    // XOR
    {
        const QByteArray data = xorc::encodeText(u"Привет"_s);
        const QByteArray key = xorc::parseKey(false, 0, u"ключ"_s);
        CHECK(xorc::apply(xorc::apply(data, key), key) == data);
        CHECK(xorc::decodeText(data) == u"Привет"_s);
        const auto steps = xorc::steps(QByteArray("AB"), QByteArray(1, char(0xFF)));
        CHECK(steps.size() == 16);
        const auto& last = std::get<XorStep>(steps.back().data);
        CHECK(last.output == QByteArray::fromHex("BEBD"));
        int majors = 0;
        for (const auto& s : steps) majors += s.major;
        CHECK(majors == 2);
        bool wasHex = false;
        CHECK(xorc::parseInput(u"4a 2F"_s, true, &wasHex) == QByteArray::fromHex("4A2F"));
        CHECK(wasHex);
        CHECK(xorc::hex2(0x0A) == u"0A"_s);
    }

    // Перестановка
    {
        QVector<QChar> h;
        QVector<int> r;
        transposition::parseKey(u"КЛЮЧ"_s, h, r);
        CHECK((r == QVector<int>{0, 1, 3, 2}));  // К<Л<Ч<Ю
        transposition::parseKey(u"3142"_s, h, r);
        CHECK((r == QVector<int>{2, 0, 3, 1}));
        const QString enc = transposition::encrypt(u"ПРИВЕТМИР"_s, u"КЛЮЧ"_s);
        CHECK(enc.size() == 12);
        CHECK(transposition::decrypt(enc, u"КЛЮЧ"_s) == u"ПРИВЕТМИР___"_s);
        CHECK(transposition::encrypt(u"ABCDEFGH"_s, u"3142"_s) == u"BFDHAECG"_s);
        CHECK_THROWS(transposition::parseKey(u"3143"_s, h, r));
        CHECK_THROWS(transposition::decryptSteps(u"abcde"_s, u"КЛЮЧ"_s));
    }

    if (failures == 0) std::printf("OK — все проверки пройдены\n");
    return failures == 0 ? 0 : 1;
}
