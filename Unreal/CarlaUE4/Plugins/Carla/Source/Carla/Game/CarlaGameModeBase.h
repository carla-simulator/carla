// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include <compiler/disable-ue4-macros.h>
#include <boost/optional.hpp>
#include <carla/rpc/Texture.h>
#include <carla/rpc/MaterialParameter.h>
#include <compiler/enable-ue4-macros.h>

#include "Carla/Actor/CarlaActorFactory.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/TaggerDelegate.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Sensor/SceneCaptureSensor.h"
#include "Carla/Settings/CarlaSettingsDelegate.h"
#include "Carla/Traffic/TrafficLightManager.h"
#include "Carla/Util/ObjectRegister.h"
#include "Carla/Weather/Weather.h"
#include "MapGen/LargeMapManager.h"

#include "CarlaGameModeBase.generated.h"

/// Base class for the CARLA Game Mode.
UCLASS(HideCategories=(ActorTick))
class CARLA_API ACarlaGameModeBase : public AGameModeBase
{
  GENERATED_BODY()

public:

  ACarlaGameModeBase(const FObjectInitializer& ObjectInitializer);

  const UCarlaEpisode &GetCarlaEpisode() const
  {
    check(Episode != nullptr);
    return *Episode;
  }

  const boost::optional<carla::road::Map>& GetMap() const {
    return Map;
  }

  const FString GetFullMapPath() const;

  // get path relative to Content folder
  const FString GetRelativeMapPath() const;

  UFUNCTION(Exec, Category = "CARLA Game Mode")
  void DebugShowSignals(bool enable);

  UFUNCTION(BlueprintCallable, Category = "CARLA Game Mode")
  ATrafficLightManager* GetTrafficLightManager();

  UFUNCTION(Category = "Carla Game Mode", BlueprintCallable)
  const TArray<FTransform>& GetSpawnPointsTransforms() const{
    return SpawnPointsTransforms;
  }

  UFUNCTION(Category = "Carla Game Mode", BlueprintCallable, CallInEditor, Exec)
  TArray<FBoundingBox> GetAllBBsOfLevel(uint8 TagQueried = 0xFF) const;

  UFUNCTION(Category = "Carla Game Mode", BlueprintCallable, CallInEditor, Exec)
  TArray<FEnvironmentObject> GetEnvironmentObjects(uint8 QueriedTag = 0xFF) const
  {
    return ObjectRegister->GetEnvironmentObjects(QueriedTag);
  }

  void EnableEnvironmentObjects(const TSet<uint64>& EnvObjectIds, bool Enable);

  void EnableOverlapEvents();

  void CheckForEmptyMeshes();

  UFUNCTION(Category = "Carla Game Mode", BlueprintCallable, CallInEditor, Exec)
  void LoadMapLayer(int32 MapLayers);

  UFUNCTION(Category = "Carla Game Mode", BlueprintCallable, CallInEditor, Exec)
  void UnLoadMapLayer(int32 MapLayers);

  UFUNCTION(Category = "Carla Game Mode")
  ULevel* GetULevelFromName(FString LevelName);

  UFUNCTION(BlueprintCallable, Category = "Carla Game Mode")
  void OnLoadStreamLevel();

  UFUNCTION(BlueprintCallable, Category = "Carla Game Mode")
  void OnUnloadStreamLevel();

  ALargeMapManager* GetLMManager() const {
    return LMManager;
  }

  AActor* FindActorByName(const FString& ActorName);

  UTexture2D* CreateUETexture(const carla::rpc::TextureColor& Texture);
  UTexture2D* CreateUETexture(const carla::rpc::TextureFloatColor& Texture);

  void ApplyTextureToActor(
      AActor* Actor,
      UTexture2D* Texture,
      const carla::rpc::MaterialParameter& TextureParam);

  TArray<FString> GetNamesOfAllActors();

protected:

  void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;

  void RestartPlayer(AController *NewPlayer) override;

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void Tick(float DeltaSeconds) override;

private:

  void SpawnActorFactories();

  void StoreSpawnPoints();

  void GenerateSpawnPoints();

  void ParseOpenDrive();

  void RegisterEnvironmentObjects();

  void ConvertMapLayerMaskToMapNames(int32 MapLayer, TArray<FName>& OutLevelNames);

  void OnEpisodeSettingsChanged(const FEpisodeSettings &Settings);

  UPROPERTY()
  UCarlaGameInstance *GameInstance = nullptr;

  UPROPERTY()
  UTaggerDelegate *TaggerDelegate = nullptr;

  UPROPERTY()
  UCarlaSettingsDelegate *CarlaSettingsDelegate = nullptr;

  UPROPERTY()
  UCarlaEpisode *Episode = nullptr;

  UPROPERTY()
  ACarlaRecorder *Recorder = nullptr;

  UPROPERTY()
  UObjectRegister* ObjectRegister = nullptr;

  /// The class of Weather to spawn.
  UPROPERTY(Category = "CARLA Game Mode", EditAnywhere)
  TSubclassOf<AWeather> WeatherClass;

  /// List of actor spawners that will be used to define and spawn the actors
  /// available in game.
  UPROPERTY(Category = "CARLA Game Mode", EditAnywhere)
  TSet<TSubclassOf<ACarlaActorFactory>> ActorFactories;

  UPROPERTY()
  TArray<FTransform> SpawnPointsTransforms;

  UPROPERTY()
  TArray<ACarlaActorFactory *> ActorFactoryInstances;

  UPROPERTY()
  ATrafficLightManager* TrafficLightManager = nullptr;

  ALargeMapManager* LMManager = nullptr;

  FDelegateHandle OnEpisodeSettingsChangeHandle;

  boost::optional<carla::road::Map> Map;

  int PendingLevelsToLoad = 0;
  int PendingLevelsToUnLoad = 0;

  bool ReadyToRegisterObjects = false;

  // We keep a global uuid to allow the load/unload layer methods to be called
  // in the same tick
  int32 LatentInfoUUID = 0;

};
