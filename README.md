# Nesca Remake - Network Scanner & Device Discovery

[![License](https://img.shields.io/badge/license-Unlicense-blue.svg)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-5.15+-green.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C%2B%2B-11-orange.svg)](https://isocpp.org/)

**Легендарный ремейк сетевого сканера для поиска IP-камер, серверов и сетевых устройств**

Nesca — мощный инструмент для сканирования сетей, обнаружения IP-камер, серверов и других сетевых устройств с поддержкой автоматического определения типов устройств, экспорта результатов и живых статистик.

## 🌟 Основные возможности

- **Многорежимное сканирование**
  - IP-диапазоны (CIDR, диапазоны, отдельные IP)
  - DNS-сканирование с поддержкой масок
  - Импорт списка целей из файла

- **Автоматическое определение устройств**
  - Распознавание производителей: Hikvision, Dahua, Axis, Panasonic, Sony и др.
  - Определение моделей и версий прошивок
  - Классификация типов устройств (камеры, DVR, NVR, серверы)

- **Аутентификация и брутфорс**
  - HTTP Basic Auth
  - FTP аутентификация
  - SSH подключения
  - Web-форм аутентификация
  - Hikvision SDK интеграция
  - RTSP поддержка

- **Экспорт результатов** (включен по умолчанию)
  - JSON формат с полной статистикой
  - CSV формат для Excel/анализа
  - Фильтрация по IP, портам, типам устройств
  - Экспорт существующих результатов без пересканирования

- **Статистика в реальном времени**
  - Живое отображение прогресса сканирования
  - Скорость сканирования (IP/сек)
  - Оценка времени до завершения (ETA)
  - Счетчики по типам устройств

- **Многопоточность**
  - Высокая производительность
  - Настраиваемое количество потоков
  - Оптимизированное использование ресурсов

## 📋 Требования

- **Qt 5.15+** (core, network)
- **libssh** - для SSH подключений
- **OpenSSL** - для SSL/TLS
- **libcurl** - для HTTP запросов
- **C++11 компилятор** (GCC 4.8+, Clang 3.3+, MSVC 2015+)

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
    build-essential
```

#### Arch Linux:
```bash
sudo pacman -S qt5-base libssh openssl curl base-devel
```

#### macOS (Homebrew):
```bash
brew install qt5 libssh openssl curl
export PATH="/usr/local/opt/qt5/bin:$PATH"
```

#### Windows (MSYS2/MinGW):
```bash
pacman -S mingw-w64-x86_64-qt5-base mingw-w64-x86_64-libssh mingw-w64-x86_64-openssl mingw-w64-x86_64-curl
```

## 🚀 Быстрый старт

### Сборка из исходников

```bash
# Клонирование репозитория
git clone <repository-url>
cd nesca-v2r

# Конфигурация проекта
qmake nesca.pro

# Компиляция
make -j$(nproc)  # Linux/macOS
mingw32-make     # Windows MinGW
```

### Использование

```bash
# Сканирование IP-диапазона (с автоматическим экспортом в JSON)
./nesca --ip 192.168.1.0/24 -p 80,443,8080

# Сканирование с экспортом в CSV
./nesca --ip 192.168.1.0/24 -p 80,443 --export csv

# DNS сканирование
./nesca --dns camera[a-z] --tld .com -p 80,443

# Импорт списка IP из файла
./nesca --import ip_list.txt -p 80,443,8080

# Экспорт существующих результатов
./nesca --export-only

# Экспорт с фильтрацией
./nesca --export-only --export-filter-type camera,auth --export csv
```

## 📖 Документация

Полная документация доступна в директории [`docs/`](docs/):

- **[Экспорт результатов](docs/EXPORT_FEATURE.md)** - Руководство по экспорту
- **[Новые функции](docs/NEW_FEATURES.md)** - Описание новых возможностей
- **[Инструкции по сборке](docs/BUILD_INSTRUCTIONS.md)** - Детальные инструкции

## 📊 Примеры использования

### Базовое сканирование

```bash
# Сканирование локальной сети с портами по умолчанию
./nesca --ip 192.168.1.0/24

# Результаты будут автоматически экспортированы в export_YYYY.MM.DD_target.json
```

### Расширенное сканирование

```bash
# Сканирование с настройками
./nesca \
  --ip 10.0.0.0/16 \
  -p 80,443,8080,554,37777,8000 \
  -t 200 \
  --export both \
  --export-file scan_results

# Это создаст:
# - scan_results.json
# - scan_results.csv
```

### Фильтрация и экспорт

```bash
# Экспорт только камер Hikvision
./nesca --export-only \
  --export-filter-type camera \
  --export-filter-ip 192.168.1 \
  --export json \
  --export-file hikvision_cameras
```

### Отключение функций

```bash
# Сканирование без экспорта и без живой статистики
./nesca --ip 192.168.1.0/24 --no-export --no-live-stats
```

## 🎯 Поддерживаемые режимы сканирования

### IP Scan Mode
```bash
./nesca --ip <TARGET>
```

**Поддерживаемые форматы:**
- CIDR: `192.168.1.0/24`
- Диапазон: `192.168.1.1-192.168.1.255`
- Отдельный IP: `192.168.1.100`

### DNS Scan Mode
```bash
./nesca --dns <MASK> [--tld <TLD>]
```

**Примеры:**
```bash
./nesca --dns camera[a-z] --tld .com -p 80,443
./nesca --dns test[0-9] --tld .ru -p 8080
```

### Import Mode
```bash
./nesca --import <FILE>
```

Файл должен содержать по одному IP-адресу на строку.

## ⚙️ Опции командной строки

### Режимы сканирования
- `--ip IP_RANGE` - IP режим сканирования
- `--dns DNS_MASK` - DNS режим сканирования  
- `--import FILE` - Импорт из файла

### Основные опции
- `-p, --ports PORTS` - Порты для сканирования (по умолчанию: `80,81,88,8080,8081,60001,60002,8008,8888,554,9000,3536,21`)
- `-t, --threads N` - Количество потоков (по умолчанию: 100)
- `--tld TLD` - TLD для DNS сканирования (по умолчанию: `.com`)

### Экспорт (по умолчанию: JSON)
- `--export FORMAT` - Формат: `json`, `csv`, `both` (по умолчанию: `json`)
- `--no-export` - Отключить автоматический экспорт
- `--export-only [DIR]` - Экспортировать существующие результаты
- `--export-file FILE` - Имя файла для экспорта
- `--export-filter-ip IP` - Фильтр по IP-паттерну
- `--export-filter-port PORTS` - Фильтр по портам
- `--export-filter-type TYPES` - Фильтр по типам: `camera`, `auth`, `ftp`, `ssh`, `other`

### Статистика
- `--live-stats` - Включить живую статистику (включена по умолчанию)
- `--no-live-stats` - Отключить живую статистику

### Справка
- `-h, --help` - Показать справку

## 📁 Структура проекта

```
nesca-v2r/
├── docs/                   # Документация
│   ├── EXPORT_FEATURE.md
│   ├── NEW_FEATURES.md
│   └── BUILD_INSTRUCTIONS.md
├── scripts/                # Скрипты сборки и утилиты
│   ├── build.sh
│   ├── generator.py
│   └── ...
├── resources/              # Ресурсы (шрифты, иконки)
├── tests/                  # Тестовые файлы
├── pwd_lists/              # Списки паролей для брутфорса
├── *.cpp, *.h             # Исходный код
├── nesca.pro              # Qt проект
├── README.md              # Этот файл
└── .gitignore            # Git ignore правила
```

## 🛠️ Разработка

### Сборка в режиме отладки

```bash
qmake CONFIG+=debug nesca.pro
make
```

### Сборка в режиме релиза

```bash
qmake CONFIG+=release nesca.pro
make
```

### Очистка проекта

```bash
make clean
rm -f moc_* qrc_* ui_* *.o
```

## 🐳 Docker

```bash
# Сборка образа
docker build -t nesca .

# Запуск
docker run -it --rm \
  -v $(pwd)/results:/app/results \
  nesca --ip 192.168.1.0/24 -p 80,443
```

## 📝 Формат результатов

### JSON формат

```json
{
  "export_timestamp": "2025-01-15T12:00:00",
  "total_results": 150,
  "filtered_results": 45,
  "results": [
    {
      "ip": "192.168.1.1",
      "port": 80,
      "type": "camera",
      "timestamp": "Wed Jan 15 12:00:00 2025",
      "data": "...",
      "login": "",
      "password": ""
    }
  ],
  "statistics": {
    "by_type": {"camera": 30, "auth": 15},
    "top_ports": [{"port": 80, "count": 20}],
    "total": 150
  }
}
```

### CSV формат

```csv
IP,Port,Type,Timestamp,Data,Login,Password
192.168.1.1,80,camera,"Wed Jan 15 12:00:00 2025","...","",""
```

## 🔍 Определение устройств

Nesca автоматически определяет:

- **Производителей**: Hikvision, Dahua, Axis, Panasonic, Sony, Samsung, Bosch, Pelco, Vivotek, Foscam, Uniview, Reolink, TP-Link
- **Модели**: По шаблонам производителей
- **Типы**: Camera, DVR, NVR, Server
- **Версии прошивок**: Из HTTP заголовков

## ⚠️ Важные замечания

- Используйте только на сетях, где у вас есть разрешение
- Соблюдайте местное законодательство
- Результаты сохраняются в директории `results_*/`
- Экспорт включен по умолчанию в формате JSON
- Живая статистика обновляется каждые 2 секунды

## 🤝 Вклад в проект

Приветствуются pull requests и issue reports! Пожалуйста:

1. Fork проекта
2. Создайте feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit изменения (`git commit -m 'Add some AmazingFeature'`)
4. Push в branch (`git push origin feature/AmazingFeature`)
5. Откройте Pull Request

## 📄 Лицензия

Проект распространяется без лицензии (Unlicense). См. файл LICENSE для деталей.

## 🙏 Благодарности

Изначально разработано группой ISKOPASI как универсальный сканер для сетевых устройств.

## 📞 Поддержка

- Создайте Issue на GitHub для багов и предложений
- Проверьте [документацию](docs/) перед созданием Issue

---

**Nesca Remake** - Легендарный сканер. Твой. Улучшенный. 🚀
