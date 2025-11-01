# Nesca Remake - Network Scanner & Device Discovery

[![License](https://img.shields.io/badge/license-Unlicense-blue.svg)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-5.15+-green.svg)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C%2B%2B-11-orange.svg)](https://isocpp.org/)

**Легендарный ремейк сетевого сканера для поиска IP-камер, серверов и сетевых устройств**

Nesca — мощный инструмент для сканирования сетей, обнаружения IP-камер, серверов и других сетевых устройств с поддержкой автоматического определения типов устройств, экспорта результатов и живых статистик.

# Документация Nesca Remake

Добро пожаловать в документацию проекта Nesca Remake!

## Основные темы

### Сканирование
- IP-диапазоны (CIDR, диапазоны, отдельные IP)
- DNS сканирование с масками
- Импорт из файлов
- Адаптивное сканирование (автоматическая оптимизация)
- Умное сканирование (приоритизация активных устройств)
- Глубокое сканирование (поиск эндпоинтов и путей)
- Сканирование уязвимостей (проверка CVE)
- Быстрые пресеты для типовых задач

### Конфигурация
- Конфигурационные файлы (.conf)
- Предустановленные профили (quick-scan, full-scan, stealth-scan, iot-scan, network-scan, pentest)
- Объединение настроек (CLI + config)
- Интерактивный режим

### Экспорт
- JSON формат с метаданными
- CSV формат для анализа
- Фильтрация результатов
- Экспорт без повторного сканирования

### Анализ
- Определение типов устройств
- Распознавание производителей
- Определение версий прошивок
- Статистика в реальном времени
- Фингерпринтинг устройств

### Мониторинг
- Непрерывное сканирование сети
- Отслеживание изменений (snapshots)
- Оповещения о новых устройствах

## Нужна помощь?

- Создайте [Issue](https://github.com/your-repo/issues) для багов и вопросов
- Изучите [примеры использования](QUICK_START.md)
- Проверьте [часто задаваемые вопросы](../README.md#-важные-замечания)

---

**Версия документации**: соответствует текущей версии проекта


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
```
или ./build_all.sh
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
Kr0n4k возродил проект

**Nesca Remake** - Легендарный сканер. Твой. Улучшенный. 🚀
