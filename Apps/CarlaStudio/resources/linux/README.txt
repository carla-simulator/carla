Carla Studio — Linux icon set (hicolor theme)

Folder layout:
  icons/hicolor/<size>/apps/carla-studio.png    PNG per size
  carla-studio.desktop                           Desktop entry stub
  install.sh                                 Installs to ~/.local/share

Qt integration:
  In your Qt app, set the window icon with
      QGuiApplication::setWindowIcon(QIcon::fromTheme("carla-studio"));
  after the icons are installed into a standard theme path.

To install system-wide instead, copy 'icons/hicolor' to /usr/share/icons/
and the .desktop file to /usr/share/applications/, then run:
  sudo gtk-update-icon-cache -f -t /usr/share/icons/hicolor
