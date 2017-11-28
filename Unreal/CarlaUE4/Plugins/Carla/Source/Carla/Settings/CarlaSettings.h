// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CameraDescription.h"
#include "WeatherDescription.h"
#include "LidarDescription.h"

#include "UObject/NoExportTypes.h"
#include "CarlaSettings.generated.h"

/// Global settings for CARLA.
UCLASS()
class CARLA_API UCarlaSettings : public UObject
{
  GENERATED_BODY()

public:

  /** Load the settings based on the command-line arguments and the INI file if provided. */
  void LoadSettings();

  /** Load the settings from the given string (formatted as INI). CarlaServer section is ignored. */
  void LoadSettingsFromString(const FString &INIFileContents);

  /** Load weather description from config files. (There may be overrides for each map). */
  void LoadWeatherDescriptions(const FString &MapName);

  /** Check if requested weather id is present in WeatherDescriptions. */
  void ValidateWeatherId();

  /** Log settings values. */
  void LogSettings() const;

  const FWeatherDescription *GetActiveWeatherDescription() const
  {
    if ((WeatherId >= 0) && (WeatherId < WeatherDescriptions.Num())) {
      return &WeatherDescriptions[WeatherId];
    }
    return nullptr;
  }

  // Special overload for blueprints.
  UFUNCTION(BlueprintCallable)
  void GetActiveWeatherDescription(
      bool &bWeatherWasChanged,
      FWeatherDescription &WeatherDescription) const;

  UFUNCTION(BlueprintCallable)
  const FWeatherDescription &GetWeatherDescriptionByIndex(int32 Index);

private:

  void LoadSettingsFromFile(const FString &FilePath, bool bLogOnFailure);

  void ResetCameraDescriptions();

  void ResetLidarDescriptions();

  /** File name of the settings file used to load this settings. Empty if none used. */
  UPROPERTY(Category = "CARLA Settings|Debug", VisibleAnywhere)
  FString CurrentFileName;

  // ===========================================================================
  /// @name CARLA Server
  // ===========================================================================
  /// @{
public:

  /** If active, wait for the client to connect and control the pawn. */
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere)
  bool bUseNetworking = true;

  /** World port to listen for client connections. */
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  uint32 WorldPort = 2000u;

  /** Time-out in milliseconds for the networking operations. */
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  uint32 ServerTimeOut = 10000u;

  /** In synchronous mode, CARLA waits every tick until the control from the
    * client is received.
    */
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  bool bSynchronousMode = true;

  /** Send info about every non-player agent in the scene every frame. */
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  bool bSendNonPlayerAgentsInfo = false;

  /// @}
  // ===========================================================================
  /// @name Level Settings
  // ===========================================================================
  /// @{
public:

  /** Path to the pawn class of the player. */
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  FString PlayerVehicle;

  /** Number of NPC vehicles to be spawned into the level. */
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  uint32 NumberOfVehicles = 5u;

  /** Number of NPC pedestrians to be spawned into the level. */
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  uint32 NumberOfPedestrians = 15u;

  /** Index of the weather setting to use. If negative, weather won't be changed. */
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  int32 WeatherId = -1;

  /** Available weather settings. */
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  TArray<FWeatherDescription> WeatherDescriptions;

  /** Random seed for the pedestrian spawner. */
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  int32 SeedPedestrians = 123456789;

  /** Random seed for the vehicle spawner. */
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  int32 SeedVehicles = 123456789;

  /// @}
  // ===========================================================================
  /// @name Scene Capture
  // ===========================================================================
  /// @{
public:

  /** Descriptions of the cameras to be attached to the player. */
  UPROPERTY(Category = "Scene Capture", VisibleAnywhere)
  TMap<FString, FCameraDescription> CameraDescriptions;

  /** Whether semantic segmentation should be activated. The mechanisms for
    * semantic segmentation impose some performance penalties even if it is not
    * used, we only enable it if necessary.
    */
  UPROPERTY(Category = "Scene Capture", VisibleAnywhere)
  bool bSemanticSegmentationEnabled = false;

  /** Descriptions of the lidars to be attached to the player. */
  UPROPERTY(Category = "Scene Capture", VisibleAnywhere)
  TMap<FString, FLidarDescription> LidarDescriptions;

  /// @}
};
