// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "WeatherDescription.h"

#include "IniFile.h"

static FString PrecipitationTypeToString(EPrecipitationType PrecipitationType)
{
  const UEnum* ptr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EPrecipitationType"), true);
  if(!ptr)
    return FString("Rain");
  return ptr->GetEnumName(static_cast<int32>(PrecipitationType));
}

static void LoadPrecipitationType(
    const IniFile &ConfigFile,
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

void FWeatherDescription::ReadFromConfigFile(const IniFile &ConfigFile, const FString &Section)
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
#undef CARLA_LOAD_FROM_INI
}

void FWeatherDescription::WriteToConfigFile(IniFile &ConfigFile) const
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
#undef CARLA_WRITE_TO_INI
}
