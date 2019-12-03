// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CarlaExporterCommands.h"

#define LOCTEXT_NAMESPACE "FCarlaExporterModule"

void FCarlaExporterCommands::RegisterCommands()
{
	UI_COMMAND(PluginActionExportAll, "Carla Exporter", "Export all or selected meshes into an .OBJ file to be used by Carla (in /saved/ folder)", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
