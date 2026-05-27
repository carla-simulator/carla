// CARLA Studio community plugin ABI.
//
// A CARLA Studio plugin is a shared library (.so / .dylib / .dll) that
// exports a single C function:
//
//   extern "C" QDockWidget *carla_studio_plugin_create(QWidget *parent);
//
// At startup, CARLA Studio scans the directories listed in the
// CARLA_STUDIO_PLUGIN_PATH environment variable (colon- or
// semicolon-separated) plus the ./plugins directory next to the
// executable. For each shared library it finds, it resolves the symbol
// above and adds the returned QDockWidget into the main window's dock
// area. Ownership of the returned widget transfers to CARLA Studio.
//
// The same source file can also be compiled as a standalone executable
// so community tools can be launched on their own without CARLA Studio.
// See Examples/QtClient/main.cpp for a reference implementation.

#ifndef CARLA_STUDIO_PLUGIN_H
#define CARLA_STUDIO_PLUGIN_H

#include <QtGlobal>

class QDockWidget;
class QWidget;

#if defined(_WIN32)
#  define CARLA_STUDIO_PLUGIN_EXPORT __declspec(dllexport)
#else
#  define CARLA_STUDIO_PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
  typedef QDockWidget *(*CarlaStudioPluginFactory)(QWidget *parent);
}

#define CARLA_STUDIO_PLUGIN_FACTORY_SYMBOL "carla_studio_plugin_create"

#endif  // CARLA_STUDIO_PLUGIN_H
