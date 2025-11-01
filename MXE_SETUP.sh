#!/bin/bash

# Скрипт для быстрой установки зависимостей MXE и сборки Qt
# Использование: ./MXE_SETUP.sh

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[OK]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

print_info "Проверка зависимостей для MXE..."

# Проверка наличия зависимостей
MISSING_DEPS=()

check_dep() {
    if ! command -v "$1" &> /dev/null; then
        MISSING_DEPS+=("$1")
    fi
}

# Проверка команд
check_dep "7za"
check_dep "gperf"
check_dep "intltoolize"
check_dep "lzip"
check_dep "ruby"

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    print_warning "Не найдены следующие зависимости: ${MISSING_DEPS[*]}"
    echo ""
    echo "Установите их командой:"
    echo "  sudo pacman -S --needed p7zip gperf intltool lzip ruby"
    echo ""
    echo "Или для полного списка зависимостей MXE:"
    echo "  sudo pacman -S --needed \\"
    echo "    autoconf automake bash bison bzip2 cmake flex \\"
    echo "    gettext git gperf intltool libtool make openssl \\"
    echo "    patch perl pkg-config python ruby sed unzip wget xz yasm nasm \\"
    echo "    p7zip lzip"
    exit 1
fi

print_success "Все зависимости найдены!"

# Проверка наличия MXE
if [ ! -d "$HOME/mxe" ]; then
    print_error "MXE не найден в $HOME/mxe"
    echo ""
    echo "Клонируйте MXE:"
    echo "  git clone https://github.com/mxe/mxe.git ~/mxe"
    exit 1
fi

print_success "MXE найден в $HOME/mxe"

# Переход в MXE
cd "$HOME/mxe"

print_info "Проверка требований MXE..."
if ! make check-requirements; then
    print_error "Не все требования MXE выполнены!"
    exit 1
fi

print_success "Все требования выполнены!"

# Сборка Qt и библиотек
print_info "Начинаю сборку Qt5 и библиотек..."
print_warning "Это может занять 30-60 минут в зависимости от системы"
echo ""

CPU_COUNT=$(nproc 2>/dev/null || echo 4)
print_info "Использую $CPU_COUNT параллельных процессов"

make qtbase qt5base libssh openssl curl nghttp2 -j${CPU_COUNT}

if [ $? -eq 0 ]; then
    print_success "MXE успешно собран!"
    echo ""
    print_info "Теперь можно собрать Windows бинарник:"
    echo "  cd /home/kron4k/Downloads/nesca-remake"
    echo "  ./build_windows.sh"
else
    print_error "Ошибка при сборке MXE"
    exit 1
fi

