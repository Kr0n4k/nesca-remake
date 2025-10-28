# Структура проекта Nesca v2r

```
nesca-v2r/
│
├── 📄 README.md                    # Основная документация проекта
├── 📄 LICENSE                      # Лицензия (Unlicense)
├── 📄 CHANGELOG.md                 # История изменений
├── 📄 STRUCTURE.md                 # Этот файл - обзор структуры
├── 📄 .gitignore                   # Правила исключения для Git
│
├── 📁 docs/                        # 📚 Документация
│   ├── README.md                   # Навигация по документации
│   ├── QUICK_START.md              # Быстрый старт
│   ├── EXPORT_FEATURE.md           # Руководство по экспорту
│   ├── NEW_FEATURES.md             # Новые функции
│   ├── PROJECT_STRUCTURE.md        # Детальная структура проекта
│   ├── BUILD_INSTRUCTIONS.md       # Инструкции по сборке
│   └── CONTRIBUTING.md             # Руководство для контрибьюторов
│
├── 📁 scripts/                     # 🔧 Скрипты и утилиты
│   ├── build.sh                    # Скрипт сборки (Linux/macOS)
│   ├── build.bat                   # Скрипт сборки (Windows)
│   ├── run.sh                      # Скрипт запуска
│   ├── run.bat                     # Скрипт запуска (Windows)
│   ├── create.sh                   # Вспомогательный скрипт
│   └── generator.py                # Генератор IP-диапазонов
│
├── 📁 resources/                   # 🎨 Ресурсы
│   ├── fonts/                      # Шрифты
│   │   ├── Eurostile.ttf
│   │   └── small_font.ttf
│   ├── icons/                      # Иконки
│   │   └── nesca.ico
│   └── images/                     # Изображения
│       └── 1.jpg
│
├── 📁 tests/                       # 🧪 Тесты
│   └── test_*.cpp                  # Тестовые файлы
│
├── 📁 pwd_lists/                   # 🔑 Списки паролей для брутфорса
│   ├── login.txt
│   ├── pass.txt
│   ├── ftplogin.txt
│   ├── ftppass.txt
│   ├── wflogin.txt
│   ├── wfpass.txt
│   └── sshpass.txt
│
├── 📁 examples/                    # 📝 Примеры
│   └── old_nesca.png              # Примеры интерфейса
│
├── 📁 results_*/                   # 📊 Результаты сканирования (генерируется)
│   └── [date]_[target]/
│       ├── camera.html
│       ├── auth.html
│       ├── ftp.html
│       ├── ssh.html
│       └── other.html
│
└── 📁 Исходный код (корень)
    │
    ├── 🔵 Основные файлы
    │   ├── main.cpp                # Главная точка входа (console)
    │   ├── main_console.cpp        # Альтернативный console mode
    │   └── nesca_3.cpp             # GUI версия
    │
    ├── ⚙️ Ядро системы
    │   ├── MainStarter.h/cpp       # Инициализация сканирования
    │   ├── STh.h/cpp               # Основной поток
    │   ├── Threader.h/cpp          # Управление потоками
    │   ├── Connector.h/cpp         # Сетевые подключения
    │   └── finder.cpp              # Поиск устройств
    │
    ├── 🔐 Аутентификация
    │   ├── BasicAuth.h/cpp
    │   ├── FTPAuth.h/cpp
    │   ├── SSHAuth.h/cpp
    │   ├── IPCAuth.h/cpp
    │   ├── HikvisionLogin.h/cpp
    │   ├── RTSP.h/cpp
    │   ├── WebformWorker.h/cpp
    │   └── BruteUtils.h/cpp
    │
    ├── 📤 Экспорт и анализ
    │   ├── ResultExporter.h/cpp    # Экспорт результатов
    │   ├── DeviceIdentifier.h/cpp  # Определение устройств
    │   └── ProgressMonitor.h/cpp   # Мониторинг прогресса
    │
    ├── 🛠️ Утилиты
    │   ├── Utils.h/cpp
    │   ├── base64.h/cpp
    │   ├── IPRandomizer.h/cpp
    │   ├── FileDownloader.h/cpp
    │   └── FileUpdater.h/cpp
    │
    ├── 🖥️ GUI компоненты (Qt)
    │   ├── nesca_3.h/cpp/ui
    │   ├── piestat.h/cpp
    │   ├── progressbardrawer.h/cpp
    │   └── DrawerTh_*.h/cpp
    │
    └── ⚙️ Конфигурация
        ├── nesca.pro               # Qt проект (console)
        ├── nesca_3.pro             # Qt проект (GUI)
        ├── mainResources.h         # Ресурсы и константы
        ├── externData.h            # Глобальные переменные
        └── externFunctions.h       # Глобальные функции
```

## Ключевые особенности структуры

### ✅ Организованность
- Четкое разделение по типам файлов
- Логическая группировка модулей
- Отдельная директория для документации

### ✅ Масштабируемость
- Легко добавлять новые модули
- Четкие интерфейсы между компонентами
- Разделение консольного и GUI режимов

### ✅ Документированность
- Полная документация в `docs/`
- README для быстрого старта
- Подробное описание структуры

### ✅ Удобство разработки
- Скрипты сборки в `scripts/`
- Тесты в отдельной директории
- Ресурсы организованы по типам

## Следующие шаги для улучшения

1. ✅ Создана структура директорий
2. ✅ Перемещены файлы документации
3. ✅ Перемещены скрипты
4. ✅ Создан .gitignore
5. ✅ Обновлен README.md
6. ⬜ Организовать исходный код в поддиректории (src/core, src/auth)
7. ⬜ Добавить CMakeLists.txt для альтернативной сборки
8. ⬜ Создать unit-тесты в tests/
9. ⬜ Добавить CI/CD конфигурацию

---

**Текущая версия структуры**: v2.0

