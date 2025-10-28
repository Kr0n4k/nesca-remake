#!/bin/bash

# Скрипт сборки Nesca для MSYS2 MINGW
# Убедитесь, что MSYS2 MINGW установлен и настроен

echo "=== Сборка Nesca для MSYS2 MINGW ==="

# Проверяем наличие qmake
if ! command -v qmake &> /dev/null; then
    echo "Ошибка: qmake не найден. Установите Qt5 через MSYS2:"
    echo "pacman -S mingw-w64-x86_64-qt5-base mingw-w64-x86_64-qt5-multimedia"
    exit 1
fi

# Проверяем наличие необходимых библиотек
echo "Проверка зависимостей..."

# Проверяем libssh
if ! pkg-config --exists libssh; then
    echo "Предупреждение: libssh не найден. Установите через:"
    echo "pacman -S mingw-w64-x86_64-libssh"
fi

# Проверяем openssl
if ! pkg-config --exists openssl; then
    echo "Предупреждение: openssl не найден. Установите через:"
    echo "pacman -S mingw-w64-x86_64-openssl"
fi

# Проверяем curl
if ! pkg-config --exists libcurl; then
    echo "Предупреждение: libcurl не найден. Установите через:"
    echo "pacman -S mingw-w64-x86_64-curl"
fi

echo "Очистка предыдущей сборки..."
make clean 2>/dev/null || true

echo "Генерация Makefile..."
qmake nesca.pro

if [ $? -ne 0 ]; then
    echo "Ошибка: Не удалось сгенерировать Makefile"
    exit 1
fi

echo "Компиляция проекта..."
mingw32-make -j$(nproc)

if [ $? -ne 0 ]; then
    echo "Ошибка: Компиляция не удалась"
    exit 1
fi

echo "Копирование DLL в папку release..."
# Создаем папку release если её нет
mkdir -p release

# Копируем необходимые DLL из MSYS2
echo "Копирование Qt DLL..."
cp -f /mingw64/bin/Qt5Core.dll release/ 2>/dev/null || true
cp -f /mingw64/bin/Qt5Gui.dll release/ 2>/dev/null || true
cp -f /mingw64/bin/Qt5Widgets.dll release/ 2>/dev/null || true
cp -f /mingw64/bin/Qt5Multimedia.dll release/ 2>/dev/null || true
cp -f /mingw64/bin/Qt5Network.dll release/ 2>/dev/null || true
cp -f /mingw64/bin/Qt5Svg.dll release/ 2>/dev/null || true

echo "Копирование системных DLL..."
cp -f /mingw64/bin/libgcc_s_seh-1.dll release/ 2>/dev/null || true
cp -f /mingw64/bin/libstdc++-6.dll release/ 2>/dev/null || true
cp -f /mingw64/bin/libwinpthread-1.dll release/ 2>/dev/null || true

echo "Копирование библиотек..."
cp -f /mingw64/bin/libssh.dll release/ 2>/dev/null || true
cp -f /mingw64/bin/libcrypto-*.dll release/ 2>/dev/null || true
cp -f /mingw64/bin/libssl-*.dll release/ 2>/dev/null || true
cp -f /mingw64/bin/libcurl-*.dll release/ 2>/dev/null || true

echo "Копирование плагинов Qt..."
mkdir -p release/platforms
cp -f /mingw64/share/qt5/plugins/platforms/qwindows.dll release/platforms/ 2>/dev/null || true

echo "=== Сборка завершена успешно! ==="
echo "Исполняемый файл: release/nesca.exe"
echo "Для запуска используйте: ./run.sh"
