// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "WeatherDescription.h"

#include "Util/IniFile.h"

static FString PrecipitationTypeToString(EPrecipitationType PrecipitationType)
{
  const UEnum* ptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPrecipitationType"), true);
  if(!ptr)
    return FString("Rain");
  return ptr->GetNameStringByIndex(static_cast<int32>(PrecipitationType));
}

static void LoadPrecipitationType(
    const FIniFile &ConfigFile,
    const TCHAR* Section,
    const TCHAR* Key,
    EPrecipitationType &Target)
{
  FString ValueString;
  if (ConfigFile.GetFConfigFile().GetString(Section, Key, ValueString)) {
    if (ValueString == "Rain") {
      Target = EPrecipitationType::Rain;
    } else {
      UE_LOG(LogCarla, Error, TEXT("Invalid precipitation type \"%s\" in INI file"), *ValueString);
      Target = EPrecipitationType::Rain;
    }
  }
}

static FString AutoExposureMethodToString(EAutoExposureMethod AutoExposureMethod)
{
  switch (AutoExposureMethod) {
    case AEM_Histogram: return "Histogram";
    case AEM_Basic:     return "Basic";
    default:            return "INVALID";
  };
}

static void LoadAutoExposureMethod(
    const FIniFile &ConfigFile,
    const TCHAR* Section,
    const TCHAR* Key,
    EAutoExposureMethod &Target)
{
  FString ValueString;
  if (ConfigFile.GetFConfigFile().GetString(Section, Key, ValueString)) {
    if (ValueString == "Basic") {
      Target = AEM_Basic;
    } else {
      Target = AEM_Histogram;
    }
  }
}

void FWeatherDescription::ReadFromConfigFile(const FIniFile &ConfigFile, const FString &Section)
{
  Name = Section;
#define CARLA_LOAD_FROM_INI(Type, Key) ConfigFile.Get ## Type(*Section, TEXT(#Key), Key);
  // Sun.
  CARLA_LOAD_FROM_INI(Float, SunPolarAngle)
  CARLA_LOAD_FROM_INI(Float, SunAzimuthAngle)
  CARLA_LOAD_FROM_INI(Float, SunBrightness)
  CARLA_LOAD_FROM_INI(Float, SunDirectionalLightIntensity)
  CARLA_LOAD_FROM_INI(LinearColor, SunDirectionalLightColor)
  CARLA_LOAD_FROM_INI(Float, SunIndirectLightIntensity)
  // Sky.
  CARLA_LOAD_FROM_INI(Float, CloudOpacity)
  CARLA_LOAD_FROM_INI(Float, HorizontFalloff)
  CARLA_LOAD_FROM_INI(LinearColor, ZenithColor)
  CARLA_LOAD_FROM_INI(LinearColor, HorizonColor)
  CARLA_LOAD_FROM_INI(LinearColor, CloudColor)
  CARLA_LOAD_FROM_INI(LinearColor, OverallSkyColor)
  CARLA_LOAD_FROM_INI(Float, SkyLightIntensity)
  CARLA_LOAD_FROM_INI(LinearColor, SkyLightColor)
  // Precipitation.
  CARLA_LOAD_FROM_INI(Bool, bPrecipitation)
  LoadPrecipitationType(ConfigFile, *Section, TEXT("PrecipitationType"), PrecipitationType);
  CARLA_LOAD_FROM_INI(Float, PrecipitationAmount)
  CARLA_LOAD_FROM_INI(Float, PrecipitationAccumulation)
  // Wind.
  CARLA_LOAD_FROM_INI(Bool, bWind)
  CARLA_LOAD_FROM_INI(Float, WindIntensity)
  CARLA_LOAD_FROM_INI(Float, WindAngle)
  // Camera Post-Process Parameters.
  CARLA_LOAD_FROM_INI(Bool, bOverrideCameraPostProcessParameters)
  EAutoExposureMethod AutoExposureMethod;
  LoadAutoExposureMethod(
      ConfigFile,
      *Section,
      TEXT("CameraPostProcessParameters.AutoExposureMethod"),
      AutoExposureMethod);
  CameraPostProcessParameters.AutoExposureMethod = AutoExposureMethod;
  CARLA_LOAD_FROM_INI(Float, CameraPostProcessParameters.AutoExposureMinBrightness)
  CARLA_LOAD_FROM_INI(Float, CameraPostProcessParameters.AutoExposureMaxBrightness)
  CARLA_LOAD_FROM_INI(Float, CameraPostProcessParameters.AutoExposureBias)
#undef CARLA_LOAD_FROM_INI
}

void FWeatherDescription::WriteToConfigFile(FIniFile &ConfigFile) const
{
  const FString &Section = Name;
  ConfigFile.AddSectionIfMissing(Section);
#define CARLA_WRITE_TO_INI(Type, Key) ConfigFile.Set ## Type(*Section, TEXT(#Key), Key);
  // Sun.
  CARLA_WRITE_TO_INI(Float, SunPolarAngle)
  CARLA_WRITE_TO_INI(Float, SunAzimuthAngle)
  CARLA_WRITE_TO_INI(Float, SunBrightness)
  CARLA_WRITE_TO_INI(Float, SunDirectionalLightIntensity)
  CARLA_WRITE_TO_INI(LinearColor, SunDirectionalLightColor)
  CARLA_WRITE_TO_INI(Float, SunIndirectLightIntensity)
  // Sky.
  CARLA_WRITE_TO_INI(Float, CloudOpacity)
  CARLA_WRITE_TO_INI(Float, HorizontFalloff)
  CARLA_WRITE_TO_INI(LinearColor, ZenithColor)
  CARLA_WRITE_TO_INI(LinearColor, HorizonColor)
  CARLA_WRITE_TO_INI(LinearColor, CloudColor)
  CARLA_WRITE_TO_INI(LinearColor, OverallSkyColor)
  CARLA_WRITE_TO_INI(Float, SkyLightIntensity)
  CARLA_WRITE_TO_INI(LinearColor, SkyLightColor)
  // Precipitation.
  CARLA_WRITE_TO_INI(Bool, bPrecipitation)
  ConfigFile.SetString(*Section, TEXT("PrecipitationType"), PrecipitationTypeToString(PrecipitationType));
  CARLA_WRITE_TO_INI(Float, PrecipitationAmount)
  CARLA_WRITE_TO_INI(Float, PrecipitationAccumulation)
  // Wind.
  CARLA_WRITE_TO_INI(Bool, bWind)
  CARLA_WRITE_TO_INI(Float, WindIntensity)
  CARLA_WRITE_TO_INI(Float, WindAngle)
  // Camera Post-Process Parameters.
  CARLA_WRITE_TO_INI(Bool, bOverrideCameraPostProcessParameters)
  ConfigFile.SetString(
      *Section,
      TEXT("CameraPostProcessParameters.AutoExposureMethod"),
      AutoExposureMethodToString(CameraPostProcessParameters.AutoExposureMethod.GetValue()));
  CARLA_WRITE_TO_INI(Float, CameraPostProcessParameters.AutoExposureMinBrightness)
  CARLA_WRITE_TO_INI(Float, CameraPostProcessParameters.AutoExposureMaxBrightness)
  CARLA_WRITE_TO_INI(Float, CameraPostProcessParameters.AutoExposureBias)
#undef CARLA_WRITE_TO_INI
}
