// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.
//
// Compatibility shim for carla-digitaltwins' UGenerationPathsHelper (CarlaMeshGeneration
// module, redirected here via CoreRedirects).

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include <util/ue-header-guard-end.h>

#include "GenerationPathsHelper.generated.h"

UCLASS()
class CARLATOOLS_API UGenerationPathsHelper : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()
public:

  UFUNCTION(BlueprintCallable, BlueprintPure)
  static FString GetRawMapDirectoryPath(FString MapName) {
    return FPaths::ProjectPluginsDir() + MapName + "/Content/Maps/";
  }

  UFUNCTION(BlueprintCallable, BlueprintPure)
  static FString GetMapDirectoryPath(FString MapName) {
    return "/" + MapName + "/Maps/";
  }

  UFUNCTION(BlueprintCallable, BlueprintPure)
  static FString GetMapContentDirectoryPath(FString MapName) {
    return "/" + MapName + "/Static/";
  }

  UFUNCTION(BlueprintCallable)
  static FString GetDigitalTwinsPluginPath() {
    TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("CarlaDigitalTwinsTool");
    return Plugin.IsValid() ? FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir()) : FString();
  }
};
