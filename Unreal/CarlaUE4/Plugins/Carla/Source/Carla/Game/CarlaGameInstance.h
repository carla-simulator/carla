// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Engine/GameInstance.h"

#include "Carla/Game/CarlaEngine.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Server/CarlaServer.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/MapLayer.h>
#include <carla/rpc/OpendriveGenerationParameters.h>
#include <compiler/enable-ue4-macros.h>

#include "CarlaGameInstance.generated.h"

class UCarlaSettings;

/// The game instance contains elements that must be kept alive in between
/// levels. It is instantiate once per game.
UCLASS()
class CARLA_API UCarlaGameInstance : public UGameInstance
{
  GENERATED_BODY()

public:

  UCarlaGameInstance();

  ~UCarlaGameInstance();

  UCarlaSettings &GetCarlaSettings()
  {
    check(CarlaSettings != nullptr);
    return *CarlaSettings;
  }

  const UCarlaSettings &GetCarlaSettings() const
  {
    check(CarlaSettings != nullptr);
    return *CarlaSettings;
  }

  // Extra overload just for blueprints.
  UFUNCTION(BlueprintCallable)
  UCarlaSettings *GetCARLASettings()
  {
    return CarlaSettings;
  }

  UFUNCTION(BlueprintCallable)
  UCarlaEpisode *GetCarlaEpisode()
  {
    return CarlaEngine.GetCurrentEpisode();
  }

  void NotifyInitGame()
  {
    CarlaEngine.NotifyInitGame(GetCarlaSettings());
  }

  void NotifyBeginEpisode(UCarlaEpisode &Episode)
  {
    CarlaEngine.NotifyBeginEpisode(Episode);
  }

  void NotifyEndEpisode()
  {
    CarlaEngine.NotifyEndEpisode();
  }

  const FCarlaServer &GetServer() const
  {
    return CarlaEngine.GetServer();
  }

  void SetOpendriveGenerationParameters(
      const carla::rpc::OpendriveGenerationParameters & Parameters)
  {
    GenerationParameters = Parameters;
  }

  const carla::rpc::OpendriveGenerationParameters&
      GetOpendriveGenerationParameters() const
  {
    return GenerationParameters;
  }

  UFUNCTION(Category = "Carla Game Instance", BlueprintCallable)
  void SetMapLayer(int32 MapLayer)
  {
    CurrentMapLayer = MapLayer;
  }

  UFUNCTION(Category = "Carla Game Instance", BlueprintCallable)
  int32 GetCurrentMapLayer() const
  {
    return CurrentMapLayer;
  }

  UFUNCTION(BlueprintCallable)
  void SetDefaultMapPath() {
    // Read the config file
    FConfigFile ConfigFile = FConfigFile();
    FString configFStr = FPaths::ProjectDir();
    configFStr += "Config/DefaultEngine.ini";
    ConfigFile.Read(configFStr);

    // Depending on where we are, set the editor or game default map
#ifdef UE_EDITOR
    ConfigFile.GetString(TEXT("/Script/EngineSettings.GameMapsSettings"), TEXT("EditorStartupMap"), _MapPath);
#else
    ConfigFile.GetString(TEXT("/Script/EngineSettings.GameMapsSettings"), TEXT("GameDefaultMap"), _MapPath);
#endif

    // Format and convert the path to absolute
    _MapPath.RemoveFromStart(TEXT("/Game/"));
    _MapPath = FPaths::ProjectContentDir() + _MapPath;
    _MapPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*_MapPath);
    _MapPath = FPaths::GetBaseFilename(_MapPath, false);
  }

  UFUNCTION(BlueprintCallable)
  void SetMapPath(const FString &MapPath)
  {
    _MapPath = MapPath;
  }

  UFUNCTION(BlueprintCallable)
  const FString &GetMapPath() const
  {
    return _MapPath;
  }

private:

  UPROPERTY(Category = "CARLA Settings", EditAnywhere)
  UCarlaSettings *CarlaSettings = nullptr;

  FCarlaEngine CarlaEngine;

  UPROPERTY()
  ACarlaRecorder *Recorder = nullptr;

  carla::rpc::OpendriveGenerationParameters GenerationParameters;

  UPROPERTY(Category = "CARLA Game Instance", EditAnywhere)
  int32 CurrentMapLayer = static_cast<int32>(carla::rpc::MapLayer::All);

  UPROPERTY()
  FString _MapPath;

};
