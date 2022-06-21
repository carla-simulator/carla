// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <string>

class FToolBarBuilder;
class FMenuBuilder;

class FCarlaExporterModule : public IModuleInterface
{
public:

  /// IModuleInterface implementation
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;

  /// This function will be bound to Command.
  void PluginButtonClicked();

private:

  void AddMenuExtension(FMenuBuilder& Builder);

private:
  TSharedPtr<class FUICommandList> PluginCommands;
};
