// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/StaticMesh.h"

#include "Carla/Settings/QualityLevelUE.h"

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

  /// Log settings values.
  void LogSettings() const;

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

  /// World port to listen for client connections.
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  uint32 RPCPort = 2000u;

  /// setting for the streaming port.
  uint32 StreamingPort = 2001u;

  /// setting for the secondary servers port.
  uint32 SecondaryPort = 2002u;

  /// setting for the IP and Port of the primary server to connect.
  std::string PrimaryIP = "";
  uint32      PrimaryPort = 2002u;

  /// In synchronous mode, CARLA waits every tick until the control from the
  /// client is received.
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere, meta = (EditCondition = bUseNetworking))
  bool bSynchronousMode = false;

  /// Enable or disable the viewport rendering of the world. Disabled by
  /// default.
  UPROPERTY(Category = "CARLA Server", VisibleAnywhere)
  bool bDisableRendering = false;

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

  /// Enable ROS2
  UPROPERTY(Category = "Quality Settings/ROS2",
      BlueprintReadOnly,
      EditAnywhere,
      config,
      DisplayName = "Enable ROS2")
  bool ROS2 = false;

  /// @}
};
