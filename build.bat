@echo off
REM Скрипт сборки Nesca для Windows (MSYS2 MINGW)
REM Убедитесь, что MSYS2 MINGW установлен и настроен

echo === Сборка Nesca для MSYS2 MINGW ===

REM Проверяем наличие MSYS2
if not exist "C:\msys64\mingw64\bin\qmake.exe" (
    echo Ошибка: MSYS2 MINGW не найден в C:\msys64\
    echo Установите MSYS2 с https://www.msys2.org/
    pause
    exit /b 1
)

REM Добавляем MSYS2 в PATH
set PATH=C:\msys64\mingw64\bin;C:\msys64\usr\bin;%PATH%

echo Проверка зависимостей...

REM Проверяем qmake
qmake --version >nul 2>&1
if errorlevel 1 (
    echo Ошибка: qmake не найден. Установите Qt5 через MSYS2:
    echo pacman -S mingw-w64-x86_64-qt5-base mingw-w64-x86_64-qt5-multimedia
    pause
    exit /b 1
)

echo Очистка предыдущей сборки...
mingw32-make clean >nul 2>&1

echo Генерация Makefile...
qmake nesca.pro
if errorlevel 1 (
    echo Ошибка: Не удалось сгенерировать Makefile
    pause
    exit /b 1
)

echo Компиляция проекта...
mingw32-make -j4
if errorlevel 1 (
    echo Ошибка: Компиляция не удалась
    pause
    exit /b 1
)

echo Копирование DLL в папку release...
REM Создаем папку release если её нет
if not exist "release" mkdir release

echo Копирование Qt DLL...
copy "C:\msys64\mingw64\bin\Qt5Core.dll" "release\" >nul 2>&1
copy "C:\msys64\mingw64\bin\Qt5Gui.dll" "release\" >nul 2>&1
copy "C:\msys64\mingw64\bin\Qt5Widgets.dll" "release\" >nul 2>&1
copy "C:\msys64\mingw64\bin\Qt5Multimedia.dll" "release\" >nul 2>&1
copy "C:\msys64\mingw64\bin\Qt5Network.dll" "release\" >nul 2>&1
copy "C:\msys64\mingw64\bin\Qt5Svg.dll" "release\" >nul 2>&1

echo Копирование системных DLL...
copy "C:\msys64\mingw64\bin\libgcc_s_seh-1.dll" "release\" >nul 2>&1
copy "C:\msys64\mingw64\bin\libstdc++-6.dll" "release\" >nul 2>&1
copy "C:\msys64\mingw64\bin\libwinpthread-1.dll" "release\" >nul 2>&1

echo Копирование библиотек...
copy "C:\msys64\mingw64\bin\libssh.dll" "release\" >nul 2>&1
copy "C:\msys64\mingw64\bin\libcrypto-*.dll" "release\" >nul 2>&1
copy "C:\msys64\mingw64\bin\libssl-*.dll" "release\" >nul 2>&1
copy "C:\msys64\mingw64\bin\libcurl-*.dll" "release\" >nul 2>&1

echo Копирование плагинов Qt...
if not exist "release\platforms" mkdir "release\platforms"
copy "C:\msys64\mingw64\share\qt5\plugins\platforms\qwindows.dll" "release\platforms\" >nul 2>&1

echo === Сборка завершена успешно! ===
echo Исполняемый файл: release\nesca.exe
echo Для запуска используйте: run.bat
pause
