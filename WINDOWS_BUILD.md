# Инструкция по компиляции Windows бинарника

Это руководство поможет вам скомпилировать Windows бинарник для Nesca.

## Варианты сборки

### Вариант 1: MXE (M cross environment) - Рекомендуется

MXE включает в себя все необходимые библиотеки, включая Qt для Windows.

#### Установка MXE:

```bash
# Клонировать MXE
git clone https://github.com/mxe/mxe.git ~/mxe

# Установить необходимые зависимости для компиляции MXE
# Arch Linux:
sudo pacman -S p7zip autoconf automake bash bison bzip2 \
    cmake flex gettext git gperf intltool libtool \
    libxslt make openssl patch perl pkg-config python \
    ruby sed unzip wget xz yasm nasm
    
# Ubuntu/Debian:
sudo apt-get install -y autoconf automake autopoint bash \
    bison bzip2 cmake flex gettext git gperf intltool \
    libffi-dev libtool libltdl-dev libssl-dev libxml-parser-perl \
    make openssl patch perl pkg-config python3 ruby \
    sed unzip wget xz-utils yasm

# Скомпилировать Qt5 и необходимые библиотеки
cd ~/mxe
make qtbase qt5base libssh openssl curl nghttp2 -j$(nproc)

# Это займет некоторое время (30-60 минут в зависимости от системы)
```

#### Сборка проекта:

```bash
cd /home/kron4k/Downloads/nesca-remake
./build_windows.sh
```

Скрипт автоматически найдет MXE и использует его для сборки.

### Вариант 2: Локальный mingw-w64

Этот вариант требует отдельной установки Qt для Windows.

#### Установка mingw-w64:

```bash
# Arch Linux:
sudo pacman -S mingw-w64-gcc mingw-w64-binutils

# Ubuntu/Debian:
sudo apt-get install mingw-w64 mingw-w64-tools
```

**Проблема:** Qt для Windows нужно устанавливать отдельно, что сложно. Рекомендуется использовать MXE.

### Быстрая установка MXE на Arch Linux:

```bash
# 1. Установить зависимости для MXE
sudo pacman -S --needed \
    autoconf automake bash bison bzip2 \
    cmake flex gettext git gperf intltool \
    libtool make openssl patch perl \
    pkg-config python ruby sed unzip wget xz yasm nasm \
    p7zip lzip

# 2. Клонировать и собрать MXE (если еще не клонирован)
# git clone https://github.com/mxe/mxe.git ~/mxe
cd ~/mxe

# 3. Проверить требования
make check-requirements

# 4. Собрать Qt5 и библиотеки (займет 30-60 минут)
make qtbase qt5base libssh openssl curl nghttp2 -j$(nproc)

# 5. После установки MXE, собрать проект
cd /home/kron4k/Downloads/nesca-remake
./build_windows.sh
```

**Важно:** 
- Не запускайте `build_windows.sh` из директории MXE - запускайте из директории проекта!
- Если MXE уже установлен, просто перейдите в директорию проекта и запустите `./build_windows.sh`

### Вариант 3: Docker

Если у вас установлен Docker, можно использовать готовый образ:

```bash
./build_windows.sh --docker
```

**Примечание:** Dockerfile требует доработки для полноценной работы.

### Вариант 4: Использование GitHub Actions / CI

Для автоматической сборки Windows бинарников можно настроить GitHub Actions.

## После сборки

После успешной сборки бинарник будет находиться в:
- `release_windows/nesca.exe`

Также будут скопированы необходимые DLL файлы (если используется MXE с shared библиотеками).

## Проверка бинарника

Вы можете проверить бинарник на Windows или используя Wine:

```bash
# Установка Wine (если не установлен)
# Arch Linux:
sudo pacman -S wine

# Запуск через Wine
wine release_windows/nesca.exe --help
```

## Требования к бинарнику

Бинарник требует следующие DLL (для shared сборки):
- Qt5Core.dll
- Qt5Network.dll
- libgcc_s_seh-1.dll
- libstdc++-6.dll
- libwinpthread-1.dll
- libssh.dll
- libcrypto-*.dll
- libssl-*.dll
- libcurl-*.dll

Все эти DLL будут автоматически скопированы при использовании MXE.

## Устранение проблем

### Проблема: MXE компилируется очень долго

**Решение:** Используйте больше ядер процессора:
```bash
make qtbase -j$(nproc) JOBS=4
```

### Проблема: Ошибка "cannot find -lssh"

**Решение:** Убедитесь, что libssh скомпилирован в MXE:
```bash
cd ~/mxe && make libssh
```

### Проблема: Бинарник не запускается на Windows

**Решение:** 
1. Проверьте, что все DLL скопированы в папку с бинарником
2. Используйте статическую сборку MXE (x86_64-w64-mingw32.static)
3. Проверьте зависимости: `ldd nesca.exe` в Git Bash на Windows

## Быстрая сборка

Если MXE уже установлен:

```bash
./build_windows.sh
```

Скрипт автоматически найдет MXE и скомпилирует проект.

