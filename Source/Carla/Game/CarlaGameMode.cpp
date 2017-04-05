// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameMode.h"

#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

#include "SceneCaptureCamera.h"

#include "CarlaGameInstance.h"
#include "CarlaGameState.h"
#include "CarlaHUD.h"
#include "CarlaPlayerState.h"
#include "CarlaSettings.h"
#include "CarlaVehicleController.h"

ACarlaGameMode::ACarlaGameMode() :
  Super(),
  GameController(nullptr),
  PlayerController(nullptr)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;

  PlayerControllerClass = ACarlaVehicleController::StaticClass();
  GameStateClass = ACarlaGameState::StaticClass();
  PlayerStateClass = ACarlaPlayerState::StaticClass();
  HUDClass = ACarlaHUD::StaticClass();
}

void ACarlaGameMode::InitGame(
    const FString &MapName,
    const FString &Options,
    FString &ErrorMessage)
{
  Super::InitGame(MapName, Options, ErrorMessage);

  GameInstance = Cast<UCarlaGameInstance>(GetGameInstance());
  checkf(
      GameInstance != nullptr,
      TEXT("GameInstance is not a UCarlaGameInstance, did you forget to set it in the project settings?"));
  GameInstance->InitializeGameControllerIfNotPresent();
  GameController = &GameInstance->GetGameController();
  GameController->Initialize(GameInstance->GetCarlaSettings());
  GameInstance->GetCarlaSettings().LogSettings();
}

void ACarlaGameMode::RestartPlayer(AController* NewPlayer)
{
  check(NewPlayer != nullptr);
  TArray<APlayerStart *> UnOccupiedStartPoints;
  APlayerStart *PlayFromHere = FindUnOccupiedStartPoints(NewPlayer, UnOccupiedStartPoints);
  if (PlayFromHere != nullptr) {
    RestartPlayerAtPlayerStart(NewPlayer, PlayFromHere);
    RegisterPlayer(*NewPlayer);
    return;
  } else if (UnOccupiedStartPoints.Num() > 0u) {
    check(GameController != nullptr);
    APlayerStart *StartSpot = GameController->ChoosePlayerStart(UnOccupiedStartPoints);
    if (StartSpot != nullptr) {
      RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
      RegisterPlayer(*NewPlayer);
      return;
    }
  }
  UE_LOG(LogCarla, Error, TEXT("No start spot found!"));
}

void ACarlaGameMode::BeginPlay()
{
  Super::BeginPlay();
  GameController->BeginPlay();
}

void ACarlaGameMode::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);
  GameController->Tick(DeltaSeconds);
}

void ACarlaGameMode::RegisterPlayer(AController &NewPlayer)
{
  check(GameController != nullptr);
  AddTickPrerequisiteActor(&NewPlayer);
  GameController->RegisterPlayer(NewPlayer);
  PlayerController = &NewPlayer;
  AttachCaptureCamerasToPlayer(*PlayerController);
}

void ACarlaGameMode::RegisterCaptureCamera(
    ASceneCaptureCamera &CaptureCamera,
    AController &Player)
{
  AddTickPrerequisiteActor(&CaptureCamera);
  ACarlaVehicleController *Vehicle = Cast<ACarlaVehicleController>(&Player);
  if (Vehicle != nullptr) {
    Vehicle->RegisterCaptureCamera(CaptureCamera);
  } else {
    UE_LOG(LogCarla, Warning, TEXT("Trying to register camera but player is not a ACarlaVehicleController"));
  }
}

static ASceneCaptureCamera *SpawnAndAttachCamera(
    AController &Player,
    const FVector &RelativeLocation,
    const FRotator &RelativeRotation,
    uint32 ImageSizeX,
    uint32 ImageSizeY,
    EPostProcessEffect PostProcessEffect)
{
  auto Camera = Player.GetWorld()->SpawnActor<ASceneCaptureCamera>(RelativeLocation, RelativeRotation);
  Camera->SetImageSize(ImageSizeX, ImageSizeY);
  Camera->SetPostProcessEffect(PostProcessEffect);
  Camera->AttachToActor(Player.GetPawn(), FAttachmentTransformRules::KeepRelativeTransform);
  Camera->SetOwner(Player.GetPawn());
  return Camera;
}

void ACarlaGameMode::AttachCaptureCamerasToPlayer(AController &Player)
{
  auto &Settings = GameInstance->GetCarlaSettings();
  if (Settings.SceneCaptureMode == ESceneCaptureMode::Mono) {
    auto TheCamera =
        SpawnAndAttachCamera(
            Player,
            Settings.Mono_CameraPosition,
            Settings.Mono_CameraRotation,
            Settings.Mono_ImageSizeX,
            Settings.Mono_ImageSizeY,
            EPostProcessEffect::SceneFinal);
    RegisterCaptureCamera(*TheCamera, Player);
  } else if (Settings.SceneCaptureMode == ESceneCaptureMode::Stereo) {
    auto RGBCamera0 =
        SpawnAndAttachCamera(
            Player,
            Settings.Stereo_Camera0Position,
            Settings.Stereo_Camera0Rotation,
            Settings.Stereo_ImageSizeX,
            Settings.Stereo_ImageSizeY,
            EPostProcessEffect::SceneFinal);
    auto DepthCamera0 =
        SpawnAndAttachCamera(
            Player,
            Settings.Stereo_Camera0Position,
            Settings.Stereo_Camera0Rotation,
            Settings.Stereo_ImageSizeX,
            Settings.Stereo_ImageSizeY,
            EPostProcessEffect::Depth);
    auto RGBCamera1 =
        SpawnAndAttachCamera(
            Player,
            Settings.Stereo_Camera1Position,
            Settings.Stereo_Camera1Rotation,
            Settings.Stereo_ImageSizeX,
            Settings.Stereo_ImageSizeY,
            EPostProcessEffect::SceneFinal);
    auto DepthCamera1 =
        SpawnAndAttachCamera(
            Player,
            Settings.Stereo_Camera1Position,
            Settings.Stereo_Camera1Rotation,
            Settings.Stereo_ImageSizeX,
            Settings.Stereo_ImageSizeY,
            EPostProcessEffect::Depth);
    RegisterCaptureCamera(*RGBCamera0, Player);
    RegisterCaptureCamera(*DepthCamera0, Player);
    RegisterCaptureCamera(*RGBCamera1, Player);
    RegisterCaptureCamera(*DepthCamera1, Player);
  }
}

APlayerStart *ACarlaGameMode::FindUnOccupiedStartPoints(
    AController *Player,
    TArray<APlayerStart *> &UnOccupiedStartPoints)
{
  APlayerStart* FoundPlayerStart = nullptr;
  UClass* PawnClass = GetDefaultPawnClassForController(Player);
  APawn* PawnToFit = PawnClass ? PawnClass->GetDefaultObject<APawn>() : nullptr;
  for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It) {
    APlayerStart* PlayerStart = *It;

    if (PlayerStart->IsA<APlayerStartPIE>()) {
      FoundPlayerStart = PlayerStart;
      break;
    } else {
      FVector ActorLocation = PlayerStart->GetActorLocation();
      const FRotator ActorRotation = PlayerStart->GetActorRotation();
      if (!GetWorld()->EncroachingBlockingGeometry(PawnToFit, ActorLocation, ActorRotation)) {
        UnOccupiedStartPoints.Add(PlayerStart);
      }
#ifdef WITH_EDITOR
      else if (GetWorld()->FindTeleportSpot(PawnToFit, ActorLocation, ActorRotation)) {
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
