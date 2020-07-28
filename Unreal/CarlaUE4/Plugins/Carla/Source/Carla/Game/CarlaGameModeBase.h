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


USTRUCT()
struct FAtlasCopyRequest
{
  GENERATED_BODY()

  void ResizeBuffer(uint32 Width, uint32 Height)
  {
    if(AtlasTextureWidth != Width || AtlasTextureHeight != Height)
    {
      UE_LOG(LogCarla, Warning, TEXT("FAtlasCopyRequest::ResizeBuffer %dx%d"), Width, Height);
      AtlasTextureWidth = Width;
      AtlasTextureHeight = Height;
      AtlasImage.Init(FColor(), Width * Height);
    }
  }

  void Start()
  {
    RenderFence.BeginFence(true);
  }

  void Wait()
  {
    RenderFence.Wait(false);
  }

  bool IsComplete()
  {
    return RenderFence.IsFenceComplete();
  }

  TArray<FColor> AtlasImage;
  FRenderCommandFence RenderFence;
  uint32 AtlasTextureWidth = 0u;
  uint32 AtlasTextureHeight = 0u;
};


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
  void SwitchReadSurfaceMode(uint32 Mode) {
#if !UE_BUILD_SHIPPING
    ReadSurfaceMode = Mode;
#endif
  }

  UFUNCTION(Exec)
  void SetAtlasSurface(uint32 W, uint32 H) {
#if !UE_BUILD_SHIPPING
    SurfaceW = W;
    SurfaceH = H;
#endif
  }

  UFUNCTION(Exec)
  void EnableCameraCopyToAtlas(bool Enable) {
#if !UE_BUILD_SHIPPING
    CameraCopyToAtlasEnable = Enable;
#endif
  }

  UFUNCTION(Exec)
  void EnableAtlasCopyToCamera(bool Enable) {
#if !UE_BUILD_SHIPPING
    AtlasCopyToCamera = Enable;
#endif
  }

  UFUNCTION(Exec)
  void EnableCameraStream(bool Enable) {
#if !UE_BUILD_SHIPPING
    CameraStreamEnable = Enable;
#endif
  }

#if !UE_BUILD_SHIPPING

  bool IsCameraCopyToAtlasEnabled() const {
    return CameraCopyToAtlasEnable;
  }

  bool IsCameraStreamEnabled() const {
    return CameraStreamEnable;
  }

#endif

protected:

  void InitGame(const FString &MapName, const FString &Options, FString &ErrorMessage) override;

  void RestartPlayer(AController *NewPlayer) override;

  void BeginPlay() override;

  void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

  void Tick(float DeltaSeconds) override;

private:

  void SpawnActorFactories();

  void ParseOpenDrive(const FString &MapName);

  FAtlasCopyRequest* GetAtlasCopyRequest();

  void CreateAtlasTextures();

  void CaptureAtlas();

  void SendAtlas();

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
  FDelegateHandle SendAtlasDelegate;

  TQueue<FAtlasCopyRequest*> AtlasCopyRequestQueue;
  TQueue<FAtlasCopyRequest*> AtlasCopyRequestsQueuePool;

  static const uint32 kMaxNumTextures = 2u; // This has to be POT
  // FRenderCommandFence RenderFence[kMaxNumTextures];
  TArray<FColor> AtlasPixels[kMaxNumTextures]; // TODO: remove
  TArray<ASceneCaptureSensor*> SceneCaptureSensors;
  FTexture2DRHIRef CamerasAtlasTexture[kMaxNumTextures];
  uint32 AtlasTextureWidth = 0u;
  uint32 AtlasTextureHeight = 0u;
  uint32 CurrentAtlasTextureWidth = 0u;
  uint32 CurrentAtlas = 0u;
  uint32 PreviousAtlas = 0u;
  bool IsAtlasTextureValid = false;

#if !UE_BUILD_SHIPPING
  uint32 ReadSurfaceMode = 1;
  uint32 SurfaceW = 0;
  uint32 SurfaceH = 0;
  bool CameraCopyToAtlasEnable = true;
  bool AtlasCopyToCamera = true;
  bool CameraStreamEnable = true;
#endif

};
