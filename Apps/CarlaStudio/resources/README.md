# Carla Studio — icon bundle

## Windows (Qt)
`windows/carla-studio.ico` — multi-resolution (16, 24, 32, 48, 64, 128, 256).

In your `.rc` file:
```
IDI_ICON1 ICON "carla-studio.ico"
```
Or set at runtime:
```cpp
app.setWindowIcon(QIcon("carla-studio.ico"));
```

## macOS (Qt)
`macos/carla-studio.icns` — ICNS with 16–1024 pt (incl. @2x retina chunks).

In your `.pro` file:
```
macx {
    ICON = carla-studio.icns
}
```
Or CMake:
```
set_target_properties(MyApp PROPERTIES MACOSX_BUNDLE_ICON_FILE "carla-studio.icns")
```

## Linux (Qt)
`linux/` — hicolor theme PNG set + `.desktop` file + `install.sh`.
Run `bash linux/install.sh` once to install into `~/.local/share/`.

Then in Qt:
```cpp
QGuiApplication::setWindowIcon(QIcon::fromTheme("carla-studio"));
```
