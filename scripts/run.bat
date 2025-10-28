@echo off
REM Скрипт запуска Nesca
REM Убедитесь, что приложение собрано и все DLL находятся в папке release

echo === Запуск Nesca ===

REM Проверяем наличие исполняемого файла
if not exist "release\nesca.exe" (
    echo Ошибка: Исполняемый файл nesca.exe не найден в папке release
    echo Сначала выполните сборку: build.bat
    pause
    exit /b 1
)

REM Проверяем наличие необходимых DLL
echo Проверка зависимостей...

set missing_dlls=0

if not exist "release\Qt5Core.dll" (
    echo Предупреждение: Qt5Core.dll не найден
    set missing_dlls=1
)
if not exist "release\Qt5Gui.dll" (
    echo Предупреждение: Qt5Gui.dll не найден
    set missing_dlls=1
)
if not exist "release\Qt5Widgets.dll" (
    echo Предупреждение: Qt5Widgets.dll не найден
    set missing_dlls=1
)
if not exist "release\Qt5Multimedia.dll" (
    echo Предупреждение: Qt5Multimedia.dll не найден
    set missing_dlls=1
)
if not exist "release\Qt5Network.dll" (
    echo Предупреждение: Qt5Network.dll не найден
    set missing_dlls=1
)
if not exist "release\libgcc_s_seh-1.dll" (
    echo Предупреждение: libgcc_s_seh-1.dll не найден
    set missing_dlls=1
)
if not exist "release\libstdc++-6.dll" (
    echo Предупреждение: libstdc++-6.dll не найден
    set missing_dlls=1
)
if not exist "release\libwinpthread-1.dll" (
    echo Предупреждение: libwinpthread-1.dll не найден
    set missing_dlls=1
)

if %missing_dlls%==1 (
    echo.
    echo Попробуйте пересобрать проект: build.bat
    echo.
)

REM Устанавливаем переменные окружения для Qt
set QT_PLUGIN_PATH=%CD%\release\platforms
set PATH=%CD%\release;%PATH%

echo Запуск Nesca...
cd release
nesca.exe

echo.
echo Nesca завершил работу.
pause
