// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaSettings.h"

#include "CommandLine.h"
#include "ConfigCacheIni.h"

#include <limits>

// INI file sections.
#define S_CARLA_SERVER                 TEXT("CARLA/Server")
#define S_CARLA_SCENECAPTURE           TEXT("CARLA/SceneCapture")

template <typename TARGET, typename SOURCE>
static void SafeCastTo(SOURCE source, TARGET &target)
{
  if ((source >= std::numeric_limits<TARGET>::lowest()) &&
      (source <= std::numeric_limits<TARGET>::max())) {
    target = static_cast<TARGET>(source);
  } else {
    UE_LOG(LogCarla, Error, TEXT("CarlaSettings: Type cast failed"));
  }
}

// =============================================================================
// -- INIFile ------------------------------------------------------------------
// =============================================================================

class INIFile {
public:

  explicit INIFile(const FString &FileName)
  {
    ConfigFile.Read(FileName);
  }

  void GetBool(const TCHAR* Section, const TCHAR* Key, bool &Target) const
  {
    bool Value;
    if (ConfigFile.GetBool(Section, Key, Value)) {
      Target = Value;
    }
  }

  template <typename T>
  void GetInt(const TCHAR* Section, const TCHAR* Key, T &Target) const
  {
    int64 Value;
    if (ConfigFile.GetInt64(Section, Key, Value)) {
      SafeCastTo<T>(Value, Target);
    }
  }

  void GetString(const TCHAR* Section, const TCHAR* Key, FString &Target) const
  {
    FString Value;
    if (ConfigFile.GetString(Section, Key, Value)) {
      Target = Value;
    }
  }

  void GetPostProcessEffect(const TCHAR* Section, const TCHAR* Key, EPostProcessEffect &Target) const
  {
    FString ValueString;
    if (ConfigFile.GetString(Section, Key, ValueString)) {
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

private:

  FConfigFile ConfigFile;
};

// =============================================================================
// -- Other static methods -----------------------------------------------------
// =============================================================================

static void GetCameraDescription(
    const INIFile &ConfigFile,
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

static void LoadSettingsFromFile(const FString &FileName, UCarlaSettings &Settings)
{
  UE_LOG(LogCarla, Log, TEXT("Loading settings from \"%s\""), *FileName);
  INIFile ConfigFile(FileName);
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
  }
}

static bool GetSettingsFileName(FString &Value)
{
  // Try to get it from the command-line arguments.
  if (FParse::Value(FCommandLine::Get(), TEXT("-carla-settings="), Value)) {
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

void UCarlaSettings::LogSettings()
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
