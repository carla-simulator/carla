// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaSettings.h"

#include "CommandLine.h"
#include "ConfigCacheIni.h"

#include <limits>

template <typename TARGET, typename SOURCE>
static void SafeCastTo(SOURCE source, TARGET &target)
{
  if ((source >= std::numeric_limits<TARGET>::min()) &&
      (source <= std::numeric_limits<TARGET>::max())) {
    target = static_cast<TARGET>(source);
  }
}

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

#define CARLA_SERVER_SECTION       TEXT("CARLA/Server")
#define CARLA_SCENECAPTURE_SECTION TEXT("CARLA/SceneCapture")
static void LoadSettingsFromFile(const FString &FileName, UCarlaSettings &Settings)
{
  UE_LOG(LogCarla, Log, TEXT("Loading settings from \"%s\""), *FileName);
  INIFile ConfigFile(FileName);
  ConfigFile.GetBool(CARLA_SERVER_SECTION, TEXT("UseNetworking"), Settings.bUseNetworking);
  ConfigFile.GetInt(CARLA_SERVER_SECTION, TEXT("WorldPort"), Settings.WorldPort);
  ConfigFile.GetInt(CARLA_SERVER_SECTION, TEXT("WritePort"), Settings.WritePort);
  ConfigFile.GetInt(CARLA_SERVER_SECTION, TEXT("ReadPort"), Settings.ReadPort);
  ConfigFile.GetSceneCaptureMode(CARLA_SCENECAPTURE_SECTION, TEXT("Mode"), Settings.SceneCaptureMode);
  ConfigFile.GetInt(CARLA_SCENECAPTURE_SECTION, TEXT("ImageSizeX"), Settings.ImageSizeX);
  ConfigFile.GetInt(CARLA_SCENECAPTURE_SECTION, TEXT("ImageSizeY"), Settings.ImageSizeY);
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
  UE_LOG(LogCarla, Log, TEXT("[%s]"), CARLA_SERVER_SECTION);
  UE_LOG(LogCarla, Log, TEXT("UseNetworking=%s"), (bUseNetworking ? TEXT("true") : TEXT("false")));
  UE_LOG(LogCarla, Log, TEXT("WorldPort=%d"), WorldPort);
  UE_LOG(LogCarla, Log, TEXT("WritePort=%d"), WritePort);
  UE_LOG(LogCarla, Log, TEXT("ReadPort=%d"), ReadPort);
  UE_LOG(LogCarla, Log, TEXT("[%s]"), CARLA_SCENECAPTURE_SECTION);
  UE_LOG(LogCarla, Log, TEXT("Mode=%s"), ModeAsString(SceneCaptureMode));
  UE_LOG(LogCarla, Log, TEXT("ImageSizeX=%d"), ImageSizeX);
  UE_LOG(LogCarla, Log, TEXT("ImageSizeY=%d"), ImageSizeY);
  UE_LOG(LogCarla, Log, TEXT("================================================================================"));
}

#undef CARLA_SERVER_SECTION
#undef CARLA_SCENECAPTURE_SECTION
