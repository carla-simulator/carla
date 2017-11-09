// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB), and the INTEL Visual Computing Lab.
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "CarlaVehicleController.h"

#include "SceneCaptureCamera.h"

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
    // Set HUD input.
    CarlaHUD = Cast<ACarlaHUD>(GetHUD());
    if (CarlaHUD != nullptr) {
      InputComponent->BindAction("ToggleHUD", IE_Pressed, CarlaHUD, &ACarlaHUD::ToggleHUDView);
    } else {
      UE_LOG(LogCarla, Warning, TEXT("Current HUD is not a ACarlaHUD"));
    }
  }
}

// =============================================================================
// -- AActor -------------------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::BeginPlay()
{
  Super::BeginPlay();

  if (CarlaPlayerState != nullptr) {
    CarlaPlayerState->Images.Empty();
    const auto NumberOfCameras = SceneCaptureCameras.Num();
    if (NumberOfCameras > 0) {
      CarlaPlayerState->Images.AddDefaulted(NumberOfCameras);
      for (auto i = 0; i < NumberOfCameras; ++i) {
        auto *Camera = SceneCaptureCameras[i];
        check(Camera != nullptr);
        auto &Image = CarlaPlayerState->Images[i];
        Image.SizeX = Camera->GetImageSizeX();
        Image.SizeY = Camera->GetImageSizeY();
        Image.PostProcessEffect = Camera->GetPostProcessEffect();
      }
    }
  }
}

void ACarlaVehicleController::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (IsPossessingAVehicle()) {
    auto Vehicle = GetPossessedVehicle();
    CarlaPlayerState->UpdateTimeStamp(DeltaTime);
    const FVector PreviousSpeed = CarlaPlayerState->ForwardSpeed * CarlaPlayerState->GetOrientation();
    CarlaPlayerState->Transform = Vehicle->GetActorTransform();
    CarlaPlayerState->ForwardSpeed = Vehicle->GetVehicleForwardSpeed();
    const FVector CurrentSpeed = CarlaPlayerState->ForwardSpeed * CarlaPlayerState->GetOrientation();
    CarlaPlayerState->Acceleration = (CurrentSpeed - PreviousSpeed) / DeltaTime;
    const auto &AutopilotControl = GetAutopilotControl();
    CarlaPlayerState->Steer = AutopilotControl.Steer;
    CarlaPlayerState->Throttle = AutopilotControl.Throttle;
    CarlaPlayerState->Brake = AutopilotControl.Brake;
    CarlaPlayerState->bHandBrake = AutopilotControl.bHandBrake;
    CarlaPlayerState->CurrentGear = Vehicle->GetVehicleCurrentGear();
    CarlaPlayerState->SpeedLimit = GetSpeedLimit();
    CarlaPlayerState->TrafficLightState = GetTrafficLightState();
    IntersectPlayerWithRoadMap();
    const auto NumberOfCameras = SceneCaptureCameras.Num();
    check(NumberOfCameras == CarlaPlayerState->Images.Num());
    for (auto i = 0; i < NumberOfCameras; ++i) {
      auto &Image = CarlaPlayerState->Images[i];
      if (!SceneCaptureCameras[i]->ReadPixels(Image.BitMap)) {
        Image.BitMap.Empty();
      }
    }
  }
}

// =============================================================================
// -- Scene capture ------------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::AddSceneCaptureCamera(
    const FCameraDescription &Description,
    const FCameraPostProcessParameters *OverridePostProcessParameters)
{
  auto Camera = GetWorld()->SpawnActor<ASceneCaptureCamera>(Description.Position, Description.Rotation);
  if (OverridePostProcessParameters != nullptr) {
    Camera->Set(Description, *OverridePostProcessParameters);
  } else {
    Camera->Set(Description);
  }
  Camera->AttachToActor(GetPawn(), FAttachmentTransformRules::KeepRelativeTransform);
  Camera->SetOwner(GetPawn());
  AddTickPrerequisiteActor(Camera);
  SceneCaptureCameras.Add(Camera);
  UE_LOG(
      LogCarla,
      Log,
      TEXT("Created capture camera %d with postprocess \"%s\""),
      SceneCaptureCameras.Num() - 1,
      *PostProcessEffect::ToString(Camera->GetPostProcessEffect()));
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
  if (FMath::Abs(GetPossessedVehicle()->GetVehicleForwardSpeed()) > 1.0f) {
    CarlaPlayerState->RegisterCollision(Actor, OtherActor, NormalImpulse, Hit);
  }
}

// =============================================================================
// -- Other --------------------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::IntersectPlayerWithRoadMap()
{
  auto RoadMap = GetRoadMap();
  if (RoadMap == nullptr) {
    UE_LOG(LogCarla, Error, TEXT("Controller doesn't have a road map!"));
    return;
  }

  check(IsPossessingAVehicle());
  auto Vehicle = GetPossessedVehicle();
  constexpr float ChecksPerCentimeter = 0.1f;
  auto Result = RoadMap->Intersect(
      Vehicle->GetActorTransform(),
      Vehicle->GetVehicleBoundsExtent(),
      ChecksPerCentimeter);

  CarlaPlayerState->OffRoadIntersectionFactor = Result.OffRoad;
  CarlaPlayerState->OtherLaneIntersectionFactor = Result.OppositeLane;
}
