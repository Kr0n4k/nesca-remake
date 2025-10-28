# 🚀 Быстрая сборка Nesca

## Linux

### Автоматическая сборка (рекомендуется)
```bash
chmod +x scripts/build-linux.sh
./scripts/build-linux.sh
```

### Ручная сборка
```bash
# 1. Установка зависимостей (Ubuntu/Debian)
sudo apt-get update
sudo apt-get install -y qtbase5-dev qtbase5-dev-tools libssh-dev libssl-dev libcurl4-openssl-dev build-essential

# 2. Компиляция
qmake nesca.pro
make -j$(nproc)

# 3. Запуск
./nesca --help
```

---

## Windows 11 (MSYS2)

### 1. Установка MSYS2
Скачайте с https://www.msys2.org/ и установите

### 2. Установка зависимостей
Откройте **MSYS2 MinGW 64-bit**:
```bash
pacman -Syu
pacman -S mingw-w64-x86_64-qt5-base mingw-w64-x86_64-gcc mingw-w64-x86_64-make \
          mingw-w64-x86_64-libssh mingw-w64-x86_64-openssl mingw-w64-x86_64-curl
```

### 3. Компиляция
```bash
cd /c/path/to/nesca-v2r  # Замените на ваш путь
qmake nesca.pro
mingw32-make -j4
```

### 4. Подготовка DLL
```bash
mkdir -p release
cp nesca.exe release/
cp /mingw64/bin/Qt5Core.dll release/
cp /mingw64/bin/Qt5Network.dll release/
cp /mingw64/bin/libgcc_s_seh-1.dll release/
cp /mingw64/bin/libstdc++-6.dll release/
cp /mingw64/bin/libssh.dll release/
cp /mingw64/bin/libcrypto-*.dll release/ 2>/dev/null || true
cp /mingw64/bin/libssl-*.dll release/ 2>/dev/null || true
```

### 5. Запуск
```bash
cd release
./nesca.exe --help
```

---

## 📚 Подробная документация

См. [COMPILE_GUIDE.md](COMPILE_GUIDE.md) для детальных инструкций и решения проблем.

---

**Архитектура:** Проект собирается для x86_64/amd64 (64-bit).

