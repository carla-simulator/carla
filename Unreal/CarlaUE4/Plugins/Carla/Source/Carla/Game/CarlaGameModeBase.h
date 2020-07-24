// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/CarlaActorFactory.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Recorder/CarlaRecorder.h"
#include "Carla/Game/TaggerDelegate.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla/Sensor/SceneCaptureSensor.h"
#include "Carla/Settings/CarlaSettingsDelegate.h"
#include "Carla/Weather/Weather.h"
#include "Carla/Traffic/TrafficLightManager.h"

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include <compiler/disable-ue4-macros.h>
#include <boost/optional.hpp>
#include <compiler/enable-ue4-macros.h>

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

  UFUNCTION(Exec, Category = "CARLA Game Mode")
  void DebugShowSignals(bool enable);

  UFUNCTION(BlueprintCallable, Category = "CARLA Game Mode")
  ATrafficLightManager* GetTrafficLightManager();

  void AddSceneCaptureSensor(ASceneCaptureSensor* SceneCaptureSensor);

  void RemoveSceneCaptureSensor(ASceneCaptureSensor* SceneCaptureSensor);

  bool IsCameraAtlasTextureValid() const
  {
    return IsAtlasTextureValid;
  }

  const TArray<FColor>& GetCurrentAtlasPixels() const{
    return AtlasPixels[CurrentAtlas];
  }

  FTexture2DRHIRef GetCurrentCamerasAtlasTexture() const{
    return CamerasAtlasTexture[CurrentAtlas];
  }

  uint32 GetAtlasTextureWidth() const {
    return AtlasTextureWidth;
  }

  uint32 GetAtlasTextureHeight() const {
    return AtlasTextureHeight;
  }

  UFUNCTION(Exec)
  void SwitchReadSurface(uint32 Mode) {
#if !UE_BUILD_SHIPPING
    ReadSurface = Mode;
#endif
  }

  UFUNCTION(Exec)
  void SetAtlasSurface(uint32 W, uint32 H) {
#if !UE_BUILD_SHIPPING
    SurfaceW = W;
    SurfaceH = H;
#endif
  }

protected:

  void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;

  void RestartPlayer(AController *NewPlayer) override;

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void Tick(float DeltaSeconds) override;

private:

  void SpawnActorFactories();

  void ParseOpenDrive(const FString &MapName);

  void CreateAtlasTextures();

  void CaptureAtlas();

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

  /// The class of Weather to spawn.
  UPROPERTY(Category = "CARLA Game Mode", EditAnywhere)
  TSubclassOf<AWeather> WeatherClass;

  /// List of actor spawners that will be used to define and spawn the actors
  /// available in game.
  UPROPERTY(Category = "CARLA Game Mode", EditAnywhere)
  TSet<TSubclassOf<ACarlaActorFactory>> ActorFactories;

  UPROPERTY()
  TArray<ACarlaActorFactory *> ActorFactoryInstances;

  UPROPERTY()
  ATrafficLightManager* TrafficLightManager = nullptr;

  boost::optional<carla::road::Map> Map;

  FDelegateHandle CaptureAtlasDelegate;

  static const uint32 kMaxNumTextures = 2u; // This has to be POT
  TArray<FColor> AtlasPixels[kMaxNumTextures];
  TArray<ASceneCaptureSensor*> SceneCaptureSensors;
  FTexture2DRHIRef CamerasAtlasTexture[kMaxNumTextures];
  uint32 AtlasTextureWidth = 0u;
  uint32 AtlasTextureHeight = 0u;
  uint32 CurrentAtlasTextureWidth = 0u;
  uint32 CurrentAtlas = 0u;
  uint32 PreviousAtlas = 0u;
  bool IsAtlasTextureValid = false;

#if !UE_BUILD_SHIPPING
  uint32 ReadSurface = 1;
  uint32 SurfaceW = 0;
  uint32 SurfaceH = 0;
#endif

};
