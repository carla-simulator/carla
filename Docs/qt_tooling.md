# Qt tooling quick start

## Install Qt6

**Qt version:** Qt6 (recommended) or Qt5.

**Ubuntu/Debian:**
```sh
# Qt6 (recommended)
sudo apt-get install qt6-base-dev qt6-tools-dev
# Or Qt5
sudo apt-get install qtbase5-dev qttools5-dev
```

**macOS (Homebrew):**
```sh
brew install qt6
# or
brew install qt5
```

**Fedora/RHEL:**
```sh
sudo dnf install qt6-qtbase-devel qt6-qttools-devel
# or
sudo dnf install qt5-qtbase-devel qt5-qttools-devel
```

**Windows (MSVC, Administrator):**
```cmd
choco install qt --version=6.4.2
```
or download from [qt.io/download](https://www.qt.io/download) (choose Qt5 or Qt6)

## Verify Installation

**Check core libraries:**
```sh
ldconfig -p | grep libQt6
```

**Expected .so files (minimum):**
```
Qt6:
 libQt6Core.so.6        → Core functionality
 libQt6Gui.so.6         → Graphics rendering
 libQt6Widgets.so.6     → UI components

Qt5:
 libQt5Core.so.5        → Core functionality
 libQt5Gui.so.5         → Graphics rendering
 libQt5Widgets.so.5     → UI components
```

**Detailed check:**
```sh
# List all Qt6 libraries
pkg-config --list-all | grep -i qt6

# Check specific library
pkg-config --modversion Qt6Core

# Verify linked libraries
ldd /path/to/binary | grep libQt6
```

**Quick validation:**
```sh
qmake6 --version          # Qt6
# or
qmake5 --version          # Qt5
```
Should output: `Using Qt version 5.x.x` or `6.x.x`

## Build

```sh
cd Build
cmake .. -DBUILD_QT_TOOLING=ON
cmake --build . --target carla-example-qt-client
```

## Run

```sh
./Examples/carla-example-qt-client
```

Shows:
- Host: localhost:2000
- Client/Server API versions
- Connection status

## Configure

Edit [Examples/QtClient/main.cpp](../../Examples/QtClient/main.cpp):
```cpp
carla::client::Client client("192.168.1.100", 2000u);
```

## Requirements

- Qt6 development libraries
- `BUILD_QT_TOOLING=ON`

## Optional

`BUILD_QT_TOOLING` defaults to `OFF`. If Qt6 missing, build skips Qt and continues normally.

## CarlaStudio docs

The detailed Qt application surface now lives in the CarlaStudio app docs:

- [CarlaStudio application overview](../Apps/CarlaStudio/README.md)
- [CarlaStudio UI notes](../Apps/CarlaStudio/src/ui/README.md)
