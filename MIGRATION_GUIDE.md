# Руководство по миграции на модульную архитектуру

## Обзор

Проект реорганизован в модульную структуру для улучшения поддерживаемости и масштабируемости.

## Быстрая миграция

### Автоматическая миграция (рекомендуется)

```bash
# 1. Выполнить скрипт перемещения файлов
./reorganize.sh

# 2. Обновить include пути
chmod +x update_includes.sh
./update_includes.sh

# 3. Обновить .pro файл
cp nesca.pro nesca.pro.backup  # Создать резервную копию
cp nesca.pro.modular nesca.pro  # Использовать модульную версию

# 4. Пересобрать проект
qmake
make clean
make
```

### Ручная миграция

Если автоматический скрипт не работает, выполните шаги вручную:

#### Шаг 1: Переместить файлы

```bash
mkdir -p src/{core,detectors,auth,exporters,network,ui,common}

# Core
mv MainStarter.cpp MainStarter.h Threader.cpp Threader.h STh.cpp STh.h finder.cpp src/core/

# Detectors
mv DeviceIdentifier.cpp DeviceIdentifier.h FingerprintScanner.cpp FingerprintScanner.h src/detectors/

# Auth
mv BasicAuth.* FTPAuth.* SSHAuth.* RTSP.* IPCAuth.* HikvisionLogin.* WebformWorker.* src/auth/

# Exporters
mv ResultExporter.* src/exporters/

# Network
mv Connector.* Utils.* BruteUtils.* AdaptiveScanner.* FileDownloader.* FileUpdater.* IPRandomizer.* base64.* ProgressMonitor.* src/network/

# Common
mv externData.h externFunctions.h mainResources.h resource.h src/common/
```

#### Шаг 2: Обновить include пути

Обновите все `#include` директивы:

```cpp
// Старый формат
#include "MainStarter.h"
#include "Connector.h"

// Новый формат (вариант 1 - с путем)
#include "core/MainStarter.h"
#include "network/Connector.h"

// Новый формат (вариант 2 - через INCLUDEPATH в .pro)
// После настройки INCLUDEPATH можно использовать:
#include <core/MainStarter.h>
#include <network/Connector.h>
```

#### Шаг 3: Обновить .pro файл

Используйте `nesca.pro.modular` как основу. Основные изменения:

```qmake
# Добавить include paths
INCLUDEPATH += $$PWD/src/common \
               $$PWD/src/core \
               $$PWD/src/detectors \
               $$PWD/src/auth \
               $$PWD/src/exporters \
               $$PWD/src/network \
               $$PWD/src/ui

# Обновить пути к файлам
SOURCES += main.cpp \
           src/core/MainStarter.cpp \
           src/core/Threader.cpp \
           ...
```

## Структура после миграции

```
nesca-v2r/
├── main.cpp                    # Точка входа (остается в корне)
├── main_console.cpp            # Console entry point
├── nesca.pro                   # Обновленный .pro файл
├── src/
│   ├── common/                 # Общие заголовки
│   │   ├── externData.h
│   │   ├── externFunctions.h
│   │   ├── mainResources.h
│   │   └── resource.h
│   ├── core/                   # Ядро сканирования
│   │   ├── MainStarter.cpp/h
│   │   ├── Threader.cpp/h
│   │   ├── STh.cpp/h
│   │   └── finder.cpp
│   ├── detectors/              # Детекторы
│   │   ├── DeviceIdentifier.cpp/h
│   │   └── FingerprintScanner.cpp/h
│   ├── auth/                   # Аутентификация
│   │   ├── BasicAuth.cpp/h
│   │   ├── FTPAuth.cpp/h
│   │   ├── SSHAuth.cpp/h
│   │   ├── RTSP.cpp/h
│   │   ├── IPCAuth.cpp/h
│   │   ├── HikvisionLogin.cpp/h
│   │   └── WebformWorker.cpp/h
│   ├── exporters/              # Экспортеры
│   │   └── ResultExporter.cpp/h
│   ├── network/                # Сетевые утилиты
│   │   ├── Connector.cpp/h
│   │   ├── Utils.cpp/h
│   │   ├── BruteUtils.cpp/h
│   │   ├── AdaptiveScanner.cpp/h
│   │   ├── FileDownloader.cpp/h
│   │   ├── FileUpdater.cpp/h
│   │   ├── IPRandomizer.cpp/h
│   │   ├── base64.cpp/h
│   │   └── ProgressMonitor.cpp/h
│   └── ui/                     # GUI (опционально)
│       └── ...
└── ...
```

## Проверка после миграции

1. Проверьте компиляцию:
   ```bash
   qmake
   make clean
   make
   ```

2. Проверьте, что все модули найдены:
   ```bash
   # Проверить наличие всех файлов в новых директориях
   find src/ -name "*.cpp" -o -name "*.h" | wc -l
   ```

3. Проверьте include пути:
   ```bash
   # Найти все include директивы
   grep -r "#include" src/ main.cpp main_console.cpp | grep -E "(core|detectors|auth|exporters|network|common)/"
   ```

## Откат изменений

Если миграция не удалась:

```bash
# Восстановить резервную копию .pro
cp nesca.pro.backup nesca.pro

# Переместить файлы обратно (если были перемещены)
# (нужно выполнить обратное перемещение вручную)
```

## Типичные проблемы

### Проблема: Не найден заголовочный файл

**Решение**: Проверьте `INCLUDEPATH` в `.pro` файле и убедитесь, что путь правильный.

### Проблема: Циклические зависимости

**Решение**: Проверьте зависимости между модулями. Общие заголовки должны быть в `src/common/`.

### Проблема: Файлы не найдены при сборке

**Решение**: Убедитесь, что все пути в `.pro` файле указаны относительно `$$PWD`.

## Дополнительные ресурсы

- `MODULAR_STRUCTURE.md` - Подробное описание структуры
- `nesca.pro.modular` - Пример модульного .pro файла
- `reorganize.sh` - Скрипт автоматической реорганизации

