// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Game/CarlaGameModeBase.h"
#include "Carla.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Game/CarlaHUD.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Game/CarlaStaticDelegates.h"
#include "Carla/Lights/CarlaLight.h"
#include "Carla/Vehicle/VehicleSpawnPoint.h"
#include "Carla/Util/BoundingBoxCalculator.h"

#include <util/disable-ue4-macros.h>
#include "carla/opendrive/OpenDriveParser.h"
#include "carla/road/element/RoadInfoSignal.h"
#include <carla/rpc/EnvironmentObject.h>
#include <carla/rpc/WeatherParameters.h>
#include <carla/rpc/MapLayer.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Engine/DecalActor.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LocalPlayer.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Async/ParallelFor.h"
#include "DynamicRHI.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h"
#include <util/ue-header-guard-end.h>

namespace cr = carla::road;
namespace crp = carla::rpc;
namespace cre = carla::road::element;

ACarlaGameModeBase::ACarlaGameModeBase(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;
  bAllowTickBeforeBeginPlay = false;

  Episode = CreateDefaultSubobject<UCarlaEpisode>(TEXT("Episode"));

  Recorder = CreateDefaultSubobject<ACarlaRecorder>(TEXT("Recorder"));

  ObjectRegister = CreateDefaultSubobject<UObjectRegister>(TEXT("ObjectRegister"));

  // HUD
  HUDClass = ACarlaHUD::StaticClass();

  TaggerDelegate = CreateDefaultSubobject<UTaggerDelegate>(TEXT("TaggerDelegate"));
  CarlaSettingsDelegate = CreateDefaultSubobject<UCarlaSettingsDelegate>(TEXT("CarlaSettingsDelegate"));
}

const FString ACarlaGameModeBase::GetRelativeMapPath() const
{
  UWorld* World = GetWorld();
  TSoftObjectPtr<UWorld> AssetPtr (World);
  FString Path = FPaths::GetPath(AssetPtr.GetLongPackageName());
  Path.RemoveFromStart("/Game/");
  return Path;
}

const FString ACarlaGameModeBase::GetFullMapPath() const
{
  FString Path = GetRelativeMapPath();
  return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + Path;
}

void ACarlaGameModeBase::InitGame(
    const FString &MapName,
    const FString &Options,
    FString &ErrorMessage)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ACarlaGameModeBase::InitGame);
  Super::InitGame(MapName, Options, ErrorMessage);

  UWorld* World = GetWorld();
  check(World != nullptr);
  FString InMapName(MapName);

  checkf(
      Episode != nullptr,
      TEXT("Missing episode, can't continue without an episode!"));

  AActor* LMManagerActor =
      UGameplayStatics::GetActorOfClass(GetWorld(), ALargeMapManager::StaticClass());
  LMManager = Cast<ALargeMapManager>(LMManagerActor);
  if (LMManager) {
    if (LMManager->GetNumTiles() == 0)
    {
      LMManager->GenerateLargeMap();
    }
    InMapName = LMManager->LargeMapName;
  }

#if WITH_EDITOR
    {
      // When playing in editor the map name gets an extra prefix, here we
      // remove it.
      FString CorrectedMapName = InMapName;
      constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
      CorrectedMapName.RemoveFromStart(PIEPrefix);
      UE_LOG(LogCarla, Log, TEXT("Corrected map name from %s to %s"), *InMapName, *CorrectedMapName);
      Episode->MapName = CorrectedMapName;
    }
#else
  Episode->MapName = InMapName;
#endif // WITH_EDITOR

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

  AActor* WeatherActor =
      UGameplayStatics::GetActorOfClass(GetWorld(), AWeather::StaticClass());
  if (WeatherActor != nullptr) {
    UE_LOG(LogCarla, Log, TEXT("Existing weather actor. Doing nothing then!"));
    Episode->Weather = static_cast<AWeather*>(WeatherActor);
  }
  else if (WeatherClass != nullptr) {
    Episode->Weather = World->SpawnActor<AWeather>(WeatherClass);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Missing weather class!"));
  }

  GameInstance->NotifyInitGame();

  OnEpisodeSettingsChangeHandle = FCarlaStaticDelegates::OnEpisodeSettingsChange.AddUObject(
        this,
        &ACarlaGameModeBase::OnEpisodeSettingsChanged);

  SpawnActorFactories();

  // make connection between Episode and Recorder
  Recorder->SetEpisode(Episode);
  Episode->SetRecorder(Recorder);

  ParseOpenDrive();

  if(Map.has_value())
  {
    StoreSpawnPoints();
  }
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

  UWorld* World = GetWorld();
  check(World != nullptr);

  LoadMapLayer(GameInstance->GetCurrentMapLayer());
  ReadyToRegisterObjects = true;

  ATagger::TagActorsInLevel(*World, true);
  TaggerDelegate->SetSemanticSegmentationEnabled();

  // HACK: fix transparency see-through issues
  // The problem: transparent objects are visible through walls.
  // This is due to a weird interaction between the SkyAtmosphere component,
  // the shadows of a directional light (the sun)
  // and the custom depth set to 3 used for semantic segmentation
  // The solution: Spawn a Decal.
  // It just works!
  World->SpawnActor<ADecalActor>(
      FVector(0,0,-1000000), FRotator(0,0,0), FActorSpawnParameters());

  ATrafficLightManager* Manager = GetTrafficLightManager();
  Manager->InitializeTrafficLights();

  Episode->InitializeAtBeginPlay();
  GameInstance->NotifyBeginEpisode(*Episode);
  OnEpisodeInitialisedDelegate.Broadcast(Episode);

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

  if(ReadyToRegisterObjects && PendingLevelsToLoad == 0)
  {
    RegisterEnvironmentObjects();
  }

  if (LMManager) {
    LMManager->RegisterInitialObjects();
    LMManager->ConsiderSpectatorAsEgo(Episode->GetSettings().SpectatorAsEgo);
  }

  // Manually run begin play on lights as it may not run on sublevels
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
  for(AActor* Actor : FoundActors)
  {
    TArray<UCarlaLight*> Lights;
    Actor->GetComponents(Lights, false);
    for(UCarlaLight* Light : Lights)
    {
      Light->RegisterLight();
    }
  }
  EnableOverlapEvents();
}

TArray<FString> ACarlaGameModeBase::GetNamesOfAllActors()
{
  TArray<FString> Names;
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
  for (AActor* Actor : Actors)
  {
    TArray<UStaticMeshComponent*> StaticMeshes;
    Actor->GetComponents(StaticMeshes);
    if (StaticMeshes.Num())
    {
      Names.Add(Actor->GetName());
    }
  }
  return Names;
}

AActor* ACarlaGameModeBase::FindActorByName(const FString& ActorName)
{
  TArray<AActor*> Actors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
  for (AActor* Actor : Actors)
  {
    if(Actor->GetName() == ActorName)
    {
      return Actor;
      break;
    }
  }
  return nullptr;
}

UTexture2D* ACarlaGameModeBase::CreateUETexture(const carla::rpc::TextureColor& Texture)
{
  FlushRenderingCommands();
  TArray<FColor> Colors;
  for (uint32_t y = 0; y < Texture.GetHeight(); y++)
  {
    for (uint32_t x = 0; x < Texture.GetWidth(); x++)
    {
      auto& Color = Texture.At(x,y);
      Colors.Add(FColor(Color.r, Color.g, Color.b, Color.a));
    }
  }
  UTexture2D* UETexture = UTexture2D::CreateTransient(Texture.GetWidth(), Texture.GetHeight(), EPixelFormat::PF_B8G8R8A8);
  FTexture2DMipMap& Mip = UETexture->GetPlatformData()->Mips[0];
  void* Data = Mip.BulkData.Lock( LOCK_READ_WRITE );
  FMemory::Memcpy( Data,
      &Colors[0],
      Texture.GetWidth()*Texture.GetHeight()*sizeof(FColor));
  Mip.BulkData.Unlock();
  UETexture->UpdateResource();
  return UETexture;
}

UTexture2D* ACarlaGameModeBase::CreateUETexture(const carla::rpc::TextureFloatColor& Texture)
{
  FlushRenderingCommands();
  TArray<FFloat16Color> Colors;
  for (uint32_t y = 0; y < Texture.GetHeight(); y++)
  {
    for (uint32_t x = 0; x < Texture.GetWidth(); x++)
    {
      auto& Color = Texture.At(x,y);
      Colors.Add(FLinearColor(Color.r, Color.g, Color.b, Color.a));
    }
  }
  UTexture2D* UETexture = UTexture2D::CreateTransient(Texture.GetWidth(), Texture.GetHeight(), EPixelFormat::PF_FloatRGBA);
  FTexture2DMipMap& Mip = UETexture->GetPlatformData()->Mips[0];
  void* Data = Mip.BulkData.Lock( LOCK_READ_WRITE );
  FMemory::Memcpy( Data,
      &Colors[0],
      Texture.GetWidth()*Texture.GetHeight()*sizeof(FFloat16Color));
  Mip.BulkData.Unlock();
  UETexture->UpdateResource();
  return UETexture;
}

void ACarlaGameModeBase::ApplyTextureToActor(
    AActor* Actor,
    UTexture2D* Texture,
    const carla::rpc::MaterialParameter& TextureParam)
{
  namespace cr = carla::rpc;
  TArray<UStaticMeshComponent*> StaticMeshes;
  Actor->GetComponents(StaticMeshes);
  for (UStaticMeshComponent* Mesh : StaticMeshes)
  {
    for (int i = 0; i < Mesh->GetNumMaterials(); ++i)
    {
      UMaterialInterface* OriginalMaterial = Mesh->GetMaterial(i);
      UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(OriginalMaterial);
      if(!DynamicMaterial)
      {
        DynamicMaterial = UMaterialInstanceDynamic::Create(OriginalMaterial, NULL);
        Mesh->SetMaterial(i, DynamicMaterial);
      }

      switch(TextureParam)
      {
        case cr::MaterialParameter::Tex_Diffuse:
          DynamicMaterial->SetTextureParameterValue("BaseColor", Texture);
          DynamicMaterial->SetTextureParameterValue("Difuse", Texture);
          DynamicMaterial->SetTextureParameterValue("Difuse 2", Texture);
          DynamicMaterial->SetTextureParameterValue("Difuse 3", Texture);
          DynamicMaterial->SetTextureParameterValue("Difuse 4", Texture);
          break;
        case cr::MaterialParameter::Tex_Normal:
          DynamicMaterial->SetTextureParameterValue("Normal", Texture);
          DynamicMaterial->SetTextureParameterValue("Normal 2", Texture);
          DynamicMaterial->SetTextureParameterValue("Normal 3", Texture);
          DynamicMaterial->SetTextureParameterValue("Normal 4", Texture);
          break;
        case cr::MaterialParameter::Tex_Emissive:
          DynamicMaterial->SetTextureParameterValue("Emissive", Texture);
          break;
        case cr::MaterialParameter::Tex_Ao_Roughness_Metallic_Emissive:
          DynamicMaterial->SetTextureParameterValue("AO / Roughness / Metallic / Emissive", Texture);
          DynamicMaterial->SetTextureParameterValue("ORMH", Texture);
          DynamicMaterial->SetTextureParameterValue("ORMH 2", Texture);
          DynamicMaterial->SetTextureParameterValue("ORMH 3", Texture);
          DynamicMaterial->SetTextureParameterValue("ORMH 4", Texture);
          break;
      }
    }
  }
}

void ACarlaGameModeBase::Tick(float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ACarlaGameModeBase::Tick);
  Super::Tick(DeltaSeconds);

  /// @todo Recorder should not tick here, FCarlaEngine should do it.
  if (Recorder)
  {
    Recorder->Tick(DeltaSeconds);
  }
}

void ACarlaGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  FCarlaStaticDelegates::OnEpisodeSettingsChange.Remove(OnEpisodeSettingsChangeHandle);

  Episode->EndPlay();
  GameInstance->NotifyEndEpisode();

  Super::EndPlay(EndPlayReason);

  if ((CarlaSettingsDelegate != nullptr) && (EndPlayReason != EEndPlayReason::EndPlayInEditor))
  {
    CarlaSettingsDelegate->Reset();
  }
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

void ACarlaGameModeBase::StoreSpawnPoints()
{
  for (TActorIterator<AVehicleSpawnPoint> It(GetWorld()); It; ++It)
  {
    SpawnPointsTransforms.Add(It->GetActorTransform());
  }

  if(SpawnPointsTransforms.Num() == 0)
  {
    GenerateSpawnPoints();
  }

  UE_LOG(LogCarla, Log, TEXT("There are %d SpawnPoints in the map"), SpawnPointsTransforms.Num());
}

void ACarlaGameModeBase::GenerateSpawnPoints()
{
  UE_LOG(LogCarla, Log, TEXT("Generating SpawnPoints ..."));
  std::vector<std::pair<carla::road::element::Waypoint, carla::road::element::Waypoint>> Topology = Map->GenerateTopology();
  UWorld* World = GetWorld();
  for(auto& Pair : Topology)
  {
    carla::geom::Transform CarlaTransform = Map->ComputeTransform(Pair.first);
    FTransform Transform(CarlaTransform);
    Transform.AddToTranslation(FVector(0.f, 0.f, 50.0f));
    SpawnPointsTransforms.Add(Transform);
  }
}

void ACarlaGameModeBase::ParseOpenDrive()
{
  std::string opendrive_xml = carla::rpc::FromLongFString(UOpenDrive::GetXODR(GetWorld()));
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
    UWorld* World = GetWorld();
    AActor* TrafficLightManagerActor = UGameplayStatics::GetActorOfClass(World, ATrafficLightManager::StaticClass());
    if(TrafficLightManagerActor == nullptr)
    {
      FActorSpawnParameters SpawnParams;
      SpawnParams.OverrideLevel = GetULevelFromName("TrafficLights");
      TrafficLightManager = World->SpawnActor<ATrafficLightManager>(SpawnParams);
    }
    else
    {
      TrafficLightManager = Cast<ATrafficLightManager>(TrafficLightManagerActor);
    }
  }
  return TrafficLightManager;
}

void ACarlaGameModeBase::CheckForEmptyMeshes()
{
  TArray<AActor*> WorldActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), WorldActors);

  for (AActor *Actor : WorldActors)
  {
    AStaticMeshActor *MeshActor = CastChecked<AStaticMeshActor>(Actor);
    if (MeshActor->GetStaticMeshComponent()->GetStaticMesh() == NULL)
    {
      UE_LOG(LogTemp, Error, TEXT("The object : %s has no mesh"), *MeshActor->GetFullName());
    }
  }
}

void ACarlaGameModeBase::EnableOverlapEvents()
{
  TArray<AActor*> WorldActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), WorldActors);

  for(AActor *Actor : WorldActors)
  {
    AStaticMeshActor *MeshActor = CastChecked<AStaticMeshActor>(Actor);
    if(MeshActor->GetStaticMeshComponent()->GetStaticMesh() != NULL)
    {
      auto MeshTag = ATagger::GetTagOfTaggedComponent(*MeshActor->GetStaticMeshComponent());
      namespace crp = carla::rpc;
      if (MeshTag != crp::CityObjectLabel::Roads && 
          MeshTag != crp::CityObjectLabel::Sidewalks && 
          MeshTag != crp::CityObjectLabel::RoadLines && 
          MeshTag != crp::CityObjectLabel::Ground &&
          MeshTag != crp::CityObjectLabel::Terrain &&
          MeshActor->GetStaticMeshComponent()->GetGenerateOverlapEvents() == false)
      {
        MeshActor->GetStaticMeshComponent()->SetGenerateOverlapEvents(true);
      }
    }
  }
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
            RoadId, lane, SignalReference->GetS()).value();

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

TArray<FBoundingBox> ACarlaGameModeBase::GetAllBBsOfLevel(uint8 TagQueried) const
{
  UWorld* World = GetWorld();

  // Get all actors of the level
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);

  TArray<FBoundingBox> BoundingBoxes;
  BoundingBoxes = UBoundingBoxCalculator::GetBoundingBoxOfActors(FoundActors, TagQueried);

  return BoundingBoxes;
}

void ACarlaGameModeBase::RegisterEnvironmentObjects()
{
  // Get all actors of the level
  TArray<AActor*> FoundActors;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
  ObjectRegister->RegisterObjects(FoundActors);
}

void ACarlaGameModeBase::EnableEnvironmentObjects(
  const TSet<uint64>& EnvObjectIds,
  bool Enable)
{
  ObjectRegister->EnableEnvironmentObjects(EnvObjectIds, Enable);
}

void ACarlaGameModeBase::LoadMapLayer(int32 MapLayers)
{
  const UWorld* World = GetWorld();
  UGameplayStatics::FlushLevelStreaming(World);

  TArray<FName> LevelsToLoad;
  ConvertMapLayerMaskToMapNames(MapLayers, LevelsToLoad);

  FLatentActionInfo LatentInfo;
  LatentInfo.CallbackTarget = this;
  LatentInfo.ExecutionFunction = "OnLoadStreamLevel";
  LatentInfo.Linkage = 0;
  LatentInfo.UUID = LatentInfoUUID;

  PendingLevelsToLoad = LevelsToLoad.Num();

  for(FName& LevelName : LevelsToLoad)
  {
    LatentInfoUUID++;
    UGameplayStatics::LoadStreamLevel(World, LevelName, true, true, LatentInfo);
    LatentInfo.UUID = LatentInfoUUID;
    UGameplayStatics::FlushLevelStreaming(World);
  }
}

void ACarlaGameModeBase::UnLoadMapLayer(int32 MapLayers)
{
  const UWorld* World = GetWorld();

  TArray<FName> LevelsToUnLoad;
  ConvertMapLayerMaskToMapNames(MapLayers, LevelsToUnLoad);

  FLatentActionInfo LatentInfo;
  LatentInfo.CallbackTarget = this;
  LatentInfo.ExecutionFunction = "OnUnloadStreamLevel";
  LatentInfo.UUID = LatentInfoUUID;
  LatentInfo.Linkage = 0;

  PendingLevelsToUnLoad = LevelsToUnLoad.Num();

  for(FName& LevelName : LevelsToUnLoad)
  {
    LatentInfoUUID++;
    UGameplayStatics::UnloadStreamLevel(World, LevelName, LatentInfo, false);
    LatentInfo.UUID = LatentInfoUUID;
    UGameplayStatics::FlushLevelStreaming(World);
  }

}

void ACarlaGameModeBase::ConvertMapLayerMaskToMapNames(int32 MapLayer, TArray<FName>& OutLevelNames)
{
  UWorld* World = GetWorld();
  const TArray <ULevelStreaming*> Levels = World->GetStreamingLevels();
  TArray<FString> LayersToLoad;

  // Get all the requested layers
  int32 LayerMask = 1;
  int32 AllLayersMask = static_cast<crp::MapLayerType>(crp::MapLayer::All);

  while(LayerMask > 0)
  {
    // Convert enum to FString
    FString LayerName = UTF8_TO_TCHAR(MapLayerToString(static_cast<crp::MapLayer>(LayerMask)).c_str());
    bool included = static_cast<crp::MapLayerType>(MapLayer) & LayerMask;
    if(included)
    {
      LayersToLoad.Emplace(LayerName);
    }
    LayerMask = (LayerMask << 1) & AllLayersMask;
  }

  // Get all the requested level maps
  for(ULevelStreaming* Level : Levels)
  {
    TArray<FString> StringArray;
    FString FullSubMapName = Level->GetWorldAssetPackageFName().ToString();
    // Discard full path, we just need the umap name
    FullSubMapName.ParseIntoArray(StringArray, TEXT("/"), false);
    FString SubMapName = StringArray[StringArray.Num() - 1];
    for(FString LayerName : LayersToLoad)
    {
      if(SubMapName.Contains(LayerName))
      {
        OutLevelNames.Emplace(FName(*SubMapName));
        break;
      }
    }
  }

}

ULevel* ACarlaGameModeBase::GetULevelFromName(FString LevelName)
{
  ULevel* OutLevel = nullptr;
  UWorld* World = GetWorld();
  const TArray <ULevelStreaming*> Levels = World->GetStreamingLevels();

  for(ULevelStreaming* Level : Levels)
  {
    FString FullSubMapName = Level->GetWorldAssetPackageFName().ToString();
    if(FullSubMapName.Contains(LevelName))
    {
      OutLevel = Level->GetLoadedLevel();
      if(!OutLevel)
      {
        UE_LOG(LogCarla, Warning, TEXT("%s has not been loaded"), *LevelName);
      }
      break;
    }
  }

  return OutLevel;
}

void ACarlaGameModeBase::OnLoadStreamLevel()
{
  PendingLevelsToLoad--;

  // Register new actors and tag them
  if(ReadyToRegisterObjects && PendingLevelsToLoad == 0)
  {
    RegisterEnvironmentObjects();
    ATagger::TagActorsInLevel(*GetWorld(), true);
  }
}

void ACarlaGameModeBase::OnUnloadStreamLevel()
{
  PendingLevelsToUnLoad--;
  // Update stored registered objects (discarding the deleted objects)
  if(ReadyToRegisterObjects && PendingLevelsToUnLoad == 0)
  {
    RegisterEnvironmentObjects();
  }
}

void ACarlaGameModeBase::OnEpisodeSettingsChanged(const FEpisodeSettings &Settings)
{
  CarlaSettingsDelegate->SetAllActorsDrawDistance(GetWorld(), Settings.MaxCullingDistance);
}
