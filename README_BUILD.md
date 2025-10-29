# Nesca v2r - Build Instructions

## Быстрая сборка

### Все версии (console + GUI)
```bash
./build_all.sh
```

### Только console версия
```bash
./build_all.sh --console-only
```

### Только GUI версия
```bash
./build_all.sh --gui-only
```

### Очистка перед сборкой
```bash
./build_all.sh --clean
```

### Debug режим
```bash
./build_all.sh --debug
```

## Доступные скрипты

### build_all.sh
Основной скрипт для сборки всех версий. Поддерживает:
- `--console-only` - Только console версия
- `--gui-only` - Только GUI версия
- `--clean` - Очистка перед сборкой
- `--debug` - Debug режим
- `--help, -h` - Справка

### build.sh
Короткий алиас для build_all.sh:
```bash
./build.sh
```

### build_gui.sh
Отдельный скрипт для GUI версии (устаревший, используйте build_all.sh)

## Примеры использования

### Стандартная сборка
```bash
./build_all.sh
```

### Сборка с очисткой
```bash
./build_all.sh --clean
```

### Только GUI в debug режиме
```bash
./build_all.sh --gui-only --debug
```

### Быстрая пересборка console
```bash
./build_all.sh --console-only
```

## Требования

### Обязательные
- **Qt 5.15+** с development headers
- **C++14 компилятор** (GCC, Clang)
- **qmake** для сборки

### Рекомендуемые библиотеки
- **libssh** - для SSH подключений
- **openssl** - для SSL/TLS
- **libcurl** - для HTTP запросов
- **nghttp2** - для HTTP/2 поддержки

## Установка зависимостей

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    qtbase5-dev \
    qtbase5-dev-tools \
    libssh-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    libnghttp2-dev \
    build-essential
```

### Arch Linux
```bash
sudo pacman -S \
    qt5-base \
    libssh \
    openssl \
    curl \
    nghttp2 \
    base-devel
```

### macOS (Homebrew)
```bash
brew install qt5 libssh openssl curl nghttp2
export PATH="/usr/local/opt/qt5/bin:$PATH"
```

### Fedora/CentOS
```bash
sudo dnf install -y \
    qt5-qtbase-devel \
    qt5-qttools-devel \
    libssh-devel \
    openssl-devel \
    libcurl-devel \
    nghttp2-devel \
    gcc-c++ \
    make
```

## Результаты сборки

После успешной сборки будут созданы:

### Console версия
- `nesca` - Исполняемый файл console приложения

### GUI версия
- `nesca-gui` - Исполняемый файл GUI приложения

## Ручная сборка (без скриптов)

### Console версия
```bash
qmake nesca.pro
make -j$(nproc)
```

### GUI версия
```bash
mkdir build_gui
cd build_gui
qmake ../nesca_gui.pro
make -j$(nproc)
cd ..
cp build_gui/nesca-gui .
```

## Режимы сборки

### Release (по умолчанию)
```bash
./build_all.sh
# или
qmake CONFIG+=release nesca.pro
make
```

### Debug
```bash
./build_all.sh --debug
# или
qmake CONFIG+=debug nesca.pro
make
```

## Очистка проекта

### Очистка console сборки
```bash
make clean
```

### Очистка GUI сборки
```bash
cd build_gui
make clean
cd ..
rm -rf build_gui nesca-gui
```

### Полная очистка
```bash
make clean
rm -rf build_gui
rm -f nesca nesca-gui
rm -f moc_* *.o
```

## Устранение проблем

### Ошибка: qmake not found
```bash
# Установите Qt development пакеты
sudo apt-get install qtbase5-dev qtbase5-dev-tools
```

### Ошибка: Missing libraries
```bash
# Установите недостающие библиотеки
sudo apt-get install libssh-dev libssl-dev libcurl4-openssl-dev
```

### Ошибка компиляции
```bash
# Попробуйте собрать в debug режиме для подробностей
./build_all.sh --debug
```

### Сборка заняла слишком много времени
```bash
# Используйте больше потоков (автоматически)
./build_all.sh

# Или вручную
make -j$(nproc)  # Использует все доступные ядра
```

## Проверка сборки

После сборки проверьте наличие исполняемых файлов:

```bash
ls -lh nesca nesca-gui
```

Запустите для проверки:

```bash
# Console версия
./nesca --help

# GUI версия
./nesca-gui
```

## Оптимизация

### Быстрая пересборка (только измененные файлы)
```bash
make -j$(nproc)
```

### Полная пересборка
```bash
./build_all.sh --clean
```

### Параллельная сборка
Скрипт автоматически использует все доступные ядра процессора.

## Автоматизация

### CI/CD
```bash
#!/bin/bash
# Example CI script

# Install dependencies (example for Ubuntu)
sudo apt-get update
sudo apt-get install -y qtbase5-dev build-essential

# Build
./build_all.sh

# Run tests
./nesca --help
./nesca-gui &
```

## Дополнительная информация

- См. `GUI_IMPROVEMENTS.md` для информации о GUI версии
- См. `README.md` для общей информации о проекте
- См. `docs/` для подробной документации

## Поддержка

При проблемах со сборкой:
1. Проверьте все зависимости установлены
2. Попробуйте `./build_all.sh --clean`
3. Проверьте версию Qt: `qmake -v`
4. Создайте issue с подробным логом

