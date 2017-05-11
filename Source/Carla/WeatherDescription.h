// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "WeatherDescription.generated.h"

UENUM(BlueprintType)
enum class EPrecipitationType : uint8
{
  Rain                  UMETA(DisplayName = "Rain"),
};

USTRUCT(BlueprintType)
struct FWeatherDescription
{
  GENERATED_USTRUCT_BODY()

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
  float SunBrightness;

  /**  */
  UPROPERTY(Category = "Weather|Sun", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float SunDirectionalLightIntensity;

  /**  */
  UPROPERTY(Category = "Weather|Sun", EditAnywhere, BlueprintReadWrite)
  FLinearColor SunDirectionalLightColor;

  /**  */
  UPROPERTY(Category = "Weather|Sun", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float SunIndirectLightIntensity;

  /// @}
  // ===========================================================================
  /// @name Weather - Sky
  // ===========================================================================
  /// @{

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float CloudOpacity;

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float HorizontFalloff;

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite)
  FLinearColor ZenithColor;

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite)
  FLinearColor HorizonColor;

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite)
  FLinearColor CloudColor;

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite)
  FLinearColor OverallSkyColor;

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite, meta=(ClampMin = "0.0", ClampMax = "100.0"))
  float SkyLightIntensity;

  /**  */
  UPROPERTY(Category = "Weather|Sky", EditAnywhere, BlueprintReadWrite)
  FLinearColor SkyLightColor;

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
  float PrecipitationAmount = 0.0f;

  /**  */
  UPROPERTY(Category = "Weather|Precipitation", EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bPrecipitation", ClampMin = "0.0", ClampMax = "100.0"))
  float PrecipitationAccumulation = 0.0f;

  /// @}
};
