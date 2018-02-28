// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "WeatherDescription.h"

#include "UObject/NoExportTypes.h"

#include "CarlaSettings.generated.h"

UENUM(BlueprintType)
enum class EQualitySettingsLevel : uint8
{
  None    UMETA(DisplayName = "Not set"),
  Low     UMETA(DisplayName = "Low"),
  Medium  UMETA(DisplayName = "Medium"),
  High    UMETA(DisplayName = "High"),
  Epic    UMETA(DisplayName = "Epic") 
	
};

class CARLA_API FQualitySettings 
{
public:
 using uint_type = typename std::underlying_type<EQualitySettingsLevel>::type;
 static EQualitySettingsLevel FromString(const FString &SQualitySettingsLevel);
 static FString ToString(EQualitySettingsLevel QualitySettingsLevel);
 static constexpr uint_type ToUInt(EQualitySettingsLevel quality_settings_level)
 {
   return static_cast<uint_type>(quality_settings_level);
 }
};

class USensorDescription;
struct FStaticMaterial;
/** Global settings for CARLA.
 * Setting object used to hold both config settings and editable ones in one place
 * To ensure the settings are saved to the specified config file make sure to add
 * props using the globalconfig or config meta.
 */
UCLASS(config = Game, defaultconfig)
class CARLA_API UCarlaSettings : public UObject
{
  GENERATED_BODY()

public:
  
  /** 
   * Sets the new quality settings level and make changes in the game related to it. 
   * Returns the result of the operation. 
   * @note This will not apply the quality settings. Use ApplyQualitySettings functions instead
   * @param newDefaultLevel Store the new quality 
   */
  bool SetQualitySettingsLevel(EQualitySettingsLevel newDefaultLevel);
  /** @return current quality settings level (could not be applied yet) */
  EQualitySettingsLevel GetQualitySettingsLevel() const { return DefaultQualitySettingsLevel; }
  /** @todo move to class */
  void ApplyQualitySettingsLevelPostRestart() const;
  void ApplyQualitySettingsLevelPreRestart() const{}
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
  ///----------- constants ------------------
public:
  /**
   * CARLA_ROAD name to tag road mesh actors
   */
  static const FName CARLA_ROAD_TAG;
  
private:
  /***/
  void LaunchLowQualityCommands(UWorld* world) const;
  /***/
  void SetAllLightsLowQuality(UWorld* world) const;
  /***/
  void SetAllRoadsLowQuality(UWorld* world) const;
  /***/
  void SetAllActorsDrawDistance(UWorld* world, const float static_mesh_max_draw_distance) const;
  /***/
  void ApplyLowQualitySettings() const;
  /***/
  void LoadSettingsFromFile(const FString &FilePath, bool bLogOnFailure);

  void ResetSensorDescriptions();

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
  bool bUseNetworking = false;

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
  /// @name Quality Settings
  // ===========================================================================
  /// @{
private:
  /** Quality Settings level. */
  UPROPERTY(Category = "Quality Settings", VisibleAnywhere, meta =(AllowPrivateAccess="true"))
  EQualitySettingsLevel DefaultQualitySettingsLevel = EQualitySettingsLevel::None;
 
 public:
  /** @TODO : Move Low quality vars to a generic map of structs with the quality level as key*/

  /** Low quality Road Materials. 
   * Using the index of array it will assign to the material with the same index in the mesh for each piece of road
   * @TODO: Use slots name to set it for each part of the road
   */
  UPROPERTY(Category = "Quality Settings/Low", EditAnywhere, config, DisplayName="Road Materials List for Low Quality")
  TArray<FStaticMaterial> LowRoadMaterials;

  //distances
  /**
   * Distance at which the light function should be completely faded to DisabledBrightness.  
   * This is useful for hiding aliasing from light functions applied in the distance.
   */
  UPROPERTY(Category = "Quality Settings/Low", EditAnywhere, config)
  float LowLightFadeDistance  = 1000.0f;

  /**
   *
   */
  UPROPERTY(Category = "Quality Settings/Low", EditAnywhere, config, meta = (ClampMin = "500.0", ClampMax = "15000.0", UIMin = "500.0", UIMax = "15000.0")) 
  float LowStaticMeshMaxDrawDistance = 10000.0f;

  /// @}

  // ===========================================================================
  /// @name Sensors
  // ===========================================================================
  /// @{
public:

  /** Descriptions of the cameras to be attached to the player. */
  UPROPERTY(Category = "Sensors", VisibleAnywhere)
  TMap<FString, USensorDescription *> SensorDescriptions;

  /** Whether semantic segmentation should be activated. The mechanisms for
    * semantic segmentation impose some performance penalties even if it is not
    * used, we only enable it if necessary.
    */
  UPROPERTY(Category = "Sensors", VisibleAnywhere)
  bool bSemanticSegmentationEnabled = false;

  /// @}
};
