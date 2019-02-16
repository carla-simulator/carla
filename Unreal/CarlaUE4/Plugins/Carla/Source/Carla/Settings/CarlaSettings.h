// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/StaticMesh.h"

#include "Carla/Settings/QualityLevelUE.h"
#include "Carla/Settings/WeatherDescription.h"

#include "CarlaSettings.generated.h"

/// Global settings for CARLA.
///
/// Setting object used to hold both config settings and editable ones in one
/// place. To ensure the settings are saved to the specified config file make
/// sure to add props using the globalconfig or config meta.
UCLASS(BlueprintType, Blueprintable, config = Game, defaultconfig)
class CARLA_API UCarlaSettings : public UObject
{
  GENERATED_BODY()

public:

  /// Sets the new quality settings level and make changes in the game related
  /// to it.
  ///
  /// @note This will not apply the quality settings. Use ApplyQualitySettings
  /// functions instead
  /// @param InQualityLevel Store the new quality.
  UFUNCTION(BlueprintCallable, Category = "CARLA Settings")
  void SetQualityLevel(EQualityLevel InQualityLevel)
  {
    QualityLevel = InQualityLevel;
  }

  /// @return current quality settings level (could not have been applied yet).
  UFUNCTION(BlueprintCallable, Category = "CARLA Settings")
  EQualityLevel GetQualityLevel() const
  {
    return QualityLevel;
  }

  /// Load the settings based on the command-line arguments and the INI file if
  /// provided.
  void LoadSettings();

  /// Load the settings from the given string (formatted as INI). CarlaServer
  /// section is ignored.
  void LoadSettingsFromString(const FString &INIFileContents);

  /// Load weather description from config files. (There may be overrides for
  /// each map).
  void LoadWeatherDescriptions();

  /// Check if requested weather id is present in WeatherDescriptions.
  void ValidateWeatherId();

  /// Log settings values.
  void LogSettings() const;

  const FWeatherDescription *GetActiveWeatherDescription() const
  {
    if ((WeatherId >= 0) && (WeatherId < WeatherDescriptions.Num()))
    {
      return &WeatherDescriptions[WeatherId];
    }
    return nullptr;
  }

  // Special overload for blueprints.
  UFUNCTION(BlueprintCallable, Category = "CARLA Settings")
  void GetActiveWeatherDescription(
      bool &bWeatherWasChanged,
      FWeatherDescription &WeatherDescription) const;

  UFUNCTION(BlueprintCallable)
  const FWeatherDescription &GetWeatherDescriptionByIndex(int32 Index);

  /// ----------- constants ------------------

public:

  /// CARLA_ROAD name to tag road mesh actors.
  static const FName CARLA_ROAD_TAG;

  /// CARLA_SKY name to tag the sky sphere (BPS) actors in the scenes.
  static const FName CARLA_SKY_TAG;

private:

  void LoadSettingsFromFile(const FString &FilePath, bool bLogOnFailure);

  /// File name of the settings file used to load this settings. Empty if none
  /// used.
  UPROPERTY(Category = "CARLA Settings|Debug", VisibleAnywhere)
  FString CurrentFileName;

  // ===========================================================================
  /// @name CARLA Server
  // ===========================================================================
  /// @{

public:

  /// If active, wait for the client to connect and control the pawn.
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere)
  bool bUseNetworking = false;

  /// World port to listen for client connections.
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  uint32 WorldPort = 2000u;

  /// Time-out in milliseconds for the networking operations.
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  uint32 ServerTimeOut = 10000u;

  /// In synchronous mode, CARLA waits every tick until the control from the
  /// client is received.
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  bool bSynchronousMode = false;

  /// Send info about every non-player agent in the scene every frame.
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  bool bSendNonPlayerAgentsInfo = false;

  /// Enable or disable the viewport rendering of the world. Disabled by
  /// default.
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere)
  bool bDisableRendering = false;

  /// @}
  // ===========================================================================
  /// @name Level Settings
  // ===========================================================================
  /// @{

public:

  /// Display name of the current map.
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  FString MapName;

  /// Path to the pawn class of the player.
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  FString PlayerVehicle;

  /// Number of NPC vehicles to be spawned into the level.
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  uint32 NumberOfVehicles = 5u;

  /// Number of NPC pedestrians to be spawned into the level.
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  uint32 NumberOfPedestrians = 15u;

  /// Index of the weather setting to use. If negative, weather won't be
  /// changed.
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  int32 WeatherId = -1;

  /// Available weather settings.
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  TArray<FWeatherDescription> WeatherDescriptions;

  /// Random seed for the pedestrian spawner.
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  int32 SeedPedestrians = 123456789;

  /// Random seed for the vehicle spawner.
  UPROPERTY(Category = "Level Settings", VisibleAnywhere)
  int32 SeedVehicles = 123456789;

  /// Disable bikes and motorbikes.
  UPROPERTY(Category = "Level Settings", BlueprintReadOnly, VisibleAnywhere)
  bool bDisableTwoWheeledVehicles = false;

  /// @}

  // ===========================================================================
  /// @name Quality Settings
  // ===========================================================================
  /// @{

private:

  /// Quality Settings level.
  UPROPERTY(Category = "Quality Settings", VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
  EQualityLevel QualityLevel = EQualityLevel::Epic;

public:

  /// Low quality Road Materials. Uses slots name to set material for each part
  /// of the road for low quality.
  ///
  /// @todo Move Low quality vars to a generic map of structs with the quality
  /// level as key.
  UPROPERTY(Category = "Quality Settings/Low",
      BlueprintReadOnly,
      EditAnywhere,
      config,
      DisplayName = "Road Materials List for Low Quality")
  TArray<FStaticMaterial> LowRoadMaterials;

  /// Distance at which the light function should be completely faded to
  /// DisabledBrightness. This is useful for hiding aliasing from light
  /// functions applied in the distance.
  UPROPERTY(Category = "Quality Settings/Low", BlueprintReadOnly, EditAnywhere, config)
  float LowLightFadeDistance  = 1000.0f;

  /// Default low distance for all primitive components.
  UPROPERTY(Category = "Quality Settings/Low", BlueprintReadOnly, EditAnywhere, config,
      meta = (ClampMin = "5000.0", ClampMax = "20000.0", UIMin = "5000.0", UIMax = "20000.0"))
  float LowStaticMeshMaxDrawDistance = 10000.0f;

  /// Default low distance for roads meshes.
  UPROPERTY(Category = "Quality Settings/Low", BlueprintReadOnly, EditAnywhere, config,
      meta = (ClampMin = "5000.0", ClampMax = "20000.0", UIMin = "5000.0", UIMax = "20000.0"))
  float LowRoadPieceMeshMaxDrawDistance = 15000.0f;

  /// EPIC quality Road Materials. Uses slots name to set material for each part
  /// of the road for Epic quality.
  UPROPERTY(Category = "Quality Settings/Epic",
      BlueprintReadOnly,
      EditAnywhere,
      config,
      DisplayName = "Road Materials List for EPIC Quality")
  TArray<FStaticMaterial> EpicRoadMaterials;

  /// @}

  // ===========================================================================
  /// @name Sensors
  // ===========================================================================
  /// @{

public:

  /// Whether semantic segmentation should be activated. The mechanisms for
  /// semantic segmentation impose some performance penalties even if it is not
  /// used, we only enable it if necessary.
  UPROPERTY(Category = "Sensors", BlueprintReadOnly, VisibleAnywhere)
  bool bSemanticSegmentationEnabled = true;

  /// @}
};
