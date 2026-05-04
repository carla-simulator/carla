// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include <QApplication>
#include <QColor>
#include <QThread>
#include <QDockWidget>
#include <QIcon>
#include <QLibrary>
#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QListWidget>
#include <QTextEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QProgressBar>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QFormLayout>
#include <QTimer>
#include <QProcess>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QScreen>
#include <QShortcut>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QFrame>
#include <QMap>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTextBrowser>
#include <QRegularExpression>
#include <QTextStream>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSet>
#include <QSettings>
#include <chrono>
#include <thread>
#include <QPalette>
#include <QStyleFactory>
#include <QImage>
#include <QPixmap>
#include <QMetaObject>
#include <QEvent>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QFontDatabase>
#include <QTextCursor>
#include <QTime>
#include <QDateTime>
#include <QStorageInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QXmlStreamReader>
#include <QUrlQuery>
#include <QSysInfo>
#include <QMouseEvent>
#include <QClipboard>
#include <QProcessEnvironment>
#include <QToolButton>
#include <QStyledItemDelegate>
#include <QButtonGroup>
#include <QRegion>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QGraphicsLineItem>
#include <QGraphicsEllipseItem>
#include <QSlider>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>
#include <array>
#include <cmath>
#include <iostream>
#include <streambuf>
#include <string>
#include <functional>
#include <tuple>
#include <cstdlib>
#include <QStandardPaths>
#include <QColorDialog>
#include <cstdio>
#include <typeinfo>
#include <exception>
#include <csignal>
#include <cerrno>
#include <cstring>

#if defined(Q_OS_LINUX) || defined(__linux__)
#include <execinfo.h>
#include <fcntl.h>
#include <csignal>
#include <ctime>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <linux/joystick.h>
#include <sys/ioctl.h>
#include <unistd.h>
#define CARLA_STUDIO_HAS_LINUX_JOYSTICK 1
#endif

#include "utils/PlatformProc.h"

#ifdef CARLA_STUDIO_WITH_LIBCARLA
#include <carla/Version.h>
#include <carla/client/Client.h>
#include <carla/client/World.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/ActorList.h>
#include <carla/client/Sensor.h>
#include <carla/client/Vehicle.h>
#include <carla/client/Walker.h>
#include <carla/client/WalkerAIController.h>
#include <carla/client/Map.h>
#include <carla/sensor/data/Image.h>
#include <carla/sensor/data/LidarMeasurement.h>
#include <carla/sensor/data/RadarMeasurement.h>
#include <carla/sensor/data/GnssMeasurement.h>
#include <carla/sensor/data/IMUMeasurement.h>
#include <carla/sensor/data/DVSEventArray.h>
#include <carla/sensor/data/CollisionEvent.h>
#include <carla/sensor/data/LaneInvasionEvent.h>
#include <carla/sensor/data/ObstacleDetectionEvent.h>
#include <carla/client/DebugHelper.h>
#include <carla/geom/Transform.h>
#include <carla/geom/Location.h>
#include <carla/geom/Rotation.h>
#include <carla/geom/BoundingBox.h>
#include <carla/sensor/data/Color.h>
#include <carla/rpc/VehicleControl.h>
#include <carla/client/TimeoutException.h>
#include <carla/client/Waypoint.h>
#include <carla/rpc/WeatherParameters.h>
#include <carla/rpc/OpendriveGenerationParameters.h>
#include <carla/agents/navigation/BehaviorAgent.h>
#endif

#ifdef CARLA_STUDIO_WITH_QT3D
#include "calibration/CalibrationScene.h"
#include <QListWidget>
#endif

#include "core/PlayerSlots.h"
#include "core/SensorMountKey.h"
#include "utils/ResourceFit.h"
#include "integrations/HuggingFace.h"
#include "setup_wizard/SetupWizardDialog.h"
#include "vehicle_import/VehicleImportContainer.h"

#ifdef CARLA_STUDIO_WITH_LIBCARLA
namespace cc = carla::client;
namespace cg = carla::geom;
namespace csd = carla::sensor::data;
#endif

namespace {
volatile std::sig_atomic_t g_shutdownSignalRequested = 0;

void HandleStudioTerminationSignal(int) {
  g_shutdownSignalRequested = 1;
}

enum class StudioTheme { QtDefault, SystemAuto, SystemLight, SystemDark, GlassLight, GlassDark, Nord };

struct ThemeDef {
  StudioTheme id;
  const char *key;
  const char *label;
  bool isDark;
  const char *qss;
};

static const char *const kLightQssStr = R"QSS(
  QWidget {
    background-color: #F5F6F8;
    color: #1E2330;
    font-family: "Segoe UI", "SF Pro Text", "Noto Sans", system-ui, sans-serif;
    font-size: 10pt;
  }
  QMainWindow::separator { background: #E1E5EC; width: 1px; height: 1px; }
  QMenuBar { background: #FFFFFF; border-bottom: 1px solid #E1E5EC; }
  QMenuBar::item { padding: 4px 10px; background: transparent; color: #374053; }
  QMenuBar::item:selected { background: #EDF0F4; border-radius: 4px; }
  QMenu { background: #FFFFFF; border: 1px solid #E1E5EC; border-radius: 6px; padding: 4px; }
  QMenu::item { padding: 4px 18px; border-radius: 3px; }
  QMenu::item:selected { background: #EDF0F4; }
  QGroupBox { background: #FFFFFF; border: 1px solid #E1E5EC; border-radius: 6px; margin-top: 12px; padding: 10px 8px 6px 8px; font-weight: 600; }
  QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; padding: 0 6px; color: #374053; background: transparent; }
  QGroupBox:disabled { background: #F2F3F6; }
  QGroupBox:disabled::title { color: #99A2B3; }
  QTabWidget::pane { border: 1px solid #E1E5EC; border-radius: 6px; background: #FFFFFF; top: -1px; }
  QTabBar::tab { background: #EDF0F4; border: 1px solid #E1E5EC; border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; padding: 5px 14px; margin-right: 2px; color: #374053; min-width: 60px; }
  QTabBar::tab:selected { background: #FFFFFF; color: #1E2330; font-weight: 600; }
  QTabBar::tab:!selected:hover { background: #E4E8ED; }
  QTabBar::tab:disabled { color: #99A2B3; }
  QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QPlainTextEdit, QTextEdit { background: #FFFFFF; border: 1px solid #CAD2DD; border-radius: 4px; padding: 3px 6px; selection-background-color: #2196F3; selection-color: #FFFFFF; }
  QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QPlainTextEdit:focus, QTextEdit:focus { border-color: #2196F3; }
  QComboBox::drop-down { width: 18px; border: none; }
  QPushButton { background: #FFFFFF; border: 1px solid #CAD2DD; border-radius: 4px; padding: 5px 12px; color: #1E2330; }
  QPushButton:hover   { background: #F0F2F5; border-color: #B8C2D1; }
  QPushButton:pressed { background: #E6E9EE; }
  QPushButton:disabled { color: #99A2B3; background: #F2F3F6; border-color: #DDE2E9; }
  QCheckBox, QRadioButton { spacing: 5px; padding: 1px 0; }
  QCheckBox::indicator, QRadioButton::indicator { width: 16px; height: 16px; }
  QCheckBox::indicator { border: 1.5px solid #8794A8; border-radius: 3px; background: #FFFFFF; }
  QCheckBox::indicator:hover    { border-color: #2196F3; }
  QCheckBox::indicator:checked  { background: #2196F3; border-color: #1976D2; }
  QCheckBox::indicator:disabled { border-color: #CFD6E0; background: #F2F3F6; }
  QRadioButton::indicator { border: 1.5px solid #8794A8; border-radius: 8px; background: #FFFFFF; }
  QRadioButton::indicator:hover   { border-color: #2196F3; }
  QRadioButton::indicator:checked { background: qradialgradient(cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5, stop:0 #2196F3, stop:0.55 #2196F3, stop:0.6 #FFFFFF, stop:1 #FFFFFF); border-color: #1976D2; }
  QRadioButton::indicator:disabled { border-color: #CFD6E0; background: #F2F3F6; }
  QTableWidget { background: #FFFFFF; alternate-background-color: #FAFBFC; border: 1px solid #E1E5EC; border-radius: 4px; gridline-color: #EDF0F4; }
  QTableWidget::item { padding: 3px 4px; }
  QTableWidget::item:selected { background: #E3F2FD; color: #1E2330; }
  QHeaderView::section { background: #EDF0F4; padding: 5px 8px; border: none; border-right: 1px solid #E1E5EC; border-bottom: 1px solid #E1E5EC; font-weight: 600; color: #374053; }
  QHeaderView::section:last { border-right: none; }
  QProgressBar { border: 1px solid #CAD2DD; border-radius: 4px; background: #F2F4F7; text-align: center; color: #1E2330; padding: 0; }
  QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #4FC3F7, stop:1 #2196F3); border-radius: 3px; }
  QProgressBar:disabled { color: #99A2B3; background: #F2F3F6; border-color: #DDE2E9; }
  QProgressBar:disabled::chunk { background: #DDE2E9; }
  QListWidget { background: #FFFFFF; border: 1px solid #E1E5EC; border-radius: 4px; padding: 2px; }
  QListWidget::item { padding: 3px 6px; border-radius: 3px; }
  QListWidget::item:selected { background: #E3F2FD; color: #1E2330; }
  QDockWidget::title { background: #EDF0F4; padding: 4px 8px; border: 1px solid #E1E5EC; border-radius: 4px; text-align: left; font-weight: 600; color: #374053; }
  QScrollBar:vertical { background: #F2F4F7; width: 10px; border: none; }
  QScrollBar::handle:vertical { background: #C8D0DC; border-radius: 4px; min-height: 24px; }
  QScrollBar::handle:vertical:hover { background: #B0BAC9; }
  QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
  QScrollBar:horizontal { background: #F2F4F7; height: 10px; border: none; }
  QScrollBar::handle:horizontal { background: #C8D0DC; border-radius: 4px; min-width: 24px; }
  QScrollBar::handle:horizontal:hover { background: #B0BAC9; }
  QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)QSS";

static const char *const kDarkQssStr = R"QSS(
  QWidget { background-color: #23262E; color: #E6E9EE; font-family: "Segoe UI","SF Pro Text","Noto Sans",system-ui,sans-serif; font-size: 10pt; }
  QMainWindow::separator { background: #3A3F4B; width: 1px; height: 1px; }
  QMenuBar { background: #2B2F38; border-bottom: 1px solid #3A3F4B; }
  QMenuBar::item { padding: 4px 10px; background: transparent; color: #CFD6E0; }
  QMenuBar::item:selected { background: #30343E; border-radius: 4px; }
  QMenu { background: #2B2F38; border: 1px solid #3A3F4B; border-radius: 6px; padding: 4px; color: #E6E9EE; }
  QMenu::item { padding: 4px 18px; border-radius: 3px; }
  QMenu::item:selected { background: #37404E; }
  QGroupBox { background: #2B2F38; border: 1px solid #3A3F4B; border-radius: 6px; margin-top: 12px; padding: 10px 8px 6px 8px; font-weight: 600; color: #E6E9EE; }
  QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; padding: 0 6px; color: #9BA3B5; background: transparent; }
  QGroupBox:disabled { background: #262A32; }
  QGroupBox:disabled::title { color: #5C6576; }
  QTabWidget::pane { border: 1px solid #3A3F4B; border-radius: 6px; background: #2B2F38; top: -1px; }
  QTabBar::tab { background: #262A32; border: 1px solid #3A3F4B; border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; padding: 5px 14px; margin-right: 2px; color: #9BA3B5; min-width: 60px; }
  QTabBar::tab:selected { background: #2B2F38; color: #E6E9EE; font-weight: 600; }
  QTabBar::tab:!selected:hover { background: #2F3440; }
  QTabBar::tab:disabled { color: #5C6576; }
  QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QPlainTextEdit, QTextEdit { background: #1F222A; border: 1px solid #3A3F4B; border-radius: 4px; padding: 3px 6px; color: #E6E9EE; selection-background-color: #2196F3; selection-color: #FFFFFF; }
  QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QPlainTextEdit:focus, QTextEdit:focus { border-color: #2196F3; }
  QComboBox::drop-down { width: 18px; border: none; }
  QPushButton { background: #2F3440; border: 1px solid #3A3F4B; border-radius: 4px; padding: 5px 12px; color: #E6E9EE; }
  QPushButton:hover   { background: #37404E; border-color: #4A526A; }
  QPushButton:pressed { background: #262A32; }
  QPushButton:disabled { color: #5C6576; background: #262A32; border-color: #2F3440; }
  QCheckBox, QRadioButton { spacing: 5px; padding: 1px 0; color: #E6E9EE; }
  QCheckBox::indicator, QRadioButton::indicator { width: 16px; height: 16px; }
  QCheckBox::indicator { border: 1.5px solid #6B7487; border-radius: 3px; background: #1F222A; }
  QCheckBox::indicator:hover    { border-color: #2196F3; }
  QCheckBox::indicator:checked  { background: #2196F3; border-color: #64B5F6; }
  QCheckBox::indicator:disabled { border-color: #3A3F4B; background: #262A32; }
  QRadioButton::indicator { border: 1.5px solid #6B7487; border-radius: 8px; background: #1F222A; }
  QRadioButton::indicator:hover   { border-color: #2196F3; }
  QRadioButton::indicator:checked { background: qradialgradient(cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5, stop:0 #64B5F6, stop:0.55 #2196F3, stop:0.6 #1F222A, stop:1 #1F222A); border-color: #64B5F6; }
  QRadioButton::indicator:disabled { border-color: #3A3F4B; background: #262A32; }
  QTableWidget { background: #2B2F38; alternate-background-color: #2F3440; border: 1px solid #3A3F4B; border-radius: 4px; gridline-color: #34394A; color: #E6E9EE; }
  QTableWidget::item { padding: 3px 4px; }
  QTableWidget::item:selected { background: #2F4A66; color: #FFFFFF; }
  QHeaderView::section { background: #262A32; padding: 5px 8px; border: none; border-right: 1px solid #3A3F4B; border-bottom: 1px solid #3A3F4B; font-weight: 600; color: #CFD6E0; }
  QHeaderView::section:last { border-right: none; }
  QProgressBar { border: 1px solid #3A3F4B; border-radius: 4px; background: #1F222A; text-align: center; color: #E6E9EE; padding: 0; }
  QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #4FC3F7, stop:1 #2196F3); border-radius: 3px; }
  QProgressBar:disabled { color: #5C6576; background: #262A32; border-color: #2F3440; }
  QProgressBar:disabled::chunk { background: #3A3F4B; }
  QListWidget { background: #2B2F38; border: 1px solid #3A3F4B; border-radius: 4px; padding: 2px; color: #E6E9EE; }
  QListWidget::item { padding: 3px 6px; border-radius: 3px; }
  QListWidget::item:selected { background: #2F4A66; color: #FFFFFF; }
  QDockWidget::title { background: #262A32; padding: 4px 8px; border: 1px solid #3A3F4B; border-radius: 4px; text-align: left; font-weight: 600; color: #CFD6E0; }
  QScrollBar:vertical { background: #262A32; width: 10px; border: none; }
  QScrollBar::handle:vertical { background: #4A526A; border-radius: 4px; min-height: 24px; }
  QScrollBar::handle:vertical:hover { background: #5C6576; }
  QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
  QScrollBar:horizontal { background: #262A32; height: 10px; border: none; }
  QScrollBar::handle:horizontal { background: #4A526A; border-radius: 4px; min-width: 24px; }
  QScrollBar::handle:horizontal:hover { background: #5C6576; }
  QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)QSS";

static const char *const kGlassLightQssStr = R"QSS(
  QMainWindow {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
      stop:0 #EAF4FF, stop:0.35 #F5F1FE, stop:0.7 #FEEEF6, stop:1 #FFF6E6);
  }
  QWidget {
    background: transparent;
    color: #1C2233;
    font-family: "SF Pro Text", "Segoe UI", "Noto Sans", system-ui, sans-serif;
    font-size: 10pt;
  }
  QMenuBar { background: rgba(255,255,255,0.55); border-bottom: 1px solid rgba(255,255,255,0.7); }
  QMenuBar::item { padding: 5px 12px; background: transparent; color: #2C3347; }
  QMenuBar::item:selected { background: rgba(255,255,255,0.8); border-radius: 6px; }
  QMenu { background: rgba(255,255,255,0.92); border: 1px solid rgba(255,255,255,0.8); border-radius: 10px; padding: 6px; }
  QMenu::item { padding: 5px 20px; border-radius: 5px; color: #1C2233; }
  QMenu::item:selected { background: rgba(33,150,243,0.2); color: #1C2233; }
  QGroupBox {
    background: rgba(255,255,255,0.55);
    border: 1px solid rgba(255,255,255,0.8);
    border-radius: 14px;
    margin-top: 14px;
    padding: 12px 10px 8px 10px;
    font-weight: 600;
  }
  QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 14px; padding: 0 6px; color: #3A4360; background: transparent; }
  QGroupBox:disabled { background: rgba(230,230,230,0.45); }
  QGroupBox:disabled::title { color: rgba(28,34,51,0.4); }
  QTabWidget::pane { border: 1px solid rgba(255,255,255,0.8); border-radius: 12px; background: rgba(255,255,255,0.45); top: -1px; }
  QTabBar::tab {
    background: rgba(255,255,255,0.35);
    border: 1px solid rgba(255,255,255,0.55);
    border-bottom: none;
    border-top-left-radius: 8px;
    border-top-right-radius: 8px;
    padding: 6px 16px;
    margin-right: 3px;
    color: #3A4360;
  }
  QTabBar::tab:selected { background: rgba(255,255,255,0.85); color: #1C2233; font-weight: 600; }
  QTabBar::tab:!selected:hover { background: rgba(255,255,255,0.55); }
  QTabBar::tab:disabled { color: rgba(28,34,51,0.35); }
  QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QPlainTextEdit, QTextEdit {
    background: rgba(255,255,255,0.7);
    border: 1px solid rgba(28,34,51,0.18);
    border-radius: 8px;
    padding: 4px 8px;
    selection-background-color: #2196F3; selection-color: #FFFFFF;
  }
  QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QPlainTextEdit:focus, QTextEdit:focus {
    border: 1.5px solid #2196F3; background: rgba(255,255,255,0.9);
  }
  QComboBox::drop-down { width: 20px; border: none; }
  QPushButton {
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 rgba(255,255,255,0.95), stop:1 rgba(255,255,255,0.6));
    border: 1px solid rgba(28,34,51,0.18);
    border-radius: 9px;
    padding: 6px 14px;
    color: #1C2233;
  }
  QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #FFFFFF, stop:1 rgba(255,255,255,0.85)); }
  QPushButton:pressed { background: rgba(255,255,255,0.5); }
  QPushButton:disabled { color: rgba(28,34,51,0.35); background: rgba(255,255,255,0.3); border-color: rgba(28,34,51,0.12); }
  QCheckBox, QRadioButton { spacing: 6px; padding: 1px 0; }
  QCheckBox::indicator, QRadioButton::indicator { width: 16px; height: 16px; }
  QCheckBox::indicator { border: 1.5px solid rgba(28,34,51,0.35); border-radius: 4px; background: rgba(255,255,255,0.9); }
  QCheckBox::indicator:hover { border-color: #2196F3; }
  QCheckBox::indicator:checked {
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #42A5F5, stop:1 #1E88E5);
    border-color: #1976D2;
  }
  QRadioButton::indicator { border: 1.5px solid rgba(28,34,51,0.35); border-radius: 8px; background: rgba(255,255,255,0.9); }
  QRadioButton::indicator:hover { border-color: #2196F3; }
  QRadioButton::indicator:checked {
    background: qradialgradient(cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5,
      stop:0 #1E88E5, stop:0.55 #1E88E5, stop:0.6 rgba(255,255,255,0.9), stop:1 rgba(255,255,255,0.9));
    border-color: #1976D2;
  }
  QTableWidget {
    background: rgba(255,255,255,0.55);
    alternate-background-color: rgba(255,255,255,0.35);
    border: 1px solid rgba(255,255,255,0.7);
    border-radius: 10px;
    gridline-color: rgba(28,34,51,0.08);
  }
  QTableWidget::item { padding: 4px 6px; }
  QTableWidget::item:selected { background: rgba(33,150,243,0.18); color: #1C2233; }
  QHeaderView::section {
    background: rgba(255,255,255,0.4);
    padding: 6px 10px;
    border: none;
    border-bottom: 1px solid rgba(255,255,255,0.6);
    font-weight: 600;
    color: #3A4360;
  }
  QProgressBar {
    border: 1px solid rgba(28,34,51,0.15);
    border-radius: 8px;
    background: rgba(255,255,255,0.5);
    text-align: center;
    color: #1C2233;
    padding: 0;
  }
  QProgressBar::chunk {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #4FC3F7, stop:0.5 #2196F3, stop:1 #7E57C2);
    border-radius: 6px;
  }
  QProgressBar:disabled { color: rgba(28,34,51,0.4); background: rgba(255,255,255,0.3); }
  QProgressBar:disabled::chunk { background: rgba(28,34,51,0.12); }
  QListWidget { background: rgba(255,255,255,0.55); border: 1px solid rgba(255,255,255,0.7); border-radius: 10px; padding: 2px; }
  QListWidget::item { padding: 4px 8px; border-radius: 6px; }
  QListWidget::item:selected { background: rgba(33,150,243,0.2); color: #1C2233; }
  QDockWidget::title { background: rgba(255,255,255,0.6); padding: 6px 10px; border: 1px solid rgba(255,255,255,0.7); border-radius: 8px; text-align: left; font-weight: 600; color: #3A4360; }
  QScrollBar:vertical { background: transparent; width: 10px; border: none; }
  QScrollBar::handle:vertical { background: rgba(28,34,51,0.22); border-radius: 5px; min-height: 24px; }
  QScrollBar::handle:vertical:hover { background: rgba(28,34,51,0.35); }
  QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
  QScrollBar:horizontal { background: transparent; height: 10px; border: none; }
  QScrollBar::handle:horizontal { background: rgba(28,34,51,0.22); border-radius: 5px; min-width: 24px; }
  QScrollBar::handle:horizontal:hover { background: rgba(28,34,51,0.35); }
  QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)QSS";

static const char *const kGlassDarkQssStr = R"QSS(
  QMainWindow {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:1,
      stop:0 #0F1629, stop:0.35 #1B1240, stop:0.7 #241433, stop:1 #0E2230);
  }
  QWidget {
    background: transparent;
    color: #E8ECF5;
    font-family: "SF Pro Text","Segoe UI","Noto Sans",system-ui,sans-serif;
    font-size: 10pt;
  }
  QMenuBar { background: rgba(255,255,255,0.04); border-bottom: 1px solid rgba(255,255,255,0.08); }
  QMenuBar::item { padding: 5px 12px; background: transparent; color: #C7CEDE; }
  QMenuBar::item:selected { background: rgba(255,255,255,0.08); border-radius: 6px; }
  QMenu { background: rgba(32,36,54,0.94); border: 1px solid rgba(255,255,255,0.1); border-radius: 10px; padding: 6px; color: #E8ECF5; }
  QMenu::item { padding: 5px 20px; border-radius: 5px; }
  QMenu::item:selected { background: rgba(100,181,246,0.22); }
  QGroupBox {
    background: rgba(255,255,255,0.05);
    border: 1px solid rgba(255,255,255,0.10);
    border-radius: 14px;
    margin-top: 14px;
    padding: 12px 10px 8px 10px;
    font-weight: 600;
    color: #E8ECF5;
  }
  QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 14px; padding: 0 6px; color: #A7B0C5; background: transparent; }
  QGroupBox:disabled { background: rgba(255,255,255,0.02); }
  QGroupBox:disabled::title { color: rgba(232,236,245,0.35); }
  QTabWidget::pane { border: 1px solid rgba(255,255,255,0.1); border-radius: 12px; background: rgba(255,255,255,0.04); top: -1px; }
  QTabBar::tab { background: rgba(255,255,255,0.04); border: 1px solid rgba(255,255,255,0.08); border-bottom: none; border-top-left-radius: 8px; border-top-right-radius: 8px; padding: 6px 16px; margin-right: 3px; color: #A7B0C5; }
  QTabBar::tab:selected { background: rgba(255,255,255,0.12); color: #E8ECF5; font-weight: 600; }
  QTabBar::tab:!selected:hover { background: rgba(255,255,255,0.08); }
  QTabBar::tab:disabled { color: rgba(232,236,245,0.3); }
  QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QPlainTextEdit, QTextEdit {
    background: rgba(255,255,255,0.06);
    border: 1px solid rgba(255,255,255,0.12);
    border-radius: 8px;
    padding: 4px 8px;
    color: #E8ECF5;
    selection-background-color: #2196F3; selection-color: #FFFFFF;
  }
  QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QPlainTextEdit:focus, QTextEdit:focus {
    border: 1.5px solid #64B5F6; background: rgba(255,255,255,0.1);
  }
  QPushButton {
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 rgba(255,255,255,0.14), stop:1 rgba(255,255,255,0.06));
    border: 1px solid rgba(255,255,255,0.14);
    border-radius: 9px;
    padding: 6px 14px;
    color: #E8ECF5;
  }
  QPushButton:hover { background: rgba(255,255,255,0.16); }
  QPushButton:pressed { background: rgba(255,255,255,0.04); }
  QPushButton:disabled { color: rgba(232,236,245,0.3); background: rgba(255,255,255,0.03); border-color: rgba(255,255,255,0.06); }
  QCheckBox, QRadioButton { spacing: 6px; padding: 1px 0; color: #E8ECF5; }
  QCheckBox::indicator, QRadioButton::indicator { width: 16px; height: 16px; }
  QCheckBox::indicator { border: 1.5px solid rgba(255,255,255,0.3); border-radius: 4px; background: rgba(255,255,255,0.06); }
  QCheckBox::indicator:hover { border-color: #64B5F6; }
  QCheckBox::indicator:checked { background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #64B5F6, stop:1 #2196F3); border-color: #64B5F6; }
  QRadioButton::indicator { border: 1.5px solid rgba(255,255,255,0.3); border-radius: 8px; background: rgba(255,255,255,0.06); }
  QRadioButton::indicator:hover { border-color: #64B5F6; }
  QRadioButton::indicator:checked { background: qradialgradient(cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5, stop:0 #64B5F6, stop:0.55 #2196F3, stop:0.6 rgba(255,255,255,0.06), stop:1 rgba(255,255,255,0.06)); border-color: #64B5F6; }
  QTableWidget {
    background: rgba(255,255,255,0.04);
    alternate-background-color: rgba(255,255,255,0.06);
    border: 1px solid rgba(255,255,255,0.1);
    border-radius: 10px;
    gridline-color: rgba(255,255,255,0.06);
    color: #E8ECF5;
  }
  QTableWidget::item { padding: 4px 6px; }
  QTableWidget::item:selected { background: rgba(100,181,246,0.22); color: #FFFFFF; }
  QHeaderView::section { background: rgba(255,255,255,0.06); padding: 6px 10px; border: none; border-bottom: 1px solid rgba(255,255,255,0.1); font-weight: 600; color: #C7CEDE; }
  QProgressBar { border: 1px solid rgba(255,255,255,0.12); border-radius: 8px; background: rgba(255,255,255,0.04); text-align: center; color: #E8ECF5; }
  QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #64B5F6, stop:0.5 #2196F3, stop:1 #AB47BC); border-radius: 6px; }
  QProgressBar:disabled { color: rgba(232,236,245,0.3); background: rgba(255,255,255,0.02); }
  QProgressBar:disabled::chunk { background: rgba(255,255,255,0.1); }
  QListWidget { background: rgba(255,255,255,0.05); border: 1px solid rgba(255,255,255,0.1); border-radius: 10px; padding: 2px; color: #E8ECF5; }
  QListWidget::item { padding: 4px 8px; border-radius: 6px; }
  QListWidget::item:selected { background: rgba(100,181,246,0.25); color: #FFFFFF; }
  QDockWidget::title { background: rgba(255,255,255,0.06); padding: 6px 10px; border: 1px solid rgba(255,255,255,0.1); border-radius: 8px; text-align: left; font-weight: 600; color: #C7CEDE; }
  QScrollBar:vertical { background: transparent; width: 10px; border: none; }
  QScrollBar::handle:vertical { background: rgba(255,255,255,0.2); border-radius: 5px; min-height: 24px; }
  QScrollBar::handle:vertical:hover { background: rgba(255,255,255,0.3); }
  QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
  QScrollBar:horizontal { background: transparent; height: 10px; border: none; }
  QScrollBar::handle:horizontal { background: rgba(255,255,255,0.2); border-radius: 5px; min-width: 24px; }
  QScrollBar::handle:horizontal:hover { background: rgba(255,255,255,0.3); }
  QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)QSS";

static const char *const kNordQssStr = R"QSS(
  QWidget { background-color: #2E3440; color: #ECEFF4; font-family: "Segoe UI","SF Pro Text","Noto Sans",system-ui,sans-serif; font-size: 10pt; }
  QMenuBar { background: #2E3440; border-bottom: 1px solid #3B4252; }
  QMenuBar::item { padding: 4px 10px; color: #D8DEE9; }
  QMenuBar::item:selected { background: #434C5E; border-radius: 4px; }
  QMenu { background: #3B4252; border: 1px solid #4C566A; border-radius: 6px; padding: 4px; color: #ECEFF4; }
  QMenu::item { padding: 4px 18px; border-radius: 3px; }
  QMenu::item:selected { background: #4C566A; }
  QGroupBox { background: #3B4252; border: 1px solid #4C566A; border-radius: 6px; margin-top: 12px; padding: 10px 8px 6px 8px; font-weight: 600; color: #ECEFF4; }
  QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 10px; padding: 0 6px; color: #88C0D0; }
  QGroupBox:disabled { background: #333845; }
  QGroupBox:disabled::title { color: #616E88; }
  QTabWidget::pane { border: 1px solid #4C566A; border-radius: 6px; background: #3B4252; top: -1px; }
  QTabBar::tab { background: #2E3440; border: 1px solid #4C566A; border-bottom: none; border-top-left-radius: 4px; border-top-right-radius: 4px; padding: 5px 14px; margin-right: 2px; color: #D8DEE9; min-width: 60px; }
  QTabBar::tab:selected { background: #3B4252; color: #ECEFF4; font-weight: 600; }
  QTabBar::tab:!selected:hover { background: #434C5E; }
  QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QPlainTextEdit, QTextEdit { background: #2E3440; border: 1px solid #4C566A; border-radius: 4px; padding: 3px 6px; color: #ECEFF4; selection-background-color: #88C0D0; selection-color: #2E3440; }
  QLineEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QComboBox:focus, QPlainTextEdit:focus, QTextEdit:focus { border-color: #88C0D0; }
  QComboBox::drop-down { width: 18px; border: none; }
  QPushButton { background: #434C5E; border: 1px solid #4C566A; border-radius: 4px; padding: 5px 12px; color: #ECEFF4; }
  QPushButton:hover { background: #4C566A; border-color: #5E81AC; }
  QPushButton:pressed { background: #3B4252; }
  QPushButton:disabled { color: #616E88; background: #333845; border-color: #3B4252; }
  QCheckBox, QRadioButton { spacing: 5px; padding: 1px 0; color: #ECEFF4; }
  QCheckBox::indicator, QRadioButton::indicator { width: 16px; height: 16px; }
  QCheckBox::indicator { border: 1.5px solid #81A1C1; border-radius: 3px; background: #2E3440; }
  QCheckBox::indicator:hover { border-color: #88C0D0; }
  QCheckBox::indicator:checked { background: #88C0D0; border-color: #8FBCBB; }
  QCheckBox::indicator:disabled { border-color: #4C566A; background: #333845; }
  QRadioButton::indicator { border: 1.5px solid #81A1C1; border-radius: 8px; background: #2E3440; }
  QRadioButton::indicator:hover { border-color: #88C0D0; }
  QRadioButton::indicator:checked { background: qradialgradient(cx:0.5,cy:0.5,radius:0.5,fx:0.5,fy:0.5, stop:0 #88C0D0, stop:0.55 #88C0D0, stop:0.6 #2E3440, stop:1 #2E3440); border-color: #8FBCBB; }
  QTableWidget { background: #3B4252; alternate-background-color: #434C5E; border: 1px solid #4C566A; border-radius: 4px; gridline-color: #434C5E; color: #ECEFF4; }
  QTableWidget::item { padding: 3px 4px; }
  QTableWidget::item:selected { background: #5E81AC; color: #ECEFF4; }
  QHeaderView::section { background: #2E3440; padding: 5px 8px; border: none; border-right: 1px solid #4C566A; border-bottom: 1px solid #4C566A; font-weight: 600; color: #88C0D0; }
  QHeaderView::section:last { border-right: none; }
  QProgressBar { border: 1px solid #4C566A; border-radius: 4px; background: #2E3440; text-align: center; color: #ECEFF4; }
  QProgressBar::chunk { background: qlineargradient(x1:0,y1:0,x2:1,y2:0, stop:0 #8FBCBB, stop:1 #88C0D0); border-radius: 3px; }
  QProgressBar:disabled { color: #616E88; background: #333845; border-color: #3B4252; }
  QProgressBar:disabled::chunk { background: #4C566A; }
  QListWidget { background: #3B4252; border: 1px solid #4C566A; border-radius: 4px; padding: 2px; color: #ECEFF4; }
  QListWidget::item { padding: 3px 6px; border-radius: 3px; }
  QListWidget::item:selected { background: #5E81AC; color: #ECEFF4; }
  QDockWidget::title { background: #2E3440; padding: 4px 8px; border: 1px solid #4C566A; border-radius: 4px; text-align: left; font-weight: 600; color: #88C0D0; }
  QScrollBar:vertical { background: #2E3440; width: 10px; border: none; }
  QScrollBar::handle:vertical { background: #4C566A; border-radius: 4px; min-height: 24px; }
  QScrollBar::handle:vertical:hover { background: #5E81AC; }
  QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
  QScrollBar:horizontal { background: #2E3440; height: 10px; border: none; }
  QScrollBar::handle:horizontal { background: #4C566A; border-radius: 4px; min-width: 24px; }
  QScrollBar::handle:horizontal:hover { background: #5E81AC; }
  QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)QSS";

static const std::array<ThemeDef, 7> kThemeCatalog = {{
  {StudioTheme::QtDefault,   "qt-default",  "Qt Default",    false, ""},
  {StudioTheme::SystemAuto,  "system",      "System (auto)", false, kLightQssStr},
  {StudioTheme::SystemLight, "light",       "Light",        false, kLightQssStr},
  {StudioTheme::SystemDark,  "dark",        "Dark",         true,  kDarkQssStr},
  {StudioTheme::GlassLight,  "glass-light", "Glass · Day",  false, kGlassLightQssStr},
  {StudioTheme::GlassDark,   "glass-dark",  "Glass · Night", true, kGlassDarkQssStr},
  {StudioTheme::Nord,        "nord",        "Nord",         true,  kNordQssStr},
}};

static const ThemeDef &themeById(StudioTheme id) {
  for (const auto &t : kThemeCatalog) if (t.id == id) return t;
  return kThemeCatalog[0];
}

static const ThemeDef &themeByKey(const QString &key) {
  for (const auto &t : kThemeCatalog)
    if (key.compare(QLatin1String(t.key), Qt::CaseInsensitive) == 0) return t;
  return kThemeCatalog[0];
}

static bool osPrefersDark() {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  const auto scheme = QGuiApplication::styleHints()->colorScheme();
  if (scheme == Qt::ColorScheme::Dark)  return true;
  if (scheme == Qt::ColorScheme::Light) return false;
#endif
  return QApplication::palette().color(QPalette::Window).lightness() < 128;
}

static bool g_lastAppliedIsDark = false;
static bool lastAppliedIsDark() { return g_lastAppliedIsDark; }

static void applyStudioTheme(QApplication &app, StudioTheme id) {
  StudioTheme effective = id;
  if (effective == StudioTheme::SystemAuto) {
    effective = osPrefersDark() ? StudioTheme::SystemDark : StudioTheme::SystemLight;
  }
  const ThemeDef &def = themeById(effective);
  g_lastAppliedIsDark = def.isDark;

  QPalette p;
  if (def.isDark) {
    p.setColor(QPalette::Window,          QColor("#23262E"));
    p.setColor(QPalette::WindowText,      QColor("#E6E9EE"));
    p.setColor(QPalette::Base,            QColor("#2B2F38"));
    p.setColor(QPalette::AlternateBase,   QColor("#30343E"));
    p.setColor(QPalette::ToolTipBase,     QColor("#2B2F38"));
    p.setColor(QPalette::ToolTipText,     QColor("#E6E9EE"));
    p.setColor(QPalette::Text,            QColor("#E6E9EE"));
    p.setColor(QPalette::Button,          QColor("#2B2F38"));
    p.setColor(QPalette::ButtonText,      QColor("#E6E9EE"));
    p.setColor(QPalette::BrightText,      QColor("#FF7043"));
    p.setColor(QPalette::Highlight,       QColor("#2196F3"));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Disabled, QPalette::Text,       QColor("#70778A"));
    p.setColor(QPalette::Disabled, QPalette::WindowText, QColor("#70778A"));
    p.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#70778A"));
  } else {
    p.setColor(QPalette::Window,          QColor(250, 250, 250));
    p.setColor(QPalette::WindowText,      QColor(32, 32, 32));
    p.setColor(QPalette::Base,            QColor(255, 255, 255));
    p.setColor(QPalette::AlternateBase,   QColor(245, 245, 245));
    p.setColor(QPalette::ToolTipBase,     QColor(255, 255, 255));
    p.setColor(QPalette::ToolTipText,     QColor(32, 32, 32));
    p.setColor(QPalette::Text,            QColor(32, 32, 32));
    p.setColor(QPalette::Button,          QColor(248, 248, 248));
    p.setColor(QPalette::ButtonText,      QColor(32, 32, 32));
    p.setColor(QPalette::BrightText,      Qt::red);
    p.setColor(QPalette::Highlight,       QColor(33, 150, 243));
    p.setColor(QPalette::HighlightedText, Qt::white);
  }
  app.setPalette(p);
  app.setStyleSheet(QLatin1String(def.qss));
}

static StudioTheme detectInitialTheme() {
  QSettings s;
  const QString key = s.value(QStringLiteral("view/theme")).toString();
  if (!key.isEmpty()) return themeByKey(key).id;
  return StudioTheme::QtDefault;
}

class PaletteChangeWatcher : public QObject {
 public:
  using Cb = std::function<void()>;
  PaletteChangeWatcher(Cb cb, QObject *parent = nullptr)
    : QObject(parent), cb_(std::move(cb)) {}
 protected:
  bool eventFilter(QObject *obj, QEvent *ev) override {
    if (ev->type() == QEvent::ApplicationPaletteChange && cb_) cb_();
    return QObject::eventFilter(obj, ev);
  }
 private:
  Cb cb_;
};

class CerrTap : public std::streambuf {
 public:
  using OnLine = std::function<void(const std::string &)>;
  CerrTap(std::streambuf *passthrough, OnLine cb)
    : passthrough_(passthrough), onLine_(std::move(cb)) {}

 protected:
  int_type overflow(int_type c) override {
    if (c == traits_type::eof()) return traits_type::not_eof(c);
    if (passthrough_) passthrough_->sputc(static_cast<char>(c));
    if (c == '\n') {
      if (onLine_ && !buffer_.empty()) onLine_(buffer_);
      buffer_.clear();
    } else {
      buffer_.push_back(static_cast<char>(c));
    }
    return c;
  }
  std::streamsize xsputn(const char *s, std::streamsize n) override {
    if (passthrough_) passthrough_->sputn(s, n);
    for (std::streamsize i = 0; i < n; ++i) {
      if (s[i] == '\n') {
        if (onLine_ && !buffer_.empty()) onLine_(buffer_);
        buffer_.clear();
      } else {
        buffer_.push_back(s[i]);
      }
    }
    return n;
  }

 private:
  std::streambuf *passthrough_;
  OnLine onLine_;
  std::string buffer_;
};
}

class KeyboardInputFilter : public QObject {
public:
  explicit KeyboardInputFilter(std::function<void(int, bool)> callback, QObject *parent = nullptr)
      : QObject(parent), callback_(std::move(callback)) {}

protected:
  bool eventFilter(QObject *obj, QEvent *event) override {
    Q_UNUSED(obj);
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
      auto *keyEvent = static_cast<QKeyEvent *>(event);
      if (!keyEvent->isAutoRepeat() && callback_) {
        callback_(keyEvent->key(), event->type() == QEvent::KeyPress);
      }
    }
    return QObject::eventFilter(obj, event);
  }

private:
  std::function<void(int, bool)> callback_;
};

static void carla_studio_signal_trace(int sig) {
  void *frames[64];
  const int n = backtrace(frames, 64);
  const int fd = open("/tmp/carla_studio_crash.log",
                      O_WRONLY | O_CREAT | O_APPEND, 0644);
  if (fd >= 0) {
    char hdr[256];
    const int len = std::snprintf(hdr, sizeof(hdr),
        "\n=== carla-studio fatal signal %d (%s) at %ld ===\n",
        sig, strsignal(sig), (long)time(nullptr));
    if (len > 0) (void)!write(fd, hdr, std::min<size_t>(len, sizeof(hdr)));
    backtrace_symbols_fd(frames, n, fd);
    close(fd);
  }
  std::signal(sig, SIG_DFL);
  std::raise(sig);
}

int main(int argc, char *argv[]) {
  // Capture a stack trace on hard signals (SIGSEGV / SIGBUS / SIGABRT) before
  // the crash takes the controlling terminal down with us. Trace lands in
  // /tmp/carla_studio_crash.log — paste it back when you hit a Drive crash.
  for (int s : {SIGSEGV, SIGBUS, SIGABRT, SIGFPE, SIGILL}) {
    std::signal(s, carla_studio_signal_trace);
  }

  std::set_terminate([]() {
    std::fprintf(stderr,
      "\n[carla-studio] FATAL: an uncaught C++ exception escaped the\n"
      "             event loop and would have called std::abort().\n");
    if (auto eptr = std::current_exception()) {
      try {
        std::rethrow_exception(eptr);
      } catch (const std::exception &e) {
        std::fprintf(stderr,
          "             type: %s\n"
          "             what: %s\n",
          typeid(e).name(), e.what());
      } catch (...) {
        std::fprintf(stderr, "             type: <non-std exception>\n");
      }
    }
    std::fprintf(stderr,
      "             Most common cause: CARLA simulator API version\n"
      "             mismatch with the libcarla-client this build was\n"
      "             linked against. Check Help → Health Check for the\n"
      "             SDK ↔ Sim version row, then either:\n"
      "               - Install the matching CARLA via Cfg → Install /\n"
      "                 Update CARLA, or\n"
      "               - Rebuild CARLA Studio against the API version\n"
      "                 your simulator reports.\n");
    std::_Exit(1);
  });

  QApplication app(argc, argv);
  app.setApplicationName("CARLA Studio");
  app.setApplicationDisplayName("CARLA Studio");
  app.setOrganizationName("CARLA Simulator");

  std::vector<QWidget *> simRequiredButtons;
  auto setSimReachable = [&simRequiredButtons](bool reachable) {
    for (auto *b : simRequiredButtons) {
      if (b) b->setEnabled(reachable);
    }
  };

  QIcon studioIcon;
  for (const int sz : {16, 32, 48, 64, 128, 256, 512}) {
    studioIcon.addFile(QString(":/icons/carla-studio-%1.png").arg(sz), QSize(sz, sz));
  }
  if (!studioIcon.isNull()) {
    app.setWindowIcon(studioIcon);
  }

  std::signal(SIGINT, HandleStudioTerminationSignal);
  std::signal(SIGTERM, HandleStudioTerminationSignal);
#ifdef SIGHUP
  std::signal(SIGHUP, HandleStudioTerminationSignal);
#endif
  app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

  QCoreApplication::setOrganizationName("CARLA Simulator");
  QCoreApplication::setOrganizationDomain("carla.org");
  QCoreApplication::setApplicationName("CARLA Studio");

  StudioTheme currentTheme = detectInitialTheme();
  applyStudioTheme(app, currentTheme);
  bool darkMode = lastAppliedIsDark();

  auto reapplyCurrentTheme = [&]() {
    if (currentTheme == StudioTheme::SystemAuto) {
      applyStudioTheme(app, currentTheme);
      darkMode = lastAppliedIsDark();
    }
  };
  app.installEventFilter(new PaletteChangeWatcher(reapplyCurrentTheme, &app));
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
  QObject::connect(QGuiApplication::styleHints(),
                   &QStyleHints::colorSchemeChanged,
                   &app, [reapplyCurrentTheme](Qt::ColorScheme) {
                     reapplyCurrentTheme();
                   });
#endif

  QMainWindow window;
  window.setWindowTitle("CARLA Studio");

  const int phoneWidthTarget  = 520;
  const int phoneHeightTarget = 960;
  QRect availableGeometry(0, 0, phoneWidthTarget, phoneHeightTarget);
  if (QScreen *screen = app.primaryScreen()) {
    availableGeometry = screen->availableGeometry();
  }
  const int fixedWidth  = std::min(phoneWidthTarget,  availableGeometry.width());
  const int fixedHeight = std::min(phoneHeightTarget, availableGeometry.height());
  const int startX = availableGeometry.x() + std::max(0, (availableGeometry.width()  - fixedWidth)  / 2);
  const int startY = availableGeometry.y() + std::max(0, (availableGeometry.height() - fixedHeight) / 2);
  window.setGeometry(startX, startY, fixedWidth, fixedHeight);
  window.setFixedSize(fixedWidth, fixedHeight);

  QTabWidget *tabs = new QTabWidget();

  QWidget *healthCheckPage = new QWidget();
  QVBoxLayout *healthCheckPageOuterLayout = nullptr;

  QWidget *w1 = new QWidget();
  QGridLayout *l1 = new QGridLayout();
  
  QGroupBox *launchGroup = new QGroupBox("Scenario");
  QVBoxLayout *launchLayout = new QVBoxLayout();
  
  
  
  
  
  
  
  
  
  
  auto discoverCarlaRoot = []() -> QString {
    const QString home = QDir::homePath();
    QStringList candidates;
    for (const QString &dir : { home + "/carla", QString("/opt/carla"), QString("/simulators/carla") }) {
      QDir d(dir);
      if (!d.exists()) continue;
      candidates << d.absolutePath();
      for (const QString &sub : d.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed)) {
        const QString subPath = d.absoluteFilePath(sub);
        candidates << subPath;
        QDir ss(subPath);
        for (const QString &deep : ss.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name | QDir::Reversed)) {
          candidates << ss.absoluteFilePath(deep);
        }
      }
    }
    QString bestUnreal, bestUE5, bestUE4;
    for (const QString &c : candidates) {
      if (QFileInfo(c + "/CarlaUnreal.sh").isFile() && bestUnreal.isEmpty()) bestUnreal = c;
      if (QFileInfo(c + "/CarlaUE5.sh").isFile()    && bestUE5.isEmpty())    bestUE5    = c;
      if (QFileInfo(c + "/CarlaUE4.sh").isFile()    && bestUE4.isEmpty())    bestUE4    = c;
    }
    if (!bestUnreal.isEmpty()) return bestUnreal;
    if (!bestUE5.isEmpty())    return bestUE5;
    if (!bestUE4.isEmpty())    return bestUE4;
    return QString();
  };

  QString root_str = QString::fromLocal8Bit(qgetenv("CARLA_ROOT"));
  {
    const bool envHasUnreal = !root_str.isEmpty()
                              && QFileInfo(root_str + "/CarlaUnreal.sh").isFile();
    if (!envHasUnreal) {
      const QString discovered = discoverCarlaRoot();
      const bool discoveredHasUnreal = !discovered.isEmpty()
                                       && QFileInfo(discovered + "/CarlaUnreal.sh").isFile();
      const bool envValid = !root_str.isEmpty() &&
        (QFileInfo(root_str + "/CarlaUE4.sh").isFile() ||
         QFileInfo(root_str + "/CarlaUE5.sh").isFile() ||
         QFileInfo(root_str + "/CarlaUnreal.sh").isFile() ||
         QFileInfo(root_str + "/Unreal/CarlaUnreal/Source").isDir() ||
         QFileInfo(root_str + "/Unreal/CarlaUE4/Source").isDir() ||
         QFileInfo(root_str + "/Unreal/CarlaUE5/Source").isDir());
      if (!envValid && (discoveredHasUnreal || !discovered.isEmpty())) {
        root_str = discovered;
      }
    }
  }

  QLineEdit *carlaRootPath = new QLineEdit();
  carlaRootPath->setPlaceholderText(
    "Enter $CARLA_ROOT path or 👆 bottom right of this page");
  carlaRootPath->setToolTip(
    "Path to your CARLA install (the directory that holds CarlaUE4.sh, "
    "CarlaUE5.sh, or CarlaUnreal.sh). Use Cfg → Install / Update CARLA "
    "to fetch one.");
  {
    QPalette pal = carlaRootPath->palette();
    pal.setColor(QPalette::PlaceholderText, QColor(0, 0, 0, 153));
    carlaRootPath->setPalette(pal);
  }
  carlaRootPath->setText(root_str);

  QFormLayout *launchForm = new QFormLayout();
  launchForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
  launchForm->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
  launchForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  launchForm->setHorizontalSpacing(8);
  launchForm->setVerticalSpacing(6);

  QLabel *rootPathLabel = new QLabel("CARLA root:");
  launchForm->addRow(rootPathLabel, carlaRootPath);

  QWidget *scenarioField = new QWidget();
  QHBoxLayout *scenarioLayout = new QHBoxLayout(scenarioField);
  scenarioLayout->setContentsMargins(0, 0, 0, 0);
  scenarioLayout->setSpacing(4);
  QComboBox *scenarioSelect = new QComboBox();
  scenarioSelect->setToolTip(
    "Map loaded into the simulator at start. Refresh (⟳) repopulates "
    "from the running sim; + Maps installs additional packs.");
  
  
  
  
  
  
  
  
  
  
  
  
  
  scenarioSelect->addItems(QStringList() << "Town10HD_Opt");
  scenarioSelect->setCurrentIndex(0);
  scenarioSelect->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  scenarioLayout->addWidget(scenarioSelect, 1);
  QPushButton *refreshScenarioBtn = new QPushButton("⟳");
  refreshScenarioBtn->setToolTip(
    "Re-query the running sim for available maps. No-op when CARLA "
    "isn't reachable.");
  refreshScenarioBtn->setFixedWidth(34);
  QPushButton *loadMapsBtn = new QPushButton("+ Maps");
  loadMapsBtn->setToolTip(
    "Browse CARLA releases and install the matching AdditionalMaps pack "
    "into your CARLA_ROOT (or follow the source-build ingestion guide "
    "if this is a source install).");
  loadMapsBtn->setFixedHeight(refreshScenarioBtn->sizeHint().height());
  scenarioLayout->addWidget(loadMapsBtn);
  scenarioLayout->addWidget(refreshScenarioBtn);
  launchForm->addRow(new QLabel("Map:"), scenarioField);

  QAction *optRenderOffscreen = new QAction("Off-screen (-RenderOffScreen)", &window);
  optRenderOffscreen->setCheckable(true);
  
  
  QAction *optWindowSmall = new QAction("Windowed 640×480 (default)", &window);
  optWindowSmall->setCheckable(true);
  QAction *optQualityEpic = new QAction("Quality Epic (-quality-level=Epic)", &window);
  optQualityEpic->setCheckable(true);
  QAction *optQualityLow = new QAction("Quality Low (-quality-level=Low)", &window);
  optQualityLow->setCheckable(true);
  QAction *optNoSound = new QAction("No Sound (-nosound)", &window);
  optNoSound->setCheckable(true);
  QAction *optRos2 = new QAction("ROS Native (--ros2)", &window);
  optRos2->setCheckable(true);
  QAction *optPreferNvidia = new QAction("Prefer NVIDIA (-prefernvidia)", &window);
  optPreferNvidia->setCheckable(true);
  
  
  
  
  
  
  optPreferNvidia->setChecked(true);
  {
    const bool savedExplicit = QSettings().contains("render/prefer_nvidia");
    if (savedExplicit) {
      optPreferNvidia->setChecked(QSettings().value("render/prefer_nvidia").toBool());
    }
    QProcess nv;
    nv.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    nv.start("/usr/bin/nvidia-smi", QStringList() << "-L");
    const bool ok = nv.waitForFinished(1500)
                    && nv.exitStatus() == QProcess::NormalExit
                    && nv.exitCode() == 0;
    const QString out = ok
        ? QString::fromLocal8Bit(nv.readAllStandardOutput()).trimmed()
        : QString();
    if (!out.isEmpty()) {
      optPreferNvidia->setToolTip(
          QString("Auto-enabled — NVIDIA GPU detected:\n%1\n\nToggle off if "
                  "you want CARLA to pick its own renderer (llvmpipe fallback).")
              .arg(out));
    } else {
      optPreferNvidia->setToolTip(
          "Default-on. Forces CARLA to use the NVIDIA Vulkan driver instead "
          "of the llvmpipe software fallback (which produces ~0 % GPU load "
          "and an unusably slow simulator). Toggle off only if you don't "
          "have an NVIDIA card.");
    }
  }
  QObject::connect(optPreferNvidia, &QAction::toggled, &window, [](bool on) {
    QSettings().setValue("render/prefer_nvidia", on);
  });
  QAction *optDriverInApp = new QAction("In-app LibCarla driver (default)", &window);
  optDriverInApp->setCheckable(true);
  QAction *optDriverPython = new QAction("Python manual_control.py (optional)", &window);
  optDriverPython->setCheckable(true);
#ifdef CARLA_STUDIO_WITH_LIBCARLA
  optDriverInApp->setChecked(true);
  optDriverPython->setChecked(false);
#else
  optDriverInApp->setChecked(false);
  optDriverInApp->setEnabled(false);
  optDriverInApp->setText("In-app LibCarla driver (unavailable: carla-client not linked)");
  optDriverPython->setChecked(false);
#endif

  QComboBox *driverViewMode = new QComboBox();
  driverViewMode->addItems(QStringList()
    << "First Person" << "Third Person" << "Cockpit" << "Bird Eye");
  driverViewMode->setCurrentText("Third Person");
  driverViewMode->setVisible(false);

  QPushButton *fpvBtn = new QPushButton("Driver");
  QPushButton *tpvBtn = new QPushButton("Chase");
  QPushButton *cpvBtn = new QPushButton("Cockpit");
  QPushButton *bevBtn = new QPushButton("Bird Eye View");
  for (QPushButton *b : {fpvBtn, tpvBtn, cpvBtn, bevBtn}) {
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    b->setMinimumWidth(0);
  }
  fpvBtn->setToolTip(
    "<b>Driver view</b> (a.k.a. FPV / first-person)<br>"
    "Camera at the driver's eye position, looking forward through the "
    "windshield. The natural viewpoint for evaluating perception models "
    "the way a human would see the road. Click to open a live RGB camera "
    "viewer at this perspective.");
  tpvBtn->setToolTip(
    "<b>Chase view</b> (a.k.a. TPV / third-person)<br>"
    "Camera follows the vehicle from behind and slightly above — same "
    "framing most driving games default to. Useful when you want to see "
    "the ego car in context with traffic and lane geometry.");
  cpvBtn->setToolTip(
    "<b>Cockpit view</b> (a.k.a. CPV)<br>"
    "Interior framing — driver display, infotainment, windshield, and "
    "partial A-pillars visible. Closest to what a human sees when "
    "driving; narrower effective FOV than Driver view.");
  bevBtn->setToolTip(
    "<b>BEV — Bird's-Eye View</b><br>"
    "Standard automotive industry term for the top-down camera. Camera "
    "straight overhead looking down. Used for path planning review, "
    "occupancy-grid visualisation, BEV perception models, and screenshots "
    "that show the intersection geometry around the ego vehicle.");
  fpvBtn->setCheckable(true);
  tpvBtn->setCheckable(true);
  cpvBtn->setCheckable(true);
  bevBtn->setCheckable(true);
  simRequiredButtons.push_back(fpvBtn);
  simRequiredButtons.push_back(tpvBtn);
  simRequiredButtons.push_back(cpvBtn);
  simRequiredButtons.push_back(bevBtn);
  for (QPushButton *b : {fpvBtn, tpvBtn, cpvBtn, bevBtn}) b->setEnabled(false);
  
  
  auto *viewBtnGroup = new QButtonGroup(&window);
  viewBtnGroup->setExclusive(true);
  viewBtnGroup->addButton(fpvBtn);
  viewBtnGroup->addButton(tpvBtn);
  viewBtnGroup->addButton(cpvBtn);
  viewBtnGroup->addButton(bevBtn);

  QHBoxLayout *viewBtnRow = new QHBoxLayout();
  viewBtnRow->setContentsMargins(0, 0, 0, 0);
  viewBtnRow->setSpacing(4);
  viewBtnRow->addWidget(fpvBtn, 1);
  viewBtnRow->addWidget(tpvBtn, 1);
  viewBtnRow->addWidget(cpvBtn, 1);
  viewBtnRow->addWidget(bevBtn, 1);
  launchForm->addRow(new QLabel("View:"), viewBtnRow);

  QComboBox *runtimeTarget = new QComboBox();
  runtimeTarget->addItem("localhost");
  runtimeTarget->addItem("container");
  runtimeTarget->addItem("remotehost - Please configure in settings");
  runtimeTarget->setToolTip(
    "<b>Runtime — where CARLA runs</b><br>"
    "<b>Local</b>: this machine.<br>"
    "<b>Docker</b>: a local container running the CARLA simulator.<br>"
    "<b>Remote</b>: SSH to a host on your subnet (configure under "
    "<i>Cfg → Remote / SSH Settings</i>; remote hosts then appear in "
    "this list as <i>&lt;hostname&gt; (remote)</i>).<br>"
    "<b>Cloud (Brev)</b>: NVIDIA-managed cloud GPU instance "
    "(Cfg → Compute (HPC) Settings — coming next pass)."
    "<br><br>The address row below adapts to your selection: hostname "
    "for Remote, container name for Docker, hidden for Local.");
  QHBoxLayout *hostPortRow = new QHBoxLayout();
  hostPortRow->setContentsMargins(0, 0, 0, 0);
  hostPortRow->setSpacing(8);
  hostPortRow->addWidget(runtimeTarget, 1);
  QWidget *hostPortHost = new QWidget();
  hostPortHost->setLayout(hostPortRow);
  launchForm->addRow(new QLabel("Host:"), hostPortHost);

  QLabel    *addrLabel = new QLabel("Target (container/machine):");
  QLineEdit *targetHost = new QLineEdit("localhost");
  targetHost->setToolTip(
    "Address of the runtime — interpretation depends on the Runtime "
    "selection above.");
  QWidget *addrHost = new QWidget();
  QHBoxLayout *addrHostLay = new QHBoxLayout(addrHost);
  addrHostLay->setContentsMargins(0, 0, 0, 0);
  addrHostLay->setSpacing(6);
  addrHostLay->addWidget(targetHost, 1);
  launchForm->addRow(addrLabel, addrHost);

  auto syncAddressRow = [addrLabel, addrHost, targetHost, runtimeTarget]() {
    const QString r = runtimeTarget->currentText();
    auto setLabel = [addrLabel](const QString &s) {
      addrLabel->setText(s);
    };
    auto suggestIfDefault = [targetHost](const QStringList &knownDefaults,
                                          const QString &nextDefault) {
      if (knownDefaults.contains(targetHost->text().trimmed())) {
        targetHost->setText(nextDefault);
      }
    };
    static const QStringList kKnownDefaults = {
      "localhost", "carla-server", "carla", ""};
    if (r == "localhost" || r == "Local") {
      addrLabel->setVisible(false);
      addrHost->setVisible(false);
      suggestIfDefault(kKnownDefaults, "localhost");
    } else if (r == "container" || r == "Docker") {
      addrLabel->setVisible(true);
      addrHost->setVisible(true);
      setLabel("Container name:");
      targetHost->setPlaceholderText("e.g. carla-server");
      suggestIfDefault(kKnownDefaults, "carla-server");
    } else if (r.endsWith("(remote)")) {
      addrLabel->setVisible(true);
      addrHost->setVisible(true);
      setLabel("Host:");
      const QString hostFromCombo = r.left(r.size() - QString(" (remote)").size());
      targetHost->setPlaceholderText(hostFromCombo);
      if (kKnownDefaults.contains(targetHost->text().trimmed()) ||
          targetHost->text().trimmed().isEmpty()) {
        targetHost->setText(hostFromCombo);
      }
    } else if (r.startsWith("remotehost") || r.startsWith("Remote")) {
      addrLabel->setVisible(true);
      addrHost->setVisible(true);
      setLabel("Host:");
      targetHost->setPlaceholderText("Configure under Cfg → Remote / SSH Settings");
    } else {
      addrLabel->setVisible(true);
      addrHost->setVisible(true);
      setLabel("Address:");
      targetHost->setPlaceholderText("");
    }
  };
  QObject::connect(runtimeTarget, &QComboBox::currentTextChanged,
    &window, [syncAddressRow](const QString &) { syncAddressRow(); });
  syncAddressRow();

  QComboBox *weatherCombo = new QComboBox();
  weatherCombo->setToolTip(
    "Sun + cloud + rain preset. Applied at sim start; <i>Default</i> "
    "leaves the server's weather untouched.");
  static const std::array<std::pair<const char *, const char *>, 21>
    kWeatherPresets = {{
      {"Default",                        "Default"},
      {"☀ Clear · Noon",                 "ClearNoon"},
      {"☁ Cloudy · Noon",                "CloudyNoon"},
      {"☂ Wet · Noon",                   "WetNoon"},
      {"☂ Wet & Cloudy · Noon",          "WetCloudyNoon"},
      {"🌦 Mid Rain · Noon",             "MidRainyNoon"},
      {"🌧 Hard Rain · Noon",            "HardRainNoon"},
      {"🌦 Soft Rain · Noon",            "SoftRainNoon"},
      {"☀ Clear · Sunset",               "ClearSunset"},
      {"☁ Cloudy · Sunset",              "CloudySunset"},
      {"☂ Wet · Sunset",                 "WetSunset"},
      {"☂ Wet & Cloudy · Sunset",        "WetCloudySunset"},
      {"🌦 Mid Rain · Sunset",           "MidRainSunset"},
      {"🌧 Hard Rain · Sunset",          "HardRainSunset"},
      {"🌦 Soft Rain · Sunset",          "SoftRainSunset"},
      {"☾ Clear · Night",               "ClearNight"},
      {"☾ Cloudy · Night",              "CloudyNight"},
      {"☾ Wet · Night",                 "WetNight"},
      {"☾ Wet & Cloudy · Night",        "WetCloudyNight"},
      {"☾ Soft Rain · Night",           "SoftRainNight"},
      {"☾ Mid Rain · Night",            "MidRainyNight"},
    }};
  for (const auto &[pretty, canonical] : kWeatherPresets) {
    weatherCombo->addItem(QString::fromUtf8(pretty), QString::fromLatin1(canonical));
  }

  class WeatherDelegate : public QStyledItemDelegate {
   public:
    using QStyledItemDelegate::QStyledItemDelegate;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override {
      QStyleOptionViewItem opt = option;
      initStyleOption(&opt, index);
      opt.text.clear();
      QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
      style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

      const QString full = index.data(Qt::DisplayRole).toString();
      const int dotIdx = full.indexOf(QStringLiteral(" · "));
      const QColor textColor = (opt.state & QStyle::State_Selected)
                                  ? opt.palette.color(QPalette::HighlightedText)
                                  : opt.palette.color(QPalette::Text);
      painter->save();
      painter->setPen(textColor);
      const QRect inner = opt.rect.adjusted(8, 0, -8, 0);

      if (dotIdx <= 0) {
        painter->drawText(inner, Qt::AlignLeft | Qt::AlignVCenter, full);
        painter->restore();
        return;
      }

      const QString time = full.mid(dotIdx + 3);
      const QString head = full.left(dotIdx).trimmed();
      QString icon, weather;
      const int sp = head.indexOf(' ');
      if (sp > 0) { icon = head.left(sp); weather = head.mid(sp + 1).trimmed(); }
      else        { weather = head; }

      const int iconCol = 22;
      const int timeCol = 64;
      QRect iconR(inner.left(),                inner.top(), iconCol,                 inner.height());
      QRect midR (inner.left() + iconCol,      inner.top(),
                   inner.width() - iconCol - timeCol,                                inner.height());
      QRect timeR(inner.right() - timeCol + 1, inner.top(), timeCol,                 inner.height());

      painter->drawText(iconR, Qt::AlignLeft   | Qt::AlignVCenter, icon);
      painter->drawText(midR,  Qt::AlignCenter | Qt::AlignVCenter, weather);
      painter->drawText(timeR, Qt::AlignRight  | Qt::AlignVCenter, time);
      painter->restore();
    }
    QSize sizeHint(const QStyleOptionViewItem &option,
                    const QModelIndex &index) const override {
      QSize s = QStyledItemDelegate::sizeHint(option, index);
      s.setHeight(std::max(s.height(), 24));
      return s;
    }
  };
  if (auto *popup = weatherCombo->view()) {
    popup->setItemDelegate(new WeatherDelegate(popup));
  }
  weatherCombo->view()->setMinimumWidth(280);

  QSpinBox *cfgVehicles    = new QSpinBox();
  cfgVehicles->setRange(0, 200);
  cfgVehicles->setValue(30);
  QSpinBox *cfgPedestrians = new QSpinBox();
  cfgPedestrians->setRange(0, 500);
  cfgPedestrians->setValue(50);
  QSpinBox *cfgSeed        = new QSpinBox();
  cfgSeed->setRange(0, 1000000);
  cfgSeed->setValue(42);

  launchLayout->addLayout(launchForm);

  QGroupBox *integrationsGroup = new QGroupBox("Co-Simulation");
  QVBoxLayout *integrationsLayout = new QVBoxLayout();

  QFormLayout *tpForm = new QFormLayout();
  tpForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
  tpForm->setFormAlignment(Qt::AlignTop | Qt::AlignLeft);
  tpForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
  tpForm->setHorizontalSpacing(8);
  tpForm->setVerticalSpacing(6);

  QSettings tpSettings;

  QHBoxLayout *terasimRow = new QHBoxLayout();
  terasimRow->setContentsMargins(0, 0, 0, 0);
  QCheckBox *terasimEnable = new QCheckBox("Enable");
  terasimEnable->setChecked(tpSettings.value("addon/terasim_enabled", false).toBool());
  terasimEnable->setToolTip(
    "Run the TeraSim co-simulation alongside CARLA (SUMO traffic, "
    "scenario library: default_sumo, construction, safetest_nade, "
    "pedestrian, cyclist, multi_cav). Requires Redis on the chosen "
    "compute host.");
  terasimRow->addWidget(terasimEnable);
  QComboBox *terasimScenario = new QComboBox();
  terasimScenario->addItems(QStringList()
    << "default_sumo" << "construction" << "safetest_nade"
    << "pedestrian"   << "cyclist"      << "multi_cav");
  terasimScenario->setCurrentText(
    tpSettings.value("addon/terasim_scenario", "default_sumo").toString());
  terasimScenario->setEnabled(terasimEnable->isChecked());
  terasimRow->addWidget(terasimScenario, 1);
  QPushButton *terasimCfg = new QPushButton("⚙");
  terasimCfg->setFixedWidth(34);
  terasimCfg->setToolTip("TeraSim settings (Redis host, vehicle blueprint, "
                          "install).");
  terasimRow->addWidget(terasimCfg);
  QWidget *terasimHost = new QWidget();
  terasimHost->setLayout(terasimRow);
  tpForm->addRow(new QLabel("TeraSim:"), terasimHost);

  QHBoxLayout *autowareRow = new QHBoxLayout();
  autowareRow->setContentsMargins(0, 0, 0, 0);
  QCheckBox *autowareEnable = new QCheckBox("Enable");
  autowareEnable->setChecked(tpSettings.value("addon/autoware_enabled", false).toBool());
  autowareEnable->setToolTip(
    "Run the Autoware planning + control stack against the CARLA world "
    "via the ROS2 bridge.");
  autowareRow->addWidget(autowareEnable);
  QComboBox *autowareStack = new QComboBox();
  autowareStack->addItems(QStringList()
    << "planning_simulator"
    << "real_car_stack"
    << "preview_control");
  autowareStack->setCurrentText(
    tpSettings.value("addon/autoware_stack", "planning_simulator").toString());
  autowareStack->setEnabled(autowareEnable->isChecked());
  autowareRow->addWidget(autowareStack, 1);
  QPushButton *autowareCfg = new QPushButton("⚙");
  autowareCfg->setFixedWidth(34);
  autowareCfg->setToolTip("Autoware settings (workspace path, ROS distro, "
                            "vehicle/sensor model, prebuilt-vs-source).");
  autowareRow->addWidget(autowareCfg);
  QWidget *autowareHost = new QWidget();
  autowareHost->setLayout(autowareRow);
  tpForm->addRow(new QLabel("Autoware:"), autowareHost);

  QLabel *integNotConfigured = new QLabel("Not Configured");
  integNotConfigured->setAlignment(Qt::AlignCenter);
  integNotConfigured->setStyleSheet(
    "QLabel { color: #888; font-style: italic; padding: 12px; }");
  integNotConfigured->setToolTip(
    "Install TeraSim or Autoware via <i>Cfg → Third-Party Tools…</i> to "
    "enable these rows.");
  integrationsLayout->addStretch(1);
  integrationsLayout->addWidget(integNotConfigured);
  integrationsLayout->addLayout(tpForm);
  integrationsLayout->addStretch(1);
  integrationsGroup->setLayout(integrationsLayout);

  auto refreshIntegrationsVisibility = [&]() {
    QSettings s;
    const bool tsOn = s.value("addon/terasim_installed",  false).toBool();
    const bool awOn = s.value("addon/autoware_installed", false).toBool();
    terasimHost->setVisible(tsOn);
    autowareHost->setVisible(awOn);
    for (int row = 0; row < tpForm->rowCount(); ++row) {
      QLayoutItem *fld = tpForm->itemAt(row, QFormLayout::FieldRole);
      QLayoutItem *lbl = tpForm->itemAt(row, QFormLayout::LabelRole);
      if (fld && fld->widget() && lbl && lbl->widget()) {
        lbl->widget()->setVisible(fld->widget()->isVisible());
      }
    }
    integNotConfigured->setVisible(!tsOn && !awOn);
  };
  refreshIntegrationsVisibility();

  QObject::connect(terasimEnable, &QCheckBox::toggled, &window,
    [terasimScenario](bool on) {
      QSettings().setValue("addon/terasim_enabled", on);
      terasimScenario->setEnabled(on);
    });
  QObject::connect(terasimScenario, &QComboBox::currentTextChanged, &window,
    [](const QString &v) { QSettings().setValue("addon/terasim_scenario", v); });
  QObject::connect(autowareEnable, &QCheckBox::toggled, &window,
    [autowareStack](bool on) {
      QSettings().setValue("addon/autoware_enabled", on);
      autowareStack->setEnabled(on);
    });
  QObject::connect(autowareStack, &QComboBox::currentTextChanged, &window,
    [](const QString &v) { QSettings().setValue("addon/autoware_stack", v); });

  QObject::connect(terasimCfg, &QPushButton::clicked, &window, [&]() {
    QMessageBox::information(&window, "TeraSim",
      "TeraSim settings dialog lands with the next pass. For now, the row "
      "above persists scenario selection; install/update lives in "
      "Cfg → Third-Party Tools.");
  });
  QObject::connect(autowareCfg, &QPushButton::clicked, &window, [&]() {
    QMessageBox::information(&window, "Autoware",
      "Autoware settings dialog lands with the next pass. For now, the row "
      "above persists stack selection; install/update lives in "
      "Cfg → Third-Party Tools.");
  });

  launchGroup->setLayout(launchLayout);

  QSpinBox *portSpin = new QSpinBox();
  portSpin->setRange(1024, 65535);
  portSpin->setValue(2000);
  portSpin->setToolTip(
    "CARLA RPC port. Default 2000 — sim listens on this port + the next "
    "two (streaming, secondary). Change only if you've reassigned ports.");
  QLabel *endpointLabel = new QLabel("Endpoint: localhost:2000");
  endpointLabel->setVisible(false);
  hostPortRow->addWidget(new QLabel("Port:"));
  hostPortRow->addWidget(portSpin);
  launchForm->addRow(new QLabel("Weather:"), weatherCombo);

  QGroupBox *connGroup = new QGroupBox();
  connGroup->setVisible(false);
  QVBoxLayout *connLayout = new QVBoxLayout();

  QLabel *statusLabel = new QLabel("Status: Idle");
  statusLabel->setObjectName("carlaStudioStatus");
  statusLabel->setStyleSheet(
    darkMode
      ? "QLabel { padding: 4px 10px; color: #E6E9EE; font-weight: 600;"
        "  background: #2F3440; border: 1px solid #3A3F4B; border-radius: 4px; }"
      : "QLabel { padding: 4px 10px; color: #374053; font-weight: 600;"
        "  background: #EDF0F4; border: 1px solid #E1E5EC; border-radius: 4px; }");

  QLabel *apiWarningLabel = new QLabel();
  apiWarningLabel->setVisible(false);
  apiWarningLabel->setCursor(Qt::PointingHandCursor);
  apiWarningLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  apiWarningLabel->setToolTip("Click to view full CARLA runtime log");
  auto libcarlaLogLinesPtr = std::make_shared<QStringList>();

  static std::streambuf *g_origCerr = std::cerr.rdbuf();
  static QPointer<QLabel> g_apiWarningLabel = apiWarningLabel;
  static std::shared_ptr<QStringList> g_libcarlaLogLines = libcarlaLogLinesPtr;
  static bool g_sdkVersionMismatch = false;
  static QString g_reportedSimVersion;
  static std::function<void()> g_refreshStatusBadge;
  static std::function<void()> g_disableSimButtonsOnMismatch;
  auto *cerrTap = new CerrTap(g_origCerr, [](const std::string &line) {
    const QString qline = QString::fromStdString(line).trimmed();
    if (qline.isEmpty()) return;
    QMetaObject::invokeMethod(QCoreApplication::instance(),
      [qline]() {
        if (!g_libcarlaLogLines) return;
        g_libcarlaLogLines->append(qline);
        while (g_libcarlaLogLines->size() > 500) g_libcarlaLogLines->removeFirst();

        bool saw = false;
        QString simVer;
        for (const QString &l : *g_libcarlaLogLines) {
          if (l.contains("Version mismatch")) saw = true;
          if (l.contains("Simulator API version")) {
            const int eq = l.indexOf('=');
            if (eq >= 0) simVer = l.mid(eq + 1).trimmed();
          }
        }
        if (saw) {
          g_sdkVersionMismatch = true;
          if (!simVer.isEmpty()) g_reportedSimVersion = simVer;
          if (g_refreshStatusBadge) g_refreshStatusBadge();
          if (g_disableSimButtonsOnMismatch) g_disableSimButtonsOnMismatch();
        }
      }, Qt::QueuedConnection);
  });
  std::cerr.rdbuf(cerrTap);

  QObject::connect(&app, &QApplication::aboutToQuit, &app, []() {
    std::cerr.rdbuf(g_origCerr);
  });

  QGroupBox *perfGroup = new QGroupBox("Processes");
  QVBoxLayout *perfLayout = new QVBoxLayout();
  QTableWidget *carlaProcessTable = new QTableWidget();
  carlaProcessTable->setColumnCount(5);
  carlaProcessTable->setHorizontalHeaderLabels(QStringList() << "PID" << "Process" << "CPU" << "Memory" << "GPU");
  carlaProcessTable->verticalHeader()->setVisible(false);
  carlaProcessTable->setSelectionMode(QAbstractItemView::NoSelection);
  carlaProcessTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  carlaProcessTable->setFocusPolicy(Qt::NoFocus);
  carlaProcessTable->setMinimumHeight(150);
  carlaProcessTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
  carlaProcessTable->setColumnWidth(0, 64);
  carlaProcessTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
  carlaProcessTable->setColumnWidth(1, 96);
  carlaProcessTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  carlaProcessTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  carlaProcessTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
  perfLayout->addWidget(carlaProcessTable);

  QGridLayout *processTotalsLayout = new QGridLayout();
  processTotalsLayout->addWidget(new QLabel("Total CPU:"), 0, 0);
  QProgressBar *totalCpuBar = new QProgressBar();
  totalCpuBar->setRange(0, 100);
  totalCpuBar->setFormat("0.0%");
  processTotalsLayout->addWidget(totalCpuBar, 0, 1);
  processTotalsLayout->addWidget(new QLabel("Total Memory:"), 1, 0);
  QProgressBar *totalMemBar = new QProgressBar();
  totalMemBar->setRange(0, 100);
  totalMemBar->setFormat("0.0%");
  processTotalsLayout->addWidget(totalMemBar, 1, 1);
  processTotalsLayout->addWidget(new QLabel("Total GPU:"), 2, 0);
  QProgressBar *totalGpuBar = new QProgressBar();
  totalGpuBar->setRange(0, 100);
  totalGpuBar->setFormat("n/a");
  processTotalsLayout->addWidget(totalGpuBar, 2, 1);
  perfLayout->addLayout(processTotalsLayout);

  QHBoxLayout *procButtons = new QHBoxLayout();
  QPushButton *refreshProcBtn = new QPushButton("⟳");
  refreshProcBtn->setToolTip(
    "Re-scan running CARLA processes. Updates the table + total bars.");
  refreshProcBtn->setFixedWidth(34);
  QPushButton *startBtn = new QPushButton("START");
  QPushButton *stopBtn = new QPushButton("STOP");
  startBtn->setToolTip(
    "Launch CARLA with the chosen Map / View / Host / Weather / Port. "
    "Disabled while CARLA root isn't configured or a launch is in flight.");
  stopBtn->setToolTip(
    "Stop the running CARLA process(es). Sends SIGTERM, then SIGKILL "
    "after 60 s if still alive.");
  startBtn->setStyleSheet(
    "QPushButton {"
    "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #43A047, stop:1 #2E7D32);"
    "  color: white; font-weight: 700; padding: 7px 14px;"
    "  border: 1px solid #1B5E20; border-radius: 4px; }"
    "QPushButton:hover { background: #388E3C; }"
    "QPushButton:pressed { background: #1B5E20; }"
    "QPushButton:disabled { background: #C5E1A5; color: #F5F5F5; border-color: #9CCC65; }");
  stopBtn->setStyleSheet(
    "QPushButton {"
    "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 #E53935, stop:1 #C62828);"
    "  color: white; font-weight: 700; padding: 7px 14px;"
    "  border: 1px solid #B71C1C; border-radius: 4px; }"
    "QPushButton:hover { background: #D32F2F; }"
    "QPushButton:pressed { background: #B71C1C; }"
    "QPushButton:disabled { background: #FFCDD2; color: #F5F5F5; border-color: #EF9A9A; }");
  procButtons->addWidget(refreshProcBtn);
  procButtons->addWidget(startBtn);
  procButtons->addWidget(stopBtn);
  QVBoxLayout *protocolHostLayout = new QVBoxLayout();
  protocolHostLayout->setContentsMargins(0, 4, 0, 4);
  perfLayout->addLayout(protocolHostLayout);
  perfLayout->addLayout(procButtons);
  perfGroup->setLayout(perfLayout);
  connGroup->setLayout(connLayout);

  QList<qint64> trackedCarlaPids;
  bool launchRequestedOrRunning = false;
  QString g_dockerContainerId;
  bool rootConfiguredOk = false;
  QStringList pluginSearchDirs;
  int pluginsLoadedCount = 0;
  QTimer *forceStopTimer = new QTimer(&window);
  forceStopTimer->setSingleShot(true);
  std::function<void()> refreshProcessList;

  auto setSimulationStatus = [&](const QString &state) {
    statusLabel->setText(QString("Status: %1").arg(state));
    const bool isBusy = state.startsWith("Running") || state.startsWith("Initializing");
    launchRequestedOrRunning = isBusy;
    startBtn->setEnabled(rootConfiguredOk && !isBusy);
    stopBtn->setEnabled(rootConfiguredOk);
    setSimReachable(state.startsWith("Running"));

    
    const bool isConnecting = isBusy && (state.contains("connecting", Qt::CaseInsensitive)
                                         || state.contains("Initializing", Qt::CaseInsensitive)
                                         || state.contains("loading", Qt::CaseInsensitive));
    const bool isRunning    = state.startsWith("Running") && !isConnecting;
    
    static const char kBtnBase[] =
      "QPushButton { color: white; font-weight: 700; padding: 7px 14px;"
      "  border: 1px solid %3; border-radius: 4px;"
      "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1, stop:0 %1, stop:1 %2); }"
      "QPushButton:hover  { background: %2; }"
      "QPushButton:pressed{ background: %3; }"
      "QPushButton:disabled{ background: #C5E1A5; color: #F5F5F5; border-color: #9CCC65; }";
    if (isConnecting) {
      startBtn->setText("Connecting…");
      startBtn->setStyleSheet(QString(kBtnBase).arg("#F9A825", "#F57F17", "#E65100"));
    } else if (isRunning) {
      startBtn->setText("Running");
      startBtn->setStyleSheet(QString(kBtnBase).arg("#43A047", "#2E7D32", "#1B5E20"));
    } else {
      startBtn->setText("START");
      startBtn->setStyleSheet(QString(kBtnBase).arg("#43A047", "#2E7D32", "#1B5E20"));
    }
  };

  auto refreshRememberedPidList = [&]() {
    Q_UNUSED(trackedCarlaPids);
  };

  auto rememberPid = [&](qint64 pid) {
    if (pid <= 0) {
      return;
    }
    if (!trackedCarlaPids.contains(pid)) {
      trackedCarlaPids.append(pid);
      refreshRememberedPidList();
    }
  };

  auto discoverCarlaSimPids = [&]() {
    QProcess finder;
    finder.start("/bin/bash", QStringList() << "-lc"
      << "pgrep -f 'CarlaUE[45]-Linux|UnrealEditor|CarlaUnreal' 2>/dev/null | head -10");
    finder.waitForFinished(1500);
    const QString out = QString::fromLocal8Bit(finder.readAllStandardOutput());
    for (const QString &token : out.split('\n', Qt::SkipEmptyParts)) {
      bool ok = false;
      const qint64 pid = token.trimmed().toLongLong(&ok);
      if (ok) {
        rememberPid(pid);
      }
    }
  };

  auto killTrackedPids = [&]() {
    for (qint64 pid : trackedCarlaPids) {
      if (pid > 0) {
        QProcess::execute("/bin/bash", QStringList() << "-lc" << QString("kill -TERM %1 2>/dev/null || true").arg(pid));
      }
    }
    trackedCarlaPids.clear();
    refreshRememberedPidList();
  };

  auto killAllCarlaProcesses = [&]() {
    
    
    
    
    
    
    QProcess::execute("/bin/bash", QStringList() << "-lc"
        << "pkill -TERM -f 'CarlaUE4|CarlaUE5|CarlaUnreal|CarlaUE' 2>/dev/null || true");
  };

  auto forceStopNow = [&]() {
    
    QProcess::execute("/bin/bash", QStringList() << "-lc"
        << "pkill -KILL -f 'CarlaUE4|CarlaUE5|CarlaUnreal|CarlaUE' 2>/dev/null || true");
    trackedCarlaPids.clear();
    refreshRememberedPidList();
    refreshProcessList();
    setSimulationStatus("Stopped");
  };

  QObject::connect(forceStopTimer, &QTimer::timeout, &window, [&]() {
    forceStopNow();
  });
  
  QGroupBox *kbGroup = new QGroupBox("Keyboard Control");
  QVBoxLayout *kbLayout = new QVBoxLayout();
  QLabel *keyLastPressedLabel = new QLabel("Last key: none");
  QLabel *keyBindingsLabel = new QLabel("Keys: W A S D / ↑ ↓ ← → / Space");
  keyBindingsLabel->setWordWrap(true);
  kbLayout->addWidget(keyLastPressedLabel);
  kbLayout->addWidget(keyBindingsLabel);
  kbGroup->setLayout(kbLayout);

  auto setControlGroupConnected = [&](QGroupBox *group, bool connected) {
    group->setEnabled(connected);
  };

  struct DriveKeyState {
    bool throttle = false;
    bool brake = false;
    bool steerLeft = false;
    bool steerRight = false;
    bool handbrake = false;
  } driveKeys;
  bool inAppDriverControlActive = false;

  auto setDriveKeyState = [&](int key, bool pressed) {
    if (!inAppDriverControlActive) {
      return;
    }
    switch (key) {
      case Qt::Key_W:
      case Qt::Key_Up:
        driveKeys.throttle = pressed;
        break;
      case Qt::Key_S:
      case Qt::Key_Down:
        driveKeys.brake = pressed;
        break;
      case Qt::Key_A:
      case Qt::Key_Left:
        driveKeys.steerLeft = pressed;
        break;
      case Qt::Key_D:
      case Qt::Key_Right:
        driveKeys.steerRight = pressed;
        break;
      case Qt::Key_Space:
        driveKeys.handbrake = pressed;
        break;
      default:
        break;
    }
  };

  KeyboardInputFilter *keyboardFilter = new KeyboardInputFilter(
    [&](int key, bool pressed) {
      setDriveKeyState(key, pressed);
      if (!pressed) {
        return;
      }
      switch (key) {
        case Qt::Key_W: keyLastPressedLabel->setText("Last key: W"); break;
        case Qt::Key_A: keyLastPressedLabel->setText("Last key: A"); break;
        case Qt::Key_S: keyLastPressedLabel->setText("Last key: S"); break;
        case Qt::Key_D: keyLastPressedLabel->setText("Last key: D"); break;
        case Qt::Key_Up: keyLastPressedLabel->setText("Last key: Up"); break;
        case Qt::Key_Down: keyLastPressedLabel->setText("Last key: Down"); break;
        case Qt::Key_Left: keyLastPressedLabel->setText("Last key: Left"); break;
        case Qt::Key_Right: keyLastPressedLabel->setText("Last key: Right"); break;
        case Qt::Key_Space: keyLastPressedLabel->setText("Last key: Space"); break;
        default: break;
      }
    },
    &window);
  app.installEventFilter(keyboardFilter);

  auto isKeyboardConnected = [&]() -> bool {
    QDir byPath("/dev/input/by-path");
    if (byPath.exists()) {
      const QStringList kbDevices = byPath.entryList(QStringList() << "*kbd*", QDir::System | QDir::Files | QDir::NoDotAndDotDot);
      if (!kbDevices.isEmpty()) {
        return true;
      }
    }
    QDir byId("/dev/input/by-id");
    if (byId.exists()) {
      const QStringList kbDevices = byId.entryList(QStringList() << "*kbd*", QDir::System | QDir::Files | QDir::NoDotAndDotDot);
      if (!kbDevices.isEmpty()) {
        return true;
      }
    }
    return false;
  };

  setControlGroupConnected(kbGroup, isKeyboardConnected());

  auto bindKeyShortcut = [&](const QString &sequence, const QString &displayText) {
    QShortcut *shortcut = new QShortcut(QKeySequence(sequence), &window);
    QObject::connect(shortcut, &QShortcut::activated, &window, [&, displayText]() {
      keyLastPressedLabel->setText(QString("Last key: %1").arg(displayText));
    });
  };
  bindKeyShortcut("W", "W");
  bindKeyShortcut("A", "A");
  bindKeyShortcut("S", "S");
  bindKeyShortcut("D", "D");
  bindKeyShortcut("Up", "Up");
  bindKeyShortcut("Down", "Down");
  bindKeyShortcut("Left", "Left");
  bindKeyShortcut("Right", "Right");
  bindKeyShortcut("Space", "Space");
  
  QGroupBox *jsGroup = new QGroupBox("Joystick Control");
  QVBoxLayout *jsLayout = new QVBoxLayout();
  QLabel *jsStatus = new QLabel("Joystick: not connected");
  jsStatus->hide();

  jsLayout->addWidget(new QLabel("Connection:"));
  QHBoxLayout *connTypeLayout = new QHBoxLayout();
  QRadioButton *usbIndicator = new QRadioButton();
  QLabel *usbText = new QLabel("USB");
  QLabel *usbCountText = new QLabel("x0");
  QRadioButton *bluetoothIndicator = new QRadioButton();
  QLabel *bluetoothText = new QLabel("Bluetooth");
  QLabel *bluetoothCountText = new QLabel("x0");
  usbIndicator->setEnabled(false);
  bluetoothIndicator->setEnabled(false);
  usbIndicator->setStyleSheet("color: #D32F2F;");
  bluetoothIndicator->setStyleSheet("color: #D32F2F;");
  connTypeLayout->addWidget(usbIndicator);
  connTypeLayout->addWidget(usbText);
  connTypeLayout->addWidget(usbCountText);
  connTypeLayout->addSpacing(12);
  connTypeLayout->addWidget(bluetoothIndicator);
  connTypeLayout->addWidget(bluetoothText);
  connTypeLayout->addWidget(bluetoothCountText);
  connTypeLayout->addStretch();
  jsLayout->addLayout(connTypeLayout);

  QTextEdit *assignments = new QTextEdit();
  assignments->setReadOnly(true);
  assignments->setMaximumHeight(90);
  assignments->setText("No active joystick assignments.");
  jsLayout->addWidget(assignments);

  QHBoxLayout *mainAssignmentLayout = new QHBoxLayout();
  mainAssignmentLayout->addWidget(new QLabel("Primary controls:"));
  QComboBox *mainControllerTarget = new QComboBox();
  mainControllerTarget->setEditable(true);
  mainControllerTarget->addItems(QStringList() << "Player 1 / Ego Car" << "Player 2 / NPC Car" << "Player 3 / Support Car");
  mainAssignmentLayout->addWidget(mainControllerTarget);
  jsLayout->addLayout(mainAssignmentLayout);

  QHBoxLayout *controllerLayout = new QHBoxLayout();

  QVBoxLayout *leftControllerLayout = new QVBoxLayout();
  QGroupBox *leftStickGroup = new QGroupBox();
  leftStickGroup->setAlignment(Qt::AlignHCenter);
  QGridLayout *leftStickGrid = new QGridLayout();
  QLabel *leftUpLabel = new QLabel("↑");
  QLabel *leftLeftLabel = new QLabel("←");
  QLabel *leftCenterLabel = new QLabel("L");
  QLabel *leftRightLabel = new QLabel("→");
  QLabel *leftDownLabel = new QLabel("↓");
  leftCenterLabel->setStyleSheet("font-weight: bold; border: 1px solid #888; border-radius: 12px; padding: 4px; min-width: 20px;");
  leftUpLabel->setAlignment(Qt::AlignCenter);
  leftLeftLabel->setAlignment(Qt::AlignCenter);
  leftCenterLabel->setAlignment(Qt::AlignCenter);
  leftRightLabel->setAlignment(Qt::AlignCenter);
  leftDownLabel->setAlignment(Qt::AlignCenter);
  leftStickGrid->addWidget(leftUpLabel, 0, 1);
  leftStickGrid->addWidget(leftLeftLabel, 1, 0);
  leftStickGrid->addWidget(leftCenterLabel, 1, 1);
  leftStickGrid->addWidget(leftRightLabel, 1, 2);
  leftStickGrid->addWidget(leftDownLabel, 2, 1);
  leftStickGroup->setLayout(leftStickGrid);
  leftControllerLayout->addWidget(leftStickGroup);

  QGroupBox *leftDpadGroup = new QGroupBox("D-Pad (Display)");
  QGridLayout *leftDpadGrid = new QGridLayout();
  QLabel *dpadUp = new QLabel("↑");
  QLabel *dpadLeft = new QLabel("←");
  QLabel *dpadRight = new QLabel("→");
  QLabel *dpadDown = new QLabel("↓");
  dpadUp->setAlignment(Qt::AlignCenter);
  dpadLeft->setAlignment(Qt::AlignCenter);
  dpadRight->setAlignment(Qt::AlignCenter);
  dpadDown->setAlignment(Qt::AlignCenter);
  dpadUp->setStyleSheet("color: #333; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; padding: 3px;");
  dpadLeft->setStyleSheet("color: #333; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; padding: 3px;");
  dpadRight->setStyleSheet("color: #333; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; padding: 3px;");
  dpadDown->setStyleSheet("color: #333; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; padding: 3px;");
  dpadUp->setMinimumSize(28, 22);
  dpadLeft->setMinimumSize(28, 22);
  dpadRight->setMinimumSize(28, 22);
  dpadDown->setMinimumSize(28, 22);
  leftDpadGrid->addWidget(dpadUp, 0, 1);
  leftDpadGrid->addWidget(dpadLeft, 1, 0);
  leftDpadGrid->addWidget(dpadRight, 1, 2);
  leftDpadGrid->addWidget(dpadDown, 2, 1);
  leftDpadGroup->setLayout(leftDpadGrid);

  QPushButton *l1ValueLabel = new QPushButton("L1");
  l1ValueLabel->setEnabled(false);
  QPushButton *ltValueLabel = new QPushButton("L2: 0.00");
  ltValueLabel->setEnabled(false);
  leftControllerLayout->addWidget(l1ValueLabel);
  leftControllerLayout->addWidget(ltValueLabel);

  QPushButton *l3ValueLabel = new QPushButton("L3");
  l3ValueLabel->setEnabled(false);
  leftControllerLayout->addWidget(l3ValueLabel);

  leftControllerLayout->addWidget(leftDpadGroup);

  leftControllerLayout->addStretch();

  QVBoxLayout *rightControllerLayout = new QVBoxLayout();
  QGroupBox *rightStickGroup = new QGroupBox();
  rightStickGroup->setAlignment(Qt::AlignHCenter);
  QGridLayout *rightStickGrid = new QGridLayout();
  QLabel *rightUpLabel = new QLabel("↑");
  QLabel *rightLeftLabel = new QLabel("←");
  QLabel *rightCenterLabel = new QLabel("R");
  QLabel *rightRightLabel = new QLabel("→");
  QLabel *rightDownLabel = new QLabel("↓");
  rightCenterLabel->setStyleSheet("font-weight: bold; border: 1px solid #888; border-radius: 12px; padding: 4px; min-width: 20px;");
  rightUpLabel->setAlignment(Qt::AlignCenter);
  rightLeftLabel->setAlignment(Qt::AlignCenter);
  rightCenterLabel->setAlignment(Qt::AlignCenter);
  rightRightLabel->setAlignment(Qt::AlignCenter);
  rightDownLabel->setAlignment(Qt::AlignCenter);
  rightStickGrid->addWidget(rightUpLabel, 0, 1);
  rightStickGrid->addWidget(rightLeftLabel, 1, 0);
  rightStickGrid->addWidget(rightCenterLabel, 1, 1);
  rightStickGrid->addWidget(rightRightLabel, 1, 2);
  rightStickGrid->addWidget(rightDownLabel, 2, 1);
  rightStickGroup->setLayout(rightStickGrid);
  rightControllerLayout->addWidget(rightStickGroup);

  QPushButton *r1ValueLabel = new QPushButton("R1");
  r1ValueLabel->setEnabled(false);
  QPushButton *rtValueLabel = new QPushButton("R2: 0.00");
  rtValueLabel->setEnabled(false);

  rightControllerLayout->addWidget(r1ValueLabel);
  rightControllerLayout->addWidget(rtValueLabel);

  QPushButton *r3ValueLabel = new QPushButton("R3");
  r3ValueLabel->setEnabled(false);
  rightControllerLayout->addWidget(r3ValueLabel);

  QGroupBox *buttonsGroup = new QGroupBox("Buttons & Function Mapping");
  QGridLayout *buttonsGrid = new QGridLayout();
  buttonsGrid->setHorizontalSpacing(6);
  buttonsGrid->setVerticalSpacing(4);
  buttonsGrid->addWidget(new QLabel("Button"), 0, 0);
  buttonsGrid->addWidget(new QLabel("Pressed"), 0, 1);
  buttonsGrid->addWidget(new QLabel("Function"), 0, 2);

  QStringList functionOptions = {
    "Accelerate", "Brake", "HandBrake", "Lights", "Horn",
    "Gear Up", "Gear Down", "Camera Toggle", "Autopilot Toggle", "Custom"
  };

  QCheckBox *btnA = new QCheckBox("A");
  QCheckBox *btnB = new QCheckBox("B");
  QCheckBox *btnX = new QCheckBox("X");
  QCheckBox *btnY = new QCheckBox("Y");
  btnA->setEnabled(false);
  btnB->setEnabled(false);
  btnX->setEnabled(false);
  btnY->setEnabled(false);

  QComboBox *mapA = new QComboBox();
  QComboBox *mapB = new QComboBox();
  QComboBox *mapX = new QComboBox();
  QComboBox *mapY = new QComboBox();
  mapA->addItems(functionOptions);
  mapB->addItems(functionOptions);
  mapX->addItems(functionOptions);
  mapY->addItems(functionOptions);
  mapA->setCurrentText("Accelerate");
  mapB->setCurrentText("Brake");
  mapX->setCurrentText("HandBrake");
  mapY->setCurrentText("Lights");
  mapA->setFixedWidth(130);
  mapB->setFixedWidth(130);
  mapX->setFixedWidth(130);
  mapY->setFixedWidth(130);

  buttonsGrid->addWidget(new QLabel("A"), 1, 0);
  buttonsGrid->addWidget(btnA, 1, 1);
  buttonsGrid->addWidget(mapA, 1, 2);
  buttonsGrid->addWidget(new QLabel("B"), 2, 0);
  buttonsGrid->addWidget(btnB, 2, 1);
  buttonsGrid->addWidget(mapB, 2, 2);
  buttonsGrid->addWidget(new QLabel("X"), 3, 0);
  buttonsGrid->addWidget(btnX, 3, 1);
  buttonsGrid->addWidget(mapX, 3, 2);
  buttonsGrid->addWidget(new QLabel("Y"), 4, 0);
  buttonsGrid->addWidget(btnY, 4, 1);
  buttonsGrid->addWidget(mapY, 4, 2);
  buttonsGroup->setLayout(buttonsGrid);
  buttonsGroup->setMaximumWidth(300);
  rightControllerLayout->addWidget(buttonsGroup);

  controllerLayout->addLayout(leftControllerLayout, 1);
  controllerLayout->addSpacing(10);
  controllerLayout->addLayout(rightControllerLayout, 1);
  jsLayout->addLayout(controllerLayout);

  auto setIndicatorState = [](QRadioButton *indicator, bool active) {
    indicator->setChecked(active);
    indicator->setStyleSheet(active ? "color: #2E7D32;" : "color: #D32F2F;");
  };

  auto setDpadState = [](QLabel *label, const QString &arrow, bool pressed) {
    label->setText(arrow);
    label->setStyleSheet(pressed
      ? "color: #111; border: 1px solid #616161; border-radius: 4px; background: #757575; padding: 3px; font-weight: bold;"
      : "color: #333; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; padding: 3px;");
  };

  double lt = 0.0, rt = 0.0, lx = 0.0, ly = 0.0, rx = 0.0, ry = 0.0;
  bool aPressed = false, bPressed = false, xPressed = false, yPressed = false;
  bool l1Pressed = false, r1Pressed = false;
  bool l3Pressed = false, r3Pressed = false;
  bool dpadUpPressed = false, dpadDownPressed = false, dpadLeftPressed = false, dpadRightPressed = false;
  int jsFd = -1;
  QString jsName;
  std::vector<QWidget *> joystickMirrorWindows;
  std::vector<QLabel *> joystickMirrorTelemetryLabels;

  auto scanJoystickDevices = [&]() -> QStringList {
    QStringList names;
#ifdef CARLA_STUDIO_HAS_LINUX_JOYSTICK
    for (int index = 0; index < 8; ++index) {
      const QString path = QString("/dev/input/js%1").arg(index);
      int fd = open(path.toStdString().c_str(), O_RDONLY | O_NONBLOCK);
      if (fd < 0) {
        continue;
      }
      char nameBuf[128] = {0};
      QString resolvedName = QString("Joystick %1").arg(index + 1);
      if (ioctl(fd, JSIOCGNAME(sizeof(nameBuf)), nameBuf) >= 0 && std::strlen(nameBuf) > 0) {
        resolvedName = QString::fromLocal8Bit(nameBuf);
      }
      close(fd);
      names.append(resolvedName);
    }
#endif
    return names;
  };

  auto rebuildJoystickMirrorWindows = [&](const QStringList &allJoysticks) {
    for (QWidget *windowPtr : joystickMirrorWindows) {
      if (windowPtr) {
        windowPtr->close();
        windowPtr->deleteLater();
      }
    }
    joystickMirrorWindows.clear();
    joystickMirrorTelemetryLabels.clear();

    for (int index = 1; index < allJoysticks.size(); ++index) {
      QWidget *mirrorWindow = new QWidget(nullptr, Qt::Window);
      mirrorWindow->setWindowTitle(QString("Joystick %1").arg(index + 1));
      mirrorWindow->resize(340, 220);
      mirrorWindow->move(180 + (index * 24), 160 + (index * 24));

      QVBoxLayout *mirrorLayout = new QVBoxLayout();
      mirrorLayout->addWidget(new QLabel(QString("Source device: %1").arg(allJoysticks[index])));
      QHBoxLayout *assignmentLayout = new QHBoxLayout();
      assignmentLayout->addWidget(new QLabel("Controlling:"));
      QComboBox *targetCombo = new QComboBox();
      targetCombo->setEditable(true);
      targetCombo->addItems(QStringList()
        << QString("Player %1 / Car %1").arg(index + 1)
        << QString("Player %1 / Ego Car").arg(index + 1)
        << QString("Player %1 / Support Car").arg(index + 1));
      assignmentLayout->addWidget(targetCombo);
      mirrorLayout->addLayout(assignmentLayout);

      QLabel *mirrorTelemetry = new QLabel("Awaiting telemetry...");
      mirrorTelemetry->setWordWrap(true);
      mirrorTelemetry->setStyleSheet("border: 1px solid #9e9e9e; border-radius: 4px; background: #f5f5f5; padding: 6px;");
      mirrorLayout->addWidget(mirrorTelemetry);

      mirrorWindow->setLayout(mirrorLayout);
      mirrorWindow->show();

      joystickMirrorWindows.push_back(mirrorWindow);
      joystickMirrorTelemetryLabels.push_back(mirrorTelemetry);
    }
  };

  auto updateTelemetry = [&]() {
    leftUpLabel->setText("↑");
    leftLeftLabel->setText("←");
    leftRightLabel->setText("→");
    leftDownLabel->setText("↓");

    rightUpLabel->setText("↑");
    rightLeftLabel->setText("←");
    rightRightLabel->setText("→");
    rightDownLabel->setText("↓");

    const bool l2Pressed = lt > 0.05;
    const bool r2Pressed = rt > 0.05;
    ltValueLabel->setText(l2Pressed
      ? QString("L2: %1 (Pressed)").arg(lt, 0, 'f', 2)
      : QString("L2: %1").arg(lt, 0, 'f', 2));
    rtValueLabel->setText(r2Pressed
      ? QString("R2: %1 (Pressed)").arg(rt, 0, 'f', 2)
      : QString("R2: %1").arg(rt, 0, 'f', 2));
    ltValueLabel->setStyleSheet(l2Pressed
      ? "font-family: monospace; border: 1px solid #616161; border-radius: 4px; background: #757575; color: #111; padding: 3px; font-weight: bold;"
      : "font-family: monospace; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; color: #333; padding: 3px;");
    rtValueLabel->setStyleSheet(r2Pressed
      ? "font-family: monospace; border: 1px solid #616161; border-radius: 4px; background: #757575; color: #111; padding: 3px; font-weight: bold;"
      : "font-family: monospace; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; color: #333; padding: 3px;");

    l1ValueLabel->setText(l1Pressed ? "L1: Pressed" : "L1");
    r1ValueLabel->setText(r1Pressed ? "R1: Pressed" : "R1");
    l1ValueLabel->setStyleSheet(l1Pressed
      ? "font-family: monospace; border: 1px solid #616161; border-radius: 4px; background: #757575; color: #111; padding: 3px; font-weight: bold;"
      : "font-family: monospace; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; color: #333; padding: 3px;");
    r1ValueLabel->setStyleSheet(r1Pressed
      ? "font-family: monospace; border: 1px solid #616161; border-radius: 4px; background: #757575; color: #111; padding: 3px; font-weight: bold;"
      : "font-family: monospace; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; color: #333; padding: 3px;");

    l3ValueLabel->setText(l3Pressed ? "L3: Pressed" : "L3");
    r3ValueLabel->setText(r3Pressed ? "R3: Pressed" : "R3");
    l3ValueLabel->setStyleSheet(l3Pressed
      ? "font-family: monospace; border: 1px solid #616161; border-radius: 4px; background: #757575; color: #111; padding: 3px; font-weight: bold;"
      : "font-family: monospace; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; color: #333; padding: 3px;");
    r3ValueLabel->setStyleSheet(r3Pressed
      ? "font-family: monospace; border: 1px solid #616161; border-radius: 4px; background: #757575; color: #111; padding: 3px; font-weight: bold;"
      : "font-family: monospace; border: 1px solid #9e9e9e; border-radius: 4px; background: #e0e0e0; color: #333; padding: 3px;");

    btnA->setChecked(aPressed);
    btnB->setChecked(bPressed);
    btnX->setChecked(xPressed);
    btnY->setChecked(yPressed);

    setDpadState(dpadUp, "↑", dpadUpPressed);
    setDpadState(dpadDown, "↓", dpadDownPressed);
    setDpadState(dpadLeft, "←", dpadLeftPressed);
    setDpadState(dpadRight, "→", dpadRightPressed);

    const QString mirrorText = QString("Controls: %1\nL(%2, %3)  R(%4, %5)\nL2: %6  R2: %7\nA:%8 B:%9 X:%10 Y:%11")
      .arg(mainControllerTarget->currentText())
      .arg(lx, 0, 'f', 2)
      .arg(ly, 0, 'f', 2)
      .arg(rx, 0, 'f', 2)
      .arg(ry, 0, 'f', 2)
      .arg(lt, 0, 'f', 2)
      .arg(rt, 0, 'f', 2)
      .arg(aPressed ? "On" : "Off")
      .arg(bPressed ? "On" : "Off")
      .arg(xPressed ? "On" : "Off")
      .arg(yPressed ? "On" : "Off");
    for (QLabel *label : joystickMirrorTelemetryLabels) {
      if (label) {
        label->setText(mirrorText);
      }
    }
  };

  auto resetJoystickUi = [&]() {
    lt = rt = lx = ly = rx = ry = 0.0;
    aPressed = bPressed = xPressed = yPressed = false;
    l1Pressed = r1Pressed = false;
    l3Pressed = r3Pressed = false;
    dpadUpPressed = dpadDownPressed = dpadLeftPressed = dpadRightPressed = false;
    jsStatus->setText("Joystick: not connected");
    setControlGroupConnected(jsGroup, false);
    usbCountText->setText("x0");
    bluetoothCountText->setText("x0");
    assignments->setText("No active joystick assignments.");
    setIndicatorState(usbIndicator, false);
    setIndicatorState(bluetoothIndicator, false);
    updateTelemetry();
  };

  auto tryConnectJoystick = [&]() {
#ifndef CARLA_STUDIO_HAS_LINUX_JOYSTICK
    resetJoystickUi();
    return;
#else
    if (jsFd >= 0) {
      return;
    }

    const char *paths[] = {"/dev/input/js0", "/dev/input/js1", "/dev/input/js2"};
    for (const char *path : paths) {
      int fd = open(path, O_RDONLY | O_NONBLOCK);
      if (fd < 0) {
        continue;
      }

      jsFd = fd;
      char nameBuf[128] = {0};
      if (ioctl(jsFd, JSIOCGNAME(sizeof(nameBuf)), nameBuf) >= 0 && std::strlen(nameBuf) > 0) {
        jsName = QString::fromLocal8Bit(nameBuf);
      } else {
        jsName = "Generic Gamepad";
      }

      jsStatus->setText("Joystick connected: " + jsName);
      setControlGroupConnected(jsGroup, true);
      const QString lower = jsName.toLower();
      const bool isBluetooth = lower.contains("bluetooth") || lower.contains("wireless");
      const QStringList connectedNames = scanJoystickDevices();
      int usbCount = 0;
      int btCount = 0;
      for (const QString &connected : connectedNames) {
        const QString connectedLower = connected.toLower();
        if (connectedLower.contains("bluetooth") || connectedLower.contains("wireless")) {
          ++btCount;
        } else {
          ++usbCount;
        }
      }
      if (connectedNames.isEmpty()) {
        usbCount = isBluetooth ? 0 : 1;
        btCount = isBluetooth ? 1 : 0;
      }
      usbCountText->setText(QString("x%1").arg(usbCount));
      bluetoothCountText->setText(QString("x%1").arg(btCount));
      setIndicatorState(usbIndicator, usbCount > 0);
      setIndicatorState(bluetoothIndicator, btCount > 0);
      assignments->setText(QString("#1 %1 -> EGO Vehicle Control").arg(jsName));
      rebuildJoystickMirrorWindows(connectedNames);
      updateTelemetry();
      return;
    }

    resetJoystickUi();
#endif
  };

  auto pollJoystick = [&]() {
#ifndef CARLA_STUDIO_HAS_LINUX_JOYSTICK
    return;
#else
    if (jsFd < 0) {
      tryConnectJoystick();
      return;
    }

    bool changed = false;
    js_event event;
    while (true) {
      const ssize_t bytesRead = read(jsFd, &event, sizeof(event));
      if (bytesRead == static_cast<ssize_t>(sizeof(event))) {
        const unsigned char type = event.type & ~JS_EVENT_INIT;
        if (type == JS_EVENT_AXIS) {
          const double value = static_cast<double>(event.value) / 32767.0;
          switch (event.number) {
            case 0: lx = value; changed = true; break;
            case 1: ly = value; changed = true; break;
            case 2: lt = (value + 1.0) / 2.0; changed = true; break;
            case 3: rx = value; changed = true; break;
            case 4: ry = value; changed = true; break;
            case 5: rt = (value + 1.0) / 2.0; changed = true; break;
            case 6:
              dpadLeftPressed = (value < -0.5);
              dpadRightPressed = (value > 0.5);
              changed = true;
              break;
            case 7:
              dpadUpPressed = (value < -0.5);
              dpadDownPressed = (value > 0.5);
              changed = true;
              break;
            default: break;
          }
        } else if (type == JS_EVENT_BUTTON) {
          const bool pressed = (event.value != 0);
          switch (event.number) {
            case 0: aPressed = pressed; changed = true; break;
            case 1: bPressed = pressed; changed = true; break;
            case 2: xPressed = pressed; changed = true; break;
            case 3: yPressed = pressed; changed = true; break;
            case 4: l1Pressed = pressed; changed = true; break;
            case 5: r1Pressed = pressed; changed = true; break;
            case 9: l3Pressed = pressed; changed = true; break;
            case 10: r3Pressed = pressed; changed = true; break;
            default: break;
          }
        }
      } else {
        if (bytesRead < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
          break;
        }

        close(jsFd);
        jsFd = -1;
        jsName.clear();
        resetJoystickUi();
        return;
      }
    }

    if (changed) {
      updateTelemetry();
    }
#endif
  };

  QTimer *joystickTimer = new QTimer(&window);
  QObject::connect(joystickTimer, &QTimer::timeout, &window, [&]() { pollJoystick(); });
#ifdef CARLA_STUDIO_HAS_LINUX_JOYSTICK
  joystickTimer->start(50);
#else
  Q_UNUSED(joystickTimer);
#endif

  QTimer *keyboardStateTimer = new QTimer(&window);
  QObject::connect(keyboardStateTimer, &QTimer::timeout, &window, [&]() {
    setControlGroupConnected(kbGroup, isKeyboardConnected());
  });
  keyboardStateTimer->start(2000);

  tryConnectJoystick();
  jsLayout->addStretch();
  jsGroup->setLayout(jsLayout);

  QString detectedEngineLabel = "Unreal ?";

  auto probeInstalledCarlaVersion = [&]() -> QString {
    const QString root = carlaRootPath->text().trimmed();
    if (root.isEmpty()) return QString();
    
    
    
    
    for (const QString &p : { root + "/VERSION", root + "/version.txt" }) {
      QFile f(p);
      if (f.open(QIODevice::ReadOnly)) {
        const QString v = QString::fromLocal8Bit(f.readLine()).trimmed();
        if (!v.isEmpty() && v[0].isDigit()) return v;
      }
    }
    QRegularExpression dirVer("CARLA[_-]v?([0-9]+\\.[0-9]+(?:\\.[0-9]+)?)",
                              QRegularExpression::CaseInsensitiveOption);
    
    
    
    QString cur = root;
    for (int i = 0; i < 3 && !cur.isEmpty(); ++i) {
      const auto m = dirVer.match(QFileInfo(cur).fileName());
      if (m.hasMatch()) return m.captured(1);
      const QString parent = QFileInfo(cur).path();
      if (parent == cur) break;
      cur = parent;
    }
    
    if (QFileInfo(root + "/CarlaUnreal.sh").isFile()) return "0.10.0";
    return QString();
  };

  auto updateStatusBadge = [&]() {
    if (!apiWarningLabel) return;
    QString text;
    QString color;
    if (!rootConfiguredOk) {
      text  = "Not Configured (click to configure)";
      color = "#F9A825";
    } else if (g_sdkVersionMismatch) {
      const QString v = g_reportedSimVersion.isEmpty()
                         ? QStringLiteral("?")
                         : g_reportedSimVersion;
      text  = QString("CARLA %1 with %2").arg(v, detectedEngineLabel);
      color = "#C62828";
    } else {
      QString v = probeInstalledCarlaVersion();
      if (v.isEmpty()) {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
        v = QString::fromLatin1(carla::version());
#else
        v = QStringLiteral("offline");
#endif
      }
      const QString sbRoot = carlaRootPath ? carlaRootPath->text().trimmed() : QString();
      const bool hasUE5Tree = !sbRoot.isEmpty() && (
          QFileInfo(sbRoot + "/Unreal/CarlaUnreal/Source").isDir()
          || QFileInfo(sbRoot + "/Unreal/CarlaUE5/Source").isDir());
      const bool hasUE4Tree = !sbRoot.isEmpty()
          && QFileInfo(sbRoot + "/Unreal/CarlaUE4/Source").isDir();
      if (hasUE5Tree || hasUE4Tree) {
        text = QString("Carla (src) w/UE.%1").arg(hasUE5Tree ? 5 : 4);
      } else {
        text = QString("CARLA %1 with %2").arg(v, detectedEngineLabel);
      }
      color = "";
    }
    apiWarningLabel->setText(text);
    apiWarningLabel->setVisible(true);
    const QString col = color.isEmpty()
                          ? QString()
                          : QString(" color: %1; font-weight: 600;").arg(color);
    apiWarningLabel->setStyleSheet(
      QString("QLabel { padding: 2px 6px; font-weight: 500;%1 }"
              "QLabel:hover { text-decoration: underline; }").arg(col));
  };

  g_refreshStatusBadge = [&]() { updateStatusBadge(); };

  auto validateCarlaRoot = [&]() {
    const QString path = carlaRootPath->text().trimmed();
    
    
    const bool hasUE4    = QFileInfo(path + "/CarlaUE4.sh").isFile();
    const bool hasUE5    = QFileInfo(path + "/CarlaUE5.sh").isFile();
    const bool hasUnreal = QFileInfo(path + "/CarlaUnreal.sh").isFile();
    const QString envUnrealPath = QString::fromLocal8Bit(qgetenv("CARLA_UNREAL_ENGINE_PATH")).trimmed();

    auto inferEngine = [&]() -> QString {
      const QString envLower = envUnrealPath.toLower();
      if (envLower.contains("ue5") || envLower.contains("unrealengine5") || envLower.contains("5.")) return "Unreal 5";
      if (envLower.contains("ue4") || envLower.contains("unrealengine4") || envLower.contains("4.")) return "Unreal 4";
      
      
      
      
      if (hasUE5 && !hasUE4) return "Unreal 5";
      if (hasUE4 && !hasUE5) return "Unreal 4";
      if (hasUE5 &&  hasUE4) return "Unreal 4/5";
      if (hasUnreal)         return "Unreal 5";
      return "Unreal ?";
    };

    detectedEngineLabel = inferEngine();
    const bool hasSourceTree =
        !path.isEmpty() && (
          QFileInfo(path + "/Unreal/CarlaUnreal/Source").isDir()
          || QFileInfo(path + "/Unreal/CarlaUE4/Source").isDir()
          || QFileInfo(path + "/Unreal/CarlaUE5/Source").isDir());
    rootConfiguredOk    = (!path.isEmpty() && (hasUE4 || hasUE5 || hasUnreal || hasSourceTree));

    const bool showPathRow = !rootConfiguredOk;
    carlaRootPath->setVisible(showPathRow);
    rootPathLabel->setVisible(showPathRow);

    startBtn->setEnabled(rootConfiguredOk && !launchRequestedOrRunning);
    stopBtn->setEnabled(rootConfiguredOk);

    updateStatusBadge();
    return rootConfiguredOk;
  };

  QSettings appSettings("CARLA", "CARLAStudio");
  QStringList configuredRemoteHosts = appSettings.value("remote/hosts").toStringList();
  QStringList managedSshKeys = appSettings.value("ssh/keys").toStringList();

  auto detectDockerReady = [&]() {
    QProcess dockerProbe;
    dockerProbe.start("/bin/bash", QStringList() << "-lc"
      << "command -v docker >/dev/null 2>&1 && docker info >/dev/null 2>&1");
    dockerProbe.waitForFinished(2500);
    return dockerProbe.exitStatus() == QProcess::NormalExit && dockerProbe.exitCode() == 0;
  };

  auto getSelectedLaunchArgs = [&]() -> QStringList {
    QStringList args;
    if (optRenderOffscreen->isChecked()) {
      args << "-RenderOffScreen";
    } else if (optWindowSmall->isChecked()) {
      
      
      
      args << "-windowed" << "-ResX=640" << "-ResY=480";
    }
    
    
    
    
    if (optQualityLow->isChecked()) {
      args << "-quality-level=Low";
    } else if (optQualityEpic->isChecked()) {
      args << "-quality-level=Epic";
    } else {
      args << "-quality-level=Low";
    }
    if (optNoSound->isChecked()) {
      args << "-nosound";
    }
    if (optRos2->isChecked()) {
      args << "--ros2";
    }
    if (optPreferNvidia->isChecked()) {
      args << "-prefernvidia";
    }
    return args;
  };

  
  
  
  auto fitCarlaWindowToSafeSize = []() {
    auto *poll = new QTimer();
    poll->setInterval(1000);
    auto *deadline = new QTimer();
    deadline->setSingleShot(true);
    deadline->setInterval(30000);
    QObject::connect(poll, &QTimer::timeout, [poll, deadline]() {
      QProcess find;
      find.start("xdotool", QStringList() << "search" << "--name" << "CarlaUnreal");
      if (!find.waitForFinished(800)) { return; }
      const QString out = QString::fromLocal8Bit(find.readAllStandardOutput()).trimmed();
      if (out.isEmpty()) return;
      const QStringList wids = out.split('\n');
      const QString wid = wids.last();
      QProcess geom;
      geom.start("xdotool", QStringList() << "getwindowgeometry" << wid);
      geom.waitForFinished(500);
      const QString g = QString::fromLocal8Bit(geom.readAllStandardOutput());
      
      if (g.contains(QRegularExpression(R"(Geometry:\s*(\d{4,})x(\d{3,}))"))) {
        QProcess resize;
        resize.start("xdotool",
            QStringList() << "windowsize" << wid << "640" << "480");
        resize.waitForFinished(500);
      }
      poll->stop();
      deadline->stop();
      poll->deleteLater();
      deadline->deleteLater();
    });
    QObject::connect(deadline, &QTimer::timeout, [poll, deadline]() {
      poll->stop();
      poll->deleteLater();
      deadline->deleteLater();
    });
    poll->start();
    deadline->start();
  };

  QObject::connect(optQualityEpic, &QAction::toggled, &window, [&](bool checked) {
    if (checked && optQualityLow->isChecked()) {
      optQualityLow->setChecked(false);
    }
    QSettings().setValue("render/quality_epic", checked);
  });
  QObject::connect(optQualityLow, &QAction::toggled, &window, [&](bool checked) {
    if (checked && optQualityEpic->isChecked()) {
      optQualityEpic->setChecked(false);
    }
    QSettings().setValue("render/quality_low", checked);
  });
  QObject::connect(optNoSound, &QAction::toggled, &window, [](bool on) {
    QSettings().setValue("render/no_sound", on);
  });
  QObject::connect(optRenderOffscreen, &QAction::toggled, &window, [&](bool on) {
    QSettings().setValue("render/offscreen", on);
    if (on && optWindowSmall->isChecked()) optWindowSmall->setChecked(false);
  });
  QObject::connect(optWindowSmall, &QAction::toggled, &window, [&](bool on) {
    QSettings().setValue("render/window_small", on);
    if (on && optRenderOffscreen->isChecked()) optRenderOffscreen->setChecked(false);
  });

  
  
  
  {
    QSettings s;
    optQualityLow->setChecked(s.value("render/quality_low",     true ).toBool());
    optQualityEpic->setChecked(s.value("render/quality_epic",   false).toBool());
    optNoSound->setChecked(s.value("render/no_sound",           true ).toBool());
    
    
    
    
    
    optWindowSmall->setChecked(s.value("render/window_small",   true ).toBool());
    // Default ON for first run (avoids the GameThread/RenderThread crash on
    // some GPU+driver combos), but the user can untick it via Cfg menu if
    // they want a visible window AND their hardware survives windowed mode.
    optRenderOffscreen->setChecked(s.value("render/offscreen", true).toBool());
  }

  auto refreshRuntimeOptions = [&]() {
    const QString previous = runtimeTarget->currentText();
    runtimeTarget->clear();
    runtimeTarget->addItem("localhost");

    const bool dockerReady = detectDockerReady();
    const int dockerIndex = runtimeTarget->count();
    runtimeTarget->addItem(dockerReady ? "container" : "container (Docker not configured)");

    if (configuredRemoteHosts.isEmpty()) {
      const int remoteIndex = runtimeTarget->count();
      runtimeTarget->addItem("remotehost - Please configure in settings");
      if (QStandardItemModel *model = qobject_cast<QStandardItemModel *>(runtimeTarget->model())) {
        if (QStandardItem *item = model->item(remoteIndex)) {
          item->setEnabled(false);
        }
      }
    } else {
      for (const QString &host : configuredRemoteHosts) {
        runtimeTarget->addItem(QString("%1 (remote)").arg(host));
      }
    }

    if (QStandardItemModel *model = qobject_cast<QStandardItemModel *>(runtimeTarget->model())) {
      if (QStandardItem *dockerItem = model->item(dockerIndex)) {
        dockerItem->setEnabled(dockerReady);
      }
    }

    const int previousIndex = runtimeTarget->findText(previous);
    if (previousIndex >= 0) {
      runtimeTarget->setCurrentIndex(previousIndex);
    } else {
      runtimeTarget->setCurrentIndex(0);
    }
  };

  auto populateScenarios = [&]() {
    const QString previous = scenarioSelect->currentText();
    QSet<QString> scenarios;
    const QStringList defaults = {"Town01", "Town02", "Town03", "Town04", "Town05", "Town06"};
    for (const QString &town : defaults) {
      scenarios.insert(town);
    }

    const QString rootPath = carlaRootPath->text().trimmed();
    const QStringList mapPaths = {
      rootPath + "/Unreal/CarlaUnreal/Content/Carla/Maps",
      rootPath + "/Unreal/CarlaUE5/Content/Carla/Maps",
      rootPath + "/Unreal/CarlaUE4/Content/Carla/Maps"
    };
    for (const QString &path : mapPaths) {
      QDir dir(path);
      if (!dir.exists()) {
        continue;
      }
      const QStringList umaps = dir.entryList(QStringList() << "*.umap", QDir::Files);
      for (const QString &umap : umaps) {
        scenarios.insert(QFileInfo(umap).baseName());
      }
    }

    QStringList sorted = scenarios.values();
    sorted.sort(Qt::CaseInsensitive);
    scenarioSelect->clear();
    
    
    
    
    scenarioSelect->addItems(sorted);
    const int idx = scenarioSelect->findText(previous);
    scenarioSelect->setCurrentIndex(idx >= 0 ? idx : 0);
  };

  auto scanCarlaPort = [&]() -> int {
    QProcess probe;
    probe.start("/bin/bash", QStringList() << "-lc"
      << "ss -ltnp 2>/dev/null | grep -E 'CarlaUE4|CarlaUE5|CarlaUE' | head -n 1");
    if (!probe.waitForFinished(2000)) {
      probe.terminate();
      if (!probe.waitForFinished(800)) {
        probe.kill();
        probe.waitForFinished(-1);
      }
      return -1;
    }
    const QString line = QString::fromLocal8Bit(probe.readAllStandardOutput()).trimmed();
    QRegularExpression re(R"((?:\*|[0-9\.]+):(\d+))");
    QRegularExpressionMatch match = re.match(line);
    if (match.hasMatch()) {
      return match.captured(1).toInt();
    }
    return -1;
  };

  auto setProcessAreaEnabled = [&](bool enabled) {
    carlaProcessTable->setEnabled(enabled);
    totalCpuBar->setEnabled(enabled);
    totalMemBar->setEnabled(enabled);
    totalGpuBar->setEnabled(enabled);
  };
  setProcessAreaEnabled(false);

  refreshProcessList = [&]() {
    // Auto-discover any newly-spawned UE editor / simulator / launcher /
    // commandlet processes so the table reflects what's actually running
    // even when nothing in Studio explicitly called rememberPid().
    discoverCarlaSimPids();
    totalCpuBar->setValue(0);
    totalMemBar->setValue(0);
    totalGpuBar->setValue(0);
    totalCpuBar->setFormat("0.0%");
    totalMemBar->setFormat("0.0%");
    totalGpuBar->setFormat("n/a");
    carlaProcessTable->setRowCount(0);

    QStringList trackedPidStrings;
    for (qint64 pid : trackedCarlaPids) {
      if (pid > 0) {
        trackedPidStrings << QString::number(pid);
      }
    }
    trackedPidStrings.removeDuplicates();

    if (trackedPidStrings.isEmpty()) {
      setProcessAreaEnabled(false);
      refreshRememberedPidList();
      return;
    }
    setProcessAreaEnabled(true);

    const QString pidCsv = trackedPidStrings.join(',');

    QMap<qint64, double> gpuByPid;
    QMap<qint64, double> gpuMemPctByPid;
    QMap<qint64, qint64> gpuMemMiBByPid;

#if !defined(Q_OS_MAC) && !defined(__APPLE__)
    {
      QProcess smProbe;
      smProbe.start("/bin/bash", QStringList() << "-lc"
        << "nvidia-smi pmon -c 2 -s um 2>/dev/null "
           "| awk 'NR>2 && $2 ~ /^[0-9]+$/ { "
           "  if ($4 != \"-\" && $4 != \"N/A\") { sm[$2] += $4 + 0; c[$2] += 1; } "
           "} END { for (pid in sm) if (c[pid] > 0) printf \"%s %.1f\\n\", pid, sm[pid] / c[pid]; }'");
      smProbe.waitForFinished(2500);
      const QStringList smLines =
        QString::fromLocal8Bit(smProbe.readAllStandardOutput())
          .split('\n', Qt::SkipEmptyParts);
      for (const QString &line : smLines) {
        const QStringList parts = line.trimmed().split(
          QRegularExpression("\\s+"), Qt::SkipEmptyParts);
        if (parts.size() < 2) continue;
        bool pidOk = false, smOk = false;
        const qint64 pid = parts[0].toLongLong(&pidOk);
        const double sm = parts[1].toDouble(&smOk);
        if (pidOk && smOk) {
          gpuByPid[pid] = std::max(0.0, std::min(100.0, sm));
        }
      }

      qint64 totalVramMiB = 0;
      QProcess totalProbe;
      totalProbe.start("/bin/bash", QStringList() << "-lc"
        << "nvidia-smi --query-gpu=memory.total --format=csv,noheader,nounits 2>/dev/null | head -1");
      totalProbe.waitForFinished(1500);
      totalVramMiB = QString::fromLocal8Bit(totalProbe.readAllStandardOutput())
                       .trimmed().toLongLong();

      QProcess appsProbe;
      appsProbe.start("/bin/bash", QStringList() << "-lc"
        << "nvidia-smi --query-compute-apps=pid,used_memory --format=csv,noheader,nounits 2>/dev/null");
      appsProbe.waitForFinished(1500);
      const QStringList appLines =
        QString::fromLocal8Bit(appsProbe.readAllStandardOutput())
          .split('\n', Qt::SkipEmptyParts);
      for (const QString &line : appLines) {
        const QStringList parts = line.split(',');
        if (parts.size() < 2) continue;
        bool pidOk = false, memOk = false;
        const qint64 pid = parts[0].trimmed().toLongLong(&pidOk);
        const qint64 memMiB = parts[1].trimmed().toLongLong(&memOk);
        if (pidOk && memOk) {
          gpuMemMiBByPid[pid] = memMiB;
          if (totalVramMiB > 0) {
            const double pct = 100.0 * static_cast<double>(memMiB) / static_cast<double>(totalVramMiB);
            gpuMemPctByPid[pid] = std::max(0.0, std::min(100.0, pct));
          }
        }
      }
    }
#endif

    QProcess proc;
    proc.start("/bin/bash", QStringList() << "-lc"
      << QString("ps -p %1 -o pid=,pcpu=,pmem=,rss=,comm= --sort=-pcpu").arg(pidCsv));
    proc.waitForFinished(2000);
    const QString output = QString::fromLocal8Bit(proc.readAllStandardOutput());
    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);

    if (lines.isEmpty()) {
      setProcessAreaEnabled(false);
      refreshRememberedPidList();
      return;
    }

    qint64 systemRamMiB = carla_studio_proc::systemTotalRamMiB();

    auto fmtMiB = [](double mib) -> QString {
      if (mib < 1.0)     return QString("%1 MB").arg(mib, 0, 'f', 1);
      if (mib < 1024.0)  return QString("%1 MB").arg(mib, 0, 'f', 0);
      return QString("%1 GB").arg(mib / 1024.0, 0, 'f', 2);
    };

    auto makeMetricBar = [](double pct, const QString &bottomLine, bool available) {
      QProgressBar *bar = new QProgressBar();
      bar->setRange(0, 100);
      bar->setTextVisible(true);
      bar->setAlignment(Qt::AlignCenter);
      bar->setMinimumHeight(36);
      if (!available) {
        bar->setValue(0);
        bar->setFormat("n/a");
        bar->setStyleSheet("QProgressBar::chunk { background-color: #9E9E9E; }");
        return bar;
      }
      const double clamped = std::max(0.0, std::min(100.0, pct));
      bar->setValue(static_cast<int>(std::round(clamped)));
      bar->setFormat(QString("%1%\n%2").arg(pct, 0, 'f', 1).arg(bottomLine));
      return bar;
    };

    double totalCpu = 0.0;
    double totalMem = 0.0;
    double totalMemMiB = 0.0;
    double totalGpu = 0.0;
    double totalGpuMiB = 0.0;
    int gpuRows = 0;

    carlaProcessTable->setRowCount(lines.size());
    carlaProcessTable->verticalHeader()->setDefaultSectionSize(42);
    int row = 0;
    for (const QString &line : lines) {
      const QStringList parts = line.trimmed().split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
      if (parts.size() < 5) {
        continue;
      }
      bool pidOk = false;
      bool cpuOk = false;
      bool memOk = false;
      bool rssOk = false;
      const qint64 pid     = parts[0].toLongLong(&pidOk);
      const double cpu     = parts[1].toDouble(&cpuOk);
      const double mem     = parts[2].toDouble(&memOk);
      const qint64 rssKib  = parts[3].toLongLong(&rssOk);
      const QString processName = parts[4];
      const double rssMiB  = rssKib / 1024.0;

      static const std::array<std::pair<const char *, const char *>, 13> kPrettyProcs = {{
        {"CarlaUE4-Linux",   "Simulator (UE4)"},
        {"CarlaUE5-Linux",   "Simulator (UE5)"},
        {"CarlaUnreal",      "Simulator (Unreal)"},
        {"UnrealEditor",     "Unreal Editor"},
        {"CarlaUE4.sh",      "Launcher (.sh)"},
        {"CarlaUE5.sh",      "Launcher (.sh)"},
        {"CarlaUnreal.sh",   "Launcher (.sh)"},
        {"manual_control",   "manual_control.py"},
        {"python3",          "Python driver"},
        {"python",           "Python driver"},
        {"ros2",             "ROS2 bridge"},
        {"carla-studio",     "CARLA Studio"},
        {"bash",             "shell"},
      }};
      QString prettyName = processName;
      for (const auto &[needle, pretty] : kPrettyProcs) {
        if (processName.contains(QLatin1String(needle), Qt::CaseInsensitive)) {
          prettyName = QString::fromLatin1(pretty);
          break;
        }
      }

      auto *pidItem = new QTableWidgetItem(pidOk ? QString::number(pid) : "?");
      pidItem->setTextAlignment(Qt::AlignCenter);
      carlaProcessTable->setItem(row, 0, pidItem);
      auto *nameItem = new QTableWidgetItem(prettyName);
      nameItem->setToolTip(processName);
      carlaProcessTable->setItem(row, 1, nameItem);

      carlaProcessTable->setCellWidget(row, 2,
        makeMetricBar(cpuOk ? cpu : 0.0,
                      cpuOk ? QString("%1 cores").arg(cpu / 100.0, 0, 'f', 2)
                            : QStringLiteral(""),
                      cpuOk));
      carlaProcessTable->setCellWidget(row, 3,
        makeMetricBar(memOk ? mem : 0.0,
                      (memOk && rssOk) ? fmtMiB(rssMiB) : QStringLiteral(""),
                      memOk));

      const bool hasSm   = pidOk && gpuByPid.contains(pid);
      const bool hasVram = pidOk && gpuMemPctByPid.contains(pid);
      const bool hasGpu  = hasSm || hasVram;
      const double gpuPct = hasSm ? gpuByPid.value(pid)
                          : hasVram ? gpuMemPctByPid.value(pid)
                          : 0.0;
      const qint64 gpuMib = pidOk ? gpuMemMiBByPid.value(pid, 0) : 0;
      if (hasGpu) {
        carlaProcessTable->setCellWidget(row, 4,
          makeMetricBar(gpuPct,
                        gpuMib > 0 ? QString("%1 MiB").arg(gpuMib) : QStringLiteral(""),
                        true));
      } else {
        carlaProcessTable->setCellWidget(row, 4, makeMetricBar(0.0, "", false));
      }

      if (cpuOk)           totalCpu    += cpu;
      if (memOk)           totalMem    += mem;
      if (memOk && rssOk)  totalMemMiB += rssMiB;
      if (hasGpu) {
        totalGpu    += gpuPct;
        totalGpuMiB += double(gpuMib);
        gpuRows     += 1;
      }
      row += 1;
    }

    carlaProcessTable->setRowCount(row);

    auto styleTotalBar = [](QProgressBar *b) {
      b->setAlignment(Qt::AlignCenter);
      b->setTextVisible(true);
      b->setMinimumHeight(28);
      b->setMinimumWidth(240);
    };

    auto applyThresholdChunk = [](QProgressBar *b, double pct) {
      const char *color = "#2196F3";
      if (pct >= 95.0)      color = "#C64545";
      else if (pct >= 75.0) color = "#D9A13A";
      b->setStyleSheet(QString(
        "QProgressBar { border: 1px solid rgba(0,0,0,50); border-radius: 4px; "
        "text-align: center; background: palette(base); }"
        "QProgressBar::chunk { background-color: %1; border-radius: 3px; }").arg(color));
    };

    const double cpuAggregate = std::max(0.0, std::min(100.0, totalCpu));
    const double memAggregate = std::max(0.0, std::min(100.0, totalMem));
    styleTotalBar(totalCpuBar);
    styleTotalBar(totalMemBar);
    styleTotalBar(totalGpuBar);

    totalCpuBar->setValue(static_cast<int>(std::round(cpuAggregate)));
    totalCpuBar->setFormat(
      QString("%1%  ·  %2 cores")
        .arg(totalCpu, 0, 'f', 1)
        .arg(totalCpu / 100.0, 0, 'f', 2));
    totalCpuBar->setToolTip(
      QString("Aggregate CPU across CARLA processes: %1% (≈ %2 cores active)")
        .arg(totalCpu, 0, 'f', 1)
        .arg(totalCpu / 100.0, 0, 'f', 2));
    applyThresholdChunk(totalCpuBar, cpuAggregate);

    totalMemBar->setValue(static_cast<int>(std::round(memAggregate)));
    const QString memBottom = systemRamMiB > 0
      ? QString("%1 / %2").arg(fmtMiB(totalMemMiB), fmtMiB(double(systemRamMiB)))
      : fmtMiB(totalMemMiB);
    totalMemBar->setFormat(QString("%1%  ·  %2").arg(totalMem, 0, 'f', 1).arg(memBottom));
    totalMemBar->setToolTip(
      QString("Aggregate memory across CARLA processes: %1% (%2)")
        .arg(totalMem, 0, 'f', 1)
        .arg(memBottom));
    applyThresholdChunk(totalMemBar, memAggregate);

    if (gpuRows > 0) {
      const double gpuAggregate = std::max(0.0, std::min(100.0, totalGpu));
      totalGpuBar->setValue(static_cast<int>(std::round(gpuAggregate)));
      totalGpuBar->setFormat(
        QString("%1%  ·  %2 MiB")
          .arg(totalGpu, 0, 'f', 1)
          .arg(qint64(totalGpuMiB)));
      totalGpuBar->setToolTip(
        QString("Aggregate GPU across CARLA processes: %1% (%2 MiB)")
          .arg(totalGpu, 0, 'f', 1)
          .arg(qint64(totalGpuMiB)));
      applyThresholdChunk(totalGpuBar, gpuAggregate);
    } else {
      totalGpuBar->setValue(0);
      totalGpuBar->setFormat("n/a");
      totalGpuBar->setToolTip(
        "No GPU usage reported by CARLA processes (or GPU telemetry unavailable on this host).");
      totalGpuBar->setStyleSheet(
        "QProgressBar { border: 1px solid rgba(0,0,0,50); border-radius: 4px; "
        "text-align: center; background: palette(base); }"
        "QProgressBar::chunk { background-color: #9E9E9E; }");
    }

    refreshRememberedPidList();
  };

  auto updateEndpoint = [&]() {
    endpointLabel->setText(QString("Endpoint: %1:%2").arg(targetHost->text().trimmed()).arg(portSpin->value()));
  };

  QTimer *inAppDriveTick = new QTimer(&window);
  inAppDriveTick->setInterval(50);
  QTimer *inAppCameraTick = new QTimer(&window);
  inAppCameraTick->setInterval(80);

  
  QDockWidget *viewDock = nullptr;
  QVBoxLayout *viewGridLayout = nullptr;
  QMap<QString, QGroupBox *> viewTiles;

#ifdef CARLA_STUDIO_WITH_LIBCARLA
  QMap<QString, carla::SharedPtr<cc::Sensor>> activeViewSensors;

  std::shared_ptr<cc::Client> inAppDriveClient;
  carla::SharedPtr<cc::Vehicle> inAppDriveVehicle;
  bool inAppDriveSpawnedVehicle = false;

  
  
  
  
  enum class SelfDriveMode {
    Off              = 0,   
    ACC              = 1,   
    LaneKeep         = 2,   
    AssistNormal     = 3,   
    AssistAggressive = 4,   
    AutonomousLoop   = 5,   
  };
  std::unique_ptr<carla::agents::navigation::BehaviorAgent> selfDriveAgent;
  SelfDriveMode selfDriveMode = SelfDriveMode::Off;

  
  
  
  
  
  auto pickForwardDestination = [&](const cg::Transform &from,
                                     float maxDist = 400.0f,
                                     float minDot  = 0.2f)
      -> std::optional<cg::Transform> {
    if (!inAppDriveClient) return std::nullopt;
    try {
      auto world = inAppDriveClient->GetWorld();
      auto map = world.GetMap();
      if (!map) return std::nullopt;
      const auto spawnPts = map->GetRecommendedSpawnPoints();
      if (spawnPts.empty()) return std::nullopt;
      const auto fwd = from.GetForwardVector();
      cg::Transform best;
      float bestScore = -1.0f;
      for (const auto &c : spawnPts) {
        const float dx = c.location.x - from.location.x;
        const float dy = c.location.y - from.location.y;
        const float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < 60.0f || dist > maxDist) continue;
        const float dotForward = (dx * fwd.x + dy * fwd.y) / dist;
        if (dotForward < minDot) continue;
        
        
        const float score = dotForward * std::min(dist, 800.0f);
        if (score > bestScore) {
          bestScore = score;
          best = c;
        }
      }
      if (bestScore < 0.0f) return std::nullopt;
      return best;
    } catch (...) {
      return std::nullopt;
    }
  };

  auto engageSelfDrive = [&](SelfDriveMode mode) -> bool {
    if (mode == SelfDriveMode::Off) {
      selfDriveAgent.reset();
      selfDriveMode = SelfDriveMode::Off;
      return true;
    }
    if (!inAppDriveClient || !inAppDriveVehicle) return false;
    try {
      using BA = carla::agents::navigation::BehaviorAgent;
      BA::Behavior behavior = BA::Behavior::Normal;
      if (mode == SelfDriveMode::AssistAggressive ||
          mode == SelfDriveMode::AutonomousLoop) {
        behavior = BA::Behavior::Aggressive;
      }
      auto agent = std::make_unique<BA>(inAppDriveVehicle, behavior);
      const auto vt = inAppDriveVehicle->GetTransform();
      
      
      
      const bool isAuto = (mode == SelfDriveMode::AutonomousLoop);
      const float destMaxDist = isAuto ? 20000.0f : 400.0f;
      const float destMinDot  = isAuto ? -0.3f    : 0.2f;
      if (auto dest = pickForwardDestination(vt, destMaxDist, destMinDot)) {
        agent->SetDestination(dest->location);
      }
      selfDriveAgent = std::move(agent);
      selfDriveMode = mode;
      return true;
    } catch (...) {
      selfDriveAgent.reset();
      selfDriveMode = SelfDriveMode::Off;
      return false;
    }
  };

  auto stopInAppDriver = [&]() {
    inAppDriverControlActive = false;
    driveKeys = {};
    inAppDriveTick->stop();
    inAppCameraTick->stop();
    selfDriveAgent.reset();
    selfDriveMode = SelfDriveMode::Off;
    try {
      if (inAppDriveSpawnedVehicle && inAppDriveVehicle) {
        inAppDriveVehicle->Destroy();
      }
    } catch (...) {
    }
    inAppDriveVehicle = nullptr;
    inAppDriveClient.reset();
    inAppDriveSpawnedVehicle = false;
    
    for (auto it = activeViewSensors.begin(); it != activeViewSensors.end(); ++it) {
      try { if (it.value()) { it.value()->Stop(); it.value()->Destroy(); } } catch (...) {}
    }
    activeViewSensors.clear();
    for (auto *t : viewTiles) t->deleteLater();
    viewTiles.clear();
    if (viewDock) viewDock->hide();
    { QSignalBlocker b(fpvBtn); fpvBtn->setChecked(false); }
    { QSignalBlocker b(tpvBtn); tpvBtn->setChecked(false); }
    { QSignalBlocker b(cpvBtn); cpvBtn->setChecked(false); }
    { QSignalBlocker b(bevBtn); bevBtn->setChecked(false); }
  };

  auto updateThirdPersonCamera = [&]() {
    if (!inAppDriveClient || !inAppDriveVehicle) {
      return;
    }
    try {
      auto world = inAppDriveClient->GetWorld();
      auto spectator = world.GetSpectator();
      const auto vehicleTransform = inAppDriveVehicle->GetTransform();
      const double yawRad = static_cast<double>(vehicleTransform.rotation.yaw) * M_PI / 180.0;

      
      
      
      const float cameraDistance = 6.0f;
      cg::Location cameraLocation = vehicleTransform.location;
      cameraLocation.x -= static_cast<float>(std::cos(yawRad) * cameraDistance);
      cameraLocation.y -= static_cast<float>(std::sin(yawRad) * cameraDistance);
      cameraLocation.z += 2.6f;
      cg::Rotation cameraRotation(-12.0f, vehicleTransform.rotation.yaw, 0.0f);

      spectator->SetTransform(cg::Transform(cameraLocation, cameraRotation));
    } catch (...) {
    }
  };

  auto applyInAppVehicleControl = [&]() {
    if (!inAppDriveVehicle) {
      return;
    }
    carla::rpc::VehicleControl control;

    
    if (selfDriveAgent && selfDriveMode != SelfDriveMode::Off) {
      try {
        
        
        
        
        if (selfDriveMode == SelfDriveMode::AutonomousLoop &&
            selfDriveAgent->Done()) {
          const auto vt = inAppDriveVehicle->GetTransform();
          if (auto dest = pickForwardDestination(vt, 20000.0f, -0.3f)) {
            selfDriveAgent->SetDestination(dest->location);
          }
        }
        control = selfDriveAgent->RunStep();

        
        if (selfDriveMode == SelfDriveMode::ACC) {
          if (driveKeys.steerLeft == driveKeys.steerRight) {
            control.steer = 0.0f;
          } else if (driveKeys.steerLeft) {
            control.steer = -0.45f;
          } else {
            control.steer = 0.45f;
          }
          if (driveKeys.handbrake) control.hand_brake = true;
        }
        
        else if (selfDriveMode == SelfDriveMode::LaneKeep) {
          control.throttle = driveKeys.throttle ? 0.55f : 0.0f;
          control.brake    = driveKeys.brake    ? 0.65f : 0.0f;
          control.hand_brake = driveKeys.handbrake;
        }
      } catch (...) {
        return;
      }
    } else {
      
      control.throttle = driveKeys.throttle ? 0.55f : 0.0f;
      control.brake = driveKeys.brake ? 0.65f : 0.0f;
      control.hand_brake = driveKeys.handbrake;

      if (driveKeys.steerLeft == driveKeys.steerRight) {
        control.steer = 0.0f;
      } else if (driveKeys.steerLeft) {
        control.steer = -0.45f;
      } else {
        control.steer = 0.45f;
      }
    }

    control.reverse = false;
    control.manual_gear_shift = false;
    try {
      inAppDriveVehicle->ApplyControl(control);
    } catch (...) {
    }
  };

  auto startInAppDriver = [&](const QString &host, int port, const QString &scenarioName) -> bool {
    stopInAppDriver();
    try {
      
      
      
      
      
      std::string hostStr = host.toStdString();
      if (hostStr == "localhost" || hostStr == "::1") {
        hostStr = "127.0.0.1";
      }
      std::cerr << "[in-app driver] connecting to " << hostStr
                << ":" << port << " (scenario=\"" << scenarioName.toStdString()
                << "\")\n";

      
      
      
      
      
      
      
      
      
      
      
      
      
      

      
      
      auto tcpOpen = [&hostStr, port]() -> bool {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) return false;
        ::fcntl(fd, F_SETFL, O_NONBLOCK);
        struct sockaddr_in sa{};
        sa.sin_family = AF_INET;
        sa.sin_port = htons(static_cast<uint16_t>(port));
        ::inet_pton(AF_INET, hostStr.c_str(), &sa.sin_addr);
        ::connect(fd, reinterpret_cast<sockaddr *>(&sa), sizeof(sa));
        fd_set ws; FD_ZERO(&ws); FD_SET(fd, &ws);
        struct timeval tv{0, 100'000}; // 100 ms — localhost ECONNREFUSED is near-instant
        const bool open = ::select(fd + 1, nullptr, &ws, nullptr, &tv) == 1;
        ::close(fd);
        return open;
      };

      // Combined probe: TCP check → fresh cc::Client → GetServerVersion().
      // Creates a new cc::Client on each TCP-success to guarantee a live
      // rpclib socket. The successful client is promoted to inAppDriveClient.
      auto rpcReadyProbe = [&, last_err = std::string()]() mutable -> bool {
        if (!tcpOpen()) return false;
        auto candidate = std::make_shared<cc::Client>(hostStr, static_cast<uint16_t>(port));
        candidate->SetTimeout(std::chrono::seconds(2)); // GetServerVersion is sub-ms on a live server
        try {
          (void)candidate->GetServerVersion();
          inAppDriveClient = std::move(candidate);
          inAppDriveClient->SetTimeout(std::chrono::seconds(30));
          return true;
        } catch (const std::exception &e) {
          const std::string msg = e.what();
          if (msg != last_err) {
            std::cerr << "[in-app driver] rpcReady probe: " << msg << '\n';
            last_err = msg;
          }
          candidate.reset();
          return false;
        } catch (...) {
          candidate.reset();
          return false;
        }
      };

      // Probe loop: retry until probe() succeeds OR the user presses STOP.
      // max_seconds > 0 adds a hard timeout for post-load checks; 0 = unlimited.
      // Sleep is broken into 50 ms event-processing slices so the UI stays
      // responsive and STOP takes effect within ~50 ms.
      auto waitFor = [&](const char *label,
                         const std::function<bool()> &probe,
                         int max_seconds = 0) -> bool {
        const auto start = std::chrono::steady_clock::now();
        int attempt = 0;
        while (launchRequestedOrRunning) {
          if (probe()) {
            const auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count();
            std::cerr << "[in-app driver] " << label
                      << " READY after " << attempt
                      << " probes (" << elapsed_ms << " ms)\n";
            setSimulationStatus(QString("Running (%1 ready, %2 ms)")
                                  .arg(QString::fromLatin1(label))
                                  .arg(qint64(elapsed_ms)));
            return true;
          }
          ++attempt;
          const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
              std::chrono::steady_clock::now() - start).count();
          if (max_seconds > 0 && elapsed >= max_seconds) {
            std::cerr << "[in-app driver] " << label << " TIMED OUT after "
                      << elapsed << " s (" << attempt << " probes)\n";
            return false;
          }
          if (attempt % 4 == 0) {
            setSimulationStatus(QString("Running (connecting… %1 s)")
                                  .arg(qint64(elapsed)));
          }
          // 5 × 50 ms = 250 ms sleep, but processEvents each slice so STOP
          // button clicks are dispatched and launchRequestedOrRunning updates.
          for (int i = 0; i < 5 && launchRequestedOrRunning; ++i) {
            QApplication::processEvents(QEventLoop::AllEvents, 50);
          }
        }
        std::cerr << "[in-app driver] " << label << " aborted (STOP)\n";
        return false;
      };

      // Retry until STOP is pressed (max_seconds=0 → unlimited).
      if (!waitFor("rpc-ready", rpcReadyProbe)) {
        setSimulationStatus("Idle");
        return false;
      }
      // inAppDriveClient is now set by rpcReadyProbe.

      // Post-load world probe used after GenerateOpenDriveWorld / LoadWorld.
      auto worldQueryable = [&]() -> bool {
        try {
          auto world = inAppDriveClient->GetWorld();
          return true;
        } catch (...) { return false; }
      };

      // -------- Honour the Map dropdown selection ----------------------
      // "Town*" → standard CARLA umap via LoadWorld(town).
      // (The former "Basic" / "Minimal" OpenDRIVE-generated demo map has
      // been removed — the dropdown no longer offers it and any stale
      // setting that still names it falls through to a Town load.)
      const QString chosenMap = scenarioName.trimmed();
      const bool wantMinimal = false;
      if (wantMinimal) {
        QFile xodrFile(":/examples/two_lane_10mi.xodr");
        if (xodrFile.open(QIODevice::ReadOnly)) {
          const QByteArray xml = xodrFile.readAll();
          xodrFile.close();
          inAppDriveClient->SetTimeout(std::chrono::seconds(120));
          bool generated = false;
          for (int attempt = 0; attempt < 4 && !generated; ++attempt) {
            try {
              carla::rpc::OpendriveGenerationParameters params;
              params.vertex_distance = 5.0;
              params.max_road_length = 500.0;
              // 1.5 m wall at both outer shoulder edges — acts as guard-rail fence
              params.wall_height = 1.5;
              // 50 m each side of centreline gives a stable physics ground plane
              // without triggering UE5 RecastNavMesh tile-build crashes.
              params.additional_width = 50.0;
              params.smooth_junctions = true;
              params.enable_mesh_visibility = true;
              setSimulationStatus(QString("Running (loading Basic map, attempt %1/4)")
                                    .arg(attempt + 1));
              inAppDriveClient->GenerateOpenDriveWorld(xml.toStdString(), params);
              generated = true;
            } catch (const std::exception &e) {
              std::cerr << "[in-app driver] GenerateOpenDriveWorld attempt "
                        << (attempt + 1) << "/4 failed: " << e.what() << '\n';
              std::this_thread::sleep_for(std::chrono::seconds(5));
            }
          }
          inAppDriveClient->SetTimeout(std::chrono::seconds(30));
          if (generated) {
            waitFor("post-load world", worldQueryable, 60);
          } else {
            std::cerr << "[in-app driver] GenerateOpenDriveWorld gave up; "
                         "continuing on boot map\n";
          }
        }
      } else if (!chosenMap.isEmpty()) {
        // LoadWorld() picks the umap by short name (Town01, Town10HD, etc).
        inAppDriveClient->SetTimeout(std::chrono::seconds(120));
        try {
          setSimulationStatus(QString("Running (loading %1)").arg(chosenMap));
          inAppDriveClient->LoadWorld(chosenMap.toStdString());
          waitFor("post-load world", worldQueryable, 60);
        } catch (const std::exception &e) {
          std::cerr << "[in-app driver] LoadWorld(" << chosenMap.toStdString()
                    << ") failed: " << e.what() << '\n';
        }
        inAppDriveClient->SetTimeout(std::chrono::seconds(30));
      }

      auto world = inAppDriveClient->GetWorld();

      // Each step below runs on the GUI thread and can throw via the RPC.
      // Wrap each in its own try/catch with stderr logging so a failure
      // localizes instead of bubbling up as an opaque "std::exception"
      // through the outer catch (which has bitten us repeatedly with the
      // InMemoryMap cache build on generated-OpenDRIVE worlds).
      carla::SharedPtr<cc::ActorList> actors;
      try {
        std::cerr << "[in-app driver] step: GetActors\n";
        actors = world.GetActors();
      } catch (const std::exception &e) {
        std::cerr << "[in-app driver] GetActors threw: " << e.what() << '\n';
      }
      carla::SharedPtr<cc::ActorList> vehicles =
          actors ? actors->Filter("vehicle.*") : nullptr;
      if (vehicles && !vehicles->empty()) {
        inAppDriveVehicle = std::static_pointer_cast<cc::Vehicle>(vehicles->at(0u));
        inAppDriveSpawnedVehicle = false;
        std::cerr << "[in-app driver] reused existing vehicle id="
                  << inAppDriveVehicle->GetId() << '\n';
      } else {
        carla::SharedPtr<cc::BlueprintLibrary> blueprints;
        try {
          std::cerr << "[in-app driver] step: GetBlueprintLibrary\n";
          blueprints = world.GetBlueprintLibrary();
        } catch (const std::exception &e) {
          std::cerr << "[in-app driver] GetBlueprintLibrary threw: " << e.what() << '\n';
        }
        if (!blueprints) {
          setSimulationStatus("Running (in-app driver failed: no blueprint library)");
          return false;
        }
        auto vehicleBps = blueprints->Filter("vehicle.*");
        if (!vehicleBps || vehicleBps->empty()) {
          setSimulationStatus("Running (in-app driver failed: no vehicle blueprint)");
          return false;
        }

        // Spawn-point selection.
        //   • Basic map (procedural OpenDRIVE): SKIP GetRecommendedSpawnPoints()
        //     because libcarla's InMemoryMap build for our 16 km straight
        
        
        
        
        
        
        cg::Transform spawn;
        if (wantMinimal) {
          spawn = cg::Transform(cg::Location(50.0f, -1.75f, 0.5f),
                                cg::Rotation(0.0f, 0.0f, 0.0f));
          std::cerr << "[in-app driver] using hardcoded Basic spawn (50, -1.75, 0.5)\n";
        } else {
          try {
            std::cerr << "[in-app driver] step: GetMap + GetRecommendedSpawnPoints\n";
            auto map = world.GetMap();
            auto spawnPoints = map ? map->GetRecommendedSpawnPoints()
                                   : std::vector<cg::Transform>{};
            if (spawnPoints.empty()) {
              setSimulationStatus("Running (in-app driver failed: no spawn points)");
              return false;
            }
            spawn = spawnPoints.front();
          } catch (const std::exception &e) {
            std::cerr << "[in-app driver] spawn-point lookup threw: " << e.what() << '\n';
            setSimulationStatus("Running (in-app driver failed: spawn-point lookup)");
            return false;
          }
        }

        
        
        
        
        auto bp = vehicleBps->at(0u);
        if (bp.ContainsAttribute("role_name")) {
          bp.SetAttribute("role_name", "hero");
        }
        try {
          std::cerr << "[in-app driver] step: TrySpawnActor at ("
                    << spawn.location.x << ", " << spawn.location.y
                    << ", " << spawn.location.z << ")\n";
          auto actor = world.TrySpawnActor(bp, spawn);
          inAppDriveVehicle = actor ? std::static_pointer_cast<cc::Vehicle>(actor) : nullptr;
          inAppDriveSpawnedVehicle = (inAppDriveVehicle != nullptr);
          std::cerr << "[in-app driver] spawn result: "
                    << (inAppDriveVehicle ? "OK id=" : "FAILED ")
                    << (inAppDriveVehicle ? inAppDriveVehicle->GetId() : 0u) << '\n';
        } catch (const std::exception &e) {
          std::cerr << "[in-app driver] TrySpawnActor threw: " << e.what() << '\n';
        }
      }

      if (!inAppDriveVehicle) {
        setSimulationStatus("Running (in-app driver failed: vehicle unavailable)");
        return false;
      }

      inAppDriveVehicle->SetAutopilot(false);

      
      
      
      try {
        auto wt = inAppDriveClient->GetWorld();
        carla::rpc::WeatherParameters weather;
        weather.cloudiness             = 8.0f;
        weather.precipitation          = 0.0f;
        weather.precipitation_deposits = 0.0f;
        weather.wind_intensity         = 5.0f;
        weather.sun_azimuth_angle      = 270.0f;
        weather.sun_altitude_angle     = 40.0f;
        weather.fog_density            = 0.0f;
        weather.fog_distance           = 0.0f;
        weather.wetness                = 0.0f;
        wt.SetWeather(weather);
      } catch (...) {}

#ifdef CARLA_STUDIO_WITH_QT3D
      try {
        auto wt = inAppDriveClient->GetWorld();
        const QString scenarioId = qEnvironmentVariableIsSet("CARLA_STUDIO_SCENARIO")
          ? qEnvironmentVariable("CARLA_STUDIO_SCENARIO")
          : QStringLiteral("default");
        const QStringList sensors = carla_studio::calibration::listPersistedSensors(scenarioId);
        if (!sensors.isEmpty()) {
          auto bps = wt.GetBlueprintLibrary();
          const auto *coneBp = bps ? bps->Find("static.prop.constructioncone") : nullptr;
          static auto sCalibCells = std::make_shared<
            std::vector<std::tuple<cg::Location, float, carla::sensor::data::Color>>>();
          sCalibCells->clear();
          int spawnedActors = 0, drawnCells = 0;
          for (const QString &sensorName : sensors) {
            const auto targets = carla_studio::calibration::loadPersisted(scenarioId, sensorName);
            for (const auto &t : targets) {
              cg::Location base(static_cast<float>(t.x),
                                static_cast<float>(t.y),
                                static_cast<float>(t.z));
              if (t.type == carla_studio::calibration::TargetType::ConeSquare
               || t.type == carla_studio::calibration::TargetType::ConeLine
               || t.type == carla_studio::calibration::TargetType::PylonWall) {
                if (!coneBp) continue;
                std::vector<std::pair<double, double>> offs;
                if (t.type == carla_studio::calibration::TargetType::ConeSquare)
                  offs = {{-0.5,-0.5},{0.5,-0.5},{-0.5,0.5},{0.5,0.5}};
                else if (t.type == carla_studio::calibration::TargetType::ConeLine)
                  for (int i = -3; i <= 4; ++i) offs.push_back({i * 0.4, 0.0});
                else
                  for (int i = -2; i <= 2; ++i)
                    for (int j = 0; j <= 1; ++j) offs.push_back({i * 0.5, j * 0.5});
                const double yawRad = t.yawDeg * M_PI / 180.0;
                const double cy = std::cos(yawRad), sy = std::sin(yawRad);
                for (auto &o : offs) {
                  cg::Location loc(
                    base.x + static_cast<float>(o.first * cy - o.second * sy),
                    base.y + static_cast<float>(o.first * sy + o.second * cy),
                    base.z);
                  cg::Transform xf(loc, cg::Rotation(0, 0, 0));
                  try { if (wt.TrySpawnActor(*coneBp, xf)) ++spawnedActors; } catch (...) {}
                }
              } else {
                int cols = 8, rows = 6;
                std::vector<std::tuple<int,int,uint8_t,uint8_t,uint8_t>> cells;
                if (t.type == carla_studio::calibration::TargetType::Checkerboard) {
                  cols = t.cols; rows = t.rows;
                  for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
                    bool w = ((r+c)%2==0);
                    cells.push_back({c, r,
                      uint8_t(w?240:20), uint8_t(w?240:20), uint8_t(w?240:20)});
                  }
                } else if (t.type == carla_studio::calibration::TargetType::AprilTag) {
                  static const int M[8][8] = {
                    {0,0,0,0,0,0,0,0},{0,1,1,0,0,1,1,0},{0,1,0,1,0,0,1,0},
                    {0,0,1,1,0,1,1,0},{0,0,0,1,1,0,1,0},{0,1,1,0,0,1,0,0},
                    {0,0,1,1,1,1,1,0},{0,0,0,0,0,0,0,0}};
                  cols = 8; rows = 8;
                  for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c) {
                    bool w = M[r][c]!=0;
                    cells.push_back({c, r,
                      uint8_t(w?240:20), uint8_t(w?240:20), uint8_t(w?240:20)});
                  }
                } else {
                  static const int MB[4][6][3] = {
                    {{115,82,68},{194,150,130},{98,122,157},{87,108,67},
                     {133,128,177},{103,189,170}},
                    {{214,126,44},{80,91,166},{193,90,99},{94,60,108},
                     {157,188,64},{224,163,46}},
                    {{56,61,150},{70,148,73},{175,54,60},{231,199,31},
                     {187,86,149},{8,133,161}},
                    {{243,243,242},{200,200,200},{160,160,160},{122,122,121},
                     {85,85,85},{52,52,52}}};
                  cols = 6; rows = 4;
                  for (int r = 0; r < rows; ++r) for (int c = 0; c < cols; ++c)
                    cells.push_back({c, r,
                      uint8_t(MB[r][c][0]), uint8_t(MB[r][c][1]), uint8_t(MB[r][c][2])});
                }
                const double cellSize = t.sizeM / cols;
                const double yawRad = t.yawDeg * M_PI / 180.0;
                const double cy = std::cos(yawRad), sy = std::sin(yawRad);
                for (const auto &cell : cells) {
                  const double oR = (std::get<0>(cell) - cols/2.0 + 0.5) * cellSize;
                  const double oU = (rows/2.0 - 0.5 - std::get<1>(cell)) * cellSize;
                  cg::Location p(
                    base.x + static_cast<float>(oR * cy),
                    base.y + static_cast<float>(oR * sy),
                    base.z + static_cast<float>(oU));
                  carla::sensor::data::Color col{
                    std::get<2>(cell), std::get<3>(cell), std::get<4>(cell)};
                  sCalibCells->emplace_back(p, static_cast<float>(cellSize * 0.5), col);
                  ++drawnCells;
                }
              }
            }
          }
          if (!sCalibCells->empty()) {
            static QTimer *sCalibTimer = nullptr;
            if (!sCalibTimer) {
              sCalibTimer = new QTimer(&window);
              sCalibTimer->setInterval(900);
              auto clientCopy = inAppDriveClient;
              QObject::connect(sCalibTimer, &QTimer::timeout, &window,
                [clientCopy]() {
                  if (!clientCopy) return;
                  try {
                    auto dh = clientCopy->GetWorld().MakeDebugHelper();
                    for (const auto &c : *sCalibCells)
                      dh.DrawPoint(std::get<0>(c), std::get<1>(c),
                                   std::get<2>(c), 1.5f, false);
                  } catch (...) {}
                });
            }
            sCalibTimer->start();
          }
          std::cerr << "[calibration] spawned " << spawnedActors
                    << " actors, drawing " << drawnCells << " debug cells\n";
        }
      } catch (const std::exception &e) {
        std::cerr << "[calibration] auto-spawn failed: " << e.what() << '\n';
      } catch (...) {}
#endif

      
      
      
      
      
      
      
      try {
        auto wt = inAppDriveClient->GetWorld();
        auto lib = wt.GetBlueprintLibrary();
        if (lib) {
          
          
          
          auto bpContainsWord = [](const std::string &lc, const std::string &word) {
            size_t pos = lc.find(word);
            while (pos != std::string::npos) {
              const bool leftOk  = (pos == 0 || lc[pos - 1] == '.' || lc[pos - 1] == '_');
              const bool rightOk = (pos + word.size() >= lc.size()
                                    || lc[pos + word.size()] == '.'
                                    || lc[pos + word.size()] == '_');
              if (leftOk && rightOk) return true;
              pos = lc.find(word, pos + 1);
            }
            return false;
          };

          std::string palmBpId, cherryBpId, fallbackTreeId;
          for (const auto &bp : *lib) {
            const std::string id = bp.GetId();
            auto lc = id;
            std::transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
            if (palmBpId.empty() && (bpContainsWord(lc, "palm")
                                     || bpContainsWord(lc, "date"))) {
              palmBpId = id;
            }
            if (cherryBpId.empty() && (bpContainsWord(lc, "cherry")
                                       || bpContainsWord(lc, "jacaranda")
                                       || bpContainsWord(lc, "japanese")
                                       || bpContainsWord(lc, "orchard"))) {
              cherryBpId = id;
            }
            
            
            if (fallbackTreeId.empty() && !bpContainsWord(lc, "palm")
                && (bpContainsWord(lc, "tree") || bpContainsWord(lc, "bush")
                    || bpContainsWord(lc, "pine") || bpContainsWord(lc, "oak")
                    || bpContainsWord(lc, "vegetation"))) {
              fallbackTreeId = id;
            }
          }
          if (cherryBpId.empty()) cherryBpId = fallbackTreeId;

          std::cerr << "[in-app driver] tree blueprints: palm=\"" << palmBpId
                    << "\" cherry=\"" << cherryBpId << "\"\n";

          
          const float interval  = 804.0f;
          const float roadLen   = 16093.0f;
          const float offsetY   = 10.0f;  
          int spawned = 0;
          for (float s = 250.0f; s < roadLen && spawned < 60; s += interval) {
            
            if (!palmBpId.empty()) {
              auto bp = lib->Find(palmBpId);
              if (bp) {
                float yaw = std::fmod(s * 37.1f, 360.0f);
                cg::Transform t(cg::Location(s, offsetY, 0.0f),
                                cg::Rotation(0.0f, yaw, 0.0f));
                wt.TrySpawnActor(*bp, t);
                ++spawned;
              }
            }
            
            if (!cherryBpId.empty()) {
              auto bp = lib->Find(cherryBpId);
              if (bp) {
                float yaw = std::fmod(s * 53.7f, 360.0f);
                cg::Transform t(cg::Location(s, -offsetY, 0.0f),
                                cg::Rotation(0.0f, yaw, 0.0f));
                wt.TrySpawnActor(*bp, t);
                ++spawned;
              }
            }
          }
          std::cerr << "[in-app driver] spawned " << spawned << " roadside trees\n";
        }
      } catch (const std::exception &e) {
        std::cerr << "[in-app driver] tree spawn: " << e.what() << '\n';
      } catch (...) {}

      
      
      
      
      
      try {
        auto wt = inAppDriveClient->GetWorld();
        auto dbg = wt.MakeDebugHelper();
        using C = cc::DebugHelper::Color;
        const C white{255, 255, 255, 255};
        const C yellow{255, 210, 0,   255};

        
        dbg.DrawString(cg::Location(125.0f,  0.0f, 1.8f), "CARLA",
                       true, yellow, -1.0f, true);
        
        dbg.DrawString(cg::Location(98.0f,   0.0f, 0.8f), "START",
                       true, white, -1.0f, true);

        
        const float rHW = 5.5f;  
        dbg.DrawLine(cg::Location(92.0f, -rHW, 0.12f),
                     cg::Location(92.0f,  rHW, 0.12f),
                     0.30f, white, -1.0f, true);

        
        
        for (int xi = 0; xi < 4; ++xi) {
          for (int yi = 0; yi < 11; ++yi) {
            if ((xi + yi) % 2 != 0) continue;   
            const float x0 = 88.0f + xi * 1.0f;
            const float y0 = -5.5f + yi * 1.0f;
            for (float dy = 0.0f; dy <= 1.0f; dy += 0.12f) {
              dbg.DrawLine(cg::Location(x0,        y0 + dy, 0.12f),
                           cg::Location(x0 + 1.0f, y0 + dy, 0.12f),
                           0.07f, white, -1.0f, true);
            }
          }
        }
        std::cerr << "[in-app driver] start-line decoration drawn\n";
      } catch (...) {}

      
      
      
      
      try {
        auto wt = inAppDriveClient->GetWorld();
        auto spec = wt.GetSpectator();
        const auto vt = inAppDriveVehicle->GetTransform();
        const auto fwd = vt.GetForwardVector();
        cg::Location camLoc(vt.location.x - 6.0f * fwd.x,
                            vt.location.y - 6.0f * fwd.y,
                            vt.location.z + 2.5f);
        cg::Rotation camRot(-12.0f, vt.rotation.yaw, 0.0f);
        spec->SetTransform(cg::Transform(camLoc, camRot));
      } catch (...) {
      }

      inAppDriverControlActive = true;
      driveKeys = {};
      inAppDriveTick->start();
      inAppCameraTick->start();
      updateThirdPersonCamera();
      setSimulationStatus(QString("Running (in-app LibCarla driver active, %1)")
                          .arg(driverViewMode->currentText().toLower()));
      return true;
    } catch (const std::exception &e) {
      setSimulationStatus(QString("Running (in-app driver error: %1)").arg(QString::fromLocal8Bit(e.what())));
      stopInAppDriver();
      return false;
    }
  };

  
  
  
  QObject::connect(inAppDriveTick, &QTimer::timeout, &window, [&]() {
    try {
      applyInAppVehicleControl();
    } catch (const std::exception &e) {
      std::cerr << "[in-app driver tick] " << e.what() << '\n';
    } catch (...) {
      std::cerr << "[in-app driver tick] unknown exception\n";
    }
  });
  QObject::connect(inAppCameraTick, &QTimer::timeout, &window, [&]() {
    try {
      updateThirdPersonCamera();
    } catch (const std::exception &e) {
      std::cerr << "[in-app camera tick] " << e.what() << '\n';
    } catch (...) {
      std::cerr << "[in-app camera tick] unknown exception\n";
    }
  });
#else
  auto stopInAppDriver = [&]() {
    inAppDriverControlActive = false;
    driveKeys = {};
    inAppDriveTick->stop();
    inAppCameraTick->stop();
  };
#endif

  QObject::connect(driverViewMode, &QComboBox::currentTextChanged, &window,
    [&, fpvBtn, tpvBtn, bevBtn](const QString &mode) {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
      if (inAppDriverControlActive) {
        updateThirdPersonCamera();
      }
#endif
      if      (mode == "First Person") { QSignalBlocker b(fpvBtn); fpvBtn->setChecked(true); }
      else if (mode == "Cockpit")      { QSignalBlocker b(cpvBtn); cpvBtn->setChecked(true); }
      else if (mode == "Bird Eye")     { QSignalBlocker b(bevBtn); bevBtn->setChecked(true); }
      else                             { QSignalBlocker b(tpvBtn); tpvBtn->setChecked(true); }
    });

  std::function<void(const QString &)> openViewTile;
  QObject::connect(fpvBtn, &QPushButton::clicked, &window, [&]() {
    if (openViewTile) openViewTile("FPV");
  });
  QObject::connect(tpvBtn, &QPushButton::clicked, &window, [&]() {
    if (openViewTile) openViewTile("TPV");
  });
  QObject::connect(cpvBtn, &QPushButton::clicked, &window, [&]() {
    if (openViewTile) openViewTile("CPV");
  });
  QObject::connect(bevBtn, &QPushButton::clicked, &window, [&]() {
    if (openViewTile) openViewTile("BEV");
  });

  openViewTile = [&](const QString &mode) {
    if (!viewDock || !viewGridLayout) return;
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    
    if (viewTiles.contains(mode)) {
      viewDock->show();
      viewDock->raise();
      return;
    }

    const QString host = targetHost->text().trimmed().isEmpty()
                          ? QStringLiteral("localhost")
                          : targetHost->text().trimmed();
    const int port = portSpin->value();
    std::shared_ptr<cc::Client> c;
    try {
      c = std::make_shared<cc::Client>(host.toStdString(), static_cast<uint16_t>(port));
      c->SetTimeout(std::chrono::seconds(4));
      (void)c->GetClientVersion();
    } catch (...) {
      QMessageBox::warning(&window, "CARLA unreachable",
        QString("Could not reach %1:%2 — is the sim running?").arg(host).arg(port));
      return;
    }
    try {
      auto world = c->GetWorld();
      carla::SharedPtr<cc::Actor> hero;
      
      if (inAppDriveVehicle) {
        hero = inAppDriveVehicle;
      } else {
        auto actorList = world.GetActors();
        auto vehicleList = actorList ? actorList->Filter("vehicle.*") : nullptr;
        if (vehicleList) {
          for (auto a : *vehicleList) {
            if (!a) continue;
            for (const auto &attr : a->GetAttributes()) {
              if (attr.GetId() == "role_name" && attr.GetValue() == "hero") { hero = a; break; }
            }
            if (hero) break;
          }
          if (!hero && !vehicleList->empty()) hero = (*vehicleList)[0];
        }
      }
      if (!hero) {
        QMessageBox::warning(&window, "No vehicle",
          "No hero vehicle in the world yet. Press START first and wait for "
          "the in-app driver to spawn one (~4 s).");
        return;
      }

      auto bps = world.GetBlueprintLibrary();
      const auto *rgb = bps ? bps->Find("sensor.camera.rgb") : nullptr;
      if (!rgb) { QMessageBox::warning(&window, "Blueprint missing", "sensor.camera.rgb not found"); return; }
      auto bpCopy = *rgb;
      auto trySet = [&](const char *k, const QString &v) {
        if (bpCopy.ContainsAttribute(k)) bpCopy.SetAttribute(k, v.toStdString());
      };

      int imgW = 640, imgH = 360;
      QString modeFov = "90";
      QString tileTitle;
      cg::Transform tr;

      if (mode == "FPV") {
        tileTitle = "Driver (FPV)";
        tr.location = cg::Location(0.5f, 0.0f, 1.4f);
        tr.rotation = cg::Rotation(-2.0f, 0.0f, 0.0f);
      } else if (mode == "CPV") {
        tileTitle = "Cockpit";
        tr.location = cg::Location(0.0f, -0.32f, 1.05f);
        tr.rotation = cg::Rotation(-8.0f, 0.0f, 0.0f);
        modeFov = "115";
        imgW = 960; imgH = 300;
      } else if (mode == "BEV") {
        tileTitle = "Bird Eye";
        tr.location = cg::Location(0.0f, 0.0f, 22.0f);
        tr.rotation = cg::Rotation(-89.5f, 0.0f, 0.0f);
      } else { 
        tileTitle = "Chase (TPV)";
        tr.location = cg::Location(-6.0f, 0.0f, 2.6f);
        tr.rotation = cg::Rotation(-12.0f, 0.0f, 0.0f);
      }

      trySet("image_size_x", QString::number(imgW));
      trySet("image_size_y", QString::number(imgH));
      trySet("fov", modeFov);

      carla::SharedPtr<cc::Actor> rawActor;
      try {
        rawActor = world.SpawnActor(bpCopy, tr, hero.get());
      } catch (const std::exception &e) {
        QMessageBox::warning(&window, "Spawn failed",
          QString("Couldn't spawn %1 camera: %2").arg(mode, e.what()));
        return;
      }
      auto camera = std::static_pointer_cast<cc::Sensor>(rawActor);
      if (!camera) return;

      
      QGroupBox *tile = new QGroupBox(tileTitle);
      QVBoxLayout *tileLayout = new QVBoxLayout(tile);
      tileLayout->setContentsMargins(4, 4, 4, 4);
      tileLayout->setSpacing(4);

      QLabel *imgLabel = new QLabel();
      imgLabel->setAlignment(Qt::AlignCenter);
      imgLabel->setMinimumHeight(180);
      imgLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      imgLabel->setStyleSheet("background-color: black; color: #888;");
      imgLabel->setText("Connecting…");
      tileLayout->addWidget(imgLabel, 1);

      QPushButton *closeBtn = new QPushButton("Close");
      closeBtn->setFixedHeight(22);
      tileLayout->addWidget(closeBtn);

      auto labelPtr = QPointer<QLabel>(imgLabel);
      camera->Listen([labelPtr](carla::SharedPtr<carla::sensor::SensorData> data) {
        auto img = std::static_pointer_cast<csd::Image>(data);
        if (!img) return;
        const int w = static_cast<int>(img->GetWidth());
        const int h = static_cast<int>(img->GetHeight());
        const auto *raw = reinterpret_cast<const unsigned char *>(img->data());
        QImage src(raw, w, h, QImage::Format_ARGB32);
        QImage frame = src.rgbSwapped().copy();
        QMetaObject::invokeMethod(labelPtr.data(), [labelPtr, frame]() {
          if (!labelPtr) return;
          labelPtr->setPixmap(QPixmap::fromImage(frame).scaled(
            labelPtr->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }, Qt::QueuedConnection);
      });

      activeViewSensors.insert(mode, camera);
      viewTiles.insert(mode, tile);

      
      auto setViewBtn = [&](const QString &m, bool on) {
        if      (m == "FPV") { QSignalBlocker b(fpvBtn); fpvBtn->setChecked(on); }
        else if (m == "TPV") { QSignalBlocker b(tpvBtn); tpvBtn->setChecked(on); }
        else if (m == "CPV") { QSignalBlocker b(cpvBtn); cpvBtn->setChecked(on); }
        else if (m == "BEV") { QSignalBlocker b(bevBtn); bevBtn->setChecked(on); }
      };
      setViewBtn(mode, true);

      QObject::connect(closeBtn, &QPushButton::clicked, tile,
        [&, mode, camera, tile]() mutable {
          try { camera->Stop(); camera->Destroy(); } catch (...) {}
          activeViewSensors.remove(mode);
          viewTiles.remove(mode);
          if      (mode == "FPV") { QSignalBlocker b(fpvBtn); fpvBtn->setChecked(false); }
          else if (mode == "TPV") { QSignalBlocker b(tpvBtn); tpvBtn->setChecked(false); }
          else if (mode == "CPV") { QSignalBlocker b(cpvBtn); cpvBtn->setChecked(false); }
          else if (mode == "BEV") { QSignalBlocker b(bevBtn); bevBtn->setChecked(false); }
          tile->deleteLater();
          if (viewTiles.isEmpty()) viewDock->hide();
        });

      const int stretchIdx = viewGridLayout->count() - 1;
      viewGridLayout->insertWidget(stretchIdx, tile);

      viewDock->show();
      viewDock->raise();
    } catch (const std::exception &e) {
      QMessageBox::warning(&window, "CARLA error",
        QString("Couldn't open %1 view: %2").arg(mode, e.what()));
    } catch (...) {
      QMessageBox::warning(&window, "CARLA error",
        QString("Couldn't open %1 view (unknown error).").arg(mode));
    }
#else
    Q_UNUSED(mode);
    QMessageBox::warning(&window, "LibCarla not linked", "This build cannot reach CARLA.");
#endif
  };

  auto launchPythonDriverClient = [&](const QString &rootPath, const QString &host, int port, const QString &scenarioName) {
    const QString manualControlPath = rootPath + "/PythonAPI/examples/manual_control.py";
    if (!QFileInfo(manualControlPath).isFile()) {
      setSimulationStatus("Running (manual_control.py not found)");
      return;
    }

    const QString configPath = rootPath + "/PythonAPI/util/config.py";
    if (QFileInfo(configPath).isFile() && !scenarioName.isEmpty()) {
      QProcess configureWorld;
      configureWorld.start(
        "/bin/bash",
        QStringList() << "-lc"
                      << QString("python3 \"%1\" --host=%2 --port=%3 --map=%4")
                            .arg(configPath)
                            .arg(host)
                            .arg(port)
                            .arg(scenarioName));
      configureWorld.waitForFinished(7000);
    }

    QProcess manual;
    manual.start(
      "/bin/bash",
      QStringList() << "-lc"
                    << QString("python3 \"%1\" --host=%2 --port=%3 >/tmp/carla_studio_manual_control.log 2>&1 & echo $!")
                          .arg(manualControlPath)
                          .arg(host)
                          .arg(port));
    manual.waitForFinished(8000);
    bool ok = false;
    const qint64 manualPid = QString::fromLocal8Bit(manual.readAllStandardOutput()).trimmed().toLongLong(&ok);
    if (ok) {
      rememberPid(manualPid);
      setSimulationStatus("Running (python manual_control started)");
    } else {
      setSimulationStatus("Running (failed to start python manual_control)");
    }
  };

  auto launchSelectedDriverMode = [&](const QString &rootPath, const QString &host, int port, const QString &scenarioName) {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    if (optDriverInApp->isChecked()) {
      const bool started = startInAppDriver(host, port, scenarioName);
      if (started) {
        return;
      }
    }
#endif

    if (optDriverPython->isChecked()) {
      launchPythonDriverClient(rootPath, host, port, scenarioName);
      return;
    }

    setSimulationStatus("Running (no driver mode selected)");
  };

  QObject::connect(targetHost, &QLineEdit::textChanged, &window, [&]() { updateEndpoint(); });
  QObject::connect(portSpin, qOverload<int>(&QSpinBox::valueChanged), &window, [&](int) { updateEndpoint(); });
  QObject::connect(refreshScenarioBtn, &QPushButton::clicked, &window, [&]() {
    populateScenarios();
  });

  QObject::connect(runtimeTarget, &QComboBox::currentTextChanged, &window, [&](const QString &text) {
    if (text == "Local") {
      targetHost->setText("localhost");
    } else if (text.endsWith("(remote)")) {
      targetHost->setText(text.left(text.size() - QString(" (remote)").size()));
    }
    updateEndpoint();
  });

  QObject::connect(carlaRootPath, &QLineEdit::editingFinished, &window, [&]() { validateCarlaRoot(); });

  QObject::connect(refreshProcBtn, &QPushButton::clicked, &window, [&]() {
    discoverCarlaSimPids();
    refreshProcessList();
  });

  QObject::connect(scenarioSelect, &QComboBox::currentTextChanged, &window,
    [&](const QString &mapName) {
      if (!launchRequestedOrRunning || mapName.isEmpty()) {
        return;
      }
      const QString rootPath = carlaRootPath->text().trimmed();
      if (rootPath.isEmpty()) {
        return;
      }
      const QString configPath = rootPath + "/PythonAPI/util/config.py";
      if (!QFileInfo(configPath).isFile()) {
        setSimulationStatus(QString("Running (cannot switch to %1: config.py missing)").arg(mapName));
        return;
      }
      const QString host = targetHost->text().trimmed().isEmpty()
                             ? QStringLiteral("localhost")
                             : targetHost->text().trimmed();
      const int port = portSpin->value();
      setSimulationStatus(QString("Running (loading %1...)").arg(mapName));

      auto *switcher = new QProcess(&window);
      switcher->setProgram("/bin/bash");
      switcher->setArguments(QStringList() << "-lc"
        << QString("python3 \"%1\" --host=%2 --port=%3 --map=%4")
            .arg(configPath).arg(host).arg(port).arg(mapName));
      QObject::connect(switcher,
        QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        &window, [&, switcher, mapName](int exitCode, QProcess::ExitStatus status) {
          const bool ok = status == QProcess::NormalExit && exitCode == 0;
          setSimulationStatus(ok
            ? QString("Running (%1 loaded)").arg(mapName)
            : QString("Running (failed to load %1)").arg(mapName));
          switcher->deleteLater();
        });
      switcher->start();
    });

  QObject::connect(stopBtn, &QPushButton::clicked, &window, [&]() {
    stopInAppDriver();
    killTrackedPids();
    killAllCarlaProcesses();
    
    
    if (!g_dockerContainerId.isEmpty()) {
      QProcess::startDetached("docker",
        QStringList() << "stop" << g_dockerContainerId.left(12));
      g_dockerContainerId.clear();
    }
    
    forceStopTimer->start(5000);
    refreshProcessList();
    setSimulationStatus("Stopped");
  });

  QObject::connect(startBtn, &QPushButton::clicked, &window, [&]() {
    if (launchRequestedOrRunning) {
      return;
    }
    driverViewMode->setCurrentText("Third Person");
    setSimulationStatus("Initializing");

    const QString target = targetHost->text().trimmed().isEmpty() ? "localhost" : targetHost->text().trimmed();
    const QString scenarioName = scenarioSelect->currentText();
    const QString runtimeName = runtimeTarget->currentText();
    const int launchPort = portSpin->value();
    updateEndpoint();

    if (runtimeName == "localhost" || runtimeName == "Local") {
      if (!validateCarlaRoot()) {
        setSimulationStatus("Idle");
        return;
      }
      const QString rootPath = carlaRootPath->text().trimmed();
      
      
      QString scriptPath = rootPath + "/CarlaUnreal.sh";
      if (!QFileInfo(scriptPath).isFile()) {
        scriptPath = rootPath + "/CarlaUE5.sh";
      }
      if (!QFileInfo(scriptPath).isFile()) {
        scriptPath = rootPath + "/CarlaUE4.sh";
      }

      
      
      
      
      bool carlaAlreadyRunning = false;
      {
        QProcess probe;
        probe.start("/bin/bash", QStringList() << "-lc"
            << QString("ss -ltn 2>/dev/null | grep -q ':%1 '").arg(launchPort));
        probe.waitForFinished(1500);
        carlaAlreadyRunning = (probe.exitCode() == 0);
      }

      
      
      
      
      qint64 launchDelayMs = 1500;
      if (carlaAlreadyRunning) {
        setSimulationStatus(QString("Running (attaching to existing CARLA on port %1)")
                              .arg(launchPort));
        launchDelayMs = 100;   
      } else {
        QStringList modeArgs;
        
        
        
        
        
        
        
        
        
        
        
        
        if (scriptPath.endsWith("/CarlaUnreal.sh")) {
          QFile::remove(QDir::homePath() +
                        "/.config/Epic/CarlaUnreal/Saved/Config/Linux/Engine.ini");
        }
        modeArgs << getSelectedLaunchArgs();
        
        
        modeArgs << QString("-carla-rpc-port=%1").arg(launchPort)
                 << QString("-carla-streaming-port=%1").arg(launchPort + 1);

        const QString cmd = QString("\"%1\" %2 >/tmp/carla_studio_launch.log 2>&1 & echo $!")
                              .arg(scriptPath)
                              .arg(modeArgs.join(' '));
        QProcess shell;
        shell.start("/bin/bash", QStringList() << "-lc" << cmd);
        if (!shell.waitForFinished(15000)) {
          shell.terminate();
          if (!shell.waitForFinished(1500)) {
            shell.kill();
            shell.waitForFinished(-1);
          }
        }
        const QString pidText = QString::fromLocal8Bit(shell.readAllStandardOutput()).trimmed();
        bool ok = false;
        const qint64 pid = pidText.toLongLong(&ok);
        if (ok) {
          rememberPid(pid);
          // Watchdog: if the simulator process exits within 30 s of START, the
          // launch crashed before opening port 2000. Otherwise Studio's poll
          // loop sits forever on "starting simulator …". Tail the launch log
          // and surface the last few lines so the user sees the actual error
          // (e.g. RenderThread timeout / SIGSEGV from a GPU driver hang).
          QTimer::singleShot(30000, &window, [pid, &window, &setSimulationStatus]() {
            if (carla_studio_proc::pidIsAlive(pid)) return;   // sim still alive — fine
            QProcess tailProc;
            tailProc.start("/bin/bash", QStringList() << "-lc"
                << "tail -n 25 /tmp/carla_studio_launch.log 2>/dev/null");
            tailProc.waitForFinished(2000);
            const QString tail = QString::fromLocal8Bit(
                tailProc.readAllStandardOutput()).trimmed();
            // Reset state so START is clickable again (otherwise Studio sits
            // forever on "Initializing"/"Running" and the user can't retry).
            setSimulationStatus("Idle");
            QMessageBox box(QMessageBox::Critical, "CARLA simulator crashed at startup",
                QString("Process %1 exited within 30 s of START.\n"
                        "Tail of /tmp/carla_studio_launch.log:").arg(pid),
                QMessageBox::Ok, &window);
            box.setDetailedText(tail.isEmpty() ? "(launch log empty)" : tail);
            box.exec();
          });
        }
      }

      
      
      
      QTimer::singleShot(launchDelayMs, &window, [=, &window]() {
        launchSelectedDriverMode(rootPath, target, launchPort, scenarioName);
      });
      
      
      if (!optRenderOffscreen->isChecked()) {
        fitCarlaWindowToSafeSize();
      }
    } else if (runtimeName.startsWith("container") || runtimeName.startsWith("Docker")) {
      QSettings s;
      const QString image    = s.value("docker/image", "carlasim/carla:0.9.16").toString();
      const bool    gpuMode  = s.value("docker/gpu", true).toBool();
      const QString network  = s.value("docker/network", "host").toString();
      const QString extraArgs = s.value("docker/extra_args", "").toString();
      const QString cName    = s.value("docker/container_name", "carla-studio-sim").toString();

      QStringList args;
      args << "run" << "-d" << "--rm";
      if (gpuMode) args << "--gpus" << "all";
      args << QString("--network=%1").arg(network);
      const QByteArray displayEnv = qgetenv("DISPLAY");
      if (!displayEnv.isEmpty()) {
        args << "-e" << QString("DISPLAY=%1").arg(QString::fromLocal8Bit(displayEnv));
        args << "-v" << "/tmp/.X11-unix:/tmp/.X11-unix";
      }
      if (network != "host") {
        args << "-p" << QString("%1:%1").arg(launchPort);
      }
      args << "--name" << cName;
      const QStringList extraParts = QProcess::splitCommand(extraArgs);
      args << extraParts;
      args << image;
      args << "/bin/bash" << "-lc"
           << QString("if [ -f ./CarlaUnreal.sh ]; then ./CarlaUnreal.sh %1 -carla-world-port=%2; "
                      "elif [ -f ./CarlaUE5.sh ]; then ./CarlaUE5.sh %1 -carla-world-port=%2; "
                      "else ./CarlaUE4.sh %1 -carla-world-port=%2; fi")
                .arg(getSelectedLaunchArgs().join(' '))
                .arg(launchPort);

      QProcess docker;
      docker.start("docker", args);
      docker.waitForFinished(20000);
      if (docker.exitStatus() != QProcess::NormalExit || docker.exitCode() != 0) {
        const QString err = QString::fromLocal8Bit(docker.readAllStandardError()).trimmed();
        QMessageBox::warning(&window, "Docker launch failed",
          QString("docker run exited %1.\n\n%2\n\nCheck Cfg → Docker Settings.")
            .arg(docker.exitCode()).arg(err));
        setSimulationStatus("Idle");
        return;
      }
      g_dockerContainerId = QString::fromLocal8Bit(docker.readAllStandardOutput()).trimmed();
      const QString shortId = g_dockerContainerId.left(12);
      QFile log("/tmp/carla_studio_launch.log");
      if (log.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream(&log) << "docker run -> " << g_dockerContainerId << "\n";
      }
      setSimulationStatus(QString("Initializing (docker %1)").arg(shortId));
    } else {
    }

    QTimer::singleShot(1500, &window, [=, &window]() {
      int detectedPort = scanCarlaPort();
      if (detectedPort > 0) {
        portSpin->setValue(detectedPort);
      }
      setSimulationStatus("Running");
      updateEndpoint();
      discoverCarlaSimPids();
      refreshProcessList();
    });
  });

  enum class InstallerMode { InstallSdk, InstallAdditionalMaps };

  struct ReleaseAsset {
    QString name;
    QString downloadUrl;
    qint64  size = 0;
  };
  struct ReleaseInfo {
    QString tag;
    QString name;
    QString publishedAt;
    QString body;
    QList<ReleaseAsset> assets;
  };
  static QList<ReleaseInfo> g_cachedReleases;

  auto detectInstalledCarlaVersion = [&]() -> QString {
    const QString root = carlaRootPath->text().trimmed();
    if (root.isEmpty() || !rootConfiguredOk) return QString();
    const QStringList probes = {root + "/VERSION", root + "/version.txt"};
    for (const QString &p : probes) {
      QFile f(p);
      if (f.open(QIODevice::ReadOnly)) {
        const QString v = QString::fromLocal8Bit(f.readLine()).trimmed();
        if (!v.isEmpty()) return v;
      }
    }
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    return QString::fromLatin1(carla::version());
#else
    return QString();
#endif
  };

  auto fetchCarlaReleases = [&](QPlainTextEdit *log) -> bool {
    if (!g_cachedReleases.isEmpty()) {
      if (log) log->appendPlainText("Using cached release list.");
      return true;
    }
    if (log) log->appendPlainText("Fetching CARLA release index from GitHub…");
    QStringList args;
    args << "-fsSL"
         << "-H" << "Accept: application/vnd.github+json";
    const QString gh_token = QString::fromLocal8Bit(qgetenv("GITHUB_TOKEN")).trimmed();
    if (!gh_token.isEmpty()) {
      args << "-H" << QStringLiteral("Authorization: Bearer %1").arg(gh_token);
    }
    args << "https://api.github.com/repos/carla-simulator/carla/releases?per_page=60";
    QProcess curl;
    curl.start("curl", args);
    if (!curl.waitForFinished(20000)) {
      curl.kill();
      curl.waitForFinished(1500);
      if (log) log->appendPlainText("✗ Release fetch timed out.");
      return false;
    }
    if (curl.exitStatus() != QProcess::NormalExit || curl.exitCode() != 0) {
      if (log) log->appendPlainText(QString("✗ Release fetch failed (exit=%1). %2")
                                      .arg(curl.exitCode())
                                      .arg(QString::fromLocal8Bit(curl.readAllStandardError())));
      return false;
    }
    QJsonParseError err;
    const auto doc = QJsonDocument::fromJson(curl.readAllStandardOutput(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isArray()) {
      if (log) log->appendPlainText(QString("✗ Release JSON parse failed: %1").arg(err.errorString()));
      return false;
    }
    g_cachedReleases.clear();
    for (const QJsonValue &v : doc.array()) {
      if (!v.isObject()) continue;
      const auto o = v.toObject();
      ReleaseInfo r;
      r.tag         = o.value("tag_name").toString();
      r.name        = o.value("name").toString();
      if (r.name.isEmpty()) r.name = r.tag;
      r.publishedAt = o.value("published_at").toString().left(10);
      r.body        = o.value("body").toString();
      for (const QJsonValue &av : o.value("assets").toArray()) {
        if (!av.isObject()) continue;
        const auto ao = av.toObject();
        ReleaseAsset a;
        a.name        = ao.value("name").toString();
        a.downloadUrl = ao.value("browser_download_url").toString();
        a.size        = qint64(ao.value("size").toDouble());
        if (!a.downloadUrl.isEmpty()) r.assets.append(a);
      }
      QSet<QString> seen;
      for (const auto &a : r.assets) seen.insert(a.name.toLower());
      static const QRegularExpression kMdLink(
        QStringLiteral("\\[([^\\]]+)\\]\\(([^)\\s]+)\\)"));
      auto linkIt = kMdLink.globalMatch(r.body);
      while (linkIt.hasNext()) {
        auto m = linkIt.next();
        const QString name = m.captured(1).trimmed();
        const QString url  = m.captured(2).trimmed();
        const QString lower = name.toLower();
        const bool archiveLike =
          lower.endsWith(".tar.gz") || lower.endsWith(".tgz") ||
          lower.endsWith(".zip")    || lower.endsWith(".tar.xz");
        if (!archiveLike) continue;
        if (seen.contains(lower)) continue;
        seen.insert(lower);
        ReleaseAsset a;
        a.name = name;
        a.downloadUrl = url;
        a.size = 0;
        r.assets.append(a);
      }
      if (!r.tag.isEmpty()) g_cachedReleases.append(r);
    }
    if (log) log->appendPlainText(QString("✓ Found %1 releases.").arg(g_cachedReleases.size()));
    return !g_cachedReleases.isEmpty();
  };

  auto pickAssetForMode = [&](const ReleaseInfo &r, InstallerMode mode) -> ReleaseAsset {
    const bool wantMaps = (mode == InstallerMode::InstallAdditionalMaps);
#if defined(Q_OS_WIN)
    const QString nativeExt = QStringLiteral(".zip");
    const QString otherExt  = QStringLiteral(".tar.gz");
#else
    const QString nativeExt = QStringLiteral(".tar.gz");
    const QString otherExt  = QStringLiteral(".zip");
#endif
    auto nameMatchesMode = [&](const QString &lower) -> bool {
      if (wantMaps) {
        if (lower.startsWith("additionalmaps")) return true;
        static const QRegularExpression kTown(
          QStringLiteral("^town\\d+_.*"));
        return kTown.match(lower).hasMatch();
      }
      return lower.startsWith("carla_") && !lower.startsWith("additionalmaps");
    };
    for (const auto &a : r.assets) {
      const QString lower = a.name.toLower();
      if (nameMatchesMode(lower) && lower.endsWith(nativeExt)) return a;
    }
    for (const auto &a : r.assets) {
      const QString lower = a.name.toLower();
      if (nameMatchesMode(lower) && lower.endsWith(otherExt)) return a;
    }
    return {};
  };

  auto isSourceBuild = [&]() -> bool {
    const QString root = carlaRootPath->text().trimmed();
    if (root.isEmpty()) return false;
    return QFileInfo(root + "/Unreal/CarlaUnreal/Source").isDir()
        || QFileInfo(root + "/Unreal/CarlaUE4/Source").isDir()
        || QFileInfo(root + "/Unreal/CarlaUE5/Source").isDir();
  };

  std::function<void(InstallerMode)> openInstallerDialog;
  std::function<void()> switchToScenarioBuilderTab;
  std::function<void()> openCleanupDialog;
  std::function<void()> openDockerSettingsDialog;
  openInstallerDialog = [&](InstallerMode mode) {
    QDialog *dlg = new QDialog(&window);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setWindowTitle(mode == InstallerMode::InstallSdk
                          ? "CARLA · Setup Wizard"
                          : "CARLA · Load Additional Maps");
    dlg->resize(720, 560);
    QVBoxLayout *root = new QVBoxLayout(dlg);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(8);

    QLabel *intro = new QLabel();
    intro->setWordWrap(true);
    if (mode == InstallerMode::InstallSdk) {
      intro->setText(
        "<b>Install CARLA</b><br>"
        "Pick a release below and choose where to extract it. The Studio will "
        "point <code>CARLA_ROOT</code> at the chosen directory after a "
        "successful install. Click <i>Show changelog</i> to view release "
        "notes before you commit to a download.");
    } else {
      const QString detectedVer = detectInstalledCarlaVersion();
      intro->setText(QString(
        "<b>Load Additional Maps</b><br>"
        "Detected CARLA version in <code>%1</code>: <b>%2</b>. The matching "
        "<code>AdditionalMaps_%2.tar.gz</code> asset (when available) will "
        "be extracted on top of your existing root, exactly as the "
        "<a href='https://carla.readthedocs.io/en/latest/tuto_M_add_map_source/#map-ingestion'>"
        "map ingestion docs</a> describe.%3")
        .arg(carlaRootPath->text().trimmed().toHtmlEscaped(),
             detectedVer.isEmpty() ? "unknown" : detectedVer.toHtmlEscaped(),
             isSourceBuild()
               ? "<br><span style='color:#B26A00;'>This looks like a "
                 "source build — for non-stock maps you may also need to "
                 "follow the digestion / cooking steps in the docs.</span>"
               : ""));
    }
    intro->setTextInteractionFlags(Qt::TextBrowserInteraction);
    intro->setOpenExternalLinks(true);
    root->addWidget(intro);

    QHBoxLayout *targetRow = new QHBoxLayout();
    QLineEdit *targetPath = new QLineEdit();
    if (mode == InstallerMode::InstallSdk) {
      targetPath->setPlaceholderText("/path/to/install/CARLA_<version>");
      const QString fallback = QDir::homePath() + "/carla";
      targetPath->setText(fallback);
    } else {
      targetPath->setText(carlaRootPath->text().trimmed());
      targetPath->setReadOnly(true);
    }
    QPushButton *browseBtn = new QPushButton("Browse…");
    QObject::connect(browseBtn, &QPushButton::clicked, dlg, [=]() {
      const QString chosen = QFileDialog::getExistingDirectory(
        dlg, "Choose CARLA install directory", targetPath->text());
      if (!chosen.isEmpty()) targetPath->setText(chosen);
    });
    targetRow->addWidget(new QLabel(mode == InstallerMode::InstallSdk
                                      ? "Install to:"
                                      : "Target root:"));
    targetRow->addWidget(targetPath, 1);
    if (mode == InstallerMode::InstallSdk) targetRow->addWidget(browseBtn);
    root->addLayout(targetRow);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, dlg);
    QListWidget *releasesList = new QListWidget(splitter);
    releasesList->setMinimumWidth(200);
    QTextBrowser *changelogView = new QTextBrowser(splitter);
    changelogView->setOpenExternalLinks(true);
    changelogView->setPlaceholderText("Select a release to view its changelog.");
    splitter->addWidget(releasesList);
    splitter->addWidget(changelogView);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    root->addWidget(splitter, 1);

    QPlainTextEdit *log = new QPlainTextEdit(dlg);
    log->setReadOnly(true);
    log->setMaximumBlockCount(2000);
    log->setMaximumHeight(140);
    log->setStyleSheet("QPlainTextEdit { font-family: monospace; font-size: 9pt; }");
    root->addWidget(log);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *refreshBtn  = new QPushButton("Refresh list");
    QPushButton *changelogBtn = new QPushButton("Show changelog");
    QPushButton *installBtn  = new QPushButton(mode == InstallerMode::InstallSdk
                                                 ? "Download && Install"
                                                 : "Download && Extract maps");
    installBtn->setEnabled(false);
    QPushButton *manageMapsBtn = nullptr;
    if (mode == InstallerMode::InstallAdditionalMaps) {
      manageMapsBtn = new QPushButton("Manage installed maps…");
      manageMapsBtn->setToolTip(
        "Switch to the Scenario Builder tab — its Map Browser lists every "
        "map currently in the running CARLA, with Load / Reload buttons.");
    }
    QPushButton *closeBtn    = new QPushButton("Close");
    QPushButton *cleanupBtn  = new QPushButton(QStringLiteral("🗑"));
    cleanupBtn->setToolTip("Cleanup / Uninstall — delete installed CARLA, "
                            "maps, Docker images, app caches, or PythonAPI "
                            "eggs (preview before delete).");
    cleanupBtn->setFixedWidth(36);
    QObject::connect(cleanupBtn, &QPushButton::clicked, dlg,
                      [&]() { openCleanupDialog(); });
    btnRow->addWidget(refreshBtn);
    btnRow->addWidget(changelogBtn);
    if (manageMapsBtn) btnRow->addWidget(manageMapsBtn);
    btnRow->addStretch();
    btnRow->addWidget(installBtn);
    btnRow->addWidget(cleanupBtn);
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);
    QObject::connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
    if (manageMapsBtn) {
      QObject::connect(manageMapsBtn, &QPushButton::clicked, dlg, [&, dlg]() {
        if (switchToScenarioBuilderTab) switchToScenarioBuilderTab();
        QTimer::singleShot(0, dlg, &QDialog::close);
      });
    }

    auto repopulateList = [=, &pickAssetForMode, &detectInstalledCarlaVersion]() {
      releasesList->clear();
      const QString detected = detectInstalledCarlaVersion();
      for (int i = 0; i < g_cachedReleases.size(); ++i) {
        const ReleaseInfo &r = g_cachedReleases[i];
        const ReleaseAsset a = pickAssetForMode(r, mode);
        const QString tagPretty = (r.tag.startsWith('v') || r.tag.startsWith('V'))
                                    ? r.tag
                                    : ("v" + r.tag);
        QString label = QString("%1  (%2)").arg(r.publishedAt, tagPretty);
        bool hasMaps = true;
        if (mode == InstallerMode::InstallAdditionalMaps && a.downloadUrl.isEmpty()) {
          label += "   — no AdditionalMaps archive found in release";
          hasMaps = false;
        }
        if (!detected.isEmpty() && r.tag.contains(detected)) {
          label = "▶ " + label + "   ← installed";
        }
        QListWidgetItem *item = new QListWidgetItem(label, releasesList);
        item->setData(Qt::UserRole, i);
        if (!hasMaps) {
          item->setFlags(item->flags() & ~Qt::ItemIsSelectable);
          item->setForeground(Qt::gray);
        }
      }
      if (mode == InstallerMode::InstallAdditionalMaps && !detected.isEmpty()) {
        for (int i = 0; i < releasesList->count(); ++i) {
          if (releasesList->item(i)->text().contains("← installed")) {
            releasesList->setCurrentRow(i);
            break;
          }
        }
      }
    };

    auto refreshAction = [=, &fetchCarlaReleases]() {
      installBtn->setEnabled(false);
      refreshBtn->setEnabled(false);
      log->appendPlainText("");
      const bool ok = fetchCarlaReleases(log);
      refreshBtn->setEnabled(true);
      if (ok) repopulateList();
    };
    QObject::connect(refreshBtn, &QPushButton::clicked, dlg, refreshAction);

    QObject::connect(releasesList, &QListWidget::currentItemChanged, dlg,
      [=](QListWidgetItem *cur, QListWidgetItem *) {
        installBtn->setEnabled(cur != nullptr);
        if (!cur) return;
        const int idx = cur->data(Qt::UserRole).toInt();
        if (idx < 0 || idx >= g_cachedReleases.size()) return;
        const ReleaseInfo &r = g_cachedReleases[idx];
        QString html = QString("<h3>%1</h3><p><i>Published %2</i></p>")
                         .arg(r.name.toHtmlEscaped(), r.publishedAt.toHtmlEscaped());
        QString body = r.body.toHtmlEscaped()
                          .replace("\n", "<br>")
                          .replace("**", "");
        html += "<div>" + body + "</div>";
        if (!r.assets.isEmpty()) {
          html += "<h4>Assets</h4><ul>";
          for (const auto &a : r.assets) {
            html += QString("<li>%1 (%2 MB)</li>")
                      .arg(a.name.toHtmlEscaped())
                      .arg(QString::number(a.size / 1024.0 / 1024.0, 'f', 1));
          }
          html += "</ul>";
        }
        changelogView->setHtml(html);
      });

    QObject::connect(changelogBtn, &QPushButton::clicked, dlg, [=]() {
      QListWidgetItem *cur = releasesList->currentItem();
      if (!cur) return;
      const int idx = cur->data(Qt::UserRole).toInt();
      if (idx < 0 || idx >= g_cachedReleases.size()) return;
      QMessageBox::information(dlg, g_cachedReleases[idx].name,
                               g_cachedReleases[idx].body);
    });

    auto promptAddCarlaRootToShellRc = [dlg, log](const QString &rootPath) {
      const QString home = QDir::homePath();
      const QString shell = QString::fromLocal8Bit(qgetenv("SHELL"));
      QString rc = home + "/.bashrc";
      QString rcLabel = "~/.bashrc";
      if (shell.contains("zsh")) { rc = home + "/.zshrc"; rcLabel = "~/.zshrc"; }
      else if (shell.contains("fish")) { rc = home + "/.config/fish/config.fish"; rcLabel = "~/.config/fish/config.fish"; }
      const auto reply = QMessageBox::question(
        dlg,
        "Add CARLA_ROOT to your shell?",
        QString("Append <code>export CARLA_ROOT=%1</code> to <b>%2</b>?<br><br>"
                "Adds the line idempotently — if CARLA_ROOT is already set in "
                "the file, the value is updated in place.")
          .arg(rootPath.toHtmlEscaped(), rcLabel),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes);
      if (reply != QMessageBox::Yes) return;
      QFile f(rc);
      QString existing;
      if (f.exists() && f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        existing = QString::fromLocal8Bit(f.readAll());
        f.close();
      }
      const QString useExport = rc.endsWith("config.fish")
        ? QString("set -gx CARLA_ROOT %1").arg(rootPath)
        : QString("export CARLA_ROOT=\"%1\"").arg(rootPath);
      QRegularExpression re(rc.endsWith("config.fish")
        ? "^[ \\t]*set[ \\t]+-gx[ \\t]+CARLA_ROOT[ \\t].*$"
        : "^[ \\t]*export[ \\t]+CARLA_ROOT=.*$",
        QRegularExpression::MultilineOption);
      QString updated;
      if (re.match(existing).hasMatch()) {
        updated = existing;
        updated.replace(re, useExport);
      } else {
        updated = existing;
        if (!updated.isEmpty() && !updated.endsWith('\n')) updated += '\n';
        updated += "\n# Added by CARLA Studio\n" + useExport + "\n";
      }
      QDir().mkpath(QFileInfo(rc).absolutePath());
      if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        log->appendPlainText(QString("✗ Could not write %1").arg(rcLabel));
        return;
      }
      f.write(updated.toLocal8Bit());
      f.close();
      log->appendPlainText(QString("✓ Wrote CARLA_ROOT into %1 (open a new shell to pick it up).")
                             .arg(rcLabel));
    };

    auto isExtractedCarlaInstall = [](const QString &dir) -> bool {
      if (dir.isEmpty() || !QDir(dir).exists()) return false;
      static const QStringList markers = {
        "CarlaUE4.sh", "CarlaUE5.sh", "CarlaUnreal.sh",
        "LinuxNoEditor/CarlaUE4.sh", "PythonAPI", "Engine"
      };
      for (const QString &m : markers) {
        if (QFileInfo::exists(dir + "/" + m)) return true;
      }
      return false;
    };

    QObject::connect(installBtn, &QPushButton::clicked, dlg,
      [=, &validateCarlaRoot, &populateScenarios]() {
        QListWidgetItem *cur = releasesList->currentItem();
        if (!cur) return;
        const int idx = cur->data(Qt::UserRole).toInt();
        if (idx < 0 || idx >= g_cachedReleases.size()) return;
        const ReleaseInfo r = g_cachedReleases[idx];
        const ReleaseAsset asset = pickAssetForMode(r, mode);
        if (asset.downloadUrl.isEmpty()) {
          QMessageBox::warning(dlg, "No matching asset",
                               QString("Release %1 has no %2 tarball published.")
                                 .arg(r.tag, mode == InstallerMode::InstallSdk
                                               ? "CARLA install" : "AdditionalMaps"));
          return;
        }

        const QString target = targetPath->text().trimmed();
        if (target.isEmpty()) {
          QMessageBox::warning(dlg, "No target path",
                               "Please choose where to install / extract.");
          return;
        }
        QDir().mkpath(target);
        const QString tarPath = target + "/" + asset.name;
        const QString plannedExtractDir = (mode == InstallerMode::InstallSdk)
                                            ? QString("%1/CARLA_%2").arg(target, r.tag)
                                            : target;

        if (mode == InstallerMode::InstallSdk
            && isExtractedCarlaInstall(plannedExtractDir)) {
          const auto pick = QMessageBox::question(
            dlg,
            "Existing install found",
            QString("<b>%1</b> already contains an extracted CARLA install.<br><br>"
                    "<b>Yes</b> — point CARLA_ROOT at it (no download).<br>"
                    "<b>No</b> — re-download and overwrite.<br>"
                    "<b>Cancel</b> — abort.").arg(plannedExtractDir.toHtmlEscaped()),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Yes);
          if (pick == QMessageBox::Cancel) return;
          if (pick == QMessageBox::Yes) {
            log->appendPlainText(QString("→ Reusing existing install at %1").arg(plannedExtractDir));
            carlaRootPath->setText(plannedExtractDir);
            qputenv("CARLA_ROOT", plannedExtractDir.toLocal8Bit());
            validateCarlaRoot();
            populateScenarios();
            log->appendPlainText("✓ CARLA configured. You can now START the simulator.");
            promptAddCarlaRootToShellRc(plannedExtractDir);
            return;
          }
        }

        installBtn->setEnabled(false);
        refreshBtn->setEnabled(false);
        log->appendPlainText("");
        log->appendPlainText(QString("→ Downloading %1 (%2 MB)…")
                               .arg(asset.name)
                               .arg(QString::number(asset.size / 1024.0 / 1024.0, 'f', 1)));

        QProcess *dl = new QProcess(dlg);
        dl->setProcessChannelMode(QProcess::MergedChannels);
        QObject::connect(dl, &QProcess::readyReadStandardOutput, dlg, [dl, log]() {
          const QByteArray chunk = dl->readAllStandardOutput();
          if (!chunk.isEmpty()) log->appendPlainText(QString::fromLocal8Bit(chunk).trimmed());
        });
        QObject::connect(dl,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), dlg,
          [=, &validateCarlaRoot, &populateScenarios](int ec, QProcess::ExitStatus es) {
            dl->deleteLater();
            if (es != QProcess::NormalExit || ec != 0) {
              log->appendPlainText(QString("✗ Download failed (exit=%1).").arg(ec));
              installBtn->setEnabled(true);
              refreshBtn->setEnabled(true);
              return;
            }
            log->appendPlainText("✓ Download complete. Extracting…");

            const QString extractDir = (mode == InstallerMode::InstallSdk)
                                         ? QString("%1/CARLA_%2").arg(target, r.tag)
                                         : target;
            QDir().mkpath(extractDir);
            QProcess *tar = new QProcess(dlg);
            tar->setProcessChannelMode(QProcess::MergedChannels);
            QObject::connect(tar, &QProcess::readyReadStandardOutput, dlg, [tar, log]() {
              const QByteArray chunk = tar->readAllStandardOutput();
              if (!chunk.isEmpty()) log->appendPlainText(QString::fromLocal8Bit(chunk).trimmed());
            });
            QObject::connect(tar,
              QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), dlg,
              [=, &validateCarlaRoot, &populateScenarios](int tec, QProcess::ExitStatus tes) {
                tar->deleteLater();
                if (tes != QProcess::NormalExit || tec != 0) {
                  log->appendPlainText(QString("✗ Extract failed (exit=%1).").arg(tec));
                  installBtn->setEnabled(true);
                  refreshBtn->setEnabled(true);
                  return;
                }
                log->appendPlainText("✓ Extract complete.");
                QFile::remove(tarPath);

                if (mode == InstallerMode::InstallSdk) {
                  log->appendPlainText(QString("→ Pointing CARLA_ROOT at %1").arg(extractDir));
                  carlaRootPath->setText(extractDir);
                  qputenv("CARLA_ROOT", extractDir.toLocal8Bit());
                  validateCarlaRoot();
                  populateScenarios();
                  log->appendPlainText("✓ CARLA configured. You can now START the simulator.");
                  promptAddCarlaRootToShellRc(extractDir);
                } else {
                  populateScenarios();
                  log->appendPlainText("✓ Maps installed. Refreshed scenario list.");
                }
                installBtn->setEnabled(true);
                refreshBtn->setEnabled(true);
              });
            tar->start("tar", QStringList() << "-xzf" << tarPath << "-C" << extractDir);
          });
        dl->start("curl", QStringList()
          << "-L" << "--fail" << "--progress-bar"
          << "-o" << tarPath
          << asset.downloadUrl);
      });

    refreshAction();
    dlg->show();
    dlg->raise();
  };

  struct CommunityMapEntry {
    const char *key;
    const char *name;
    const char *summary;
    const char *source;
    const char *gitUrl;
    bool useLfs;
    bool recurseSubmodules;
    const char *srcSubdir;
    const char *destSubdir;
    const char *compatNote;
    const char *postInstall;
  };

  static const std::array<CommunityMapEntry, 2> kCommunityMaps = {{
    { "mcity",
      "Mcity Digital Twin",
      "Digital twin of the University of Michigan Mcity test facility "
      "(200-acre AV proving ground). Ships a full Unreal asset pack "
      "(McityMap_Main.umap) plus an OpenDRIVE (.xodr) counterpart.",
      "https://github.com/mcity/mcity-digital-twin",
      "https://github.com/mcity/mcity-digital-twin.git",
       true,
       false,
      "CARLA/packaged_version/linux",
      "CarlaUE4/Content/McityMap",
      "CARLA 0.9.12+ (UE4.26) — not formally verified against 0.10.0",
      "Then edit CarlaUE4/Config/DefaultGame.ini and add, under\n"
      "[/Script/UnrealEd.ProjectPackagingSettings]:\n"
      "  +MapsToCook=(FilePath=\"/Game/McityMap/Maps/McityMap_Main\")\n"
      "  +DirectoriesToAlwaysCook=(Path=\"McityMap\")\n\n"
      "Then spawn the map via:\n"
      "  python PythonAPI/examples/load_mcity_digital_twin.py"
    },
    { "apollo-hd",
      "Apollo HD Maps (Town01–05)",
      "HD-Map overlays (in Apollo bin/txt format) for the stock CARLA "
      "towns 1 through 5. Useful if you're running Baidu Apollo with "
      "CARLA as the sim backend — not a standalone CARLA world.",
      "https://github.com/MaisJamal/Carla_apollo_maps",
      "https://github.com/MaisJamal/Carla_apollo_maps.git",
       false,
       false,
      ".",
      "CommunityMaps/Apollo",
      "CARLA 0.9.10+ stock towns — Apollo-specific",
      "Point your Apollo stack at:\n"
      "  <CARLA_ROOT>/CommunityMaps/Apollo/<Town>/\n"
      "Each Town folder contains base_map.bin, routing_map.bin, and "
      "sim_map.bin alongside the raw txt sources."
    }
  }};

  std::function<void()> openCommunityMapsDialog;
  openCommunityMapsDialog = [&]() {
    QDialog *dlg = new QDialog(&window);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setWindowTitle("CARLA · Community Maps");
    dlg->resize(760, 580);
    QVBoxLayout *root = new QVBoxLayout(dlg);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(8);

    QLabel *intro = new QLabel(
      "<b>Community Maps</b><br>"
      "A short catalogue of well-known third-party CARLA maps that ship as "
      "downloadable asset packs. The app will clone the source repository "
      "and copy the map folder into your <code>CARLA_ROOT</code>. Any "
      "residual manual steps (DefaultGame.ini edits, cook scripts) are "
      "surfaced in the log pane — the app will not edit engine configs.");
    intro->setWordWrap(true);
    intro->setTextInteractionFlags(Qt::TextBrowserInteraction);
    intro->setOpenExternalLinks(true);
    root->addWidget(intro);

    QHBoxLayout *targetRow = new QHBoxLayout();
    QLineEdit *targetPath = new QLineEdit();
    targetPath->setText(carlaRootPath->text().trimmed());
    targetPath->setReadOnly(true);
    targetRow->addWidget(new QLabel("Target root:"));
    targetRow->addWidget(targetPath, 1);
    root->addLayout(targetRow);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, dlg);
    QListWidget *mapsList = new QListWidget();
    splitter->addWidget(mapsList);

    QTextBrowser *detailView = new QTextBrowser();
    detailView->setOpenExternalLinks(true);
    splitter->addWidget(detailView);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    root->addWidget(splitter, 1);

    QPlainTextEdit *log = new QPlainTextEdit();
    log->setReadOnly(true);
    log->setMaximumBlockCount(500);
    log->setPlaceholderText("Install log will appear here.");
    log->setMaximumHeight(140);
    root->addWidget(log);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *openSourceBtn = new QPushButton("Open source page");
    QPushButton *installBtn    = new QPushButton("Clone & Install");
    QPushButton *closeBtn      = new QPushButton("Close");
    btnRow->addWidget(openSourceBtn);
    btnRow->addStretch(1);
    btnRow->addWidget(installBtn);
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);
    QObject::connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);

    for (const auto &e : kCommunityMaps) {
      auto *item = new QListWidgetItem(QString::fromUtf8(e.name));
      item->setData(Qt::UserRole, QString::fromUtf8(e.key));
      mapsList->addItem(item);
    }

    auto findEntry = [](const QString &key) -> const CommunityMapEntry * {
      for (const auto &e : kCommunityMaps) {
        if (QString::fromUtf8(e.key) == key) return &e;
      }
      return nullptr;
    };

    auto renderDetail = [&](const CommunityMapEntry *e) {
      if (!e) { detailView->clear(); return; }
      const QString html = QString(
        "<h3>%1</h3>"
        "<p>%2</p>"
        "<p><b>Source:</b> <a href=\"%3\">%3</a></p>"
        "<p><b>Git clone:</b> <code>%4</code>%5</p>"
        "<p><b>Target:</b> <code>&lt;CARLA_ROOT&gt;/%6</code></p>"
        "<p><b>Compatibility:</b> %7</p>"
        "<hr>"
        "<p><b>After Clone &amp; Install, manual steps:</b></p>"
        "<pre>%8</pre>")
        .arg(QString::fromUtf8(e->name).toHtmlEscaped(),
             QString::fromUtf8(e->summary).toHtmlEscaped(),
             QString::fromUtf8(e->source),
             QString::fromUtf8(e->gitUrl).toHtmlEscaped(),
             e->useLfs ? QStringLiteral(" <i>(requires git-lfs)</i>") : QString(),
             QString::fromUtf8(e->destSubdir).toHtmlEscaped(),
             QString::fromUtf8(e->compatNote).toHtmlEscaped(),
             QString::fromUtf8(e->postInstall).toHtmlEscaped());
      detailView->setHtml(html);
    };

    QObject::connect(mapsList, &QListWidget::currentRowChanged, dlg,
      [&, findEntry, renderDetail](int r) {
        if (r < 0) return;
        renderDetail(findEntry(mapsList->item(r)->data(Qt::UserRole).toString()));
      });
    mapsList->setCurrentRow(0);

    QObject::connect(openSourceBtn, &QPushButton::clicked, dlg,
      [&, findEntry]() {
        QListWidgetItem *it = mapsList->currentItem();
        if (!it) return;
        const CommunityMapEntry *e = findEntry(it->data(Qt::UserRole).toString());
        if (e) QDesktopServices::openUrl(QUrl(QString::fromUtf8(e->source)));
      });

    QObject::connect(installBtn, &QPushButton::clicked, dlg,
      [&, findEntry]() {
        QListWidgetItem *it = mapsList->currentItem();
        if (!it) return;
        const CommunityMapEntry *e = findEntry(it->data(Qt::UserRole).toString());
        if (!e) return;

        const QString root = targetPath->text().trimmed();
        if (root.isEmpty() || !QDir(root).exists()) {
          log->appendPlainText("✗ CARLA_ROOT is not configured — install CARLA first.");
          return;
        }

        const QString tmpBase = QDir::tempPath() + "/carla-studio-community-maps";
        QDir().mkpath(tmpBase);
        const QString workDir = tmpBase + "/" + QString::fromUtf8(e->key);
        QDir(workDir).removeRecursively();

        installBtn->setEnabled(false);
        log->clear();
        log->appendPlainText(QString("→ Cloning %1 …").arg(QString::fromUtf8(e->gitUrl)));

        auto *git = new QProcess(dlg);
        git->setProcessChannelMode(QProcess::MergedChannels);
        QObject::connect(git, &QProcess::readyReadStandardOutput, dlg, [git, log]() {
          log->appendPlainText(QString::fromUtf8(git->readAllStandardOutput()).trimmed());
        });
        QObject::connect(git, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), dlg,
          [=, &window, destKey = QString::fromUtf8(e->key),
           useLfs = e->useLfs, srcSub = QString::fromUtf8(e->srcSubdir),
           destSub = QString::fromUtf8(e->destSubdir),
           postInstall = QString::fromUtf8(e->postInstall)](int rc, QProcess::ExitStatus) {
            Q_UNUSED(&window);
            Q_UNUSED(destKey);
            if (rc != 0) {
              log->appendPlainText(QString("✗ git clone failed (rc=%1). Is git installed and can you reach GitHub?").arg(rc));
              installBtn->setEnabled(true);
              return;
            }
            log->appendPlainText("✓ Cloned.");

            auto runCopyAndHints = [=]() {
              const QString src = workDir + "/" + srcSub;
              const QString dst = root + "/" + destSub;
              if (!QFileInfo(src).exists()) {
                log->appendPlainText(QString("✗ Expected source folder %1 not in clone.").arg(src));
                installBtn->setEnabled(true);
                return;
              }
              QDir().mkpath(QFileInfo(dst).absolutePath());
              log->appendPlainText(QString("→ Copying %1 → %2 …").arg(src, dst));
              auto *cp = new QProcess(dlg);
              cp->setProcessChannelMode(QProcess::MergedChannels);
              QObject::connect(cp, &QProcess::readyReadStandardOutput, dlg, [cp, log]() {
                log->appendPlainText(QString::fromUtf8(cp->readAllStandardOutput()).trimmed());
              });
              QObject::connect(cp, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), dlg,
                [=](int rc2, QProcess::ExitStatus) {
                  if (rc2 != 0) {
                    log->appendPlainText(QString("✗ Copy failed (rc=%1).").arg(rc2));
                  } else {
                    log->appendPlainText("✓ Copy complete.");
                    log->appendPlainText("");
                    log->appendPlainText("── Post-install manual steps ──");
                    log->appendPlainText(postInstall);
                  }
                  installBtn->setEnabled(true);
                });
              cp->start("cp", QStringList() << "-r" << src << dst);
            };

            if (useLfs) {
              log->appendPlainText("→ Pulling LFS objects (this can take a while) …");
              auto *lfs = new QProcess(dlg);
              lfs->setProcessChannelMode(QProcess::MergedChannels);
              lfs->setWorkingDirectory(workDir);
              QObject::connect(lfs, &QProcess::readyReadStandardOutput, dlg, [lfs, log]() {
                log->appendPlainText(QString::fromUtf8(lfs->readAllStandardOutput()).trimmed());
              });
              QObject::connect(lfs, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), dlg,
                [=](int rc2, QProcess::ExitStatus) {
                  if (rc2 != 0) {
                    log->appendPlainText(QString("✗ git lfs pull failed (rc=%1). Is git-lfs installed?").arg(rc2));
                    installBtn->setEnabled(true);
                    return;
                  }
                  log->appendPlainText("✓ LFS objects pulled.");
                  runCopyAndHints();
                });
              lfs->start("git", QStringList() << "lfs" << "pull");
            } else {
              runCopyAndHints();
            }
          });

        QStringList cloneArgs;
        cloneArgs << "clone";
        if (e->recurseSubmodules) cloneArgs << "--recurse-submodules";
        cloneArgs << "--depth" << "1";
        cloneArgs << QString::fromUtf8(e->gitUrl) << workDir;
        git->start("git", cloneArgs);
      });

    dlg->show();
    dlg->raise();
  };

  struct CleanupCandidate {
    QString id;
    QString category;
    QString display;
    QString detail;
    qint64  sizeBytes = -1;
    enum class Kind { Path, DockerImage };
    Kind    kind = Kind::Path;
    QString target;
  };

  auto humanBytes = [](qint64 b) -> QString {
    if (b < 0) return QStringLiteral("?");
    const double kB = 1024.0, MB = kB * 1024, GB = MB * 1024;
    if (b >= GB) return QString::number(b / GB, 'f', 2) + " GB";
    if (b >= MB) return QString::number(b / MB, 'f', 1) + " MB";
    if (b >= kB) return QString::number(b / kB, 'f', 1) + " KB";
    return QString::number(b) + " B";
  };

  auto duSize = [](const QString &path) -> qint64 {
    QProcess p;
    p.start("/bin/bash", QStringList() << "-lc"
              << QString("du -sb -- %1 2>/dev/null | cut -f1")
                   .arg(QString("'") + QString(path).replace("'", "'\\''") + "'"));
    p.waitForFinished(5000);
    bool ok = false;
    const qint64 v =
      QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed().toLongLong(&ok);
    return ok ? v : -1;
  };

  auto parseDockerSize = [](const QString &s) -> qint64 {
    QString t = s.trimmed();
    if (t.isEmpty()) return -1;
    qint64 mult = 1;
    if (t.endsWith("GB", Qt::CaseInsensitive))      { mult = 1024LL*1024*1024; t.chop(2); }
    else if (t.endsWith("MB", Qt::CaseInsensitive)) { mult = 1024LL*1024;       t.chop(2); }
    else if (t.endsWith("kB", Qt::CaseInsensitive)) { mult = 1024;              t.chop(2); }
    else if (t.endsWith("B",  Qt::CaseInsensitive)) { mult = 1;                 t.chop(1); }
    bool ok = false;
    const double v = t.toDouble(&ok);
    return ok ? qint64(v * mult) : -1;
  };

  auto detectCleanupCandidates = [&]() -> QList<CleanupCandidate> {
    QList<CleanupCandidate> out;
    const QString root = carlaRootPath->text().trimmed();

    if (!root.isEmpty() && QDir(root).exists()) {
      out.append({"carla_root_full", "CARLA install",
                  "CARLA root (entire directory)", root, duSize(root),
                  CleanupCandidate::Kind::Path, root});
      for (const QString &sub :
           { QStringLiteral("CarlaUE4/Saved"),
             QStringLiteral("CarlaUE5/Saved"),
             QStringLiteral("CarlaUnreal/Saved") }) {
        const QString p = root + "/" + sub;
        if (QDir(p).exists()) {
          QString idTail = sub;
          idTail.replace('/', '_');
          out.append({"carla_soft_" + idTail,
                      "CARLA install (soft cleanup)",
                      "Saved/Logs: " + sub, p, duSize(p),
                      CleanupCandidate::Kind::Path, p});
        }
      }
    }

    if (!root.isEmpty()) {
      for (const QString &contentDir : { QStringLiteral("CarlaUE4/Content"),
                                          QStringLiteral("CarlaUE5/Content") }) {
        QDir cd(root + "/" + contentDir);
        if (!cd.exists()) continue;
        for (const QString &mapName :
             { QStringLiteral("McityMap"), QStringLiteral("CommunityMaps") }) {
          const QString p = cd.absoluteFilePath(mapName);
          if (QFileInfo(p).isDir()) {
            out.append({"community_" + mapName + "_" + contentDir.section('/', 0, 0),
                        "Community maps", mapName + " (" + contentDir + ")", p,
                        duSize(p), CleanupCandidate::Kind::Path, p});
          }
        }
      }
    }

    {
      QProcess docker;
      docker.start("docker", QStringList()
        << "images" << "--format"
        << "{{.Repository}}:{{.Tag}}|{{.Size}}" << "carlasim/carla");
      if (docker.waitForFinished(5000) && docker.exitCode() == 0) {
        const QString s = QString::fromLocal8Bit(docker.readAllStandardOutput());
        for (const QString &line : s.split('\n', Qt::SkipEmptyParts)) {
          const auto parts = line.split('|');
          if (parts.size() < 2) continue;
          const QString tag  = parts[0].trimmed();
          const QString sStr = parts[1].trimmed();
          out.append({"docker_" + tag, "Docker images", tag, "size: " + sStr,
                      parseDockerSize(sStr),
                      CleanupCandidate::Kind::DockerImage, tag});
        }
      }
    }

    for (const QString &cache : {
           QDir::tempPath() + "/carla-studio-examples",
           QDir::tempPath() + "/carla-studio-osm",
           QDir::tempPath() + "/carla-studio-community-maps",
           QDir::homePath() + "/.carla-studio/scenarios" }) {
      if (QDir(cache).exists()) {
        out.append({"cache_" + QFileInfo(cache).fileName(), "App caches",
                    QFileInfo(cache).fileName(), cache, duSize(cache),
                    CleanupCandidate::Kind::Path, cache});
      }
    }

    if (!root.isEmpty()) {
      QDir distDir(root + "/PythonAPI/carla/dist");
      if (distDir.exists()) {
        const QStringList eggs = distDir.entryList(
          QStringList() << "carla-*.egg" << "carla-*.whl", QDir::Files);
        for (const QString &egg : eggs) {
          const QString p = distDir.absoluteFilePath(egg);
          out.append({"egg_" + egg, "PythonAPI", egg, p,
                      QFileInfo(p).size(),
                      CleanupCandidate::Kind::Path, p});
        }
      }
    }
    return out;
  };

  openCleanupDialog = [&, humanBytes, detectCleanupCandidates]() {
    QDialog *dlg = new QDialog(&window);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setWindowTitle("CARLA Studio · Cleanup / Uninstall");
    dlg->resize(760, 600);
    QVBoxLayout *root = new QVBoxLayout(dlg);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(8);

    QLabel *intro = new QLabel(
      "<b>Cleanup / Uninstall</b><br>"
      "Detects installed components and caches, lets you preview what would "
      "be removed, and only deletes after a second click. "
      "<i>Preview</i> never modifies anything; <i>Confirm delete</i> is "
      "irreversible.");
    intro->setWordWrap(true);
    root->addWidget(intro);

    QTreeWidget *tree = new QTreeWidget();
    tree->setColumnCount(3);
    tree->setHeaderLabels(QStringList() << "Component" << "Path / Detail" << "Size");
    tree->setColumnWidth(0, 250);
    tree->setColumnWidth(1, 340);
    tree->setSelectionMode(QAbstractItemView::NoSelection);
    tree->setRootIsDecorated(true);
    root->addWidget(tree, 1);

    QLabel *totalLabel = new QLabel("Total selected: 0 B");
    totalLabel->setStyleSheet("font-weight: 600; padding-top: 4px;");
    root->addWidget(totalLabel);

    QPlainTextEdit *log = new QPlainTextEdit();
    log->setReadOnly(true);
    log->setMaximumBlockCount(500);
    log->setMaximumHeight(110);
    log->setPlaceholderText("Preview / delete log will appear here.");
    root->addWidget(log);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *detectBtn  = new QPushButton("⟳ Detect");
    QPushButton *previewBtn = new QPushButton("Preview");
    QPushButton *deleteBtn  = new QPushButton("Confirm delete");
    QPushButton *closeBtn   = new QPushButton("Close");
    previewBtn->setEnabled(false);
    deleteBtn->setEnabled(false);
    deleteBtn->setStyleSheet(
      "QPushButton:enabled { background: #C62828; color: white; font-weight: 600;"
      "  border: 1px solid #8E1B1B; border-radius: 4px; padding: 6px 12px; }"
      "QPushButton:disabled { background: #E8E8E8; color: #999;"
      "  border: 1px solid #CCC; border-radius: 4px; padding: 6px 12px; }");
    btnRow->addWidget(detectBtn);
    btnRow->addWidget(previewBtn);
    btnRow->addStretch();
    btnRow->addWidget(deleteBtn);
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);
    QObject::connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);

    auto state = std::make_shared<QHash<QTreeWidgetItem *, CleanupCandidate>>();
    auto previewedSinceLastChange = std::make_shared<bool>(false);

    auto refreshTotal = [=]() {
      qint64 total = 0;
      int picked = 0;
      for (auto it = state->constBegin(); it != state->constEnd(); ++it) {
        if (it.key()->checkState(0) == Qt::Checked && it.value().sizeBytes > 0) {
          total += it.value().sizeBytes;
          ++picked;
        } else if (it.key()->checkState(0) == Qt::Checked) {
          ++picked;
        }
      }
      totalLabel->setText(QString("Total selected: %1 across %2 item%3")
                            .arg(humanBytes(total))
                            .arg(picked)
                            .arg(picked == 1 ? "" : "s"));
      previewBtn->setEnabled(picked > 0);
      *previewedSinceLastChange = false;
      deleteBtn->setEnabled(false);
    };

    QObject::connect(tree, &QTreeWidget::itemChanged, dlg,
      [refreshTotal](QTreeWidgetItem *, int col) {
        if (col == 0) refreshTotal();
      });

    auto runDetect = [=]() {
      tree->blockSignals(true);
      tree->clear();
      state->clear();
      const auto candidates = detectCleanupCandidates();
      QHash<QString, QTreeWidgetItem *> categoryNode;
      for (const auto &c : candidates) {
        QTreeWidgetItem *parent = categoryNode.value(c.category, nullptr);
        if (!parent) {
          parent = new QTreeWidgetItem(tree);
          parent->setText(0, c.category);
          parent->setExpanded(true);
          QFont f = parent->font(0); f.setBold(true); parent->setFont(0, f);
          categoryNode.insert(c.category, parent);
        }
        auto *child = new QTreeWidgetItem(parent);
        child->setText(0, c.display);
        child->setText(1, c.detail);
        child->setText(2, humanBytes(c.sizeBytes));
        child->setFlags(child->flags() | Qt::ItemIsUserCheckable);
        child->setCheckState(0, Qt::Unchecked);
        state->insert(child, c);
      }
      tree->blockSignals(false);
      log->appendPlainText(QString("✓ detected %1 component%2")
                            .arg(candidates.size())
                            .arg(candidates.size() == 1 ? "" : "s"));
      refreshTotal();
    };

    QObject::connect(detectBtn, &QPushButton::clicked, dlg, [=]() { runDetect(); });

    QObject::connect(previewBtn, &QPushButton::clicked, dlg, [=]() {
      log->appendPlainText("--- PREVIEW ---");
      qint64 total = 0;
      int picked = 0;
      for (auto it = state->constBegin(); it != state->constEnd(); ++it) {
        if (it.key()->checkState(0) != Qt::Checked) continue;
        const auto &c = it.value();
        ++picked;
        if (c.sizeBytes > 0) total += c.sizeBytes;
        if (c.kind == CleanupCandidate::Kind::Path) {
          log->appendPlainText(
            QString("  would rm -rf  %1   (%2)").arg(c.target, humanBytes(c.sizeBytes)));
        } else {
          log->appendPlainText(
            QString("  would docker rmi  %1   (%2)").arg(c.target, humanBytes(c.sizeBytes)));
        }
      }
      log->appendPlainText(QString("--- %1 item%2, %3 total ---")
                            .arg(picked).arg(picked == 1 ? "" : "s")
                            .arg(humanBytes(total)));
      *previewedSinceLastChange = (picked > 0);
      deleteBtn->setEnabled(*previewedSinceLastChange);
    });

    QObject::connect(deleteBtn, &QPushButton::clicked, dlg, [=, &window]() {
      if (!*previewedSinceLastChange) return;

      qint64 total = 0;
      int picked = 0;
      for (auto it = state->constBegin(); it != state->constEnd(); ++it) {
        if (it.key()->checkState(0) != Qt::Checked) continue;
        ++picked;
        if (it.value().sizeBytes > 0) total += it.value().sizeBytes;
      }
      QMessageBox box(&window);
      box.setIcon(QMessageBox::Warning);
      box.setWindowTitle("Confirm delete");
      box.setText(QString("This will permanently delete %1 item%2 (%3 total).")
                    .arg(picked).arg(picked == 1 ? "" : "s").arg(humanBytes(total)));
      box.setInformativeText("This action cannot be undone. Proceed?");
      box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
      box.setDefaultButton(QMessageBox::Cancel);
      if (box.exec() != QMessageBox::Yes) {
        log->appendPlainText("✗ delete cancelled by user");
        return;
      }

      log->appendPlainText("--- DELETING ---");
      int ok = 0, fail = 0;
      for (auto it = state->begin(); it != state->end(); ++it) {
        if (it.key()->checkState(0) != Qt::Checked) continue;
        const auto &c = it.value();
        if (c.kind == CleanupCandidate::Kind::Path) {
          QFileInfo fi(c.target);
          bool removed = false;
          if (fi.isFile()) {
            removed = QFile::remove(c.target);
          } else if (fi.isDir()) {
            removed = QDir(c.target).removeRecursively();
          }
          if (removed) {
            log->appendPlainText("  ✓ removed " + c.target);
            it.key()->setText(2, "deleted");
            ++ok;
          } else {
            log->appendPlainText("  ✗ failed   " + c.target);
            ++fail;
          }
        } else {
          QProcess p;
          p.start("docker", QStringList() << "rmi" << c.target);
          p.waitForFinished(20000);
          if (p.exitCode() == 0) {
            log->appendPlainText("  ✓ docker rmi " + c.target);
            it.key()->setText(2, "deleted");
            ++ok;
          } else {
            log->appendPlainText(
              "  ✗ docker rmi " + c.target + " — " +
              QString::fromLocal8Bit(p.readAllStandardError()).trimmed());
            ++fail;
          }
        }
      }
      log->appendPlainText(QString("--- done: %1 succeeded, %2 failed ---")
                             .arg(ok).arg(fail));
      *previewedSinceLastChange = false;
      deleteBtn->setEnabled(false);
    });

    dlg->show();
    dlg->raise();
    runDetect();
  };

  openDockerSettingsDialog = [&]() {
    QDialog *dlg = new QDialog(&window);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setWindowTitle("CARLA Studio · Docker Settings");
    dlg->resize(600, 520);
    QVBoxLayout *root = new QVBoxLayout(dlg);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(8);

    QLabel *intro = new QLabel(
      "<b>Docker Runtime</b><br>"
      "Configures the <code>docker run</code> command used when the Home "
      "tab's <i>Runtime</i> is set to Docker. Saved under <code>QSettings</code>; "
      "applied on the next <b>START</b>.");
    intro->setWordWrap(true);
    intro->setTextFormat(Qt::RichText);
    root->addWidget(intro);

    QFormLayout *form = new QFormLayout();
    QSettings s;

    QComboBox *imageCombo = new QComboBox();
    imageCombo->setEditable(true);
    imageCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    imageCombo->setMinimumContentsLength(28);
    imageCombo->addItem("carlasim/carla:0.9.16");
    {
      QProcess p;
      p.start("docker", QStringList()
        << "images" << "--format" << "{{.Repository}}:{{.Tag}}"
        << "carlasim/carla");
      if (p.waitForFinished(3000) && p.exitCode() == 0) {
        const QString out = QString::fromLocal8Bit(p.readAllStandardOutput());
        for (const QString &line : out.split('\n', Qt::SkipEmptyParts)) {
          const QString trimmed = line.trimmed();
          if (!trimmed.isEmpty() && imageCombo->findText(trimmed) < 0) {
            imageCombo->addItem(trimmed);
          }
        }
      }
    }
    const QString savedImage = s.value("docker/image", "carlasim/carla:0.9.16").toString();
    if (imageCombo->findText(savedImage) < 0) imageCombo->addItem(savedImage);
    imageCombo->setCurrentText(savedImage);
    form->addRow("Image:", imageCombo);

    QCheckBox *gpuCheck = new QCheckBox("--gpus all (NVIDIA Container Toolkit)");
    gpuCheck->setChecked(s.value("docker/gpu", true).toBool());
    gpuCheck->setToolTip("Forwards the host GPU to the container. Requires "
                          "nvidia-container-toolkit installed and the Docker "
                          "daemon configured for it.");
    form->addRow(QString(), gpuCheck);

    QComboBox *netCombo = new QComboBox();
    netCombo->addItems(QStringList() << "host" << "bridge");
    netCombo->setEditable(true);
    netCombo->setCurrentText(s.value("docker/network", "host").toString());
    netCombo->setToolTip("'host' lets the sim share the host's network "
                          "(simplest, Linux only). 'bridge' isolates and "
                          "requires --publish for the world port.");
    form->addRow("Network:", netCombo);

    QLineEdit *extraArgs = new QLineEdit();
    extraArgs->setText(s.value("docker/extra_args", "").toString());
    extraArgs->setPlaceholderText("--shm-size=2g --ulimit nofile=1024:1024 …");
    form->addRow("Extra docker args:", extraArgs);

    QLineEdit *containerName = new QLineEdit();
    containerName->setText(s.value("docker/container_name", "carla-studio-sim").toString());
    form->addRow("Container name:", containerName);

    root->addLayout(form);

    QPlainTextEdit *log = new QPlainTextEdit();
    log->setReadOnly(true);
    log->setMaximumBlockCount(500);
    log->setMaximumHeight(160);
    log->setPlaceholderText("docker pull / status output appears here.");
    root->addWidget(log);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *pullBtn  = new QPushButton("⇩ Pull image");
    QPushButton *checkBtn = new QPushButton("⟳ Re-check docker");
    QPushButton *saveBtn  = new QPushButton("Save");
    QPushButton *closeBtn = new QPushButton("Close");
    btnRow->addWidget(pullBtn);
    btnRow->addWidget(checkBtn);
    btnRow->addStretch();
    btnRow->addWidget(saveBtn);
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);

    QObject::connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);
    QObject::connect(saveBtn, &QPushButton::clicked, dlg, [=]() {
      QSettings s2;
      s2.setValue("docker/image",          imageCombo->currentText().trimmed());
      s2.setValue("docker/gpu",            gpuCheck->isChecked());
      s2.setValue("docker/network",        netCombo->currentText().trimmed());
      s2.setValue("docker/extra_args",     extraArgs->text());
      s2.setValue("docker/container_name", containerName->text().trimmed());
      log->appendPlainText("✓ Saved.");
    });

    QObject::connect(checkBtn, &QPushButton::clicked, dlg, [log]() {
      QProcess p;
      p.start("docker", QStringList() << "info" << "--format" << "{{.ServerVersion}}");
      if (!p.waitForFinished(3000) || p.exitCode() != 0) {
        log->appendPlainText("✗ docker info failed — daemon not running or no perms");
        log->appendPlainText(QString::fromLocal8Bit(p.readAllStandardError()).trimmed());
      } else {
        log->appendPlainText("✓ docker " + QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed());
      }
    });

    QObject::connect(pullBtn, &QPushButton::clicked, dlg,
                      [imageCombo, log, pullBtn, dlg]() {
      const QString image = imageCombo->currentText().trimmed();
      if (image.isEmpty()) { log->appendPlainText("✗ image tag empty"); return; }
      pullBtn->setEnabled(false);
      log->appendPlainText("→ docker pull " + image + " …");
      auto *p = new QProcess(dlg);
      p->setProcessChannelMode(QProcess::MergedChannels);
      QObject::connect(p, &QProcess::readyReadStandardOutput, dlg, [p, log]() {
        log->appendPlainText(QString::fromLocal8Bit(p->readAllStandardOutput()).trimmed());
      });
      QObject::connect(p,
        static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        dlg, [p, log, pullBtn](int rc, QProcess::ExitStatus) {
          log->appendPlainText(rc == 0 ? "✓ pull complete" :
            QString("✗ pull failed (rc=%1)").arg(rc));
          pullBtn->setEnabled(true);
          p->deleteLater();
        });
      p->start("docker", QStringList() << "pull" << image);
    });

    dlg->show();
    dlg->raise();
  };

  QObject::connect(loadMapsBtn, &QPushButton::clicked, &window, [&]() {
    openInstallerDialog(InstallerMode::InstallAdditionalMaps);
  });

  class ApiBadgeClickFilter : public QObject {
   public:
    std::function<bool()> isConfigured;
    std::function<void()> openInstaller;
    std::function<void()> openLog;
    bool eventFilter(QObject *, QEvent *ev) override {
      if (ev->type() == QEvent::MouseButtonRelease) {
        if (isConfigured && !isConfigured()) {
          if (openInstaller) openInstaller();
        } else {
          if (openLog) openLog();
        }
        return true;
      }
      return false;
    }
  };
  auto *badgeFilter = new ApiBadgeClickFilter();
  badgeFilter->setParent(apiWarningLabel);
  badgeFilter->isConfigured  = [&]() { return rootConfiguredOk; };
  badgeFilter->openInstaller = [&]() { openInstallerDialog(InstallerMode::InstallSdk); };
  badgeFilter->openLog       = [lines = libcarlaLogLinesPtr, owner = &window]() {
    QMessageBox box(owner);
    box.setWindowTitle("CARLA runtime log");
    box.setIcon(QMessageBox::Information);
    box.setText(lines && !lines->isEmpty() ? lines->join('\n')
                                           : QStringLiteral("No warnings recorded."));
    box.setStandardButtons(QMessageBox::Close);
    box.exec();
  };
  apiWarningLabel->installEventFilter(badgeFilter);

  enum class HealthStatus { Ok, Warn, Err, Info };
  struct HealthResult {
    HealthStatus status = HealthStatus::Info;
    QString detail;
    QString actionText;
    std::function<void()> action;
  };
  struct HealthRow {
    QLabel *dot = nullptr;
    QLabel *detail = nullptr;
    QPushButton *action = nullptr;
  };

  {
    QVBoxLayout *healthLayout = new QVBoxLayout(healthCheckPage);
    healthLayout->setContentsMargins(10, 10, 10, 10);
    healthLayout->setSpacing(6);
    healthCheckPageOuterLayout = healthLayout;

    QLabel *healthIntro = new QLabel(
      "Detects whether CARLA and the most common ecosystem tools are "
      "available on this machine.");
    healthIntro->setWordWrap(true);
    healthLayout->addWidget(healthIntro);

    QLabel *healthLastRun = new QLabel("Last checked: never");
    healthLastRun->setStyleSheet("color: #888; font-size: 10px;");
    QPushButton *healthRefreshBtn = new QPushButton("↻ Re-run checks");

    auto makeRow = [&](const QString &labelText) -> HealthRow {
      QFrame *frame = new QFrame();
      frame->setFrameShape(QFrame::StyledPanel);
      QHBoxLayout *rowLayout = new QHBoxLayout(frame);
      rowLayout->setContentsMargins(8, 4, 8, 4);
      rowLayout->setSpacing(8);

      QLabel *dot = new QLabel();
      dot->setFixedSize(12, 12);
      dot->setStyleSheet("background-color: #888; border-radius: 6px;");
      rowLayout->addWidget(dot);

      QLabel *label = new QLabel(labelText);
      label->setStyleSheet("font-weight: 600;");
      label->setFixedWidth(140);
      rowLayout->addWidget(label);

      QLabel *detail = new QLabel("—");
      detail->setStyleSheet("color: #666;");
      detail->setTextInteractionFlags(Qt::TextSelectableByMouse);
      detail->setWordWrap(false);
      rowLayout->addWidget(detail, 1);

      QPushButton *action = new QPushButton();
      action->setVisible(false);
      rowLayout->addWidget(action);

      healthLayout->addWidget(frame);
      return {dot, detail, action};
    };

    auto makePairedRow = [&](const QString &labelA, const QString &labelB)
        -> std::pair<HealthRow, HealthRow> {
      QFrame *frame = new QFrame();
      frame->setFrameShape(QFrame::StyledPanel);
      QHBoxLayout *rowLayout = new QHBoxLayout(frame);
      rowLayout->setContentsMargins(8, 4, 8, 4);
      rowLayout->setSpacing(8);

      auto buildCell = [&](const QString &labelText) -> HealthRow {
        QLabel *dot = new QLabel();
        dot->setFixedSize(12, 12);
        dot->setStyleSheet("background-color: #888; border-radius: 6px;");
        rowLayout->addWidget(dot);

        QLabel *label = new QLabel(labelText);
        label->setStyleSheet("font-weight: 600;");
        label->setFixedWidth(110);
        rowLayout->addWidget(label);

        QLabel *detail = new QLabel("—");
        detail->setStyleSheet("color: #666;");
        detail->setTextInteractionFlags(Qt::TextSelectableByMouse);
        detail->setWordWrap(false);
        rowLayout->addWidget(detail, 1);

        QPushButton *action = new QPushButton();
        action->setVisible(false);
        action->setMaximumWidth(60);
        rowLayout->addWidget(action);

        return { dot, detail, action };
      };

      HealthRow rowA = buildCell(labelA);
      QFrame *vline = new QFrame();
      vline->setFrameShape(QFrame::VLine);
      vline->setStyleSheet("color: rgba(0,0,0,40);");
      rowLayout->addWidget(vline);
      HealthRow rowB = buildCell(labelB);

      healthLayout->addWidget(frame);
      return { rowA, rowB };
    };

    HealthRow rowCarlaRoot = makeRow("CARLA root");
    HealthRow rowEngineVer = makeRow("Engine");
    HealthRow rowVersionCompat = makeRow("SDK ↔ Sim version");
    HealthRow rowPythonApi, rowCppApi;
    std::tie(rowPythonApi, rowCppApi) = makePairedRow("PythonAPI", "C++ API");
    HealthRow rowDiskFree  = makeRow("Disk free");
    HealthRow rowNvidia    = makeRow("NVIDIA driver");
    HealthRow rowRos2, rowRosBridge;
    std::tie(rowRos2, rowRosBridge) = makePairedRow("ROS 2 (native)", "ROS-bridge");
    HealthRow rowScenarioR = makeRow("Scenario Runner");
    HealthRow rowLeaderbrd = makeRow("Leaderboard");

    {
      auto *tpDivider = new QFrame();
      tpDivider->setFrameShape(QFrame::HLine);
      tpDivider->setStyleSheet("color: rgba(0,0,0,40); margin-top: 4px;");
      healthLayout->addWidget(tpDivider);
      auto *tpHeader = new QLabel("Third Party");
      tpHeader->setStyleSheet("font-weight: 700; color: #555; padding: 2px 0 0 2px;");
      healthLayout->addWidget(tpHeader);
    }
    HealthRow rowAutoware  = makeRow("Autoware");

    HealthRow rowPlugins   = makeRow("Plugins");

    healthLayout->addStretch();

    {
      auto *footerLine = new QFrame();
      footerLine->setFrameShape(QFrame::HLine);
      footerLine->setStyleSheet("color: rgba(0,0,0,40);");
      healthLayout->addWidget(footerLine);

      healthRefreshBtn->setText(QStringLiteral("↻ Re-check"));

      auto makeLegendChip = [](const char *color, const QString &text) -> QWidget * {
        auto *w = new QWidget();
        auto *h = new QHBoxLayout(w);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(4);
        QLabel *dot = new QLabel();
        dot->setFixedSize(10, 10);
        dot->setStyleSheet(QString("background-color: %1; border-radius: 5px;").arg(color));
        h->addWidget(dot);
        QLabel *lbl = new QLabel(text);
        lbl->setStyleSheet("color: #888; font-size: 10px;");
        h->addWidget(lbl);
        return w;
      };

      auto *footerRow = new QHBoxLayout();
      footerRow->setContentsMargins(0, 4, 0, 0);
      footerRow->setSpacing(10);
      footerRow->addWidget(healthRefreshBtn);
      footerRow->addSpacing(6);
      footerRow->addWidget(healthLastRun, 0, Qt::AlignVCenter);
      footerRow->addStretch(1);
      footerRow->addWidget(makeLegendChip("#2FA66B", "OK"));
      footerRow->addWidget(makeLegendChip("#D9A13A", "optional / not found"));
      footerRow->addWidget(makeLegendChip("#C64545", "blocks launch"));
      healthLayout->addLayout(footerRow);
    }

    auto setDot = [](QLabel *dot, HealthStatus s) {
      const char *color = "#888";
      switch (s) {
        case HealthStatus::Ok:   color = "#2FA66B"; break;
        case HealthStatus::Warn: color = "#D9A13A"; break;
        case HealthStatus::Err:  color = "#C64545"; break;
        case HealthStatus::Info: color = "#7A869A"; break;
      }
      dot->setStyleSheet(QString("background-color: %1; border-radius: 6px;").arg(color));
    };

    auto applyResult = [setDot](HealthRow &row, const HealthResult &r) {
      setDot(row.dot, r.status);
      row.detail->setText(r.detail.isEmpty() ? QStringLiteral("—") : r.detail);
      row.detail->setToolTip(r.detail);
      row.action->disconnect();
      if (r.actionText.isEmpty()) {
        row.action->setVisible(false);
      } else {
        row.action->setText(r.actionText);
        if (r.action) {
          auto cb = r.action;
          QObject::connect(row.action, &QPushButton::clicked, row.action, [cb]() { cb(); });
        }
        row.action->setVisible(true);
      }
    };

    auto probeCarlaRoot = [&]() -> HealthResult {
      const QString path = carlaRootPath->text().trimmed();
      if (path.isEmpty()) {
        return { HealthStatus::Err, "not set", "Install…",
                 [&]() { openInstallerDialog(InstallerMode::InstallSdk); } };
      }
      if (!QDir(path).exists()) {
        return { HealthStatus::Err, path + " (missing)", "Install…",
                 [&]() { openInstallerDialog(InstallerMode::InstallSdk); } };
      }
      if (!rootConfiguredOk) {
        return { HealthStatus::Warn, path + " (no UE4/UE5 layout)", "Install…",
                 [&]() { openInstallerDialog(InstallerMode::InstallSdk); } };
      }
      return { HealthStatus::Ok, path, {}, {} };
    };

    auto probeEngineVer = [&]() -> HealthResult {
      const QString detected = detectInstalledCarlaVersion();
      const QString ue = detectedEngineLabel.isEmpty()
                           ? QStringLiteral("Unreal ?")
                           : detectedEngineLabel;
      if (g_sdkVersionMismatch) {
        const QString sim = g_reportedSimVersion.isEmpty() ? QStringLiteral("?") : g_reportedSimVersion;
        const QString det = detected.isEmpty() ? QStringLiteral("?") : detected;
        return { HealthStatus::Err,
                 QString("%1 · version mismatch (sim %2, client %3)").arg(ue, sim, det), {}, {} };
      }
      if (!detected.isEmpty())
        return { HealthStatus::Ok, QString("%1 · CARLA %2").arg(ue, detected), {}, {} };
      if (!g_reportedSimVersion.isEmpty())
        return { HealthStatus::Warn, QString("%1 · sim %2").arg(ue, g_reportedSimVersion), {}, {} };
      return { HealthStatus::Warn, QString("%1 · CARLA version not detected").arg(ue), {}, {} };
    };

    auto probePythonApi = [&]() -> HealthResult {
      const QString root = carlaRootPath->text().trimmed();
      if (root.isEmpty()) return { HealthStatus::Info, "needs CARLA root", {}, {} };
      QDir dist(root + "/PythonAPI/carla/dist");
      if (!dist.exists()) return { HealthStatus::Warn, "dist/ missing", {}, {} };
      const QStringList eggs = dist.entryList(
        QStringList() << "carla-*.egg" << "carla-*.whl", QDir::Files);
      if (eggs.isEmpty()) return { HealthStatus::Warn, "no egg/wheel", {}, {} };
      return { HealthStatus::Ok, eggs.first(), {}, {} };
    };

    auto probeCppApi = [&]() -> HealthResult {
      const QString root = carlaRootPath->text().trimmed();
      if (root.isEmpty()) return { HealthStatus::Info, "needs CARLA root", {}, {} };
      const QString header = root + "/LibCarla/source/carla/client/Client.h";
      if (QFileInfo(header).isFile())
        return { HealthStatus::Ok, "LibCarla source", {}, {} };
      QDir buildDir(root + "/Build");
      if (buildDir.exists()) {
        const QStringList prebuilt = buildDir.entryList(
          QStringList() << "libcarla-client-build.*", QDir::Dirs | QDir::NoDotAndDotDot);
        if (!prebuilt.isEmpty())
          return { HealthStatus::Ok, "prebuilt client lib", {}, {} };
      }
      return { HealthStatus::Warn, "headers not found", {}, {} };
    };

    auto probePlugins = [&]() -> HealthResult {
      const QString primary = pluginSearchDirs.isEmpty()
                                ? QStringLiteral("(none)")
                                : pluginSearchDirs.first();
      const QString allDirs = pluginSearchDirs.join("\n  ");
      if (pluginsLoadedCount > 0) {
        return { HealthStatus::Ok,
                 QString("%1 loaded · %2").arg(pluginsLoadedCount).arg(primary),
                 {}, {} };
      }
      return { HealthStatus::Info,
               QString("0 loaded — drop qt-*.{so,dylib,dll} into: %1").arg(primary)
                 + (pluginSearchDirs.size() > 1
                     ? QString(" (or %1 others)").arg(pluginSearchDirs.size() - 1)
                     : QString()),
               {}, {} };
    };

    auto probeDiskFree = [&]() -> HealthResult {
      const QString probePath = carlaRootPath->text().trimmed().isEmpty()
                                  ? QDir::homePath()
                                  : carlaRootPath->text().trimmed();
      QStorageInfo storage(probePath);
      if (!storage.isValid() || !storage.isReady()) {
        return { HealthStatus::Warn, "couldn't query " + probePath, {}, {} };
      }
      const qint64 freeGB = storage.bytesAvailable() / (1024LL * 1024 * 1024);
      const QString detail = QString("%1 GB free on %2").arg(freeGB).arg(storage.rootPath());
      if (freeGB < 2)  return { HealthStatus::Err,  detail, {}, {} };
      if (freeGB < 10) return { HealthStatus::Warn, detail, {}, {} };
      return { HealthStatus::Ok, detail, {}, {} };
    };

    auto probeNvidia = [&]() -> HealthResult {
      auto runNvidia = [](const QStringList &args) -> std::pair<int, QString> {
        QProcess p;
        p.setProcessChannelMode(QProcess::MergedChannels);
        p.start("nvidia-smi", args);
        if (!p.waitForStarted(1000)) return {-1, {}};
        if (!p.waitForFinished(3500)) { p.kill(); p.waitForFinished(500); return {-2, {}}; }
        return { p.exitCode(), QString::fromUtf8(p.readAll()).trimmed() };
      };

      auto [rc1, out1] = runNvidia(QStringList() << "--query-gpu=driver_version" << "--format=csv,noheader");
      if (rc1 == 0 && !out1.isEmpty()) {
        return { HealthStatus::Ok, "driver " + out1.split('\n').first().trimmed(), {}, {} };
      }

      auto [rc2, out2] = runNvidia(QStringList());
      if (rc2 == 0 && !out2.isEmpty()) {
        QRegularExpression re(QStringLiteral("Driver Version:\\s*(\\S+)"));
        auto m = re.match(out2);
        if (m.hasMatch()) {
          return { HealthStatus::Ok, "driver " + m.captured(1), {}, {} };
        }
        return { HealthStatus::Ok, "nvidia-smi ok (version not parsed)", {}, {} };
      }

      if (rc1 == -1 && rc2 == -1) {
        return { HealthStatus::Warn, "nvidia-smi not found (ok on non-NVIDIA hosts)", {}, {} };
      }
      if (rc1 == -2 || rc2 == -2) {
        return { HealthStatus::Warn, "nvidia-smi timed out (driver slow to respond)", {}, {} };
      }
      const QString tail = (out2.isEmpty() ? out1 : out2).left(100).replace('\n', ' ');
      return { HealthStatus::Warn,
               QString("nvidia-smi failed (rc=%1): %2").arg(rc1).arg(tail), {}, {} };
    };

    auto probeRos2 = [&]() -> HealthResult {
      QProcess p;
      p.start("/bin/bash", QStringList() << "-lc"
        << "if [ -n \"$CARLA_STUDIO_ROS_PATH\" ] && [ -f \"$CARLA_STUDIO_ROS_PATH/setup.bash\" ]; then "
             "source \"$CARLA_STUDIO_ROS_PATH/setup.bash\" 2>/dev/null; fi; "
           "if command -v ros2 >/dev/null 2>&1; then "
             "echo \"path:${ROS_DISTRO:-?}\"; "
           "elif [ -n \"$ROS_DISTRO\" ]; then "
             "echo \"env:$ROS_DISTRO\"; "
           "elif ls /opt/ros/*/setup.bash >/dev/null 2>&1; then "
             "for d in /opt/ros/*/setup.bash; do echo \"sysprefix:$(basename $(dirname $d))\"; break; done; "
           "else echo missing; fi");
      p.waitForFinished(1500);
      const QString out = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
      if (out == "missing" || out.isEmpty()) {
        return { HealthStatus::Warn,
                 "not detected (set $CARLA_STUDIO_ROS_PATH for source builds)", {}, {} };
      }
      const int colon = out.indexOf(':');
      const QString kind   = colon >= 0 ? out.left(colon) : out;
      const QString distro = colon >= 0 ? out.mid(colon + 1) : QString();
      QString detail;
      if (kind == "path")      detail = distro + " (ros2 on PATH)";
      else if (kind == "env")  detail = distro + " (env only — source setup.bash)";
      else if (kind == "sysprefix") detail = distro + " (/opt/ros — not sourced)";
      else                     detail = out;
      return { HealthStatus::Ok, detail, {}, {} };
    };

    auto probeScenarioRunner = [&]() -> HealthResult {
      const QString env = QString::fromLocal8Bit(qgetenv("SCENARIO_RUNNER_ROOT"));
      if (!env.isEmpty() && QDir(env).exists()) {
        return { HealthStatus::Ok, env, {}, {} };
      }
      const QString root = carlaRootPath->text().trimmed();
      if (!root.isEmpty()) {
        const QString sibling = QDir::cleanPath(QDir(root).absoluteFilePath("../scenario_runner"));
        if (QFileInfo(sibling + "/scenario_runner.py").exists()) {
          return { HealthStatus::Ok, sibling, {}, {} };
        }
      }
      return { HealthStatus::Warn, "not detected (set $SCENARIO_RUNNER_ROOT)", "Docs",
               []() { QDesktopServices::openUrl(
                  QUrl("https://github.com/carla-simulator/scenario_runner")); } };
    };

    auto probeLeaderboard = [&]() -> HealthResult {
      const QString env = QString::fromLocal8Bit(qgetenv("LEADERBOARD_ROOT"));
      if (!env.isEmpty() && QDir(env).exists()) {
        return { HealthStatus::Ok, env, {}, {} };
      }
      const QString root = carlaRootPath->text().trimmed();
      if (!root.isEmpty()) {
        const QString sibling = QDir::cleanPath(QDir(root).absoluteFilePath("../leaderboard"));
        if (QDir(sibling).exists()) {
          return { HealthStatus::Ok, sibling, {}, {} };
        }
      }
      return { HealthStatus::Warn, "not detected (set $LEADERBOARD_ROOT)", "Docs",
               []() { QDesktopServices::openUrl(
                  QUrl("https://leaderboard.carla.org/")); } };
    };

    auto probeRosBridge = [&]() -> HealthResult {
      auto ros2OnPath = []() {
        QProcess q; q.start("ros2", QStringList() << "--help");
        return q.waitForStarted(600) && q.waitForFinished(1500);
      };
      const bool nativeAvailable = ros2OnPath();

      bool bridgeInstalled = false;
      QString bridgePath;
      {
        QProcess p;
        p.start("ros2", QStringList() << "pkg" << "list");
        if (p.waitForStarted(600) && p.waitForFinished(1500)) {
          const QString out = QString::fromUtf8(p.readAllStandardOutput());
          if (out.contains("carla_ros_bridge")) {
            bridgeInstalled = true;
            bridgePath = QStringLiteral("carla_ros_bridge (ROS 2 pkg)");
          }
        }
      }
      if (!bridgeInstalled) {
        const QString root = carlaRootPath->text().trimmed();
        if (!root.isEmpty()) {
          for (const QString &name : { QStringLiteral("../ros-bridge"),
                                       QStringLiteral("../carla-ros-bridge") }) {
            const QString sibling = QDir::cleanPath(QDir(root).absoluteFilePath(name));
            if (QDir(sibling).exists()) {
              bridgeInstalled = true;
              bridgePath = sibling;
              break;
            }
          }
        }
      }

      if (nativeAvailable) {
        if (bridgeInstalled) {
          return { HealthStatus::Info,
                   QString("installed (%1) — superseded by native ROS 2, pick one").arg(bridgePath),
                   "Docs", []() { QDesktopServices::openUrl(
                      QUrl("https://carla.org/2025/09/16/release-0.9.16/")); } };
        }
        return { HealthStatus::Info, "not needed — native ROS 2 available", {}, {} };
      }
      if (bridgeInstalled) {
        return { HealthStatus::Ok, bridgePath, {}, {} };
      }
      return { HealthStatus::Warn, "not detected", "Docs",
               []() { QDesktopServices::openUrl(
                  QUrl("https://github.com/carla-simulator/ros-bridge")); } };
    };

    auto probeAutoware = [&]() -> HealthResult {
      const QStringList candidates = {
        QStringLiteral("/opt/autoware"),
        QDir::homePath() + "/autoware",
        QDir::homePath() + "/autoware_universe",
      };
      for (const QString &c : candidates) {
        if (QDir(c).exists()) return { HealthStatus::Ok, c, {}, {} };
      }
      QProcess p;
      p.start("which", QStringList() << "autoware");
      if (p.waitForStarted(400) && p.waitForFinished(1000) && p.exitCode() == 0) {
        const QString line = QString::fromUtf8(p.readAllStandardOutput()).trimmed();
        if (!line.isEmpty()) return { HealthStatus::Ok, line, {}, {} };
      }
      return { HealthStatus::Warn, "not detected", "Docs",
               []() { QDesktopServices::openUrl(QUrl("https://autoware.org/")); } };
    };

    auto probeSdkVsSim = [&]() -> HealthResult {
      if (g_sdkVersionMismatch) {
        const QString simVer = g_reportedSimVersion.isEmpty()
                                 ? QStringLiteral("?")
                                 : g_reportedSimVersion;
#ifdef CARLA_STUDIO_WITH_LIBCARLA
        const QString cliVer = QString::fromLatin1(carla::version());
#else
        const QString cliVer = QStringLiteral("(unlinked)");
#endif
        return { HealthStatus::Err,
                 QString("client %1 vs sim %2 — incompatible RPC").arg(cliVer, simVer),
                 "Install / Update CARLA",
                 [&]() { openInstallerDialog(InstallerMode::InstallSdk); } };
      }
      return { HealthStatus::Ok, "client matches running sim (or not probed yet)",
               QString(), nullptr };
    };

    auto runHealthChecks = [=, &window]() mutable {
      applyResult(rowCarlaRoot, probeCarlaRoot());
      applyResult(rowEngineVer, probeEngineVer());
      applyResult(rowVersionCompat, probeSdkVsSim());
      applyResult(rowPythonApi, probePythonApi());
      applyResult(rowCppApi,    probeCppApi());
      applyResult(rowDiskFree,  probeDiskFree());
      applyResult(rowNvidia,    probeNvidia());
      applyResult(rowRos2,      probeRos2());
      applyResult(rowRosBridge, probeRosBridge());
      applyResult(rowScenarioR, probeScenarioRunner());
      applyResult(rowLeaderbrd, probeLeaderboard());
      applyResult(rowAutoware,  probeAutoware());
      applyResult(rowPlugins,   probePlugins());
      healthLastRun->setText(
        "Last checked: " + QDateTime::currentDateTime().toString("hh:mm:ss"));
      Q_UNUSED(&window);
    };

    QObject::connect(healthRefreshBtn, &QPushButton::clicked, healthCheckPage,
                     [runHealthChecks]() mutable { runHealthChecks(); });

    class FirstShowProbeFilter : public QObject {
     public:
      std::function<void()> runOnce;
      bool done = false;
      bool eventFilter(QObject *, QEvent *ev) override {
        if (!done && ev->type() == QEvent::Show) {
          done = true;
          if (runOnce) QTimer::singleShot(0, [fn = runOnce]() { fn(); });
        }
        return false;
      }
    };
    auto *showFilter = new FirstShowProbeFilter();
    showFilter->setParent(healthCheckPage);
    showFilter->runOnce = [runHealthChecks]() mutable { runHealthChecks(); };
    healthCheckPage->installEventFilter(showFilter);
  }

  validateCarlaRoot();
  populateScenarios();
  refreshRuntimeOptions();
  updateEndpoint();
  refreshProcessList();

  // 2-second periodic poll so the Process table tracks every UE editor /
  // commandlet / simulator launched by Studio buttons (Import, Drive,
  // Export) from launch through to closure, without needing each call site
  // to explicitly notify the table.
  {
    QTimer *procPoll = new QTimer(&window);
    procPoll->setInterval(2000);
    QObject::connect(procPoll, &QTimer::timeout, &window,
                     [refreshProcessList]() { refreshProcessList(); });
    procPoll->start();
  }

  if (!trackedCarlaPids.isEmpty()) {
    setSimulationStatus("Initializing");
    killAllCarlaProcesses();
    forceStopTimer->start(60000);
    QTimer::singleShot(1500, &window, [&]() {
      refreshProcessList();
      if (trackedCarlaPids.isEmpty()) {
        forceStopTimer->stop();
        setSimulationStatus("Idle");
      }
    });
  } else {
    setSimulationStatus("Idle");
  }

  l1->addWidget(launchGroup,       0, 0);
  l1->addWidget(integrationsGroup, 1, 0);
  l1->addWidget(perfGroup,         2, 0);
  {
    QWidget *statusRow = new QWidget();
    QHBoxLayout *statusRowLayout = new QHBoxLayout(statusRow);
    statusRowLayout->setContentsMargins(0, 0, 0, 0);
    statusRowLayout->addWidget(statusLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    statusRowLayout->addStretch(1);
    statusRowLayout->addWidget(apiWarningLabel, 0, Qt::AlignRight | Qt::AlignVCenter);
    l1->addWidget(statusRow, 3, 0);
  }
  l1->setColumnStretch(0, 1);
  l1->setRowStretch(0, 2);
  l1->setRowStretch(1, 3);
  l1->setRowStretch(2, 3);
  l1->setRowStretch(3, 0);
  w1->setLayout(l1);
  QWidget *w2 = new QWidget();
  QVBoxLayout *interfacesLayout = new QVBoxLayout();
  interfacesLayout->setContentsMargins(2, 2, 2, 2);
  interfacesLayout->setSpacing(1);
  QHBoxLayout *interfacesContentLayout = new QHBoxLayout();
  interfacesContentLayout->setContentsMargins(0, 0, 0, 0);
  interfacesContentLayout->setSpacing(0);

  QWidget *leftPane = new QWidget();
  QVBoxLayout *leftPaneLayout = new QVBoxLayout();
  leftPaneLayout->setContentsMargins(0, 0, 0, 0);
  QLabel *senseHeading = new QLabel("Sense");
  senseHeading->setAlignment(Qt::AlignCenter);
  senseHeading->setStyleSheet("font-weight: 700;");
  leftPaneLayout->addWidget(senseHeading);

  QGroupBox *vehicleGroup = new QGroupBox("Vehicle");
  vehicleGroup->setToolTip(
    "Default ego-vehicle settings. Per-POV overrides live on the Actuate "
    "pane (each player can pin its own colour + finish).");
  QFormLayout *vehicleForm = new QFormLayout();
  vehicleForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
  vehicleForm->setHorizontalSpacing(8);
  vehicleForm->setVerticalSpacing(4);
  vehicleForm->setContentsMargins(8, 4, 8, 4);

  QComboBox *vehicleBlueprintCombo = new QComboBox();
  vehicleBlueprintCombo->setToolTip(
    "CARLA blueprint ID (vehicle.* family). Spawns this model as the "
    "ego at simulation start. Refresh from the live BlueprintLibrary "
    "via Apply on the Sense pane.");
  const QStringList kDefaultVehicleBlueprints = {
    "vehicle.lincoln.mkz_2020",
    "vehicle.lincoln.mkz_2017",
    "vehicle.tesla.model3",
    "vehicle.audi.a2",
    "vehicle.audi.tt",
    "vehicle.audi.etron",
    "vehicle.bmw.grandtourer",
    "vehicle.chevrolet.impala",
    "vehicle.citroen.c3",
    "vehicle.dodge.charger_2020",
    "vehicle.dodge.charger_police",
    "vehicle.ford.mustang",
    "vehicle.jeep.wrangler_rubicon",
    "vehicle.mercedes.coupe",
    "vehicle.mini.cooper_s",
    "vehicle.nissan.micra",
    "vehicle.nissan.patrol",
    "vehicle.seat.leon",
    "vehicle.toyota.prius",
    "vehicle.harley-davidson.low_rider",
    "vehicle.kawasaki.ninja",
    "vehicle.yamaha.yzf",
    "vehicle.bh.crossbike",
    "vehicle.diamondback.century",
    "vehicle.gazelle.omafiets",
    "vehicle.carlamotors.carlacola",
    "vehicle.carlamotors.firetruck",
    "vehicle.tesla.cybertruck",
    "vehicle.volkswagen.t2",
    "vehicle.mercedes.sprinter",
    "vehicle.ford.ambulance",
  };
  for (const QString &bp : kDefaultVehicleBlueprints) {
    vehicleBlueprintCombo->addItem(bp);
  }
  {
    const QString saved = QSettings().value(
      "vehicle/blueprint", "vehicle.lincoln.mkz_2020").toString();
    const int idx = vehicleBlueprintCombo->findText(saved);
    if (idx >= 0) vehicleBlueprintCombo->setCurrentIndex(idx);
    else vehicleBlueprintCombo->setCurrentText(saved);
  }
  QObject::connect(vehicleBlueprintCombo, &QComboBox::currentTextChanged,
    &window, [](const QString &s) {
      QSettings().setValue("vehicle/blueprint", s);
    });
  vehicleForm->addRow("Blueprint:", vehicleBlueprintCombo);

  QHBoxLayout *colorRow = new QHBoxLayout();
  colorRow->setContentsMargins(0, 0, 0, 0);
  QPushButton *colorSwatch = new QPushButton();
  colorSwatch->setFixedSize(60, 22);
  colorSwatch->setToolTip(
    "Click to pick a body colour. Drives CARLA's <code>color</code> "
    "blueprint attribute when spawning the ego.");
  QLabel *colorHexLabel = new QLabel();
  colorHexLabel->setStyleSheet("font-family: monospace; color: #555;");
  auto applyVehicleColor = [colorSwatch, colorHexLabel](const QColor &c) {
    colorSwatch->setStyleSheet(
      QString("QPushButton { background:%1; border:1px solid #888; "
              "border-radius:3px; }").arg(c.name()));
    colorHexLabel->setText(c.name(QColor::HexRgb).toUpper());
  };
  {
    const QString savedHex = QSettings().value("vehicle/color", "#1E1E1E").toString();
    const QColor c(savedHex);
    applyVehicleColor(c.isValid() ? c : QColor("#1E1E1E"));
  }
  QObject::connect(colorSwatch, &QPushButton::clicked, &window,
    [applyVehicleColor]() {
      const QString cur = QSettings().value("vehicle/color", "#1E1E1E").toString();
      QColor picked = QColorDialog::getColor(
        QColor(cur), nullptr, "Vehicle body colour");
      if (picked.isValid()) {
        applyVehicleColor(picked);
        QSettings().setValue("vehicle/color", picked.name(QColor::HexRgb));
      }
    });
  colorRow->addWidget(colorSwatch);
  colorRow->addWidget(colorHexLabel);
  colorRow->addStretch();
  QWidget *colorHost = new QWidget();
  colorHost->setLayout(colorRow);
  vehicleForm->addRow("Colour:", colorHost);

  QComboBox *vehicleFinishCombo = new QComboBox();
  vehicleFinishCombo->addItem("Standard (universal)", "standard");
  vehicleFinishCombo->addItem("Clear-Coat",            "clearcoat");
  vehicleFinishCombo->addItem("Matte",                 "matte");
  vehicleFinishCombo->addItem("Metallic Pearlescent",  "pearl");
  vehicleFinishCombo->addItem("Triple-Coat Show Car",  "triplecoat");
  {
    const QString saved = QSettings().value("vehicle/finish", "standard").toString();
    const int idx = vehicleFinishCombo->findData(saved);
    vehicleFinishCombo->setCurrentIndex(idx >= 0 ? idx : 0);
  }
  auto detectUe5Available = [&]() -> bool {
    const QString root = carlaRootPath->text().trimmed();
    if (root.isEmpty()) return false;
    return QFileInfo(root + "/CarlaUE5.sh").exists() ||
           QDir(root + "/CarlaUE5").exists() ||
           QFileInfo(root + "/CarlaUnreal.sh").exists() ||  
           QDir(root + "/CarlaUnreal").exists();
  };
  if (!detectUe5Available()) {
    QStandardItemModel *finishModel =
      qobject_cast<QStandardItemModel *>(vehicleFinishCombo->model());
    if (finishModel) {
      for (int i = 1; i < vehicleFinishCombo->count(); ++i) {
        QStandardItem *it = finishModel->item(i);
        if (it) {
          it->setFlags(it->flags() & ~Qt::ItemIsEnabled);
          it->setToolTip(
            "Substrate-only finish. Requires CARLA UE5 + Epic's "
            "Automotive Substrate Materials pack imported. Switch to "
            "the UE5 build of CARLA via Cfg → Install / Update CARLA.");
        }
      }
    }
    if (vehicleFinishCombo->currentData().toString() != "standard") {
      vehicleFinishCombo->setCurrentIndex(0);
    }
  }
  vehicleFinishCombo->setToolTip(
    "Paint finish. Standard works on every CARLA build. The Substrate-"
    "based finishes (Clear-Coat / Matte / Pearlescent / Triple-Coat) "
    "need CARLA's UE5 build with Epic's free Automotive Substrate pack "
    "from Fab.");
  QObject::connect(vehicleFinishCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged), &window,
    [vehicleFinishCombo](int) {
      QSettings().setValue("vehicle/finish",
                           vehicleFinishCombo->currentData().toString());
    });
  vehicleForm->addRow("Finish:", vehicleFinishCombo);

  QLabel *vehicleFooter = new QLabel(
    "<small style='color:#777'>Per-POV overrides on the Actuate pane.</small>");
  vehicleFooter->setTextFormat(Qt::RichText);
  vehicleForm->addRow("", vehicleFooter);

  vehicleGroup->setLayout(vehicleForm);

  QGroupBox *sensorGroup = new QGroupBox();
  QVBoxLayout *sensorLayout = new QVBoxLayout();

  std::function<void(const QString &)> openSensorConfigDialog;
  std::function<void(const QString &)> startSensorPreview;

  struct SensorMountConfig {
    QString framePreset;
    double tx;
    double ty;
    double tz;
    double rx;
    double ry;
    double rz;
  };

  const double vehicleLengthM = 4.90648;
  const double frontCenterXRearBaselinkM = 3.792;
  const double frontCenterYRearBaselinkM = 0.0;
  const double frontCenterZRearBaselinkM = 0.365;

  auto frameOriginRearBaselink = [&](const QString &framePreset) -> std::pair<double, double> {
    if (framePreset == "Rear Baselink") {
      return {0.0, 0.0};
    }
    if (framePreset == "Front Baselink") {
      return {2.776, 0.0};
    }
    if (framePreset == "Vehicle Center") {
      return {1.38724, 0.358};
    }
    if (framePreset == "Rear Center") {
      return {-1.066, 0.0};
    }
    if (framePreset == "Front Center") {
      return {vehicleLengthM - 1.066, 0.0};
    }
    return {0.0, 0.0};
  };

  auto frontCenterMountForFrame = [&](const QString &framePreset) -> SensorMountConfig {
    const auto origin = frameOriginRearBaselink(framePreset);
    SensorMountConfig mount;
    mount.framePreset = framePreset;
    mount.tx = frontCenterXRearBaselinkM - origin.first;
    mount.ty = frontCenterYRearBaselinkM;
    mount.tz = frontCenterZRearBaselinkM - origin.second;
    mount.rx = 0.0;
    mount.ry = 0.0;
    mount.rz = 0.0;
    return mount;
  };

  QMap<QString, SensorMountConfig> sensorMountConfigs;
  QMap<QString, int> sensorInstanceCount;
  using carla::studio::core::sensorMountKey;

  struct CameraOptics {
    QString preset = "(Default rectilinear)";
    int    imgW = 800;
    int    imgH = 450;
    double fov = 90.0;
    double lensK = 0.0;
    double lensKcube = 0.0;
    double lensXSize = 0.08;
    double lensYSize = 0.08;
    double lensCircleFalloff = 5.0;
    double lensCircleMultiplier = 1.0;
    double chromaticAberration = 0.0;
  };
  QMap<QString, CameraOptics> cameraOpticsByName;

  auto isCameraSensorName = [](const QString &name) -> bool {
    static const QSet<QString> kCameraNames = {
      "RGB", "Depth", "Semantic Seg", "Instance Seg",
      "DVS", "Optical Flow", "Fisheye"
    };
    return kCameraNames.contains(name);
  };

  struct LensFit {
    double k = 0.0, kcube = 0.0;
    double fov = 90.0;
    double vignetteFalloff = 5.0;
    double vignetteMultiplier = 1.0;
    bool   hasVignette = false;
  };
  auto fitRadialLensCsv = [](const QString &csvText, const QString &nameHint) -> LensFit {
    LensFit out;
    if (nameHint.contains("190")) out.fov = 190;
    else if (nameHint.contains("180")) out.fov = 180;
    else if (nameHint.contains("Fisheye", Qt::CaseInsensitive)) out.fov = 180;
    else out.fov = 90;

    bool inHeader = false;
    QVector<QPair<double, double>> rows;
    QVector<double> vignetteSamples;
    for (const QString &lineRaw : csvText.split('\n', Qt::SkipEmptyParts)) {
      QString line = lineRaw.trimmed();
      if (line.isEmpty() || line.startsWith('#')) continue;
      if (line.startsWith("HEADER_START", Qt::CaseInsensitive)) { inHeader = true; continue; }
      if (line.startsWith("HEADER_END",   Qt::CaseInsensitive)) { inHeader = false; continue; }
      if (inHeader) continue;
      const QStringList parts = line.split(';', Qt::SkipEmptyParts);
      if (parts.size() < 2) continue;
      bool aOk = false, bOk = false;
      const double r  = parts[0].toDouble(&aOk);
      const double rp = parts[1].toDouble(&bOk);
      if (!aOk || !bOk) continue;
      rows.append({ r, rp });
      if (parts.size() >= 3) {
        bool vOk = false;
        const double vv = parts[2].toDouble(&vOk);
        if (vOk) vignetteSamples.append(vv);
      }
    }
    if (rows.size() < 4) return out;

    double Srr = 0, Sr3r3 = 0, Srr3 = 0, Srrp = 0, Sr3rp = 0;
    for (const auto &p : rows) {
      const double r  = p.first;
      const double rp = p.second;
      const double r3 = r * r * r;
      Srr   += r  * r;
      Sr3r3 += r3 * r3;
      Srr3  += r  * r3;
      Srrp  += r  * rp;
      Sr3rp += r3 * rp;
    }
    const double det = Srr * Sr3r3 - Srr3 * Srr3;
    if (std::abs(det) > 1e-12) {
      out.k     = ( Sr3r3 * Srrp  - Srr3  * Sr3rp) / det;
      out.kcube = (-Srr3  * Srrp  + Srr   * Sr3rp) / det;
    }
    if (!vignetteSamples.isEmpty()) {
      out.hasVignette = true;
      double rAtHalf = 1.0;
      for (int i = 0; i < std::min(rows.size(), vignetteSamples.size()); ++i) {
        if (vignetteSamples[i] >= 0.5) { rAtHalf = rows[i].first; break; }
      }
      if (rAtHalf > 0.05 && rAtHalf < 0.99) {
        out.vignetteFalloff = std::log(0.5) / std::log(rAtHalf);
        out.vignetteFalloff = std::max(1.0, std::min(10.0, out.vignetteFalloff));
      }
      out.vignetteMultiplier = 1.0;
    }
    return out;
  };

  const QString sensorConfigDir = QString::fromLocal8Bit(qgetenv("CARLA_WORKSPACE_ROOT")).isEmpty()
    ? QDir::homePath() + "/.carla_studio"
    : QString::fromLocal8Bit(qgetenv("CARLA_WORKSPACE_ROOT")) + "/.carla_studio";
  const QString sensorConfigFilePath = sensorConfigDir + "/sensor_mounts.json";

  auto saveSensorMountConfigs = [&]() {
    QDir().mkpath(sensorConfigDir);
    QJsonObject root;
    for (auto it = sensorMountConfigs.constBegin(); it != sensorMountConfigs.constEnd(); ++it) {
      const SensorMountConfig &mount = it.value();
      QJsonObject entry;
      entry.insert("framePreset", mount.framePreset);
      entry.insert("tx", mount.tx);
      entry.insert("ty", mount.ty);
      entry.insert("tz", mount.tz);
      entry.insert("rx", mount.rx);
      entry.insert("ry", mount.ry);
      entry.insert("rz", mount.rz);
      root.insert(it.key(), entry);
    }
    QFile outFile(sensorConfigFilePath);
    if (outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      outFile.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
      outFile.close();
    }
  };

  auto loadSensorMountConfigs = [&]() {
    QFile inFile(sensorConfigFilePath);
    if (!inFile.open(QIODevice::ReadOnly)) {
      return;
    }
    const QByteArray payload = inFile.readAll();
    inFile.close();
    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(payload, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
      return;
    }
    const QJsonObject root = doc.object();
    for (auto it = root.begin(); it != root.end(); ++it) {
      if (!it.value().isObject()) {
        continue;
      }
      const QJsonObject entry = it.value().toObject();
      SensorMountConfig mount;
      mount.framePreset = entry.value("framePreset").toString("Rear Baselink");
      mount.tx = entry.value("tx").toDouble(0.0);
      mount.ty = entry.value("ty").toDouble(0.0);
      mount.tz = entry.value("tz").toDouble(0.0);
      mount.rx = entry.value("rx").toDouble(0.0);
      mount.ry = entry.value("ry").toDouble(0.0);
      mount.rz = entry.value("rz").toDouble(0.0);
      sensorMountConfigs.insert(it.key(), mount);
    }
  };

  loadSensorMountConfigs();
  auto getOrCreateSensorMount = [&](const QString &sensorName) -> SensorMountConfig {
    if (!sensorMountConfigs.contains(sensorName)) {
      sensorMountConfigs.insert(sensorName, frontCenterMountForFrame("Rear Baselink"));
    }
    return sensorMountConfigs.value(sensorName);
  };

  class VerticalTitleLabel : public QLabel {
   public:
    explicit VerticalTitleLabel(const QString &t, QWidget *p = nullptr)
        : QLabel(t, p) {
      setAlignment(Qt::AlignCenter);
      setStyleSheet("letter-spacing: 0.5px; font-weight: 400;");
      setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    QSize sizeHint() const override {
      const QFontMetrics fm(font());
      return QSize(fm.height() + 4,
                    fm.horizontalAdvance(text()) + 16);
    }
    QSize minimumSizeHint() const override { return sizeHint(); }
    void paintEvent(QPaintEvent *) override {
      QPainter p(this);
      p.setRenderHint(QPainter::TextAntialiasing);
      p.setPen(palette().color(QPalette::WindowText));
      p.setFont(font());
      p.translate(0, height());
      p.rotate(-90);
      QRect r(0, 0, height(), width());
      p.drawText(r, Qt::AlignCenter, text());
    }
  };
  auto makeVerticalTitle = [&](const QString &title) -> QLabel * {
    return new VerticalTitleLabel(title);
  };

  QIcon sensorAssemblyIcon;
  {
    QPixmap pm(28, 28);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);

    const QColor stroke = app.palette().color(QPalette::ButtonText);
    QPen linePen(stroke, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p.setPen(linePen);
    p.setBrush(Qt::NoBrush);

    QPainterPath car;
    car.moveTo(4, 24);
    car.lineTo(4, 21);
    car.cubicTo(4, 18, 7, 17, 10, 17);
    car.lineTo(12, 14);
    car.lineTo(20, 14);
    car.lineTo(22, 17);
    car.cubicTo(25, 17, 26, 19, 26, 21);
    car.lineTo(26, 24);
    p.drawPath(car);
    p.drawLine(QPointF(4, 24), QPointF(26, 24));
    p.drawEllipse(QPointF(9, 24), 2.0, 2.0);
    p.drawEllipse(QPointF(21, 24), 2.0, 2.0);
    p.drawLine(QPointF(12, 14), QPointF(12, 17));
    p.drawLine(QPointF(20, 14), QPointF(20, 17));

    p.drawEllipse(QPointF(3.5, 14), 1.6, 1.6);
    p.drawLine(QPointF(3.5, 14), QPointF(8, 5.5));
    p.drawEllipse(QPointF(8, 5.5), 1.4, 1.4);
    p.drawLine(QPointF(8, 5.5), QPointF(12.5, 9.5));
    p.drawLine(QPointF(12.5, 9.5), QPointF(12.5, 11.5));
    p.drawLine(QPointF(12.5, 11.5), QPointF(11.0, 13.0));
    p.drawLine(QPointF(12.5, 11.5), QPointF(14.0, 13.0));

    const QPointF gc(21.5, 6.0);
    const double rIn = 2.4, rOut = 3.5;
    for (int i = 0; i < 8; ++i) {
      const double a = i * (M_PI / 4.0);
      p.drawLine(QPointF(gc.x() + rIn * std::cos(a),
                          gc.y() + rIn * std::sin(a)),
                  QPointF(gc.x() + rOut * std::cos(a),
                          gc.y() + rOut * std::sin(a)));
    }
    p.drawEllipse(gc, rIn, rIn);
    p.setBrush(stroke);
    p.drawEllipse(gc, 0.7, 0.7);
    p.setBrush(Qt::NoBrush);

    sensorAssemblyIcon = QIcon(pm);
  }

  auto makeCompactCategory = [&](const QString &title, const QStringList &sensorNames, QLabel *countText,
                                 std::vector<QCheckBox *> &checks, std::vector<QPushButton *> &gears,
                                 std::vector<QPushButton *> &pluses, std::vector<QLabel *> &sensorCountLabels,
                                 std::vector<int> &sensorCounts) {
    QGroupBox *box = new QGroupBox();
    QHBoxLayout *row = new QHBoxLayout();

    QGridLayout *titleColumn = new QGridLayout();
    titleColumn->setContentsMargins(0, 0, 0, 0);
    titleColumn->setSpacing(0);
    titleColumn->addWidget(countText, 0, 0,
                            Qt::AlignTop | Qt::AlignHCenter);
    titleColumn->addWidget(makeVerticalTitle(title), 0, 0,
                            Qt::AlignVCenter | Qt::AlignHCenter);
    QWidget *titleColumnHost = new QWidget();
    titleColumnHost->setLayout(titleColumn);
    titleColumnHost->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    row->setContentsMargins(2, 2, 6, 2);
    row->addWidget(titleColumnHost);

    QVBoxLayout *inner = new QVBoxLayout();
    for (const QString &sensorName : sensorNames) {
      QHBoxLayout *sensorRow = new QHBoxLayout();
      QCheckBox *sensorCheck = new QCheckBox(sensorName);
      sensorCheck->setStyleSheet(
        "QCheckBox { spacing: 0; padding-right: 0; }"
        "QCheckBox::indicator { margin: 0; }");
      sensorCheck->setProperty("sensorName", sensorName);
      sensorCheck->setText("");

      QPushButton *sensorGear = new QPushButton();
      sensorGear->setIcon(sensorAssemblyIcon);
      sensorGear->setIconSize(QSize(20, 20));
      sensorGear->setToolTip("Configure sensor mount, transform, and preset.");
      QPushButton *sensorPlus = new QPushButton("±");
      sensorPlus->setToolTip("Left-click: add one  ·  Right-click: remove one");
      sensorPlus->setContextMenuPolicy(Qt::CustomContextMenu);
      QLabel *sensorNameLabel = new QLabel(sensorName);
      sensorNameLabel->setTextInteractionFlags(Qt::NoTextInteraction);
      QLabel *sensorCount = new QLabel("x0");
      sensorGear->setFixedWidth(30);
      sensorPlus->setFixedSize(16, 16);
      sensorPlus->setStyleSheet(
        "QPushButton { padding: 0; margin: 0; border: 1px solid #b0b0b0; "
        "  border-radius: 2px; font-size: 11px; }");
      sensorCount->setMinimumWidth(30);
      sensorCount->setAlignment(Qt::AlignCenter);
      sensorGear->setEnabled(false);
      QObject::connect(sensorGear, &QPushButton::clicked, &window, [&, sensorName]() {
        if (openSensorConfigDialog) {
          openSensorConfigDialog(sensorName);
        }
      });
      sensorNameLabel->installEventFilter(new QObject(sensorNameLabel));
      QObject::connect(sensorCheck, &QCheckBox::toggled, sensorNameLabel,
                        [sensorNameLabel](bool checked) {
        sensorNameLabel->setStyleSheet(checked ? "" : "color: #999;");
      });
      sensorNameLabel->setStyleSheet("color: #999;");
      class _LabelClickToggle : public QObject {
       public:
        QCheckBox *target;
        _LabelClickToggle(QCheckBox *t, QObject *p) : QObject(p), target(t) {}
        bool eventFilter(QObject *, QEvent *e) override {
          if (e->type() == QEvent::MouseButtonRelease && target) {
            target->setChecked(!target->isChecked());
            return true;
          }
          return false;
        }
      };
      sensorNameLabel->installEventFilter(new _LabelClickToggle(sensorCheck, sensorNameLabel));
      sensorNameLabel->setCursor(Qt::PointingHandCursor);

      checks.push_back(sensorCheck);
      gears.push_back(sensorGear);
      pluses.push_back(sensorPlus);
      sensorCountLabels.push_back(sensorCount);
      sensorCounts.push_back(0);
      sensorCount->setVisible(false);
      QPushButton *sensorPreview = new QPushButton("⊞");
      sensorPreview->setFixedSize(16, 16);
      sensorPreview->setStyleSheet(
        "QPushButton { padding: 0; margin: 0; border: 1px solid #b0b0b0; "
        "  border-radius: 2px; font-size: 11px; }");
      sensorPreview->setToolTip(QString(
        "Open a live preview tile for this sensor (%1).").arg(sensorName));
      QObject::connect(sensorPreview, &QPushButton::clicked, &window,
        [&, sensorName]() {
          if (openSensorConfigDialog) openSensorConfigDialog(sensorName);
        });
      sensorRow->addWidget(sensorCheck);
      sensorRow->addWidget(sensorPlus);
      sensorRow->addWidget(sensorNameLabel, 1);
      sensorRow->addWidget(sensorGear);
      sensorRow->addWidget(sensorPreview);
      inner->addLayout(sensorRow);
    }

    row->addLayout(inner, 1);
    box->setLayout(row);
    return box;
  };

  std::vector<QCheckBox *> cameraChecks;
  std::vector<QPushButton *> cameraGears;
  std::vector<QPushButton *> cameraPluses;
  std::vector<QLabel *> cameraSensorCountLabels;
  std::vector<int> cameraSensorCounts;
  QLabel *cameraCountLabel = new VerticalTitleLabel("×0");
  QGroupBox *cameraCategory = makeCompactCategory("Camera", QStringList()
    << "RGB" << "Depth" << "Semantic Seg" << "Instance Seg" << "DVS" << "Optical Flow" << "Fisheye",
    cameraCountLabel, cameraChecks, cameraGears, cameraPluses, cameraSensorCountLabels, cameraSensorCounts);
  sensorLayout->addWidget(cameraCategory);

  std::vector<QCheckBox *> radarChecks;
  std::vector<QPushButton *> radarGears;
  std::vector<QPushButton *> radarPluses;
  std::vector<QLabel *> radarSensorCountLabels;
  std::vector<int> radarSensorCounts;
  QLabel *radarCountLabel = new VerticalTitleLabel("×0");
  QGroupBox *radarCategory = makeCompactCategory("RADAR", QStringList()
    << "Front" << "Rear" << "Long-Range",
    radarCountLabel, radarChecks, radarGears, radarPluses, radarSensorCountLabels, radarSensorCounts);
  sensorLayout->addWidget(radarCategory);

  std::vector<QCheckBox *> lidarChecks;
  std::vector<QPushButton *> lidarGears;
  std::vector<QPushButton *> lidarPluses;
  std::vector<QLabel *> lidarSensorCountLabels;
  std::vector<int> lidarSensorCounts;
  QLabel *lidarCountLabel = new VerticalTitleLabel("×0");
  QGroupBox *lidarCategory = makeCompactCategory("LiDAR", QStringList()
    << "Ray Cast" << "Semantic" << "Roof 360",
    lidarCountLabel, lidarChecks, lidarGears, lidarPluses, lidarSensorCountLabels, lidarSensorCounts);
  sensorLayout->addWidget(lidarCategory);

  std::vector<QCheckBox *> navChecks;
  std::vector<QPushButton *> navGears;
  std::vector<QPushButton *> navPluses;
  std::vector<QLabel *> navSensorCountLabels;
  std::vector<int> navSensorCounts;
  QLabel *navCountLabel = new VerticalTitleLabel("×0");
  QGroupBox *navCategory = makeCompactCategory("NAV", QStringList()
    << "GNSS" << "IMU",
    navCountLabel, navChecks, navGears, navPluses, navSensorCountLabels, navSensorCounts);
  sensorLayout->addWidget(navCategory);

  std::vector<QCheckBox *> gtChecks;
  std::vector<QPushButton *> gtGears;
  std::vector<QPushButton *> gtPluses;
  std::vector<QLabel *> gtSensorCountLabels;
  std::vector<int> gtSensorCounts;
  QLabel *gtCountLabel = new VerticalTitleLabel("×0");
  QGroupBox *groundTruthCategory = makeCompactCategory("GT", QStringList()
    << "Collision" << "Lane Invasion" << "Obstacle",
    gtCountLabel, gtChecks, gtGears, gtPluses, gtSensorCountLabels, gtSensorCounts);
  sensorLayout->addWidget(groundTruthCategory);

  QGroupBox *rosPseudoCategory = new QGroupBox("ros-bridge");
  rosPseudoCategory->setToolTip(
    "<b>ros-bridge Pseudo-Sensors</b><br>"
    "Virtual publishers the <a "
    "href=\"https://github.com/carla-simulator/ros-bridge\">ros-bridge</a> "
    "package synthesises from CARLA's world state — <i>not</i> CARLA "
    "blueprints, they don't spawn actors in the simulator. ros-bridge "
    "supports both ROS 1 and ROS 2; CARLA's native <code>--ros2</code> "
    "publishers don't generate these (they emit raw blueprint topics "
    "via DDS directly, no bridge layer).<br><br>"
    "Toggles persist regardless of whether ros-bridge is currently "
    "running — they're written into the bridge's <code>objects.json</code> "
    "alongside your real sensors at launch time.");
  QGridLayout *rosPseudoGrid = new QGridLayout();
  rosPseudoGrid->setHorizontalSpacing(10);
  rosPseudoGrid->setVerticalSpacing(2);
  rosPseudoGrid->setContentsMargins(6, 4, 6, 4);

  struct PseudoSensor {
    const char *id;
    const char *label;
    const char *topic;
    const char *msgType;
    const char *purpose;
  };
  static const PseudoSensor kPseudoSensors[] = {
    { "odometry",      "Odometry",       "/odometry",
      "nav_msgs/Odometry",
      "Ego vehicle pose + twist derived from CARLA's actor transform. "
      "The standard input for AV planning stacks." },
    { "speedometer",   "Speedometer",    "/speedometer",
      "std_msgs/Float32",
      "Scalar ground-speed of the ego in m/s." },
    { "map",           "Map",            "/map",
      "std_msgs/String",
      "OpenDRIVE map XML for the currently-loaded world. Latched "
      "once at start so late subscribers still receive it." },
    { "object_array",  "Object List",    "/objects",
      "derived_object_msgs/ObjectArray",
      "Bounding-box list of every actor in the world (vehicles, "
      "walkers, props). Useful for closed-loop perception evaluation." },
    { "marker",        "Markers",        "/markers",
      "visualization_msgs/Marker",
      "RViz-friendly marker stream — bounding boxes and pose arrows "
      "for visualisation only." },
    { "traffic_lights","Traffic Lights", "/traffic_lights/{status,info}",
      "carla_msgs/CarlaTrafficLight*",
      "Per-junction traffic light state and metadata. Required by "
      "Autoware's intersection-handling stack." },
    { "actor_list",    "Actor List",     "/actor_list",
      "carla_msgs/CarlaActorList",
      "Flat list of every actor + their type/role. The minimal feed "
      "for inspection tooling (rqt, custom dashboards)." },
  };

  QSettings rosPseudoSettings;
  std::vector<QCheckBox *> rosPseudoChecks;
  rosPseudoChecks.reserve(7);
  for (size_t i = 0; i < sizeof(kPseudoSensors) / sizeof(kPseudoSensors[0]); ++i) {
    const auto &p = kPseudoSensors[i];
    QCheckBox *cb = new QCheckBox(p.label);
    const QString key = QString("ros/pseudo/%1").arg(p.id);
    cb->setChecked(rosPseudoSettings.value(key, false).toBool());
    cb->setToolTip(QString(
      "<b>%1</b><br>"
      "<small>Topic: <code>%2</code><br>"
      "Type: <code>%3</code></small><br><br>%4")
      .arg(p.label).arg(p.topic).arg(p.msgType).arg(p.purpose));
    QObject::connect(cb, &QCheckBox::toggled, &window, [key](bool on) {
      QSettings().setValue(key, on);
    });
    rosPseudoGrid->addWidget(cb, static_cast<int>(i / 2), static_cast<int>(i % 2));
    rosPseudoChecks.push_back(cb);
  }
  rosPseudoCategory->setLayout(rosPseudoGrid);
  rosPseudoCategory->setVisible(false);

  QPushButton *previewSensorsBtn = new QPushButton("⊞ Preview");
  previewSensorsBtn->setVisible(false);
  sensorGroup->setLayout(sensorLayout);
  leftPaneLayout->addWidget(sensorGroup, 1);

  QHBoxLayout *sensorActionsRow = new QHBoxLayout();
  QPushButton *applySensorsBtn = new QPushButton("⚡ Apply");
  simRequiredButtons.push_back(applySensorsBtn);
  QPushButton *saveLayoutBtn   = new QPushButton("Save");
  QPushButton *loadLayoutBtn   = new QPushButton("Load");
  QPushButton *exportYamlBtn   = new QPushButton("YAML");
  for (QPushButton *b : {applySensorsBtn, saveLayoutBtn, loadLayoutBtn, exportYamlBtn}) {
    b->setFixedHeight(28);
  }
  applySensorsBtn->setToolTip("Spawn every enabled sensor × N at the configured "
                              "mount transform, attached to the hero vehicle. "
                              "Applies camera optics to the blueprint.");
  saveLayoutBtn->setToolTip("Write the full sensor layout (mounts + counts + "
                            "optics) to JSON.");
  loadLayoutBtn->setToolTip("Restore a sensor layout JSON saved earlier.");
  exportYamlBtn->setToolTip("Export the enabled sensors as a Scenario Runner "
                            "YAML block (sensors:).");
  sensorActionsRow->addWidget(applySensorsBtn);
  sensorActionsRow->addWidget(saveLayoutBtn);
  sensorActionsRow->addWidget(loadLayoutBtn);
  sensorActionsRow->addWidget(exportYamlBtn);
  leftPaneLayout->addLayout(sensorActionsRow);
  leftPane->setLayout(leftPaneLayout);

  QWidget *rightPane = new QWidget();
  QVBoxLayout *rightPaneLayout = new QVBoxLayout();
  rightPaneLayout->setContentsMargins(0, 0, 0, 0);
  QLabel *actuateHeading = new QLabel("Actuate");
  actuateHeading->setAlignment(Qt::AlignCenter);
  actuateHeading->setStyleSheet("font-weight: 700;");
  rightPaneLayout->addWidget(actuateHeading);
  QGroupBox *actuateGroup = new QGroupBox();
  QVBoxLayout *actuateLayout = new QVBoxLayout();

  struct ProtocolDef {
    const char *name;
    const char *tooltip;
    enum class Detector { None, Ros, Can, SomeIp, Tsn };
    Detector detector;
  };
  static const std::array<ProtocolDef, 12> kProtocols = {{
    { "LIN",
      "Local Interconnect Network — low-speed in-vehicle bus (~20 Kbps). "
      "Window controls, seat modules, climate systems. Always subordinate "
      "to CAN (master/slave). Rarely simulated in CARLA unless modelling "
      "body electronics. Requires plugged-in hardware to show status.",
      ProtocolDef::Detector::None },
    { "FlexRay",
      "Deterministic time-triggered bus (up to 10 Mbps). Drive-by-wire and "
      "safety-critical control. Now declining — being replaced by "
      "Automotive Ethernet. Requires plugged-in hardware to show status.",
      ProtocolDef::Detector::None },
    { "MOST",
      "Media Oriented Systems Transport — infotainment audio/video, often "
      "fiber-based. Mostly legacy; Ethernet has taken over. "
      "Requires plugged-in hardware to show status.",
      ProtocolDef::Detector::None },
    { "Automotive Eth",
      "Automotive Ethernet (100BASE-T1, 1000BASE-T1, …). The modern "
      "high-bandwidth backbone: 100 Mbps → multi-Gbps. Replacing "
      "CAN/FlexRay for sensor data (cameras, radar), central compute, "
      "ADAS/AV systems. Closest real-vehicle match for high-bandwidth "
      "CARLA data paths. Requires plugged-in hardware (100BASE-T1 / "
      "1000BASE-T1 PHY) to show status.",
      ProtocolDef::Detector::None },
    { "TSN",
      "Time-Sensitive Networking — extension of Ethernet with "
      "deterministic timing. Guarantees latency and synchronisation for "
      "real-time control over Ethernet, safety-critical AV pipelines. "
      "Think \"Ethernet that behaves like FlexRay, but faster\". "
      "Live-detected via tc qdisc (taprio / etf / mqprio) on this host.",
      ProtocolDef::Detector::Tsn },
    { "ROS",
      "Robot Operating System (ROS 2) — autonomy middleware. Uses DDS "
      "under the hood for real-time pub/sub with QoS. Live-detected "
      "on this machine via ros2 on PATH + topic flow.",
      ProtocolDef::Detector::Ros },
    { "SOME/IP",
      "Scalable service-Oriented MiddlewarE over IP — AUTOSAR Adaptive "
      "standard. RPC + pub/sub over IP; production-grade equivalent of "
      "ROS-style messaging in many OEM stacks. Live-detected via "
      "vsomeipd process / libvsomeip on this host.",
      ProtocolDef::Detector::SomeIp },
    { "gRPC",
      "Google RPC over HTTP/2 — high-performance RPC. Not automotive-"
      "specific but increasingly used in internal microservices and "
      "cloud-connected vehicle systems (software-defined vehicles).",
      ProtocolDef::Detector::None },
    { "DSRC",
      "Dedicated Short-Range Communications — early V2X standard "
      "(802.11p-based). Vehicle-to-vehicle / vehicle-to-infrastructure "
      "messaging. Requires plugged-in hardware (802.11p radio) to "
      "show status.",
      ProtocolDef::Detector::None },
    { "Cellular-V2X",
      "Cellular V2X (LTE / 5G) — replacing DSRC in many regions. "
      "Cooperative driving, traffic coordination, safety messaging. "
      "Relevant if you extend CARLA into multi-agent or smart-city "
      "simulations. Requires plugged-in hardware (C-V2X modem / "
      "cellular module) to show status.",
      ProtocolDef::Detector::None },
    { "XCP",
      "Universal Measurement and Calibration Protocol — ECU tuning and "
      "calibration. Works over CAN, Ethernet. Important in HIL/SIL "
      "setups tied to CARLA. (UDS — Unified Diagnostic Services — also "
      "lives over CAN/Ethernet for flashing and fault codes.)",
      ProtocolDef::Detector::None },
    { "CAN",
      "Controller Area Network — the canonical in-vehicle bus (~1 Mbps "
      "classic, up to 8 Mbps for CAN FD). Carries body/powertrain/ADAS "
      "signals and hosts higher-level protocols like UDS and XCP. "
      "Live-detected via the can0/vcan0 interface state on this machine.",
      ProtocolDef::Detector::Can },
  }};

  struct ProtocolWidget {
    QLabel *dot;
    QLabel *text;
    ProtocolDef::Detector detector;
    QString solidColor;
    bool    blinkActive = false;
  };
  std::array<ProtocolWidget, 12> protocolWidgets;

  QGridLayout *interfaceGrid = new QGridLayout();
  interfaceGrid->setContentsMargins(2, 2, 2, 2);
  interfaceGrid->setHorizontalSpacing(14);
  interfaceGrid->setVerticalSpacing(4);

  auto dotStyle = [](const QString &background) {
    return QString(
      "background-color: %1; border-radius: 6px; border: 1px solid #616161;")
      .arg(background);
  };

  for (int i = 0; i < 12; ++i) {
    const auto &p = kProtocols[i];
    QWidget *cell = new QWidget();
    cell->setToolTip(QString::fromUtf8(p.tooltip));
    QHBoxLayout *cellLayout = new QHBoxLayout(cell);
    cellLayout->setContentsMargins(0, 0, 0, 0);
    cellLayout->setSpacing(5);

    QLabel *dot = new QLabel();
    dot->setFixedSize(12, 12);
    dot->setStyleSheet(dotStyle("#7A869A"));
    cellLayout->addWidget(dot);

    QLabel *text = new QLabel(QString::fromUtf8(p.name));
    text->setStyleSheet("font-size: 11px;");
    cellLayout->addWidget(text);
    cellLayout->addStretch();

    interfaceGrid->addWidget(cell, i / 6, i % 6);
    protocolWidgets[i] = { dot, text, p.detector };
  }

  auto findDotForDetector = [&](ProtocolDef::Detector d) -> ProtocolWidget * {
    for (auto &w : protocolWidgets) {
      if (w.detector == d) return &w;
    }
    return nullptr;
  };

  auto setCanIndicatorColor = [&](const QString &hexColor, bool blink = false) {
    if (auto *w = findDotForDetector(ProtocolDef::Detector::Can)) {
      w->solidColor = hexColor;
      w->blinkActive = blink;
      w->dot->setStyleSheet(dotStyle(hexColor));
    }
  };

  std::function<void(bool)> g_setRosToolsMenuVisible;

  auto setRosIndicatorColor = [&](const QString &hexColor, bool blink = false) {
    if (auto *w = findDotForDetector(ProtocolDef::Detector::Ros)) {
      w->solidColor = hexColor;
      w->blinkActive = blink;
      w->dot->setStyleSheet(dotStyle(hexColor));
    }
  };

  auto setSomeIpIndicatorColor = [&](const QString &hexColor, bool blink = false) {
    if (auto *w = findDotForDetector(ProtocolDef::Detector::SomeIp)) {
      w->solidColor = hexColor;
      w->blinkActive = blink;
      w->dot->setStyleSheet(dotStyle(hexColor));
    }
  };

  auto setTsnIndicatorColor = [&](const QString &hexColor) {
    if (auto *w = findDotForDetector(ProtocolDef::Detector::Tsn))
      w->dot->setStyleSheet(dotStyle(hexColor));
  };

  QLabel *rosTextLabel = protocolWidgets[5].text;
  QLabel *canTextLabel = protocolWidgets[11].text;
  QWidget *rosStatusIndicator = protocolWidgets[5].dot;
  QWidget *canStatusIndicator = protocolWidgets[11].dot;

  auto refreshCanInterfaceState = [&]() {
    QProcess canProbe;
    canProbe.start("/bin/bash", QStringList() << "-lc"
      << "iface=$(ls /sys/class/net 2>/dev/null | grep -E '^(can|vcan)[0-9]+' | head -n1); "
         "if [ -z \"$iface\" ]; then echo missing; else "
         "state=$(cat /sys/class/net/$iface/operstate 2>/dev/null || echo down); "
         "rx=$(cat /sys/class/net/$iface/statistics/rx_packets 2>/dev/null || echo 0); "
         "tx=$(cat /sys/class/net/$iface/statistics/tx_packets 2>/dev/null || echo 0); "
         "echo \"$iface $state $rx $tx\"; fi");
    canProbe.waitForFinished(800);

    const QString result = QString::fromLocal8Bit(canProbe.readAllStandardOutput()).trimmed();
    if (result == "missing" || result.isEmpty()) {
      canTextLabel->setEnabled(false);
      canTextLabel->setToolTip("CAN not configured: expected can0/vcan0.");
      setCanIndicatorColor("#FDD835");
      canStatusIndicator->setToolTip("CAN not configured");
      return;
    }

    const QStringList fields = result.split(' ', Qt::SkipEmptyParts);
    if (fields.size() < 4) {
      canTextLabel->setEnabled(false);
      canTextLabel->setToolTip("Unable to parse CAN interface status.");
      setCanIndicatorColor("#FDD835");
      canStatusIndicator->setToolTip("CAN status unknown");
      return;
    }

    const QString iface = fields[0];
    const QString state = fields[1];
    bool okRx = false;
    bool okTx = false;
    const qint64 rxPackets = fields[2].toLongLong(&okRx);
    const qint64 txPackets = fields[3].toLongLong(&okTx);
    const bool hasSignal = (okRx && okTx && (rxPackets > 0 || txPackets > 0));

    canTextLabel->setEnabled(true);
    if (state == "up" && hasSignal) {
      setCanIndicatorColor("#2E7D32", true);
      canStatusIndicator->setToolTip(QString("%1 active (rx=%2, tx=%3)").arg(iface).arg(rxPackets).arg(txPackets));
      canTextLabel->setToolTip(QString("%1 active and receiving CAN signal").arg(iface));
    } else {
      setCanIndicatorColor("#C62828");
      canStatusIndicator->setToolTip(QString("%1 configured but inactive (state=%2, rx=%3, tx=%4)")
                                     .arg(iface)
                                     .arg(state)
                                     .arg(okRx ? QString::number(rxPackets) : QString("?"))
                                     .arg(okTx ? QString::number(txPackets) : QString("?")));
      canTextLabel->setToolTip(QString("%1 configured but not active").arg(iface));
    }
  };

  auto refreshRosInterfaceState = [&]() {
    QProcess rosProbe;
    rosProbe.start("/bin/bash", QStringList() << "-lc"
      << "if [ -n \"$CARLA_STUDIO_ROS_PATH\" ] && [ -f \"$CARLA_STUDIO_ROS_PATH/setup.bash\" ]; then "
           "source \"$CARLA_STUDIO_ROS_PATH/setup.bash\" 2>/dev/null; fi; "
         "if command -v ros2 >/dev/null 2>&1 || [ -n \"$ROS_DISTRO\" ] || ls /opt/ros/* >/dev/null 2>&1; then "
           "if pgrep -f 'ros2|roscore|dds|fastdds' >/dev/null 2>&1; then "
             "if command -v ros2 >/dev/null 2>&1 && timeout 2s ros2 topic list 2>/dev/null "
               "| grep -Ev '^/(parameter_events|rosout)$' | grep -q .; then echo active; "
             "else echo running; fi; "
           "else echo installed; fi; "
         "else echo missing; fi");
    rosProbe.waitForFinished(800);
    const QString rosState = QString::fromLocal8Bit(rosProbe.readAllStandardOutput()).trimmed();
    if (rosState == "active") {
      setRosIndicatorColor("#2E7D32", true);
      rosTextLabel->setEnabled(true);
      rosTextLabel->setToolTip("ROS2 runtime active with topic flow");
      rosStatusIndicator->setToolTip("ROS2 active");
    } else if (rosState == "running") {
      setRosIndicatorColor("#FDD835");
      rosTextLabel->setEnabled(true);
      rosTextLabel->setToolTip("ROS2 runtime detected, no active topic flow");
      rosStatusIndicator->setToolTip("ROS2 running");
    } else if (rosState == "installed") {
      setRosIndicatorColor("#FDD835");
      rosTextLabel->setEnabled(true);
      rosTextLabel->setToolTip("ROS2 installed but runtime not active");
      rosStatusIndicator->setToolTip("ROS2 installed");
    } else {
      setRosIndicatorColor("#C62828");
      rosTextLabel->setEnabled(false);
      rosTextLabel->setToolTip("ROS2 not installed");
      rosStatusIndicator->setToolTip("ROS2 not installed");
    }
    if (g_setRosToolsMenuVisible) {
      g_setRosToolsMenuVisible(rosState != "missing");
    }
  };

  auto refreshSomeIpState = [&]() {
    QProcess p;
    p.start("/bin/bash", QStringList() << "-lc"
      << "if pgrep -f '(vsomeipd|someipd)' >/dev/null 2>&1; then "
           "if ss -lun 2>/dev/null | grep -Eq ':30490(\\\\b|[^0-9])'; then echo traffic; "
           "else echo daemon; fi; "
         "elif ldconfig -p 2>/dev/null | grep -qi 'libvsomeip'; then echo installed; "
         "else echo missing; fi");
    p.waitForFinished(800);
    const QString state = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    ProtocolWidget *w = findDotForDetector(ProtocolDef::Detector::SomeIp);
    if (!w) return;
    if (state == "traffic") {
      setSomeIpIndicatorColor("#2E7D32", true);
      w->dot->setToolTip("vsomeipd running with SD socket on UDP :30490");
    } else if (state == "daemon") {
      setSomeIpIndicatorColor("#FDD835");
      w->dot->setToolTip("vsomeipd running, no SOME/IP traffic observed "
                         "(no UDP :30490 listener)");
    } else if (state == "installed") {
      setSomeIpIndicatorColor("#FDD835");
      w->dot->setToolTip("libvsomeip installed, vsomeipd not running");
    } else {
      setSomeIpIndicatorColor("#7A869A");
      w->dot->setToolTip("SOME/IP runtime not detected on this host");
    }
  };

  auto refreshTsnState = [&]() {
    QProcess p;
    p.start("/bin/bash", QStringList() << "-lc"
      << "if tc qdisc show 2>/dev/null | grep -Eiq 'taprio|^qdisc etf|mqprio'; then echo active; "
         "elif tc qdisc help 2>&1 | grep -qi taprio; then echo capable; "
         "else echo missing; fi");
    p.waitForFinished(800);
    const QString state = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    ProtocolWidget *w = findDotForDetector(ProtocolDef::Detector::Tsn);
    if (!w) return;
    if (state == "active") {
      setTsnIndicatorColor("#2E7D32");
      w->dot->setToolTip("TSN qdisc (taprio / etf / mqprio) configured");
    } else if (state == "capable") {
      setTsnIndicatorColor("#FDD835");
      w->dot->setToolTip("Kernel supports TSN qdiscs, none currently installed");
    } else {
      setTsnIndicatorColor("#7A869A");
      w->dot->setToolTip("TSN qdisc support not detected on this host");
    }
  };

  QTimer *canStatusTimer = new QTimer(&window);
  QObject::connect(canStatusTimer, &QTimer::timeout, &window, [&]() { refreshCanInterfaceState(); });
  canStatusTimer->start(2000);
  refreshCanInterfaceState();

  QTimer *rosStatusTimer = new QTimer(&window);
  QObject::connect(rosStatusTimer, &QTimer::timeout, &window, [&]() { refreshRosInterfaceState(); });
  rosStatusTimer->start(2000);
  refreshRosInterfaceState();

  QTimer *someIpStatusTimer = new QTimer(&window);
  QObject::connect(someIpStatusTimer, &QTimer::timeout, &window, [&]() { refreshSomeIpState(); });
  someIpStatusTimer->start(10000);
  refreshSomeIpState();

  QTimer *tsnStatusTimer = new QTimer(&window);
  QObject::connect(tsnStatusTimer, &QTimer::timeout, &window, [&]() { refreshTsnState(); });
  tsnStatusTimer->start(10000);
  refreshTsnState();

  QTimer *protocolBlinkTimer = new QTimer(&window);
  QObject::connect(protocolBlinkTimer, &QTimer::timeout, &window,
    [&, phase = 0]() mutable {
      phase ^= 1;
      for (auto &w : protocolWidgets) {
        if (!w.blinkActive || w.solidColor.isEmpty()) continue;
        QColor c(w.solidColor);
        if (phase) c = c.lighter(140);
        w.dot->setStyleSheet(dotStyle(c.name()));
      }
    });
  protocolBlinkTimer->start(550);

  kbGroup->setParent(actuateGroup);
  kbGroup->hide();
  jsGroup->setParent(actuateGroup);
  jsGroup->hide();

  struct ActuatePlayerRow {
    QString       slot;
    QString       defaultName;
    QLineEdit    *nameEdit = nullptr;
    QComboBox    *control = nullptr;
    QPushButton  *popOut  = nullptr;
  };
  auto actuatePlayers = std::make_shared<std::vector<ActuatePlayerRow>>(17);
  (*actuatePlayers)[0].slot = "EGO";
  (*actuatePlayers)[0].defaultName = carla::studio::core::defaultPlayerName(0);
  for (int i = 0; i < 10; ++i) {
    const QString slot = QString("POV%1").arg(i + 1, 2, 10, QChar('0'));
    (*actuatePlayers)[i + 1].slot = slot;
    (*actuatePlayers)[i + 1].defaultName = carla::studio::core::defaultPlayerName(i + 1);
  }
  for (int i = 0; i < 6; ++i) {
    const QString slot = QString("V2X%1").arg(i + 1, 2, 10, QChar('0'));
    (*actuatePlayers)[11 + i].slot = slot;
    (*actuatePlayers)[11 + i].defaultName = carla::studio::core::defaultPlayerName(11 + i);
  }

  auto detectJoystickCount = []() -> int {
    QDir d("/dev/input");
    return d.entryList(QStringList() << "js*", QDir::Files | QDir::System).size();
  };
  auto detectedJoystickCount = std::make_shared<int>(detectJoystickCount());

  auto repopulateControlCombos =
      [actuatePlayers, detectedJoystickCount]() {
    const int joyCount = *detectedJoystickCount;
    for (size_t i = 0; i < actuatePlayers->size(); ++i) {
      QComboBox *combo = (*actuatePlayers)[i].control;
      if (!combo) continue;
      const QString prev = combo->currentText();
      QSignalBlocker blocker(combo);
      combo->clear();
      combo->addItem("(unassigned)");
      bool kbTaken = false;
      for (size_t j = 0; j < actuatePlayers->size(); ++j) {
        if (j == i) continue;
        QComboBox *other = (*actuatePlayers)[j].control;
        if (other && other->currentText() == "Keyboard") { kbTaken = true; break; }
      }
      if (!kbTaken || prev == "Keyboard") combo->addItem("Keyboard");
      for (int k = 1; k <= joyCount; ++k) {
        const QString jname = QString("Joystick %1").arg(k);
        bool taken = false;
        for (size_t j = 0; j < actuatePlayers->size(); ++j) {
          if (j == i) continue;
          QComboBox *other = (*actuatePlayers)[j].control;
          if (other && other->currentText() == jname) { taken = true; break; }
        }
        if (!taken || prev == jname) combo->addItem(jname);
      }
      const int restoreIdx = combo->findText(prev);
      combo->setCurrentIndex(restoreIdx >= 0 ? restoreIdx : 0);
      if (combo->count() <= 1) {
        combo->setToolTip(
          "No joystick connected and no integration (e.g. SUMO / TeraSim) "
          "enabled. Plug in a controller and click ↻ to re-scan, or enable "
          "an integration via Cfg → Third-Party Tools.");
      } else {
        combo->setToolTip(QString());
      }
    }
  };

  auto displayName = [](const ActuatePlayerRow &p) -> QString {
    if (p.nameEdit && !p.nameEdit->text().isEmpty()) return p.nameEdit->text();
    return p.defaultName;
  };
  auto openControlPopout = [&](int rowIdx) {
    const ActuatePlayerRow &p = (*actuatePlayers)[rowIdx];
    const QString assigned = p.control ? p.control->currentText() : QString();
    const QString shown = displayName(p);
    QDialog *dlg = new QDialog(&window);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setWindowTitle(QString("Actuate · %1").arg(shown));
    dlg->resize(440, 260);
    QVBoxLayout *l = new QVBoxLayout(dlg);
    l->setContentsMargins(12, 12, 12, 12);
    QLabel *title = new QLabel(QString("<h3>%1</h3>").arg(shown));
    title->setTextFormat(Qt::RichText);
    l->addWidget(title);
    QLabel *header = new QLabel(QString("<b>Control:</b> %1")
      .arg(assigned.isEmpty() ? "(unassigned)" : assigned));
    header->setTextFormat(Qt::RichText);
    l->addWidget(header);

    QLabel *liveLabel = new QLabel("Live input:");
    l->addWidget(liveLabel);
    QLabel *stateLabel = new QLabel("waiting…");
    stateLabel->setTextFormat(Qt::PlainText);
    stateLabel->setStyleSheet(
      "QLabel { font-family: monospace; background: #111; color: #6f6; "
      "padding: 8px; border-radius: 4px; }");
    stateLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    stateLabel->setMinimumHeight(120);
    l->addWidget(stateLabel, 1);

    QTimer *liveTimer = new QTimer(dlg);
    QObject::connect(liveTimer, &QTimer::timeout, dlg, [&, assigned, stateLabel]() {
      QStringList lines;
      if (assigned == "Keyboard") {
        auto mark = [](bool b) -> QString { return b ? "[X]" : "[ ]"; };
        lines << QString("throttle  %1   W / Up").arg(mark(driveKeys.throttle));
        lines << QString("brake     %1   S / Down").arg(mark(driveKeys.brake));
        lines << QString("steer L   %1   A / Left").arg(mark(driveKeys.steerLeft));
        lines << QString("steer R   %1   D / Right").arg(mark(driveKeys.steerRight));
        lines << QString("handbrake %1   Space").arg(mark(driveKeys.handbrake));
        if (!inAppDriverControlActive) {
          lines << QString();
          lines << "(driver control inactive — START the sim to activate)";
        }
      } else if (assigned.startsWith("Joystick")) {
#ifdef CARLA_STUDIO_HAS_LINUX_JOYSTICK
        auto bar = [](double v) -> QString {
          int n = qBound(0, static_cast<int>((v + 1.0) * 10.0), 20);
          QString s; s.fill(' ', 20); for (int i = 0; i < n; ++i) s[i] = '#'; return s;
        };
        auto trig = [](double v) -> QString {
          int n = qBound(0, static_cast<int>(v * 20.0), 20);
          QString s; s.fill(' ', 20); for (int i = 0; i < n; ++i) s[i] = '#'; return s;
        };
        auto mark = [](bool b) -> QString { return b ? "[X]" : "[ ]"; };
        lines << QString("LX  [%1]  %2").arg(bar(lx)).arg(lx, 6, 'f', 2);
        lines << QString("LY  [%1]  %2").arg(bar(ly)).arg(ly, 6, 'f', 2);
        lines << QString("RX  [%1]  %2").arg(bar(rx)).arg(rx, 6, 'f', 2);
        lines << QString("RY  [%1]  %2").arg(bar(ry)).arg(ry, 6, 'f', 2);
        lines << QString("LT  [%1]  %2").arg(trig(lt)).arg(lt, 6, 'f', 2);
        lines << QString("RT  [%1]  %2").arg(trig(rt)).arg(rt, 6, 'f', 2);
        lines << QString("A%1 B%2 X%3 Y%4 L1%5 R1%6")
                  .arg(mark(aPressed)).arg(mark(bPressed)).arg(mark(xPressed))
                  .arg(mark(yPressed)).arg(mark(l1Pressed)).arg(mark(r1Pressed));
        if (jsFd < 0) {
          lines << QString();
          lines << "(no joystick device open — plug in a controller)";
        }
#else
        lines << "(joystick support not compiled in)";
#endif
      } else {
        lines << "(no device assigned)";
        lines << QString();
        lines << "Pick Keyboard or Joystick N from the dropdown";
        lines << "to wire input into this player slot.";
      }
      stateLabel->setText(lines.join("\n"));
    });
    liveTimer->start(50);

    dlg->show();
    dlg->raise();
  };

  QSettings actuateSettings;
  auto buildPlayerRow = [&](size_t i, QGridLayout *grid, int row) {
    ActuatePlayerRow &pl = (*actuatePlayers)[i];

    QLineEdit *nameEdit = new QLineEdit();
    nameEdit->setMaxLength(16);
    nameEdit->setFixedWidth(64);
    nameEdit->setFrame(false);
    nameEdit->setStyleSheet(
      "QLineEdit { background: transparent; padding: 0 2px; color: #000; }"
      "QLineEdit:disabled { color: #c0c0c0; }");
    {
      const QString saved = actuateSettings.value(
        QString("actuate/displayname_%1").arg(pl.slot)).toString();
      nameEdit->setText(saved.isEmpty() ? pl.defaultName : saved);
    }
    pl.nameEdit = nameEdit;
    grid->addWidget(nameEdit, row, 0);

    QComboBox *combo = new QComboBox();
    combo->addItem("(unassigned)");
    combo->setStyleSheet(
      "QComboBox:disabled { color: #c0c0c0; background: #f4f4f4; }");
    pl.control = combo;
    grid->addWidget(combo, row, 1);

    QPushButton *popBtn = new QPushButton("⊞");
    popBtn->setFixedSize(16, 16);
    popBtn->setStyleSheet(
      "QPushButton { padding: 0; margin: 0; border: 1px solid #b0b0b0; "
      "  border-radius: 2px; font-size: 11px; }"
      "QPushButton:disabled { color: #c8c8c8; border-color: #dcdcdc; }");
    popBtn->setToolTip("Pop out the live control viewer for this player.");
    pl.popOut = popBtn;
    grid->addWidget(popBtn, row, 2);

    QObject::connect(nameEdit, &QLineEdit::textEdited, &window,
      [actuatePlayers, i](const QString &text) {
        QSettings().setValue(
          QString("actuate/displayname_%1").arg((*actuatePlayers)[i].slot),
          text);
      });
    QObject::connect(combo,
      QOverload<int>::of(&QComboBox::currentIndexChanged), &window,
      [actuatePlayers, repopulateControlCombos, i]() {
        repopulateControlCombos();
        QComboBox *c = (*actuatePlayers)[i].control;
        if (!c) return;
        QSettings().setValue(
          QString("actuate/player_%1").arg((*actuatePlayers)[i].slot),
          c->currentText());
      });
    QObject::connect(popBtn, &QPushButton::clicked, &window,
      [openControlPopout, i]() { openControlPopout(static_cast<int>(i)); });
  };

  QGroupBox *egoGroup = new QGroupBox("EGO");
  QGridLayout *egoGrid = new QGridLayout();
  egoGrid->setHorizontalSpacing(6);
  egoGrid->setVerticalSpacing(2);
  egoGrid->setContentsMargins(8, 4, 8, 4);
  buildPlayerRow(0, egoGrid, 0);
  egoGroup->setLayout(egoGrid);
  actuateLayout->addWidget(egoGroup);

  QGroupBox *povsGroup = new QGroupBox("POVs");
  QGridLayout *povsGrid = new QGridLayout();
  povsGrid->setHorizontalSpacing(6);
  povsGrid->setVerticalSpacing(2);
  povsGrid->setContentsMargins(8, 4, 8, 4);
  for (size_t i = 1; i < 11; ++i) {
    buildPlayerRow(i, povsGrid, static_cast<int>(i - 1));
  }
  povsGroup->setLayout(povsGrid);
  actuateLayout->addWidget(povsGroup);

  QGroupBox *v2xGroup = new QGroupBox("V2X");
  QGridLayout *v2xGrid = new QGridLayout();
  v2xGrid->setHorizontalSpacing(6);
  v2xGrid->setVerticalSpacing(2);
  v2xGrid->setContentsMargins(8, 4, 8, 4);
  for (size_t i = 11; i < actuatePlayers->size(); ++i) {
    buildPlayerRow(i, v2xGrid, static_cast<int>(i - 11));
  }
  v2xGroup->setLayout(v2xGrid);
  actuateLayout->addWidget(v2xGroup);

  for (size_t i = 0; i < actuatePlayers->size(); ++i) {
    const QString key = QString("actuate/player_%1").arg((*actuatePlayers)[i].slot);
    QString saved = actuateSettings.value(key).toString();
    if (i == 0 && saved.isEmpty()) saved = "Keyboard";
    if (!saved.isEmpty() && (*actuatePlayers)[i].control) {
      (*actuatePlayers)[i].control->setCurrentText(saved);
    }
  }
  repopulateControlCombos();

  auto isAssigned = [](QComboBox *c) {
    if (!c) return false;
    const QString s = c->currentText();
    return !s.isEmpty() && s != "(unassigned)";
  };
  auto updatePovGating = [actuatePlayers, &terasimEnable, &autowareEnable,
                          isAssigned]() {
    if (actuatePlayers->size() < 11) return;
    const bool integrationActive =
      (terasimEnable && terasimEnable->isChecked()) ||
      (autowareEnable && autowareEnable->isChecked());
    bool prevAssigned = isAssigned((*actuatePlayers)[0].control) ||
                         integrationActive;
    for (size_t i = 1; i < 11; ++i) {
      QComboBox   *cb  = (*actuatePlayers)[i].control;
      QPushButton *pop = (*actuatePlayers)[i].popOut;
      QLineEdit   *nm  = (*actuatePlayers)[i].nameEdit;
      if (cb)  cb->setEnabled(prevAssigned);
      if (pop) pop->setEnabled(prevAssigned);
      if (nm)  nm->setEnabled(prevAssigned);
      prevAssigned = prevAssigned && isAssigned(cb);
    }
    bool prevV2x = isAssigned((*actuatePlayers)[0].control) || integrationActive;
    for (size_t i = 11; i < actuatePlayers->size(); ++i) {
      QComboBox   *cb  = (*actuatePlayers)[i].control;
      QPushButton *pop = (*actuatePlayers)[i].popOut;
      QLineEdit   *nm  = (*actuatePlayers)[i].nameEdit;
      if (cb)  cb->setEnabled(prevV2x);
      if (pop) pop->setEnabled(prevV2x);
      if (nm)  nm->setEnabled(prevV2x);
      prevV2x = prevV2x && isAssigned(cb);
    }
  };
  for (size_t i = 0; i < actuatePlayers->size(); ++i) {
    QComboBox *c = (*actuatePlayers)[i].control;
    if (!c) continue;
    QObject::connect(c, &QComboBox::currentTextChanged, &window,
      [updatePovGating](const QString &) { updatePovGating(); });
  }
  QObject::connect(terasimEnable, &QCheckBox::toggled, &window,
    [updatePovGating](bool) { updatePovGating(); });
  QObject::connect(autowareEnable, &QCheckBox::toggled, &window,
    [updatePovGating](bool) { updatePovGating(); });
  updatePovGating();

  QGroupBox *saeGroup = new QGroupBox("Self Driving Demo");
  saeGroup->setToolTip(
    "Self Driving Demo — drives the in-app vehicle through the C++ "
    "BehaviorAgent. SAE level selects the automation tier (L0 manual → "
    "L5 full).");
  QVBoxLayout *saeRow = new QVBoxLayout();
  saeRow->setSpacing(2);
  saeRow->setContentsMargins(8, 4, 8, 4);
  auto *saeBtnGroup = new QButtonGroup(&window);
  saeBtnGroup->setExclusive(true);

  auto makeSaeIconPair = [](int level) -> QPair<QPixmap, QPixmap> {
    auto draw = [level](bool isCar) -> QPixmap {
      const int sz = 20;
      QPixmap pm(sz, sz);
      pm.fill(Qt::transparent);
      QPainter p(&pm);
      p.setRenderHint(QPainter::Antialiasing);
      const double drvA = 1.0 - 0.18 * level;
      const double vehA = 0.10 + 0.18 * level;
      p.setOpacity(qMax(0.10, isCar ? vehA : drvA));
      QColor c(40, 40, 46);
      if (isCar && level >= 3) c = QColor(225, 110, 30);
      const QRect box(0, 0, sz, sz);
      if (!isCar) {
        p.setPen(QPen(c, 1.6));
        p.setBrush(Qt::NoBrush);
        const QRectF outer = QRectF(box).adjusted(2, 2, -2, -2);
        p.drawEllipse(outer);
        const QPointF cx = outer.center();
        const qreal r = outer.width() / 2.0;
        p.drawLine(cx, QPointF(cx.x(),               cx.y() - r * 0.60));
        p.drawLine(cx, QPointF(cx.x() - r * 0.55,    cx.y() + r * 0.40));
        p.drawLine(cx, QPointF(cx.x() + r * 0.55,    cx.y() + r * 0.40));
        p.setBrush(c);
        p.drawEllipse(cx, r * 0.18, r * 0.18);
      } else {
        p.setPen(QPen(c.darker(150), 1.2));
        p.setBrush(c);
        QRectF body = QRectF(box).adjusted(1, box.height() * 0.45,
                                              -1, -box.height() * 0.10);
        QPainterPath bodyPath;
        bodyPath.addRoundedRect(body, body.height() * 0.30,
                                          body.height() * 0.30);
        p.drawPath(bodyPath);
        QPainterPath cabin;
        cabin.moveTo(body.left()  + body.width() * 0.10, body.top());
        cabin.lineTo(body.left()  + body.width() * 0.25,
                      body.top() - body.height() * 0.50);
        cabin.lineTo(body.right() - body.width() * 0.25,
                      body.top() - body.height() * 0.50);
        cabin.lineTo(body.right() - body.width() * 0.10, body.top());
        cabin.closeSubpath();
        p.drawPath(cabin);
        p.setBrush(c.darker(180));
        p.setPen(Qt::NoPen);
        const qreal wr = body.height() * 0.30;
        p.drawEllipse(QPointF(body.left()  + body.width() * 0.22,
                                body.bottom() - 1), wr, wr);
        p.drawEllipse(QPointF(body.right() - body.width() * 0.22,
                                body.bottom() - 1), wr, wr);
      }
      return pm;
    };
    return { draw(false), draw(true) };
  };

  static const std::array<std::pair<const char *, const char *>, 6> kSaeLevels = {{
    {"No Automation",
      "<b>SAE L0 — No Automation</b><br>"
      "Human driver performs all dynamic driving tasks. The keyboard / "
      "joystick controls in the player table above operate the vehicle "
      "directly at this level."},
    {"Driver Assistance",
      "<b>SAE L1 — Driver Assistance</b><br>"
      "One automated feature — adaptive cruise control or lane keeping, "
      "but not both simultaneously. Driver remains responsible."},
    {"Partial Automation",
      "<b>SAE L2 — Partial Automation</b><br>"
      "Two or more features simultaneously (e.g. ACC + lane centering). "
      "Driver still supervises and is ready to take over. Examples: "
      "Tesla Autopilot, GM SuperCruise."},
    {"Conditional Automation",
      "<b>SAE L3 — Conditional Automation</b><br>"
      "System handles all driving within a defined ODD; human must be "
      "available to take over when requested. Mercedes Drive Pilot, "
      "Honda Sensing Elite."},
    {"High Automation",
      "<b>SAE L4 — High Automation</b><br>"
      "No human intervention required within the defined ODD. Examples: "
      "Waymo robotaxis in Phoenix / SF, Cruise pre-pause."},
    {"Full Automation",
      "<b>SAE L5 — Full Automation</b><br>"
      "No human intervention required anywhere a human could drive. "
      "Keyboard / joystick controls become inapplicable at this level — "
      "the AI driver owns every manoeuvre."},
  }};
  
  
  auto makeLaneKeepPixmap = []() -> QPixmap {
    const int sz = 18;
    QPixmap pm(sz, sz);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor(40, 40, 46), 1.6);
    p.setPen(pen);
    p.drawLine(QPointF(sz * 0.30, 1), QPointF(sz * 0.30, sz - 1));
    p.drawLine(QPointF(sz * 0.70, 1), QPointF(sz * 0.70, sz - 1));
    pen.setStyle(Qt::DashLine);
    pen.setWidthF(1.2);
    p.setPen(pen);
    p.drawLine(QPointF(sz * 0.50, 2), QPointF(sz * 0.50, sz - 2));
    return pm;
  };
  auto makeAccPixmap = []() -> QPixmap {
    const int sz = 18;
    QPixmap pm(sz, sz);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(40, 40, 46), 1.4));
    p.setBrush(Qt::NoBrush);
    const QRectF outer = QRectF(2, 2, sz - 4, sz - 4);
    p.drawArc(outer, 30 * 16, 120 * 16);
    p.setPen(QPen(QColor(225, 110, 30), 1.6));
    const QPointF cx = outer.center();
    const qreal r = outer.width() / 2.0;
    p.drawLine(cx, QPointF(cx.x() + r * 0.65, cx.y() - r * 0.45));
    p.setBrush(QColor(40, 40, 46));
    p.setPen(Qt::NoPen);
    p.drawEllipse(cx, 1.4, 1.4);
    return pm;
  };

  
  
  QSettings actuateSelfDriveSettings;
  bool l1FeatureIsLaneKeep =
      actuateSelfDriveSettings.value("actuate/sae_l1_feature", QString("acc"))
                              .toString()
                              .toLower() == "lanekeep";
  QPushButton *l1LaneKeepBtn = nullptr;
  QPushButton *l1AccBtn      = nullptr;

  std::vector<QPushButton *> saeButtons;
  saeButtons.reserve(kSaeLevels.size());
  for (size_t i = 0; i < kSaeLevels.size(); ++i) {
    QWidget *rowHost = new QWidget();
    QHBoxLayout *rowLay = new QHBoxLayout(rowHost);
    rowLay->setContentsMargins(0, 0, 0, 0);
    rowLay->setSpacing(6);

    auto pair = makeSaeIconPair(static_cast<int>(i));
    QLabel *leftIcon  = new QLabel();
    QLabel *rightIcon = new QLabel();
    leftIcon->setPixmap(pair.first);
    rightIcon->setPixmap(pair.second);
    leftIcon->setFixedSize(20, 20);
    rightIcon->setFixedSize(20, 20);
    leftIcon->setToolTip(QString::fromLatin1(kSaeLevels[i].second));
    rightIcon->setToolTip(QString::fromLatin1(kSaeLevels[i].second));

    QPushButton *btn = new QPushButton(QString("SAE Level %1").arg(i));
    btn->setCheckable(true);
    btn->setMinimumHeight(28);
    btn->setToolTip(QString::fromLatin1(kSaeLevels[i].second));
#ifndef CARLA_STUDIO_WITH_LIBCARLA
    
    if (i > 0) btn->setEnabled(false);
#endif
    saeBtnGroup->addButton(btn, static_cast<int>(i));
    saeButtons.push_back(btn);

    rowLay->addWidget(leftIcon, 0, Qt::AlignVCenter);

    
    
    
    
    if (i == 1) {
      l1LaneKeepBtn = new QPushButton();
      l1AccBtn      = new QPushButton();
      l1LaneKeepBtn->setIcon(QIcon(makeLaneKeepPixmap()));
      l1AccBtn->setIcon(QIcon(makeAccPixmap()));
      l1LaneKeepBtn->setIconSize(QSize(14, 14));
      l1AccBtn->setIconSize(QSize(14, 14));
      l1LaneKeepBtn->setFixedSize(20, 22);
      l1AccBtn->setFixedSize(20, 22);
      l1LaneKeepBtn->setCheckable(true);
      l1AccBtn->setCheckable(true);
      l1LaneKeepBtn->setToolTip(
          "<b>L1 — Lane Keeping only</b><br>"
          "Agent steers; you control throttle and brake. Click to enable.");
      l1AccBtn->setToolTip(
          "<b>L1 — Adaptive Cruise Control only</b><br>"
          "Agent paces speed; you steer. Click to enable.");
      auto *l1Group = new QButtonGroup(&window);
      l1Group->setExclusive(true);
      l1Group->addButton(l1LaneKeepBtn);
      l1Group->addButton(l1AccBtn);
      if (l1FeatureIsLaneKeep) l1LaneKeepBtn->setChecked(true);
      else                     l1AccBtn->setChecked(true);
      rowLay->addWidget(l1LaneKeepBtn, 0, Qt::AlignVCenter);
      rowLay->addWidget(btn,           1);
      rowLay->addWidget(l1AccBtn,      0, Qt::AlignVCenter);
    } else {
      rowLay->addWidget(btn, 1);
    }

    rowLay->addWidget(rightIcon, 0, Qt::AlignVCenter);
    saeRow->addWidget(rowHost);
  }
  {
    const int saved = actuateSettings.value("actuate/sae_level", 0).toInt();
    const int clamped = (saved < 0 || saved >= static_cast<int>(saeButtons.size())) ? 0 : saved;
    saeButtons[static_cast<size_t>(clamped)]->setChecked(true);
  }
  
  
  
  auto applyL5InputLockout = [actuatePlayers](int level) {
    if (actuatePlayers->empty()) return;
    auto *combo = (*actuatePlayers)[0].control;   
    if (!combo) return;
    const bool lockout = (level == 5);
    combo->setEnabled(!lockout);
    combo->setToolTip(lockout
        ? QString("Locked at SAE Level 5 — Full Automation. The C++ "
                  "BehaviorAgent has exclusive control of the vehicle.")
        : QString());
  };

  QObject::connect(saeBtnGroup, &QButtonGroup::idClicked, &window,
    [&, applyL5InputLockout](int level) {
    try {
    QSettings().setValue("actuate/sae_level", level);
    applyL5InputLockout(level);
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    SelfDriveMode mode = SelfDriveMode::Off;
    switch (level) {
      case 0: mode = SelfDriveMode::Off;              break;
      case 1:
        mode = l1FeatureIsLaneKeep ? SelfDriveMode::LaneKeep
                                    : SelfDriveMode::ACC;
        break;
      case 2: mode = SelfDriveMode::AssistNormal;     break;
      case 3: mode = SelfDriveMode::AssistAggressive; break;
      case 4: mode = SelfDriveMode::AutonomousLoop;   break;
      case 5: mode = SelfDriveMode::AutonomousLoop;   break;
      default: mode = SelfDriveMode::Off;             break;
    }
    if (mode != SelfDriveMode::Off && !inAppDriveVehicle) {
      setSimulationStatus(QString("Self-Drive L%1 needs a running in-app vehicle").arg(level));
      return;
    }
    if (engageSelfDrive(mode)) {
      static const char *kLevelLabel[6] = {
          "manual",
          "L1 ACC",       
          "L2 Partial Automation (Normal)",
          "L3 Conditional (Aggressive)",
          "L4 High Automation (continuous)",
          "L5 Full Automation (continuous)",
      };
      const int li = std::clamp(level, 0, 5);
      QString label = QString::fromLatin1(kLevelLabel[li]);
      if (level == 1 && l1FeatureIsLaneKeep) label = "L1 Lane Keeping";
      setSimulationStatus(QString("Running (Self-Drive %1)").arg(label));
    } else {
      setSimulationStatus(QString("Self-Drive L%1 engagement failed").arg(level));
    }
#else
    Q_UNUSED(level);
#endif
    } catch (const std::exception &e) {
      std::cerr << "[SAE click L" << level << "] " << e.what() << '\n';
      setSimulationStatus(QString("Self-Drive L%1 raised: %2")
                            .arg(level).arg(QString::fromLocal8Bit(e.what())));
    } catch (...) {
      std::cerr << "[SAE click L" << level << "] unknown exception\n";
      setSimulationStatus(QString("Self-Drive L%1 raised an unknown exception").arg(level));
    }
  });

  
  
  applyL5InputLockout(saeBtnGroup->checkedId());

  
  
  if (l1LaneKeepBtn && l1AccBtn) {
    auto onL1FeaturePicked = [&, l1LaneKeepBtn, l1AccBtn]() {
      l1FeatureIsLaneKeep = l1LaneKeepBtn->isChecked();
      QSettings().setValue("actuate/sae_l1_feature",
                           l1FeatureIsLaneKeep ? "lanekeep" : "acc");
      
      if (saeButtons.size() > 1) {
        saeButtons[1]->setChecked(true);
        saeBtnGroup->idClicked(1);   
      }
    };
    QObject::connect(l1LaneKeepBtn, &QPushButton::toggled, &window,
      [onL1FeaturePicked](bool on) { if (on) onL1FeaturePicked(); });
    QObject::connect(l1AccBtn, &QPushButton::toggled, &window,
      [onL1FeaturePicked](bool on) { if (on) onL1FeaturePicked(); });
  }
  saeGroup->setLayout(saeRow);
  actuateLayout->addWidget(saeGroup, 1);

  QHBoxLayout *backendBtnRow = new QHBoxLayout();
  backendBtnRow->setSpacing(4);
  backendBtnRow->setContentsMargins(0, 4, 0, 0);
  QPushButton *backendLibCarlaBtn = new QPushButton("LibCarla");
  QPushButton *backendPythonBtn   = new QPushButton("Python API");
  backendLibCarlaBtn->setFixedHeight(28);
  backendPythonBtn->setFixedHeight(28);
  backendLibCarlaBtn->setCheckable(true);
  backendPythonBtn->setCheckable(true);
  backendLibCarlaBtn->setToolTip(
    "<b>LibCarla — in-app driver</b><br>"
    "Studio's bundled <code>carla-client</code> drives the ego vehicle "
    "directly via RPC. Lowest latency; works without any extra Python "
    "install.");
  backendPythonBtn->setToolTip(
    "<b>Python API — manual_control.py</b><br>"
    "Spawns CARLA's official <code>PythonAPI/examples/manual_control.py</code> "
    "as a child process. Useful when you want to experiment with the "
    "official driver script alongside Studio.");
  auto *backendBtnGroup = new QButtonGroup(&window);
  backendBtnGroup->setExclusive(true);
  backendBtnGroup->addButton(backendLibCarlaBtn);
  backendBtnGroup->addButton(backendPythonBtn);
#ifndef CARLA_STUDIO_WITH_LIBCARLA
  backendLibCarlaBtn->setEnabled(false);
  backendLibCarlaBtn->setToolTip(
    "LibCarla unavailable — this Studio build was not linked against "
    "carla-client. Only Python API is usable.");
#endif
  backendBtnRow->addWidget(backendLibCarlaBtn, 1);
  backendBtnRow->addWidget(backendPythonBtn,   1);

  auto applyActuateBackend = [optDriverInApp, optDriverPython](const QString &mode) {
    const bool wantInApp = (mode == "libcarla");
    {
      QSignalBlocker bA(optDriverInApp);
      QSignalBlocker bP(optDriverPython);
#ifdef CARLA_STUDIO_WITH_LIBCARLA
      optDriverInApp->setChecked(wantInApp);
      optDriverPython->setChecked(!wantInApp);
#else
      Q_UNUSED(wantInApp);
      optDriverInApp->setChecked(false);
      optDriverPython->setChecked(true);
#endif
    }
    QSettings().setValue("actuate/backend", mode);
  };

  {
    QString saved = actuateSettings.value("actuate/backend", "libcarla").toString();
#ifndef CARLA_STUDIO_WITH_LIBCARLA
    saved = "python";
#endif
    if (saved == "libcarla") backendLibCarlaBtn->setChecked(true);
    else                     backendPythonBtn->setChecked(true);
    applyActuateBackend(saved);
  }
  QObject::connect(backendLibCarlaBtn, &QPushButton::toggled, &window,
    [applyActuateBackend](bool on) { if (on) applyActuateBackend("libcarla"); });
  QObject::connect(backendPythonBtn, &QPushButton::toggled, &window,
    [applyActuateBackend](bool on) { if (on) applyActuateBackend("python"); });

  auto syncBackendButtonsFromActions =
      [backendLibCarlaBtn, backendPythonBtn, optDriverInApp, optDriverPython]() {
    const QString mode = optDriverPython->isChecked() && !optDriverInApp->isChecked()
                         ? "python" : "libcarla";
    QSignalBlocker b1(backendLibCarlaBtn);
    QSignalBlocker b2(backendPythonBtn);
    if (mode == "libcarla") backendLibCarlaBtn->setChecked(true);
    else                    backendPythonBtn->setChecked(true);
    QSettings().setValue("actuate/backend", mode);
  };
  QObject::connect(optDriverInApp, &QAction::toggled, &window,
    [syncBackendButtonsFromActions](bool) { syncBackendButtonsFromActions(); });
  QObject::connect(optDriverPython, &QAction::toggled, &window,
    [syncBackendButtonsFromActions](bool) { syncBackendButtonsFromActions(); });

  QPushButton *rescanJsBtn = new QPushButton("↻");
  rescanJsBtn->setFixedSize(28, 28);
  rescanJsBtn->setToolTip("Re-scan /dev/input/js* for newly-connected "
                            "joysticks. Useful after plugging in a controller.");
  QObject::connect(rescanJsBtn, &QPushButton::clicked, &window,
    [detectedJoystickCount, detectJoystickCount, repopulateControlCombos]() {
      *detectedJoystickCount = detectJoystickCount();
      repopulateControlCombos();
    });
  backendBtnRow->addWidget(rescanJsBtn);

  actuateGroup->setLayout(actuateLayout);
  rightPaneLayout->addWidget(actuateGroup, 1);
  rightPaneLayout->addLayout(backendBtnRow);
  rightPane->setLayout(rightPaneLayout);

  interfacesContentLayout->addWidget(leftPane, 1);
  interfacesContentLayout->addWidget(rightPane, 1);

  interfacesLayout->addLayout(interfacesContentLayout, 1);
  protocolHostLayout->addLayout(interfaceGrid);
  w2->setLayout(interfacesLayout);
  openSensorConfigDialog = [&](const QString &sensorName) {
    QDialog dialog(&window);
    const int instanceCount = std::max(1, sensorInstanceCount.value(sensorName, 1));
    auto currentInstance = std::make_shared<int>(1);
    auto displayNameForInstance = [&](int idx) -> QString {
      const QString k = sensorMountKey(sensorName, idx);
      return QSettings().value(QString("sensor/displayname/%1").arg(k)).toString();
    };
    auto fallbackName = [&](int idx) -> QString {
      const QString custom = displayNameForInstance(idx);
      if (!custom.isEmpty()) return custom;
      return instanceCount > 1
        ? QString("%1 #%2").arg(sensorName).arg(idx)
        : sensorName;
    };
    dialog.setWindowTitle(QString("Calibration View - %1").arg(fallbackName(1)));
    dialog.resize(1500, 720);
    QVBoxLayout *dialogLayout = new QVBoxLayout();

    SensorMountConfig mount = getOrCreateSensorMount(sensorMountKey(sensorName, 1));

    QGroupBox *assemblyGroup = new QGroupBox("Assembly");
    QVBoxLayout *assemblyLayout = new QVBoxLayout();

    QHBoxLayout *instanceRow = new QHBoxLayout();
    instanceRow->setContentsMargins(0, 0, 0, 0);
    instanceRow->setSpacing(8);
    QLabel *instanceLbl = new QLabel(instanceCount > 1
      ? QString("%1 instances:").arg(instanceCount)
      : QString("Instance:"));
    QComboBox *instanceCombo = new QComboBox();
    for (int i = 1; i <= instanceCount; ++i) {
      instanceCombo->addItem(QString("%1 / %2").arg(i).arg(instanceCount), i);
    }
    instanceCombo->setEnabled(instanceCount > 1);
    QLineEdit *instanceNameEdit = new QLineEdit();
    instanceNameEdit->setPlaceholderText(instanceCount > 1
      ? QString("%1 #1").arg(sensorName)
      : sensorName);
    {
      const QString saved = displayNameForInstance(1);
      if (!saved.isEmpty()) instanceNameEdit->setText(saved);
    }
    instanceRow->addWidget(instanceLbl);
    instanceRow->addWidget(instanceCombo);
    instanceRow->addWidget(new QLabel("Name:"));
    instanceRow->addWidget(instanceNameEdit, 1);
    if (instanceCount <= 1) {
      instanceLbl->setVisible(false);
      instanceCombo->setVisible(false);
    }
    assemblyLayout->addLayout(instanceRow);

    enum class AssemblyViewKey {
      Top = 0,
      Left = 1,
      Right = 2,
      Front = 3,
      Rear = 4,
      Bottom = 5,
    };

    auto viewName = [&](AssemblyViewKey view) -> QString {
      switch (view) {
        case AssemblyViewKey::Top: return "Top";
        case AssemblyViewKey::Left: return "Left";
        case AssemblyViewKey::Right: return "Right";
        case AssemblyViewKey::Front: return "Front";
        case AssemblyViewKey::Rear: return "Rear";
        case AssemblyViewKey::Bottom: return "Bottom";
      }
      return "Top";
    };

    auto viewSubtitle = [&](AssemblyViewKey view) -> QString {
      switch (view) {
        case AssemblyViewKey::Top: return "Plan view (−Z)";
        case AssemblyViewKey::Left: return "Driver side (−Y)";
        case AssemblyViewKey::Right: return "Passenger (+Y)";
        case AssemblyViewKey::Front: return "From front (−X)";
        case AssemblyViewKey::Rear: return "From rear (+X)";
        case AssemblyViewKey::Bottom: return "Underside (+Z)";
      }
      return "Plan view (−Z)";
    };

    auto allViews = QList<AssemblyViewKey>()
      << AssemblyViewKey::Top
      << AssemblyViewKey::Left
      << AssemblyViewKey::Right
      << AssemblyViewKey::Front
      << AssemblyViewKey::Rear
      << AssemblyViewKey::Bottom;

    auto createViewCombo = [&](QComboBox *box, AssemblyViewKey selected, AssemblyViewKey blocked) {
      QSignalBlocker blocker(box);
      const int desired = static_cast<int>(selected);
      box->clear();
      for (AssemblyViewKey view : allViews) {
        if (view == blocked) {
          continue;
        }
        box->addItem(viewName(view), static_cast<int>(view));
      }
      int idx = box->findData(desired);
      if (idx < 0) {
        idx = 0;
      }
      box->setCurrentIndex(idx);
    };

    QHBoxLayout *assemblyPreviewLayout = new QHBoxLayout();
    assemblyPreviewLayout->setSpacing(8);

    auto buildViewSlot = [&](const QString &slotTitle, AssemblyViewKey initialView) {
      QWidget *slot = new QWidget();
      QVBoxLayout *slotLayout = new QVBoxLayout();
      slotLayout->setContentsMargins(0, 0, 0, 0);

      QHBoxLayout *slotHeader = new QHBoxLayout();
      QLabel *slotTitleLabel = new QLabel(slotTitle);
      slotTitleLabel->setStyleSheet("font-weight: bold;");
      QComboBox *viewBox = new QComboBox();
      slotHeader->addWidget(slotTitleLabel);
      slotHeader->addWidget(viewBox, 1);
      slotLayout->addLayout(slotHeader);

      QLabel *viewPreview = new QLabel();
      viewPreview->setFixedSize(290, 150);
      viewPreview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
      viewPreview->setAlignment(Qt::AlignCenter);
      slotLayout->addWidget(viewPreview);
      slot->setLayout(slotLayout);
      viewBox->setProperty("initialView", static_cast<int>(initialView));
      return std::tuple<QWidget *, QComboBox *, QLabel *>{slot, viewBox, viewPreview};
    };

    auto [primarySlot, primaryViewBox, primaryPreview] = buildViewSlot("View A", AssemblyViewKey::Front);
    auto [secondarySlot, secondaryViewBox, secondaryPreview] = buildViewSlot("View B", AssemblyViewKey::Right);
    assemblyPreviewLayout->addWidget(primarySlot, 1);
    assemblyPreviewLayout->addWidget(secondarySlot, 1);
    assemblyLayout->addLayout(assemblyPreviewLayout);

    QComboBox *framePresetBox = new QComboBox();
    framePresetBox->addItems(QStringList()
      << "Rear Baselink"
      << "Front Baselink"
      << "Vehicle Center"
      << "Rear Center"
      << "Front Center");
    framePresetBox->setCurrentText(mount.framePreset);

    QDoubleSpinBox *tx = new QDoubleSpinBox(); tx->setRange(-20.0, 20.0); tx->setDecimals(3); tx->setSingleStep(0.01); tx->setValue(mount.tx);
    QDoubleSpinBox *ty = new QDoubleSpinBox(); ty->setRange(-20.0, 20.0); ty->setDecimals(3); ty->setSingleStep(0.01); ty->setValue(mount.ty);
    QDoubleSpinBox *tz = new QDoubleSpinBox(); tz->setRange(-5.0, 10.0); tz->setDecimals(3); tz->setSingleStep(0.01); tz->setValue(mount.tz);
    QDoubleSpinBox *rx = new QDoubleSpinBox(); rx->setRange(0.0, 359.0); rx->setDecimals(1); rx->setSingleStep(0.5); rx->setValue(std::fmod(std::max(0.0, mount.rx), 360.0));
    QDoubleSpinBox *ry = new QDoubleSpinBox(); ry->setRange(0.0, 359.0); ry->setDecimals(1); ry->setSingleStep(0.5); ry->setValue(std::fmod(std::max(0.0, mount.ry), 360.0));
    QDoubleSpinBox *rz = new QDoubleSpinBox(); rz->setRange(0.0, 359.0); rz->setDecimals(1); rz->setSingleStep(0.5); rz->setValue(std::fmod(std::max(0.0, mount.rz), 360.0));

    QFormLayout *frameForm = new QFormLayout();
    frameForm->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    frameForm->setHorizontalSpacing(8);
    frameForm->addRow("Frame of Reference:", framePresetBox);
    assemblyLayout->addLayout(frameForm);

    auto axisCell = [&](QDoubleSpinBox *box, const QString &caption,
                        const QString &unit, const QString &tooltip) {
      QWidget *cell = new QWidget();
      QVBoxLayout *cellLayout = new QVBoxLayout(cell);
      cellLayout->setContentsMargins(0, 0, 0, 0);
      cellLayout->setSpacing(2);
      QLabel *cap = new QLabel(QString("%1 (%2)").arg(caption, unit));
      cap->setStyleSheet("QLabel { color: #6E7790; font-size: 9pt; }");
      cap->setAlignment(Qt::AlignLeft);
      box->setMinimumWidth(72);
      box->setToolTip(tooltip);
      cellLayout->addWidget(cap);
      cellLayout->addWidget(box);
      return cell;
    };

    QGroupBox *positionGroup = new QGroupBox("Position");
    positionGroup->setStyleSheet(
      "QGroupBox { font-weight: 600; }"
      "QGroupBox::title { color: #5E81AC; }");
    QHBoxLayout *positionRow = new QHBoxLayout(positionGroup);
    positionRow->setContentsMargins(8, 14, 8, 8);
    positionRow->addWidget(axisCell(tx, "X · forward", "m",
      "Distance forward (+) / back (−) from the chosen frame of reference."));
    positionRow->addWidget(axisCell(ty, "Y · right",   "m",
      "Distance right (+) / left (−) of the frame of reference."));
    positionRow->addWidget(axisCell(tz, "Z · up",      "m",
      "Height above (+) / below (−) the frame of reference."));
    assemblyLayout->addWidget(positionGroup);

    QGroupBox *orientationGroup = new QGroupBox("Orientation");
    orientationGroup->setStyleSheet(
      "QGroupBox { font-weight: 600; }"
      "QGroupBox::title { color: #B48EAD; }");
    QHBoxLayout *orientationRow = new QHBoxLayout(orientationGroup);
    orientationRow->setContentsMargins(8, 14, 8, 8);
    orientationRow->addWidget(axisCell(rx, "Roll",  "°",
      "Rotation around the X axis. CARLA applies Z-Y-X order."));
    orientationRow->addWidget(axisCell(ry, "Pitch", "°",
      "Rotation around the Y axis (nose up / down)."));
    orientationRow->addWidget(axisCell(rz, "Yaw",   "°",
      "Rotation around the Z axis (left / right heading)."));
    assemblyLayout->addWidget(orientationGroup);

    struct MountPreset {
      const char *label;
      const char *tooltip;
      double tx, ty, tz, rx, ry, rz;
    };
    static const std::array<MountPreset, 7> kMountPresets = {{
      {"Roof Center",   "Top-of-roof, looking forward",         2.4,  0.0,  1.6, 0,   0,    0},
      {"Front Bumper",  "Front bumper, low forward-facing",     3.7,  0.0,  0.4, 0,   0,    0},
      {"Rear Bumper",   "Rear bumper, looking back",            0.0,  0.0,  0.5, 0,   0,  180},
      {"Hood",          "Hood, slight downward pitch",          3.0,  0.0,  0.9, 0,  -5,    0},
      {"Trunk",         "Trunk lid, slight downward pitch back",-0.2, 0.0, 1.0, 0,  -5, 180},
      {"Mirror · Left", "Driver-side mirror, looking out left", 2.4, -0.9, 1.0, 0,   0,  -90},
      {"Mirror · Right","Passenger mirror, looking out right",  2.4,  0.9, 1.0, 0,   0,   90},
    }};
    QGroupBox *presetsGroup = new QGroupBox("Mount Presets");
    presetsGroup->setStyleSheet(
      "QGroupBox { font-weight: 600; }"
      "QGroupBox::title { color: #A3BE8C; }");
    QGridLayout *presetsGrid = new QGridLayout(presetsGroup);
    presetsGrid->setContentsMargins(8, 14, 8, 8);
    presetsGrid->setHorizontalSpacing(6);
    presetsGrid->setVerticalSpacing(4);
    int presetCol = 0, presetRow = 0;
    for (const MountPreset &p : kMountPresets) {
      QPushButton *b = new QPushButton(QString::fromLatin1(p.label));
      b->setToolTip(QString::fromLatin1(p.tooltip));
      b->setCursor(Qt::PointingHandCursor);
      const MountPreset captured = p;
      QObject::connect(b, &QPushButton::clicked, &dialog, [=, &dialog]() {
        Q_UNUSED(dialog);
        framePresetBox->setCurrentText("Rear Baselink");
        tx->setValue(captured.tx);
        ty->setValue(captured.ty);
        tz->setValue(captured.tz);
        rx->setValue(std::fmod(std::max(0.0, captured.rx + 360.0), 360.0));
        ry->setValue(std::fmod(std::max(0.0, captured.ry + 360.0), 360.0));
        rz->setValue(std::fmod(std::max(0.0, captured.rz + 360.0), 360.0));
      });
      presetsGrid->addWidget(b, presetRow, presetCol);
      ++presetCol;
      if (presetCol >= 4) { presetCol = 0; ++presetRow; }
    }
    QPushButton *resetBtn = new QPushButton("Reset");
    resetBtn->setToolTip("Zero out the mount pose (origin, no rotation).");
    QObject::connect(resetBtn, &QPushButton::clicked, &dialog, [=]() {
      tx->setValue(0); ty->setValue(0); tz->setValue(0);
      rx->setValue(0); ry->setValue(0); rz->setValue(0);
    });
    presetsGrid->addWidget(resetBtn, presetRow, presetCol);
    assemblyLayout->addWidget(presetsGroup);

    QLabel *axisHint = new QLabel(
      "<small>CARLA convention: <b>+X</b> forward · <b>+Y</b> right · "
      "<b>+Z</b> up · rotations applied in Z–Y–X order.</small>");
    axisHint->setStyleSheet("QLabel { color: #6E7790; padding: 2px 4px; }");
    axisHint->setWordWrap(true);
    axisHint->setTextFormat(Qt::RichText);
    assemblyLayout->addWidget(axisHint);

    assemblyGroup->setLayout(assemblyLayout);
    QHBoxLayout *bodyRow = new QHBoxLayout();
    bodyRow->setContentsMargins(0, 0, 0, 0);
    bodyRow->addWidget(assemblyGroup, 1);
    QWidget *configPanel = new QWidget();
    QVBoxLayout *configCol = new QVBoxLayout(configPanel);
    configCol->setContentsMargins(0, 0, 0, 0);
    bodyRow->addWidget(configPanel, 1);

#ifdef CARLA_STUDIO_WITH_QT3D
    QGroupBox *calibrationGroup = new QGroupBox("Calibration View");
    calibrationGroup->setStyleSheet(
      "QGroupBox { font-weight: 600; }"
      "QGroupBox::title { color: #D08770; }");
    QVBoxLayout *calibrationLayout = new QVBoxLayout(calibrationGroup);
    calibrationLayout->setContentsMargins(8, 14, 8, 8);

    auto *calibScene = new carla_studio::calibration::CalibrationScene();
    calibScene->setMinimumHeight(280);
    calibrationLayout->addWidget(calibScene, 1);

    QLabel *sceneHint = new QLabel(
      "Tip: pick a target type below, then click on the dark ground "
      "in the 3D scene to place it at that exact spot. Drag with the "
      "right mouse button to orbit, scroll to zoom.");
    sceneHint->setWordWrap(true);
    sceneHint->setStyleSheet(
      "QLabel { color: #6E7790; padding: 2px 4px; font-size: 9pt; "
      "background: #f5f5f5; border-radius: 3px; }");
    calibrationLayout->addWidget(sceneHint);

    QHBoxLayout *paletteRow = new QHBoxLayout();
    auto *paletteList = new QListWidget();
    paletteList->setDragEnabled(false);
    paletteList->setMaximumHeight(140);
    paletteList->setSelectionMode(QAbstractItemView::SingleSelection);
    paletteList->addItem("Checkerboard");
    paletteList->addItem("April Tag");
    paletteList->addItem("Color Checker");
    paletteList->addItem("Cone Square");
    paletteList->addItem("Cone Line");
    paletteList->addItem("Pylon Wall");
    paletteList->setCurrentRow(0);
    paletteRow->addWidget(paletteList, 1);

    QVBoxLayout *paletteBtnCol = new QVBoxLayout();
    QPushButton *addBtn   = new QPushButton("Add at center");
    QPushButton *clearBtn = new QPushButton("Clear all");
    paletteBtnCol->addWidget(addBtn);
    paletteBtnCol->addWidget(clearBtn);
    paletteBtnCol->addStretch();
    paletteRow->addLayout(paletteBtnCol);
    calibrationLayout->addLayout(paletteRow);

    auto *placedList = new QListWidget();
    placedList->setMaximumHeight(120);
    calibrationLayout->addWidget(new QLabel("Placed targets:"));
    calibrationLayout->addWidget(placedList);

    QGroupBox *coordEditGroup = new QGroupBox("Selected target pose");
    QGridLayout *coordGrid = new QGridLayout(coordEditGroup);
    coordGrid->setContentsMargins(6, 12, 6, 6);
    QDoubleSpinBox *coordX = new QDoubleSpinBox();
    QDoubleSpinBox *coordY = new QDoubleSpinBox();
    QDoubleSpinBox *coordZ = new QDoubleSpinBox();
    QDoubleSpinBox *coordYaw = new QDoubleSpinBox();
    for (auto *sp : {coordX, coordY, coordZ}) {
      sp->setRange(-2000.0, 2000.0); sp->setDecimals(2); sp->setSingleStep(0.1);
      sp->setEnabled(false);
    }
    coordYaw->setRange(-180.0, 180.0); coordYaw->setDecimals(1);
    coordYaw->setSingleStep(5.0); coordYaw->setEnabled(false);
    coordGrid->addWidget(new QLabel("X (m)"),   0, 0); coordGrid->addWidget(coordX,   0, 1);
    coordGrid->addWidget(new QLabel("Y (m)"),   0, 2); coordGrid->addWidget(coordY,   0, 3);
    coordGrid->addWidget(new QLabel("Z (m)"),   1, 0); coordGrid->addWidget(coordZ,   1, 1);
    coordGrid->addWidget(new QLabel("Yaw (°)"), 1, 2); coordGrid->addWidget(coordYaw, 1, 3);
    calibrationLayout->addWidget(coordEditGroup);

    QHBoxLayout *applyRow = new QHBoxLayout();
    QPushButton *removeBtn = new QPushButton("Remove selected");
    QPushButton *applyBtn  = new QPushButton("Save && Apply");
    applyRow->addWidget(removeBtn);
    applyRow->addStretch();
    applyRow->addWidget(applyBtn);
    calibrationLayout->addLayout(applyRow);

    QLabel *calibStatus = new QLabel("(idle)");
    calibStatus->setStyleSheet(
      "QLabel { color: #6E7790; padding: 2px 4px; font-size: 9pt; }");
    calibStatus->setWordWrap(true);
    calibrationLayout->addWidget(calibStatus);

    bodyRow->addWidget(calibrationGroup, 2);

    const QString scenarioId = qEnvironmentVariableIsSet("CARLA_STUDIO_SCENARIO")
      ? qEnvironmentVariable("CARLA_STUDIO_SCENARIO")
      : QStringLiteral("default");
    calibScene->setTargets(
      carla_studio::calibration::loadPersisted(scenarioId, sensorName));

    auto syncCoordEditor = [calibScene, coordX, coordY, coordZ, coordYaw]() {
      const int sel = calibScene->selectedIndex();
      const auto ts = calibScene->targets();
      const bool valid = (sel >= 0 && sel < ts.size());
      for (auto *sp : {coordX, coordY, coordZ, coordYaw}) sp->setEnabled(valid);
      if (!valid) return;
      const auto &t = ts[sel];
      QSignalBlocker b1(coordX), b2(coordY), b3(coordZ), b4(coordYaw);
      coordX->setValue(t.x);
      coordY->setValue(t.y);
      coordZ->setValue(t.z);
      coordYaw->setValue(t.yawDeg);
    };

    auto refreshPlacedList = [placedList, calibScene, syncCoordEditor]() {
      placedList->clear();
      const auto ts = calibScene->targets();
      for (int i = 0; i < ts.size(); ++i) {
        const auto &t = ts[i];
        placedList->addItem(QString("%1: %2  (%3, %4) z=%5")
          .arg(i + 1)
          .arg(carla_studio::calibration::labelFor(t.type))
          .arg(t.x, 0, 'f', 2).arg(t.y, 0, 'f', 2).arg(t.z, 0, 'f', 2));
      }
      const int sel = calibScene->selectedIndex();
      if (sel >= 0 && sel < placedList->count())
        placedList->setCurrentRow(sel);
      syncCoordEditor();
    };
    refreshPlacedList();
    QObject::connect(calibScene, &carla_studio::calibration::CalibrationScene::targetsChanged,
                     &dialog, [refreshPlacedList](const auto &) { refreshPlacedList(); });
    QObject::connect(calibScene, &carla_studio::calibration::CalibrationScene::selectionChanged,
                     &dialog, [refreshPlacedList](int) { refreshPlacedList(); });

    QObject::connect(paletteList, &QListWidget::currentRowChanged, &dialog,
      [calibScene](int row) {
        if (row >= 0 && row <= 5)
          calibScene->setNextDropType(
            static_cast<carla_studio::calibration::TargetType>(row));
      });
    if (paletteList->currentRow() >= 0)
      calibScene->setNextDropType(
        static_cast<carla_studio::calibration::TargetType>(paletteList->currentRow()));

    QObject::connect(placedList, &QListWidget::currentRowChanged, &dialog,
      [syncCoordEditor](int) { syncCoordEditor(); });

    auto pushCoords = [calibScene, coordX, coordY, coordZ, coordYaw]() {
      const int sel = calibScene->selectedIndex();
      if (sel < 0) return;
      calibScene->updateTargetPose(sel,
        coordX->value(), coordY->value(), coordZ->value(), coordYaw->value());
    };
    QObject::connect(coordX,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     &dialog, [pushCoords](double) { pushCoords(); });
    QObject::connect(coordY,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     &dialog, [pushCoords](double) { pushCoords(); });
    QObject::connect(coordZ,   QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     &dialog, [pushCoords](double) { pushCoords(); });
    QObject::connect(coordYaw, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                     &dialog, [pushCoords](double) { pushCoords(); });

    QObject::connect(addBtn, &QPushButton::clicked, &dialog,
      [calibScene, paletteList]() {
        const int idx = paletteList->currentRow();
        if (idx < 0) return;
        calibScene->addTargetAtCenter(
          static_cast<carla_studio::calibration::TargetType>(idx));
      });
    QObject::connect(clearBtn, &QPushButton::clicked, &dialog,
      [calibScene]() { calibScene->clearAll(); });
    QObject::connect(removeBtn, &QPushButton::clicked, &dialog,
      [calibScene, placedList]() {
        const int idx = placedList->currentRow();
        if (idx >= 0) calibScene->removeAt(idx);
      });
    QObject::connect(applyBtn, &QPushButton::clicked, &dialog,
      [calibScene, calibStatus, scenarioId, sensorName]() {
        const auto ts = calibScene->targets();
        carla_studio::calibration::savePersisted(scenarioId, sensorName, ts);
        calibStatus->setText(
          QString("Saved %1 target(s). Will spawn on next START / immediately if sim is running.")
            .arg(ts.size()));
      });
#endif

    dialogLayout->addLayout(bodyRow, 1);

    auto renderAssemblyView = [&](AssemblyViewKey view, QLabel *target) {
      QPixmap pix(target->width(), target->height());
      pix.fill(QColor("#dde7f4"));
      QPainter painter(&pix);
      painter.setRenderHint(QPainter::Antialiasing, true);

      const QRectF drawArea(8, 8, pix.width() - 16, pix.height() - 16);
      painter.setPen(QPen(QColor("#3b5a8a"), 1));
      painter.setBrush(QColor("#e8efff"));
      painter.drawRoundedRect(drawArea, 8, 8);

      painter.setPen(QPen(QColor("#9bb3d6"), 0.6, Qt::DotLine));
      const double gridStep = 16.0;
      for (double x = drawArea.left() + gridStep; x < drawArea.right(); x += gridStep) {
        painter.drawLine(QPointF(x, drawArea.top() + 1), QPointF(x, drawArea.bottom() - 1));
      }
      for (double y = drawArea.top() + gridStep; y < drawArea.bottom(); y += gridStep) {
        painter.drawLine(QPointF(drawArea.left() + 1, y), QPointF(drawArea.right() - 1, y));
      }

      painter.setPen(QPen(QColor("#5a7bb0"), 1, Qt::DashLine));
      painter.drawLine(QPointF(drawArea.left(), drawArea.center().y()), QPointF(drawArea.right(), drawArea.center().y()));

      const auto origin = frameOriginRearBaselink(framePresetBox->currentText());
      const double xRearBaselink = tx->value() + origin.first;
      const double yRearBaselink = ty->value();
      const double zRearBaselink = tz->value() + origin.second;

      auto putSensor = [&](const QPointF &pt, const QColor &color, const QString &label, const QPointF &labelOffset, double angleDeg, bool showCone = true) {
        painter.setPen(QPen(color, 2));
        painter.setBrush(color);
        painter.drawEllipse(pt, 3.5, 3.5);
        if (showCone) {
          painter.save();
          painter.translate(pt);
          painter.rotate(angleDeg);
          QPolygonF cone;
          cone << QPointF(0, 0)
               << QPointF(10, -5)
               << QPointF(15, 0)
               << QPointF(10, 5);
          painter.setBrush(QColor(color.red(), color.green(), color.blue(), 36));
          painter.drawPolygon(cone);
          painter.restore();
        }
        painter.setPen(QPen(color, 1));
        painter.drawText(pt + labelOffset, label);
      };

      auto drawTopStyle = [&](bool invertedY) {
        const double carLeft = drawArea.left() + 16;
        const double carRight = drawArea.right() - 16;
        const double carTop = drawArea.top() + 22;
        const double carBottom = drawArea.bottom() - 18;
        const double carMidY = drawArea.center().y();

        painter.setPen(QPen(QColor("#1e3a8a"), 1.4));
        painter.setBrush(QColor("#cdd9ee"));
        QPainterPath carBody;
        carBody.moveTo(carLeft + 16, carTop + 8);
        carBody.lineTo(carRight - 24, carTop + 8);
        carBody.quadTo(carRight - 8, carTop + 8, carRight - 8, carMidY);
        carBody.quadTo(carRight - 8, carBottom - 8, carRight - 22, carBottom - 8);
        carBody.lineTo(carLeft + 22, carBottom - 8);
        carBody.quadTo(carLeft + 8, carBottom - 8, carLeft + 8, carMidY);
        carBody.quadTo(carLeft + 8, carTop + 8, carLeft + 16, carTop + 8);
        painter.drawPath(carBody);

        painter.setPen(QPen(QColor("#2c4a78"), 0.9));
        painter.setBrush(QColor("#b6c7e2"));
        painter.drawRoundedRect(QRectF(carLeft + 40, carTop + 12, (carRight - carLeft) - 80, (carBottom - carTop) - 24), 10, 10);

        painter.setPen(QPen(QColor("#1e3a8a"), 1));
        painter.setBrush(QColor("#3b5a8a"));
        for (double wx : {carLeft + 22.0, carRight - 22.0}) {
          painter.drawEllipse(QPointF(wx, carTop + 6), 5, 5);
          painter.drawEllipse(QPointF(wx, carBottom - 6), 5, 5);
        }

        painter.setPen(QPen(QColor("#5a7bb0"), 1, Qt::DashLine));
        painter.drawLine(QPointF(drawArea.left() + 8, drawArea.center().y()), QPointF(drawArea.right() - 8, drawArea.center().y()));

        auto mapTopX = [&](double xMeters) {
          const double minX = -1.2;
          const double maxX = vehicleLengthM + 0.4;
          return drawArea.left() + ((xMeters - minX) / (maxX - minX)) * drawArea.width();
        };
        auto mapTopY = [&](double yMeters) {
          const double minY = -1.3;
          const double maxY = 1.3;
          double v = invertedY ? -yMeters : yMeters;
          return drawArea.bottom() - ((v - minY) / (maxY - minY)) * drawArea.height();
        };

        const QPointF sensorPoint(mapTopX(xRearBaselink), mapTopY(yRearBaselink));
        putSensor(sensorPoint, QColor("#ff6b1f"), sensorName, QPointF(8, -8), rz->value());

        painter.setPen(QPen(QColor("#1e3a8a"), 1));
        painter.drawLine(QPointF(drawArea.left() + 14, drawArea.center().y()), QPointF(drawArea.left() + 38, drawArea.center().y()));
        painter.drawLine(QPointF(drawArea.left() + 28, drawArea.center().y() - 14), QPointF(drawArea.left() + 28, drawArea.center().y() + 14));
        painter.drawText(QPointF(drawArea.left() + 42, drawArea.center().y() - 2), "+X");
        painter.drawText(QPointF(drawArea.left() + 22, drawArea.center().y() - 18), invertedY ? "−Y" : "+Y");
      };

      auto drawSideStyle = [&](bool mirrored) {
        const double bodyLeft = drawArea.left() + 16;
        const double bodyRight = drawArea.right() - 16;
        const double groundY = drawArea.bottom() - 16;
        const double roofY = drawArea.top() + 26;
        const double wheelR = 7;

        painter.setPen(QPen(QColor("#1e3a8a"), 1.4));
        painter.setBrush(QColor("#cdd9ee"));
        QPainterPath profile;
        profile.moveTo(bodyLeft + 6, groundY - 6);
        profile.lineTo(bodyLeft + 10, groundY - 22);
        profile.quadTo(bodyLeft + 22, roofY + 6, bodyLeft + 54, roofY);
        profile.lineTo(bodyRight - 58, roofY);
        profile.quadTo(bodyRight - 22, roofY + 6, bodyRight - 10, groundY - 22);
        profile.lineTo(bodyRight - 6, groundY - 6);
        profile.lineTo(bodyRight - 18, groundY - 6);
        profile.lineTo(bodyRight - 26, groundY - 1);
        profile.lineTo(bodyLeft + 26, groundY - 1);
        profile.lineTo(bodyLeft + 18, groundY - 6);
        profile.closeSubpath();
        if (mirrored) {
          painter.save();
          painter.translate(drawArea.center().x() * 2, 0);
          painter.scale(-1, 1);
          painter.drawPath(profile);
          painter.restore();
        } else {
          painter.drawPath(profile);
        }

        painter.setPen(QPen(QColor("#2c4a78"), 0.9));
        painter.setBrush(QColor("#b6c7e2"));
        painter.drawRoundedRect(QRectF(bodyLeft + 32, roofY + 3, (bodyRight - bodyLeft) - 64, 18), 5, 5);

        painter.setPen(QPen(QColor("#1e3a8a"), 1));
        painter.setBrush(QColor("#3b5a8a"));
        painter.drawEllipse(QPointF(bodyLeft + 32, groundY - 2), wheelR, wheelR);
        painter.drawEllipse(QPointF(bodyRight - 32, groundY - 2), wheelR, wheelR);

        painter.setPen(QPen(QColor("#5a7bb0"), 1, Qt::DashLine));
        painter.drawLine(QPointF(drawArea.left() + 8, groundY - 14), QPointF(drawArea.right() - 8, groundY - 14));

        auto mapSideX = [&](double xMeters) {
          const double minX = -1.2;
          const double maxX = vehicleLengthM + 0.4;
          return drawArea.left() + ((xMeters - minX) / (maxX - minX)) * drawArea.width();
        };
        auto mapSideZ = [&](double zMeters) {
          const double minZ = -0.1;
          const double maxZ = 2.0;
          return drawArea.bottom() - ((zMeters - minZ) / (maxZ - minZ)) * drawArea.height();
        };

        const QPointF sensorPoint(mapSideX(xRearBaselink), mapSideZ(zRearBaselink));
        putSensor(sensorPoint, QColor("#ff6b1f"), sensorName, QPointF(8, -8), mirrored ? ry->value() : rx->value());

        painter.setPen(QPen(QColor("#1e3a8a"), 1));
        painter.drawLine(QPointF(drawArea.left() + 14, groundY - 14), QPointF(drawArea.left() + 38, groundY - 14));
        painter.drawLine(QPointF(drawArea.left() + 28, groundY - 26), QPointF(drawArea.left() + 28, groundY - 2));
        painter.drawText(QPointF(drawArea.left() + 42, groundY - 10), "+X");
        painter.drawText(QPointF(drawArea.left() + 22, groundY - 30), "+Z");
      };

      auto drawFrontRearStyle = [&](bool rearView) {
        const double bodyLeft = drawArea.left() + 54;
        const double bodyRight = drawArea.right() - 54;
        const double groundY = drawArea.bottom() - 16;
        const double roofY = drawArea.top() + 26;
        const double bodyBottom = groundY - 6;
        const double wheelR = 7;

        painter.setPen(QPen(QColor("#1e3a8a"), 1.4));
        painter.setBrush(QColor("#cdd9ee"));
        QPainterPath face;
        face.moveTo(bodyLeft + 24, bodyBottom);
        face.quadTo(drawArea.center().x(), roofY - 2, bodyRight - 24, bodyBottom);
        face.lineTo(bodyRight - 20, bodyBottom - 12);
        face.lineTo(bodyLeft + 20, bodyBottom - 12);
        face.closeSubpath();
        painter.drawPath(face);

        painter.setPen(QPen(QColor("#2c4a78"), 0.9));
        painter.setBrush(QColor("#b6c7e2"));
        painter.drawRoundedRect(QRectF(drawArea.center().x() - 28, roofY + 6, 56, 20), 6, 6);

        painter.setPen(QPen(QColor("#1e3a8a"), 1));
        painter.setBrush(QColor("#3b5a8a"));
        painter.drawRect(QRectF(bodyLeft + 2, bodyBottom - 30, 18, 48));
        painter.drawRect(QRectF(bodyRight - 20, bodyBottom - 30, 18, 48));
        painter.drawEllipse(QPointF(bodyLeft + 12, bodyBottom), wheelR, wheelR);
        painter.drawEllipse(QPointF(bodyRight - 12, bodyBottom), wheelR, wheelR);

        painter.setPen(QPen(QColor("#5a7bb0"), 1, Qt::DashLine));
        painter.drawLine(QPointF(drawArea.left() + 8, drawArea.bottom() - 24), QPointF(drawArea.right() - 8, drawArea.bottom() - 24));

        auto mapFrontY = [&](double yMeters) {
          const double minY = -1.3;
          const double maxY = 1.3;
          return drawArea.center().x() + (rearView ? -1.0 : 1.0) * ((yMeters - minY) / (maxY - minY) - 0.5) * (drawArea.width() - 40);
        };
        auto mapFrontZ = [&](double zMeters) {
          const double minZ = -0.1;
          const double maxZ = 2.0;
          return drawArea.bottom() - ((zMeters - minZ) / (maxZ - minZ)) * drawArea.height();
        };

        const QPointF sensorPoint(mapFrontY(yRearBaselink), mapFrontZ(zRearBaselink));
        putSensor(sensorPoint, QColor("#ff6b1f"), sensorName, QPointF(8, -8), rearView ? -ry->value() : ry->value());

        painter.setPen(QPen(QColor("#1e3a8a"), 1));
        painter.drawLine(QPointF(drawArea.center().x(), drawArea.bottom() - 20), QPointF(drawArea.center().x(), drawArea.bottom() - 44));
        painter.drawText(QPointF(drawArea.center().x() + 5, drawArea.bottom() - 24), rearView ? "−X" : "+X");
        painter.drawText(QPointF(drawArea.center().x() + 22, drawArea.bottom() - 8), "±Y");
      };

      painter.setPen(QPen(QColor("#1e3a8a"), 1));
      painter.setBrush(Qt::NoBrush);
      painter.drawText(QRectF(12, 10, drawArea.width(), 16), Qt::AlignLeft | Qt::AlignVCenter, viewName(view));
      painter.drawText(QRectF(12, 22, drawArea.width(), 16), Qt::AlignLeft | Qt::AlignVCenter, viewSubtitle(view));

      switch (view) {
        case AssemblyViewKey::Top: drawTopStyle(false); break;
        case AssemblyViewKey::Bottom: drawTopStyle(true); break;
        case AssemblyViewKey::Left: drawSideStyle(false); break;
        case AssemblyViewKey::Right: drawSideStyle(true); break;
        case AssemblyViewKey::Front: drawFrontRearStyle(false); break;
        case AssemblyViewKey::Rear: drawFrontRearStyle(true); break;
      }

      target->setPixmap(pix);
    };

    auto refreshViewSelectors = [&]() {
      auto resolveSelection = [](QComboBox *box) -> AssemblyViewKey {
        if (box->count() == 0) {
          return static_cast<AssemblyViewKey>(box->property("initialView").toInt());
        }
        return static_cast<AssemblyViewKey>(box->currentData().toInt());
      };
      AssemblyViewKey leftSelection  = resolveSelection(primaryViewBox);
      AssemblyViewKey rightSelection = resolveSelection(secondaryViewBox);
      if (leftSelection == rightSelection) {
        for (AssemblyViewKey k : allViews) {
          if (k != leftSelection) { rightSelection = k; break; }
        }
      }
      createViewCombo(primaryViewBox,   leftSelection,  rightSelection);
      createViewCombo(secondaryViewBox, rightSelection, leftSelection);
    };

    auto updateAssemblyPreview = [&]() {
      renderAssemblyView(static_cast<AssemblyViewKey>(primaryViewBox->currentData().toInt()), primaryPreview);
      renderAssemblyView(static_cast<AssemblyViewKey>(secondaryViewBox->currentData().toInt()), secondaryPreview);
    };

    QObject::connect(primaryViewBox, qOverload<int>(&QComboBox::currentIndexChanged), &dialog, [&](int) { refreshViewSelectors(); updateAssemblyPreview(); });
    QObject::connect(secondaryViewBox, qOverload<int>(&QComboBox::currentIndexChanged), &dialog, [&](int) { refreshViewSelectors(); updateAssemblyPreview(); });
    QObject::connect(framePresetBox, &QComboBox::currentTextChanged, &dialog, [&](const QString &) { updateAssemblyPreview(); });
    QObject::connect(tx, qOverload<double>(&QDoubleSpinBox::valueChanged), &dialog, [&](double) { updateAssemblyPreview(); });
    QObject::connect(ty, qOverload<double>(&QDoubleSpinBox::valueChanged), &dialog, [&](double) { updateAssemblyPreview(); });
    QObject::connect(tz, qOverload<double>(&QDoubleSpinBox::valueChanged), &dialog, [&](double) { updateAssemblyPreview(); });
    QObject::connect(rx, qOverload<double>(&QDoubleSpinBox::valueChanged), &dialog, [&](double) { updateAssemblyPreview(); });
    QObject::connect(ry, qOverload<double>(&QDoubleSpinBox::valueChanged), &dialog, [&](double) { updateAssemblyPreview(); });
    QObject::connect(rz, qOverload<double>(&QDoubleSpinBox::valueChanged), &dialog, [&](double) { updateAssemblyPreview(); });
    refreshViewSelectors();
    updateAssemblyPreview();

    auto writeUiToInstance = [&](int idx) {
      SensorMountConfig m;
      m.framePreset = framePresetBox->currentText();
      m.tx = tx->value(); m.ty = ty->value(); m.tz = tz->value();
      m.rx = rx->value(); m.ry = ry->value(); m.rz = rz->value();
      sensorMountConfigs[sensorMountKey(sensorName, idx)] = m;
    };
    auto loadInstanceIntoUi = [&](int idx) {
      const SensorMountConfig m = getOrCreateSensorMount(sensorMountKey(sensorName, idx));
      framePresetBox->setCurrentText(m.framePreset);
      tx->setValue(m.tx); ty->setValue(m.ty); tz->setValue(m.tz);
      rx->setValue(std::fmod(std::max(0.0, m.rx), 360.0));
      ry->setValue(std::fmod(std::max(0.0, m.ry), 360.0));
      rz->setValue(std::fmod(std::max(0.0, m.rz), 360.0));
      const QString custom = displayNameForInstance(idx);
      QSignalBlocker b(instanceNameEdit);
      instanceNameEdit->setText(custom);
      instanceNameEdit->setPlaceholderText(instanceCount > 1
        ? QString("%1 #%2").arg(sensorName).arg(idx)
        : sensorName);
      dialog.setWindowTitle(QString("Sensor Config - %1").arg(fallbackName(idx)));
      updateAssemblyPreview();
    };
    QObject::connect(instanceCombo, qOverload<int>(&QComboBox::currentIndexChanged), &dialog,
      [&, currentInstance](int ) {
        const int newIdx = instanceCombo->currentData().toInt();
        if (newIdx == *currentInstance) return;
        writeUiToInstance(*currentInstance);
        *currentInstance = newIdx;
        loadInstanceIntoUi(newIdx);
      });
    QObject::connect(instanceNameEdit, &QLineEdit::textEdited, &dialog,
      [&, currentInstance](const QString &text) {
        const QString k = sensorMountKey(sensorName, *currentInstance);
        QSettings().setValue(QString("sensor/displayname/%1").arg(k), text);
        dialog.setWindowTitle(QString("Sensor Config - %1").arg(fallbackName(*currentInstance)));
      });

    QGroupBox *characteristicsGroup = new QGroupBox("Sensor Characteristics");
    QVBoxLayout *characteristicsLayout = new QVBoxLayout();
    QFormLayout *form = new QFormLayout();

    if (sensorName == "RGB" || sensorName == "Depth" || sensorName == "Semantic Seg" || sensorName == "Instance Seg" || sensorName == "DVS" || sensorName == "Optical Flow" || sensorName == "Fisheye") {
      QSpinBox *w = new QSpinBox(); w->setRange(64, 8192); w->setValue(800);
      QSpinBox *h = new QSpinBox(); h->setRange(64, 8192); h->setValue(600);
      QDoubleSpinBox *fov = new QDoubleSpinBox(); fov->setRange(1.0, 179.0); fov->setValue(90.0);
      QDoubleSpinBox *tick = new QDoubleSpinBox(); tick->setRange(0.0, 10.0); tick->setDecimals(3); tick->setValue(0.0);
      form->addRow("image_size_x", w);
      form->addRow("image_size_y", h);
      form->addRow("fov", fov);
      form->addRow("sensor_tick", tick);
      if (sensorName.contains("Fisheye")) {
        QDoubleSpinBox *lensK = new QDoubleSpinBox(); lensK->setRange(-100.0, 100.0); lensK->setValue(-1.0);
        QDoubleSpinBox *lensKcube = new QDoubleSpinBox(); lensKcube->setRange(-100.0, 100.0); lensKcube->setValue(0.0);
        QDoubleSpinBox *lensCircle = new QDoubleSpinBox(); lensCircle->setRange(0.0, 10.0); lensCircle->setValue(5.0);
        QDoubleSpinBox *lensX = new QDoubleSpinBox(); lensX->setRange(0.0, 1.0); lensX->setSingleStep(0.01); lensX->setValue(0.08);
        QDoubleSpinBox *lensY = new QDoubleSpinBox(); lensY->setRange(0.0, 1.0); lensY->setSingleStep(0.01); lensY->setValue(0.08);
        QDoubleSpinBox *chromAb = new QDoubleSpinBox(); chromAb->setRange(0.0, 5.0); chromAb->setSingleStep(0.01); chromAb->setValue(0.12);
        QDoubleSpinBox *vignette = new QDoubleSpinBox(); vignette->setRange(0.0, 1.0); vignette->setSingleStep(0.01); vignette->setValue(0.20);
        form->addRow("lens_k", lensK);
        form->addRow("lens_kcube", lensKcube);
        form->addRow("lens_circle_falloff", lensCircle);
        form->addRow("lens_x_size", lensX);
        form->addRow("lens_y_size", lensY);
        form->addRow("chromatic_aberration", chromAb);
        form->addRow("vignette_intensity", vignette);
      }
    } else if (sensorName == "Front" || sensorName == "Rear" || sensorName == "Long-Range") {
      QDoubleSpinBox *hfov = new QDoubleSpinBox(); hfov->setRange(1.0, 180.0); hfov->setValue(30.0);
      QDoubleSpinBox *vfov = new QDoubleSpinBox(); vfov->setRange(1.0, 180.0); vfov->setValue(30.0);
      QDoubleSpinBox *range = new QDoubleSpinBox(); range->setRange(1.0, 10000.0); range->setValue(sensorName == "Long-Range" ? 300.0 : 100.0);
      QSpinBox *pps = new QSpinBox(); pps->setRange(1, 1000000); pps->setValue(sensorName == "Long-Range" ? 2500 : 1500);
      QDoubleSpinBox *tick = new QDoubleSpinBox(); tick->setRange(0.0, 10.0); tick->setDecimals(3); tick->setValue(0.0);
      form->addRow("horizontal_fov", hfov);
      form->addRow("vertical_fov", vfov);
      form->addRow("range", range);
      form->addRow("points_per_second", pps);
      form->addRow("sensor_tick", tick);
    } else if (sensorName == "Ray Cast" || sensorName == "Semantic" || sensorName == "Roof 360") {
      QSpinBox *channels = new QSpinBox(); channels->setRange(1, 256); channels->setValue(32);
      QDoubleSpinBox *range = new QDoubleSpinBox(); range->setRange(1.0, 10000.0); range->setValue(sensorName == "Roof 360" ? 120.0 : 10.0);
      QSpinBox *pps = new QSpinBox(); pps->setRange(1, 5000000); pps->setValue(56000);
      QDoubleSpinBox *rot = new QDoubleSpinBox(); rot->setRange(0.1, 200.0); rot->setValue(10.0);
      QDoubleSpinBox *upper = new QDoubleSpinBox(); upper->setRange(-90.0, 90.0); upper->setValue(10.0);
      QDoubleSpinBox *lower = new QDoubleSpinBox(); lower->setRange(-90.0, 90.0); lower->setValue(-30.0);
      QDoubleSpinBox *hfov = new QDoubleSpinBox(); hfov->setRange(0.0, 360.0); hfov->setValue(360.0);
      QDoubleSpinBox *tick = new QDoubleSpinBox(); tick->setRange(0.0, 10.0); tick->setDecimals(3); tick->setValue(0.0);
      form->addRow("channels", channels);
      form->addRow("range", range);
      form->addRow("points_per_second", pps);
      form->addRow("rotation_frequency", rot);
      form->addRow("upper_fov", upper);
      form->addRow("lower_fov", lower);
      form->addRow("horizontal_fov", hfov);
      form->addRow("sensor_tick", tick);
      if (sensorName != "Semantic") {
        QDoubleSpinBox *noise = new QDoubleSpinBox(); noise->setRange(0.0, 10.0); noise->setSingleStep(0.01); noise->setValue(0.0);
        form->addRow("noise_stddev", noise);
      }
    } else {
      QDoubleSpinBox *tick = new QDoubleSpinBox(); tick->setRange(0.0, 10.0); tick->setDecimals(3); tick->setValue(0.0);
      form->addRow("sensor_tick", tick);
      if (sensorName == "GNSS") {
        QDoubleSpinBox *noiseLat = new QDoubleSpinBox(); noiseLat->setRange(0.0, 100.0); noiseLat->setValue(0.0);
        QDoubleSpinBox *noiseLon = new QDoubleSpinBox(); noiseLon->setRange(0.0, 100.0); noiseLon->setValue(0.0);
        form->addRow("noise_lat_stddev", noiseLat);
        form->addRow("noise_lon_stddev", noiseLon);
      } else if (sensorName == "IMU") {
        QDoubleSpinBox *accelNoise = new QDoubleSpinBox(); accelNoise->setRange(0.0, 100.0); accelNoise->setValue(0.0);
        QDoubleSpinBox *gyroNoise = new QDoubleSpinBox(); gyroNoise->setRange(0.0, 100.0); gyroNoise->setValue(0.0);
        form->addRow("noise_accel_stddev_xyz", accelNoise);
        form->addRow("noise_gyro_stddev_xyz", gyroNoise);
      }
    }

    characteristicsLayout->addLayout(form);
    characteristicsGroup->setLayout(characteristicsLayout);
    dialogLayout->addWidget(characteristicsGroup);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, [&, currentInstance]() {
      writeUiToInstance(*currentInstance);
      saveSensorMountConfigs();
      dialog.accept();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (isCameraSensorName(sensorName)) {
      CameraOptics optics = cameraOpticsByName.value(sensorName, CameraOptics{});

      QGroupBox *optGroup = new QGroupBox("Lens / Optics");
      QFormLayout *optForm = new QFormLayout();

      auto synthesizeLensCsv = [](const QString &model, double fovDeg, bool withVignette) -> QString {
        QString out = QString("# Synthetic %1 (FOV %2°)\n").arg(model).arg(fovDeg);
        const double thetaMax = qDegreesToRadians(fovDeg) / 2.0;
        const int N = 64;
        for (int i = 0; i <= N; ++i) {
          const double r = static_cast<double>(i) / N;
          const double theta = thetaMax * r;
          double rp = r;
          if (model == "fisheye-equidistant") {
            rp = (theta) / thetaMax;
          } else if (model == "fisheye-equisolid") {
            rp = 2.0 * std::sin(theta / 2.0) / (2.0 * std::sin(thetaMax / 2.0));
          } else if (model == "rectilinear") {
            const double tMax = std::tan(thetaMax);
            rp = (tMax > 1e-9) ? std::tan(theta) / tMax : r;
          }
          QString row = QString("%1;%2").arg(r, 0, 'f', 5).arg(rp, 0, 'f', 5);
          if (withVignette) {
            const double v = std::pow(std::cos(theta), 4.0);
            row += QString(";%1").arg(v, 0, 'f', 5);
          }
          out += row + "\n";
        }
        return out;
      };
      auto lensModelKey = [](const QString &id) -> QString { return "synth://" + id; };
      QComboBox *presetCombo = new QComboBox();
      presetCombo->addItem("(Default rectilinear)",          QString());
      presetCombo->addItem("Fisheye · equidistant 180°",     lensModelKey("fisheye-equidistant-180"));
      presetCombo->addItem("Fisheye · equisolid 190°",       lensModelKey("fisheye-equisolid-190"));
      presetCombo->addItem("Rectilinear · 90° (mild)",       lensModelKey("rectilinear-90"));
      presetCombo->addItem("Rectilinear · 60° + vignette",   lensModelKey("rectilinear-60-vignette"));
      presetCombo->addItem("Custom (load CSV…)",              QString("__custom__"));
      const int restoreIdx = presetCombo->findText(optics.preset);
      if (restoreIdx >= 0) presetCombo->setCurrentIndex(restoreIdx);
      optForm->addRow("Preset:", presetCombo);

      QSpinBox *imgWSpin = new QSpinBox(); imgWSpin->setRange(64, 8192); imgWSpin->setValue(optics.imgW);
      QSpinBox *imgHSpin = new QSpinBox(); imgHSpin->setRange(64, 8192); imgHSpin->setValue(optics.imgH);
      QHBoxLayout *resRow = new QHBoxLayout();
      resRow->addWidget(imgWSpin); resRow->addWidget(new QLabel("×")); resRow->addWidget(imgHSpin);
      optForm->addRow("Resolution (px):", resRow);

      QDoubleSpinBox *fovSpin = new QDoubleSpinBox();
      fovSpin->setRange(10.0, 220.0); fovSpin->setSuffix("°"); fovSpin->setValue(optics.fov);
      optForm->addRow("Field of view:", fovSpin);

      QDoubleSpinBox *kSpin = new QDoubleSpinBox();
      kSpin->setRange(-2.0, 2.0); kSpin->setDecimals(4); kSpin->setSingleStep(0.01);
      kSpin->setValue(optics.lensK);
      kSpin->setToolTip("CARLA blueprint attribute lens_k — linear radial coefficient.");
      optForm->addRow("lens_k:", kSpin);

      QDoubleSpinBox *kcubeSpin = new QDoubleSpinBox();
      kcubeSpin->setRange(-2.0, 2.0); kcubeSpin->setDecimals(4); kcubeSpin->setSingleStep(0.01);
      kcubeSpin->setValue(optics.lensKcube);
      kcubeSpin->setToolTip("CARLA blueprint attribute lens_kcube — cubic radial coefficient.");
      optForm->addRow("lens_kcube:", kcubeSpin);

      QDoubleSpinBox *xSize = new QDoubleSpinBox();
      xSize->setRange(0.0, 1.0); xSize->setDecimals(4); xSize->setSingleStep(0.01);
      xSize->setValue(optics.lensXSize);
      QDoubleSpinBox *ySize = new QDoubleSpinBox();
      ySize->setRange(0.0, 1.0); ySize->setDecimals(4); ySize->setSingleStep(0.01);
      ySize->setValue(optics.lensYSize);
      QHBoxLayout *lensSizeRow = new QHBoxLayout();
      lensSizeRow->addWidget(xSize); lensSizeRow->addWidget(new QLabel("×")); lensSizeRow->addWidget(ySize);
      optForm->addRow("lens size (m):", lensSizeRow);

      QDoubleSpinBox *vFalloff = new QDoubleSpinBox();
      vFalloff->setRange(0.0, 20.0); vFalloff->setDecimals(2); vFalloff->setSingleStep(0.1);
      vFalloff->setValue(optics.lensCircleFalloff);
      vFalloff->setToolTip("CARLA blueprint attribute lens_circle_falloff — vignette exponent.");
      optForm->addRow("vignette falloff:", vFalloff);

      QDoubleSpinBox *vMult = new QDoubleSpinBox();
      vMult->setRange(0.0, 5.0); vMult->setDecimals(2); vMult->setSingleStep(0.1);
      vMult->setValue(optics.lensCircleMultiplier);
      vMult->setToolTip("CARLA blueprint attribute lens_circle_multiplier.");
      optForm->addRow("vignette mult:", vMult);

      QDoubleSpinBox *chromAb = new QDoubleSpinBox();
      chromAb->setRange(0.0, 5.0); chromAb->setDecimals(2); chromAb->setSingleStep(0.05);
      chromAb->setValue(optics.chromaticAberration);
      chromAb->setToolTip("CARLA blueprint attribute chromatic_aberration_intensity (RGB camera).");
      optForm->addRow("chromatic aberration:", chromAb);

      optGroup->setLayout(optForm);
      configCol->addWidget(optGroup);
      configCol->addStretch(1);

      auto applyPreset = [&, presetCombo, fovSpin, kSpin, kcubeSpin,
                          vFalloff, vMult, synthesizeLensCsv]() {
        const QString id = presetCombo->currentData().toString();
        if (id.isEmpty()) {
          kSpin->setValue(0.0);
          kcubeSpin->setValue(0.0);
          fovSpin->setValue(90.0);
          return;
        }
        QString csvText;
        QString hint = presetCombo->currentText();
        if (id == "__custom__") {
          const QString f = QFileDialog::getOpenFileName(&dialog,
            "Select lens CSV", QDir::homePath(),
            "Lens CSV (*.csv);;All files (*)");
          if (f.isEmpty()) return;
          QFile in(f);
          if (!in.open(QIODevice::ReadOnly)) return;
          csvText = QString::fromUtf8(in.readAll());
          hint = QFileInfo(f).fileName();
        } else if (id.startsWith("synth://")) {
          const QString tag = id.mid(QString("synth://").size());
          if      (tag == "fisheye-equidistant-180")   csvText = synthesizeLensCsv("fisheye-equidistant", 180.0, false);
          else if (tag == "fisheye-equisolid-190")     csvText = synthesizeLensCsv("fisheye-equisolid",   190.0, false);
          else if (tag == "rectilinear-90")            csvText = synthesizeLensCsv("rectilinear",          90.0, false);
          else if (tag == "rectilinear-60-vignette")   csvText = synthesizeLensCsv("rectilinear",          60.0, true);
          hint = tag;
        }
        if (csvText.isEmpty()) return;
        const LensFit fit = fitRadialLensCsv(csvText, hint);
        fovSpin->setValue(fit.fov);
        kSpin->setValue(fit.k);
        kcubeSpin->setValue(fit.kcube);
        if (fit.hasVignette) {
          vFalloff->setValue(fit.vignetteFalloff);
          vMult->setValue(fit.vignetteMultiplier);
        }
      };
      QObject::connect(presetCombo,
        QOverload<int>::of(&QComboBox::currentIndexChanged), &dialog,
        [applyPreset](int) { applyPreset(); });

      QObject::connect(&dialog, &QDialog::accepted, &dialog,
        [&, sensorName, presetCombo, imgWSpin, imgHSpin, fovSpin, kSpin, kcubeSpin,
         xSize, ySize, vFalloff, vMult, chromAb]() {
          CameraOptics o;
          o.preset = presetCombo->currentText();
          o.imgW = imgWSpin->value();
          o.imgH = imgHSpin->value();
          o.fov = fovSpin->value();
          o.lensK = kSpin->value();
          o.lensKcube = kcubeSpin->value();
          o.lensXSize = xSize->value();
          o.lensYSize = ySize->value();
          o.lensCircleFalloff = vFalloff->value();
          o.lensCircleMultiplier = vMult->value();
          o.chromaticAberration = chromAb->value();
          cameraOpticsByName[sensorName] = o;
        });
    } else {
      QLabel *placeholder = new QLabel(
        QString("<i>No additional configuration available for %1 yet.</i>")
          .arg(sensorName));
      placeholder->setTextFormat(Qt::RichText);
      placeholder->setStyleSheet("color: #888;");
      placeholder->setWordWrap(true);
      placeholder->setAlignment(Qt::AlignTop);
      configCol->addWidget(placeholder);
      configCol->addStretch(1);
    }

    QHBoxLayout *configuringRow = new QHBoxLayout();
    configuringRow->addStretch(1);
    QLabel *configuringLabel = new QLabel(QString("Configuring: %1").arg(sensorName));
    configuringLabel->setStyleSheet(
      "color: #777; font-style: italic; padding: 2px 4px;");
    configuringRow->addWidget(configuringLabel);
    dialogLayout->addLayout(configuringRow);

    dialogLayout->addWidget(buttons);
    dialog.setLayout(dialogLayout);
    dialog.exec();
  };

  auto wireCategory = [&](QString categoryName, std::vector<QCheckBox *> &checks, std::vector<QPushButton *> &gears,
                          std::vector<QPushButton *> &pluses, std::vector<QLabel *> &sensorCountLabels,
                          std::vector<int> &sensorCounts, QLabel *countLabel) {
    auto updateCategoryCount = [countLabel, &checks, &sensorCounts, categoryName]() {
      int totalCount = 0;
      for (size_t j = 0; j < checks.size(); ++j) {
        if (checks[j] && checks[j]->isChecked()) {
          totalCount += sensorCounts[j];
        }
      }
      countLabel->setText(QString("×%1").arg(totalCount));
    };

    for (size_t i = 0; i < checks.size(); ++i) {
      QCheckBox *check = checks[i];
      QPushButton *gear = gears[i];
      QPushButton *plus = pluses[i];
      QLabel *sensorCountLabel = sensorCountLabels[i];
      QObject::connect(check, &QCheckBox::toggled, &window, [&, i, gear, sensorCountLabel, updateCategoryCount](bool checked) {
        gear->setEnabled(checked);
        sensorCountLabel->setText(QString("x%1").arg(checked ? sensorCounts[i] : 0));
        const QString sn = checks[i]->property("sensorName").toString();
        sensorInstanceCount[sn] = checked ? sensorCounts[i] : 0;
        updateCategoryCount();
      });
      QObject::connect(plus, &QPushButton::clicked, &window, [&, i, check, sensorCountLabel, updateCategoryCount]() {
        sensorCounts[i] += 1;
        const QString sn = check->property("sensorName").toString();
        sensorMountConfigs[sensorMountKey(sn, sensorCounts[i])] = frontCenterMountForFrame("Rear Baselink");
        sensorInstanceCount[sn] = sensorCounts[i];
        if (!check->isChecked()) {
          check->setChecked(true);
        }
        sensorCountLabel->setText(QString("x%1").arg(sensorCounts[i]));
        updateCategoryCount();
      });
      QObject::connect(plus, &QPushButton::customContextMenuRequested, &window,
                        [&, i, check, sensorCountLabel, updateCategoryCount]() {
        if (sensorCounts[i] <= 0) return;
        sensorCounts[i] -= 1;
        const QString sn = check->property("sensorName").toString();
        sensorInstanceCount[sn] = sensorCounts[i];
        sensorCountLabel->setText(QString("x%1").arg(sensorCounts[i]));
        if (sensorCounts[i] == 0 && check->isChecked()) {
          check->setChecked(false);
        } else {
          updateCategoryCount();
        }
      });
    }
    updateCategoryCount();
  };

  wireCategory("Camera", cameraChecks, cameraGears, cameraPluses, cameraSensorCountLabels, cameraSensorCounts, cameraCountLabel);
  wireCategory("RADAR",  radarChecks,  radarGears,  radarPluses,  radarSensorCountLabels,  radarSensorCounts,  radarCountLabel);
  wireCategory("LiDAR",  lidarChecks,  lidarGears,  lidarPluses,  lidarSensorCountLabels,  lidarSensorCounts,  lidarCountLabel);
  wireCategory("NAV", navChecks, navGears, navPluses, navSensorCountLabels, navSensorCounts, navCountLabel);
  wireCategory("GT", gtChecks, gtGears, gtPluses, gtSensorCountLabels, gtSensorCounts, gtCountLabel);

  QDockWidget *previewDock = new QDockWidget("Sensor Preview Group", &window);
  previewDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
  previewDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
  QWidget *previewDockContent = new QWidget();
  QVBoxLayout *previewDockLayout = new QVBoxLayout(previewDockContent);
  previewDockLayout->setContentsMargins(4, 4, 4, 4);
  previewDockLayout->setSpacing(6);

  QScrollArea *previewScroll = new QScrollArea(previewDockContent);
  previewScroll->setWidgetResizable(true);
  previewScroll->setFrameShape(QFrame::NoFrame);
  previewScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QWidget *previewGridHost = new QWidget();
  QVBoxLayout *previewGridLayout = new QVBoxLayout(previewGridHost);
  previewGridLayout->setContentsMargins(0, 0, 0, 0);
  previewGridLayout->setSpacing(6);
  previewGridLayout->addStretch(1);
  previewScroll->setWidget(previewGridHost);

  previewDockLayout->addWidget(previewScroll, 1);

  QHBoxLayout *previewDockButtons = new QHBoxLayout();
  QPushButton *reattachPreviewBtn = new QPushButton("Re-attach All");
  previewDockButtons->addStretch();
  previewDockButtons->addWidget(reattachPreviewBtn);
  previewDockLayout->addLayout(previewDockButtons);
  previewDock->setWidget(previewDockContent);
  window.addDockWidget(Qt::RightDockWidgetArea, previewDock);
  previewDock->hide();

  
  {
    QDockWidget *vd = new QDockWidget("Camera Views", &window);
    vd->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    vd->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetClosable);
    QWidget *vdContent = new QWidget();
    QVBoxLayout *vdLayout = new QVBoxLayout(vdContent);
    vdLayout->setContentsMargins(4, 4, 4, 4);
    vdLayout->setSpacing(6);
    QScrollArea *vdScroll = new QScrollArea(vdContent);
    vdScroll->setWidgetResizable(true);
    vdScroll->setFrameShape(QFrame::NoFrame);
    vdScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QWidget *vdHost = new QWidget();
    viewGridLayout = new QVBoxLayout(vdHost);
    viewGridLayout->setContentsMargins(0, 0, 0, 0);
    viewGridLayout->setSpacing(6);
    viewGridLayout->addStretch(1);
    vdScroll->setWidget(vdHost);
    vdLayout->addWidget(vdScroll, 1);
    vd->setWidget(vdContent);
    window.addDockWidget(Qt::RightDockWidgetArea, vd);
    vd->hide();
    viewDock = vd;
  }

  QMap<QString, QGroupBox *> previewTiles;

  std::vector<QWidget *> detachedPreviewWindows;

  QMap<QString, QLabel *> previewImageSinks;
  QMap<QString, QPlainTextEdit *> previewTextSinks;

  struct SensorDescriptor {
    QString tabName;
    QString blueprintId;
    enum Kind { Camera, CameraDepth, CameraSemantic, CameraInstance, CameraOpticalFlow,
                LidarTopDown, RadarText, GnssText, ImuText,
                CollisionEvt, LaneInvasionEvt, ObstacleEvt, DvsText };
    Kind kind;
    int imgW = 800, imgH = 450;
    double fov = 90.0;
  };
  const QMap<QString, SensorDescriptor> kSensorCatalog = {
    {"RGB",            {"RGB",            "sensor.camera.rgb",                   SensorDescriptor::Camera}},
    {"Depth",          {"Depth",          "sensor.camera.depth",                 SensorDescriptor::CameraDepth}},
    {"Semantic Seg",   {"Semantic Seg",   "sensor.camera.semantic_segmentation", SensorDescriptor::CameraSemantic}},
    {"Instance Seg",   {"Instance Seg",   "sensor.camera.instance_segmentation", SensorDescriptor::CameraInstance}},
    {"DVS",            {"DVS",            "sensor.camera.dvs",                   SensorDescriptor::DvsText}},
    {"Optical Flow",   {"Optical Flow",   "sensor.camera.optical_flow",          SensorDescriptor::CameraOpticalFlow}},
    {"Fisheye",        {"Fisheye",        "sensor.camera.rgb",                   SensorDescriptor::Camera, 800, 450, 160.0}},
    {"Front",          {"Front",          "sensor.other.radar",                  SensorDescriptor::RadarText}},
    {"Rear",           {"Rear",           "sensor.other.radar",                  SensorDescriptor::RadarText}},
    {"Long-Range",     {"Long-Range",     "sensor.other.radar",                  SensorDescriptor::RadarText}},
    {"Ray Cast",       {"Ray Cast",       "sensor.lidar.ray_cast",               SensorDescriptor::LidarTopDown}},
    {"Semantic",       {"Semantic",       "sensor.lidar.ray_cast_semantic",      SensorDescriptor::LidarTopDown}},
    {"Roof 360",       {"Roof 360",       "sensor.lidar.ray_cast",               SensorDescriptor::LidarTopDown}},
    {"GNSS",           {"GNSS",           "sensor.other.gnss",                   SensorDescriptor::GnssText}},
    {"IMU",            {"IMU",            "sensor.other.imu",                    SensorDescriptor::ImuText}},
    {"Collision",      {"Collision",      "sensor.other.collision",              SensorDescriptor::CollisionEvt}},
    {"Lane Invasion",  {"Lane Invasion",  "sensor.other.lane_invasion",          SensorDescriptor::LaneInvasionEvt}},
    {"Obstacle",       {"Obstacle",       "sensor.other.obstacle",               SensorDescriptor::ObstacleEvt}},
  };

  auto sensorIsTextKind = [&](const SensorDescriptor &d) {
    return d.kind == SensorDescriptor::RadarText ||
           d.kind == SensorDescriptor::GnssText ||
           d.kind == SensorDescriptor::ImuText ||
           d.kind == SensorDescriptor::CollisionEvt ||
           d.kind == SensorDescriptor::LaneInvasionEvt ||
           d.kind == SensorDescriptor::ObstacleEvt ||
           d.kind == SensorDescriptor::DvsText;
  };

#ifdef CARLA_STUDIO_WITH_LIBCARLA
  std::shared_ptr<cc::Client> previewClient;
  carla::SharedPtr<cc::Actor> previewParent;
  QMap<QString, carla::SharedPtr<cc::Sensor>> activePreviewSensors;
  std::vector<carla::SharedPtr<cc::Actor>> appliedSensorActors;
#endif

  auto stopAllPreviews = [&]() {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    for (auto it = activePreviewSensors.begin(); it != activePreviewSensors.end(); ++it) {
      try {
        if (it.value()) {
          it.value()->Stop();
          it.value()->Destroy();
        }
      } catch (...) {
      }
    }
    activePreviewSensors.clear();
    previewParent = nullptr;
    previewClient.reset();
#else
    (void)0;
#endif
  };

#ifdef CARLA_STUDIO_WITH_LIBCARLA
  auto appendPreviewText = [&](const QString &name, const QString &line, bool replace) {
    QPlainTextEdit *sink = previewTextSinks.value(name, nullptr);
    if (!sink) return;
    QMetaObject::invokeMethod(sink, [sink, line, replace]() {
      if (!sink) return;
      if (replace) sink->setPlainText(line);
      else {
        sink->appendPlainText(line);
        if (sink->document()->blockCount() > 400) {
          QTextCursor c(sink->document());
          c.movePosition(QTextCursor::Start);
          c.select(QTextCursor::BlockUnderCursor);
          c.removeSelectedText();
          c.deleteChar();
        }
      }
    }, Qt::QueuedConnection);
  };

  auto postImageToSink = [&](const QString &name, QImage frame) {
    QLabel *sink = previewImageSinks.value(name, nullptr);
    if (!sink) return;
    QMetaObject::invokeMethod(sink, [sink, frame]() {
      if (!sink) return;
      sink->setPixmap(QPixmap::fromImage(frame).scaled(
        sink->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }, Qt::QueuedConnection);
  };

  std::function<void(int)> tryConnectForPreviews;
  tryConnectForPreviews = [&](int attemptsLeft) {
    if (previewClient) return;

    const QString host = targetHost->text().trimmed().isEmpty()
                           ? QStringLiteral("localhost")
                           : targetHost->text().trimmed();
    const int port = portSpin->value();

    try {
      auto client = std::make_shared<cc::Client>(host.toStdString(),
                                                  static_cast<uint16_t>(port));
      client->SetTimeout(std::chrono::seconds(3));
      auto world = client->GetWorld();
      auto actors = world.GetActors();
      auto vehicles = actors->Filter("vehicle.*");
      if (!vehicles || vehicles->empty()) {
        if (attemptsLeft > 0) {
          for (auto it = previewImageSinks.begin(); it != previewImageSinks.end(); ++it) {
            if (it.value())
              QMetaObject::invokeMethod(it.value(), [label = it.value(), n = it.key()]() {
                if (label) label->setText(QString("%1\nWaiting for hero vehicle…").arg(n));
              }, Qt::QueuedConnection);
          }
          for (auto it = previewTextSinks.begin(); it != previewTextSinks.end(); ++it) {
            if (it.value())
              QMetaObject::invokeMethod(it.value(), [sink = it.value()]() {
                if (sink) sink->setPlainText("Waiting for hero vehicle…");
              }, Qt::QueuedConnection);
          }
          QTimer::singleShot(1500, &window, [=]() { tryConnectForPreviews(attemptsLeft - 1); });
        }
        return;
      }
      previewClient = client;
      previewParent = vehicles->at(0u);
      return;
    } catch (const std::exception &) {
      if (attemptsLeft > 0) {
        for (auto it = previewImageSinks.begin(); it != previewImageSinks.end(); ++it) {
          if (it.value())
            QMetaObject::invokeMethod(it.value(),
              [label = it.value(), n = it.key(), a = attemptsLeft]() {
                if (label)
                  label->setText(QString("%1\nConnecting to CARLA… (%2 tries left)").arg(n).arg(a));
              }, Qt::QueuedConnection);
        }
        QTimer::singleShot(1500, &window, [=]() { tryConnectForPreviews(attemptsLeft - 1); });
      } else {
        for (auto it = previewImageSinks.begin(); it != previewImageSinks.end(); ++it) {
          if (it.value())
            QMetaObject::invokeMethod(it.value(),
              [label = it.value(), n = it.key()]() {
                if (label)
                  label->setText(QString("%1\nCARLA unreachable — is the sim up and port set correctly?").arg(n));
              }, Qt::QueuedConnection);
        }
      }
    }
  };
#endif

#ifdef CARLA_STUDIO_WITH_LIBCARLA
  auto spawnAndListen = [&](const SensorDescriptor &desc) {
    if (!previewClient || !previewParent) return;
    if (activePreviewSensors.contains(desc.tabName)) return;

    try {
      auto world = previewClient->GetWorld();
      auto blueprints = world.GetBlueprintLibrary();
      const auto *bp = blueprints ? blueprints->Find(desc.blueprintId.toStdString()) : nullptr;
      if (!bp) {
        if (QLabel *sink = previewImageSinks.value(desc.tabName, nullptr))
          sink->setText(QString("%1\nBlueprint %2 not available").arg(desc.tabName, desc.blueprintId));
        if (QPlainTextEdit *sink = previewTextSinks.value(desc.tabName, nullptr))
          sink->setPlainText(QString("Blueprint %1 not available").arg(desc.blueprintId));
        return;
      }
      auto bpCopy = *bp;

      auto trySet = [&](const char *k, const QString &v) {
        if (bpCopy.ContainsAttribute(k)) bpCopy.SetAttribute(k, v.toStdString());
      };
      switch (desc.kind) {
        case SensorDescriptor::Camera:
        case SensorDescriptor::CameraDepth:
        case SensorDescriptor::CameraSemantic:
        case SensorDescriptor::CameraInstance:
        case SensorDescriptor::CameraOpticalFlow:
        case SensorDescriptor::DvsText:
          trySet("image_size_x", QString::number(desc.imgW));
          trySet("image_size_y", QString::number(desc.imgH));
          trySet("fov",          QString::number(desc.fov));
          trySet("sensor_tick",  "0.10");
          break;
        case SensorDescriptor::LidarTopDown:
          trySet("range",             "50.0");
          trySet("channels",          "32");
          trySet("points_per_second", "200000");
          trySet("rotation_frequency","20");
          trySet("sensor_tick",       "0.10");
          break;
        case SensorDescriptor::RadarText:
          trySet("horizontal_fov", "35");
          trySet("vertical_fov",   "20");
          trySet("range",          "80.0");
          trySet("sensor_tick",    "0.20");
          break;
        case SensorDescriptor::GnssText:
        case SensorDescriptor::ImuText:
          trySet("sensor_tick", "0.25");
          break;
        case SensorDescriptor::CollisionEvt:
        case SensorDescriptor::LaneInvasionEvt:
        case SensorDescriptor::ObstacleEvt:
          break;
      }

      const auto attach = cg::Transform{
        cg::Location{-5.5f, 0.0f, 2.6f}, cg::Rotation{-12.0f, 0.0f, 0.0f}};
      auto actor = world.SpawnActor(bpCopy, attach, previewParent.get());
      auto sensor = std::static_pointer_cast<cc::Sensor>(actor);
      if (!sensor) {
        if (QLabel *sink = previewImageSinks.value(desc.tabName, nullptr))
          sink->setText(QString("%1\nFailed to spawn sensor").arg(desc.tabName));
        return;
      }
      activePreviewSensors.insert(desc.tabName, sensor);

      const QString tab = desc.tabName;
      const auto kind = desc.kind;
      sensor->Listen([this_ = &window, tab, kind, &postImageToSink, &appendPreviewText]
        (carla::SharedPtr<carla::sensor::SensorData> data) {
          Q_UNUSED(this_);
          try {
            switch (kind) {
              case SensorDescriptor::Camera:
              case SensorDescriptor::CameraInstance: {
                auto image = std::static_pointer_cast<csd::Image>(data);
                if (!image) return;
                const int w = int(image->GetWidth()), h = int(image->GetHeight());
                if (w <= 0 || h <= 0) return;
                QImage frame(w, h, QImage::Format_RGB888);
                for (int y = 0; y < h; ++y) {
                  uchar *L = frame.scanLine(y);
                  for (int x = 0; x < w; ++x) {
                    const auto &p = (*image)[size_t(y * w + x)];
                    L[x*3+0] = p.r; L[x*3+1] = p.g; L[x*3+2] = p.b;
                  }
                }
                postImageToSink(tab, frame);
                break;
              }
              case SensorDescriptor::CameraDepth: {
                auto image = std::static_pointer_cast<csd::Image>(data);
                if (!image) return;
                const int w = int(image->GetWidth()), h = int(image->GetHeight());
                if (w <= 0 || h <= 0) return;
                QImage frame(w, h, QImage::Format_RGB888);
                for (int y = 0; y < h; ++y) {
                  uchar *L = frame.scanLine(y);
                  for (int x = 0; x < w; ++x) {
                    const auto &p = (*image)[size_t(y * w + x)];
                    const double norm =
                      (p.r + p.g * 256.0 + p.b * 65536.0) / (256.0*256.0*256.0 - 1.0);
                    const double shade = std::pow(norm, 0.35);
                    const uchar g = uchar(std::min(255.0, shade * 255.0));
                    L[x*3+0] = g; L[x*3+1] = g; L[x*3+2] = g;
                  }
                }
                postImageToSink(tab, frame);
                break;
              }
              case SensorDescriptor::CameraSemantic: {
                auto image = std::static_pointer_cast<csd::Image>(data);
                if (!image) return;
                const int w = int(image->GetWidth()), h = int(image->GetHeight());
                if (w <= 0 || h <= 0) return;
                QImage frame(w, h, QImage::Format_RGB888);
                auto colorFor = [](uchar cls) {
                  const uint32_t h_ = uint32_t(cls) * 2654435761u;
                  return std::tuple<uchar,uchar,uchar>(uchar((h_>>16)&0xFF),
                                                       uchar((h_>>8)&0xFF),
                                                       uchar(h_&0xFF));
                };
                for (int y = 0; y < h; ++y) {
                  uchar *L = frame.scanLine(y);
                  for (int x = 0; x < w; ++x) {
                    const auto &p = (*image)[size_t(y * w + x)];
                    auto [r,g,b] = colorFor(p.r);
                    L[x*3+0] = r; L[x*3+1] = g; L[x*3+2] = b;
                  }
                }
                postImageToSink(tab, frame);
                break;
              }
              case SensorDescriptor::CameraOpticalFlow: {
                auto flow = std::static_pointer_cast<csd::OpticalFlowImage>(data);
                if (!flow) return;
                const int w = int(flow->GetWidth()), h = int(flow->GetHeight());
                if (w <= 0 || h <= 0) return;

                double maxMag = 1e-6;
                for (size_t i = 0; i < flow->size(); ++i) {
                  const auto &p = (*flow)[i];
                  const double m = std::sqrt(double(p.x) * p.x + double(p.y) * p.y);
                  if (m > maxMag) maxMag = m;
                }

                QImage frame(w, h, QImage::Format_RGB888);
                for (int y = 0; y < h; ++y) {
                  uchar *L = frame.scanLine(y);
                  for (int x = 0; x < w; ++x) {
                    const auto &p = (*flow)[size_t(y * w + x)];
                    const double mag = std::sqrt(double(p.x) * p.x + double(p.y) * p.y);
                    const double ang = std::atan2(double(p.y), double(p.x));
                    const double hue = (ang + M_PI) / (2.0 * M_PI);
                    const double sat = std::min(1.0, mag / maxMag);
                    const QColor c = QColor::fromHsvF(hue, sat, 1.0);
                    L[x*3+0] = c.red(); L[x*3+1] = c.green(); L[x*3+2] = c.blue();
                  }
                }
                postImageToSink(tab, frame);
                break;
              }
              case SensorDescriptor::LidarTopDown: {
                auto measurement =
                  std::static_pointer_cast<carla::sensor::data::LidarMeasurement>(data);
                if (!measurement) return;
                const int W = 360, H = 360;
                const double range = 50.0;
                QImage frame(W, H, QImage::Format_RGB888);
                frame.fill(QColor(18, 18, 24));
                for (const auto &pt : *measurement) {
                  const double px = pt.point.x, py = pt.point.y;
                  const int u = int(W/2 + (px / range) * (W/2));
                  const int v = int(H/2 - (py / range) * (H/2));
                  if (u < 0 || u >= W || v < 0 || v >= H) continue;
                  uchar *L = frame.scanLine(v);
                  const int idx = u * 3;
                  L[idx+0] = 0x6E; L[idx+1] = 0xE6; L[idx+2] = 0x6E;
                }
                postImageToSink(tab, frame);
                break;
              }
              case SensorDescriptor::RadarText: {
                auto meas =
                  std::static_pointer_cast<carla::sensor::data::RadarMeasurement>(data);
                if (!meas) return;
                int count = int(meas->size());
                double minDepth = std::numeric_limits<double>::infinity();
                double maxDepth = 0.0;
                for (const auto &det : *meas) {
                  minDepth = std::min<double>(minDepth, det.depth);
                  maxDepth = std::max<double>(maxDepth, det.depth);
                }
                if (!std::isfinite(minDepth)) minDepth = 0.0;
                appendPreviewText(tab,
                  QString("targets=%1  range=%2-%3 m")
                    .arg(count).arg(minDepth, 0, 'f', 1).arg(maxDepth, 0, 'f', 1),
                  true);
                break;
              }
              case SensorDescriptor::GnssText: {
                auto m = std::static_pointer_cast<carla::sensor::data::GnssMeasurement>(data);
                if (!m) return;
                appendPreviewText(tab,
                  QString("lat=%1  lon=%2  alt=%3 m")
                    .arg(m->GetLatitude(), 0, 'f', 6)
                    .arg(m->GetLongitude(), 0, 'f', 6)
                    .arg(m->GetAltitude(), 0, 'f', 2),
                  true);
                break;
              }
              case SensorDescriptor::ImuText: {
                auto m = std::static_pointer_cast<carla::sensor::data::IMUMeasurement>(data);
                if (!m) return;
                const auto a = m->GetAccelerometer();
                const auto g = m->GetGyroscope();
                appendPreviewText(tab,
                  QString("accel=(%1,%2,%3)  gyro=(%4,%5,%6)  compass=%7°")
                    .arg(a.x, 0, 'f', 2).arg(a.y, 0, 'f', 2).arg(a.z, 0, 'f', 2)
                    .arg(g.x, 0, 'f', 2).arg(g.y, 0, 'f', 2).arg(g.z, 0, 'f', 2)
                    .arg(m->GetCompass() * 180.0 / M_PI, 0, 'f', 1),
                  true);
                break;
              }
              case SensorDescriptor::CollisionEvt: {
                auto e = std::static_pointer_cast<csd::CollisionEvent>(data);
                if (!e) return;
                const auto imp = e->GetNormalImpulse();
                const auto other = e->GetOtherActor();
                const QString otherName = other
                  ? QString::fromStdString(other->GetTypeId())
                  : QStringLiteral("<unknown>");
                appendPreviewText(tab,
                  QString("[%1] ↯ %2  impulse=%3")
                    .arg(QTime::currentTime().toString("HH:mm:ss"))
                    .arg(otherName)
                    .arg(std::sqrt(double(imp.x)*imp.x + double(imp.y)*imp.y + double(imp.z)*imp.z), 0, 'f', 1),
                  false);
                break;
              }
              case SensorDescriptor::LaneInvasionEvt: {
                auto e = std::static_pointer_cast<csd::LaneInvasionEvent>(data);
                if (!e) return;
                const auto &markings = e->GetCrossedLaneMarkings();
                appendPreviewText(tab,
                  QString("[%1] lane invasion, %2 marking(s) crossed")
                    .arg(QTime::currentTime().toString("HH:mm:ss"))
                    .arg(int(markings.size())),
                  false);
                break;
              }
              case SensorDescriptor::ObstacleEvt: {
                auto e = std::static_pointer_cast<csd::ObstacleDetectionEvent>(data);
                if (!e) return;
                const auto other = e->GetOtherActor();
                const QString otherName = other
                  ? QString::fromStdString(other->GetTypeId())
                  : QStringLiteral("<unknown>");
                appendPreviewText(tab,
                  QString("[%1] %2 at %3 m")
                    .arg(QTime::currentTime().toString("HH:mm:ss"))
                    .arg(otherName)
                    .arg(e->GetDistance(), 0, 'f', 2),
                  false);
                break;
              }
              case SensorDescriptor::DvsText: {
                auto events = std::static_pointer_cast<csd::DVSEventArray>(data);
                if (!events) return;
                int pos = 0, neg = 0;
                for (const auto &e : *events) {
                  if (e.pol) ++pos; else ++neg;
                }
                appendPreviewText(tab,
                  QString("[%1] %2 events  (+%3 / -%4)")
                    .arg(QTime::currentTime().toString("HH:mm:ss.zzz"))
                    .arg(int(events->size())).arg(pos).arg(neg),
                  true);
                break;
              }
            }
          } catch (...) {
          }
        });
    } catch (const std::exception &e) {
      if (QLabel *sink = previewImageSinks.value(desc.tabName, nullptr))
        sink->setText(QString("%1\n%2").arg(desc.tabName, QString::fromLocal8Bit(e.what())));
      if (QPlainTextEdit *sink = previewTextSinks.value(desc.tabName, nullptr))
        sink->setPlainText(QString::fromLocal8Bit(e.what()));
    }
  };
#endif

  startSensorPreview = [&](const QString &sensorName) {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    const SensorDescriptor desc = kSensorCatalog.value(sensorName);
    if (desc.blueprintId.isEmpty()) return;

    if (!previewClient || !previewParent) {
      tryConnectForPreviews(10);
      QTimer::singleShot(2500, &window, [&, sensorName]() {
        if (previewClient && previewParent) {
          startSensorPreview(sensorName);
        }
      });
      return;
    }
    spawnAndListen(desc);
#else
    Q_UNUSED(sensorName);
#endif
  };

  auto createPreviewTile = [&](const QString &sensorName) -> QGroupBox * {
    QGroupBox *tile = new QGroupBox(sensorName);
    tile->setProperty("sensorName", sensorName);
    tile->setStyleSheet(
      "QGroupBox { font-weight: bold; border: 1px solid #3a3a3a; "
      "border-radius: 4px; margin-top: 8px; padding-top: 4px; }"
      "QGroupBox::title { subcontrol-origin: margin; left: 8px; "
      "padding: 0 4px 0 4px; }");

    QVBoxLayout *tileLayout = new QVBoxLayout(tile);
    tileLayout->setContentsMargins(4, 4, 4, 4);
    tileLayout->setSpacing(2);

    const SensorDescriptor desc = kSensorCatalog.value(sensorName);
    const bool isText = !desc.blueprintId.isEmpty() && sensorIsTextKind(desc);

    if (isText) {
      QPlainTextEdit *textView = new QPlainTextEdit(tile);
      textView->setReadOnly(true);
      textView->setPlaceholderText(QStringLiteral("waiting for data"));
      textView->setStyleSheet(
        "QPlainTextEdit { font-family: monospace; background: #111; "
        "color: #0F0; border: none; }");
      textView->setMinimumHeight(90);
      textView->setMaximumHeight(140);
      previewTextSinks[sensorName] = textView;
      tileLayout->addWidget(textView);
    } else {
      QLabel *previewLabel = new QLabel(QStringLiteral("Ready to stream"), tile);
      previewLabel->setAlignment(Qt::AlignCenter);
      previewLabel->setStyleSheet(
        "QLabel { border: 1px solid #006600; background: #111; color: #0F0; "
        "font-weight: bold; }");
      previewLabel->setMinimumSize(260, 130);
      previewLabel->setMaximumHeight(180);
      previewLabel->setScaledContents(false);
      previewImageSinks[sensorName] = previewLabel;
      tileLayout->addWidget(previewLabel);
    }

    QPushButton *detachBtn = new QPushButton("⇱ detach", tile);
    detachBtn->setMaximumWidth(80);
    detachBtn->setStyleSheet("QPushButton { font-size: 10px; padding: 1px; }");
    tileLayout->addWidget(detachBtn, 0, Qt::AlignRight);

    QObject::connect(detachBtn, &QPushButton::clicked, tile, [&, sensorName]() {
      QGroupBox *src = previewTiles.value(sensorName, nullptr);
      if (!src) return;
      previewGridLayout->removeWidget(src);
      src->setParent(nullptr);
      src->setWindowTitle(QString("CARLA Preview - %1").arg(sensorName));
      src->setWindowFlag(Qt::Window, true);
      src->resize(420, 260);
      detachedPreviewWindows.push_back(src);
      src->show();
    });

    return tile;
  };

  auto addPreviewTileByName = [&](const QString &sensorName) {
    if (previewTiles.contains(sensorName)) return;
    QGroupBox *tile = createPreviewTile(sensorName);
    previewTiles.insert(sensorName, tile);
    const int stretchIdx = previewGridLayout->count() - 1;
    previewGridLayout->insertWidget(stretchIdx, tile);
  };

  auto selectedAllSensors = [&]() -> QStringList {
    QStringList selected;
    auto collectChecked = [&](const std::vector<QCheckBox *> &checks) {
      for (QCheckBox *check : checks) {
        if (check && check->isChecked()) {
          selected << check->text();
        }
      }
    };
    collectChecked(cameraChecks);
    collectChecked(radarChecks);
    collectChecked(lidarChecks);
    collectChecked(navChecks);
    collectChecked(gtChecks);
    selected.removeDuplicates();
    return selected;
  };

  QObject::connect(previewSensorsBtn, &QPushButton::clicked, &window, [&]() {
    QStringList selectedSensors = selectedAllSensors();
    for (const QString &sensorName : selectedSensors) {
      addPreviewTileByName(sensorName);
    }
    if (!selectedSensors.isEmpty()) {
      previewDock->show();
      previewDock->raise();
    }

#ifdef CARLA_STUDIO_WITH_LIBCARLA
    tryConnectForPreviews(10);
    for (const QString &sensorName : selectedSensors) {
      startSensorPreview(sensorName);
    }
#else
    for (const QString &sensorName : selectedSensors) {
      if (QLabel *sink = previewImageSinks.value(sensorName, nullptr)) {
        sink->setText("LibCarla unavailable\nin this build");
      }
      if (QPlainTextEdit *sink = previewTextSinks.value(sensorName, nullptr)) {
        sink->setPlainText("LibCarla integration unavailable in this build");
      }
    }
#endif
  });

  QObject::connect(applySensorsBtn, &QPushButton::clicked, &window, [&]() {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    const QString host = targetHost->text().trimmed().isEmpty()
                          ? QStringLiteral("localhost")
                          : targetHost->text().trimmed();
    const int port = portSpin->value();
    std::shared_ptr<cc::Client> c;
    try {
      c = std::make_shared<cc::Client>(host.toStdString(), static_cast<uint16_t>(port));
      c->SetTimeout(std::chrono::seconds(8));
      (void)c->GetClientVersion();
    } catch (...) {
      QMessageBox::warning(&window, "CARLA unreachable",
        QString("Couldn't connect to %1:%2 — is the sim up?").arg(host).arg(port));
      return;
    }

    auto world = c->GetWorld();
    auto actorList = world.GetActors();
    auto vehicleList = actorList ? actorList->Filter("vehicle.*") : nullptr;
    carla::SharedPtr<cc::Actor> hero;
    if (vehicleList) {
      for (auto a : *vehicleList) {
        if (!a) continue;
        for (const auto &attr : a->GetAttributes()) {
          if (attr.GetId() == "role_name" && attr.GetValue() == "hero") { hero = a; break; }
        }
        if (hero) break;
      }
      if (!hero && !vehicleList->empty()) hero = (*vehicleList)[0];
    }
    if (!hero) {
      QMessageBox::warning(&window, "No hero vehicle",
        "Spawn a hero vehicle first (Home tab → START).");
      return;
    }

    for (auto &a : appliedSensorActors) {
      try { if (a) a->Destroy(); } catch (...) {}
    }
    appliedSensorActors.clear();

    auto bps = world.GetBlueprintLibrary();
    if (!bps) {
      QMessageBox::warning(&window, "No blueprints", "Blueprint library unavailable.");
      return;
    }

    int totalSpawned = 0, totalRequested = 0;

    auto applyCategory = [&](std::vector<QCheckBox *> &checks,
                              std::vector<int> &counts) {
      for (size_t i = 0; i < checks.size(); ++i) {
        if (!checks[i] || !checks[i]->isChecked()) continue;
        if (counts[i] <= 0) continue;
        const QString name = checks[i]->text();
        const auto descIt = kSensorCatalog.find(name);
        if (descIt == kSensorCatalog.end()) continue;
        const SensorDescriptor &desc = descIt.value();
        const auto *bp = bps->Find(desc.blueprintId.toStdString());
        if (!bp) continue;

        const SensorMountConfig mount = getOrCreateSensorMount(name);
        cg::Location loc(mount.tx, mount.ty, mount.tz);
        cg::Rotation rot(mount.ry, mount.rz, mount.rx);
        cg::Transform tr(loc, rot);

        for (int k = 0; k < counts[i]; ++k) {
          auto bpCopy = *bp;
          if (isCameraSensorName(name)) {
            const CameraOptics o = cameraOpticsByName.value(name, CameraOptics{});
            auto trySetAttr = [&](const char *attr, const QString &v) {
              if (bpCopy.ContainsAttribute(attr)) {
                bpCopy.SetAttribute(attr, v.toStdString());
              }
            };
            trySetAttr("image_size_x", QString::number(o.imgW));
            trySetAttr("image_size_y", QString::number(o.imgH));
            trySetAttr("fov", QString::number(o.fov, 'f', 4));
            trySetAttr("lens_k", QString::number(o.lensK, 'f', 6));
            trySetAttr("lens_kcube", QString::number(o.lensKcube, 'f', 6));
            trySetAttr("lens_x_size", QString::number(o.lensXSize, 'f', 6));
            trySetAttr("lens_y_size", QString::number(o.lensYSize, 'f', 6));
            trySetAttr("lens_circle_falloff", QString::number(o.lensCircleFalloff, 'f', 4));
            trySetAttr("lens_circle_multiplier", QString::number(o.lensCircleMultiplier, 'f', 4));
            trySetAttr("chromatic_aberration_intensity",
                        QString::number(o.chromaticAberration, 'f', 4));
          }
          ++totalRequested;
          auto actor = world.TrySpawnActor(bpCopy, tr, hero.get());
          if (actor) {
            appliedSensorActors.push_back(actor);
            ++totalSpawned;
          }
        }
      }
    };
    applyCategory(cameraChecks,    cameraSensorCounts);
    applyCategory(radarChecks,     radarSensorCounts);
    applyCategory(lidarChecks,     lidarSensorCounts);
    applyCategory(navChecks,       navSensorCounts);
    applyCategory(gtChecks,        gtSensorCounts);

    QMessageBox::information(&window, "Sensors applied",
      QString("Spawned %1 / %2 sensors attached to hero.").arg(totalSpawned).arg(totalRequested));
#else
    QMessageBox::warning(&window, "LibCarla not linked", "This build cannot reach CARLA.");
#endif
  });

  auto walkSensors = [&](std::function<void(const QString &, bool, int)> cb) {
    auto walk = [&](std::vector<QCheckBox *> &checks, std::vector<int> &counts) {
      for (size_t i = 0; i < checks.size(); ++i) {
        if (!checks[i]) continue;
        cb(checks[i]->text(), checks[i]->isChecked(), counts[i]);
      }
    };
    walk(cameraChecks, cameraSensorCounts);
    walk(radarChecks,  radarSensorCounts);
    walk(lidarChecks,  lidarSensorCounts);
    walk(navChecks,    navSensorCounts);
    walk(gtChecks,     gtSensorCounts);
  };

  QObject::connect(saveLayoutBtn, &QPushButton::clicked, &window, [&, walkSensors]() {
    const QString dir = QDir::homePath() + "/.carla-studio/sensor_layouts";
    QDir().mkpath(dir);
    const QString path = QFileDialog::getSaveFileName(&window, "Save sensor layout",
      dir + "/layout.json", "Sensor JSON (*.json)");
    if (path.isEmpty()) return;
    QJsonObject root;
    root["version"] = 1;
    QJsonArray sensors;
    walkSensors([&](const QString &name, bool enabled, int count) {
      QJsonObject e;
      e["name"] = name;
      e["enabled"] = enabled;
      e["count"] = count;
      const SensorMountConfig m = getOrCreateSensorMount(name);
      QJsonObject mo;
      mo["frame"] = m.framePreset;
      mo["x"] = m.tx; mo["y"] = m.ty; mo["z"] = m.tz;
      mo["roll"] = m.rx; mo["pitch"] = m.ry; mo["yaw"] = m.rz;
      e["mount"] = mo;
      if (isCameraSensorName(name)) {
        const CameraOptics o = cameraOpticsByName.value(name, CameraOptics{});
        QJsonObject oo;
        oo["preset"] = o.preset;
        oo["image_size_x"] = o.imgW;
        oo["image_size_y"] = o.imgH;
        oo["fov"] = o.fov;
        oo["lens_k"] = o.lensK;
        oo["lens_kcube"] = o.lensKcube;
        oo["lens_x_size"] = o.lensXSize;
        oo["lens_y_size"] = o.lensYSize;
        oo["lens_circle_falloff"] = o.lensCircleFalloff;
        oo["lens_circle_multiplier"] = o.lensCircleMultiplier;
        oo["chromatic_aberration_intensity"] = o.chromaticAberration;
        e["optics"] = oo;
      }
      sensors.append(e);
    });
    root["sensors"] = sensors;
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
      QMessageBox::warning(&window, "Save failed", "Cannot write " + path);
      return;
    }
    f.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    QMessageBox::information(&window, "Saved", "Layout saved to:\n" + path);
  });

  QObject::connect(loadLayoutBtn, &QPushButton::clicked, &window, [&]() {
    const QString dir = QDir::homePath() + "/.carla-studio/sensor_layouts";
    const QString path = QFileDialog::getOpenFileName(&window, "Load sensor layout",
      dir, "Sensor JSON (*.json)");
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
      QMessageBox::warning(&window, "Load failed", "Cannot read " + path);
      return;
    }
    QJsonParseError err;
    const auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
      QMessageBox::warning(&window, "Load failed", "Invalid JSON: " + err.errorString());
      return;
    }
    const auto sensors = doc.object().value("sensors").toArray();
    QMap<QString, QJsonObject> byName;
    for (const auto &v : sensors) byName.insert(v.toObject().value("name").toString(), v.toObject());
    auto restore = [&](std::vector<QCheckBox *> &checks,
                        std::vector<int> &counts,
                        std::vector<QLabel *> &countLabels) {
      for (size_t i = 0; i < checks.size(); ++i) {
        if (!checks[i]) continue;
        const QJsonObject e = byName.value(checks[i]->text());
        if (e.isEmpty()) continue;
        checks[i]->setChecked(e.value("enabled").toBool());
        counts[i] = e.value("count").toInt(0);
        if (countLabels[i]) countLabels[i]->setText(QString("x%1").arg(counts[i]));
        const auto mo = e.value("mount").toObject();
        if (!mo.isEmpty()) {
          SensorMountConfig m;
          m.framePreset = mo.value("frame").toString("Rear Baselink");
          m.tx = mo.value("x").toDouble();
          m.ty = mo.value("y").toDouble();
          m.tz = mo.value("z").toDouble();
          m.rx = mo.value("roll").toDouble();
          m.ry = mo.value("pitch").toDouble();
          m.rz = mo.value("yaw").toDouble();
          sensorMountConfigs[checks[i]->text()] = m;
        }
        const auto oo = e.value("optics").toObject();
        if (!oo.isEmpty()) {
          CameraOptics o;
          o.preset = oo.value("preset").toString("(Default rectilinear)");
          o.imgW = oo.value("image_size_x").toInt(800);
          o.imgH = oo.value("image_size_y").toInt(450);
          o.fov = oo.value("fov").toDouble(90.0);
          o.lensK = oo.value("lens_k").toDouble();
          o.lensKcube = oo.value("lens_kcube").toDouble();
          o.lensXSize = oo.value("lens_x_size").toDouble(0.08);
          o.lensYSize = oo.value("lens_y_size").toDouble(0.08);
          o.lensCircleFalloff = oo.value("lens_circle_falloff").toDouble(5.0);
          o.lensCircleMultiplier = oo.value("lens_circle_multiplier").toDouble(1.0);
          o.chromaticAberration = oo.value("chromatic_aberration_intensity").toDouble();
          cameraOpticsByName[checks[i]->text()] = o;
        }
      }
    };
    restore(cameraChecks, cameraSensorCounts, cameraSensorCountLabels);
    restore(radarChecks,  radarSensorCounts,  radarSensorCountLabels);
    restore(lidarChecks,  lidarSensorCounts,  lidarSensorCountLabels);
    restore(navChecks,    navSensorCounts,    navSensorCountLabels);
    restore(gtChecks,     gtSensorCounts,     gtSensorCountLabels);
    saveSensorMountConfigs();
    QMessageBox::information(&window, "Loaded", "Layout loaded from:\n" + path);
  });

  QObject::connect(exportYamlBtn, &QPushButton::clicked, &window, [&, walkSensors]() {
    const QString path = QFileDialog::getSaveFileName(&window,
      "Export sensors as Scenario Runner YAML",
      QDir::homePath() + "/sensors.yaml", "YAML (*.yaml *.yml)");
    if (path.isEmpty()) return;
    QString out;
    out += "# Generated by CARLA Studio — sensors block for Scenario Runner.\n";
    out += "sensors:\n";
    int instanceCounter = 0;
    walkSensors([&](const QString &name, bool enabled, int count) {
      if (!enabled || count <= 0) return;
      const auto descIt = kSensorCatalog.find(name);
      if (descIt == kSensorCatalog.end()) return;
      const SensorDescriptor &desc = descIt.value();
      const SensorMountConfig m = getOrCreateSensorMount(name);
      const QString slug = QString(name).replace(' ', '_').replace('-', '_').toLower();
      for (int k = 0; k < count; ++k) {
        out += QString("  - id: %1_%2\n").arg(slug).arg(instanceCounter++);
        out += QString("    type: %1\n").arg(desc.blueprintId);
        out += QString("    transform: { x: %1, y: %2, z: %3, roll: %4, pitch: %5, yaw: %6 }\n")
                 .arg(m.tx).arg(m.ty).arg(m.tz).arg(m.rx).arg(m.ry).arg(m.rz);
        if (isCameraSensorName(name)) {
          const CameraOptics o = cameraOpticsByName.value(name, CameraOptics{});
          out += QString("    image_size_x: %1\n").arg(o.imgW);
          out += QString("    image_size_y: %1\n").arg(o.imgH);
          out += QString("    fov: %1\n").arg(o.fov);
          out += QString("    lens_k: %1\n").arg(o.lensK);
          out += QString("    lens_kcube: %1\n").arg(o.lensKcube);
        }
      }
    });
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
      QMessageBox::warning(&window, "Export failed", "Cannot write " + path);
      return;
    }
    f.write(out.toUtf8());
    QMessageBox::information(&window, "Exported", "YAML saved to:\n" + path);
  });

  QObject::connect(reattachPreviewBtn, &QPushButton::clicked, &window, [&]() {
    for (QWidget *detachedWindow : detachedPreviewWindows) {
      if (!detachedWindow) continue;
      QGroupBox *tile = qobject_cast<QGroupBox *>(detachedWindow);
      if (!tile) continue;
      const QString name = tile->property("sensorName").toString();
      if (name.isEmpty()) continue;
      tile->setWindowFlag(Qt::Window, false);
      tile->setParent(previewGridHost);
      const int stretchIdx = previewGridLayout->count() - 1;
      previewGridLayout->insertWidget(stretchIdx, tile);
      tile->show();
    }
    detachedPreviewWindows.clear();
    if (!previewTiles.isEmpty()) previewDock->show();
  });

  bool exitCleanupRan = false;
  auto cleanupStudioLaunchedProcesses = [&]() {
    if (exitCleanupRan) {
      return;
    }
    exitCleanupRan = true;

    setSimulationStatus("Shutdown cleanup in progress...");
    app.processEvents();

    stopInAppDriver();
    stopAllPreviews();

    const QList<qint64> launchedPids = trackedCarlaPids;
    for (qint64 pid : launchedPids) {
      if (pid > 0) {
        QProcess::execute("/bin/bash", QStringList() << "-lc" << QString("kill -TERM %1 2>/dev/null || true").arg(pid));
      }
    }

    QProcess::execute("/bin/bash", QStringList() << "-lc" << "sleep 1");

    for (qint64 pid : launchedPids) {
      if (pid > 0) {
        QProcess::execute("/bin/bash", QStringList() << "-lc" << QString("kill -0 %1 2>/dev/null && kill -KILL %1 2>/dev/null || true").arg(pid));
      }
    }

    trackedCarlaPids.clear();
    refreshRememberedPidList();
  };

  QTimer *signalCleanupTimer = new QTimer(&window);
  signalCleanupTimer->setInterval(100);
  QObject::connect(signalCleanupTimer, &QTimer::timeout, &window, [&]() {
    if (g_shutdownSignalRequested == 0) {
      return;
    }
    cleanupStudioLaunchedProcesses();
    app.quit();
  });
  signalCleanupTimer->start();

  QObject::connect(&app, &QApplication::aboutToQuit, &window, [&]() {
    cleanupStudioLaunchedProcesses();
  });
  
  QWidget *w3 = new QWidget();
  QVBoxLayout *w3Outer = new QVBoxLayout(w3);
  w3Outer->setContentsMargins(0, 0, 0, 0);
  QScrollArea *w3Scroll = new QScrollArea();
  w3Scroll->setWidgetResizable(true);
  w3Scroll->setFrameShape(QFrame::NoFrame);
  QWidget *w3Inner = new QWidget();
  w3Scroll->setWidget(w3Inner);
  w3Outer->addWidget(w3Scroll);
  QVBoxLayout *l3 = new QVBoxLayout(w3Inner);
  l3->setContentsMargins(10, 10, 10, 10);
  l3->setSpacing(8);

  QLabel *scenarioIntro = new QLabel(
    "Build, save and run CARLA scenarios. Buttons prefixed with <b>⚡</b> "
    "need a reachable CARLA server (Endpoint on the Home tab).");
  scenarioIntro->setWordWrap(true);
  scenarioIntro->setTextFormat(Qt::RichText);
  l3->addWidget(scenarioIntro);

  QPlainTextEdit *scenarioLog = new QPlainTextEdit();
  scenarioLog->setReadOnly(true);
  scenarioLog->setMaximumBlockCount(500);
  scenarioLog->setPlaceholderText("Scenario builder log will appear here.");
  {
    const QFontMetrics fm(scenarioLog->font());
    scenarioLog->setMaximumHeight(fm.lineSpacing() * 2 + 14);
  }

  auto scenarioLogLine = [scenarioLog](const QString &s) {
    scenarioLog->appendPlainText(s);
  };

#ifdef CARLA_STUDIO_WITH_LIBCARLA
  auto scenarioConnectClient = [&]() -> std::shared_ptr<cc::Client> {
    const QString host = targetHost->text().trimmed().isEmpty()
                           ? QStringLiteral("localhost")
                           : targetHost->text().trimmed();
    const int port = portSpin->value();
    try {
      auto c = std::make_shared<cc::Client>(host.toStdString(),
                                             static_cast<uint16_t>(port));
      c->SetTimeout(std::chrono::seconds(5));
      (void)c->GetClientVersion();
      return c;
    } catch (const std::exception &e) {
      scenarioLogLine(QString("✗ CARLA unreachable at %1:%2 — %3")
                        .arg(host).arg(port).arg(e.what()));
    } catch (...) {
      scenarioLogLine(QString("✗ CARLA unreachable at %1:%2").arg(host).arg(port));
    }
    return nullptr;
  };
#endif

  QGroupBox *mapGroup = new QGroupBox("Map Browser");
  QVBoxLayout *mapLayout = new QVBoxLayout();
  QHBoxLayout *mapRow1 = new QHBoxLayout();
  QComboBox *mapCombo = new QComboBox();
  mapCombo->addItem("(press Refresh to query sim)");
  mapCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  mapCombo->setMinimumContentsLength(20);
  QPushButton *mapRefreshBtn = new QPushButton("⚡ ⟳ Refresh");
  simRequiredButtons.push_back(mapRefreshBtn);
  mapRefreshBtn->setToolTip("Query the running CARLA for installed maps.");
  mapRow1->addWidget(mapCombo, 1);
  mapRow1->addWidget(mapRefreshBtn);
  mapLayout->addLayout(mapRow1);

  QHBoxLayout *mapRow2 = new QHBoxLayout();
  QPushButton *mapLoadBtn = new QPushButton("⚡ Load");
  simRequiredButtons.push_back(mapLoadBtn);
  mapLoadBtn->setToolTip("Load the selected map into the running CARLA.");
  QPushButton *mapReloadBtn = new QPushButton("⚡ Reload");
  simRequiredButtons.push_back(mapReloadBtn);
  mapReloadBtn->setToolTip("Reload the current world (drops all actors).");
  QPushButton *mapInstallBtn = new QPushButton("Install more…");
  mapInstallBtn->setToolTip("Open the AdditionalMaps installer dialog.");
  mapRow2->addWidget(mapLoadBtn);
  mapRow2->addWidget(mapReloadBtn);
  mapRow2->addWidget(mapInstallBtn);
  mapLayout->addLayout(mapRow2);
  mapGroup->setLayout(mapLayout);

  QGroupBox *xodrGroup = new QGroupBox("OpenDRIVE Tools (XODR + OSM)");
  QVBoxLayout *xodrLayout = new QVBoxLayout();
  QHBoxLayout *xodrPathRow = new QHBoxLayout();
  xodrPathRow->addWidget(new QLabel("File:"));
  QLineEdit *xodrPath = new QLineEdit();
  xodrPath->setPlaceholderText("/path/to/map.xodr");
  QPushButton *xodrBrowseBtn = new QPushButton("Browse…");
  xodrPathRow->addWidget(xodrPath, 1);
  xodrPathRow->addWidget(xodrBrowseBtn);
  xodrLayout->addLayout(xodrPathRow);

  QHBoxLayout *xodrBtnRow = new QHBoxLayout();
  QPushButton *xodrValidateBtn = new QPushButton("Validate");
  xodrValidateBtn->setToolTip("Offline XML validation of the OpenDRIVE file.");
  QPushButton *xodrImportBtn = new QPushButton("⚡ Import");
  simRequiredButtons.push_back(xodrImportBtn);
  xodrImportBtn->setToolTip("Load this XODR into the running CARLA as a "
                              "standalone-OpenDRIVE world.");
  QPushButton *xodrExportBtn = new QPushButton("⚡ Export");
  simRequiredButtons.push_back(xodrExportBtn);
  xodrExportBtn->setToolTip("Save the running world's XODR to disk.");
  xodrBtnRow->addWidget(xodrValidateBtn);
  xodrBtnRow->addWidget(xodrImportBtn);
  xodrBtnRow->addWidget(xodrExportBtn);
  xodrLayout->addLayout(xodrBtnRow);

  QHBoxLayout *osmRow = new QHBoxLayout();
  osmRow->addWidget(new QLabel("OSM →"));
  QLineEdit *osmPath = new QLineEdit();
  osmPath->setPlaceholderText("/path/to/area.osm  (Convert via Python carla.Osm2Odr)");
  QPushButton *osmBrowseBtn  = new QPushButton("Browse…");
  QPushButton *osmConvertBtn = new QPushButton("Convert → XODR");
  osmConvertBtn->setToolTip(
    "Run Python carla.Osm2Odr.convert() on the selected OSM file.\n"
    "Requires the carla module to be importable from python3 — set\n"
    "PYTHONPATH to your CARLA PythonAPI dist/ directory if it's not\n"
    "system-installed. Health Check's PythonAPI row covers this.");
  osmRow->addWidget(osmPath, 1);
  osmRow->addWidget(osmBrowseBtn);
  osmRow->addWidget(osmConvertBtn);
  xodrLayout->addLayout(osmRow);

  xodrGroup->setLayout(xodrLayout);

  QGroupBox *cfgGroup = new QGroupBox("Scenario Config");
  {
    QFormLayout *cfgForm = new QFormLayout();
    cfgForm->addRow("Vehicles to spawn:",   cfgVehicles);
    cfgForm->addRow("Pedestrians to spawn:", cfgPedestrians);
    cfgForm->addRow("Random seed:",         cfgSeed);
    cfgGroup->setLayout(cfgForm);
  }

  QGroupBox *fileGroup = new QGroupBox("Scenario File (save / load / apply)");
  QVBoxLayout *fileLayout = new QVBoxLayout();
  QHBoxLayout *nameRow = new QHBoxLayout();
  nameRow->addWidget(new QLabel("Name:"));
  QLineEdit *scenarioName = new QLineEdit();
  scenarioName->setPlaceholderText("my_scenario");
  nameRow->addWidget(scenarioName, 1);
  fileLayout->addLayout(nameRow);
  QHBoxLayout *fileBtnRow = new QHBoxLayout();
  QPushButton *scenSaveBtn = new QPushButton("Save JSON");
  QPushButton *scenLoadBtn = new QPushButton("Load JSON");
  QPushButton *scenApplyBtn = new QPushButton("⚡ Apply");
  simRequiredButtons.push_back(scenApplyBtn);
  scenApplyBtn->setToolTip("Apply weather + spawn the actor counts above to "
                            "the running CARLA.");
  fileBtnRow->addWidget(scenSaveBtn);
  fileBtnRow->addWidget(scenLoadBtn);
  fileBtnRow->addWidget(scenApplyBtn);
  fileLayout->addLayout(fileBtnRow);
  fileGroup->setLayout(fileLayout);

  QGroupBox *examplesGroup = new QGroupBox("Quickstart Examples");
  QVBoxLayout *examplesLayout = new QVBoxLayout();
  QLabel *examplesIntro = new QLabel(
    "One-click demos. The XODR examples ship inside the app and load via "
    "the Standalone-OpenDRIVE pipeline (no Unreal rebuild). The scenario "
    "example fills in the form fields above with a ready-to-Apply preset.");
  examplesIntro->setWordWrap(true);
  examplesIntro->setStyleSheet("color: #666; font-size: 11px;");
  examplesLayout->addWidget(examplesIntro);

  QHBoxLayout *examplesBtnRow = new QHBoxLayout();
  QPushButton *exFlatBtn   = new QPushButton("⚡ Flat XODR");
  simRequiredButtons.push_back(exFlatBtn);
  QPushButton *exRuralBtn  = new QPushButton("⚡ OSM rural");
  simRequiredButtons.push_back(exRuralBtn);
  QPushButton *exTown01Btn = new QPushButton("Town01 preset");
  exFlatBtn->setToolTip(
    "Load a 1 km straight OpenDRIVE road as a standalone world.");
  exRuralBtn->setToolTip(
    "Load a hand-crafted rural T-junction OpenDRIVE — emulates the "
    "carla.Osm2Odr.convert() pipeline that issue #9565 fixed for 0.9.16.");
  exTown01Btn->setToolTip(
    "Populate the Scenario File form with a ClearNoon Town01 + 30 vehicle / "
    "50 pedestrian preset. Click 'Apply to sim' afterwards to run it.");
  examplesBtnRow->addWidget(exFlatBtn);
  examplesBtnRow->addWidget(exRuralBtn);
  examplesBtnRow->addWidget(exTown01Btn);
  examplesLayout->addLayout(examplesBtnRow);
  examplesGroup->setLayout(examplesLayout);

  auto materializeBundledExample = [&, scenarioLogLine](const QString &resourcePath,
                                                         const QString &outName) -> QString {
    QFile in(resourcePath);
    if (!in.open(QIODevice::ReadOnly)) {
      scenarioLogLine(QString("✗ bundled resource missing: %1").arg(resourcePath));
      return {};
    }
    const QString dir = QDir::tempPath() + "/carla-studio-examples";
    QDir().mkpath(dir);
    const QString outPath = dir + "/" + outName;
    QFile out(outPath);
    if (!out.open(QIODevice::WriteOnly)) {
      scenarioLogLine("✗ cannot write " + outPath);
      return {};
    }
    out.write(in.readAll());
    return outPath;
  };

  auto loadBundledXodr = [&, materializeBundledExample, scenarioLogLine]
      (const QString &resourcePath, const QString &outName, const QString &friendly) {
    const QString diskPath = materializeBundledExample(resourcePath, outName);
    if (diskPath.isEmpty()) return;
    xodrPath->setText(diskPath);
    scenarioLogLine(QString("→ loaded bundled %1 → %2").arg(friendly, diskPath));
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    QFile f(diskPath);
    if (!f.open(QIODevice::ReadOnly)) {
      scenarioLogLine("✗ re-read failed: " + diskPath); return;
    }
    const QByteArray xml = f.readAll();
    auto c = scenarioConnectClient();
    if (!c) return;
    try {
      carla::rpc::OpendriveGenerationParameters params;
      c->GenerateOpenDriveWorld(xml.toStdString(), params);
      scenarioLogLine(QString("✓ %1 imported (%2 bytes)").arg(friendly).arg(xml.size()));
    } catch (const std::exception &e) {
      scenarioLogLine(QString("✗ GenerateOpenDriveWorld failed: %1").arg(e.what()));
    }
#endif
  };

  QObject::connect(exFlatBtn, &QPushButton::clicked, &window, [&, loadBundledXodr]() {
    loadBundledXodr(":/examples/empty_flat.xodr", "empty_flat.xodr", "empty flat XODR");
  });

  QObject::connect(exRuralBtn, &QPushButton::clicked, &window, [&, loadBundledXodr]() {
    loadBundledXodr(":/examples/sample_osm_rural.xodr", "sample_osm_rural.xodr",
                    "OSM rural sample");
  });

  QObject::connect(exTown01Btn, &QPushButton::clicked, &window,
                    [&, scenarioLogLine]() {
    QFile in(":/examples/scenario_town01_demo.json");
    if (!in.open(QIODevice::ReadOnly)) {
      scenarioLogLine("✗ bundled scenario JSON missing");
      return;
    }
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(in.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
      scenarioLogLine("✗ bundled scenario JSON invalid: " + err.errorString());
      return;
    }
    const auto o = doc.object();
    scenarioName->setText(o.value("name").toString());
    const QString mapName = o.value("map").toString();
    if (!mapName.isEmpty()) {
      if (mapCombo->findText(mapName) < 0) mapCombo->insertItem(0, mapName);
      mapCombo->setCurrentText(mapName);
    }
    {
      const QString canonical = o.value("weather").toString("Default");
      const int idx = weatherCombo->findData(canonical);
      if (idx >= 0) weatherCombo->setCurrentIndex(idx);
    }
    cfgVehicles->setValue(o.value("vehicles").toInt(30));
    cfgPedestrians->setValue(o.value("pedestrians").toInt(50));
    cfgSeed->setValue(o.value("seed").toInt(42));
    scenarioLogLine("✓ Town01 demo populated — click 'Apply to sim' to run it");
  });

  QGroupBox *previewGroup = new QGroupBox("2D Map Preview");
  QVBoxLayout *previewLayout = new QVBoxLayout();
  QHBoxLayout *previewBtnRow = new QHBoxLayout();
  QPushButton *previewRefreshBtn = new QPushButton("⚡ ⟳ Render");
  simRequiredButtons.push_back(previewRefreshBtn);
  previewRefreshBtn->setToolTip("Fetch topology + spawn points from the "
                                  "running CARLA and render them.");
  QPushButton *previewPopoutBtn = new QPushButton("⇱ Pop out");
  previewPopoutBtn->setToolTip("Open the same scene in a resizable detached "
                                "window. The docked preview keeps updating "
                                "in sync (same QGraphicsScene).");
  QLabel *previewLegend = new QLabel(
    "<span style='color:#888'>● topology</span> &nbsp; "
    "<span style='color:#2FA66B'>● spawn points</span>");
  previewLegend->setTextFormat(Qt::RichText);
  previewBtnRow->addWidget(previewRefreshBtn);
  previewBtnRow->addWidget(previewPopoutBtn);
  previewBtnRow->addWidget(previewLegend, 1);
  previewLayout->addLayout(previewBtnRow);

  QGraphicsScene *previewScene = new QGraphicsScene(&window);
  QGraphicsView *previewView = new QGraphicsView(previewScene);
  previewView->setMaximumHeight(140);
  previewView->setRenderHint(QPainter::Antialiasing);
  previewView->setMinimumHeight(110);
  previewView->setBackgroundBrush(QColor(245, 245, 247));
  previewView->setDragMode(QGraphicsView::ScrollHandDrag);
  previewLayout->addWidget(previewView);

  auto previewPopoutHolder = std::make_shared<QPointer<QDialog>>(nullptr);
  QObject::connect(previewPopoutBtn, &QPushButton::clicked, &window,
                    [&, previewPopoutHolder]() {
    if (!*previewPopoutHolder) {
      auto *dlg = new QDialog(&window);
      dlg->setAttribute(Qt::WA_DeleteOnClose);
      dlg->setWindowTitle("CARLA Studio · Map Preview (popout)");
      dlg->resize(720, 600);
      auto *l = new QVBoxLayout(dlg);
      l->setContentsMargins(6, 6, 6, 6);
      auto *popoutView = new QGraphicsView(previewScene);
      popoutView->setRenderHint(QPainter::Antialiasing);
      popoutView->setBackgroundBrush(QColor(245, 245, 247));
      popoutView->setDragMode(QGraphicsView::ScrollHandDrag);
      l->addWidget(popoutView);
      const QRectF r = previewScene->sceneRect();
      if (!r.isEmpty()) popoutView->fitInView(r, Qt::KeepAspectRatio);
      *previewPopoutHolder = dlg;
    }
    (*previewPopoutHolder)->show();
    (*previewPopoutHolder)->raise();
    (*previewPopoutHolder)->activateWindow();
  });
  previewGroup->setLayout(previewLayout);

  l3->addWidget(examplesGroup);
  l3->addWidget(mapGroup);
  l3->addWidget(xodrGroup);
  l3->addWidget(previewGroup);
  l3->addWidget(cfgGroup);
  l3->addWidget(fileGroup);

  l3->addWidget(scenarioLog);
  l3->addStretch();

  QObject::connect(mapRefreshBtn, &QPushButton::clicked, &window, [&, scenarioLogLine]() {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    auto c = scenarioConnectClient();
    if (!c) return;
    try {
      auto maps = c->GetAvailableMaps();
      mapCombo->clear();
      for (const auto &m : maps) mapCombo->addItem(QString::fromStdString(m));
      scenarioLogLine(QString("✓ %1 maps available from sim").arg(maps.size()));
    } catch (const std::exception &e) {
      scenarioLogLine(QString("✗ GetAvailableMaps failed: %1").arg(e.what()));
    }
#else
    scenarioLogLine("✗ LibCarla not linked in this build");
#endif
  });

  QObject::connect(mapLoadBtn, &QPushButton::clicked, &window, [&, scenarioLogLine]() {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    const QString m = mapCombo->currentText();
    if (m.isEmpty() || m.startsWith("(")) {
      scenarioLogLine("✗ select a map first (press Refresh)");
      return;
    }
    auto c = scenarioConnectClient();
    if (!c) return;
    try {
      c->LoadWorld(m.toStdString());
      scenarioLogLine(QString("✓ loaded %1").arg(m));
    } catch (const std::exception &e) {
      scenarioLogLine(QString("✗ LoadWorld failed: %1").arg(e.what()));
    }
#endif
  });

  QObject::connect(mapReloadBtn, &QPushButton::clicked, &window, [&, scenarioLogLine]() {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    auto c = scenarioConnectClient();
    if (!c) return;
    try {
      c->ReloadWorld();
      scenarioLogLine("✓ world reloaded");
    } catch (const std::exception &e) {
      scenarioLogLine(QString("✗ ReloadWorld failed: %1").arg(e.what()));
    }
#endif
  });

  QObject::connect(mapInstallBtn, &QPushButton::clicked, &window, [&]() {
    openInstallerDialog(InstallerMode::InstallAdditionalMaps);
  });

  QObject::connect(xodrBrowseBtn, &QPushButton::clicked, &window, [&]() {
    const QString f = QFileDialog::getOpenFileName(&window,
      "Select OpenDRIVE file", QDir::homePath(),
      "OpenDRIVE (*.xodr *.xml);;All files (*)");
    if (!f.isEmpty()) xodrPath->setText(f);
  });

  QObject::connect(xodrValidateBtn, &QPushButton::clicked, &window, [&, scenarioLogLine]() {
    const QString path = xodrPath->text().trimmed();
    if (path.isEmpty()) { scenarioLogLine("✗ no XODR file selected"); return; }
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
      scenarioLogLine("✗ cannot read " + path); return;
    }
    QXmlStreamReader xml(&f);
    int roads = 0, junctions = 0;
    bool sawOpenDrive = false;
    while (!xml.atEnd()) {
      xml.readNext();
      if (!xml.isStartElement()) continue;
      const auto n = xml.name();
      if (n == QStringLiteral("OpenDRIVE")) sawOpenDrive = true;
      else if (n == QStringLiteral("road")) ++roads;
      else if (n == QStringLiteral("junction")) ++junctions;
    }
    if (xml.hasError()) {
      scenarioLogLine(QString("✗ XML error: %1").arg(xml.errorString()));
    } else if (!sawOpenDrive) {
      scenarioLogLine("✗ root element is not <OpenDRIVE>");
    } else {
      scenarioLogLine(QString("✓ valid OpenDRIVE: %1 roads, %2 junctions")
                        .arg(roads).arg(junctions));
    }
  });

  QObject::connect(xodrImportBtn, &QPushButton::clicked, &window, [&, scenarioLogLine]() {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    const QString path = xodrPath->text().trimmed();
    if (path.isEmpty()) { scenarioLogLine("✗ no XODR file selected"); return; }
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
      scenarioLogLine("✗ cannot read " + path); return;
    }
    const QByteArray xml = f.readAll();
    auto c = scenarioConnectClient();
    if (!c) return;
    try {
      carla::rpc::OpendriveGenerationParameters params;
      c->GenerateOpenDriveWorld(xml.toStdString(), params);
      scenarioLogLine(QString("✓ imported %1 bytes of XODR as standalone world")
                        .arg(xml.size()));
    } catch (const std::exception &e) {
      scenarioLogLine(QString("✗ GenerateOpenDriveWorld failed: %1").arg(e.what()));
    }
#endif
  });

  QObject::connect(xodrExportBtn, &QPushButton::clicked, &window, [&, scenarioLogLine]() {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    auto c = scenarioConnectClient();
    if (!c) return;
    try {
      auto world = c->GetWorld();
      auto m = world.GetMap();
      const std::string xml = m->GetOpenDrive();
      QString outPath = xodrPath->text().trimmed();
      if (outPath.isEmpty()) {
        outPath = QFileDialog::getSaveFileName(&window, "Export current world XODR",
          QDir::homePath() + "/carla-world.xodr", "OpenDRIVE (*.xodr)");
        if (outPath.isEmpty()) return;
        xodrPath->setText(outPath);
      }
      QFile out(outPath);
      if (!out.open(QIODevice::WriteOnly)) {
        scenarioLogLine("✗ cannot write " + outPath); return;
      }
      out.write(QByteArray::fromStdString(xml));
      scenarioLogLine(QString("✓ exported %1 bytes → %2").arg(xml.size()).arg(outPath));
    } catch (const std::exception &e) {
      scenarioLogLine(QString("✗ export failed: %1").arg(e.what()));
    }
#endif
  });

  QObject::connect(osmBrowseBtn, &QPushButton::clicked, &window, [&]() {
    const QString f = QFileDialog::getOpenFileName(&window,
      "Select OpenStreetMap file", QDir::homePath(),
      "OpenStreetMap (*.osm *.xml *.osm.xml);;All files (*)");
    if (!f.isEmpty()) osmPath->setText(f);
  });

  QObject::connect(osmConvertBtn, &QPushButton::clicked, &window,
                    [&, scenarioLogLine]() {
    const QString inPath = osmPath->text().trimmed();
    if (inPath.isEmpty()) {
      scenarioLogLine("✗ no OSM file selected");
      return;
    }
    if (!QFileInfo(inPath).exists()) {
      scenarioLogLine("✗ OSM file not found: " + inPath);
      return;
    }
    const QString outDir = QDir::tempPath() + "/carla-studio-osm";
    QDir().mkpath(outDir);
    const QString outPath = outDir + "/" + QFileInfo(inPath).baseName() + ".xodr";

    scenarioLogLine("→ Converting " + inPath + " via Python carla.Osm2Odr…");
    osmConvertBtn->setEnabled(false);

    static const char kProgram[] =
      "import sys\n"
      "try:\n"
      "    import carla\n"
      "except Exception as e:\n"
      "    sys.stderr.write('carla import failed: ' + repr(e) + '\\n')\n"
      "    sys.exit(2)\n"
      "with open(sys.argv[1], 'r') as f: osm = f.read()\n"
      "settings = carla.Osm2OdrSettings()\n"
      "settings.use_offsets = False\n"
      "settings.center_map = True\n"
      "xodr = carla.Osm2Odr.convert(osm, settings)\n"
      "with open(sys.argv[2], 'w') as f: f.write(xodr)\n"
      "print('OK ' + str(len(xodr)))\n";

    auto *py = new QProcess(&window);
    py->setProcessChannelMode(QProcess::SeparateChannels);
    {
      const QString activeDist = QSettings().value(
        QStringLiteral("python/active_dist")).toString();
      if (!activeDist.isEmpty()) {
        QProcessEnvironment penv = QProcessEnvironment::systemEnvironment();
        const QString existing = penv.value("PYTHONPATH");
        penv.insert("PYTHONPATH",
          existing.isEmpty() ? activeDist : (activeDist + ":" + existing));
        py->setProcessEnvironment(penv);
        scenarioLogLine("→ using PythonAPI dist: " + activeDist);
      }
    }
    QObject::connect(py,
      static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
      &window, [py, outPath, scenarioLogLine,
                xodrPathPtr = xodrPath, btn = osmConvertBtn]
                (int rc, QProcess::ExitStatus) {
        const QString stderrOut =
          QString::fromLocal8Bit(py->readAllStandardError()).trimmed();
        const QString stdoutOut =
          QString::fromLocal8Bit(py->readAllStandardOutput()).trimmed();
        py->deleteLater();
        btn->setEnabled(true);
        if (rc == 2) {
          scenarioLogLine(
            "✗ Python `import carla` failed. Set up the PythonAPI:\n"
            "    pip install carla              (recommended)\n"
            "  OR set PYTHONPATH to <CARLA_ROOT>/PythonAPI/carla/dist/<egg or wheel>\n"
            "  Health Check → PythonAPI row reports the dist/ location.");
          if (!stderrOut.isEmpty()) scenarioLogLine("  detail: " + stderrOut);
          return;
        }
        if (rc != 0) {
          scenarioLogLine(QString("✗ OSM conversion failed (rc=%1): %2")
                            .arg(rc).arg(stderrOut.isEmpty() ? stdoutOut : stderrOut));
          return;
        }
        scenarioLogLine(QString("✓ converted (%1) → %2").arg(stdoutOut, outPath));
        scenarioLogLine("→ XODR path set; click 'Import to sim' to load it.");
        xodrPathPtr->setText(outPath);
      });
    QObject::connect(py, &QProcess::errorOccurred, &window,
      [scenarioLogLine, btn = osmConvertBtn](QProcess::ProcessError e) {
        if (e == QProcess::FailedToStart) {
          scenarioLogLine("✗ python3 not found on PATH — install Python 3 or set PATH.");
          btn->setEnabled(true);
        }
      });
    py->start("python3", QStringList() << "-c"
                << QString::fromLatin1(kProgram) << inPath << outPath);
  });

#ifdef CARLA_STUDIO_WITH_LIBCARLA
  auto weatherByName = [](const QString &name) -> carla::rpc::WeatherParameters {
    using W = carla::rpc::WeatherParameters;
    if (name == "Default")         return W::Default;
    if (name == "ClearNoon")       return W::ClearNoon;
    if (name == "CloudyNoon")      return W::CloudyNoon;
    if (name == "WetNoon")         return W::WetNoon;
    if (name == "WetCloudyNoon")   return W::WetCloudyNoon;
    if (name == "MidRainyNoon")    return W::MidRainyNoon;
    if (name == "HardRainNoon")    return W::HardRainNoon;
    if (name == "SoftRainNoon")    return W::SoftRainNoon;
    if (name == "ClearSunset")     return W::ClearSunset;
    if (name == "CloudySunset")    return W::CloudySunset;
    if (name == "WetSunset")       return W::WetSunset;
    if (name == "WetCloudySunset") return W::WetCloudySunset;
    if (name == "MidRainSunset")   return W::MidRainSunset;
    if (name == "HardRainSunset")  return W::HardRainSunset;
    if (name == "SoftRainSunset")  return W::SoftRainSunset;
    if (name == "ClearNight")      return W::ClearNight;
    if (name == "CloudyNight")     return W::CloudyNight;
    if (name == "WetNight")        return W::WetNight;
    if (name == "WetCloudyNight")  return W::WetCloudyNight;
    if (name == "SoftRainNight")   return W::SoftRainNight;
    return W::Default;
  };
#endif

  QObject::connect(scenSaveBtn, &QPushButton::clicked, &window, [&, scenarioLogLine]() {
    const QString name = scenarioName->text().trimmed().isEmpty()
                           ? QStringLiteral("scenario")
                           : scenarioName->text().trimmed();
    const QString dir = QDir::homePath() + "/.carla-studio/scenarios";
    QDir().mkpath(dir);
    const QString path = QFileDialog::getSaveFileName(&window, "Save scenario",
      dir + "/" + name + ".json", "Scenario JSON (*.json)");
    if (path.isEmpty()) return;
    QJsonObject obj;
    obj["name"]        = name;
    obj["map"]         = mapCombo->currentText();
    obj["weather"]     = weatherCombo->currentData().toString();
    obj["vehicles"]    = cfgVehicles->value();
    obj["pedestrians"] = cfgPedestrians->value();
    obj["seed"]        = cfgSeed->value();
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
      scenarioLogLine("✗ cannot write " + path); return;
    }
    f.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    scenarioLogLine("✓ saved → " + path);
  });

  QObject::connect(scenLoadBtn, &QPushButton::clicked, &window, [&, scenarioLogLine]() {
    const QString dir = QDir::homePath() + "/.carla-studio/scenarios";
    const QString path = QFileDialog::getOpenFileName(&window, "Load scenario",
      dir, "Scenario JSON (*.json)");
    if (path.isEmpty()) return;
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
      scenarioLogLine("✗ cannot read " + path); return;
    }
    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
      scenarioLogLine("✗ invalid JSON: " + err.errorString()); return;
    }
    const auto o = doc.object();
    scenarioName->setText(o.value("name").toString());
    const QString mapName = o.value("map").toString();
    if (!mapName.isEmpty()) {
      if (mapCombo->findText(mapName) < 0) mapCombo->insertItem(0, mapName);
      mapCombo->setCurrentText(mapName);
    }
    {
      const QString canonical = o.value("weather").toString("Default");
      const int idx = weatherCombo->findData(canonical);
      if (idx >= 0) weatherCombo->setCurrentIndex(idx);
    }
    cfgVehicles->setValue(o.value("vehicles").toInt(30));
    cfgPedestrians->setValue(o.value("pedestrians").toInt(50));
    cfgSeed->setValue(o.value("seed").toInt(42));
    scenarioLogLine("✓ loaded ← " + path);
  });

  QObject::connect(scenApplyBtn, &QPushButton::clicked, &window, [&, scenarioLogLine]() {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    auto c = scenarioConnectClient();
    if (!c) return;
    c->SetTimeout(std::chrono::seconds(15));
    try {
      auto world = c->GetWorld();

      const QString wname  = weatherCombo->currentData().toString();
      const QString wlabel = weatherCombo->currentText();
      world.SetWeather(weatherByName(wname));
      scenarioLogLine(QString("✓ weather applied: %1 (%2)").arg(wlabel, wname));

      const int wantVehicles = cfgVehicles->value();
      const int wantPedestrians = cfgPedestrians->value();
      const int seed = cfgSeed->value();

      auto bps = world.GetBlueprintLibrary();
      if (!bps) { scenarioLogLine("✗ blueprint library unavailable"); return; }

      std::mt19937 rng(static_cast<uint32_t>(seed));

      int vSpawned = 0;
      if (wantVehicles > 0) {
        auto vehicleBps = bps->Filter("vehicle.*");
        auto m = world.GetMap();
        const auto &spawns = m->GetRecommendedSpawnPoints();
        if (!vehicleBps || vehicleBps->empty()) {
          scenarioLogLine("⚠ no vehicle blueprints available — skipping vehicles");
        } else if (spawns.empty()) {
          scenarioLogLine("⚠ map has no spawn points — skipping vehicles");
        } else {
          std::vector<size_t> idx(spawns.size());
          std::iota(idx.begin(), idx.end(), 0);
          std::shuffle(idx.begin(), idx.end(), rng);

          const int target = std::min<int>(wantVehicles, int(idx.size()));
          std::uniform_int_distribution<size_t> bpDist(0, vehicleBps->size() - 1);
          for (int i = 0; i < target; ++i) {
            auto bp = vehicleBps->at(bpDist(rng));
            if (bp.ContainsAttribute("color")) {
              const auto colorAttr = bp.GetAttribute("color");
              const auto recs = colorAttr.GetRecommendedValues();
              if (!recs.empty()) {
                std::uniform_int_distribution<size_t> cDist(0, recs.size() - 1);
                bp.SetAttribute("color", recs[cDist(rng)]);
              }
            }
            if (bp.ContainsAttribute("role_name")) {
              bp.SetAttribute("role_name", "scenario_traffic");
            }
            auto actor = world.TrySpawnActor(bp, spawns[idx[i]]);
            if (!actor) continue;
            try {
              auto v = std::static_pointer_cast<cc::Vehicle>(actor);
              if (v) v->SetAutopilot(true);
            } catch (...) {
            }
            ++vSpawned;
          }
        }
      }
      scenarioLogLine(QString("✓ vehicles spawned: %1 / %2 requested")
                        .arg(vSpawned).arg(wantVehicles));

      int pSpawned = 0;
      if (wantPedestrians > 0) {
        auto walkerBps = bps->Filter("walker.pedestrian.*");
        const auto *controllerBp = bps->Find("controller.ai.walker");
        if (!walkerBps || walkerBps->empty()) {
          scenarioLogLine("⚠ no walker.pedestrian.* blueprints — skipping pedestrians");
        } else if (!controllerBp) {
          scenarioLogLine("⚠ controller.ai.walker blueprint missing — skipping pedestrians");
        } else {
          std::uniform_int_distribution<size_t> bpDist(0, walkerBps->size() - 1);
          for (int i = 0; i < wantPedestrians; ++i) {
            auto loc = world.GetRandomLocationFromNavigation();
            if (!loc) continue;
            cg::Transform tr;
            tr.location = *loc;
            auto bp = walkerBps->at(bpDist(rng));
            if (bp.ContainsAttribute("is_invincible")) {
              bp.SetAttribute("is_invincible", "false");
            }
            if (bp.ContainsAttribute("role_name")) {
              bp.SetAttribute("role_name", "scenario_pedestrian");
            }
            auto walker = world.TrySpawnActor(bp, tr);
            if (!walker) continue;
            auto controllerBpCopy = *controllerBp;
            auto controller = world.TrySpawnActor(
              controllerBpCopy, cg::Transform(), walker.get());
            if (!controller) continue;
            try {
              auto aiCtrl = std::static_pointer_cast<cc::WalkerAIController>(controller);
              if (aiCtrl) {
                aiCtrl->Start();
                auto target = world.GetRandomLocationFromNavigation();
                if (target) aiCtrl->GoToLocation(*target);
              }
              ++pSpawned;
            } catch (...) {
            }
          }
        }
      }
      scenarioLogLine(QString("✓ pedestrians spawned: %1 / %2 requested")
                        .arg(pSpawned).arg(wantPedestrians));

      if ((vSpawned > 0 || pSpawned > 0)
          && (vSpawned < wantVehicles || pSpawned < wantPedestrians)) {
        scenarioLogLine("ℹ Spawn shortfalls usually mean the spawn points were "
                        "already busy or the navmesh isn't ready. Click 'Reload "
                        "[sim]' for a clean slate, then re-apply.");
      }
    } catch (const std::exception &e) {
      scenarioLogLine(QString("✗ apply failed: %1").arg(e.what()));
    } catch (...) {
      scenarioLogLine("✗ apply failed: unknown error");
    }
#else
    scenarioLogLine("✗ LibCarla not linked in this build");
#endif
  });

  QObject::connect(previewRefreshBtn, &QPushButton::clicked, &window, [&, scenarioLogLine]() {
#ifdef CARLA_STUDIO_WITH_LIBCARLA
    auto c = scenarioConnectClient();
    if (!c) return;
    try {
      auto world = c->GetWorld();
      auto m = world.GetMap();
      auto topology = m->GetTopology();
      const auto &spawns = m->GetRecommendedSpawnPoints();

      previewScene->clear();

      double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
      auto grow = [&](double x, double y) {
        minX = std::min(minX, x); maxX = std::max(maxX, x);
        minY = std::min(minY, y); maxY = std::max(maxY, y);
      };
      for (const auto &pair : topology) {
        if (!pair.first || !pair.second) continue;
        const auto l1 = pair.first->GetTransform().location;
        const auto l2 = pair.second->GetTransform().location;
        grow(l1.x, l1.y);
        grow(l2.x, l2.y);
      }
      for (const auto &tr : spawns) {
        grow(tr.location.x, tr.location.y);
      }
      if (minX > maxX) {
        scenarioLogLine("✗ empty topology — is a map loaded?");
        return;
      }

      QPen topoPen(QColor(150, 150, 150));
      topoPen.setWidthF(0);
      for (const auto &pair : topology) {
        if (!pair.first || !pair.second) continue;
        const auto l1 = pair.first->GetTransform().location;
        const auto l2 = pair.second->GetTransform().location;
        previewScene->addLine(l1.x, -l1.y, l2.x, -l2.y, topoPen);
      }

      const double r = std::max(1.5, (std::max(maxX - minX, maxY - minY)) / 300.0);
      QPen spawnPen(QColor(47, 166, 107));
      spawnPen.setWidthF(0);
      QBrush spawnBrush(QColor(47, 166, 107, 200));
      for (const auto &tr : spawns) {
        previewScene->addEllipse(tr.location.x - r, -tr.location.y - r,
                                 2 * r, 2 * r, spawnPen, spawnBrush);
      }

      const double pad = 5.0;
      const QRectF bounds(minX - pad, -maxY - pad,
                          (maxX - minX) + 2 * pad, (maxY - minY) + 2 * pad);
      previewScene->setSceneRect(bounds);
      previewView->fitInView(bounds, Qt::KeepAspectRatio);

      scenarioLogLine(QString("✓ topology: %1 edges, %2 spawn points (bounds %3 × %4 m)")
                        .arg(topology.size())
                        .arg(spawns.size())
                        .arg(maxX - minX, 0, 'f', 1)
                        .arg(maxY - minY, 0, 'f', 1));
    } catch (const std::exception &e) {
      scenarioLogLine(QString("✗ topology fetch failed: %1").arg(e.what()));
    }
#else
    scenarioLogLine("✗ LibCarla not linked in this build");
#endif
  });
  QAction *openThirdParty = nullptr;
  QAction *openComputeHpc = nullptr;

  QGroupBox *pyGroup = new QGroupBox("Python API Environment");
  QVBoxLayout *pyLayout = new QVBoxLayout();
  QLabel *pyDesc = new QLabel(
    "Detected CARLA PythonAPI distributions. The chosen one is prepended "
    "to <code>PYTHONPATH</code> for any Python helper the app shells out "
    "(e.g. the OSM → XODR converter on the Scenario Builder tab).");
  pyDesc->setWordWrap(true);
  pyDesc->setTextFormat(Qt::RichText);
  pyLayout->addWidget(pyDesc);

  QHBoxLayout *pyTopRow = new QHBoxLayout();
  pyTopRow->addWidget(new QLabel("Active version:"));
  QComboBox *pyActiveCombo = new QComboBox();
  pyActiveCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  pyActiveCombo->setMinimumContentsLength(24);
  pyTopRow->addWidget(pyActiveCombo, 1);

  QPushButton *pyRefreshBtn = new QPushButton("⟳");
  pyRefreshBtn->setFixedSize(28, 28);
  pyRefreshBtn->setToolTip(
    "Re-scan: refresh the list of detected PythonAPI distributions "
    "(<CARLA_ROOT>/PythonAPI/carla/dist + system/pip-installed).");
  pyTopRow->addWidget(pyRefreshBtn);

  QPushButton *pyDeleteBtn = new QPushButton("🗑");
  pyDeleteBtn->setFixedSize(28, 28);
  pyDeleteBtn->setToolTip(
    "Delete the selected PythonAPI distribution. Wheels / eggs removed "
    "as files; pip-installed entries via `pip uninstall -y carla`. "
    "Disabled until you select a row in the list below.");
  pyDeleteBtn->setEnabled(false);
  pyTopRow->addWidget(pyDeleteBtn);

  pyLayout->addLayout(pyTopRow);

  QHBoxLayout *pyDetectedHeaderRow = new QHBoxLayout();
  QLabel *pyDetectedHeader = new QLabel("All detected:");
  pyDetectedHeader->setStyleSheet("color:#888; font-size:10px; padding-top: 4px;");
  pyDetectedHeaderRow->addWidget(pyDetectedHeader);
  pyDetectedHeaderRow->addStretch();
  pyLayout->addLayout(pyDetectedHeaderRow);

  QListWidget *pyDetectedList = new QListWidget();
  pyDetectedList->setMaximumHeight(140);
  pyDetectedList->setSelectionMode(QAbstractItemView::SingleSelection);
  pyDetectedList->setStyleSheet("QListWidget { font-family: monospace; font-size: 10px; }");
  pyLayout->addWidget(pyDetectedList);
  QObject::connect(pyDetectedList, &QListWidget::itemSelectionChanged, pyDetectedList,
    [pyDetectedList, pyDeleteBtn]() {
      auto *item = pyDetectedList->currentItem();
      const QString path = item ? item->data(Qt::UserRole).toString() : QString();
      pyDeleteBtn->setEnabled(!path.isEmpty());
    });

  QLabel *pyHint = new QLabel(
    "<i>If you have a source-built PythonAPI in a non-standard location, "
    "either pip-install <code>carla</code> or point <code>$PYTHONPATH</code> "
    "at the egg / wheel before launching CARLA Studio.</i>");
  pyHint->setWordWrap(true);
  pyHint->setTextFormat(Qt::RichText);
  pyHint->setStyleSheet("color:#666; font-size:10px;");
  pyLayout->addWidget(pyHint);

  pyGroup->setLayout(pyLayout);
  if (healthCheckPageOuterLayout) {
    healthCheckPageOuterLayout->addWidget(pyGroup);
  } else {
    pyGroup->setParent(healthCheckPage);
    pyGroup->setVisible(false);
  }
  QWidget *w6 = new QWidget();
  QVBoxLayout *l6 = new QVBoxLayout();
  l6->setContentsMargins(12, 12, 12, 12);
  l6->setSpacing(10);

  QLabel *reconHeader = new QLabel(
    "<h3>Scenario Re-Construction</h3>"
    "<p style='color:#555'>Rebuild scenes from real-world captures via "
    "NVIDIA's reconstruction stack.</p>");
  reconHeader->setTextFormat(Qt::RichText);
  reconHeader->setWordWrap(true);
  l6->addWidget(reconHeader);

  carla_studio::utils::SystemProbe reconProbe;

  QGroupBox *rendererCard = new QGroupBox("Renderer");
  QVBoxLayout *rendererCardLay = new QVBoxLayout();
  rendererCardLay->setContentsMargins(10, 10, 10, 10);
  QLabel *rendererBlurb = new QLabel(
    "<b>Default</b>: Unreal real-time. <b>NuRec</b>: NVIDIA neural "
    "reconstruction (≥ 24 GB VRAM, CUDA 12.8+).");
  rendererBlurb->setWordWrap(true);
  rendererBlurb->setTextFormat(Qt::RichText);
  rendererCardLay->addWidget(rendererBlurb);

  QHBoxLayout *rendererRow = new QHBoxLayout();
  rendererRow->addWidget(new QLabel("Active:"));
  QComboBox *rendererCombo = new QComboBox();
  rendererCombo->addItem("Default (Unreal Engine)", "default");
  rendererCombo->addItem("NuRec (neural rendering)", "nurec");
  {
    const QString saved = QSettings().value("addon/renderer", "default").toString();
    const int idx = rendererCombo->findData(saved);
    rendererCombo->setCurrentIndex(idx >= 0 ? idx : 0);
  }
  {
    const auto fit = carla_studio::utils::evaluate(
        carla_studio::utils::Tool::NuRec, reconProbe);
    if (fit.verdict == carla_studio::utils::Eligibility::HardGrey) {
      QStandardItemModel *model = qobject_cast<QStandardItemModel *>(
          rendererCombo->model());
      if (model) {
        const int nurecIdx = rendererCombo->findData("nurec");
        if (nurecIdx >= 0) {
          QStandardItem *item = model->item(nurecIdx);
          if (item) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
            QString tip = QString("NuRec disabled — %1")
                              .arg(fit.minimumSpecPretty);
            if (!fit.reasons.isEmpty()) tip += "\n• " + fit.reasons.join("\n• ");
            if (!fit.suggestion.isEmpty()) tip += "\n→ " + fit.suggestion;
            item->setToolTip(tip);
          }
        }
      }
    }
  }
  rendererCombo->setToolTip(
    "Default: Unreal Engine real-time renderer (everyday CARLA).\n"
    "NuRec: NVIDIA Neural Reconstruction — photoreal playback of NCore-"
    "captured scenes. Requires ≥ 24 GB VRAM + CUDA 12.8+. The CARLA-side "
    "integration is open-source MIT; the renderer container itself is "
    "proprietary NVIDIA software, free to pull from NGC under their "
    "AI Enterprise EULA.");
  QObject::connect(rendererCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
    &window, [rendererCombo](int) {
      QSettings().setValue("addon/renderer", rendererCombo->currentData().toString());
    });
  rendererRow->addWidget(rendererCombo, 1);
  rendererCardLay->addLayout(rendererRow);
  rendererCard->setLayout(rendererCardLay);
  l6->addWidget(rendererCard);

  QLabel *paiHint = new QLabel(
    "<small>📦 Tools consume the "
    "<a href=\"https://huggingface.co/datasets/nvidia/PhysicalAI-Autonomous-Vehicles\">"
    "Physical AI AV</a> dataset — fetch via <i>Cfg → HF Datasets</i> "
    "(gated repo).</small>");
  paiHint->setTextFormat(Qt::RichText);
  paiHint->setOpenExternalLinks(true);
  paiHint->setWordWrap(true);
  paiHint->setStyleSheet("color:#555; padding: 2px 4px;");
  l6->addWidget(paiHint);

  auto buildReconTile = [&](const QString &name,
                             const QString &desc,
                             const QString &repoUrl,
                             carla_studio::utils::Tool tool) -> QGroupBox * {
    QGroupBox *card = new QGroupBox();
    card->setTitle(name);
    QVBoxLayout *cl = new QVBoxLayout();
    cl->setContentsMargins(10, 10, 10, 10);
    cl->setSpacing(6);

    QLabel *body = new QLabel(desc);
    body->setWordWrap(true);
    cl->addWidget(body);

    QLabel *repo = new QLabel(QString(
      "<small><a href=\"%1\">%1</a></small>").arg(repoUrl));
    repo->setTextFormat(Qt::RichText);
    repo->setOpenExternalLinks(true);
    cl->addWidget(repo);

    const auto fit = carla_studio::utils::evaluate(tool, reconProbe);
    QString badgeColor;
    QString badgeText;
    switch (fit.verdict) {
      case carla_studio::utils::Eligibility::Eligible:
        badgeColor = "#1b5e20"; badgeText = "● Eligible on this machine";
        break;
      case carla_studio::utils::Eligibility::SoftWarn:
        badgeColor = "#f57f17"; badgeText = "● Runs with reduced fidelity";
        break;
      case carla_studio::utils::Eligibility::HardGrey:
        badgeColor = "#b71c1c"; badgeText = "● Local hardware insufficient";
        break;
    }
    QLabel *badge = new QLabel(QString("<small style='color:%1'>%2</small>")
                                 .arg(badgeColor).arg(badgeText));
    badge->setTextFormat(Qt::RichText);
    cl->addWidget(badge);

    QString tip = QString("Minimum: %1").arg(fit.minimumSpecPretty);
    if (!fit.reasons.isEmpty()) {
      tip += "\n• " + fit.reasons.join("\n• ");
    }
    if (!fit.suggestion.isEmpty()) {
      tip += "\n→ " + fit.suggestion;
    }
    badge->setToolTip(tip);

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *installBtn = new QPushButton("Install");
    QPushButton *openBtn    = new QPushButton("Open");
    QPushButton *runBtn     = new QPushButton("Run");
    installBtn->setToolTip("Open Cfg → Third-Party Tools to install or "
                            "update this component.");
    openBtn->setToolTip("Open the upstream documentation page.");
    runBtn->setToolTip(tip);
    if (fit.verdict == carla_studio::utils::Eligibility::HardGrey) {
      runBtn->setEnabled(false);
    }
    btnRow->addWidget(installBtn);
    btnRow->addWidget(openBtn);
    btnRow->addWidget(runBtn);
    btnRow->addStretch();
    cl->addLayout(btnRow);
    card->setLayout(cl);

    QObject::connect(installBtn, &QPushButton::clicked, &window, [&]() {
      openThirdParty->trigger();
    });
    QObject::connect(openBtn, &QPushButton::clicked, &window, [repoUrl]() {
      QDesktopServices::openUrl(QUrl(repoUrl));
    });
    QObject::connect(runBtn, &QPushButton::clicked, &window, [&]() {
      openThirdParty->trigger();
    });
    return card;
  };

  QGridLayout *tileGrid = new QGridLayout();
  tileGrid->setHorizontalSpacing(10);
  tileGrid->setVerticalSpacing(10);

  tileGrid->addWidget(buildReconTile(
    "NuRec — Neural Renderer",
    "Re-render NCore captures with NVIDIA's neural reconstructor.",
    "https://carla.readthedocs.io/en/0.9.16/nvidia_nurec/",
    carla_studio::utils::Tool::NuRec), 0, 0);

  tileGrid->addWidget(buildReconTile(
    "Asset-Harvester",
    "Lift sim-ready 3D assets out of real driving logs.",
    "https://github.com/NVIDIA/asset-harvester",
    carla_studio::utils::Tool::AssetHarvester), 0, 1);

  tileGrid->addWidget(buildReconTile(
    "NCore — Data Pipeline",
    "Canonical multi-sensor format. Feeds Asset-Harvester + NuRec.",
    "https://github.com/NVIDIA/ncore",
    carla_studio::utils::Tool::CarlaServer
  ), 1, 0);

  tileGrid->addWidget(buildReconTile(
    "Lyra 2.0 — Generative 3D Worlds",
    "Generate explorable 3D scenes from prompts.",
    "https://github.com/nv-tlabs/lyra",
    carla_studio::utils::Tool::Lyra2Generation), 1, 1);

  l6->addLayout(tileGrid);

  QLabel *reconFooter = new QLabel(QString(
    "<small>Detected GPU: <b>%1</b> · VRAM: <b>%2</b> · CUDA: <b>%3</b></small>")
      .arg(reconProbe.primaryGpu()
            ? reconProbe.primaryGpu()->name.toHtmlEscaped()
            : "none")
      .arg(carla_studio::utils::humanBytes(reconProbe.bestVramBytes()))
      .arg(reconProbe.cudaToolkitVersion().isEmpty()
            ? "—" : reconProbe.cudaToolkitVersion()));
  reconFooter->setTextFormat(Qt::RichText);
  reconFooter->setStyleSheet("color:#555;");
  l6->addWidget(reconFooter);
  l6->addStretch();
  w6->setLayout(l6);

  QWidget *w7 = new QWidget();
  QVBoxLayout *l7 = new QVBoxLayout();
  l7->setContentsMargins(12, 12, 12, 12);
  l7->setSpacing(10);
  QLabel *lbHeader = new QLabel(
    "Leaderboard. Submit and rank agents on standardised routes — "
    "<a href=\"https://leaderboard.carla.org/\">leaderboard.carla.org</a>. "
    "Plus regulator protocol catalogues (Euro NCAP, NHTSA).");
  lbHeader->setTextFormat(Qt::RichText);
  lbHeader->setOpenExternalLinks(true);
  lbHeader->setWordWrap(true);
  QLabel *lbStatus = new QLabel(
    "<div style='background:#fff8e1; padding:10px; "
    "border:1px solid #ffe082; border-radius:6px;'>"
    "🛣 Roadmap — placeholder. Tracked under <i>Roadmap</i> in the "
    "bundled CHANGELOG (Help → License → CHANGELOG).</div>");
  lbStatus->setTextFormat(Qt::RichText);
  lbStatus->setWordWrap(true);
  l7->addWidget(lbStatus);

  QHBoxLayout *regPanels = new QHBoxLayout();
  regPanels->setSpacing(8);

  QGroupBox *regEuro = new QGroupBox("Euro NCAP");
  QVBoxLayout *regEuroLay = new QVBoxLayout();
  regEuroLay->addWidget(new QLabel(
    "<small><a href=\"https://www.euroncap.com/en/for-engineers/protocols/\">"
    "euroncap.com/en/for-engineers/protocols</a></small>"));
  QListWidget *regEuroList = new QListWidget();
  regEuroList->setSelectionMode(QAbstractItemView::NoSelection);
  const QStringList kEuroNcapProtocols = {
    "AEB Car-to-Car Rear (CCR)",
    "AEB Car-to-Car Front Turn-Across-Path (FTAP)",
    "AEB Vulnerable Road User (VRU) — Pedestrian",
    "AEB VRU — Cyclist",
    "AEB VRU — Powered Two-Wheeler",
    "Lane Support Systems (LSS) — LKA / LDW / ELK",
    "Speed Assist Systems (SAS)",
    "Driver State Monitoring (DSM)",
  };
  for (const QString &name : kEuroNcapProtocols) regEuroList->addItem(name);
  regEuroLay->addWidget(regEuroList, 1);
  regEuro->setLayout(regEuroLay);
  regPanels->addWidget(regEuro, 1);

  QGroupBox *regNhtsa = new QGroupBox("NHTSA");
  QVBoxLayout *regNhtsaLay = new QVBoxLayout();
  regNhtsaLay->addWidget(new QLabel(
    "<small><a href=\"https://www.nhtsa.gov/laws-regulations/fmvss\">"
    "nhtsa.gov/laws-regulations/fmvss</a></small>"));
  QListWidget *regNhtsaList = new QListWidget();
  regNhtsaList->setSelectionMode(QAbstractItemView::NoSelection);
  const QStringList kNhtsaProtocols = {
    "FMVSS-126 — Electronic Stability Control",
    "FMVSS-127 — Automatic Emergency Braking",
    "FMVSS-208 — Occupant Crash Protection",
    "FMVSS-214 — Side Impact Protection",
    "NCAP — Frontal Crash",
    "NCAP — Side Crash (MDB / Pole)",
    "NCAP — Rollover Resistance",
    "AEB — Pedestrian (NCAP 2024+)",
  };
  for (const QString &name : kNhtsaProtocols) regNhtsaList->addItem(name);
  regNhtsaLay->addWidget(regNhtsaList, 1);
  regNhtsa->setLayout(regNhtsaLay);
  regPanels->addWidget(regNhtsa, 1);

  l7->addLayout(regPanels);
  l7->addWidget(lbHeader);

  QGroupBox *lbRanking = new QGroupBox("Ranking");
  QVBoxLayout *lbRankingLay = new QVBoxLayout();
  lbRankingLay->addWidget(new QLabel("Browse + filter current submissions."));
  QPushButton *lbOpenWeb = new QPushButton("Open leaderboard.carla.org");
  QObject::connect(lbOpenWeb, &QPushButton::clicked, &window, []() {
    QDesktopServices::openUrl(QUrl("https://leaderboard.carla.org/"));
  });
  lbRankingLay->addWidget(lbOpenWeb);
  lbRanking->setLayout(lbRankingLay);
  l7->addWidget(lbRanking);

  QGroupBox *lbRunner = new QGroupBox("Local agent runner");
  QVBoxLayout *lbRunnerLay = new QVBoxLayout();
  lbRunnerLay->addWidget(new QLabel(
    "Pick a Python agent + route, run it locally, score it."));
  QPushButton *lbRunnerBtn = new QPushButton("Run agent");
  lbRunnerBtn->setEnabled(false);
  lbRunnerBtn->setToolTip("Coming in roadmap — see CHANGELOG.");
  lbRunnerLay->addWidget(lbRunnerBtn);
  lbRunner->setLayout(lbRunnerLay);
  l7->addWidget(lbRunner);

  QGroupBox *lbSubmit = new QGroupBox("Submission helper");
  QVBoxLayout *lbSubmitLay = new QVBoxLayout();
  lbSubmitLay->addWidget(new QLabel(
    "Build the agent container + push to the autograder."));
  QPushButton *lbSubmitBtn = new QPushButton("Submit");
  lbSubmitBtn->setEnabled(false);
  lbSubmitBtn->setToolTip("Coming in roadmap — see CHANGELOG.");
  lbSubmitLay->addWidget(lbSubmitBtn);
  lbSubmit->setLayout(lbSubmitLay);
  l7->addWidget(lbSubmit);

  l7->addStretch();
  w7->setLayout(l7);

  QWidget *w9 = new QWidget();
  QHBoxLayout *l9 = new QHBoxLayout();
  l9->setContentsMargins(12, 12, 12, 12);
  l9->setSpacing(12);

  vehicleGroup->setParent(w9);
  l9->addWidget(vehicleGroup, 0);

  class VehiclePreview : public QFrame {
   public:
    QString blueprintId;
    QColor  bodyColor = QColor("#1E1E1E");
    QString finish    = "standard";
    VehiclePreview() {
      setFrameShape(QFrame::StyledPanel);
      setMinimumSize(320, 220);
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    void paintEvent(QPaintEvent *) override {
      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing);
      QLinearGradient bg(0, 0, 0, height());
      bg.setColorAt(0.0, QColor(245, 247, 250));
      bg.setColorAt(1.0, QColor(220, 225, 232));
      p.fillRect(rect(), bg);
      const QRectF floor(width() * 0.10, height() * 0.72,
                          width() * 0.80, height() * 0.10);
      p.setPen(Qt::NoPen);
      p.setBrush(QColor(0, 0, 0, 50));
      p.drawEllipse(floor);

      const bool isBike = blueprintId.contains("crossbike") ||
                          blueprintId.contains("ninja") ||
                          blueprintId.contains("low_rider") ||
                          blueprintId.contains("yzf") ||
                          blueprintId.contains("century") ||
                          blueprintId.contains("omafiets");
      const bool isTruck = blueprintId.contains("firetruck") ||
                           blueprintId.contains("ambulance") ||
                           blueprintId.contains("carlacola") ||
                           blueprintId.contains("sprinter") ||
                           blueprintId.contains("cybertruck") ||
                           blueprintId.contains("t2");
      const bool isSuv  = blueprintId.contains("wrangler") ||
                          blueprintId.contains("patrol") ||
                          blueprintId.contains("etron");

      const qreal cx = width()  * 0.50;
      const qreal cy = height() * 0.60;
      const qreal w  = width()  * 0.70;
      const qreal h  = isBike  ? height() * 0.18
                       : isTruck ? height() * 0.36
                       : isSuv   ? height() * 0.32
                                 : height() * 0.26;

      QRectF body(cx - w / 2, cy - h / 2, w, h);
      QPainterPath bodyPath;
      bodyPath.addRoundedRect(body, h * 0.35, h * 0.35);
      p.setBrush(bodyColor);
      p.setPen(QPen(bodyColor.darker(140), 1.2));
      p.drawPath(bodyPath);

      if (!isBike) {
        QRectF cabin(body.left() + body.width() * 0.18,
                     body.top()  - body.height() * 0.30,
                     body.width() * 0.55,
                     body.height() * 0.55);
        QPainterPath cabinPath;
        cabinPath.moveTo(cabin.left() + 8,                   cabin.bottom());
        cabinPath.lineTo(cabin.left() + cabin.width() * 0.18, cabin.top() + 4);
        cabinPath.lineTo(cabin.left() + cabin.width() * 0.78, cabin.top() + 4);
        cabinPath.lineTo(cabin.right() - 8,                   cabin.bottom());
        cabinPath.closeSubpath();
        p.setBrush(QColor(0, 0, 0, 35));
        p.setPen(QPen(bodyColor.darker(150), 1.0));
        p.drawPath(cabinPath);
      }

      const qreal wheelR = isBike ? h * 0.55
                          : isTruck ? h * 0.30
                                    : h * 0.27;
      QPointF wheelL(body.left()  + body.width() * (isBike ? 0.18 : 0.20),
                      body.bottom() - 2);
      QPointF wheelR2(body.right() - body.width() * (isBike ? 0.18 : 0.20),
                      body.bottom() - 2);
      p.setBrush(QColor(35, 35, 38));
      p.setPen(QPen(QColor(20, 20, 22), 1.0));
      p.drawEllipse(wheelL,  wheelR, wheelR);
      p.drawEllipse(wheelR2, wheelR, wheelR);
      p.setBrush(QColor(180, 180, 185));
      p.setPen(Qt::NoPen);
      p.drawEllipse(wheelL,  wheelR * 0.25, wheelR * 0.25);
      p.drawEllipse(wheelR2, wheelR * 0.25, wheelR * 0.25);

      if (finish == "matte") {
        p.setBrush(QColor(0, 0, 0, 35));
        p.setPen(Qt::NoPen);
        p.drawPath(bodyPath);
      } else if (finish == "clearcoat" || finish == "pearl") {
        QLinearGradient sheen(body.left(), body.top(), body.right(), body.bottom());
        sheen.setColorAt(0.0, QColor(255, 255, 255, 0));
        sheen.setColorAt(0.45, QColor(255, 255, 255, finish == "pearl" ? 90 : 60));
        sheen.setColorAt(0.6, QColor(255, 255, 255, 0));
        p.setBrush(sheen);
        p.setPen(Qt::NoPen);
        p.drawPath(bodyPath);
      } else if (finish == "triplecoat") {
        for (int i = 0; i < 2; ++i) {
          QLinearGradient sheen(body.left() - i * 30, body.top(),
                                  body.right(), body.bottom() + i * 20);
          sheen.setColorAt(0.0, QColor(255, 255, 255, 0));
          sheen.setColorAt(0.5, QColor(255, 255, 255, 70));
          sheen.setColorAt(1.0, QColor(255, 255, 255, 0));
          p.setBrush(sheen);
          p.setPen(Qt::NoPen);
          p.drawPath(bodyPath);
        }
      }

      p.setPen(QColor(80, 90, 105));
      QFont f = p.font();
      f.setPointSizeF(f.pointSizeF() * 0.95);
      p.setFont(f);
      p.drawText(QRectF(0, height() - 24, width(), 22),
                  Qt::AlignCenter,
                  QString("%1 · %2").arg(
                    blueprintId.isEmpty() ? "vehicle.*" : blueprintId,
                    finish.isEmpty()      ? "standard"  : finish));
    }
  };
  VehiclePreview *vehiclePreview = new VehiclePreview();
  vehiclePreview->blueprintId = vehicleBlueprintCombo->currentText();
  vehiclePreview->bodyColor   =
    QColor(QSettings().value("vehicle/color", "#1E1E1E").toString());
  vehiclePreview->finish      =
    QSettings().value("vehicle/finish", "standard").toString();
  l9->addWidget(vehiclePreview, 1);

  QObject::connect(vehicleBlueprintCombo, &QComboBox::currentTextChanged,
    vehiclePreview, [vehiclePreview](const QString &s) {
      vehiclePreview->blueprintId = s;
      vehiclePreview->update();
    });
  QObject::connect(colorSwatch, &QPushButton::clicked, vehiclePreview,
    [vehiclePreview]() {
      vehiclePreview->bodyColor =
        QColor(QSettings().value("vehicle/color", "#1E1E1E").toString());
      vehiclePreview->update();
    }, Qt::QueuedConnection);
  QObject::connect(vehicleFinishCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged), vehiclePreview,
    [vehiclePreview, vehicleFinishCombo](int) {
      vehiclePreview->finish = vehicleFinishCombo->currentData().toString();
      vehiclePreview->update();
    });

  w9->setLayout(l9);

  auto detectPythonApiInstalls = [&]() -> QList<QPair<QString, QString>> {
    QList<QPair<QString, QString>> out;
    QSet<QString> seen;
    const QString root = carlaRootPath->text().trimmed();
    if (!root.isEmpty()) {
      QDir distDir(root + "/PythonAPI/carla/dist");
      if (distDir.exists()) {
        const QStringList eggs = distDir.entryList(
          QStringList() << "carla-*.egg" << "carla-*.whl", QDir::Files);
        for (const QString &egg : eggs) {
          const QString p = distDir.absoluteFilePath(egg);
          out.append({ egg + "   →   " + p, p });
          seen.insert(p);
        }
      }
    }
    QProcess p;
    p.start("python3", QStringList() << "-c"
      << "import sys\n"
         "try:\n"
         "    import carla, os\n"
         "    print(getattr(carla, '__version__', '?'), os.path.dirname(carla.__file__))\n"
         "except Exception:\n"
         "    sys.exit(1)");
    p.waitForFinished(2500);
    if (p.exitCode() == 0) {
      const QString line = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
      const int sp = line.indexOf(' ');
      if (sp > 0) {
        const QString ver = line.left(sp);
        const QString path = line.mid(sp + 1);
        if (!seen.contains(path)) {
          out.append({ QString("carla %1   →   %2 (system / pip)").arg(ver, path), path });
        }
      }
    }
    return out;
  };

  auto refreshPyApiList = [&, detectPythonApiInstalls]() {
    const QString persisted = QSettings().value(
      QStringLiteral("python/active_dist")).toString();
    pyActiveCombo->blockSignals(true);
    pyActiveCombo->clear();
    pyDetectedList->clear();
    pyActiveCombo->addItem("(none — system default)", QString());
    const auto entries = detectPythonApiInstalls();
    for (const auto &[label, path] : entries) {
      pyActiveCombo->addItem(label, path);
      auto *item = new QListWidgetItem(label);
      item->setData(Qt::UserRole, path);
      pyDetectedList->addItem(item);
    }
    if (entries.isEmpty()) {
      pyDetectedList->addItem(new QListWidgetItem(
        "(no carla-*.egg / *.whl found — pip install carla, or set CARLA_ROOT)"));
    }
    int idx = persisted.isEmpty() ? 0 : pyActiveCombo->findData(persisted);
    if (idx < 0) idx = 0;
    pyActiveCombo->setCurrentIndex(idx);
    pyActiveCombo->blockSignals(false);
  };
  QObject::connect(pyRefreshBtn, &QPushButton::clicked, healthCheckPage,
                    [refreshPyApiList]() { refreshPyApiList(); });
  QObject::connect(pyActiveCombo,
    QOverload<int>::of(&QComboBox::currentIndexChanged), healthCheckPage,
    [pyActiveCombo](int) {
      QSettings().setValue(QStringLiteral("python/active_dist"),
                            pyActiveCombo->currentData().toString());
    });

  QObject::connect(pyDeleteBtn, &QPushButton::clicked, healthCheckPage,
    [&, refreshPyApiList, pyDetectedList]() {
      auto *item = pyDetectedList->currentItem();
      if (!item) return;
      const QString path = item->data(Qt::UserRole).toString();
      if (path.isEmpty()) return;

      const bool isFile = path.endsWith(".whl") || path.endsWith(".egg");
      const bool isPipPackage = !isFile &&
                                 (path.contains("/site-packages/carla") ||
                                  path.contains("\\site-packages\\carla"));

      QString summary;
      if (isFile) {
        summary = QString("Delete this CARLA PythonAPI wheel/egg?\n\n%1\n\n"
                          "The file will be removed from disk. The Active "
                          "version dropdown will reset to (none) if it pointed "
                          "here.").arg(path);
      } else if (isPipPackage) {
        summary = QString("Uninstall the pip-installed CARLA package?\n\n%1\n\n"
                          "Studio will run `python3 -m pip uninstall -y carla` "
                          "to remove it cleanly.").arg(path);
      } else {
        summary = QString("Remove this CARLA distribution?\n\n%1\n\n"
                          "Path doesn't match a known installer pattern; the "
                          "directory will be removed recursively. Double-check "
                          "the path before confirming.").arg(path);
      }

      const auto ans = QMessageBox::question(&window,
        "Delete PythonAPI distribution", summary,
        QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);
      if (ans != QMessageBox::Yes) return;

      QString failure;
      if (isFile) {
        if (!QFile::remove(path)) {
          failure = QString("Could not remove %1. Check write permissions.").arg(path);
        }
      } else if (isPipPackage) {
        QProcess pip;
        pip.start("python3", QStringList()
                              << "-m" << "pip" << "uninstall" << "-y" << "carla");
        pip.waitForFinished(20000);
        if (pip.exitStatus() != QProcess::NormalExit || pip.exitCode() != 0) {
          failure = QString("`pip uninstall -y carla` failed.\n\nstderr: %1")
                      .arg(QString::fromLocal8Bit(pip.readAllStandardError()).left(600));
        }
      } else {
        QDir d(path);
        if (!d.removeRecursively()) {
          failure = QString("Could not remove %1 recursively. Check permissions.").arg(path);
        }
      }

      if (!failure.isEmpty()) {
        QMessageBox::warning(&window, "Delete failed", failure);
      }
      if (QSettings().value("python/active_dist").toString() == path) {
        QSettings().remove("python/active_dist");
      }
      refreshPyApiList();
    });

  refreshPyApiList();
  QWidget *w5 = new QWidget();
  QVBoxLayout *l5 = new QVBoxLayout();

  QGroupBox *sshGroup = new QGroupBox("SSH / Remote Settings (Built-in)");
  QVBoxLayout *sshLayout = new QVBoxLayout();

  QPushButton *reloadSavedBtn = new QPushButton("Reload Saved Data");
  sshLayout->addWidget(reloadSavedBtn);

  QListWidget *remoteHostsList = new QListWidget();
  sshLayout->addWidget(new QLabel("Configured remote hosts (remembered):"));
  sshLayout->addWidget(remoteHostsList);

  QHBoxLayout *hostInputLayout = new QHBoxLayout();
  QLineEdit *remoteHostInput = new QLineEdit();
  remoteHostInput->setPlaceholderText("hash-gpu-server");
  QPushButton *addHostBtn = new QPushButton("Add Host");
  hostInputLayout->addWidget(remoteHostInput);
  hostInputLayout->addWidget(addHostBtn);
  sshLayout->addLayout(hostInputLayout);
  QPushButton *removeHostBtn = new QPushButton("Remove Selected Host");
  sshLayout->addWidget(removeHostBtn);

  QHBoxLayout *keyInputLayout = new QHBoxLayout();
  QLineEdit *sshKeyPath = new QLineEdit();
  sshKeyPath->setPlaceholderText("/home/user/.ssh/id_rsa");
  QPushButton *addKeyBtn = new QPushButton("Add Key");
  keyInputLayout->addWidget(sshKeyPath);
  keyInputLayout->addWidget(addKeyBtn);
  sshLayout->addLayout(keyInputLayout);

  QListWidget *sshKeysList = new QListWidget();
  sshLayout->addWidget(new QLabel("Managed SSH keys:"));
  sshLayout->addWidget(sshKeysList);
  QPushButton *removeKeyBtn = new QPushButton("Remove Selected Key");
  sshLayout->addWidget(removeKeyBtn);

  sshGroup->setLayout(sshLayout);
  l5->addWidget(sshGroup);
  l5->addStretch();
  w5->setLayout(l5);
  const int homeTabIndex = tabs->addTab(w1, QStringLiteral("⌂"));
  tabs->setTabToolTip(homeTabIndex, "Home");
  const int interfacesTabIndex = tabs->addTab(w2, QStringLiteral("⇄"));
  tabs->setTabToolTip(interfacesTabIndex, "Interfaces");
  const QColor tabIconColor = app.palette().color(QPalette::ButtonText);
  auto makeTabIcon = [tabIconColor](const std::function<void(QPainter &)> &draw) -> QIcon {
    QPixmap pm(22, 22);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(tabIconColor, 1.7));
    p.setBrush(Qt::NoBrush);
    draw(p);
    return QIcon(pm);
  };

  const QIcon healthCheckIcon = makeTabIcon([](QPainter &p) {
    QPainterPath tube;
    tube.moveTo(5, 3);
    tube.lineTo(5, 9);
    tube.cubicTo(5, 14, 14, 14, 14, 9);
    tube.lineTo(14, 3);
    p.drawPath(tube);
    p.drawEllipse(QPointF(14, 16), 3.0, 3.0);
    p.setBrush(p.pen().color());
    p.drawEllipse(QPointF(5, 3), 1.2, 1.2);
    p.drawEllipse(QPointF(14, 3), 1.2, 1.2);
  });

  const QIcon scenarioBuilderIcon = makeTabIcon([](QPainter &p) {
    p.setBrush(p.pen().color());
    QPolygonF peaks;
    peaks << QPointF(2, 19) << QPointF(8, 7) << QPointF(11, 12)
          << QPointF(15, 5) << QPointF(20, 19);
    p.drawPolygon(peaks);
  });

  const QIcon loggingIcon = makeTabIcon([](QPainter &p) {
    QPainterPath doc;
    doc.moveTo(4, 3);
    doc.lineTo(13, 3);
    doc.lineTo(18, 8);
    doc.lineTo(18, 19);
    doc.lineTo(4, 19);
    doc.closeSubpath();
    p.drawPath(doc);
    p.drawLine(QPointF(13, 3), QPointF(13, 8));
    p.drawLine(QPointF(13, 8), QPointF(18, 8));
    p.drawLine(QPointF(7, 11), QPointF(15, 11));
    p.drawLine(QPointF(7, 14), QPointF(15, 14));
    p.drawLine(QPointF(7, 17), QPointF(12, 17));
  });

  const int healthCheckTabIndex = tabs->addTab(healthCheckPage, healthCheckIcon, QString());
  tabs->setTabToolTip(healthCheckTabIndex, "Health Check");
  const int scenarioBuilderTabIndex = tabs->addTab(w3, scenarioBuilderIcon, QString());
  tabs->setTabToolTip(scenarioBuilderTabIndex, "Scenario Builder");

  const QIcon vehicleIcon = makeTabIcon([](QPainter &p) {
    QPen thin = p.pen();
    thin.setWidthF(1.1);
    p.setPen(thin);
    QPainterPath car;
    car.moveTo(5,  14);
    car.lineTo(5,  11);
    car.lineTo(8,  11);
    car.lineTo(9,  7);
    car.lineTo(13, 7);
    car.lineTo(14, 11);
    car.lineTo(17, 11);
    car.lineTo(17, 14);
    car.closeSubpath();
    p.drawPath(car);
    p.drawEllipse(QPointF(8.0,  15.0), 1.6, 1.6);
    p.drawEllipse(QPointF(14.0, 15.0), 1.6, 1.6);
  });
  tabs->setIconSize(QSize(22, 22));

  auto findUnrealEditorBin = [&]() -> QString {
    QStringList candidates;
    const QString ueEnv = QString::fromLocal8Bit(qgetenv("CARLA_UNREAL_ENGINE_PATH")).trimmed();
    if (!ueEnv.isEmpty())
      candidates << ueEnv + "/Engine/Binaries/Linux/UnrealEditor";
    const QString carlaRoot = carlaRootPath ? carlaRootPath->text().trimmed() : QString();
    if (!carlaRoot.isEmpty()) {
      candidates << carlaRoot + "/Engine/Binaries/Linux/UnrealEditor";
      QDir d(carlaRoot); d.cdUp();
      candidates << d.absolutePath() + "/Engine/Binaries/Linux/UnrealEditor";
      candidates << d.absolutePath() + "/UnrealEngine/Engine/Binaries/Linux/UnrealEditor";
    }
    for (const QString &p : candidates)
      if (QFileInfo(p).isExecutable()) return p;
    return QString();
  };
  auto findVehicleUproject = [&]() -> QString {
    QStringList roots;
    if (carlaRootPath) roots << QDir::cleanPath(carlaRootPath->text().trimmed());
    {
      QDir d(QCoreApplication::applicationDirPath());
      for (int i = 0; i < 6; ++i) { roots << d.absolutePath(); if (!d.cdUp()) break; }
    }
    const QString src = QString::fromLocal8Bit(qgetenv("CARLA_SRC_ROOT")).trimmed();
    if (!src.isEmpty()) roots << QDir::cleanPath(src);

    auto hasVehicleImporter = [](const QString &uproj) {
      const QDir d(QFileInfo(uproj).absolutePath());
      return QFileInfo(d.absoluteFilePath(
          "Plugins/CarlaTools/Source/CarlaTools/Public/VehicleImporter.h")).isFile();
    };
    auto hasCarlaPlugin = [](const QString &uproj) {
      const QDir d(QFileInfo(uproj).absolutePath());
      return QFileInfo(d.absoluteFilePath("Plugins/Carla/Carla.uplugin")).isFile();
    };
    QString carlaOnly, anyUproject;
    for (const QString &root : roots) {
      for (const QString &c : {
          root + "/Unreal/CarlaUnreal/CarlaUnreal.uproject",
          root + "/Unreal/CarlaUE4/CarlaUE4.uproject",
      }) {
        if (!QFileInfo(c).isFile()) continue;
        if (hasCarlaPlugin(c) && hasVehicleImporter(c)) return c;
        if (carlaOnly.isEmpty()   && hasCarlaPlugin(c)) carlaOnly   = c;
        if (anyUproject.isEmpty())                       anyUproject = c;
      }
    }
    return !carlaOnly.isEmpty() ? carlaOnly : anyUproject;
  };

  auto findCarlaRootForPkg = [&]() -> QString {
    return carlaRootPath ? carlaRootPath->text().trimmed() : QString();
  };
  auto *vehicleImportContainer = new carla_studio::vehicle_import::VehicleImportContainer(
      findUnrealEditorBin, findVehicleUproject, findCarlaRootForPkg);
  Q_UNUSED(startBtn);
  vehicleImportContainer->setVisible(false);
  bool vehicleImportTabVisible = false;
  auto setVehicleImportTabVisible = [&, vehicleImportContainer, vehicleIcon](bool on) {
    if (on == vehicleImportTabVisible) return;
    vehicleImportTabVisible = on;
    if (on) {
      vehicleImportContainer->setVisible(true);
      const int idx = tabs->addTab(vehicleImportContainer, vehicleIcon, QString());
      tabs->setTabToolTip(idx, "Vehicle Import");
      tabs->setCurrentIndex(idx);
    } else {
      const int idx = tabs->indexOf(vehicleImportContainer);
      if (idx >= 0) tabs->removeTab(idx);
      vehicleImportContainer->setVisible(false);
    }
  };
  const int loggingTabIndex = -1;
  Q_UNUSED(loggingIcon);

  const QIcon cameraIcon = makeTabIcon([](QPainter &p) {
    QPainterPath body;
    body.addRoundedRect(2.5, 7, 17, 11, 2, 2);
    p.drawPath(body);
    QPainterPath bump;
    bump.addRect(8, 4, 6, 3);
    p.drawPath(bump);
    p.setBrush(p.pen().color());
    p.drawEllipse(QPointF(11, 13), 3.0, 3.0);
    p.setBrush(Qt::NoBrush);
  });

  class SnippingOverlay : public QWidget {
   public:
    std::function<void(const QRect &)> onSelected;
    QPixmap capturedDesktop;
    QPoint pressPt;
    QRect  selection;
    bool   dragging = false;
    SnippingOverlay() {
      setWindowFlags(Qt::FramelessWindowHint
                     | Qt::WindowStaysOnTopHint
                     | Qt::Tool);
      setAttribute(Qt::WA_DeleteOnClose);
      setCursor(Qt::CrossCursor);
      setMouseTracking(true);
    }
    void paintEvent(QPaintEvent *) override {
      QPainter p(this);
      if (!capturedDesktop.isNull()) {
        p.drawPixmap(rect(), capturedDesktop);
      } else {
        p.fillRect(rect(), QColor(20, 20, 24));
      }
      const QColor dim(0, 0, 0, 100);
      if (selection.isNull()) {
        p.fillRect(rect(), dim);
        p.setPen(QColor(255, 255, 255, 230));
        QFont f = p.font(); f.setPointSize(11); p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter,
                   QStringLiteral("Drag to select a region · Esc / right-click to cancel"));
      } else {
        QRegion outside(rect());
        outside -= selection;
        for (const QRect &r : outside) p.fillRect(r, dim);
        p.setPen(QPen(QColor(47, 166, 107), 2));
        p.drawRect(selection);
        p.setPen(QColor(255, 255, 255));
        p.setBrush(QColor(0, 0, 0, 160));
        const QString label = QString("%1 × %2").arg(selection.width()).arg(selection.height());
        const QRect badge(selection.left() + 4, selection.top() + 4, 90, 20);
        p.drawRect(badge);
        p.drawText(badge, Qt::AlignCenter, label);
      }
    }
    void mousePressEvent(QMouseEvent *e) override {
      if (e->button() == Qt::LeftButton) {
        pressPt = e->pos();
        selection = QRect(pressPt, QSize(0, 0));
        dragging = true;
        update();
      } else if (e->button() == Qt::RightButton) {
        close();
      }
    }
    void mouseMoveEvent(QMouseEvent *e) override {
      if (dragging) {
        selection = QRect(pressPt, e->pos()).normalized();
        update();
      }
    }
    void mouseReleaseEvent(QMouseEvent *e) override {
      if (e->button() != Qt::LeftButton || !dragging) return;
      dragging = false;
      const QRect localSel = selection;
      hide();
      if (localSel.width() >= 5 && localSel.height() >= 5 && onSelected) {
        const QRect global(mapToGlobal(localSel.topLeft()), localSel.size());
        onSelected(global);
      }
      close();
    }
    void keyPressEvent(QKeyEvent *e) override {
      if (e->key() == Qt::Key_Escape) close();
    }
  };

  QToolButton *snipBtn = new QToolButton();
  snipBtn->setIcon(cameraIcon);
  snipBtn->setAutoRaise(true);
  snipBtn->setIconSize(QSize(20, 20));
  snipBtn->setToolTip("Snip a screen region — drag to select, the result is "
                      "saved to /tmp/carla_studio_v…/<date>/snippets/ and "
                      "copied to the clipboard.");
  snipBtn->setFixedSize(28, 28);

  QLabel *docsLink = new QLabel(QStringLiteral(
    "<a href='https://carla.readthedocs.io/en/latest/tuto_content_authoring_vehicles/'"
    " style='color:#9BA3B5;'>guide</a>"));
  docsLink->setOpenExternalLinks(true);
  docsLink->setAlignment(Qt::AlignCenter);
  docsLink->setStyleSheet("font-size: 10px;");
  docsLink->setMinimumWidth(28);
  docsLink->setToolTip("Content authoring guide — supported formats and conversion tips.");

  QWidget *cornerBox = new QWidget();
  QVBoxLayout *cornerLayout = new QVBoxLayout(cornerBox);
  cornerLayout->setContentsMargins(0, 1, 4, 1);
  cornerLayout->setSpacing(0);
  cornerLayout->addStretch();
  cornerLayout->addWidget(snipBtn,  0, Qt::AlignHCenter);
  cornerLayout->addWidget(docsLink, 0, Qt::AlignHCenter);
  cornerLayout->addStretch();
  tabs->setCornerWidget(cornerBox, Qt::TopRightCorner);

  QObject::connect(snipBtn, &QToolButton::clicked, &window,
                    [&, versionStr = QString(CARLA_STUDIO_VERSION_STRING),
                      cameraIcon]() {
    snipBtn->setEnabled(false);
    snipBtn->setIcon(QIcon());
    snipBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    snipBtn->setText("3");
    snipBtn->setToolTip("Capturing in 3…");
    auto remaining = std::make_shared<int>(3);
    auto *countdownTimer = new QTimer(&window);
    countdownTimer->setInterval(1000);
    QObject::connect(countdownTimer, &QTimer::timeout, &window,
                      [remaining, snipBtn, countdownTimer, cameraIcon]() {
      *remaining -= 1;
      if (*remaining > 0) {
        snipBtn->setText(QString::number(*remaining));
        snipBtn->setToolTip(QString("Capturing in %1…").arg(*remaining));
      } else {
        countdownTimer->stop();
        countdownTimer->deleteLater();
        snipBtn->setText(QString());
        snipBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
        snipBtn->setIcon(cameraIcon);
        snipBtn->setEnabled(true);
        snipBtn->setToolTip("Snip a screen region — drag to select, the "
                            "result is saved + copied to the clipboard.");
      }
    });
    countdownTimer->start();
    QTimer::singleShot(3000, &window, [&, versionStr]() {
    const QRect virt = QApplication::primaryScreen()->virtualGeometry();
    QPixmap fullShot = QApplication::primaryScreen()->grabWindow(
        0, virt.x(), virt.y(), virt.width(), virt.height());

    auto *ov = new SnippingOverlay();
    ov->capturedDesktop = fullShot;
    ov->setGeometry(virt);
    ov->onSelected = [&, versionStr, fullShot, virt](const QRect &globalRect) {
      const QRect localInShot = globalRect.translated(-virt.topLeft());
      QPixmap shot = fullShot.copy(localInShot);
      if (shot.isNull()) {
        QMessageBox::warning(&window, "Snip failed",
                              "Could not capture the selected region.");
        return;
      }

      const QString dirSuffix = QString(versionStr).replace('.', '-');
      const QString date = QDate::currentDate().toString("yyyy-MM-dd");
      const QString outDir = QString("/tmp/carla_studio_v%1/%2/snippets")
                               .arg(dirSuffix, date);
      QDir().mkpath(outDir);
      const QString stamp = QDateTime::currentDateTime().toString("HH-mm-ss");
      const QString outPath = outDir + "/snippet-" + stamp + ".png";
      const bool savedOk = shot.save(outPath, "PNG");
      if (savedOk) {
        QApplication::clipboard()->setPixmap(shot);
      }

      QMessageBox box(&window);
      box.setIcon(QMessageBox::Information);
      box.setWindowTitle("Snippet captured");
      box.setText(savedOk
        ? QString("Saved to:\n%1\n\n(also copied to clipboard)").arg(outPath)
        : QString("Captured to clipboard, but couldn't write %1").arg(outPath));
      box.setStandardButtons(QMessageBox::Ok);
      box.exec();
    };
    ov->show();
    ov->raise();
    ov->activateWindow();
    });
  });

  {
#ifndef CARLA_STUDIO_VERSION_STRING
#define CARLA_STUDIO_VERSION_STRING "0.0.0-untagged"
#endif
    QLabel *versionLabel = new QLabel(
      QStringLiteral("v") + QStringLiteral(CARLA_STUDIO_VERSION_STRING));
    versionLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    versionLabel->setContextMenuPolicy(Qt::PreventContextMenu);
    versionLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    versionLabel->setStyleSheet(
      "QLabel { color: #888; padding: 4px 10px 4px 10px; font-size: 11px; }"
      "QLabel:hover { color: #444; }");
    QString changelog;
    {
      QFile cf(":/meta/CHANGELOG.md");
      if (cf.open(QIODevice::ReadOnly)) {
        changelog = QString::fromUtf8(cf.readAll());
      }
    }
    if (changelog.isEmpty()) {
      changelog = QStringLiteral(
        "CARLA Studio v") + QStringLiteral(CARLA_STUDIO_VERSION_STRING)
        + QStringLiteral("\n\n(changelog resource missing)");
    }
    if (changelog.size() > 6000) {
      changelog = changelog.left(5950)
                + QStringLiteral("\n\n…(see Apps/CarlaStudio/CHANGELOG.md for full history)");
    }
    versionLabel->setToolTip(
      "<pre>" + changelog.toHtmlEscaped() + "</pre>"
      "<br><i>Left-click: capture debug bundle (logs + screenshots) to "
      "/tmp/carla_studio_v…/&lt;date&gt;/<br>"
      "Right-click: open pre-filled GitHub issue on carla-simulator/carla</i>");
    versionLabel->setCursor(Qt::PointingHandCursor);
    window.menuBar()->setCornerWidget(versionLabel, Qt::TopRightCorner);

    class VersionLabelClickFilter : public QObject {
     public:
      std::function<void()> onLeftClick;
      std::function<void()> onRightClick;
      bool eventFilter(QObject *, QEvent *e) override {
        if (e->type() == QEvent::MouseButtonPress) {
          auto *me = static_cast<QMouseEvent *>(e);
          if (me->button() == Qt::LeftButton || me->button() == Qt::RightButton) {
            return true;
          }
        }
        if (e->type() == QEvent::MouseButtonRelease) {
          auto *me = static_cast<QMouseEvent *>(e);
          if (me->button() == Qt::LeftButton  && onLeftClick)  onLeftClick();
          else if (me->button() == Qt::RightButton && onRightClick) onRightClick();
          return true;
        }
        if (e->type() == QEvent::ContextMenu) {
          if (onRightClick) onRightClick();
          return true;
        }
        return false;
      }
    };

    auto captureDebugBundle = [&, versionStr = QString(CARLA_STUDIO_VERSION_STRING)]() {
      const QString dirSuffix = QString(versionStr).replace('.', '-');
      const QString date = QDate::currentDate().toString("yyyy-MM-dd");
      const QString outDir = QString("/tmp/carla_studio_v%1/%2").arg(dirSuffix, date);
      if (!QDir().mkpath(outDir)) {
        QMessageBox::warning(&window, "Capture failed",
                             "Couldn't create " + outDir);
        return;
      }
      const int prevTab = tabs->currentIndex();
      QStringList captured;
      for (int i = 0; i < tabs->count(); ++i) {
        tabs->setCurrentIndex(i);
        QApplication::processEvents();
        QString name = tabs->tabToolTip(i).isEmpty() ? tabs->tabText(i)
                                                      : tabs->tabToolTip(i);
        name.replace(QRegularExpression("[^A-Za-z0-9_]+"), "_");
        if (name.isEmpty()) name = QString("tab_%1").arg(i);
        const QString shotPath = QString("%1/tab-%2-%3.png").arg(outDir, QString::number(i), name);
        if (window.grab().save(shotPath, "PNG")) captured.append(shotPath);
      }
      tabs->setCurrentIndex(prevTab);
      QApplication::processEvents();
      const QString fullPath = outDir + "/window-full.png";
      if (window.grab().save(fullPath, "PNG")) captured.append(fullPath);

      QFile hi(outDir + "/host_info.txt");
      if (hi.open(QIODevice::WriteOnly)) {
        QTextStream s(&hi);
        s << "CARLA Studio v" << versionStr << "\n";
        s << "Captured: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n\n";
        s << "[Qt]\n";
        s << "kernel: " << QSysInfo::kernelType() << " " << QSysInfo::kernelVersion() << "\n";
        s << "product: " << QSysInfo::prettyProductName() << "\n";
        s << "arch: " << QSysInfo::currentCpuArchitecture() << "\n\n";
        s << "[CARLA]\n";
        s << "CARLA_ROOT: " << carlaRootPath->text().trimmed() << "\n";
        s << "rootConfiguredOk: " << (rootConfiguredOk ? "yes" : "no") << "\n";
        s << "detectedEngineLabel: " << detectedEngineLabel << "\n";
        s << "g_reportedSimVersion: "
          << (g_reportedSimVersion.isEmpty() ? "(unknown)" : g_reportedSimVersion) << "\n";
        s << "g_sdkVersionMismatch: " << (g_sdkVersionMismatch ? "yes" : "no") << "\n\n";
        auto runOne = [&](const QString &heading, const QString &cmd) {
          QProcess p;
          p.start("/bin/bash", QStringList() << "-lc" << cmd);
          p.waitForFinished(2500);
          s << "[" << heading << "]\n"
            << QString::fromLocal8Bit(p.readAllStandardOutput()) << "\n";
        };
        runOne("uname",       "uname -a");
        runOne("os-release",  "cat /etc/os-release 2>/dev/null | head -8");
        runOne("nvidia-smi",  "nvidia-smi --query-gpu=name,driver_version,memory.total --format=csv 2>/dev/null");
        runOne("docker",      "docker --version 2>/dev/null; docker info 2>&1 | head -10");
        runOne("python3",     "python3 -V 2>&1; python3 -c 'import carla; print(\"carla\", carla.__version__)' 2>&1");
        runOne("ros",         "ros2 --version 2>&1; echo ROS_DISTRO=${ROS_DISTRO}");
      }
      if (libcarlaLogLinesPtr && !libcarlaLogLinesPtr->isEmpty()) {
        QFile rl(outDir + "/runtime.log");
        if (rl.open(QIODevice::WriteOnly)) {
          QTextStream(&rl) << libcarlaLogLinesPtr->join("\n");
        }
      }
      QMessageBox::information(&window, "Debug bundle captured",
        QString("Saved %1 file%2 to:\n%3")
          .arg(captured.size() + 1)
          .arg(captured.size() + 1 == 1 ? "" : "s")
          .arg(outDir));
    };

    auto openGitHubIssue = [&, versionStr = QString(CARLA_STUDIO_VERSION_STRING)]() {
      const QString dirSuffix = QString(versionStr).replace('.', '-');
      const QString today = QDate::currentDate().toString("yyyy-MM-dd");
      const QString carlaRoot = carlaRootPath->text().trimmed();
      const QString simVer = g_reportedSimVersion.isEmpty()
                               ? QStringLiteral("unknown")
                               : g_reportedSimVersion;
      const QString body = QString(
        "## CARLA Studio version\n"
        "v%1\n\n"
        "## Host\n"
        "- OS: %2\n"
        "- Arch: %3\n"
        "- Kernel: %4 %5\n\n"
        "## CARLA install\n"
        "- CARLA_ROOT: `%6`\n"
        "- Configured: %7\n"
        "- Engine: %8\n"
        "- Detected sim version: %9\n\n"
        "## Steps to reproduce\n"
        "1. \n2. \n3. \n\n"
        "## Expected behavior\n\n"
        "## Actual behavior\n\n"
        "## Logs / screenshots\n"
        "Attach files from `/tmp/carla_studio_v%10/%11/` "
        "(captured by **left-clicking the version label** in the menu bar).\n")
        .arg(versionStr,
             QSysInfo::prettyProductName(),
             QSysInfo::currentCpuArchitecture(),
             QSysInfo::kernelType(), QSysInfo::kernelVersion(),
             carlaRoot.isEmpty() ? QStringLiteral("(not set)") : carlaRoot,
             rootConfiguredOk ? QStringLiteral("yes") : QStringLiteral("no"),
             detectedEngineLabel,
             simVer,
             dirSuffix, today);

      QUrl url(QStringLiteral("https://github.com/carla-simulator/carla/issues/new"));
      QUrlQuery q;
      q.addQueryItem("title", QString("[Studio v%1] ").arg(versionStr));
      q.addQueryItem("body", body);
      q.addQueryItem("labels", "studio");
      url.setQuery(q);
      QDesktopServices::openUrl(url);
    };

    auto *clickFilter = new VersionLabelClickFilter();
    clickFilter->setParent(versionLabel);
    clickFilter->onLeftClick  = captureDebugBundle;
    clickFilter->onRightClick = openGitHubIssue;
    versionLabel->installEventFilter(clickFilter);
  }

  QAction *toggleHealthCheckAction = nullptr;
  QAction *toggleScenarioBuilderAction = nullptr;
  QAction *toggleLoggingAction = nullptr;

  QMenu *toolsMenu = window.menuBar()->addMenu("Menu");
  toggleScenarioBuilderAction = toolsMenu->addAction("Scenario Builder");
  toggleScenarioBuilderAction->setCheckable(true);
  toggleScenarioBuilderAction->setChecked(false);
  QAction *toggleScenarioReconstructionAction = toolsMenu->addAction("Scenario Re-Construction");
  toggleScenarioReconstructionAction->setCheckable(true);
  toggleScenarioReconstructionAction->setChecked(false);
  QAction *toggleLeaderboardAction = toolsMenu->addAction("Leaderboard");
  toggleLeaderboardAction->setCheckable(true);
  toggleLeaderboardAction->setChecked(false);
  QAction *toggleTestingAction = new QAction("Testing", &window);
  toggleTestingAction->setCheckable(true);
  toggleTestingAction->setChecked(false);
  toggleLoggingAction = new QAction("Logging", &window);
  toggleLoggingAction->setCheckable(true);
  toggleLoggingAction->setChecked(false);
  toolsMenu->addSeparator();
  QAction *quitAction = toolsMenu->addAction("Quit");
  quitAction->setShortcut(QKeySequence::Quit);
  quitAction->setMenuRole(QAction::QuitRole);
  QObject::connect(quitAction, &QAction::triggered, &window, [&]() {
    window.close();
    QCoreApplication::quit();
  });

  QMenu *cfgMenu = window.menuBar()->addMenu("Cfg");

  QMenu *themeMenu = cfgMenu->addMenu("Theme");
  auto *themeGroup = new QActionGroup(&window);
  themeGroup->setExclusive(true);
  for (const ThemeDef &t : kThemeCatalog) {
    QAction *a = themeMenu->addAction(QString::fromUtf8(t.label));
    a->setCheckable(true);
    a->setData(QString::fromLatin1(t.key));
    a->setChecked(t.id == currentTheme);
    themeGroup->addAction(a);
    QObject::connect(a, &QAction::triggered, &window, [&, a]() {
      const QString key = a->data().toString();
      const ThemeDef &def = themeByKey(key);
      currentTheme = def.id;
      applyStudioTheme(app, currentTheme);
      darkMode = lastAppliedIsDark();
      QSettings().setValue(QStringLiteral("view/theme"), QString::fromLatin1(def.key));
    });
  }
  cfgMenu->addSeparator();

  QMenu *toolsTopMenu = window.menuBar()->addMenu("Tools");
  QMenu *rosToolsMenu = toolsTopMenu->addMenu("ROS Tools");
  rosToolsMenu->menuAction()->setVisible(false);
  auto launchRosTool = [&](const QString &cmd, const QString &friendlyName) {
    const QString sourced =
      "{ "
        "if [ -n \"$CARLA_STUDIO_ROS_PATH\" ] && "
              "[ -f \"$CARLA_STUDIO_ROS_PATH/setup.bash\" ]; then "
          "source \"$CARLA_STUDIO_ROS_PATH/setup.bash\" 2>/dev/null && "
            "echo \"[carla-studio] sourced $CARLA_STUDIO_ROS_PATH/setup.bash\"; "
        "elif [ -n \"$ROS_DISTRO\" ] && "
              "[ -f \"/opt/ros/$ROS_DISTRO/setup.bash\" ]; then "
          "source \"/opt/ros/$ROS_DISTRO/setup.bash\" 2>/dev/null && "
            "echo \"[carla-studio] sourced /opt/ros/$ROS_DISTRO/setup.bash\"; "
        "else "
          "for f in /opt/ros/*/setup.bash; do "
            "[ -f \"$f\" ] && source \"$f\" 2>/dev/null && "
              "echo \"[carla-studio] sourced $f\" && break; "
          "done; "
        "fi; "
      "} >>/tmp/carla_studio_ros.log 2>&1; "
      "exec " + cmd + " >>/tmp/carla_studio_ros.log 2>&1";
    if (!QProcess::startDetached("/bin/bash", QStringList() << "-lc" << sourced)) {
      QMessageBox::warning(&window, "Launch failed",
        QString("Couldn't start %1. See /tmp/carla_studio_ros.log").arg(friendlyName));
    }
  };
  QAction *rosRvizAct       = rosToolsMenu->addAction("RViz (3D visualisation)");
  QAction *rosRqtAct        = rosToolsMenu->addAction("rqt (plugin host)");
  QAction *rosRqtGraphAct   = rosToolsMenu->addAction("rqt_graph (node graph)");
  QAction *rosRqtConsoleAct = rosToolsMenu->addAction("rqt_console (log viewer)");
  QAction *rosRqtPlotAct    = rosToolsMenu->addAction("rqt_plot (real-time plot)");
  QAction *rosRqtTopicAct   = rosToolsMenu->addAction("rqt_topic (topic monitor)");
  rosToolsMenu->addSeparator();
  QAction *rosTopicListAct  = rosToolsMenu->addAction("List active topics…");
  QObject::connect(rosRvizAct,       &QAction::triggered, &window,
    [launchRosTool]() { launchRosTool("rviz2 || rviz", "RViz"); });
  QObject::connect(rosRqtAct,        &QAction::triggered, &window,
    [launchRosTool]() { launchRosTool("rqt", "rqt"); });
  QObject::connect(rosRqtGraphAct,   &QAction::triggered, &window,
    [launchRosTool]() { launchRosTool("rqt_graph || rqt --standalone rqt_graph",
                                        "rqt_graph"); });
  QObject::connect(rosRqtConsoleAct, &QAction::triggered, &window,
    [launchRosTool]() { launchRosTool("rqt_console || rqt --standalone rqt_console",
                                        "rqt_console"); });
  QObject::connect(rosRqtPlotAct,    &QAction::triggered, &window,
    [launchRosTool]() { launchRosTool("rqt_plot || rqt --standalone rqt_plot",
                                        "rqt_plot"); });
  QObject::connect(rosRqtTopicAct,   &QAction::triggered, &window,
    [launchRosTool]() { launchRosTool("rqt_topic || rqt --standalone rqt_topic",
                                        "rqt_topic"); });
  QObject::connect(rosTopicListAct,  &QAction::triggered, &window, [&]() {
    QProcess p;
    p.start("/bin/bash", QStringList() << "-lc"
      << "if [ -n \"$CARLA_STUDIO_ROS_PATH\" ] && [ -f \"$CARLA_STUDIO_ROS_PATH/setup.bash\" ]; then "
           "source \"$CARLA_STUDIO_ROS_PATH/setup.bash\" 2>/dev/null; fi; "
         "timeout 3s ros2 topic list 2>&1");
    p.waitForFinished(4000);
    const QString out = QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    QMessageBox box(&window);
    box.setWindowTitle("ROS · active topics");
    box.setIcon(QMessageBox::Information);
    box.setText(out.isEmpty() ? QStringLiteral("(no output — daemon not running?)")
                              : out);
    box.exec();
  });
  g_setRosToolsMenuVisible = [rosToolsMenu, toolsTopMenu](bool v) {
    rosToolsMenu->menuAction()->setVisible(v);
    bool anyVisible = false;
    for (QAction *a : toolsTopMenu->actions()) {
      if (a->isVisible() && !a->isSeparator()) { anyVisible = true; break; }
    }
    toolsTopMenu->menuAction()->setVisible(anyVisible);
  };

  toolsTopMenu->addSeparator();
  QAction *gazeboAct = toolsTopMenu->addAction("Gazebo");
  gazeboAct->setToolTip(
    "Open-source robotics simulator (gazebosim.org). Launches the "
    "<code>gz sim</code> harmonic command, falling back to "
    "<code>gazebo</code> for the legacy series.");
  {
    const bool gzNew = !QStandardPaths::findExecutable("gz").isEmpty();
    const bool gzOld = !QStandardPaths::findExecutable("gazebo").isEmpty();
    gazeboAct->setVisible(gzNew || gzOld);
  }
  QObject::connect(gazeboAct, &QAction::triggered, &window,
    [launchRosTool]() {
      launchRosTool("gz sim || gazebo", "Gazebo");
    });

  QAction *foxgloveAct = toolsTopMenu->addAction("Foxglove Studio");
  foxgloveAct->setToolTip(
    "Foxglove Studio (foxglove.dev) — visualise robotics data, MCAP / "
    "ROS bag playback, live topic plots. Pairs well with the ros-bridge "
    "or the CARLA Native --ros2 publishers.");
  {
    const bool fgKebab = !QStandardPaths::findExecutable("foxglove-studio").isEmpty();
    const bool fgUnder = !QStandardPaths::findExecutable("foxglove_studio").isEmpty();
    const bool fgFlatpak = !QStandardPaths::findExecutable("flatpak").isEmpty();
    foxgloveAct->setVisible(fgKebab || fgUnder || fgFlatpak);
  }
  QObject::connect(foxgloveAct, &QAction::triggered, &window,
    [launchRosTool]() {
      launchRosTool(
        "foxglove-studio || foxglove_studio || "
        "flatpak run dev.foxglove.studio",
        "Foxglove Studio");
    });

  auto makeMenuIcon = [](std::function<void(QPainter &)> draw) -> QIcon {
    QPixmap pm(16, 16);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(60, 60, 64), 1.2));
    p.setBrush(Qt::NoBrush);
    draw(p);
    return QIcon(pm);
  };

  toolsTopMenu->addSeparator();
  QAction *vehicleImportAct = toolsTopMenu->addAction("Vehicle Import");
  vehicleImportAct->setCheckable(true);
  vehicleImportAct->setChecked(false);
  vehicleImportAct->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawRect(2, 8, 12, 4);
    p.drawLine(4, 8, 5, 5);
    p.drawLine(5, 5, 10, 5);
    p.drawLine(10, 5, 12, 8);
    p.drawEllipse(4, 11, 3, 3);
    p.drawEllipse(10, 11, 3, 3);
  }));
  vehicleImportAct->setToolTip(
    "Import a 3D model (OBJ, glTF, GLB, DAE) as a driveable CARLA vehicle.\n"
    "Toggle to add/remove the Vehicle Import tab.");
  QObject::connect(vehicleImportAct, &QAction::toggled, &window,
    [setVehicleImportTabVisible](bool checked) {
      setVehicleImportTabVisible(checked);
    });

  QAction *toolsInstallSdk = toolsTopMenu->addAction("Install / Update CARLA…");
  toolsInstallSdk->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawEllipse(2, 2, 12, 12);
    for (int i = 0; i < 8; ++i) {
      const double a = i * 3.14159 / 4.0;
      p.drawLine(QPointF(8 + 5 * std::cos(a), 8 + 5 * std::sin(a)),
                 QPointF(8 + 7 * std::cos(a), 8 + 7 * std::sin(a)));
    }
  }));
  QObject::connect(toolsInstallSdk, &QAction::triggered, &window,
    [&]() { openInstallerDialog(InstallerMode::InstallSdk); });

  QAction *toolsBuildSrc = toolsTopMenu->addAction("Build CARLA from Source…");
  toolsBuildSrc->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawLine(3, 13, 7, 5);
    p.drawLine(13, 13, 9, 5);
    p.drawText(QRect(0,0,16,16), Qt::AlignCenter, "/");
  }));
  QObject::connect(toolsBuildSrc, &QAction::triggered, &window, [&]() {
    auto setRoot = [&](const QString &path) {
      if (carlaRootPath) carlaRootPath->setText(path);
    };
    auto *dlg = new carla_studio::setup_wizard::SetupWizardDialog(setRoot, 1, &window);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->show();
    dlg->raise();
  });

  QAction *toolsLoadMaps = toolsTopMenu->addAction("Load Additional Maps…");
  toolsLoadMaps->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawRect(2, 2, 5, 5);
    p.drawRect(9, 2, 5, 5);
    p.drawRect(2, 9, 5, 5);
    p.drawRect(9, 9, 5, 5);
  }));
  QObject::connect(toolsLoadMaps, &QAction::triggered, &window,
    [&]() { openInstallerDialog(InstallerMode::InstallAdditionalMaps); });

  QAction *toolsCommunityMaps = toolsTopMenu->addAction("Community Maps…");
  toolsCommunityMaps->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawEllipse(2, 2, 12, 12);
    p.drawLine(2, 8, 14, 8);
    p.drawLine(8, 2, 8, 14);
    p.drawArc(4, 2, 8, 12, 0, 5760);
  }));
  QObject::connect(toolsCommunityMaps, &QAction::triggered, &window,
    [&]() { openCommunityMapsDialog(); });

  QAction *toolsCleanup = toolsTopMenu->addAction("Cleanup / Uninstall…");
  toolsCleanup->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawLine(3, 4, 13, 4);
    p.drawRect(4, 4, 8, 10);
    p.drawLine(6, 2, 10, 2);
    p.drawLine(6, 2, 6, 4);
    p.drawLine(10, 2, 10, 4);
  }));
  QObject::connect(toolsCleanup, &QAction::triggered, &window,
    [&]() { openCleanupDialog(); });

  QAction *toolsDocker = toolsTopMenu->addAction("Docker Settings…");
  toolsDocker->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawRect(2, 8, 3, 3);
    p.drawRect(6, 8, 3, 3);
    p.drawRect(10, 8, 3, 3);
    p.drawRect(2, 4, 3, 3);
    p.drawRect(6, 4, 3, 3);
    p.drawArc(2, 11, 12, 4, 180 * 16, 180 * 16);
  }));
  QObject::connect(toolsDocker, &QAction::triggered, &window,
    [&]() { openDockerSettingsDialog(); });

  QAction *toolsThirdParty = toolsTopMenu->addAction("Third-Party Tools…");
  toolsThirdParty->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawLine(3, 13, 9, 7);
    p.drawEllipse(8, 2, 6, 6);
    p.drawLine(2, 14, 4, 12);
  }));
  if (openThirdParty) {
    QObject::connect(toolsThirdParty, &QAction::triggered, &window,
      [&]() { if (openThirdParty) openThirdParty->trigger(); });
  }

  {
    bool anyVisible = false;
    for (QAction *a : toolsTopMenu->actions()) {
      if (a->isVisible() && !a->isSeparator()) { anyVisible = true; break; }
    }
    toolsTopMenu->menuAction()->setVisible(anyVisible);
  }

  QMenu *helpMenu = window.menuBar()->addMenu("Help");
  QAction *helpDocs = helpMenu->addAction("CARLA Documentation");
  helpDocs->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawLine(8, 4, 8, 13);
    p.drawLine(2, 4, 8, 4);   p.drawLine(8, 4, 14, 4);
    p.drawLine(2, 13, 8, 13); p.drawLine(8, 13, 14, 13);
    p.drawLine(2, 4, 2, 13);  p.drawLine(14, 4, 14, 13);
    p.drawLine(3, 7, 7, 7);  p.drawLine(9, 7, 13, 7);
    p.drawLine(3, 10, 7, 10); p.drawLine(9, 10, 13, 10);
  }));
  QObject::connect(helpDocs, &QAction::triggered, &window, []() {
    QDesktopServices::openUrl(
      QUrl(QStringLiteral("https://carla.readthedocs.io/en/latest/")));
  });
  toggleHealthCheckAction = helpMenu->addAction(healthCheckIcon, "Health Check");
  toggleHealthCheckAction->setCheckable(true);
  toggleHealthCheckAction->setChecked(false);

  
  
  
  
  
  
  
  helpMenu->setToolTipsVisible(true);
  
  
  
  
  struct SysInfo {
    QString distro, kernel, host, user, cpu, gpu, ram;
  };
  auto gatherSystemInfo = []() -> SysInfo {
    auto runCmd = [](const QString &cmd) -> QString {
      QProcess p;
      p.start("/bin/bash", QStringList() << "-lc" << cmd);
      p.waitForFinished(1500);
      return QString::fromLocal8Bit(p.readAllStandardOutput()).trimmed();
    };
    auto readField = [](const QString &path, const QString &field) -> QString {
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
      while (!f.atEnd()) {
        const QString line = QString::fromLocal8Bit(f.readLine());
        const int colon = line.indexOf(QLatin1Char(':'));
        if (colon < 0) continue;
        if (line.left(colon).trimmed().compare(field, Qt::CaseInsensitive) == 0) {
          return line.mid(colon + 1).trimmed();
        }
      }
      return QString();
    };
    auto readKv = [](const QString &path, const QString &key) -> QString {
      QFile f(path);
      if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
      while (!f.atEnd()) {
        const QString line = QString::fromLocal8Bit(f.readLine()).trimmed();
        const int eq = line.indexOf(QLatin1Char('='));
        if (eq < 0) continue;
        if (line.left(eq) == key) {
          QString v = line.mid(eq + 1);
          if (v.startsWith('"') && v.endsWith('"')) v = v.mid(1, v.size() - 2);
          return v;
        }
      }
      return QString();
    };

    SysInfo s;
    s.distro = readKv("/etc/os-release", "PRETTY_NAME");
    if (s.distro.isEmpty()) s.distro = "Unknown distro";
    s.kernel = runCmd("uname -srm");
    s.host   = QSysInfo::machineHostName();
    s.user   = QString::fromLocal8Bit(qgetenv("USER"));

    
    const QString cpuModel = carla_studio_proc::cpuModelString();
    QString clockGhz;
    const double ghz = carla_studio_proc::cpuClockGHz();
    if (ghz > 0.0) clockGhz = QString::number(ghz, 'f', 2) + " GHz";
    const int logicalCount = carla_studio_proc::cpuLogicalCount();
    QStringList tags;
    if (logicalCount > 0) tags << QString::number(logicalCount) + " cores";
    if (!clockGhz.isEmpty()) tags << clockGhz;
    s.cpu = cpuModel.isEmpty() ? QString() : cpuModel;
    if (!tags.isEmpty()) {
      s.cpu = s.cpu.isEmpty() ? tags.join(", ")
                              : s.cpu + " (" + tags.join(", ") + ")";
    }
    if (s.cpu.isEmpty()) s.cpu = "n/a";

    
#if defined(Q_OS_MAC) || defined(__APPLE__)
    s.gpu = runCmd(
      "system_profiler SPDisplaysDataType 2>/dev/null "
      "| awk -F': *' '/Chipset Model:/ { name=$2 } "
      "                /VRAM \\(Total\\):/ { vram=$2 } "
      "                /Vendor:/ { vendor=$2 } "
      "                END { if (name) printf \"%s\", name; "
      "                      if (vendor) printf \" \\xc2\\xb7 %s\", vendor; "
      "                      if (vram) printf \" \\xc2\\xb7 %s\", vram }'");
#else
    const QString smi = runCmd(
      "nvidia-smi --query-gpu=name,driver_version,memory.total "
      "--format=csv,noheader,nounits 2>/dev/null | head -1");
    if (!smi.isEmpty()) {
      const QStringList parts = smi.split(',', Qt::SkipEmptyParts);
      if (parts.size() >= 3) {
        const QString name = parts[0].trimmed();
        const QString drv  = parts[1].trimmed();
        bool gok = false;
        const qint64 mib = parts[2].trimmed().toLongLong(&gok);
        s.gpu = name + " · driver " + drv;
        if (gok) s.gpu += " · " + QString::number(mib / 1024.0, 'f', 1) + " GB VRAM";
      } else {
        s.gpu = smi;
      }
    } else {
      s.gpu = runCmd("lspci 2>/dev/null | grep -E 'VGA|3D' | head -1 "
                     "| sed 's/^[^:]*: //'");
    }
#endif
    if (s.gpu.isEmpty()) s.gpu = "n/a";

    const qint64 mib = carla_studio_proc::systemTotalRamMiB();
    s.ram = (mib > 0)
              ? QString::number(mib / 1024.0, 'f', 1) + " GiB"
              : QStringLiteral("n/a");
    return s;
  };

  auto sysInfoToTooltip = [](const SysInfo &s) -> QString {
    return QStringLiteral(
      "Distro:  %1\n"
      "Kernel:  %2\n"
      "Host:    %3\n"
      "User:    %4\n"
      "CPU:     %5\n"
      "GPU:     %6\n"
      "RAM:     %7"
    ).arg(s.distro, s.kernel, s.host, s.user, s.cpu, s.gpu, s.ram);
  };

  QAction *helpSysInfo = helpMenu->addAction("System Info");
  helpSysInfo->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawEllipse(2, 2, 12, 12);
    p.drawLine(8, 6, 8, 12);
    p.drawPoint(8, 4);
  }));
  helpSysInfo->setToolTip(sysInfoToTooltip(gatherSystemInfo()));
  QObject::connect(helpMenu, &QMenu::aboutToShow, &window,
    [helpSysInfo, gatherSystemInfo, sysInfoToTooltip]() {
      helpSysInfo->setToolTip(sysInfoToTooltip(gatherSystemInfo()));
    });
  QObject::connect(helpSysInfo, &QAction::triggered, &window,
    [&window, gatherSystemInfo]() {
      const SysInfo s = gatherSystemInfo();
      QDialog *dlg = new QDialog(&window);
      dlg->setAttribute(Qt::WA_DeleteOnClose);
      dlg->setWindowTitle("CARLA Studio — System Info");
      dlg->resize(560, 320);
      QVBoxLayout *outer = new QVBoxLayout(dlg);
      outer->setContentsMargins(16, 14, 16, 12);
      outer->setSpacing(10);

      QLabel *title = new QLabel("<h2 style='margin:0'>System Info</h2>");
      title->setTextFormat(Qt::RichText);
      outer->addWidget(title);

      
      
      auto *frame = new QFrame();
      frame->setFrameShape(QFrame::StyledPanel);
      frame->setStyleSheet("QFrame { background: palette(base); border-radius: 6px; }");
      auto *grid = new QGridLayout(frame);
      grid->setContentsMargins(14, 12, 14, 12);
      grid->setHorizontalSpacing(18);
      grid->setVerticalSpacing(8);
      grid->setColumnStretch(1, 1);

      const QFont monoFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
      const QList<QPair<QString, QString>> rows = {
        {"Distro",  s.distro}, {"Kernel",  s.kernel},
        {"Host",    s.host},   {"User",    s.user},
        {"CPU",     s.cpu},    {"GPU",     s.gpu},
        {"RAM",     s.ram},
      };
      int r = 0;
      for (const auto &kv : rows) {
        auto *k = new QLabel("<b>" + kv.first + "</b>");
        k->setTextFormat(Qt::RichText);
        k->setStyleSheet("color: palette(text);");
        auto *v = new QLabel(kv.second);
        v->setFont(monoFont);
        v->setTextInteractionFlags(Qt::TextSelectableByMouse);
        v->setWordWrap(true);
        grid->addWidget(k, r, 0, Qt::AlignTop | Qt::AlignLeft);
        grid->addWidget(v, r, 1, Qt::AlignTop | Qt::AlignLeft);
        ++r;
      }
      outer->addWidget(frame, 1);

      auto *btnRow = new QHBoxLayout();
      btnRow->addStretch(1);
      auto *closeBtn = new QPushButton("Close");
      QObject::connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
      btnRow->addWidget(closeBtn);
      outer->addLayout(btnRow);
      dlg->show();
    });

  helpMenu->addSeparator();
  QAction *helpLicense = helpMenu->addAction("License");
  helpLicense->setIcon(makeMenuIcon([](QPainter &p) {
    p.drawRect(3, 2, 10, 12);
    p.drawLine(5, 5, 11, 5);
    p.drawLine(5, 8, 11, 8);
    p.drawLine(5, 11, 9, 11);
  }));
  QObject::connect(helpLicense, &QAction::triggered, &window, [&]() {
    auto loadResource = [](const char *path) -> QString {
      QFile f(QString::fromLatin1(path));
      if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
      return QString::fromUtf8(f.readAll());
    };
    QDialog *dlg = new QDialog(&window);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setWindowTitle("CARLA Studio — License");
    dlg->resize(760, 580);
    QVBoxLayout *lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(10, 10, 10, 10);

    QLabel *legalNotice = new QLabel(QString(
      "<small>Copyright © 2026 Abdul, Hashim · "
      "Distributed under the GNU Affero General Public License v3 or later · "
      "<b>This program comes with ABSOLUTELY NO WARRANTY.</b> · "
      "<a href=\"https://github.com/im-hashim\">Source</a> · "
      "<a href=\"https://www.gnu.org/licenses/agpl-3.0.html\">License full text</a>"
      "</small>"));
    legalNotice->setTextFormat(Qt::RichText);
    legalNotice->setOpenExternalLinks(true);
    legalNotice->setWordWrap(true);
    legalNotice->setStyleSheet("padding: 4px 6px; color: #444;");
    lay->addWidget(legalNotice);

    QTabWidget *licTabs = new QTabWidget();

    QPlainTextEdit *studioLic = new QPlainTextEdit();
    studioLic->setReadOnly(true);
    studioLic->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    const QString licText = loadResource(":/meta/LICENSE");
    studioLic->setPlainText(licText.isEmpty()
                              ? "LICENSE resource not found."
                              : licText);
    licTabs->addTab(studioLic, "CARLA Studio (AGPL-3.0)");

    QTextBrowser *third = new QTextBrowser();
    third->setReadOnly(true);
    third->setOpenExternalLinks(true);
    third->setSearchPaths(QStringList() << ":/meta");
    const QString tpText = loadResource(":/meta/THIRD_PARTY.md");
    if (tpText.isEmpty()) {
      third->setPlainText("THIRD_PARTY.md resource not found.");
    } else {
      third->setMarkdown(tpText);
    }
    QObject::connect(third, &QTextBrowser::sourceChanged, third,
      [third](const QUrl &) {
        Q_UNUSED(third);
      });
    licTabs->addTab(third, "Third-Party Notices");

    QPlainTextEdit *cite = new QPlainTextEdit();
    cite->setReadOnly(true);
    cite->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    const QString cffText = loadResource(":/meta/CITATION.cff");
    cite->setPlainText(cffText.isEmpty()
                         ? "CITATION.cff resource not found."
                         : cffText);
    licTabs->addTab(cite, "Citation");

    lay->addWidget(licTabs, 1);
    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::close);
    lay->addWidget(bb);
    dlg->show();
    dlg->raise();
  });

  cfgMenu->addAction(optRenderOffscreen);
  cfgMenu->addAction(optWindowSmall);
  cfgMenu->addAction(optQualityEpic);
  cfgMenu->addAction(optQualityLow);
  cfgMenu->addSeparator();
  cfgMenu->addAction(optPreferNvidia);
  cfgMenu->addAction(optNoSound);
  cfgMenu->addAction(optRos2);
  cfgMenu->addSeparator();
  cfgMenu->addAction(optDriverInApp);
  cfgMenu->addAction(optDriverPython);

  cfgMenu->addSeparator();
  QAction *openInstallerSdk   = cfgMenu->addAction("Install / Update CARLA…");
  openInstallerSdk->setToolTip(
    "Browse CARLA releases on GitHub and install the chosen tarball "
    "to a target directory; sets CARLA_ROOT to the result.");
  QAction *openInstallerMaps  = cfgMenu->addAction("Load Additional Maps…");
  openInstallerMaps->setToolTip(
    "Download the AdditionalMaps_X.Y.Z.tar.gz pack matching your "
    "installed CARLA and extract it over CARLA_ROOT.");
  QAction *openCommunityMaps  = cfgMenu->addAction("Community Maps…");
  openCommunityMaps->setToolTip(
    "Browse community-contributed map packs and pull the ones you want.");
  QAction *openCleanup        = cfgMenu->addAction("Cleanup / Uninstall…");
  openCleanup->setToolTip(
    "Detect installed CARLA bits + caches; preview before deleting. "
    "Two-step (Detect → Confirm).");
  QAction *openDockerSettings = cfgMenu->addAction("Docker Settings…");
  openDockerSettings->setToolTip(
    "Choose carlasim/carla image tag, GPU flags, and bind mounts for "
    "Docker mode launches.");

  QAction *openRosBridge = cfgMenu->addAction("ros-bridge Pseudo-Sensors…");
  openRosBridge->setToolTip(
    "Toggle which ros-bridge pseudo-sensors (odometry, speedometer, map, "
    "object list, markers, traffic lights, actor list) get emitted at "
    "launch. Persists to QSettings(\"ros/pseudo/<id>\").");
  QObject::connect(openRosBridge, &QAction::triggered, &window,
    [&, rosPseudoCategory]() {
      QDialog *dlg = new QDialog(&window);
      dlg->setAttribute(Qt::WA_DeleteOnClose);
      dlg->setModal(false);
      dlg->setWindowTitle("CARLA Studio — ros-bridge Pseudo-Sensors");
      dlg->resize(440, 280);
      QVBoxLayout *l = new QVBoxLayout(dlg);
      l->setContentsMargins(12, 12, 12, 12);
      QLabel *intro = new QLabel(
        "<small>Synthesised by the <a "
        "href=\"https://github.com/carla-simulator/ros-bridge\">ros-bridge</a> "
        "package — not CARLA blueprints. Toggles persist regardless of "
        "whether the bridge is currently running.</small>");
      intro->setTextFormat(Qt::RichText);
      intro->setOpenExternalLinks(true);
      intro->setWordWrap(true);
      l->addWidget(intro);
      rosPseudoCategory->setParent(dlg);
      rosPseudoCategory->setVisible(true);
      l->addWidget(rosPseudoCategory, 1);
      QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
      QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::close);
      l->addWidget(bb);
      QObject::connect(dlg, &QDialog::destroyed, rosPseudoCategory, [rosPseudoCategory]() {
        if (rosPseudoCategory) rosPseudoCategory->setVisible(false);
      });
      dlg->show();
      dlg->raise();
    });
  QObject::connect(openDockerSettings, &QAction::triggered, &window,
    [&]() { openDockerSettingsDialog(); });
  QObject::connect(openInstallerSdk, &QAction::triggered, &window,
    [&]() { openInstallerDialog(InstallerMode::InstallSdk); });
  QObject::connect(openInstallerMaps, &QAction::triggered, &window,
    [&]() { openInstallerDialog(InstallerMode::InstallAdditionalMaps); });
  QObject::connect(openCommunityMaps, &QAction::triggered, &window,
    [&]() { openCommunityMapsDialog(); });
  QObject::connect(openCleanup, &QAction::triggered, &window,
    [&]() { openCleanupDialog(); });

  cfgMenu->addSeparator();
  QAction *openHfAuth     = cfgMenu->addAction("Hugging Face Account…");
  QAction *openHfDatasets = cfgMenu->addAction("HF Datasets…");
  QAction *openHfModels   = cfgMenu->addAction("HF Models…");
  cfgMenu->addSeparator();
  openThirdParty          = cfgMenu->addAction("Third-Party Tools…");
  openComputeHpc          = cfgMenu->addAction("Compute (HPC) Settings…");
  QAction *openUvEnvs     = cfgMenu->addAction("Python Environments (uv)…");

  auto openHfAuthDialog = [&]() {
    QDialog *dlg = new QDialog(&window);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setWindowTitle("CARLA Studio — Hugging Face Account");
    dlg->resize(540, 360);
    QVBoxLayout *lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(14, 14, 14, 14);

    QLabel *intro = new QLabel(
      "Studio reads HF_TOKEN from one place: this field. The value is "
      "persisted (obfuscated) in CARLA Studio's settings and exported as "
      "<code>HF_TOKEN</code> when launching downstream tools "
      "(<code>huggingface-cli</code>, <code>git-lfs</code>, NVIDIA "
      "containers).");
    intro->setWordWrap(true);
    intro->setTextFormat(Qt::RichText);
    lay->addWidget(intro);

    QHBoxLayout *getRow = new QHBoxLayout();
    QLabel *getLbl = new QLabel(
      "<a href=\"https://huggingface.co/settings/tokens\">Get a token on "
      "huggingface.co/settings/tokens →</a>");
    getLbl->setTextFormat(Qt::RichText);
    getLbl->setOpenExternalLinks(true);
    getRow->addWidget(getLbl);
    getRow->addStretch();
    lay->addLayout(getRow);

    QFormLayout *form = new QFormLayout();
    form->setLabelAlignment(Qt::AlignRight);
    QLineEdit *tokenEdit = new QLineEdit();
    tokenEdit->setEchoMode(QLineEdit::Password);
    tokenEdit->setPlaceholderText("hf_… (paste your token)");
    tokenEdit->setText(carla_studio::integrations::hf::loadToken());

    QToolButton *eye = new QToolButton();
    eye->setText("👁");
    eye->setCheckable(true);
    eye->setToolTip("Show / hide token");
    QObject::connect(eye, &QToolButton::toggled, tokenEdit, [tokenEdit](bool on) {
      tokenEdit->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
    });
    QHBoxLayout *tokenRow = new QHBoxLayout();
    tokenRow->setContentsMargins(0, 0, 0, 0);
    tokenRow->addWidget(tokenEdit, 1);
    tokenRow->addWidget(eye);
    QWidget *tokenHost = new QWidget();
    tokenHost->setLayout(tokenRow);
    form->addRow("Token:", tokenHost);
    lay->addLayout(form);

    QLabel *statusLbl = new QLabel();
    statusLbl->setTextFormat(Qt::RichText);
    statusLbl->setWordWrap(true);
    if (carla_studio::integrations::hf::hasToken()) {
      statusLbl->setText("<i>Token is saved. Click <b>Verify</b> to confirm "
                         "it still works.</i>");
    } else {
      statusLbl->setText("<i>No token set yet.</i>");
    }
    lay->addWidget(statusLbl);
    lay->addStretch();

    QHBoxLayout *btnRow = new QHBoxLayout();
    QPushButton *verifyBtn = new QPushButton("Verify");
    QPushButton *saveBtn   = new QPushButton("Save");
    QPushButton *clearBtn  = new QPushButton("Sign out");
    QPushButton *closeBtn  = new QPushButton("Close");
    btnRow->addWidget(verifyBtn);
    btnRow->addWidget(saveBtn);
    btnRow->addWidget(clearBtn);
    btnRow->addStretch();
    btnRow->addWidget(closeBtn);
    lay->addLayout(btnRow);

    QObject::connect(verifyBtn, &QPushButton::clicked, dlg,
      [tokenEdit, statusLbl, verifyBtn, dlg]() {
        const QString token = tokenEdit->text().trimmed();
        if (token.isEmpty()) {
          statusLbl->setText("<span style='color:#b71c1c'>"
                             "Enter a token first.</span>");
          return;
        }
        verifyBtn->setEnabled(false);
        statusLbl->setText("<i>Contacting huggingface.co/api/whoami-v2…</i>");
        carla_studio::integrations::hf::verifyAsync(dlg, token,
          [statusLbl, verifyBtn](carla_studio::integrations::hf::AccountInfo info) {
            verifyBtn->setEnabled(true);
            if (info.ok) {
              QString orgList = info.orgs.isEmpty()
                  ? QStringLiteral("—")
                  : info.orgs.join(", ");
              statusLbl->setText(QString(
                "<span style='color:#1b5e20'>"
                "✅ Authenticated as <b>%1</b>%2"
                "</span><br>"
                "<small>Plan: %3 · Orgs: %4</small>")
                  .arg(info.user.toHtmlEscaped())
                  .arg(info.fullname.isEmpty()
                       ? QString()
                       : QStringLiteral(" (%1)").arg(info.fullname.toHtmlEscaped()))
                  .arg(info.plan.isEmpty() ? "—" : info.plan.toHtmlEscaped())
                  .arg(orgList.toHtmlEscaped()));
            } else if (info.httpStatus == 401) {
              statusLbl->setText(
                "<span style='color:#b71c1c'>"
                "✗ HTTP 401 — token rejected. Check that you copied the full "
                "token and that it has Read access.</span>");
            } else if (info.httpStatus == 0) {
              statusLbl->setText(QString(
                "<span style='color:#b71c1c'>✗ Network error. %1</span>")
                  .arg(info.rawError.toHtmlEscaped()));
            } else {
              statusLbl->setText(QString(
                "<span style='color:#b71c1c'>✗ HTTP %1 — %2</span>")
                  .arg(info.httpStatus)
                  .arg(info.rawError.left(240).toHtmlEscaped()));
            }
          });
      });

    QObject::connect(saveBtn, &QPushButton::clicked, dlg,
      [tokenEdit, statusLbl]() {
        const QString token = tokenEdit->text().trimmed();
        carla_studio::integrations::hf::saveToken(token);
        if (token.isEmpty()) {
          statusLbl->setText("<i>Token cleared.</i>");
        } else {
          statusLbl->setText("<i>Saved. Studio will now export "
                             "<code>HF_TOKEN</code> to downstream tools.</i>");
        }
      });

    QObject::connect(clearBtn, &QPushButton::clicked, dlg,
      [tokenEdit, statusLbl]() {
        carla_studio::integrations::hf::clearToken();
        tokenEdit->clear();
        statusLbl->setText("<i>Signed out. Token removed from Studio.</i>");
      });

    QObject::connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::close);

    dlg->show();
    dlg->raise();
  };

  auto openStubDialog = [&](const QString &title, const QString &intent) {
    QDialog *dlg = new QDialog(&window);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setWindowTitle(QString("CARLA Studio — %1").arg(title));
    dlg->resize(440, 220);
    QVBoxLayout *l = new QVBoxLayout(dlg);
    l->setContentsMargins(14, 14, 14, 14);
    QLabel *t = new QLabel(QString("<h3>%1</h3>").arg(title));
    t->setTextFormat(Qt::RichText);
    l->addWidget(t);
    QLabel *body = new QLabel(intent);
    body->setWordWrap(true);
    body->setTextFormat(Qt::RichText);
    l->addWidget(body);
    l->addStretch();
    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::close);
    l->addWidget(bb);
    dlg->show();
    dlg->raise();
  };

  QObject::connect(openHfAuth,     &QAction::triggered, &window, openHfAuthDialog);
  QObject::connect(openHfDatasets, &QAction::triggered, &window, [openStubDialog]() {
    openStubDialog("HF Datasets",
      "Browse and download Hugging Face datasets curated for the NVIDIA AV "
      "stack (Physical AI Autonomous Vehicles, NCore variant, NuRec variant) "
      "plus any free-form <code>org/dataset</code> slug. Includes a volume "
      "picker so large pulls land on the right SSD/HDD/NW share.");
  });
  QObject::connect(openHfModels,   &QAction::triggered, &window, [openStubDialog]() {
    openStubDialog("HF Models",
      "Curated NVIDIA model tiles (Alpamayo R1 10B, Lyra 2.0, Asset-Harvester) "
      "with revision pickers, hourly update checks, and gated-repo handling. "
      "Greyed out where local VRAM is insufficient — switch Compute → Cloud "
      "(Brev) to unlock.");
  });
  QObject::connect(openThirdParty, &QAction::triggered, &window,
    [&, refreshIntegrationsVisibility]() {
      QDialog *dlg = new QDialog(&window);
      dlg->setAttribute(Qt::WA_DeleteOnClose);
      dlg->setModal(false);
      dlg->setWindowTitle("CARLA Studio — Third-Party Tools");
      dlg->resize(540, 420);
      QVBoxLayout *l = new QVBoxLayout(dlg);
      l->setContentsMargins(12, 12, 12, 12);
      QLabel *intro = new QLabel(
        "Install / uninstall third-party integrations. Toggling here "
        "shows the matching row on the Home tab → <i>Integrations</i>.");
      intro->setWordWrap(true);
      intro->setTextFormat(Qt::RichText);
      l->addWidget(intro);

      auto addToolRow = [&](const QString &key, const QString &label,
                             const QString &blurb, const QString &repoUrl) {
        QGroupBox *card = new QGroupBox(label);
        QHBoxLayout *cardLay = new QHBoxLayout(card);
        QLabel *desc = new QLabel(QString(
          "<small>%1<br><a href=\"%2\">%2</a></small>").arg(blurb, repoUrl));
        desc->setWordWrap(true);
        desc->setTextFormat(Qt::RichText);
        desc->setOpenExternalLinks(true);
        cardLay->addWidget(desc, 1);
        QPushButton *toggleBtn = new QPushButton();
        toggleBtn->setFixedWidth(96);
        auto applyState = [toggleBtn, key]() {
          const bool on = QSettings().value(
            QString("addon/%1_installed").arg(key), false).toBool();
          toggleBtn->setText(on ? "Uninstall" : "Install");
        };
        applyState();
        QObject::connect(toggleBtn, &QPushButton::clicked, dlg,
          [key, applyState, refreshIntegrationsVisibility]() {
            const QString flag = QString("addon/%1_installed").arg(key);
            QSettings s;
            s.setValue(flag, !s.value(flag, false).toBool());
            applyState();
            refreshIntegrationsVisibility();
          });
        cardLay->addWidget(toggleBtn);
        l->addWidget(card);
      };
      addToolRow("terasim", "TeraSim",
        "Co-simulation traffic + scenario library. Bridges SUMO + CARLA.",
        "https://github.com/michigan-traffic-lab/TeraSim-Mcity-2.0");
      addToolRow("autoware", "Autoware",
        "End-to-end open-source autonomous driving stack.",
        "https://github.com/michigan-traffic-lab/autoware");
      addToolRow("alpasim", "AlpaSim",
        "NVIDIA's open-source AV simulation framework (Alpamayo stack).",
        "https://github.com/NVlabs/alpasim");
      addToolRow("nurec", "NuRec runtime image",
        "NVIDIA Neural Reconstruction renderer container (NGC).",
        "https://docs.nvidia.com/nurec/basics/hardware.html");
      addToolRow("ncore", "NCore",
        "Canonical multi-sensor data format for AV + robotics.",
        "https://github.com/NVIDIA/ncore");
      addToolRow("lyra2", "Lyra 2.0",
        "Generative 3D world models — explorable scenes from prompts.",
        "https://github.com/nv-tlabs/lyra");
      addToolRow("asset_harvester", "Asset-Harvester",
        "Lift sim-ready 3D assets out of real driving logs.",
        "https://github.com/NVIDIA/asset-harvester");

      l->addStretch();
      QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
      QObject::connect(bb, &QDialogButtonBox::rejected, dlg, &QDialog::close);
      l->addWidget(bb);
      dlg->show();
      dlg->raise();
    });
  QObject::connect(openComputeHpc, &QAction::triggered, &window, [openStubDialog]() {
    openStubDialog("Compute (HPC) Settings",
      "Choose where workloads run: Local / Remote LAN host / Cloud (Brev). "
      "Detected local resources surfaced inline; tools that don't fit the "
      "local box show their deficit and route the user to a viable backend.");
  });
  QObject::connect(openUvEnvs,     &QAction::triggered, &window, [openStubDialog]() {
    openStubDialog("Python Environments (uv)",
      "Per-repo isolated Python envs from each tool's <code>uv.lock</code>. "
      "Default is to match your local Python when the lock allows it; "
      "otherwise Studio creates a dedicated env under "
      "<code>~/.cache/carla_studio/uv-envs/&lt;tool&gt;</code>. Includes "
      "a Purge action to reclaim disk.");
  });

  cfgMenu->addSeparator();
  QAction *openSettingsDialog = cfgMenu->addAction("Remote / SSH Settings…");
  QDialog *settingsDialog = new QDialog(&window);
  settingsDialog->setWindowTitle("CARLA Studio — Remote / SSH Settings");
  settingsDialog->setModal(false);
  settingsDialog->resize(420, 540);
  {
    QVBoxLayout *dlgLayout = new QVBoxLayout(settingsDialog);
    dlgLayout->setContentsMargins(8, 8, 8, 8);
    dlgLayout->addWidget(w5, 1);
    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    QObject::connect(bb, &QDialogButtonBox::rejected, settingsDialog, &QDialog::hide);
    dlgLayout->addWidget(bb);
  }
  QObject::connect(openSettingsDialog, &QAction::triggered, &window, [settingsDialog]() {
    settingsDialog->show();
    settingsDialog->raise();
    settingsDialog->activateWindow();
  });

  QObject::connect(optDriverInApp, &QAction::toggled, &window, [&](bool checked) {
    if (checked && optDriverPython->isChecked()) {
      optDriverPython->setChecked(false);
    }
  });
  QObject::connect(optDriverPython, &QAction::toggled, &window, [&](bool checked) {
    if (checked && optDriverInApp->isChecked()) {
      optDriverInApp->setChecked(false);
    }
  });

  Q_UNUSED(interfacesTabIndex);
  Q_UNUSED(healthCheckTabIndex);
  Q_UNUSED(scenarioBuilderTabIndex);
  Q_UNUSED(loggingTabIndex);
  Q_UNUSED(homeTabIndex);

  bool healthCheckTabVisible = false;
  bool scenarioBuilderTabVisible = false;
  bool scenarioReconstructionTabVisible = false;
  bool leaderboardTabVisible = false;
  bool testingTabVisible = false;
  bool loggingTabVisible = false;

  const QIcon scenarioReconstructionIcon = makeTabIcon([](QPainter &p) {
    p.setRenderHint(QPainter::Antialiasing);
    QPen wirePen(QColor(80, 80, 80), 1.2);
    p.setPen(wirePen);
    QRectF front(3.5, 6.5, 8.0, 8.0);
    QPointF d(2.5, -2.5);
    p.drawRect(front);
    QRectF back = front.translated(d);
    p.drawRect(back);
    p.drawLine(front.topLeft(),     back.topLeft());
    p.drawLine(front.topRight(),    back.topRight());
    p.drawLine(front.bottomRight(), back.bottomRight());
    QPen arrPen(QColor(225, 110, 30), 1.3);
    p.setPen(arrPen);
    QRectF arc(0.5, 0.5, 5.0, 5.0);
    p.drawArc(arc, 30 * 16, 270 * 16);
    p.drawLine(QPointF(4.6, 0.6), QPointF(5.4, 1.6));
    p.drawLine(QPointF(4.6, 0.6), QPointF(3.5, 1.0));
  });

  auto rebuildTabs = [&]() {
    QWidget *currentWidget = tabs->currentWidget();
    tabs->clear();

    const int hi = tabs->addTab(w1, QStringLiteral("⌂"));
    tabs->setTabToolTip(hi, "Home");
    const int ii = tabs->addTab(w2, QStringLiteral("⇄"));
    tabs->setTabToolTip(ii, "Interfaces");
    if (healthCheckTabVisible) {
      const int hci = tabs->addTab(healthCheckPage, healthCheckIcon, QString());
      tabs->setTabToolTip(hci, "Health Check");
    }
    if (scenarioBuilderTabVisible) {
      const int sbi = tabs->addTab(w3, scenarioBuilderIcon, QString());
      tabs->setTabToolTip(sbi, "Scenario Builder");
    }
    if (scenarioReconstructionTabVisible) {
      const int sri = tabs->addTab(w6, scenarioReconstructionIcon, QString());
      tabs->setTabToolTip(sri, "Scenario Re-Construction");
    }
    if (leaderboardTabVisible) {
      const int lbi = tabs->addTab(w7, QStringLiteral("🏆"));
      tabs->setTabToolTip(lbi, "Leaderboard (roadmap)");
    }

    int restoreIndex = tabs->indexOf(currentWidget);
    if (restoreIndex < 0) {
      restoreIndex = 0;
    }
    tabs->setCurrentIndex(restoreIndex);
  };

  rebuildTabs();

  QObject::connect(toggleHealthCheckAction, &QAction::toggled, &window, [&](bool checked) {
    healthCheckTabVisible = checked;
    rebuildTabs();
  });
  QObject::connect(toggleScenarioBuilderAction, &QAction::toggled, &window, [&](bool checked) {
    scenarioBuilderTabVisible = checked;
    rebuildTabs();
  });
  QObject::connect(toggleScenarioReconstructionAction, &QAction::toggled, &window, [&](bool checked) {
    scenarioReconstructionTabVisible = checked;
    rebuildTabs();
  });
  QObject::connect(toggleLeaderboardAction, &QAction::toggled, &window, [&](bool checked) {
    leaderboardTabVisible = checked;
    rebuildTabs();
  });
  QObject::connect(toggleTestingAction, &QAction::toggled, &window, [&](bool checked) {
    testingTabVisible = checked;
    rebuildTabs();
  });
  QObject::connect(toggleLoggingAction, &QAction::toggled, &window, [&](bool checked) {
    loggingTabVisible = checked;
    rebuildTabs();
  });

  switchToScenarioBuilderTab = [&]() {
    if (!toggleScenarioBuilderAction->isChecked()) {
      toggleScenarioBuilderAction->setChecked(true);
    }
    const int idx = tabs->indexOf(w3);
    if (idx >= 0) tabs->setCurrentIndex(idx);
  };

  auto persistBuiltinSshData = [&]() {
    appSettings.setValue("remote/hosts", configuredRemoteHosts);
    appSettings.setValue("ssh/keys", managedSshKeys);
    appSettings.sync();
  };

  auto refreshBuiltinLists = [&]() {
    remoteHostsList->clear();
    if (configuredRemoteHosts.isEmpty()) {
      remoteHostsList->addItem("No remote hosts configured.");
    } else {
      for (const QString &host : configuredRemoteHosts) {
        remoteHostsList->addItem(host);
      }
    }

    sshKeysList->clear();
    if (managedSshKeys.isEmpty()) {
      sshKeysList->addItem("No SSH keys added.");
    } else {
      for (const QString &keyPath : managedSshKeys) {
        sshKeysList->addItem(keyPath);
      }
    }

    refreshRuntimeOptions();
  };

  QObject::connect(reloadSavedBtn, &QPushButton::clicked, &window, [&]() {
    configuredRemoteHosts = appSettings.value("remote/hosts").toStringList();
    managedSshKeys = appSettings.value("ssh/keys").toStringList();
    refreshBuiltinLists();
  });
  QObject::connect(addHostBtn, &QPushButton::clicked, &window, [&]() {
    const QString host = remoteHostInput->text().trimmed();
    if (host.isEmpty()) {
      return;
    }
    if (!configuredRemoteHosts.contains(host)) {
      configuredRemoteHosts.append(host);
      configuredRemoteHosts.sort(Qt::CaseInsensitive);
      persistBuiltinSshData();
      refreshBuiltinLists();
    }
    remoteHostInput->clear();
  });
  QObject::connect(removeHostBtn, &QPushButton::clicked, &window, [&]() {
    QListWidgetItem *item = remoteHostsList->currentItem();
    if (!item) {
      return;
    }
    const QString host = item->text().trimmed();
    if (configuredRemoteHosts.contains(host)) {
      configuredRemoteHosts.removeAll(host);
      persistBuiltinSshData();
      refreshBuiltinLists();
    }
  });
  QObject::connect(addKeyBtn, &QPushButton::clicked, &window, [&]() {
    const QString keyPath = sshKeyPath->text().trimmed();
    if (keyPath.isEmpty()) {
      return;
    }
    if (QFileInfo(keyPath).isFile()) {
      if (!managedSshKeys.contains(keyPath)) {
        managedSshKeys.append(keyPath);
        persistBuiltinSshData();
        refreshBuiltinLists();
      }
      sshKeyPath->clear();
    }
  });
  QObject::connect(removeKeyBtn, &QPushButton::clicked, &window, [&]() {
    QListWidgetItem *item = sshKeysList->currentItem();
    if (!item) {
      return;
    }
    const QString keyPath = item->text().trimmed();
    if (managedSshKeys.contains(keyPath)) {
      managedSshKeys.removeAll(keyPath);
      persistBuiltinSshData();
      refreshBuiltinLists();
    }
  });

  refreshBuiltinLists();

  window.setCentralWidget(tabs);
  window.setDockOptions(QMainWindow::AllowTabbedDocks |
                        QMainWindow::AllowNestedDocks |
                        QMainWindow::AnimatedDocks);

  using PluginFactory = QDockWidget *(*)(QWidget *);

  const QDir exeDir(QCoreApplication::applicationDirPath());
  pluginSearchDirs << exeDir.filePath("plugins")
                   << exeDir.filePath("../lib/carla-studio/plugins");

  const QByteArray envPath = qgetenv("CARLA_STUDIO_PLUGIN_PATH");
  if (!envPath.isEmpty()) {
#if defined(Q_OS_WIN)
    const QChar sep = QLatin1Char(';');
#else
    const QChar sep = QLatin1Char(':');
#endif
    for (const QString &p : QString::fromLocal8Bit(envPath).split(sep, Qt::SkipEmptyParts)) {
      pluginSearchDirs << p;
    }
  }

  QMenu *pluginsMenu = cfgMenu->addMenu("Plugins");
  QAction *rescanAction = pluginsMenu->addAction("Rescan plugin directories");
  pluginsMenu->addSeparator();

  auto loadPluginsFrom = [&](const QString &dirPath) {
    QDir dir(dirPath);
    if (!dir.exists()) {
      return;
    }
    const QStringList filters = {"qt-*.so", "qt-*.dylib", "qt-*.dll"};
    for (const QFileInfo &entry : dir.entryInfoList(filters, QDir::Files)) {
      QLibrary lib(entry.absoluteFilePath());
      if (!lib.load()) {
        qWarning("carla-studio: skipping plugin %s (%s)",
                 qPrintable(entry.absoluteFilePath()),
                 qPrintable(lib.errorString()));
        continue;
      }
      auto factory = reinterpret_cast<PluginFactory>(
        lib.resolve("carla_studio_plugin_create"));
      if (!factory) {
        lib.unload();
        continue;
      }
      QDockWidget *dock = factory(&window);
      if (!dock) {
        continue;
      }
      dock->setParent(&window);
      window.addDockWidget(Qt::RightDockWidgetArea, dock);
      QAction *toggle = pluginsMenu->addAction(dock->windowTitle().isEmpty()
                                                 ? entry.baseName()
                                                 : dock->windowTitle());
      toggle->setCheckable(true);
      toggle->setChecked(true);
      QObject::connect(toggle, &QAction::toggled, dock, &QDockWidget::setVisible);
      QObject::connect(dock, &QDockWidget::visibilityChanged, toggle, &QAction::setChecked);
      pluginsLoadedCount += 1;
    }
  };

  auto scanAll = [&]() {
    pluginsLoadedCount = 0;
    for (const QString &d : pluginSearchDirs) {
      loadPluginsFrom(d);
    }
  };
  scanAll();

  QObject::connect(rescanAction, &QAction::triggered, &window, [&]() {
    scanAll();
  });

  setSimReachable(launchRequestedOrRunning);
  g_disableSimButtonsOnMismatch = [&]() { setSimReachable(false); };

  window.show();
  return app.exec();
}
