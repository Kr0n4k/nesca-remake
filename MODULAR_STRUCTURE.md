# Модульная архитектура проекта

Проект реорганизован в модульную структуру для лучшей поддерживаемости и расширяемости.

## Структура директорий

```
src/
├── common/          # Общие заголовки и ресурсы
│   ├── externData.h
│   ├── externFunctions.h
│   ├── mainResources.h
│   └── resource.h
│
├── core/            # Ядро сканирования
│   ├── main.cpp
│   ├── MainStarter.cpp/h
│   ├── Threader.cpp/h
│   ├── STh.cpp/h
│   └── finder.cpp
│
├── detectors/       # Детекторы устройств
│   ├── DeviceIdentifier.cpp/h
│   └── FingerprintScanner.cpp/h
│
├── auth/            # Методы аутентификации
│   ├── BasicAuth.cpp/h
│   ├── FTPAuth.cpp/h
│   ├── SSHAuth.cpp/h
│   ├── RTSP.cpp/h
│   ├── IPCAuth.cpp/h
│   ├── HikvisionLogin.cpp/h
│   └── WebformWorker.cpp/h
│
├── exporters/       # Экспортеры результатов
│   └── ResultExporter.cpp/h
│
├── network/         # Сетевые утилиты
│   ├── Connector.cpp/h
│   ├── Utils.cpp/h
│   ├── BruteUtils.cpp/h
│   ├── AdaptiveScanner.cpp/h
│   ├── FileDownloader.cpp/h
│   ├── FileUpdater.cpp/h
│   ├── IPRandomizer.cpp/h
│   ├── base64.cpp/h
│   └── ProgressMonitor.cpp/h
│
└── ui/              # GUI компоненты (опционально)
    ├── nesca_3.cpp/h/ui/qrc/pro
    ├── ActivityDrawerTh_HorNet.cpp/h
    ├── CheckKey_Th.cpp/h
    ├── DrawerTh_*.cpp/h
    ├── progressbardrawer.cpp/h
    ├── piestat.cpp/h
    ├── msgcheckerthread.cpp/h
    └── vercheckerthread.cpp/h
```

## Миграция

### Шаг 1: Перемещение файлов

Выполните скрипт реорганизации:
```bash
./reorganize.sh
```

Или переместите файлы вручную согласно структуре выше.

### Шаг 2: Обновление путей в .pro файле

Используйте `nesca.pro.modular` как основу или обновите существующий `nesca.pro`:

```qmake
INCLUDEPATH += $$PWD/src/common \
               $$PWD/src/core \
               $$PWD/src/detectors \
               $$PWD/src/auth \
               $$PWD/src/exporters \
               $$PWD/src/network \
               $$PWD/src/ui
```

### Шаг 3: Обновление include путей

Обновите все `#include` директивы в исходных файлах:

**Старый формат:**
```cpp
#include "MainStarter.h"
#include "Connector.h"
#include "DeviceIdentifier.h"
```

**Новый формат:**
```cpp
#include "core/MainStarter.h"        // или <core/MainStarter.h>
#include "network/Connector.h"       // или <network/Connector.h>
#include "detectors/DeviceIdentifier.h" // или <detectors/DeviceIdentifier.h>
```

**Для common заголовков:**
```cpp
#include "common/externData.h"       // или <common/externData.h>
#include "common/externFunctions.h"
```

### Важные замечания

1. **Общие заголовки** (`externData.h`, `externFunctions.h`, `mainResources.h`) находятся в `src/common/`**

2. **Циклические зависимости**: Убедитесь, что модули не создают циклических зависимостей

3. **Пути к ресурсам**: Если используются относительные пути к файлам (например, `pwd_lists/`, `results_*/`), они остаются в корне проекта

4. **GUI компоненты**: Опциональны для console-only сборки

## Преимущества новой структуры

- ✅ Логическая организация кода
- ✅ Упрощенная навигация
- ✅ Легче добавлять новые модули
- ✅ Ясные зависимости между модулями
- ✅ Проще тестирование и отладка
- ✅ Лучшая масштабируемость

## Обратная совместимость

Для постепенной миграции можно использовать символические ссылки:
```bash
ln -s src/core/MainStarter.h MainStarter.h
```

Но рекомендуется полностью мигрировать на новую структуру.

