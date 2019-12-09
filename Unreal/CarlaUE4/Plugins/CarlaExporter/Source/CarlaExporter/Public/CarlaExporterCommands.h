// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FCarlaExporterCommands : public TCommands<FCarlaExporterCommands>
{
public:

  FCarlaExporterCommands()
    : TCommands<FCarlaExporterCommands>(
      TEXT("CarlaExporter"),
      NSLOCTEXT("Contexts", "CarlaExporter", "CarlaExporter Plugin"),
      NAME_None,
      FEditorStyle::GetStyleSetName())
  {
  }

  // TCommands<> interface
  virtual void RegisterCommands() override;

public:
  TSharedPtr< FUICommandInfo > PluginActionExportAll;
};
