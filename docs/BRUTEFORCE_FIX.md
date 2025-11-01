# Исправление брутфорса для Hikvision камер

## Проблема

Программа не брутфорсила камеры Hikvision на портах 8000 и 37777, даже если в базе есть подходящие логины и пароли.

## Причина

Проверка Hikvision камер (`checkHikk`, `checkRVI`) выполнялась **только если** размер ответа был равен 0 (`buffcpy->size() == 0`). Если камера возвращала какой-либо ответ, эта проверка пропускалась.

Код был:
```cpp
if (buffcpy->size() == 0) {
    if (port == 8000) {
        // проверка Hikvision
    }
    else if (port == 37777) {
        // проверка RVI
    }
}
```

## Исправление

Проверка Hikvision/RVI камер теперь выполняется **независимо от размера ответа** для портов 8000 и 37777:

```cpp
// Check Hikvision/RVI ports regardless of buffer size
if (port == 8000) {
    if (HikVis::checkHikk(ip, port))     return 4; //Hikkvision iVMS
    else if (HikVis::checkRVI(ip, port)) return 5; //RVI
}
else if (port == 37777) {
    if (HikVis::checkRVI(ip, port))      return 5; //RVI
    else if(HikVis::checkHikk(ip, port)) return 4; //Hikkvision iVMS
    else if (HikVis::checkSAFARI(ip, port)) return 6; //Safari CCTV
}
```

## Результат

Теперь программа будет:
1. Проверять все IP на портах 8000 и 37777
2. Определять тип камеры (Hikvision iVMS или RVI)
3. Запускать брутфорс с логами и паролями из базы
4. Сохранять результаты в файлы

## Использование

```bash
# Сканирование с портами Hikvision
./nesca --import camera_test.txt -p 80,81,88,8080,8081,60001,60002,8008,8888,554,9000,3536,21,37777,8000

# Брутфорс включен по умолчанию
# Максимальное количество потоков: 2000
# Логины: 37 записей
# Пароли: 55 записей
```

## Проверка

После исправления программа должна:
- Находить камеры на портах 8000 и 37777
- Брутфорсить их с логами и паролями
- Сохранять найденные учетные данные в:
  - `results_*/hikkafile_*.csv` - для Hikvision iVMS
  - `results_*/rvifile_*.xml` - для RVI камер

## Файлы результатов

Для Hikvision iVMS (flag 4):
```
results_YYYY.MM.DD_target/hikkafile_YYYY.MM.DD_part_N.csv
```

Для RVI камер (flag 5):
```
results_YYYY.MM.DD_target/rvifile_YYYY.MM.DD(ip)_part_N.xml
```

## Тестирование

Проверьте файл `camera_test.txt`:
```
79.120.58.1-79.120.58.254
81.28.167.1-81.28.167.254
```

Программа должна найти камеры и успешно их брутфорсить, если логины/пароли есть в базе.

