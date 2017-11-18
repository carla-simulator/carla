// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"

#include "CommandLine.h"
#include "UnrealMathUtility.h"

#include "DynamicWeather.h"
#include "Settings/CarlaSettings.h"
#include "Util/IniFile.h"

// INI file sections.
#define S_CARLA_SERVER                 TEXT("CARLA/Server")
#define S_CARLA_LEVELSETTINGS          TEXT("CARLA/LevelSettings")
#define S_CARLA_SCENECAPTURE           TEXT("CARLA/SceneCapture")

// =============================================================================
// -- MyIniFile ----------------------------------------------------------------
// =============================================================================

class MyIniFile : public IniFile {
public:

  MyIniFile() = default;

  explicit MyIniFile(const FString &FileName) : IniFile(FileName) {}

  void GetPostProcessEffect(const TCHAR* Section, const TCHAR* Key, EPostProcessEffect &Target) const
  {
    FString ValueString;
    if (GetFConfigFile().GetString(Section, Key, ValueString)) {
      if (ValueString == "None") {
        Target = EPostProcessEffect::None;
      } else if (ValueString == "SceneFinal") {
        Target = EPostProcessEffect::SceneFinal;
      } else if (ValueString == "Depth") {
        Target = EPostProcessEffect::Depth;
      } else if (ValueString == "SemanticSegmentation") {
        Target = EPostProcessEffect::SemanticSegmentation;
      } else {
        UE_LOG(LogCarla, Error, TEXT("Invalid post-processing \"%s\" in INI file"), *ValueString);
        Target = EPostProcessEffect::INVALID;
      }
    }
  }
};

// =============================================================================
// -- Static methods -----------------------------------------------------------
// =============================================================================

static void GetCameraDescription(
    const MyIniFile &ConfigFile,
    const TCHAR* Section,
    FCameraDescription &Camera)
{
  ConfigFile.GetInt(Section, TEXT("ImageSizeX"), Camera.ImageSizeX);
  ConfigFile.GetInt(Section, TEXT("ImageSizeY"), Camera.ImageSizeY);
  ConfigFile.GetInt(Section, TEXT("CameraFOV"), Camera.FOVAngle);
  ConfigFile.GetInt(Section, TEXT("CameraPositionX"), Camera.Position.X);
  ConfigFile.GetInt(Section, TEXT("CameraPositionY"), Camera.Position.Y);
  ConfigFile.GetInt(Section, TEXT("CameraPositionZ"), Camera.Position.Z);
  ConfigFile.GetInt(Section, TEXT("CameraRotationPitch"), Camera.Rotation.Pitch);
  ConfigFile.GetInt(Section, TEXT("CameraRotationRoll"), Camera.Rotation.Roll);
  ConfigFile.GetInt(Section, TEXT("CameraRotationYaw"), Camera.Rotation.Yaw);
  ConfigFile.GetPostProcessEffect(Section, TEXT("PostProcessing"), Camera.PostProcessEffect);
}

static void ValidateCameraDescription(FCameraDescription &Camera)
{
  FMath::Clamp(Camera.FOVAngle, 0.001f, 360.0f);
  Camera.ImageSizeX = (Camera.ImageSizeX == 0u ? 720u : Camera.ImageSizeX);
  Camera.ImageSizeY = (Camera.ImageSizeY == 0u ? 512u : Camera.ImageSizeY);
}

static bool RequestedSemanticSegmentation(const FCameraDescription &Camera)
{
  return (Camera.PostProcessEffect == EPostProcessEffect::SemanticSegmentation);
}

static void LoadSettingsFromConfig(
    const MyIniFile &ConfigFile,
    UCarlaSettings &Settings,
    const bool bLoadCarlaServerSection)
{
  // CarlaServer.
  if (bLoadCarlaServerSection) {
    ConfigFile.GetBool(S_CARLA_SERVER, TEXT("UseNetworking"), Settings.bUseNetworking);
    ConfigFile.GetInt(S_CARLA_SERVER, TEXT("WorldPort"), Settings.WorldPort);
    ConfigFile.GetInt(S_CARLA_SERVER, TEXT("ServerTimeOut"), Settings.ServerTimeOut);
  }
  ConfigFile.GetBool(S_CARLA_SERVER, TEXT("SynchronousMode"), Settings.bSynchronousMode);
  ConfigFile.GetBool(S_CARLA_SERVER, TEXT("SendNonPlayerAgentsInfo"), Settings.bSendNonPlayerAgentsInfo);
  // LevelSettings.
  ConfigFile.GetString(S_CARLA_LEVELSETTINGS, TEXT("PlayerVehicle"), Settings.PlayerVehicle);
  ConfigFile.GetInt(S_CARLA_LEVELSETTINGS, TEXT("NumberOfVehicles"), Settings.NumberOfVehicles);
  ConfigFile.GetInt(S_CARLA_LEVELSETTINGS, TEXT("NumberOfPedestrians"), Settings.NumberOfPedestrians);
  ConfigFile.GetInt(S_CARLA_LEVELSETTINGS, TEXT("WeatherId"), Settings.WeatherId);
  ConfigFile.GetInt(S_CARLA_LEVELSETTINGS, TEXT("SeedVehicles"), Settings.SeedVehicles);
  ConfigFile.GetInt(S_CARLA_LEVELSETTINGS, TEXT("SeedPedestrians"), Settings.SeedPedestrians);
  // SceneCapture.
  FString Cameras;
  ConfigFile.GetString(S_CARLA_SCENECAPTURE, TEXT("Cameras"), Cameras);
  TArray<FString> CameraNames;
  Cameras.ParseIntoArray(CameraNames, TEXT(","), true);
  for (FString &Name : CameraNames) {
    FCameraDescription &Camera = Settings.CameraDescriptions.FindOrAdd(Name);
    GetCameraDescription(ConfigFile, S_CARLA_SCENECAPTURE, Camera);

    TArray<FString> SubSections;
    Name.ParseIntoArray(SubSections, TEXT("/"), true);
    check(SubSections.Num() > 0);
    FString Section = S_CARLA_SCENECAPTURE;
    for (FString &SubSection : SubSections) {
      Section += TEXT("/");
      Section += SubSection;
      GetCameraDescription(ConfigFile, *Section, Camera);
    }

    ValidateCameraDescription(Camera);
    Settings.bSemanticSegmentationEnabled |= RequestedSemanticSegmentation(Camera);
  }
}

static bool GetSettingsFilePathFromCommandLine(FString &Value)
{
  if (FParse::Value(FCommandLine::Get(), TEXT("-carla-settings="), Value)) {
    if (FPaths::IsRelative(Value)) {
      Value = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir(), Value);
      return true;
    }
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
  LoadSettingsFromFile(FPaths::Combine(FPaths::GameConfigDir(), TEXT("CarlaSettings.ini")), false);
  // Load settings given by command-line arg if provided.
  {
    FString FilePath;
    if (GetSettingsFilePathFromCommandLine(FilePath)) {
      LoadSettingsFromFile(FilePath, true);
    }
  }
  // Override settings from command-line.
  {
    if (FParse::Param(FCommandLine::Get(), TEXT("carla-server"))) {
      bUseNetworking = true;
    }
    uint32 Value;
    if (FParse::Value(FCommandLine::Get(), TEXT("-world-port="), Value) ||
        FParse::Value(FCommandLine::Get(), TEXT("-carla-world-port="), Value)) {
      WorldPort = Value;
      bUseNetworking = true;
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("carla-no-networking"))) {
      bUseNetworking = false;
    }
  }
}

void UCarlaSettings::LoadSettingsFromString(const FString &INIFileContents)
{
  UE_LOG(LogCarla, Log, TEXT("Loading CARLA settings from string"));
  ResetCameraDescriptions();
  MyIniFile ConfigFile;
  ConfigFile.ProcessInputFileContents(INIFileContents);
  constexpr bool bLoadCarlaServerSection = false;
  LoadSettingsFromConfig(ConfigFile, *this, bLoadCarlaServerSection);
  CurrentFileName = TEXT("<string-provided-by-client>");
}

void UCarlaSettings::LoadWeatherDescriptions(const FString &MapName)
{
  WeatherDescriptions.Empty();
  ADynamicWeather::LoadWeatherDescriptionsFromFile(MapName, WeatherDescriptions);
  check(WeatherDescriptions.Num() > 0);
}

void UCarlaSettings::ValidateWeatherId()
{
  if (WeatherId >= WeatherDescriptions.Num()) {
    UE_LOG(LogCarla, Error, TEXT("Provided weather id %d cannot be found"), WeatherId);
    WeatherId = -1;
  }
}

void UCarlaSettings::LogSettings() const
{
  auto EnabledDisabled = [](bool bValue) { return (bValue ? TEXT("Enabled") : TEXT("Disabled")); };
  UE_LOG(LogCarla, Log, TEXT("== CARLA Settings =============================================================="));
  UE_LOG(LogCarla, Log, TEXT("Last settings file loaded: %s"), *CurrentFileName);
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SERVER);
  UE_LOG(LogCarla, Log, TEXT("Networking = %s"), EnabledDisabled(bUseNetworking));
  UE_LOG(LogCarla, Log, TEXT("World Port = %d"), WorldPort);
  UE_LOG(LogCarla, Log, TEXT("Server Time-out = %d ms"), ServerTimeOut);
  UE_LOG(LogCarla, Log, TEXT("Synchronous Mode = %s"), EnabledDisabled(bSynchronousMode));
  UE_LOG(LogCarla, Log, TEXT("Send Non-Player Agents Info = %s"), EnabledDisabled(bSendNonPlayerAgentsInfo));
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_LEVELSETTINGS);
  UE_LOG(LogCarla, Log, TEXT("Player Vehicle        = %s"), (PlayerVehicle.IsEmpty() ? TEXT("Default") : *PlayerVehicle));
  UE_LOG(LogCarla, Log, TEXT("Number Of Vehicles    = %d"), NumberOfVehicles);
  UE_LOG(LogCarla, Log, TEXT("Number Of Pedestrians = %d"), NumberOfPedestrians);
  UE_LOG(LogCarla, Log, TEXT("Weather Id = %d"), WeatherId);
  UE_LOG(LogCarla, Log, TEXT("Seed Vehicle Spawner = %d"), SeedVehicles);
  UE_LOG(LogCarla, Log, TEXT("Seed Pedestrian Spawner = %d"), SeedPedestrians);
  UE_LOG(LogCarla, Log, TEXT("Found %d available weather settings."), WeatherDescriptions.Num());
  for (auto i = 0; i < WeatherDescriptions.Num(); ++i) {
    UE_LOG(LogCarla, Log, TEXT("  * %d - %s"), i, *WeatherDescriptions[i].Name);
  }
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SCENECAPTURE);
  UE_LOG(LogCarla, Log, TEXT("Added %d cameras."), CameraDescriptions.Num());
  UE_LOG(LogCarla, Log, TEXT("Semantic Segmentation = %s"), EnabledDisabled(bSemanticSegmentationEnabled));
  for (auto &Item : CameraDescriptions) {
    UE_LOG(LogCarla, Log, TEXT("[%s/%s]"), S_CARLA_SCENECAPTURE, *Item.Key);
    UE_LOG(LogCarla, Log, TEXT("Image Size = %dx%d"), Item.Value.ImageSizeX, Item.Value.ImageSizeY);
    UE_LOG(LogCarla, Log, TEXT("Camera Position = (%s)"), *Item.Value.Position.ToString());
    UE_LOG(LogCarla, Log, TEXT("Camera Rotation = (%s)"), *Item.Value.Rotation.ToString());
    UE_LOG(LogCarla, Log, TEXT("Post-Processing = %s"), *PostProcessEffect::ToString(Item.Value.PostProcessEffect));
  }
  UE_LOG(LogCarla, Log, TEXT("================================================================================"));
}

#undef S_CARLA_SERVER
#undef S_CARLA_LEVELSETTINGS
#undef S_CARLA_SCENECAPTURE

void UCarlaSettings::GetActiveWeatherDescription(
    bool &bWeatherWasChanged,
    FWeatherDescription &WeatherDescription) const
{
  auto WeatherPtr = GetActiveWeatherDescription();
  if (WeatherPtr != nullptr) {
    WeatherDescription = *WeatherPtr;
    bWeatherWasChanged = true;
  } else {
    bWeatherWasChanged = false;
  }
}

const FWeatherDescription &UCarlaSettings::GetWeatherDescriptionByIndex(int32 Index)
{
  check(WeatherDescriptions.Num() > 0);
  FMath::Clamp(Index, 0, WeatherDescriptions.Num());
  return WeatherDescriptions[Index];
}

void UCarlaSettings::ResetCameraDescriptions()
{
  CameraDescriptions.Empty();
  bSemanticSegmentationEnabled = false;
}

void UCarlaSettings::LoadSettingsFromFile(const FString &FilePath, const bool bLogOnFailure)
{
  if (FPaths::FileExists(FilePath)) {
    UE_LOG(LogCarla, Log, TEXT("Loading CARLA settings from \"%s\""), *FilePath);
    ResetCameraDescriptions();
    const MyIniFile ConfigFile(FilePath);
    constexpr bool bLoadCarlaServerSection = true;
    LoadSettingsFromConfig(ConfigFile, *this, bLoadCarlaServerSection);
    CurrentFileName = FilePath;
  } else if (bLogOnFailure) {
    UE_LOG(LogCarla, Error, TEXT("Unable to find settings file \"%s\""), *FilePath);
  }
}
