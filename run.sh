#!/bin/bash

# Скрипт запуска Nesca
# Убедитесь, что приложение собрано и все DLL находятся в папке release

echo "=== Запуск Nesca ==="

# Проверяем наличие исполняемого файла
if [ ! -f "release/nesca.exe" ]; then
    echo "Ошибка: Исполняемый файл nesca.exe не найден в папке release"
    echo "Сначала выполните сборку: ./build.sh"
    exit 1
fi

# Проверяем наличие необходимых DLL
echo "Проверка зависимостей..."

required_dlls=(
    "Qt5Core.dll"
    "Qt5Gui.dll" 
    "Qt5Widgets.dll"
    "Qt5Multimedia.dll"
    "Qt5Network.dll"
    "libgcc_s_seh-1.dll"
    "libstdc++-6.dll"
    "libwinpthread-1.dll"
)

missing_dlls=()

for dll in "${required_dlls[@]}"; do
    if [ ! -f "release/$dll" ]; then
        missing_dlls+=("$dll")
    fi
done

if [ ${#missing_dlls[@]} -ne 0 ]; then
    echo "Предупреждение: Отсутствуют следующие DLL:"
    for dll in "${missing_dlls[@]}"; do
        echo "  - $dll"
    done
    echo "Попробуйте пересобрать проект: ./build.sh"
    echo ""
fi

# Устанавливаем переменные окружения для Qt
export QT_PLUGIN_PATH="$(pwd)/release/platforms"
export PATH="$(pwd)/release:$PATH"

echo "Запуск Nesca..."
cd release
./nesca.exe

echo "Nesca завершил работу."
