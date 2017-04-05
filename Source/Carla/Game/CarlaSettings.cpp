// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaSettings.h"

#include "CommandLine.h"
#include "ConfigCacheIni.h"

#include <limits>

// INI file sections.
#define S_CARLA_SERVER                 TEXT("CARLA/Server")
#define S_CARLA_SCENECAPTURE           TEXT("CARLA/SceneCapture")
#define S_CARLA_SCENECAPTURE_MONO      TEXT("CARLA/SceneCapture/Mono")
#define S_CARLA_SCENECAPTURE_STEREO    TEXT("CARLA/SceneCapture/Stereo")

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

  void GetSceneCaptureMode(const TCHAR* Section, const TCHAR* Key, ESceneCaptureMode &Target) const
  {
    FString ModeString;
    if (ConfigFile.GetString(Section, Key, ModeString)) {
      if (ModeString == "Stereo") {
        Target = ESceneCaptureMode::Stereo;
      } else if (ModeString == "Mono") {
        Target = ESceneCaptureMode::Mono;
      } else if (ModeString == "NoCapture") {
        Target = ESceneCaptureMode::NoCapture;
      }
    }
  }

private:

  FConfigFile ConfigFile;
};

// =============================================================================
// -- Other static methods -----------------------------------------------------
// =============================================================================

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
  ConfigFile.GetSceneCaptureMode(S_CARLA_SCENECAPTURE, TEXT("Mode"), Settings.SceneCaptureMode);
  // SceneCapture - Mono.
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE, TEXT("ImageSizeX"), Settings.Mono_ImageSizeX);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE, TEXT("ImageSizeY"), Settings.Mono_ImageSizeY);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_MONO, TEXT("ImageSizeX"), Settings.Mono_ImageSizeX);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_MONO, TEXT("ImageSizeY"), Settings.Mono_ImageSizeY);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_MONO, TEXT("CameraPositionX"), Settings.Mono_CameraPosition.X);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_MONO, TEXT("CameraPositionY"), Settings.Mono_CameraPosition.Y);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_MONO, TEXT("CameraPositionZ"), Settings.Mono_CameraPosition.Z);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_MONO, TEXT("CameraRotationPitch"), Settings.Mono_CameraRotation.Pitch);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_MONO, TEXT("CameraRotationRoll"), Settings.Mono_CameraRotation.Roll);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_MONO, TEXT("CameraRotationYaw"), Settings.Mono_CameraRotation.Yaw);
  // SceneCapture - Stereo.
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE, TEXT("ImageSizeX"), Settings.Stereo_ImageSizeX);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE, TEXT("ImageSizeY"), Settings.Stereo_ImageSizeY);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("ImageSizeX"), Settings.Stereo_ImageSizeX);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("ImageSizeY"), Settings.Stereo_ImageSizeY);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera0PositionX"), Settings.Stereo_Camera0Position.X);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera0PositionY"), Settings.Stereo_Camera0Position.Y);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera0PositionZ"), Settings.Stereo_Camera0Position.Z);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera1PositionX"), Settings.Stereo_Camera1Position.X);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera1PositionY"), Settings.Stereo_Camera1Position.Y);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera1PositionZ"), Settings.Stereo_Camera1Position.Z);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera0RotationPitch"), Settings.Stereo_Camera0Rotation.Pitch);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera0RotationRoll"), Settings.Stereo_Camera0Rotation.Roll);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera0RotationYaw"), Settings.Stereo_Camera0Rotation.Yaw);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera1RotationPitch"), Settings.Stereo_Camera1Rotation.Pitch);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera1RotationRoll"), Settings.Stereo_Camera1Rotation.Roll);
  ConfigFile.GetInt(S_CARLA_SCENECAPTURE_STEREO, TEXT("Camera1RotationYaw"), Settings.Stereo_Camera1Rotation.Yaw);
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
  auto ModeAsString = [](ESceneCaptureMode Mode) {
    switch (Mode) {
      case ESceneCaptureMode::Mono:      return TEXT("Mono");
      case ESceneCaptureMode::Stereo:    return TEXT("Stereo");
      case ESceneCaptureMode::NoCapture: return TEXT("NoCapture");
      default:                           return TEXT("INVALID");
    };
  };
  UE_LOG(LogCarla, Log, TEXT("== CARLA Settings =============================================================="));
  UE_LOG(LogCarla, Log, TEXT("Settings file: %s"), *CurrentFileName);
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SERVER);
  UE_LOG(LogCarla, Log, TEXT("Use Networking = %s"), (bUseNetworking ? TEXT("true") : TEXT("false")));
  UE_LOG(LogCarla, Log, TEXT("World Port = %d"), WorldPort);
  UE_LOG(LogCarla, Log, TEXT("Write Port = %d"), WritePort);
  UE_LOG(LogCarla, Log, TEXT("Read Port = %d"), ReadPort);
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SCENECAPTURE);
  UE_LOG(LogCarla, Log, TEXT("Mode = %s"), ModeAsString(SceneCaptureMode));
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SCENECAPTURE_MONO);
  UE_LOG(LogCarla, Log, TEXT("Image Size = %dx%d"), Mono_ImageSizeX, Mono_ImageSizeY);
  UE_LOG(LogCarla, Log, TEXT("Camera Position = (%s)"), *Mono_CameraPosition.ToString());
  UE_LOG(LogCarla, Log, TEXT("Camera Rotation = (%s)"), *Mono_CameraRotation.ToString());
  UE_LOG(LogCarla, Log, TEXT("[%s]"), S_CARLA_SCENECAPTURE_STEREO);
  UE_LOG(LogCarla, Log, TEXT("ImageSize = %dx%d"), Stereo_ImageSizeX, Stereo_ImageSizeY);
  UE_LOG(LogCarla, Log, TEXT("Camera0 Position = (%s)"), *Stereo_Camera0Position.ToString());
  UE_LOG(LogCarla, Log, TEXT("Camera0 Rotation = (%s)"), *Stereo_Camera0Rotation.ToString());
  UE_LOG(LogCarla, Log, TEXT("Camera1 Position = (%s)"), *Stereo_Camera1Position.ToString());
  UE_LOG(LogCarla, Log, TEXT("Camera1 Rotation = (%s)"), *Stereo_Camera1Rotation.ToString());
  UE_LOG(LogCarla, Log, TEXT("================================================================================"));
}

#undef S_CARLA_SERVER
#undef S_CARLA_SCENECAPTURE
#undef S_CARLA_SCENECAPTURE_MONO
#undef S_CARLA_SCENECAPTURE_STEREO
