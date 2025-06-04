// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Settings/CarlaSettings.h"
#include "Carla.h"
#include "Carla/Util/IniFile.h"

#include <util/ue-header-guard-begin.h>
#include "Misc/ConfigCacheIni.h"
#include "Misc/CommandLine.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstance.h"
#include "UObject/Package.h"
#include "Math/UnrealMathUtility.h"
#include <util/ue-header-guard-end.h>


// INI file sections.
#define S_CARLA_SERVER          TEXT("CARLA/Server")
#define S_CARLA_QUALITYSETTINGS TEXT("CARLA/QualitySettings")

// =============================================================================
// -- Static variables & constants ---------------------------------------------
// =============================================================================

const FName UCarlaSettings::CARLA_ROAD_TAG = FName("CARLA_ROAD");
const FName UCarlaSettings::CARLA_SKY_TAG = FName("CARLA_SKY");

// =============================================================================
// -- Static methods -----------------------------------------------------------
// =============================================================================

static EQualityLevel QualityLevelFromString(
    const FString &SQualitySettingsLevel,
    const EQualityLevel Default = EQualityLevel::INVALID)
{
  if (SQualitySettingsLevel.Equals("Low"))
  {
    return EQualityLevel::Low;
  }
  if (SQualitySettingsLevel.Equals("Epic"))
  {
    return EQualityLevel::Epic;
  }
  return Default;
}

FString QualityLevelToString(EQualityLevel QualitySettingsLevel)
{
  static_assert(TIsEnumClass<EQualityLevel>::Value);
  return StaticEnum<EQualityLevel>()->GetNameStringByValue((int64)QualitySettingsLevel);
}

static void LoadSettingsFromConfig(
    const FIniFile &ConfigFile,
    UCarlaSettings &Settings,
    const bool bLoadCarlaServerSection)
{
  // CarlaServer.
  if (bLoadCarlaServerSection)
  {
    ConfigFile.GetInt(S_CARLA_SERVER,    TEXT("WorldPort"), Settings.RPCPort);
    ConfigFile.GetInt(S_CARLA_SERVER,    TEXT("RPCPort"), Settings.RPCPort);
    Settings.StreamingPort = Settings.RPCPort + 1u;
    Settings.SecondaryPort = Settings.RPCPort + 2u;
    ConfigFile.GetInt(S_CARLA_SERVER,    TEXT("StreamingPort"), Settings.StreamingPort);
    ConfigFile.GetInt(S_CARLA_SERVER,    TEXT("SecondaryPort"), Settings.SecondaryPort);
    FString Tmp;
    ConfigFile.GetString(S_CARLA_SERVER, TEXT("PrimaryIP"), Tmp);
    Settings.PrimaryIP = TCHAR_TO_UTF8(*Tmp);
    ConfigFile.GetInt(S_CARLA_SERVER,    TEXT("PrimaryPort"), Settings.PrimaryPort);
  }
  ConfigFile.GetBool(S_CARLA_SERVER, TEXT("SynchronousMode"), Settings.bSynchronousMode);
  ConfigFile.GetBool(S_CARLA_SERVER, TEXT("DisableRendering"), Settings.bDisableRendering);
  // QualitySettings.
  FString sQualityLevel;
  ConfigFile.GetString(S_CARLA_QUALITYSETTINGS, TEXT("QualityLevel"), sQualityLevel);
  Settings.SetQualityLevel(QualityLevelFromString(sQualityLevel));
}

static bool GetSettingsFilePathFromCommandLine(FString &Value)
{
  if (FParse::Value(FCommandLine::Get(), TEXT("-carla-settings="), Value))
  {
    if (FPaths::IsRelative(Value))
    {
      Value = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir(), Value);
    }
    return true;
  }
  return false;
}

// =============================================================================
// -- UCarlaSettings -----------------------------------------------------------
// =============================================================================

void UCarlaSettings::LoadSettings()
{
  CurrentFileName = TEXT("");
  // Load settings from project Config folder if present.
  LoadSettingsFromFile(FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("CarlaSettings.ini")), false);
  // Load settings given by command-line arg if provided.
  {
    FString FilePath;
    if (GetSettingsFilePathFromCommandLine(FilePath))
    {
      LoadSettingsFromFile(FilePath, true);
    }
  }
  // Override settings from command-line.
  {
    uint32 Value;
    if (FParse::Value(FCommandLine::Get(), TEXT("-world-port="), Value) ||
        FParse::Value(FCommandLine::Get(), TEXT("-carla-port="), Value) ||
        FParse::Value(FCommandLine::Get(), TEXT("-carla-rpc-port="), Value) ||
        FParse::Value(FCommandLine::Get(), TEXT("-carla-world-port="), Value))
    {
      RPCPort = Value;
      StreamingPort = Value + 1u;
      SecondaryPort = Value + 2u;
    }
    if (FParse::Value(FCommandLine::Get(), TEXT("-carla-streaming-port="), Value))
    {
      StreamingPort = Value;
    }
    if (FParse::Value(FCommandLine::Get(), TEXT("-carla-secondary-port="), Value))
    {
      SecondaryPort = Value;
    }
    FString Tmp;
    if (FParse::Value(FCommandLine::Get(), TEXT("-carla-primary-host="), Tmp))
    {
      PrimaryIP = TCHAR_TO_UTF8(*Tmp);
    }
    if (FParse::Value(FCommandLine::Get(), TEXT("-carla-primary-port="), Value))
    {
      PrimaryPort = Value;
    }
    FString StringQualityLevel;
    if (FParse::Value(FCommandLine::Get(), TEXT("-quality-level="), StringQualityLevel))
    {
      QualityLevel = QualityLevelFromString(StringQualityLevel, EQualityLevel::Epic);
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("-no-rendering")))
    {
      bDisableRendering = true;
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("-ros2")))
    {
      ROS2 = true;
    }
  }
}

void UCarlaSettings::LoadSettingsFromString(const FString &INIFileContents)
{
  UE_LOG(LogCarla, Log, TEXT("Loading CARLA settings from string"));
  FIniFile ConfigFile;
  CurrentFileName = TEXT("<string-provided-by-client>");
  ConfigFile.ProcessInputFileContents(INIFileContents, CurrentFileName);
  constexpr bool bLoadCarlaServerSection = false;
  LoadSettingsFromConfig(ConfigFile, *this, bLoadCarlaServerSection);
}

void UCarlaSettings::LogSettings() const
{
  auto EnabledDisabled = [](bool bValue) { return (bValue ? TEXT("Enabled") : TEXT("Disabled")); };
  UE_LOG(LogCarla, Log,
      TEXT("== CARLA Settings =============================================================="));
  UE_LOG(LogCarla, Log, TEXT("Last settings file loaded: %s"), *CurrentFileName);
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SERVER);
  UE_LOG(LogCarla, Log, TEXT("RPC Port = %d"), RPCPort);
  UE_LOG(LogCarla, Log, TEXT("Streaming Port = %d"), StreamingPort);
  UE_LOG(LogCarla, Log, TEXT("Secondary Port = %d"), SecondaryPort);
  UE_LOG(LogCarla, Log, TEXT("Synchronous Mode = %s"), EnabledDisabled(bSynchronousMode));
  UE_LOG(LogCarla, Log, TEXT("Rendering = %s"), EnabledDisabled(!bDisableRendering));
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_QUALITYSETTINGS);
  UE_LOG(LogCarla, Log, TEXT("Quality Level = %s"), *QualityLevelToString(QualityLevel));
  UE_LOG(LogCarla, Log,
      TEXT("================================================================================"));
}

#undef S_CARLA_QUALITYSETTINGS
#undef S_CARLA_SERVER

void UCarlaSettings::LoadSettingsFromFile(const FString &FilePath, const bool bLogOnFailure)
{
  if (FPaths::FileExists(FilePath))
  {
    UE_LOG(LogCarla, Log, TEXT("Loading CARLA settings from \"%s\""), *FilePath);
    const FIniFile ConfigFile(FilePath);
    constexpr bool bLoadCarlaServerSection = true;
    LoadSettingsFromConfig(ConfigFile, *this, bLoadCarlaServerSection);
    CurrentFileName = FilePath;
  }
  else if (bLogOnFailure)
  {
    UE_LOG(LogCarla, Error, TEXT("Unable to find settings file \"%s\""), *FilePath);
  }
}
