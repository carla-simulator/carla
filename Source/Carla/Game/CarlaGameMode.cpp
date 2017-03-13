// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaGameMode.h"

#include "Engine/PlayerStartPIE.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"

#include "CarlaGameInstance.h"
#include "CarlaGameState.h"
#include "CarlaPlayerState.h"
#include "CarlaVehicleController.h"

ACarlaGameMode::ACarlaGameMode() :
  Super(),
  GameController(nullptr)
{
  PlayerControllerClass = ACarlaVehicleController::StaticClass();
  GameStateClass = ACarlaGameState::StaticClass();
  PlayerStateClass = ACarlaPlayerState::StaticClass();

  PrimaryActorTick.bCanEverTick = true;
}

void ACarlaGameMode::InitGame(
    const FString &MapName,
    const FString &Options,
    FString &ErrorMessage)
{
  Super::InitGame(MapName, Options, ErrorMessage);

  UCarlaGameInstance *GameInstance = Cast<UCarlaGameInstance>(GetGameInstance());
  checkf(
      GameInstance != nullptr,
      TEXT("GameInstance is not a UCarlaGameInstance, did you forget to set it in the project settings?"));
  GameInstance->InitializeGameControllerIfNotPresent(bUseMockController);
  GameController = &GameInstance->GetGameController();
}

void ACarlaGameMode::RestartPlayer(AController* NewPlayer)
{
  TArray<APlayerStart *> UnOccupiedStartPoints;
  APlayerStart *PlayFromHere = FindUnOccupiedStartPoints(NewPlayer, UnOccupiedStartPoints);
  if (PlayFromHere != nullptr) {
    RestartPlayerAtPlayerStart(NewPlayer, PlayFromHere);
    GameController->RegisterPlayer(NewPlayer);
    return;
  } else if (UnOccupiedStartPoints.Num() > 0u) {
    APlayerStart *StartSpot = GameController->ChoosePlayerStart(UnOccupiedStartPoints);
    if (StartSpot != nullptr) {
      RestartPlayerAtPlayerStart(NewPlayer, UnOccupiedStartPoints[0u]);
      GameController->RegisterPlayer(NewPlayer);
      return;
    }
  }
  UE_LOG(LogCarla, Error, TEXT("No start spot found!"));
}

void ACarlaGameMode::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);
  GameController->Tick(DeltaSeconds);
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
