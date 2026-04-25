// Example: CARLA Studio community tool — dockable + standalone.
//
// This file is a template for community-built Qt tool windows that can be:
//
//   1. Compiled as a standalone executable and launched on its own.
//   2. Compiled as a shared plugin (.so / .dylib / .dll) and loaded by
//      CARLA Studio, which docks the widget into its main window.
//
// To create your own tool, copy this directory, rename the class, and
// replace the contents of buildToolUi() with whatever you want to expose.
//
// The plugin ABI is a single exported C function, declared in
// include/CarlaStudioPlugin.h.

#include "CarlaStudioPlugin.h"

#include <QApplication>
#include <QCheckBox>
#include <QDockWidget>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <QDateTime>

namespace {

QWidget *buildToolUi(QDockWidget *owner) {
  QWidget *root = new QWidget(owner);
  QVBoxLayout *rootLayout = new QVBoxLayout(root);

  QLabel *title = new QLabel(QStringLiteral("Example community tool"), root);
  title->setStyleSheet(QStringLiteral("font-weight: 700; font-size: 14px;"));
  rootLayout->addWidget(title);

  QLabel *intro = new QLabel(
    QStringLiteral(
      "Replace this dock's contents with anything your tool exposes. "
      "This same widget is usable standalone or docked inside CARLA Studio."),
    root);
  intro->setWordWrap(true);
  rootLayout->addWidget(intro);

  QGroupBox *inputsGroup = new QGroupBox(QStringLiteral("Inputs"), root);
  QVBoxLayout *inputsLayout = new QVBoxLayout(inputsGroup);
  QLineEdit *label = new QLineEdit(inputsGroup);
  label->setPlaceholderText(QStringLiteral("Tag to stamp into the log"));
  inputsLayout->addWidget(label);
  QCheckBox *verbose = new QCheckBox(QStringLiteral("Verbose timestamps"), inputsGroup);
  inputsLayout->addWidget(verbose);
  rootLayout->addWidget(inputsGroup);

  QHBoxLayout *buttons = new QHBoxLayout();
  QPushButton *stamp = new QPushButton(QStringLiteral("Stamp log"), root);
  QPushButton *clear = new QPushButton(QStringLiteral("Clear"), root);
  buttons->addWidget(stamp);
  buttons->addWidget(clear);
  buttons->addStretch();
  rootLayout->addLayout(buttons);

  QPlainTextEdit *log = new QPlainTextEdit(root);
  log->setReadOnly(true);
  log->setPlaceholderText(QStringLiteral("Log output appears here."));
  rootLayout->addWidget(log, 1);

  QObject::connect(stamp, &QPushButton::clicked, log, [log, label, verbose]() {
    const QString tag = label->text().trimmed().isEmpty()
      ? QStringLiteral("event")
      : label->text().trimmed();
    const QString ts = verbose->isChecked()
      ? QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
      : QDateTime::currentDateTime().toString(Qt::ISODate);
    log->appendPlainText(QStringLiteral("[%1] %2").arg(ts, tag));
  });
  QObject::connect(clear, &QPushButton::clicked, log, &QPlainTextEdit::clear);

  return root;
}

QDockWidget *createExampleToolDock(QWidget *parent) {
  QDockWidget *dock = new QDockWidget(QStringLiteral("Example Tool"), parent);
  dock->setObjectName(QStringLiteral("CarlaStudio.ExampleCommunityTool"));
  dock->setFeatures(QDockWidget::DockWidgetMovable |
                    QDockWidget::DockWidgetFloatable |
                    QDockWidget::DockWidgetClosable);
  dock->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock->setWidget(buildToolUi(dock));
  return dock;
}

}  // namespace

// Plugin entry point. CARLA Studio resolves this symbol via QLibrary /
// dlopen after loading the shared library.
extern "C" CARLA_STUDIO_PLUGIN_EXPORT QDockWidget *
carla_studio_plugin_create(QWidget *parent) {
  return createExampleToolDock(parent);
}

#ifndef CARLA_STUDIO_BUILD_AS_PLUGIN
// Standalone build: host the same dock widget inside a minimal QMainWindow.
int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName(QStringLiteral("CARLA Studio Example Tool"));

  QMainWindow window;
  window.setWindowTitle(QStringLiteral("CARLA Studio Example Tool (standalone)"));
  window.resize(640, 480);
  window.setDockOptions(QMainWindow::AllowTabbedDocks |
                        QMainWindow::AllowNestedDocks |
                        QMainWindow::AnimatedDocks);

  QDockWidget *dock = createExampleToolDock(&window);
  window.addDockWidget(Qt::LeftDockWidgetArea, dock);

  window.show();
  return app.exec();
}
#endif  // CARLA_STUDIO_BUILD_AS_PLUGIN
