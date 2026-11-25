// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"

/// Where the Traffic Light Tool's assets live in the base Content: meshes, materials, data
/// tables, the rig blueprint and the JSON presets, so the tool needs no plugin. The folder
/// sits under /Game/Carla/Static/TrafficLight/, so the tagger labels every rig mesh as a
/// traffic light. /Game/Carla/Static/TrafficLight/TrafficLights2025 is a different, older
/// set used by the towns; the tool's own assets carry the SM_TLTool_ / MI_TLTool_ prefix so
/// the two never get mixed up.
namespace TLToolContent
{
/// Package path of the tool's content folder, with a trailing slash.
inline const TCHAR* Root() { return TEXT("/Game/Carla/Static/TrafficLight/TrafficLightTool/"); }

/// Object paths of the assets the code loads by path.
inline FString DataTable(const TCHAR* Name)
{
	return FString::Printf(TEXT("%sDataTables/%s.%s"), Root(), Name, Name);
}
inline FString LampMaterial() { return FString(Root()) + TEXT("Modules/Materials/MI_TLTool_Lamp.MI_TLTool_Lamp"); }
inline FString RigBlueprintClass()
{
	return FString(Root()) + TEXT("Blueprints/BP_TLTool_TrafficLight.BP_TLTool_TrafficLight_C");
}

/// Directory on disk that holds the JSON presets (Default.json, TrafficLight_01.json, ...).
CARLATOOLS_API FString PresetsDir();
}	 // namespace TLToolContent
