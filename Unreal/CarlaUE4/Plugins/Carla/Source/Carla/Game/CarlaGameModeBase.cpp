// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Game/CarlaGameModeBase.h"

#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaHUD.h"
#include "Carla/Game/CarlaPlayerState.h"
#include "Carla/Game/Tagger.h"
#include "Carla/Game/TaggerDelegate.h"
#include "Carla/Sensor/DeprecatedSensor.h"
#include "Carla/Sensor/OldSensorFactory.h"
#include "Carla/Settings/CarlaSettings.h"
#include "Carla/Settings/CarlaSettingsDelegate.h"
#include "Carla/Util/RandomEngine.h"
#include "Carla/Vehicle/CarlaVehicleController.h"

#include "ConstructorHelpers.h"
#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "SceneViewport.h"

ACarlaGameModeBase::ACarlaGameModeBase(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer),
  GameController(nullptr),
  PlayerController(nullptr)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;
  bAllowTickBeforeBeginPlay = false;

  PlayerControllerClass = ACarlaVehicleController::StaticClass();
  PlayerStateClass = ACarlaPlayerState::StaticClass();
  HUDClass = ACarlaHUD::StaticClass();

  TaggerDelegate = CreateDefaultSubobject<UTaggerDelegate>(TEXT("TaggerDelegate"));
  CarlaSettingsDelegate = CreateDefaultSubobject<UCarlaSettingsDelegate>(TEXT("CarlaSettingsDelegate"));
}

void ACarlaGameModeBase::InitGame(
    const FString &MapName,
    const FString &Options,
    FString &ErrorMessage)
{
  Super::InitGame(MapName, Options, ErrorMessage);

  GameInstance = Cast<UCarlaGameInstance>(GetGameInstance());
  checkf(
      GameInstance != nullptr,
      TEXT("GameInstance is not a UCarlaGameInstance, did you forget to set it in the project settings?"));

  GameInstance->InitializeGameControllerIfNotPresent(MockGameControllerSettings);
  GameController = &GameInstance->GetGameController();
  auto &CarlaSettings = GameInstance->GetCarlaSettings();
  UWorld *world = GetWorld();
  { // Load weather descriptions and initialize game controller.
#if WITH_EDITOR
    {
      // Hack to be able to test level-specific weather descriptions in editor.
      // When playing in editor the map name gets an extra prefix, here we
      // remove it.
      FString CorrectedMapName = MapName;
      constexpr auto PIEPrefix = TEXT("UEDPIE_0_");
      CorrectedMapName.RemoveFromStart(PIEPrefix);
      UE_LOG(LogCarla, Log, TEXT("Corrected map name from %s to %s"), *MapName, *CorrectedMapName);
      CarlaSettings.MapName = CorrectedMapName;
      CarlaSettings.LoadWeatherDescriptions();
    }
#else
    CarlaSettings.MapName = MapName;
    CarlaSettings.LoadWeatherDescriptions();
#endif // WITH_EDITOR
    GameController->Initialize(CarlaSettings);
    CarlaSettings.ValidateWeatherId();
    CarlaSettings.LogSettings();
  }

  // Set default pawn class.
  if (!CarlaSettings.PlayerVehicle.IsEmpty())
  {
    auto Class = FindObject<UClass>(ANY_PACKAGE, *CarlaSettings.PlayerVehicle);
    if (Class)
    {
      DefaultPawnClass = Class;
    }
    else
    {
      UE_LOG(LogCarla, Error, TEXT("Failed to load player pawn class \"%s\""), *CarlaSettings.PlayerVehicle)
    }
  }

  if (TaggerDelegate != nullptr)
  {
    TaggerDelegate->RegisterSpawnHandler(world);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Missing TaggerDelegate!"));
  }

  if(CarlaSettingsDelegate!=nullptr)
  {
    //apply quality settings
    CarlaSettingsDelegate->ApplyQualitySettingsLevelPostRestart();
    //assign settings delegate for every new actor from now on
    CarlaSettingsDelegate->RegisterSpawnHandler(world);

  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Missing CarlaSettingsDelegate!"));
  }

  if (DynamicWeatherClass != nullptr)
  {
    DynamicWeather = world->SpawnActor<ADynamicWeather>(DynamicWeatherClass);
  }

  if (VehicleSpawnerClass != nullptr)
  {
    VehicleSpawner = world->SpawnActor<AVehicleSpawnerBase>(VehicleSpawnerClass);
  }

  if (WalkerSpawnerClass != nullptr)
  {
    WalkerSpawner = world->SpawnActor<AWalkerSpawnerBase>(WalkerSpawnerClass);
  }
}

void ACarlaGameModeBase::RestartPlayer(AController* NewPlayer)
{
  check(NewPlayer != nullptr);
  TArray<APlayerStart *> UnOccupiedStartPoints;
  APlayerStart *PlayFromHere = FindUnOccupiedStartPoints(NewPlayer, UnOccupiedStartPoints);
  bool bStartSpotFound = false;
  if (PlayFromHere != nullptr)
  {
    RestartPlayerAtPlayerStart(NewPlayer, PlayFromHere);
    RegisterPlayer(*NewPlayer);
    bStartSpotFound = true;
  }
  else if (UnOccupiedStartPoints.Num() > 0u)
  {
    check(GameController != nullptr);
    APlayerStart *StartSpot = GameController->ChoosePlayerStart(UnOccupiedStartPoints);
    if (StartSpot != nullptr)
    {
      RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
      RegisterPlayer(*NewPlayer);
      bStartSpotFound = true;
    }
  }
  if(!bStartSpotFound)
  {
    UE_LOG(LogCarla, Error, TEXT("No start spot found!"));
  }
  if(CarlaSettingsDelegate != nullptr)
  {
    CarlaSettingsDelegate->ApplyQualitySettingsLevelPreRestart();
  }
}

void ACarlaGameModeBase::BeginPlay()
{
  Super::BeginPlay();

  const auto &CarlaSettings = GameInstance->GetCarlaSettings();

  // Setup semantic segmentation if necessary.
  if (CarlaSettings.bSemanticSegmentationEnabled)
  {
    TagActorsForSemanticSegmentation();
    TaggerDelegate->SetSemanticSegmentationEnabled();
  }

  // Change weather.
  if (DynamicWeather != nullptr)
  {
    const auto *Weather = CarlaSettings.GetActiveWeatherDescription();
    if (Weather != nullptr)
    {
      UE_LOG(LogCarla, Log, TEXT("Changing weather settings to \"%s\""), *Weather->Name);
      DynamicWeather->SetWeatherDescription(*Weather);
      DynamicWeather->RefreshWeather();
    }
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Missing dynamic weather actor!"));
  }

  // Find road map.
  TActorIterator<ACityMapGenerator> It(GetWorld());
  URoadMap *RoadMap = (It ? It->GetRoadMap() : nullptr);

  if (PlayerController != nullptr)
  {
    PlayerController->SetRoadMap(RoadMap);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Player controller is not a AWheeledVehicleAIController!"));
  }

  // Setup other vehicles.
  if (VehicleSpawner != nullptr)
  {
    VehicleSpawner->SetNumberOfVehicles(CarlaSettings.NumberOfVehicles);
    VehicleSpawner->SetSeed(CarlaSettings.SeedVehicles);
    VehicleSpawner->SetRoadMap(RoadMap);
    if (PlayerController != nullptr)
    {
      PlayerController->GetRandomEngine()->Seed(
          VehicleSpawner->GetRandomEngine()->GenerateSeed());
    }
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Missing vehicle spawner actor!"));
  }

  // Setup walkers.
  if (WalkerSpawner != nullptr)
  {
    WalkerSpawner->SetNumberOfWalkers(CarlaSettings.NumberOfPedestrians);
    WalkerSpawner->SetSeed(CarlaSettings.SeedPedestrians);
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("Missing walker spawner actor!"));
  }

  GameController->BeginPlay();
}

void ACarlaGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (CarlaSettingsDelegate != nullptr && EndPlayReason!=EEndPlayReason::EndPlayInEditor)
	{
	  CarlaSettingsDelegate->Reset();
	}
}

void ACarlaGameModeBase::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);
  GameController->Tick(DeltaSeconds);
}

void ACarlaGameModeBase::RegisterPlayer(AController &NewPlayer)
{
  check(GameController != nullptr);
  AddTickPrerequisiteActor(&NewPlayer);
  PlayerController = Cast<ACarlaVehicleController>(&NewPlayer);
  if (PlayerController != nullptr)
  {
    GetDataRouter().RegisterPlayer(*PlayerController);
    GameController->RegisterPlayer(*PlayerController);
    AttachSensorsToPlayer();
  }
  else
  {
    UE_LOG(LogCarla, Error, TEXT("ACarlaGameModeBase: Player is not a ACarlaVehicleController"));
  }
}

void ACarlaGameModeBase::AttachSensorsToPlayer()
{
  check(PlayerController != nullptr);
  const auto &Settings = GameInstance->GetCarlaSettings();
  const auto *Weather = Settings.GetActiveWeatherDescription();

  for (auto &Item : Settings.SensorDescriptions)
  {
    check(Item.Value != nullptr);
    auto &SensorDescription = *Item.Value;
    if (Weather != nullptr)
    {
      SensorDescription.AdjustToWeather(*Weather);
    }
    auto *Sensor = FSensorFactory::Make(SensorDescription, *GetWorld());
    check(Sensor != nullptr);
    Sensor->AttachToActor(PlayerController->GetPawn());
    GetDataRouter().RegisterSensor(*Sensor);
  }
}

void ACarlaGameModeBase::TagActorsForSemanticSegmentation()
{
  check(GetWorld() != nullptr);
  ATagger::TagActorsInLevel(*GetWorld(), true);
}

APlayerStart *ACarlaGameModeBase::FindUnOccupiedStartPoints(
    AController *Player,
    TArray<APlayerStart *> &UnOccupiedStartPoints)
{
  APlayerStart* FoundPlayerStart = nullptr;
  UClass* PawnClass = GetDefaultPawnClassForController(Player);
  APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
  for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
  {
    APlayerStart* PlayerStart = *It;

    if (PlayerStart->IsA<APlayerStartPIE>())
    {
      FoundPlayerStart = PlayerStart;
      break;
    }
    else
    {
      FVector ActorLocation = PlayerStart->GetActorLocation();
      const FRotator ActorRotation = PlayerStart->GetActorRotation();
      if (!GetWorld()->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation))
      {
        UnOccupiedStartPoints.Add(PlayerStart);
      }
#if WITH_EDITOR
      else if (GetWorld()->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation))
      {
        UE_LOG(
            LogCarla,
            Warning,
            TEXT("Player start cannot be used, occupied location: %s"),
            *PlayerStart->GetActorLocation().ToString());
      }
#endif // WITH_EDITOR
    }
  }
  return FoundPlayerStart;
}
