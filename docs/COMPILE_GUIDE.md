# Руководство по компиляции Nesca v2r

Это подробное руководство по компиляции проекта Nesca для **Linux** и **Windows 11**.

---

## 📋 Содержание

1. [Компиляция для Linux](#компиляция-для-linux)
2. [Компиляция для Windows 11](#компиляция-для-windows-11)
3. [Устранение проблем](#устранение-проблем)

---

## 🐧 Компиляция для Linux

### Требования

- **Qt 5.15+** (core, network)
- **libssh** - для SSH подключений
- **OpenSSL** - для SSL/TLS
- **libcurl** - для HTTP запросов
- **GCC/G++** с поддержкой C++11
- **make** или **ninja**

### Установка зависимостей

#### Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install -y \
    qtbase5-dev \
    qtbase5-dev-tools \
    libssh-dev \
    libssl-dev \
    libcurl4-openssl-dev \
    build-essential \
    g++
```

#### Arch Linux:
```bash
sudo pacman -S qt5-base libssh openssl curl base-devel gcc
```

#### Fedora/RHEL/CentOS:
```bash
sudo dnf install qt5-qtbase-devel libssh-devel openssl-devel libcurl-devel gcc-c++ make
```

#### openSUSE:
```bash
sudo zypper install libqt5-qtbase-devel libssh-devel libopenssl-devel libcurl-devel gcc-c++ make
```

### Шаг 1: Проверка зависимостей

```bash
# Проверка версии Qt
qmake --version

# Проверка компилятора
g++ --version
```

### Шаг 2: Конфигурация проекта

```bash
cd /home/kron4k/Downloads/nesca-v2r

# Генерация Makefile
qmake nesca.pro

# Если нужно проверить конфигурацию:
qmake -query
```

### Шаг 3: Компиляция

```bash
# Компиляция (используйте количество ядер вашего процессора)
make -j$(nproc)

# Или если nproc недоступен:
make -j4  # для 4-ядерного процессора
```

### Шаг 4: Проверка результата

```bash
# Проверка, что исполняемый файл создан
ls -lh nesca

# Проверка зависимостей
ldd nesca

# Запуск
./nesca --help
```

### Режимы сборки

#### Debug режим:
```bash
qmake nesca.pro CONFIG+=debug
make clean
make -j$(nproc)
```

#### Release режим (оптимизированный):
```bash
qmake nesca.pro CONFIG+=release
make clean
make -j$(nproc)
```

#### Очистка:
```bash
make clean
# Удаление всех сгенерированных файлов
rm -f moc_* qrc_* ui_* *.o
```

---

## 🪟 Компиляция для Windows 11

### Вариант 1: MSYS2 + MinGW-w64 (Рекомендуется)

#### Шаг 1: Установка MSYS2

1. Скачайте MSYS2 с официального сайта: https://www.msys2.org/
2. Установите MSYS2 (например, в `C:\msys64\`)
3. Откройте **MSYS2 MSYS** (не MinGW!) и обновите систему:

```bash
pacman -Syu
# Если потребуется, перезапустите терминал и выполните снова:
pacman -Su
```

#### Шаг 2: Установка зависимостей

Откройте **MSYS2 MinGW 64-bit** (важно: именно MinGW 64-bit, не MSYS!) и выполните:

```bash
# Обновление базы пакетов
pacman -Syu

# Установка Qt5 и инструментов разработки
pacman -S \
    mingw-w64-x86_64-qt5-base \
    mingw-w64-x86_64-qt5-multimedia \
    mingw-w64-x86_64-gcc \
    mingw-w64-x86_64-make \
    mingw-w64-x86_64-pkg-config

# Установка библиотек
pacman -S \
    mingw-w64-x86_64-libssh \
    mingw-w64-x86_64-openssl \
    mingw-w64-x86_64-curl
```

#### Шаг 3: Добавление путей в PATH

Добавьте в переменную окружения PATH Windows:
- `C:\msys64\mingw64\bin`
- `C:\msys64\usr\bin`

Или выполните в PowerShell:
```powershell
$env:Path += ";C:\msys64\mingw64\bin;C:\msys64\usr\bin"
```

#### Шаг 4: Компиляция проекта

Откройте **MSYS2 MinGW 64-bit** и выполните:

```bash
cd /c/path/to/nesca-v2r  # Замените на ваш путь

# Очистка предыдущей сборки
make clean 2>/dev/null || true

# Генерация Makefile
qmake nesca.pro

# Компиляция
mingw32-make -j4
```

#### Шаг 5: Подготовка исполняемого файла

После компиляции создайте папку `release` и скопируйте необходимые DLL:

```bash
# Создание папки release
mkdir -p release

# Копирование исполняемого файла
cp nesca.exe release/

# Копирование Qt DLL
cp /mingw64/bin/Qt5Core.dll release/
cp /mingw64/bin/Qt5Network.dll release/

# Копирование системных DLL
cp /mingw64/bin/libgcc_s_seh-1.dll release/
cp /mingw64/bin/libstdc++-6.dll release/
cp /mingw64/bin/libwinpthread-1.dll release/

# Копирование библиотек
cp /mingw64/bin/libssh.dll release/
cp /mingw64/bin/libcrypto-*.dll release/
cp /mingw64/bin/libssl-*.dll release/ 2>/dev/null || true
cp /mingw64/bin/libssl-*.dll release/ 2>/dev/null || true
cp /mingw64/bin/libcurl-*.dll release/ 2>/dev/null || true
```

#### Шаг 6: Использование скриптов автоматизации

В проекте есть готовые скрипты для Windows:

```bash
# Использование bash скрипта (в MSYS2 MinGW 64-bit)
chmod +x scripts/build.sh
./scripts/build.sh

# Или использование bat скрипта (в обычном cmd/PowerShell)
scripts\build.bat
```

#### Запуск на Windows:

```bash
cd release
./nesca.exe --help
```

---

### Вариант 2: Visual Studio 2019/2022 + Qt (Альтернативный)

#### Требования:
- Visual Studio 2019 или 2022 (Community, Professional или Enterprise)
- Qt 5.15+ установленный через Qt Installer
- Qt Visual Studio Tools

#### Шаг 1: Установка Qt

1. Скачайте Qt Installer с https://www.qt.io/download
2. Установите Qt 5.15.x (например, в `C:\Qt\5.15.2`)
3. Выберите компоненты:
   - Qt 5.15.2 -> MSVC 2019 64-bit
   - Qt Creator (опционально)

#### Шаг 2: Установка зависимостей

Установите библиотеки через vcpkg или вручную:
- libssh
- OpenSSL
- libcurl

#### Шаг 3: Открытие проекта

1. Откройте Qt Creator
2. File -> Open File or Project
3. Выберите `nesca_3.pro` (для GUI версии) или создайте новый проект на основе `nesca.pro`

**Примечание:** `nesca_3.pro` предназначен для GUI версии с Qt Widgets. Для консольной версии используйте `nesca.pro` или адаптируйте проект.

#### Шаг 4: Настройка проекта

1. В Qt Creator: Projects -> Build Settings
2. Выберите Kit: Desktop Qt 5.15.2 MSVC2019 64bit
3. Настройте пути к библиотекам (libssh, OpenSSL, libcurl)

#### Шаг 5: Компиляция

Нажмите "Build" (Ctrl+B) или выберите Build -> Build All

---

## 🔧 Устранение проблем

### Linux

#### Проблема: "qmake: command not found"
**Решение:**
```bash
# Ubuntu/Debian
sudo apt-get install qt5-qmake qtbase5-dev-tools

# Arch
sudo pacman -S qt5-base

# Проверьте путь
which qmake
```

#### Проблема: "libssh not found"
**Решение:**
```bash
# Ubuntu/Debian
sudo apt-get install libssh-dev

# Arch
sudo pacman -S libssh

# Проверьте наличие
pkg-config --exists libssh && echo "OK" || echo "NOT FOUND"
```

#### Проблема: "cannot find -lssh"
**Решение:**
```bash
# Проверьте установку
ls -la /usr/lib/libssh* /usr/lib/x86_64-linux-gnu/libssh*

# Если библиотека в другом месте, добавьте путь в nesca.pro:
LIBS += -L/path/to/libssh/lib -lssh
```

#### Проблема: Ошибки компиляции Qt
**Решение:**
```bash
# Убедитесь, что установлена правильная версия Qt
qmake --version

# Если Qt 6, проект может потребовать адаптации
# Проект написан для Qt 5.15+
```

### Windows (MSYS2)

#### Проблема: "qmake: command not found"
**Решение:**
```bash
# Убедитесь, что используете MSYS2 MinGW 64-bit (не MSYS!)
# Проверьте установку Qt5
pacman -Q | grep qt5-base

# Если не установлено:
pacman -S mingw-w64-x86_64-qt5-base

# Проверьте путь
which qmake
```

#### Проблема: "mingw32-make: command not found"
**Решение:**
```bash
# Установите make
pacman -S mingw-w64-x86_64-make

# Проверьте
which mingw32-make
```

#### Проблема: "DLL not found" при запуске
**Решение:**
1. Убедитесь, что все DLL скопированы в папку `release/`
2. Проверьте, что приложение запускается из папки `release/`
3. Используйте скрипт `build.sh`, который автоматически копирует DLL
4. Проверьте зависимости:
   ```bash
   cd release
   ldd nesca.exe  # в Git Bash или MSYS2
   ```

#### Проблема: Ошибки линковки libssh/openssl/curl
**Решение:**
```bash
# Убедитесь, что библиотеки установлены
pacman -Q | grep -E "(libssh|openssl|curl)"

# Если отсутствуют:
pacman -S mingw-w64-x86_64-libssh mingw-w64-x86_64-openssl mingw-w64-x86_64-curl

# Проверьте наличие .dll файлов
ls /mingw64/bin/*.dll | grep -E "(ssh|ssl|curl)"
```

#### Проблема: Сборка работает, но программа не запускается
**Решение:**
1. Проверьте, что все DLL находятся в одной папке с .exe файлом
2. Используйте Dependency Walker (depends.exe) для проверки отсутствующих DLL
3. Убедитесь, что используете правильную разрядность (x64)

---

## 📝 Проверка успешной сборки

### Linux:
```bash
# Проверка файла
file nesca
# Должно показать: ELF 64-bit LSB executable

# Проверка зависимостей
ldd nesca | grep -E "(Qt|ssh|ssl|curl)"

# Тестовый запуск
./nesca --help
```

### Windows:
```bash
# В MSYS2 MinGW 64-bit
file release/nesca.exe
# Должно показать: PE32+ executable

# Запуск
cd release
./nesca.exe --help
```

---

## 🎯 Следующие шаги

После успешной компиляции:

1. **Проверка работоспособности:**
   ```bash
   ./nesca --help  # Linux
   release\nesca.exe --help  # Windows
   ```

2. **Тестовое сканирование:**
   ```bash
   ./nesca --ip 127.0.0.1 -p 80,443
   ```

3. **Использование:**
   См. [README.md](README.md) для примеров использования

---

## 📚 Дополнительные ресурсы

- [Официальная документация Qt](https://doc.qt.io/)
- [MSYS2 документация](https://www.msys2.org/docs/)
- [MinGW-w64 документация](https://www.mingw-w64.org/)

---

**Удачи в компиляции!** 🚀

