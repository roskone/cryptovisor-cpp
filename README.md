# Криптовизор (C++ / Qt 6)

Интерактивный пошаговый визуализатор шифров для лекций: шифр Цезаря, XOR, столбцовая перестановка.
Порт Python-версии ([roskone/cryptovisor](https://github.com/roskone/cryptovisor)) на C++20 и Qt Widgets —
один бинарник без интерпретатора, шрифты и иконка вшиты в ресурсы.

## Сборка

Нужны CMake 3.21+, компилятор с C++20 и Qt 6 (Core, Widgets, Test).

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j8
```

- **macOS**: `brew install qt cmake` — путь к Homebrew-Qt подхватывается автоматически. Результат: `build/cryptovisor.app`.
- **Windows**: установить Qt через Qt Online Installer (MSVC или MinGW), указать путь
  `-DCMAKE_PREFIX_PATH=C:/Qt/6.8.2/msvc2022_64` (или переменную окружения `QT_DIR`).
  Результат: `build/Release/cryptovisor.exe`; для переносимой папки рядом с exe выполнить `windeployqt cryptovisor.exe`.
- **CLion**: открыть папку как CMake-проект; если Qt не найден, в настройках CMake добавить `-DCMAKE_PREFIX_PATH=<путь к Qt>`.

## Тесты

```bash
ctest --test-dir build --output-on-failure
```

`test_ciphers` — логика шифров, `test_keys` — горячие клавиши и авто-режим (запускается offscreen).

## Структура

- `src/core/` — логика шифров без GUI, статическая библиотека `cryptocore`; каждый алгоритм возвращает список шагов (`Step`)
- `src/ui/scenes/` — сцены визуализации (по одной на алгоритм), рисуют текущий шаг на `QPainter`
- `src/ui/sidebar.*` — панель ввода, `tabbar.*` — вкладки, `transport.*` — управление воспроизведением и журнал
- `src/ui/main_window.*` — связывает всё вместе, горячие клавиши
- `src/ui/theme.*`, `draw.*` — палитра, QSS, примитивы рисования
- `tests/screenshot_main.cpp` — утилита `cryptovisor_shots <папка>` для рендера сцен в PNG без экрана

## Горячие клавиши

| Клавиша | Действие |
|---|---|
| Пробел / → | шаг вперёд |
| ← | шаг назад |
| Home / End | в начало / в конец |
| Enter | авто-воспроизведение |
| + / − | быстрее / медленнее |
| 1 · 2 · 3 | выбор алгоритма |
| E / D | шифровать / дешифровать |
| M | крупные шаги (по символу / байту / столбцу) |
| F | полный экран |
| Esc | выйти из поля ввода / из полного экрана |
