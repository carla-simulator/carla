// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Game/CarlaEpisode.h"
#include "Carla/OpenDrive/OpenDrive.h"
#include "Carla.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Sensor/Sensor.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "Carla/Util/RandomEngine.h"

#include "Carla/Vehicle/VehicleSpawnPoint.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Game/CarlaStaticDelegates.h"
#include "Carla/MapGen/LargeMapManager.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Walker/WalkerBase.h"

#include <util/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <util/enable-ue4-macros.h>

#include <util/ue-header-guard-begin.h>
#include "Async/Async.h"
#include "HAL/PlatformProcess.h"
#include "Components/WorldPartitionStreamingSourceComponent.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/SpectatorPawn.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include <util/ue-header-guard-end.h>

constexpr TCHAR DefaultRecastBuilderPath[] = TEXT(RECASTBUILDER_PATH);

static FString BuildRecastBuilderFile()
{
    auto Path = FPaths::RootDir();
#if PLATFORM_WINDOWS
    Path += TEXT("Tools/RecastBuilder.exe");
#else
    Path += TEXT("Tools/RecastBuilder");
#endif
    Path = FPaths::ConvertRelativePathToFull(Path);

    if (FPaths::FileExists(Path))
        return Path;
    else
        return DefaultRecastBuilderPath;
}

// State of the background RecastBuilder run of the last generated OpenDRIVE
// world (there is at most one). Readers of the navmesh wait on it.
static FCriticalSection GNavBuildLock;
static bool GNavBuildPending = false;
static bool GNavBuildSucceeded = false;

bool UCarlaEpisode::WaitForPendingNavigationBuild(double TimeoutSeconds)
{
  const double Start = FPlatformTime::Seconds();
  bool bLogged = false;
  for (;;)
  {
    {
      FScopeLock Lock(&GNavBuildLock);
      if (!GNavBuildPending)
      {
        return GNavBuildSucceeded;
      }
    }
    if (!bLogged)
    {
      UE_LOG(LogCarla, Log, TEXT("Waiting for RecastBuilder to finish the pedestrian navigation (up to %.0f s)"),
          TimeoutSeconds);
      bLogged = true;
    }
    if (FPlatformTime::Seconds() - Start > TimeoutSeconds)
    {
      UE_LOG(LogCarla, Error, TEXT("RecastBuilder did not finish within %.0f s; "
          "the pedestrian navigation of this OpenDRIVE world is not available yet"), TimeoutSeconds);
      return false;
    }
    FPlatformProcess::Sleep(0.05f);
  }
}

void UCarlaEpisode::ClearGeneratedWorldFiles()
{
  // Saved/ copies of the generated world outlive the process; a stale pair
  // (or a .bin from another .xodr) must never shadow the cooked stub.
  const FString SavedDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir());
  for (const FString &File : {
      SavedDir / TEXT("OpenDrive/OpenDriveMap.xodr"),
      SavedDir / TEXT("Nav/OpenDriveMap.obj"),
      SavedDir / TEXT("Nav/OpenDriveMap.bin")})
  {
    if (IFileManager::Get().FileExists(*File))
    {
      IFileManager::Get().Delete(*File, false, true, true);
      UE_LOG(LogCarla, Log, TEXT("Removed stale generated-world file '%s'"), *File);
    }
  }
}

static FString UCarlaEpisode_GetTrafficSignId(ETrafficSignState State)
{
  using TSS = ETrafficSignState;
  switch (State)
  {
    case TSS::TrafficLightRed:
    case TSS::TrafficLightYellow:
    case TSS::TrafficLightGreen:  return TEXT("traffic.traffic_light");
    case TSS::SpeedLimit_30:      return TEXT("traffic.speed_limit.30");
    case TSS::SpeedLimit_40:      return TEXT("traffic.speed_limit.40");
    case TSS::SpeedLimit_50:      return TEXT("traffic.speed_limit.50");
    case TSS::SpeedLimit_60:      return TEXT("traffic.speed_limit.60");
    case TSS::SpeedLimit_90:      return TEXT("traffic.speed_limit.90");
    case TSS::SpeedLimit_100:     return TEXT("traffic.speed_limit.100");
    case TSS::SpeedLimit_120:     return TEXT("traffic.speed_limit.120");
    case TSS::SpeedLimit_130:     return TEXT("traffic.speed_limit.130");
    case TSS::StopSign:           return TEXT("traffic.stop");
    case TSS::YieldSign:          return TEXT("traffic.yield");
    default:                      return TEXT("traffic.unknown");
  }
}

UCarlaEpisode::UCarlaEpisode(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer),
    Id(URandomEngine::GenerateRandomId())
{
  ActorDispatcher = CreateDefaultSubobject<UActorDispatcher>(TEXT("ActorDispatcher"));
  FrameData.SetEpisode(this);

  const FString AbsoluteRecastBuilderPath = BuildRecastBuilderFile();

  if (!FPaths::FileExists(AbsoluteRecastBuilderPath))
  {
      UE_LOG(LogCarla, Warning, TEXT("'RecastBuilder' not present under '%s', "
          "the binaries for pedestrian navigation will not be created."),
          *AbsoluteRecastBuilderPath);
  }
}

bool UCarlaEpisode::LoadNewEpisode(const FString &MapString, bool ResetSettings)
{
  bool bIsFileFound = false;

  FString FinalPath = UCarlaStatics::FindMapPath(MapString);

  if(FPaths::FileExists(FinalPath) || FPackageName::DoesPackageExist(FinalPath))
  {
    bIsFileFound = true;
    if (FPaths::FileExists(FinalPath))
    {
      FinalPath = MapString;
    }
  }

  if (bIsFileFound)
  {
    UE_LOG(LogCarla, Warning, TEXT("Loading a new episode: %s"), *FinalPath);
    UGameplayStatics::OpenLevel(GetWorld(), *FinalPath, true);
    if (ResetSettings)
      ApplySettings(FEpisodeSettings{});
    
    // send 'LOAD_MAP' command to all secondary servers (if any)
    if (bIsPrimaryServer)
    {
      UCarlaGameInstance *GameInstance = UCarlaStatics::GetGameInstance(GetWorld());
      if(GameInstance)
      {
        FCarlaEngine *CarlaEngine = GameInstance->GetCarlaEngine();
        auto SecondaryServer = CarlaEngine->GetSecondaryServer();
        if (SecondaryServer->HasClientsConnected()) 
        {
          SecondaryServer->GetCommander().SendLoadMap(std::string(TCHAR_TO_UTF8(*FinalPath)));
        }
      }
    }
  }
  return bIsFileFound;
}



bool UCarlaEpisode::LoadNewOpendriveEpisode(
    const FString &OpenDriveString,
    const carla::rpc::OpendriveGenerationParameters &Params)
{
  if (OpenDriveString.IsEmpty())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive string is empty."));
    return false;
  }

  // Build the Map from the OpenDRIVE data
  const auto CarlaMap = carla::opendrive::OpenDriveParser::Load(
      carla::rpc::FromLongFString(OpenDriveString));

  // Check the Map is correclty generated
  if (!CarlaMap.has_value())
  {
    UE_LOG(LogCarla, Error, TEXT("The OpenDrive string is invalid or not supported"));
    return false;
  }

  // Generate the OBJ (as string)
  const auto RoadMesh = CarlaMap->GenerateMesh(Params.vertex_distance);
  const auto CrosswalksMesh = CarlaMap->GetAllCrosswalkMesh();
  const auto RecastOBJ = (RoadMesh + CrosswalksMesh).GenerateOBJForRecast();

  // Both files go to Saved/: in a packaged build the content dir is backed
  // by the pak and not writable. The readers (UOpenDrive::FindPathToXODRFile,
  // FNavigationMesh::Load) look in Saved/ first. RecastBuilder writes the
  // .bin next to the .obj. Start from a clean slate so a .bin can never
  // belong to a different .xodr.
  WaitForPendingNavigationBuild(60.0);
  ClearGeneratedWorldFiles();
  const FString AbsoluteOBJPath = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectSavedDir() / TEXT("Nav/OpenDriveMap.obj"));

  // Store the OBJ string to a file in order to that RecastBuilder can load it
  FFileHelper::SaveStringToFile(
      carla::rpc::ToLongFString(RecastOBJ),
      *AbsoluteOBJPath,
      FFileHelper::EEncodingOptions::ForceUTF8,
      &IFileManager::Get());

  const FString AbsoluteXODRPath = UOpenDrive::GetSavedXODRPath(TEXT("OpenDriveMap"));

  // Copy the OpenDrive as a file in the serverside
  FFileHelper::SaveStringToFile(
      OpenDriveString,
      *AbsoluteXODRPath,
      FFileHelper::EEncodingOptions::ForceUTF8,
      &IFileManager::Get());

  if (!FPaths::FileExists(AbsoluteXODRPath))
  {
    UE_LOG(LogCarla, Error, TEXT("ERROR: XODR not copied!"));
    return false;
  }

  UCarlaGameInstance * GameInstance = UCarlaStatics::GetGameInstance(GetWorld());
  if(GameInstance)
  {
    GameInstance->SetOpendriveGenerationParameters(Params);
  }
  else
  {
    carla::log_warning("Missing game instance");
  }

  const FString AbsoluteRecastBuilderPath = BuildRecastBuilderFile();
  if (FPaths::FileExists(AbsoluteRecastBuilderPath) &&
      Params.enable_pedestrian_navigation)
  {
    // RecastBuilder writes <obj stem>.bin next to the .obj and always exits
    // 0, so the .bin is the only success signal: drop the previous one first
    // so a failed build can never hand out last session's navmesh, then watch
    // the process off the game thread and log the outcome loudly.
    const FString AbsoluteBinPath = FPaths::ChangeExtension(AbsoluteOBJPath, TEXT("bin"));
    IFileManager::Get().Delete(*AbsoluteBinPath, false, true, true);

    void *PipeRead = nullptr;
    void *PipeWrite = nullptr;
    FPlatformProcess::CreatePipe(PipeRead, PipeWrite);

    /// @todo this can take too long to finish, clients need a method
    /// to know if the navigation is available or not.
    FProcHandle RecastHandle = FPlatformProcess::CreateProc(
        *AbsoluteRecastBuilderPath, *AbsoluteOBJPath,
        true, true, true, nullptr, 0, nullptr, PipeWrite, nullptr, PipeWrite);
    if (!RecastHandle.IsValid())
    {
      FPlatformProcess::ClosePipe(PipeRead, PipeWrite);
      UE_LOG(LogCarla, Error, TEXT("RecastBuilder could not be launched ('%s' '%s'); "
          "no pedestrian navigation for this OpenDRIVE world."),
          *AbsoluteRecastBuilderPath, *AbsoluteOBJPath);
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("RecastBuilder started: '%s' '%s' -> '%s'"),
          *AbsoluteRecastBuilderPath, *AbsoluteOBJPath, *AbsoluteBinPath);
      {
        FScopeLock Lock(&GNavBuildLock);
        GNavBuildPending = true;
        GNavBuildSucceeded = false;
      }
      Async(EAsyncExecution::Thread,
          [RecastHandle, PipeRead, PipeWrite, AbsoluteBinPath, AbsoluteRecastBuilderPath]() mutable
      {
        FString Output;
        while (FPlatformProcess::IsProcRunning(RecastHandle))
        {
          Output += FPlatformProcess::ReadPipe(PipeRead);
          FPlatformProcess::Sleep(0.1f);
        }
        Output += FPlatformProcess::ReadPipe(PipeRead);
        int32 ReturnCode = -1;
        FPlatformProcess::GetProcReturnCode(RecastHandle, &ReturnCode);
        FPlatformProcess::CloseProc(RecastHandle);
        FPlatformProcess::ClosePipe(PipeRead, PipeWrite);
        Output.TrimEndInline();
        const bool bSucceeded = ReturnCode == 0 && FPaths::FileExists(AbsoluteBinPath);
        if (!bSucceeded)
        {
          UE_LOG(LogCarla, Error, TEXT("RecastBuilder failed (exit code %d, '%s' %s): no pedestrian "
              "navigation for this OpenDRIVE world. Output:\n%s"),
              ReturnCode, *AbsoluteBinPath,
              FPaths::FileExists(AbsoluteBinPath) ? TEXT("written") : TEXT("missing"), *Output);
        }
        else
        {
          UE_LOG(LogCarla, Log, TEXT("RecastBuilder finished: navmesh '%s' ready. Output:\n%s"),
              *AbsoluteBinPath, *Output);
        }
        FScopeLock Lock(&GNavBuildLock);
        GNavBuildPending = false;
        GNavBuildSucceeded = bSucceeded;
      });
    }
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("'RecastBuilder' not present under '%s', "
        "the binaries for pedestrian navigation will not be created."),
        *AbsoluteRecastBuilderPath);
  }

  return true;
}

void UCarlaEpisode::ApplySettings(const FEpisodeSettings &Settings)
{
  EpisodeSettings = Settings;
  if(EpisodeSettings.ActorActiveDistance > EpisodeSettings.TileStreamingDistance)
  {
    UE_LOG(LogCarla, Warning, TEXT("Setting ActorActiveDistance is smaller that TileStreamingDistance, TileStreamingDistance will be increased"));
    EpisodeSettings.TileStreamingDistance = EpisodeSettings.ActorActiveDistance;
  }
  FCarlaStaticDelegates::OnEpisodeSettingsChange.Broadcast(EpisodeSettings);
}

TArray<FTransform> UCarlaEpisode::GetRecommendedSpawnPoints() const
{
  ACarlaGameModeBase *GM = UCarlaStatics::GetGameMode(GetWorld());

  return GM->GetSpawnPointsTransforms();
}

carla::rpc::Actor UCarlaEpisode::SerializeActor(FCarlaActor *CarlaActor) const
{
  carla::rpc::Actor Actor;
  if (CarlaActor)
  {
    Actor = CarlaActor->GetActorInfo()->SerializedData;
    auto ParentId = CarlaActor->GetParent();
    if (ParentId)
    {
      Actor.parent_id = ParentId;
    }
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("Trying to serialize invalid actor"));
  }
  return Actor;
}

carla::rpc::Actor UCarlaEpisode::SerializeActor(AActor* Actor) const
{
  FCarlaActor* CarlaActor = FindCarlaActor(Actor);
  if (CarlaActor)
  {
    return SerializeActor(CarlaActor);
  }
  else
  {
    carla::rpc::Actor SerializedActor;
    SerializedActor.id = 0u;
    SerializedActor.bounding_box = UBoundingBoxCalculator::GetActorBoundingBox(Actor);
    TSet<crp::CityObjectLabel> SemanticTags;
    ATagger::GetTagsOfTaggedActor(*Actor, SemanticTags);
    FActorDescription Description;
    Description.Id = TEXT("static.") + CarlaGetRelevantTagAsString(SemanticTags);
    SerializedActor.description = Description;
    SerializedActor.semantic_tags.reserve(SemanticTags.Num());
    for (auto &&Tag : SemanticTags)
    {
      using tag_t = decltype(SerializedActor.semantic_tags)::value_type;
      SerializedActor.semantic_tags.emplace_back(static_cast<tag_t>(Tag));
    }
    return SerializedActor;
  }
}

void UCarlaEpisode::AttachActors(
    AActor *Child,
    AActor *Parent,
    EAttachmentType InAttachmentType)
{
  Child->AddActorWorldOffset(FVector(CurrentMapOrigin));

  UActorAttacher::AttachActors(Child, Parent, InAttachmentType);

  if (bIsPrimaryServer)
  {
    GetFrameData().AddEvent(
        CarlaRecorderEventParent{
          FindCarlaActor(Child)->GetActorId(),
          FindCarlaActor(Parent)->GetActorId()});
  }
  // recorder event
  if (Recorder->IsEnabled())
  {
    CarlaRecorderEventParent RecEvent
    {
      FindCarlaActor(Child)->GetActorId(),
      FindCarlaActor(Parent)->GetActorId()
    };
    Recorder->AddEvent(std::move(RecEvent));
  }
}

void UCarlaEpisode::InitializeAtBeginPlay()
{
  auto World = GetWorld();
  check(World != nullptr);
  auto PlayerController = UGameplayStatics::GetPlayerController(World, 0);
  if (PlayerController == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("Can't find player controller!"));
    return;
  }
  Spectator = PlayerController->GetPawn();
  if (Spectator != nullptr)
  {
    FActorDescription Description;
    Description.Id = TEXT("spectator");
    Description.Class = Spectator->GetClass();
    ActorDispatcher->RegisterActor(*Spectator, Description);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Can't find spectator!"));
  }

  // material parameters collection
  UMaterialParameterCollection *Collection = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Game/Carla/Blueprints/Game/CarlaParameters.CarlaParameters"), nullptr, LOAD_None, nullptr);
	if (Collection != nullptr)
  {
    MaterialParameters = World->GetParameterCollectionInstance(Collection);
    if (MaterialParameters == nullptr)
    {
      UE_LOG(LogCarla, Error, TEXT("Can't find CarlaParameters instance!"));
    }
  }
  else
	{
    UE_LOG(LogCarla, Error, TEXT("Can't find CarlaParameters asset!"));
	}

  for (TActorIterator<ATrafficSignBase> It(World); It; ++It)
  {
    ATrafficSignBase *Actor = *It;
    check(Actor != nullptr);
    FActorDescription Description;
    Description.Id = UCarlaEpisode_GetTrafficSignId(Actor->GetTrafficSignState());
    Description.Class = Actor->GetClass();
    ActorDispatcher->RegisterActor(*Actor, Description);
  }

  // get the definition id for static.prop.mesh
  auto Definitions = GetActorDefinitions();
  uint32 StaticMeshUId = 0;
  for (auto& Definition : Definitions)
  {
    if (Definition.Id == "static.prop.mesh")
    {
      StaticMeshUId = Definition.UId;
      break;
    }
  }

  for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
  {
    auto Actor = *It;
    check(Actor != nullptr);
    auto MeshComponent = Actor->GetStaticMeshComponent();
    check(MeshComponent != nullptr);
    if (MeshComponent->Mobility == EComponentMobility::Movable)
    {
      FActorDescription Description;
      Description.Id = TEXT("static.prop.mesh");
      Description.UId = StaticMeshUId;
      Description.Class = Actor->GetClass();
      Description.Variations.Add("mesh_path",
          FActorAttribute{"mesh_path", EActorAttributeType::String,
          MeshComponent->GetStaticMesh()->GetPathName()});
      Description.Variations.Add("mass",
          FActorAttribute{"mass", EActorAttributeType::Float,
          FString::SanitizeFloat(MeshComponent->GetMass())});
      ActorDispatcher->RegisterActor(*Actor, Description);
    }
  }
}

void UCarlaEpisode::EndPlay(void)
{
  // stop recorder and replayer
  if (Recorder)
  {
    Recorder->Stop();
    if (Recorder->GetReplayer()->IsEnabled())
    {
      Recorder->GetReplayer()->Stop();
    }
  }
}

std::string UCarlaEpisode::StartRecorder(std::string Name, bool AdditionalData, bool StopReplayer)
{
  std::string result;

  if (Recorder)
  {
    result = Recorder->Start(Name, MapName, AdditionalData, StopReplayer);
  }
  else
  {
    result = "Recorder is not ready";
  }

  return result;
}

TPair<EActorSpawnResultStatus, FCarlaActor*> UCarlaEpisode::SpawnActorWithInfo(
    const FTransform &Transform,
    FActorDescription thisActorDescription,
    FCarlaActor::IdType DesiredId)
{
  ALargeMapManager* LargeMap = UCarlaStatics::GetLargeMapManager(GetWorld());
  FTransform LocalTransform = Transform;
  if(LargeMap)
  {
    LocalTransform = LargeMap->GlobalToLocalTransform(LocalTransform);
  }

  // NewTransform.AddToTranslation(-1.0f * FVector(CurrentMapOrigin));
  auto result = ActorDispatcher->SpawnActor(LocalTransform, thisActorDescription, DesiredId);
  if (result.Key == EActorSpawnResultStatus::Success &&
      GetWorld()->GetWorldPartition() != nullptr)
  {
    // World Partition map: every physics actor needs the ground under it
    // resident, mirroring the tile streaming the legacy LargeMapManager
    // provided on tiled maps. The engine only streams cells around registered
    // streaming sources; without one, a vehicle spawned away from the
    // spectator free-falls through the unloaded road.
    AActor* Actor = result.Value->GetActor();
    const bool bNeedsGround =
        Cast<ACarlaWheeledVehicle>(Actor) != nullptr ||
        Cast<AWalkerBase>(Actor) != nullptr;
    if (bNeedsGround &&
        !Actor->FindComponentByClass<UWorldPartitionStreamingSourceComponent>())
    {
      // The hero keeps a wide loading ring like the legacy tiled maps gave
      // it; background traffic only needs the ground in its vicinity.
      const FActorAttribute* Attribute =
          thisActorDescription.Variations.Find("role_name");
      const bool bIsHero = Attribute && (Attribute->Value.Contains("hero") ||
                                         Attribute->Value.Contains("ego_vehicle"));
      auto* Source = NewObject<UWorldPartitionStreamingSourceComponent>(Actor);
      FStreamingSourceShape Shape;
      Shape.bUseGridLoadingRange = false;
      Shape.Radius = bIsHero ? EpisodeSettings.TileStreamingDistance : 20000.0f;
      Source->Shapes.Add(Shape);
      Source->RegisterComponent();

      // Physics drops the actor immediately, but cells only stream during the
      // world partition subsystem update; run one update now and block until
      // the requested cells are resident so the actor never outruns its
      // ground.
      UWorldPartitionSubsystem* WPSubsystem =
          GetWorld()->GetSubsystem<UWorldPartitionSubsystem>();
      if (WPSubsystem != nullptr && !WPSubsystem->IsStreamingCompleted(Source))
      {
        WPSubsystem->OnUpdateStreamingState();
        GetWorld()->BlockTillLevelStreamingCompleted();
      }
      UE_LOG(LogCarla, Log, TEXT(
          "WP streaming source on %s (role %s, radius %.0f cm): completed=%d"),
          *Actor->GetName(), Attribute ? *Attribute->Value : TEXT("none"), Shape.Radius,
          WPSubsystem ? WPSubsystem->IsStreamingCompleted(Source) : -1);
    }
  }
  if (result.Key == EActorSpawnResultStatus::Success && bIsPrimaryServer)
  {
    if (Recorder->IsEnabled())
    {
      Recorder->CreateRecorderEventAdd(
        result.Value->GetActorId(),
        static_cast<uint8_t>(result.Value->GetActorType()),
        Transform,
        thisActorDescription
      );
    }
    if (bIsPrimaryServer)
    {
      GetFrameData().CreateRecorderEventAdd(
          result.Value->GetActorId(),
          static_cast<uint8_t>(result.Value->GetActorType()),
          Transform,
          std::move(thisActorDescription));
    }
  }

  return result;
}
