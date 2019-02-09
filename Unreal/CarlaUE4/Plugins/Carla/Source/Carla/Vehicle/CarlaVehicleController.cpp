// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CarlaVehicleController.h"

#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "WheeledVehicle.h"
#include "WheeledVehicleMovementComponent.h"

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

ACarlaVehicleController::ACarlaVehicleController(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

ACarlaVehicleController::~ACarlaVehicleController() {}

// =============================================================================
// -- APlayerController --------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::Possess(APawn *aPawn)
{
  Super::Possess(aPawn);

  if (IsPossessingAVehicle()) {
    // Bind hit events.
    aPawn->OnActorHit.AddDynamic(this, &ACarlaVehicleController::OnCollisionEvent);
    // Get custom player state.
    CarlaPlayerState = Cast<ACarlaPlayerState>(PlayerState);
    check(CarlaPlayerState != nullptr);
    // We can set the bounding box already as it's not going to change.
    CarlaPlayerState->BoundingBoxTransform = GetPossessedVehicle()->GetVehicleBoundingBoxTransform();
    CarlaPlayerState->BoundingBoxExtent = GetPossessedVehicle()->GetVehicleBoundingBoxExtent();
    // Set HUD input.
    CarlaHUD = Cast<ACarlaHUD>(GetHUD());
    if (CarlaHUD != nullptr) {
      InputComponent->BindAction("ToggleHUD", IE_Pressed, CarlaHUD, &ACarlaHUD::ToggleHUDView);
      CarlaHUD->bShowHUD = !(Cast<UCarlaGameInstance>(GetWorld()->GetGameInstance())->GetCarlaSettings().bDisableRendering);
      CarlaHUD->SetVisible(CarlaHUD->bShowHUD);
    } else {
      UE_LOG(LogCarla, Warning, TEXT("Current HUD is not a ACarlaHUD"));
    }
  }
}

// =============================================================================
// -- AActor -------------------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (IsPossessingAVehicle()) {
    auto CurrentVehicle = GetPossessedVehicle();
    CarlaPlayerState->UpdateTimeStamp(DeltaTime);
    const FVector PreviousSpeed = CarlaPlayerState->ForwardSpeed * CarlaPlayerState->GetOrientation();
    CarlaPlayerState->Transform = CurrentVehicle->GetVehicleTransform();
    CarlaPlayerState->ForwardSpeed = CurrentVehicle->GetVehicleForwardSpeed();
    const FVector CurrentSpeed = CarlaPlayerState->ForwardSpeed * CarlaPlayerState->GetOrientation();
    CarlaPlayerState->Acceleration = (CurrentSpeed - PreviousSpeed) / DeltaTime;
    const auto &AutopilotCtrl = GetAutopilotControl();
    CarlaPlayerState->Steer = AutopilotCtrl.Steer;
    CarlaPlayerState->Throttle = AutopilotCtrl.Throttle;
    CarlaPlayerState->Brake = AutopilotCtrl.Brake;
    CarlaPlayerState->bHandBrake = AutopilotCtrl.bHandBrake;
    CarlaPlayerState->CurrentGear = CurrentVehicle->GetVehicleCurrentGear();
    CarlaPlayerState->SpeedLimit = GetSpeedLimit();
    CarlaPlayerState->TrafficLightState = GetTrafficLightState();
    IntersectPlayerWithRoadMap();
  }
}

// =============================================================================
// -- Events -------------------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::OnCollisionEvent(
    AActor* Actor,
    AActor* OtherActor,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
  // Register collision only if we are moving faster than 1 km/h.
  check(IsPossessingAVehicle());
  if (FMath::Abs(GetPossessedVehicle()->GetVehicleForwardSpeed() * 0.036f) > 1.0f) {
    CarlaPlayerState->RegisterCollision(Actor, OtherActor, NormalImpulse, Hit);
  }
}

// =============================================================================
// -- Other --------------------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::IntersectPlayerWithRoadMap()
{
  auto CurrentRoadMap = GetRoadMap();
  if (CurrentRoadMap == nullptr) {
    return;
  }

  check(IsPossessingAVehicle());
  auto CurrentVehicle = GetPossessedVehicle();
  constexpr float ChecksPerCentimeter = 0.1f;
  const auto *BoundingBox = CurrentVehicle->GetVehicleBoundingBox();
  check(BoundingBox != nullptr);
  auto Result = CurrentRoadMap->Intersect(
      BoundingBox->GetComponentTransform(),
      BoundingBox->GetUnscaledBoxExtent(),
      ChecksPerCentimeter);

  CarlaPlayerState->OffRoadIntersectionFactor = Result.OffRoad;
  CarlaPlayerState->OtherLaneIntersectionFactor = Result.OppositeLane;
}
