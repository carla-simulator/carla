// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaStatics.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDriveParser.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>

#include "Carla/Sensor/Sensor.h"
#include "Carla/Util/BoundingBoxCalculator.h"
#include "Carla/Util/RandomEngine.h"
#include "Carla/Vehicle/VehicleSpawnPoint.h"
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Game/CarlaStaticDelegates.h"
#include "Carla/MapGen/LargeMapManager.h"

#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/SpectatorPawn.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

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
}

bool UCarlaEpisode::LoadNewEpisode(const FString &MapString, bool ResetSettings)
{
  bool bIsFileFound = false;

  FString FinalPath = MapString.IsEmpty() ? GetMapName() : MapString;
  FinalPath += !MapString.EndsWith(".umap") ? ".umap" : "";

  if (MapString.StartsWith("/Game"))
  {
    // Some conversions...
    FinalPath.RemoveFromStart(TEXT("/Game/"));
    FinalPath = FPaths::ProjectContentDir() + FinalPath;
    FinalPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FinalPath);

    if (FPaths::FileExists(FinalPath)) {
      bIsFileFound = true;
      FinalPath = MapString;
    }
  }
  else
  {
    if (MapString.Contains("/")) return false;

    // Find the full path under Carla
    TArray<FString> TempStrArray, PathList;
    IFileManager::Get().FindFilesRecursive(PathList, *FPaths::ProjectContentDir(), *FinalPath, true, false, false);
    if (PathList.Num() > 0)
    {
      FinalPath = PathList[0];
      FinalPath.ParseIntoArray(TempStrArray, TEXT("Content/"), true);
      FinalPath = TempStrArray[1];
      FinalPath.ParseIntoArray(TempStrArray, TEXT("."), true);
      FinalPath = "/Game/" + TempStrArray[0];

      return LoadNewEpisode(FinalPath, ResetSettings);
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

static FString BuildRecastBuilderFile()
{
  // Define filename with extension depending on if we are on Windows or not
#if PLATFORM_WINDOWS
  const FString RecastToolName = "RecastBuilder.exe";
#else
  const FString RecastToolName = "RecastBuilder";
#endif // PLATFORM_WINDOWS

  // Define path depending on the UE4 build type (Package or Editor)
#if UE_BUILD_SHIPPING
  const FString AbsoluteRecastBuilderPath = FPaths::ConvertRelativePathToFull(
      FPaths::RootDir() + "Tools/" + RecastToolName);
#else
  const FString AbsoluteRecastBuilderPath = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectDir() + "../../Util/DockerUtils/dist/" + RecastToolName);
#endif
  return AbsoluteRecastBuilderPath;
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

  const FString AbsoluteOBJPath = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectContentDir() + "Carla/Maps/Nav/OpenDriveMap.obj");

  // Store the OBJ string to a file in order to that RecastBuilder can load it
  FFileHelper::SaveStringToFile(
      carla::rpc::ToLongFString(RecastOBJ),
      *AbsoluteOBJPath,
      FFileHelper::EEncodingOptions::ForceUTF8,
      &IFileManager::Get());

  const FString AbsoluteXODRPath = FPaths::ConvertRelativePathToFull(
      FPaths::ProjectContentDir() + "Carla/Maps/OpenDrive/OpenDriveMap.xodr");

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
    /// @todo this can take too long to finish, clients need a method
    /// to know if the navigation is available or not.
    FPlatformProcess::CreateProc(
        *AbsoluteRecastBuilderPath, *AbsoluteOBJPath,
        true, true, true, nullptr, 0, nullptr, nullptr);
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

std::string UCarlaEpisode::StartRecorder(std::string Name, bool AdditionalData)
{
  std::string result;

  if (Recorder)
  {
    result = Recorder->Start(Name, MapName, AdditionalData);
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
