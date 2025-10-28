# Инструкция по сборке и запуску Nesca через MSYS2 MINGW

## Предварительные требования

### 1. Установка MSYS2
Скачайте и установите MSYS2 с официального сайта: https://www.msys2.org/

### 2. Установка необходимых пакетов
Откройте MSYS2 MINGW64 и выполните следующие команды:

```bash
# Обновление системы
pacman -Syu

# Установка Qt5 и зависимостей
pacman -S mingw-w64-x86_64-qt5-base mingw-w64-x86_64-qt5-multimedia mingw-w64-x86_64-qt5-svg

# Установка библиотек
pacman -S mingw-w64-x86_64-libssh mingw-w64-x86_64-openssl mingw-w64-x86_64-curl

# Установка инструментов разработки
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-make mingw-w64-x86_64-pkg-config
```

## Сборка проекта

### Автоматическая сборка
```bash
# Сделайте скрипты исполняемыми
chmod +x build.sh run.sh

# Запустите сборку
./build.sh
```

### Ручная сборка
```bash
# Очистка предыдущей сборки
make clean

# Генерация Makefile
qmake nesca.pro

# Компиляция
make -j4

# Копирование DLL (выполняется автоматически в build.sh)
```

## Запуск приложения

```bash
# Запуск Nesca
./run.sh
```

Или напрямую:
```bash
cd release
./nesca.exe
```

## Структура проекта

- `nesca.pro` - файл проекта Qt
- `build.sh` - скрипт автоматической сборки
- `run.sh` - скрипт запуска приложения
- `release/` - папка с исполняемым файлом и DLL
- `release/platforms/` - плагины Qt

## Возможные проблемы

### Ошибка "qmake не найден"
Убедитесь, что Qt5 установлен:
```bash
pacman -S mingw-w64-x86_64-qt5-base
```

### Ошибка "libssh не найден"
Установите libssh:
```bash
pacman -S mingw-w64-x86_64-libssh
```

### Ошибка при запуске "DLL не найдена"
1. Убедитесь, что все DLL скопированы в папку `release/`
2. Проверьте, что путь к папке `release` добавлен в PATH
3. Пересоберите проект: `./build.sh`

### Проблемы с плагинами Qt
Убедитесь, что папка `release/platforms/` содержит `qwindows.dll`

## Дополнительные DLL

Проект может требовать дополнительные DLL в зависимости от функциональности:
- `libcurl-*.dll` - для HTTP запросов
- `libcrypto-*.dll`, `libssl-*.dll` - для SSL/TLS
- `libssh.dll` - для SSH соединений
- `HCNetSDK.dll` - для работы с камерами Hikvision

Все необходимые DLL автоматически копируются скриптом `build.sh`.
