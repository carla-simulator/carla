// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CarlaExporterCommands.h"

#define LOCTEXT_NAMESPACE "FCarlaExporterModule"

void FCarlaExporterCommands::RegisterCommands()
{
	// UI_COMMAND(PluginAction, "CarlaExporter", "Execute CarlaExporter action", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginActionExportAsBlock, "Export as Blocks", "Export for Blocks", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginActionExportAsRoad, "Export as Roads", "Export for Roads", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginActionExportAsSide, "Export as Sidewalks", "Export for Sidewalks", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginActionExportAsCross, "Export as Cross zones", "Export for Cross zones", EUserInterfaceActionType::Button, FInputGesture());
	UI_COMMAND(PluginActionExportAll, "Export by Nomenclature", "Export by Nomenclature", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
