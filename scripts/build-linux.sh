#!/bin/bash

# Скрипт сборки Nesca для Linux
# Автоматически определяет систему и использует соответствующие команды

set -e  # Выход при ошибке

echo "=== Сборка Nesca для Linux ==="

# Определение дистрибутива
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        echo "$ID"
    elif [ -f /etc/debian_version ]; then
        echo "debian"
    elif [ -f /etc/arch-release ]; then
        echo "arch"
    elif [ -f /etc/redhat-release ]; then
        echo "rhel"
    else
        echo "unknown"
    fi
}

DISTRO=$(detect_distro)
echo "Обнаружен дистрибутив: $DISTRO"

# Проверка наличия qmake
if ! command -v qmake &> /dev/null; then
    echo "❌ Ошибка: qmake не найден."
    echo ""
    echo "Установите Qt5 в зависимости от вашего дистрибутива:"
    echo ""
    case "$DISTRO" in
        ubuntu|debian)
            echo "  sudo apt-get update"
            echo "  sudo apt-get install -y qtbase5-dev qtbase5-dev-tools build-essential"
            ;;
        arch|manjaro)
            echo "  sudo pacman -S qt5-base base-devel"
            ;;
        fedora|rhel|centos)
            echo "  sudo dnf install qt5-qtbase-devel gcc-c++ make"
            ;;
        opensuse*)
            echo "  sudo zypper install libqt5-qtbase-devel gcc-c++ make"
            ;;
        *)
            echo "  Установите Qt5 для вашего дистрибутива"
            ;;
    esac
    exit 1
fi

echo "✓ qmake найден: $(which qmake)"
qmake --version

# Проверка компилятора
if ! command -v g++ &> /dev/null; then
    echo "❌ Ошибка: g++ не найден."
    echo "Установите компилятор:"
    case "$DISTRO" in
        ubuntu|debian)
            echo "  sudo apt-get install -y build-essential g++"
            ;;
        arch|manjaro)
            echo "  sudo pacman -S base-devel gcc"
            ;;
        fedora|rhel|centos)
            echo "  sudo dnf install gcc-c++"
            ;;
        opensuse*)
            echo "  sudo zypper install gcc-c++"
            ;;
    esac
    exit 1
fi

echo "✓ g++ найден: $(which g++)"
g++ --version

# Проверка зависимостей (предупреждения, не критические)
echo ""
echo "Проверка зависимостей..."

MISSING_DEPS=()

# Проверка libssh
if ! pkg-config --exists libssh 2>/dev/null; then
    MISSING_DEPS+=("libssh-dev")
    echo "⚠ Предупреждение: libssh не найден"
fi

# Проверка openssl
if ! pkg-config --exists openssl 2>/dev/null; then
    MISSING_DEPS+=("libssl-dev")
    echo "⚠ Предупреждение: openssl не найден"
fi

# Проверка libcurl
if ! pkg-config --exists libcurl 2>/dev/null; then
    MISSING_DEPS+=("libcurl4-openssl-dev")
    echo "⚠ Предупреждение: libcurl не найден"
fi

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    echo ""
    echo "Рекомендуется установить недостающие зависимости:"
    case "$DISTRO" in
        ubuntu|debian)
            echo "  sudo apt-get install -y ${MISSING_DEPS[*]}"
            ;;
        arch|manjaro)
            echo "  sudo pacman -S libssh openssl curl"
            ;;
        fedora|rhel|centos)
            echo "  sudo dnf install libssh-devel openssl-devel libcurl-devel"
            ;;
        opensuse*)
            echo "  sudo zypper install libssh-devel libopenssl-devel libcurl-devel"
            ;;
    esac
    echo ""
    read -p "Продолжить сборку без них? (y/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

# Определение количества ядер для параллельной сборки
if command -v nproc &> /dev/null; then
    CORES=$(nproc)
elif [ -f /proc/cpuinfo ]; then
    CORES=$(grep -c processor /proc/cpuinfo)
else
    CORES=4
fi

echo ""
echo "=== Начало сборки ==="
echo "Использование ядер: $CORES"
echo ""

# Очистка предыдущей сборки
echo "Очистка предыдущей сборки..."
make clean 2>/dev/null || true
rm -f moc_* qrc_* ui_* *.o 2>/dev/null || true

# Генерация Makefile
echo "Генерация Makefile..."
qmake nesca.pro

if [ $? -ne 0 ]; then
    echo "❌ Ошибка: Не удалось сгенерировать Makefile"
    exit 1
fi

# Компиляция
echo "Компиляция проекта..."
make -j"$CORES"

if [ $? -ne 0 ]; then
    echo "❌ Ошибка: Компиляция не удалась"
    exit 1
fi

# Проверка результата
if [ -f nesca ]; then
    echo ""
    echo "=== Сборка завершена успешно! ==="
    echo "Исполняемый файл: $(pwd)/nesca"
    echo ""
    
    # Информация о файле
    echo "Информация о файле:"
    ls -lh nesca
    echo ""
    
    # Проверка зависимостей
    echo "Проверка зависимостей:"
    if command -v ldd &> /dev/null; then
        ldd nesca | head -5
    fi
    echo ""
    
    # Тест запуска
    echo "Тестовый запуск:"
    ./nesca --help | head -10 || true
    echo ""
    
    echo "✅ Готово! Используйте: ./nesca --help для получения справки"
else
    echo "❌ Ошибка: Исполняемый файл не найден после сборки"
    exit 1
fi

