// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarlaExporterCommands.h"

#define LOCTEXT_NAMESPACE "FCarlaExporterModule"

void FCarlaExporterCommands::RegisterCommands()
{
  UI_COMMAND(PluginActionExportAll, "Carla Exporter", "Export all or selected meshes into an .OBJ file to be used by Carla (in /saved/ folder)", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
