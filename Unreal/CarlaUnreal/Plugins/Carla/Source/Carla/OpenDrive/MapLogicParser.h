// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Traffic/TrafficLightManager.h"

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include <util/ue-header-guard-end.h>

#include "MapLogicParser.generated.h"

/// Represents timing configuration for a traffic light from map_logic.json
USTRUCT()
struct FTrafficLightTiming
{
  GENERATED_BODY()

  UPROPERTY()
  float RedDuration = 2.0f;

  UPROPERTY()
  float GreenDuration = 10.0f;

  UPROPERTY()
  float AmberDuration = 3.0f;

  UPROPERTY()
  float AmberBlinkInterval = 0.25f;
};

/// Represents a traffic light module with its lane assignments
USTRUCT()
struct FTrafficLightModule
{
  GENERATED_BODY()

  UPROPERTY()
  TArray<int32> LaneIds;
};

/// Represents a traffic light configuration entry from map_logic.json
USTRUCT()
struct FTrafficLightLogicData
{
  GENERATED_BODY()

  UPROPERTY()
  FString ActorName;

  UPROPERTY()
  FString SignalID;

  UPROPERTY()
  int32 JunctionID = -1;

  UPROPERTY()
  FString TrafficLightGroupID;

  UPROPERTY()
  FTrafficLightTiming Timing;

  UPROPERTY()
  TArray<FTrafficLightModule> Modules;
};

/// Utility class to parse map_logic.json files and apply traffic light timing configuration
class CARLA_API UMapLogicParser
{
public:

  /// Parse map_logic.json file and apply traffic light timing configuration
  /// to the TrafficLightManager. Returns true if file was found and parsed successfully.
  ///
  /// @param XODRFilePath Path to the .xodr file (will look for map_logic.json in same directory)
  /// @param TrafficLightManager The traffic light manager to apply configuration to
  /// @return true if map_logic.json was found and applied successfully, false otherwise
  static bool ParseAndApplyMapLogic(const FString& XODRFilePath, ATrafficLightManager* TrafficLightManager);

  /// Apply lane IDs from map_logic.json to traffic light components
  /// @param XODRFilePath Path to the .xodr file (will look for map_logic.json in same directory)
  /// @param TrafficLightManager The traffic light manager to apply configuration to
  static void ApplyLaneIdsFromMapLogic(const FString& XODRFilePath, ATrafficLightManager* TrafficLightManager);

private:

  /// Load and parse the map_logic.json file from the same directory as the XODR file
  /// @param XODRFilePath Path to the .xodr file
  /// @return Array of parsed traffic light logic data, empty if file not found or parse failed
  static TArray<FTrafficLightLogicData> LoadMapLogicFromJSON(const FString& XODRFilePath);

  /// Apply traffic light timing configuration to the TrafficLightManager
  /// @param LogicData Array of traffic light configurations to apply
  /// @param TrafficLightManager The traffic light manager to configure
  static void ApplyTrafficLightLogic(const TArray<FTrafficLightLogicData>& LogicData, ATrafficLightManager* TrafficLightManager);

  /// Parse individual traffic light data from JSON object
  /// @param TrafficLightJson JSON object representing a single traffic light configuration
  /// @return Parsed traffic light logic data
  static FTrafficLightLogicData ParseTrafficLightFromJSON(TSharedPtr<FJsonObject> TrafficLightJson);

  /// Parse timing data from JSON object
  /// @param TimingJson JSON object containing timing information
  /// @return Parsed timing configuration
  static FTrafficLightTiming ParseTimingFromJSON(TSharedPtr<FJsonObject> TimingJson);

  /// Get the directory path from a full file path
  /// @param FilePath Full path to a file
  /// @return Directory path without the filename
  static FString GetDirectoryPath(const FString& FilePath);
};
