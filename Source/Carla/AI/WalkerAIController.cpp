// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "WalkerAIController.h"

#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "WheeledVehicle.h"
#include "WheeledVehicleMovementComponent.h"

#ifdef CARLA_AI_WALKERS_EXTRA_LOG
#  define LOG_AI_WALKER(Verbosity, Text) UE_LOG(LogCarla, Verbosity, Text, *GetPawn()->GetName());
#else
#  define LOG_AI_WALKER(Verbosity, Text)
#endif // CARLA_AI_WALKERS_EXTRA_LOG

AWalkerAIController::AWalkerAIController(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickInterval = 5.0f; // seconds.

  auto Perception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception Component"));
  check(Perception != nullptr);
  SetPerceptionComponent(*Perception);

  SightConfiguration = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfiguration"));
  SightConfiguration->SightRadius = 800.0f;
  SightConfiguration->LoseSightRadius = (SightConfiguration->SightRadius + 100.0f);
  SightConfiguration->PeripheralVisionAngleDegrees = 90.0f;
  SightConfiguration->DetectionByAffiliation.bDetectEnemies = true;
  SightConfiguration->DetectionByAffiliation.bDetectNeutrals = true;
  SightConfiguration->DetectionByAffiliation.bDetectFriendlies = true;

  Perception->ConfigureSense(*SightConfiguration);
  Perception->SetDominantSense(SightConfiguration->GetSenseImplementation());
  Perception->OnPerceptionUpdated.AddDynamic(this, &AWalkerAIController::SenseActors);
}

void AWalkerAIController::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  switch (GetMoveStatus()) {
    case EPathFollowingStatus::Idle:
    case EPathFollowingStatus::Waiting:
      LOG_AI_WALKER(Warning, TEXT("Walker %s is stuck!"));
      bIsStuck = true;
      break;
    case EPathFollowingStatus::Paused:
      LOG_AI_WALKER(Log, TEXT("Walker %s is paused"));
      bIsPaused = true;
      TryResumeMovement();
      break;
  };
}

FPathFollowingRequestResult AWalkerAIController::MoveTo(
      const FAIMoveRequest& MoveRequest,
      FNavPathSharedPtr* OutPath)
{
  bIsStuck = false;
  LOG_AI_WALKER(Log, TEXT("Walker %s requested to move"));
  return Super::MoveTo(MoveRequest, OutPath);
}

static bool VehicleIsMoving(const AWheeledVehicle *Vehicle)
{
  const auto *MovementComponent =
      (Vehicle != nullptr ? Vehicle->GetVehicleMovementComponent() : nullptr);
  return
      (MovementComponent != nullptr) &&
      (FMath::Abs(MovementComponent->GetForwardSpeed()) > 0.0f);
}

static bool ContainsAMovingVehicle(const TArray<AActor *> &Actors)
{
  for (auto *Actor : Actors) {
    if (VehicleIsMoving(Cast<AWheeledVehicle>(Actor))) {
      return true;
    }
  }
  return false;
}

void AWalkerAIController::SenseActors(const TArray<AActor *> Actors)
{
  if (!bIsPaused && ContainsAMovingVehicle(Actors)) {
    TryPauseMovement();
  }
}

void AWalkerAIController::TryResumeMovement()
{
  auto MoveRequestID = GetCurrentMoveRequestID();
  if (MoveRequestID == FAIRequestID()) { // equals invalid request.
    LOG_AI_WALKER(Error, TEXT("Walker %s: Invalid move ID"));
  } else {
    if (!ResumeMove(MoveRequestID)) {
      LOG_AI_WALKER(Error, TEXT("Unable to resume walker %s move"));
    } else {
      LOG_AI_WALKER(Log, TEXT("Resume walker %s's move"));
      bIsPaused = false;
    }
  }
}

void AWalkerAIController::TryPauseMovement()
{
  auto MoveRequestID = GetCurrentMoveRequestID();
  if (MoveRequestID == FAIRequestID()) { // equals invalid request.
    LOG_AI_WALKER(Error, TEXT("Walker %s: Invalid move ID"));
  } else {
    if (!PauseMove(MoveRequestID)) {
      LOG_AI_WALKER(Error, TEXT("Unable to pause walker %s move"));
    } else {
      LOG_AI_WALKER(Log, TEXT("Pause walker %s's move"));
      bIsPaused = true;
    }
  }
}
