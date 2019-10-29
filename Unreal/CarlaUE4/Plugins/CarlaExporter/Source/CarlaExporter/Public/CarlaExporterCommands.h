// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "CarlaExporterStyle.h"

class FCarlaExporterCommands : public TCommands<FCarlaExporterCommands>
{
public:

	FCarlaExporterCommands()
		: TCommands<FCarlaExporterCommands>(TEXT("CarlaExporter"), NSLOCTEXT("Contexts", "CarlaExporter", "CarlaExporter Plugin"), NAME_None, FCarlaExporterStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	// TSharedPtr< FUICommandInfo > PluginAction;
	TSharedPtr< FUICommandInfo > PluginActionExportAsBlock;
	TSharedPtr< FUICommandInfo > PluginActionExportAsRoad;
	TSharedPtr< FUICommandInfo > PluginActionExportAsSide;
	TSharedPtr< FUICommandInfo > PluginActionExportAsCross;
};
