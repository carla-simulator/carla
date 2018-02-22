// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Settings/CameraPostProcessParameters.h"

#include "WeatherDescription.generated.h"

class FIniFile;

UENUM(BlueprintType)
enum class EPrecipitationType : uint8
{
  Rain                  UMETA(DisplayName = "Rain")
};

USTRUCT(BlueprintType)
struct CARLA_API FWeatherDescription
{
  GENERATED_USTRUCT_BODY()

  void ReadFromConfigFile(const FIniFile &ConfigFile, const FString &Section);

  void WriteToConfigFile(FIniFile &ConfigFile) const;

  // ===========================================================================
  /// @name Weather
  // ===========================================================================
  /// @{

  /** Display name of the current weather. */
  UPROPERTY(Category = "Weather", EditAnywhere, BlueprintReadWrite)
  FString Name;

  /// @}
  // ===========================================================================
  /// @name Weather - Sun
  // ===========================================================================
  /// @{

  /** Polar angle of the Sun in degrees, with 0.0 at zenith, 90.0 at equator. */
  UPROPERTY(Category = "Weather|Sun", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "180.0"))
  float SunPolarAngle = 45.0f;

  /** Azimuth angle of the Sun in degrees. */
  UPROPERTY(Category = "Weather|Sun", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-180.0", ClampMax = "180.0"))
  float SunAzimuthAngle = 0.0f;

  /**  */
  UPROPERTY(Category = "Weather|Sun", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float SunBrightness = 50.0f;

  /**  */
  UPROPERTY(Category = "Weather|Sun", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float SunDirectionalLightIntensity = 5.0f;

  /**  */
  UPROPERTY(Category = "Weather|Sun", EditAnywhere, BlueprintReadWrite)
  FLinearColor SunDirectionalLightColor = FLinearColor(255.0f, 240.0f, 195.0f);

  /**  */
  UPROPERTY(Category = "Weather|Sun", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float SunIndirectLightIntensity = 6.0f;

  /// @}
  // ===========================================================================
  /// @name Weather - Sky
  // ===========================================================================
  /// @{

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float CloudOpacity = 10.0f;

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float HorizontFalloff = 3.0f;

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite)
  FLinearColor ZenithColor = FLinearColor(0.0340f, 0.1092f, 0.2950f);

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite)
  FLinearColor HorizonColor = FLinearColor(0.6599f, 0.8622f, 1.0f);

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite)
  FLinearColor CloudColor = FLinearColor(0.8558f, 0.9190f, 1.0f);

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite)
  FLinearColor OverallSkyColor = FLinearColor(1.0f, 1.0f, 1.0f);

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float SkyLightIntensity = 4.0f;

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite)
  FLinearColor SkyLightColor = FLinearColor(0.1950f, 0.1851f, 0.1413f, 0.0f);

  /// @}
  // ===========================================================================
  /// @name Weather - Precipitation
  // ===========================================================================
  /// @{

  /**  */
  UPROPERTY(Category = "Weather|Precipitation", EditAnywhere, BlueprintReadWrite)
  bool bPrecipitation = false;

  /**  */
  UPROPERTY(Category = "Weather|Precipitation", EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bPrecipitation"))
  EPrecipitationType PrecipitationType = EPrecipitationType::Rain;

  /**  */
  UPROPERTY(Category = "Weather|Precipitation", EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bPrecipitation", ClampMin = "0.0", ClampMax = "100.0"))
  float PrecipitationAmount = 50.0f;

  /**  */
  UPROPERTY(Category = "Weather|Precipitation", EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bPrecipitation", ClampMin = "0.0", ClampMax = "100.0"))
  float PrecipitationAccumulation = 50.0f;

  /// @}
  // ===========================================================================
  /// @name Weather - Wind
  // ===========================================================================
  /// @{

  /**  */
  UPROPERTY(Category = "Weather|Wind", EditAnywhere, BlueprintReadWrite)
  bool bWind = false;

  /**  */
  UPROPERTY(Category = "Weather|Wind", EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bWind", ClampMin = "0.0", ClampMax = "100.0"))
  float WindIntensity = 20.0f;

  /**  */
  UPROPERTY(Category = "Weather|Wind", EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bWind", ClampMin = "-180.0", ClampMax = "180.0"))
  float WindAngle = 0.0f;

  /// @}
  // ===========================================================================
  /// @name Weather - Camera Post-Process Parameters
  // ===========================================================================
  /// @{

  /** If disabled the camera default values will be used instead. */
  UPROPERTY(Category = "Weather|Camera Post-Process Parameters", EditAnywhere, BlueprintReadWrite)
  bool bOverrideCameraPostProcessParameters = false;

  /** Camera post-process parameters to be overriden. */
  UPROPERTY(Category = "Weather|Camera Post-Process Parameters", EditAnywhere, BlueprintReadWrite, meta=(EditCondition = "bOverrideCameraPostProcessParameters"))
  FCameraPostProcessParameters CameraPostProcessParameters;

  /// @}
};
