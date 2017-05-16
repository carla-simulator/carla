// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaSettings.h"

#include "CommandLine.h"
#include "IniFile.h"

// INI file sections.
#define S_CARLA_SERVER                 TEXT("CARLA/Server")
#define S_CARLA_SCENECAPTURE           TEXT("CARLA/SceneCapture")

// =============================================================================
// -- MyIniFile ----------------------------------------------------------------
// =============================================================================

class MyIniFile : public IniFile {
public:

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

static void LoadSettingsFromFile(const FString &FileName, UCarlaSettings &Settings)
{
  UE_LOG(LogCarla, Log, TEXT("Loading settings from \"%s\""), *FileName);
  MyIniFile ConfigFile(FileName);
  // CarlaServer.
  ConfigFile.GetBool(S_CARLA_SERVER, TEXT("UseNetworking"), Settings.bUseNetworking);
  ConfigFile.GetInt(S_CARLA_SERVER, TEXT("WorldPort"), Settings.WorldPort);
  ConfigFile.GetInt(S_CARLA_SERVER, TEXT("WritePort"), Settings.WritePort);
  ConfigFile.GetInt(S_CARLA_SERVER, TEXT("ReadPort"), Settings.ReadPort);
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

static bool GetSettingsFileName(FString &Value)
{
  // Try to get it from the command-line arguments.
  if (FParse::Value(FCommandLine::Get(), TEXT("-carla-settings="), Value)) {
    if (FPaths::IsRelative(Value)) {
      Value = FPaths::ConvertRelativePathToFull(FPaths::LaunchDir(), Value);
    }
    if (FPaths::FileExists(Value)) {
      return true;
    }
    UE_LOG(LogCarla, Error, TEXT("Unable to find settings file \"%s\", falling back to default values"), *Value);
  }
  // If fails, check if there is one in the config folder.
  Value = FPaths::Combine(FPaths::GameConfigDir(), TEXT("CarlaSettings.ini"));
  if (FPaths::FileExists(Value)) {
    return true;
  }
  return false;
}

// =============================================================================
// -- UCarlaSettings -----------------------------------------------------------
// =============================================================================

void UCarlaSettings::LoadSettings()
{
  FString FileName;
  if (GetSettingsFileName(FileName)) {
    LoadSettingsFromFile(FileName, *this);
    CurrentFileName = FileName;
  } else {
    CurrentFileName = TEXT("");
  }
  // Override settings from command-line.
  uint32 Value;
  if (FParse::Value(FCommandLine::Get(), TEXT("-world-port="), Value)) {
    WorldPort = Value;
    WritePort = Value + 1u;
    ReadPort = Value + 2u;
  }
}

void UCarlaSettings::LogSettings() const
{
  UE_LOG(LogCarla, Log, TEXT("== CARLA Settings =============================================================="));
  UE_LOG(LogCarla, Log, TEXT("Settings file: %s"), *CurrentFileName);
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SERVER);
  UE_LOG(LogCarla, Log, TEXT("Use Networking = %s"), (bUseNetworking ? TEXT("true") : TEXT("false")));
  UE_LOG(LogCarla, Log, TEXT("World Port = %d"), WorldPort);
  UE_LOG(LogCarla, Log, TEXT("Write Port = %d"), WritePort);
  UE_LOG(LogCarla, Log, TEXT("Read Port = %d"), ReadPort);
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SCENECAPTURE);
  UE_LOG(LogCarla, Log, TEXT("Added %d cameras."), CameraDescriptions.Num());
  UE_LOG(LogCarla, Log, TEXT("Semantic Segmentation = %s"), (bSemanticSegmentationEnabled ? TEXT("enabled") : TEXT("disabled")));
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
#undef S_CARLA_SCENECAPTURE
