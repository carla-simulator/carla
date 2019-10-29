// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CarlaExporterCommands.h"

#define LOCTEXT_NAMESPACE "FCarlaExporterModule"

void FCarlaExporterCommands::RegisterCommands()
{
	// UI_COMMAND(PluginAction, "CarlaExporter", "Execute CarlaExporter action", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginActionExportAsBlock, "Export as Blocks", "Exporter for Blocks", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginActionExportAsRoad, "Export as Roads", "Exporter for Roads", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginActionExportAsSide, "Export as Sidewalks", "Exporter for Sidewalks", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginActionExportAsCross, "Export as Cross zones", "Exporter for Cross zones", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
