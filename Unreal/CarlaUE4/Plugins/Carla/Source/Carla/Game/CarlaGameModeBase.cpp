// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla/Game/CarlaHUD.h"
#include "Engine/DecalActor.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/WeatherParameters.h>
#include "carla/opendrive/OpenDriveParser.h"
#include "carla/road/element/RoadInfoSignal.h"
#include <compiler/enable-ue4-macros.h>

#include "Async/ParallelFor.h"
#include "DynamicRHI.h"

#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"

namespace cr = carla::road;
namespace cre = carla::road::element;

ACarlaGameModeBase::ACarlaGameModeBase(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;
  bAllowTickBeforeBeginPlay = false;

  Episode = CreateDefaultSubobject<UCarlaEpisode>(TEXT("Episode"));

  Recorder = CreateDefaultSubobject<ACarlaRecorder>(TEXT("Recorder"));

  // HUD
  HUDClass = ACarlaHUD::StaticClass();

  TaggerDelegate = CreateDefaultSubobject<UTaggerDelegate>(TEXT("TaggerDelegate"));
  CarlaSettingsDelegate = CreateDefaultSubobject<UCarlaSettingsDelegate>(TEXT("CarlaSettingsDelegate"));
}

void ACarlaGameModeBase::AddSceneCaptureSensor(ASceneCaptureSensor* SceneCaptureSensor)
{
  uint32 ImageWidth = SceneCaptureSensor->ImageWidth;
  uint32 ImageHeight = SceneCaptureSensor->ImageHeight;

  if(AtlasTextureWidth < ImageWidth)
  {
    IsAtlasTextureValid = false;
    AtlasTextureWidth = ImageWidth;
  }

  if(AtlasTextureHeight < (CurrentAtlasTextureHeight + ImageHeight) )
  {
    IsAtlasTextureValid = false;
    AtlasTextureHeight = CurrentAtlasTextureHeight + ImageHeight;
  }

  SceneCaptureSensor->PositionInAtlas = FIntVector(0, CurrentAtlasTextureHeight, 0);
  CurrentAtlasTextureHeight += ImageHeight;

  SceneCaptureSensors.Add(SceneCaptureSensor);

  UE_LOG(LogCarla, Warning, TEXT("ACarlaGameModeBase::AddSceneCaptureSensor %d %dx%d"), SceneCaptureSensors.Num(), AtlasTextureWidth, AtlasTextureHeight);
}

void ACarlaGameModeBase::RemoveSceneCaptureSensor(ASceneCaptureSensor* SceneCaptureSensor)
{
  // Remove camera
  SceneCaptureSensors.Remove(SceneCaptureSensor);

  // Recalculate PositionInAtlas for each camera
  CurrentAtlasTextureHeight = 0;
  for(ASceneCaptureSensor* Camera :  SceneCaptureSensors)
  {
    Camera->PositionInAtlas = FIntVector(0, CurrentAtlasTextureHeight, 0);
    CurrentAtlasTextureHeight += Camera->ImageHeight;
  }
}

void ACarlaGameModeBase::InitGame(
    const FString &MapName,
    const FString &Options,
    FString &ErrorMessage)
{
  Super::InitGame(MapName, Options, ErrorMessage);

  checkf(
      Episode != nullptr,
      TEXT("Missing episode, can't continue without an episode!"));

#if WITH_EDITOR
    {
      // When playing in editor the map name gets an extra prefix, here we
      // remove it.
      FString CorrectedMapName = MapName;
      constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
      CorrectedMapName.RemoveFromStart(PIEPrefix);
      UE_LOG(LogCarla, Log, TEXT("Corrected map name from %s to %s"), *MapName, *CorrectedMapName);
      Episode->MapName = CorrectedMapName;
    }
#else
  Episode->MapName = MapName;
#endif // WITH_EDITOR

  auto World = GetWorld();
  check(World != nullptr);

  GameInstance = Cast<UCarlaGameInstance>(GetGameInstance());
  checkf(
      GameInstance != nullptr,
      TEXT("GameInstance is not a UCarlaGameInstance, did you forget to set "
           "it in the project settings?"));

  if (TaggerDelegate != nullptr) {
    TaggerDelegate->RegisterSpawnHandler(World);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Missing TaggerDelegate!"));
  }

  if(CarlaSettingsDelegate != nullptr) {
    CarlaSettingsDelegate->ApplyQualityLevelPostRestart();
    CarlaSettingsDelegate->RegisterSpawnHandler(World);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Missing CarlaSettingsDelegate!"));
  }

  if (WeatherClass != nullptr) {
    Episode->Weather = World->SpawnActor<AWeather>(WeatherClass);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Missing weather class!"));
  }

  GameInstance->NotifyInitGame();

  SpawnActorFactories();

  // make connection between Episode and Recorder
  Recorder->SetEpisode(Episode);
  Episode->SetRecorder(Recorder);

  ParseOpenDrive(MapName);
}

void ACarlaGameModeBase::RestartPlayer(AController *NewPlayer)
{
  if (CarlaSettingsDelegate != nullptr)
  {
    CarlaSettingsDelegate->ApplyQualityLevelPreRestart();
  }

  Super::RestartPlayer(NewPlayer);
}

void ACarlaGameModeBase::BeginPlay()
{
  Super::BeginPlay();

  if (true) { /// @todo If semantic segmentation enabled.
    check(GetWorld() != nullptr);
    ATagger::TagActorsInLevel(*GetWorld(), true);
    TaggerDelegate->SetSemanticSegmentationEnabled();
  }

  // HACK: fix transparency see-through issues
  // The problem: transparent objects are visible through walls.
  // This is due to a weird interaction between the SkyAtmosphere component,
  // the shadows of a directional light (the sun)
  // and the custom depth set to 3 used for semantic segmentation
  // The solution: Spawn a Decal.
  // It just works!
  GetWorld()->SpawnActor<ADecalActor>(
      FVector(0,0,-1000000), FRotator(0,0,0), FActorSpawnParameters());

  ATrafficLightManager* Manager = GetTrafficLightManager();
  Manager->InitializeTrafficLights();

  Episode->InitializeAtBeginPlay();
  GameInstance->NotifyBeginEpisode(*Episode);

  if (Episode->Weather != nullptr)
  {
    Episode->Weather->ApplyWeather(carla::rpc::WeatherParameters::Default);
  }

  /// @todo Recorder should not tick here, FCarlaEngine should do it.
  // check if replayer is waiting to autostart
  if (Recorder)
  {
    Recorder->GetReplayer()->CheckPlayAfterMapLoaded();
  }

  // OnBeginFrame
  // OnEndFrame
  CaptureAtlasDelegate = FCoreDelegates::OnEndFrame.AddUObject(this, &ACarlaGameModeBase::CaptureAtlas);
  SendAtlasDelegate = FCoreDelegates::OnEndFrame.AddUObject(this, &ACarlaGameModeBase::SendAtlas);

}

void ACarlaGameModeBase::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  /// @todo Recorder should not tick here, FCarlaEngine should do it.
  if (Recorder)
  {
    Recorder->Tick(DeltaSeconds);
  }

}

void ACarlaGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Episode->EndPlay();
  GameInstance->NotifyEndEpisode();

  Super::EndPlay(EndPlayReason);

  if ((CarlaSettingsDelegate != nullptr) && (EndPlayReason != EEndPlayReason::EndPlayInEditor))
  {
    CarlaSettingsDelegate->Reset();
  }

  // TODO: improve this delete processs; not multithread friendly
  while(!AtlasCopyRequestQueue.IsEmpty())
  {
    delete AtlasCopyRequestQueue.Peek();
    AtlasCopyRequestQueue.Pop();
  }

  while(!AtlasCopyRequestsQueuePool.IsEmpty())
  {
    delete AtlasCopyRequestsQueuePool.Peek();
    AtlasCopyRequestsQueuePool.Pop();
  }


  // OnEndFrame
  FCoreDelegates::OnEndFrame.Remove(CaptureAtlasDelegate);
  FCoreDelegates::OnEndFrame.Remove(SendAtlasDelegate);
}

void ACarlaGameModeBase::SpawnActorFactories()
{
  auto *World = GetWorld();
  check(World != nullptr);

  for (auto &FactoryClass : ActorFactories)
  {
    if (FactoryClass != nullptr)
    {
      auto *Factory = World->SpawnActor<ACarlaActorFactory>(FactoryClass);
      if (Factory != nullptr)
      {
        Episode->RegisterActorFactory(*Factory);
        ActorFactoryInstances.Add(Factory);
      }
      else
      {
        UE_LOG(LogCarla, Error, TEXT("Failed to spawn actor spawner"));
      }
    }
  }
}

void ACarlaGameModeBase::ParseOpenDrive(const FString &MapName)
{
  std::string opendrive_xml = carla::rpc::FromLongFString(UOpenDrive::LoadXODR(MapName));
  Map = carla::opendrive::OpenDriveParser::Load(opendrive_xml);
  if (!Map.has_value()) {
    UE_LOG(LogCarla, Error, TEXT("Invalid Map"));
  }
  else
  {
    Episode->MapGeoReference = Map->GetGeoReference();
  }
}

ATrafficLightManager* ACarlaGameModeBase::GetTrafficLightManager()
{
  if (!TrafficLightManager)
  {
    AActor* TrafficLightManagerActor = UGameplayStatics::GetActorOfClass(GetWorld(), ATrafficLightManager::StaticClass());
    if(TrafficLightManagerActor == nullptr)
    {
      TrafficLightManager = GetWorld()->SpawnActor<ATrafficLightManager>();
    }
    else
    {
      TrafficLightManager = Cast<ATrafficLightManager>(TrafficLightManagerActor);
    }
  }
  return TrafficLightManager;
}

void ACarlaGameModeBase::DebugShowSignals(bool enable)
{

  auto World = GetWorld();
  check(World != nullptr);

  if(!Map)
  {
    return;
  }

  if(!enable)
  {
    UKismetSystemLibrary::FlushDebugStrings(World);
    UKismetSystemLibrary::FlushPersistentDebugLines(World);
    return;
  }

  //const std::unordered_map<carla::road::SignId, std::unique_ptr<carla::road::Signal>>
  const auto& Signals = Map->GetSignals();
  const auto& Controllers = Map->GetControllers();

  for(const auto& Signal : Signals) {
    const auto& ODSignal = Signal.second;
    const FTransform Transform = ODSignal->GetTransform();
    const FVector Location = Transform.GetLocation();
    const FQuat Rotation = Transform.GetRotation();
    const FVector Up = Rotation.GetUpVector();
    DrawDebugSphere(
      World,
      Location,
      50.0f,
      10,
      FColor(0, 255, 0),
      true
    );
  }

  TArray<const cre::RoadInfoSignal*> References;
  auto waypoints = Map->GenerateWaypointsOnRoadEntries();
  std::unordered_set<cr::RoadId> ExploredRoads;
  for (auto & waypoint : waypoints)
  {
    // Check if we already explored this road
    if (ExploredRoads.count(waypoint.road_id) > 0)
    {
      continue;
    }
    ExploredRoads.insert(waypoint.road_id);

    // Multiple times for same road (performance impact, not in behavior)
    auto SignalReferences = Map->GetLane(waypoint).
        GetRoad()->GetInfos<cre::RoadInfoSignal>();
    for (auto *SignalReference : SignalReferences)
    {
      References.Add(SignalReference);
    }
  }
  for (auto& Reference : References)
  {
    auto RoadId = Reference->GetRoadId();
    const auto* SignalReference = Reference;
    const FTransform SignalTransform = SignalReference->GetSignal()->GetTransform();
    for(auto &validity : SignalReference->GetValidities())
    {
      for(auto lane : carla::geom::Math::GenerateRange(validity._from_lane, validity._to_lane))
      {
        if(lane == 0)
          continue;

        auto signal_waypoint = Map->GetWaypoint(
            RoadId, lane, SignalReference->GetS()).get();

        if(Map->GetLane(signal_waypoint).GetType() != cr::Lane::LaneType::Driving)
          continue;

        FTransform ReferenceTransform = Map->ComputeTransform(signal_waypoint);

        DrawDebugSphere(
            World,
            ReferenceTransform.GetLocation(),
            50.0f,
            10,
            FColor(0, 0, 255),
            true
        );

        DrawDebugLine(
            World,
            ReferenceTransform.GetLocation(),
            SignalTransform.GetLocation(),
            FColor(0, 0, 255),
            true
        );
      }
    }
  }

}

FAtlasCopyRequest* ACarlaGameModeBase::GetAtlasCopyRequest()
{
  if(AtlasCopyRequestsQueuePool.IsEmpty()) {
    UE_LOG(LogCarla, Warning, TEXT("ACarlaGameModeBase::GetAtlasCopyRequest new FAtlasCopyRequest"));
    return new FAtlasCopyRequest();
  }
  FAtlasCopyRequest* AtlasCopyRequest = nullptr;
  AtlasCopyRequestsQueuePool.Dequeue(AtlasCopyRequest);
  return AtlasCopyRequest;
}

void ACarlaGameModeBase::CreateAtlasTextures()
{
  if(AtlasTextureWidth > 0 && AtlasTextureHeight > 0)
  {
    UE_LOG(LogCarla, Warning, TEXT("ACarlaGameModeBase::CreateAtlasTextures %d %dx%d"), SceneCaptureSensors.Num(), AtlasTextureWidth, AtlasTextureHeight);

    FRHIResourceCreateInfo CreateInfo;
    for(int i = 0; i < kMaxNumTextures; i++)
    {
      CamerasAtlasTexture[i] = RHICreateTexture2D(AtlasTextureWidth, AtlasTextureHeight, PF_B8G8R8A8, 1, 1, TexCreate_CPUReadback, CreateInfo);
      // Prepare some AtlasCopyRequests
      AtlasCopyRequestQueue.Enqueue(new FAtlasCopyRequest());
    }
    IsAtlasTextureValid = true;
  }
}

extern FDynamicRHI* GDynamicRHI;

void ACarlaGameModeBase::CaptureAtlas()
{

  ACarlaGameModeBase* This = this;

  if(!SceneCaptureSensors.Num()) return;

  if(!IsAtlasTextureValid)
  {
    CreateAtlasTextures();
  }

  //if (!AtlasCopyRequestQueue.IsEmpty())
  //{
  //  UE_LOG(LogCarla, Error, TEXT("ACarlaGameModeBase::CaptureAtlas: request full"));
  //  return;
  //}
  //UE_LOG(LogCarla, Warning, TEXT("ACarlaGameModeBase::CaptureAtlas: creating request"));

  FAtlasCopyRequest* AtlasCopyRequest = GetAtlasCopyRequest();
  AtlasCopyRequest->ResizeBuffer(AtlasTextureWidth, AtlasTextureHeight);

  ENQUEUE_RENDER_COMMAND(ACarlaGameModeBase_CaptureAtlas)
  (
    [This, &CurrentAtlasPixels = AtlasCopyRequest->AtlasImage](FRHICommandListImmediate& RHICmdList) mutable
    {
      FTexture2DRHIRef AtlasTexture = This->CamerasAtlasTexture[This->PreviousAtlas];
      //TArray<FColor>& CurrentAtlasPixels = This->AtlasPixels[This->CurrentAtlas];

      if (!AtlasTexture)
      {
        UE_LOG(LogCarla, Error, TEXT("ACarlaGameModeBase::CaptureAtlas: Missing atlas texture"));
        return;
      }

      // Download Atlas texture

      FIntRect Rect = FIntRect(0, 0, This->AtlasTextureWidth, This->AtlasTextureHeight);

#if !UE_BUILD_SHIPPING
      if(This->ReadSurfaceMode == 2) {
        Rect = FIntRect(0, 0, This->SurfaceW, This->SurfaceH);
      }
#endif

      // GDynamicRHI->RHIReadSurfaceData(AtlasTexture, Rect, CurrentAtlasPixels, FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));

#if !UE_BUILD_SHIPPING
      if (This->ReadSurfaceMode == 0) return;
#endif

      SCOPE_CYCLE_COUNTER(STAT_CarlaSensorReadRT);
      RHICmdList.ReadSurfaceData(
        AtlasTexture,
        Rect,
        CurrentAtlasPixels,
        FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));


        This->PreviousAtlas = This->CurrentAtlas;
        This->CurrentAtlas = (This->CurrentAtlas + 1) & ~kMaxNumTextures;
    }
  );

  // AtlasCopyRequest->Start();
  // AtlasCopyRequest->Wait();
  AtlasCopyRequestQueue.Enqueue(AtlasCopyRequest);

}

void ACarlaGameModeBase::SendAtlas()
{
  if(!AtlasCopyRequestQueue.IsEmpty())
  {
    FAtlasCopyRequest* AtlasCopyRequest = nullptr;
    AtlasCopyRequestQueue.Peek(AtlasCopyRequest);

    // Be sure that the request has finished
    
    if(AtlasCopyRequest /* && AtlasCopyRequest->IsComplete() */ )
    {
      //UE_LOG(LogCarla, Warning, TEXT("ACarlaGameModeBase::SendAtlas: DONE"));
#if !UE_BUILD_SHIPPING
      if(!AtlasCopyToCamera)
      {
        AtlasCopyRequestQueue.Pop();
        AtlasCopyRequestsQueuePool.Enqueue(AtlasCopyRequest);
        return;
      }
#endif

      ACarlaGameModeBase* This = this;
      // We need to make a copy of the atlas to avoid possible threads issues later
      // since the AtlasCopyRequest could be used later
      //TArray<FColor> AtlasImage(AtlasCopyRequest->AtlasImage);
      //ParallelFor(SceneCaptureSensors.Num(),
      //  [This, &AtlasImage](int32 Index)
      for(int32 Index = 0; Index < SceneCaptureSensors.Num(); Index++)
      {
        ASceneCaptureSensor* Sensor = SceneCaptureSensors[Index];
        Sensor->CopyTextureFromAtlas(AtlasCopyRequest->AtlasImage);
      }
      //);

      // Remove from pending queue ...
      AtlasCopyRequestQueue.Pop();
      // ... add to pool
      AtlasCopyRequestsQueuePool.Enqueue(AtlasCopyRequest);
    }
    else
    {
      //UE_LOG(LogCarla, Error, TEXT("ACarlaGameModeBase::SendAtlas: request didn't finish"));
    }

  }

}