// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "BlueprintUtilFunctions.h"

#include <util/ue-header-guard-begin.h>
#include "Misc/App.h"
#include <util/ue-header-guard-end.h>

UObject* UBlueprintUtilFunctions::CopyAssetToPlugin(UObject* SourceObject, FString PluginName)
{
  // The digitaltwins original duplicates the asset (and its reference graph) into the
  // generated per-map plugin so packaged maps are self-contained. CARLA maps here are not
  // per-map plugins, so the identity mapping is the desired behaviour: the generator keeps
  // using the source atom assets in place.
  return SourceObject;
}

FString UBlueprintUtilFunctions::GetProjectName()
{
  return FApp::GetProjectName();
}
