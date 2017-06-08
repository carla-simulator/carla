// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

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
  Super(ObjectInitializer),
  MovementComponent(nullptr)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;
}

ACarlaVehicleController::~ACarlaVehicleController() {}

// =============================================================================
// -- APlayerController --------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::SetupInputComponent()
{
  Super::SetupInputComponent();

  if (InputComponent != nullptr) {
    InputComponent->BindAction("ToggleManualMode", IE_Pressed, this, &ACarlaVehicleController::ToggleManualMode);
  }
}

void ACarlaVehicleController::Possess(APawn *aPawn)
{
  Super::Possess(aPawn);

  if (IsPossessingAVehicle()) {
    UE_LOG(LogCarla, Error, TEXT("Controller already possessing a pawn!"));
    return;
  }
  auto *WheeledVehicle = Cast<AWheeledVehicle>(aPawn);
  if (WheeledVehicle != nullptr) {
    // Bind hit events.
    aPawn->OnActorHit.AddDynamic(this, &ACarlaVehicleController::OnCollisionEvent);
    // Get vehicle movement component.
    MovementComponent = WheeledVehicle->GetVehicleMovementComponent();
    check(MovementComponent != nullptr);
    MovementComponent->bReverseAsBrake = false;
    // Get vehicle box component.
    TArray<UBoxComponent *> BoundingBoxes;
    WheeledVehicle->GetComponents<UBoxComponent>(BoundingBoxes);
    if (BoundingBoxes.Num() > 0) {
      VehicleBounds = BoundingBoxes[0];
    } else {
      UE_LOG(LogCarla, Error, TEXT("Pawn is missing the bounding box!"));
    }
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

  TActorIterator<ACityMapGenerator> It(GetWorld());
  if (It) {
    RoadMap = It->GetRoadMap();
  }
}

void ACarlaVehicleController::Tick(float DeltaTime)
{
  Super::PlayerTick(DeltaTime);

  if (IsPossessingAVehicle()) {
    CarlaPlayerState->UpdateTimeStamp(DeltaTime);
    CarlaPlayerState->Location = GetVehicleLocation();
    const FVector PreviousSpeed = CarlaPlayerState->ForwardSpeed * CarlaPlayerState->Orientation;
    CarlaPlayerState->Orientation = GetVehicleOrientation();
    CarlaPlayerState->ForwardSpeed = GetVehicleForwardSpeed();
    const FVector CurrentSpeed = CarlaPlayerState->ForwardSpeed * CarlaPlayerState->Orientation;
    CarlaPlayerState->Acceleration = (CurrentSpeed - PreviousSpeed) / DeltaTime;
    CarlaPlayerState->CurrentGear = GetVehicleCurrentGear();
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
// -- Vehicle pawn info --------------------------------------------------------
// =============================================================================

FVector ACarlaVehicleController::GetVehicleLocation() const
{
  check(GetPawn() != nullptr);
  return GetPawn()->GetActorLocation();
}

float ACarlaVehicleController::GetVehicleForwardSpeed() const
{
  check(MovementComponent != nullptr);
  return MovementComponent->GetForwardSpeed() * 0.036f;
}

FVector ACarlaVehicleController::GetVehicleOrientation() const
{
  check(GetPawn() != nullptr);
  return GetPawn()->GetTransform().GetRotation().GetForwardVector();
}

int32 ACarlaVehicleController::GetVehicleCurrentGear() const
{
  check(MovementComponent != nullptr);
  return MovementComponent->GetCurrentGear();
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
// -- Vehicle movement methods -------------------------------------------------
// =============================================================================

void ACarlaVehicleController::SetThrottleInput(float Value)
{
  check(MovementComponent != nullptr);
  MovementComponent->SetThrottleInput(Value);
}

void ACarlaVehicleController::SetSteeringInput(float Value)
{
  check(MovementComponent != nullptr);
  MovementComponent->SetSteeringInput(Value);
}

void ACarlaVehicleController::SetBrakeInput(float Value)
{
  check(MovementComponent != nullptr);
  MovementComponent->SetBrakeInput(Value);
}

void ACarlaVehicleController::SetReverse(bool Value)
{
  if (Value != bIsInReverse) {
    check(MovementComponent != nullptr);
    bIsInReverse = Value;
    MovementComponent->SetUseAutoGears(!bIsInReverse);
    MovementComponent->SetTargetGear(bIsInReverse ? -1 : 1, true);
  }
}

void ACarlaVehicleController::SetHandbrakeInput(bool Value)
{
  check(MovementComponent != nullptr);
  MovementComponent->SetHandbrakeInput(Value);
}

// =============================================================================
// -- Manual mode --------------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::SetManualMode(bool On)
{
  if (On != bManualMode) {
    bManualMode = On;
    SetupControllerInput();
  }
}

void ACarlaVehicleController::ToggleManualMode()
{
  SetManualMode(!bManualMode);
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
  CarlaPlayerState->RegisterCollision(Actor, OtherActor, NormalImpulse, Hit);
}

// =============================================================================
// -- Input bindings -----------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::SetupControllerInput()
{
  check(InputComponent != nullptr);
  // Vehicle movement.
  if (IsInManualMode()) {
    InputComponent->BindAxis("MoveForward", this, &ACarlaVehicleController::SetThrottleInput);
    InputComponent->BindAxis("MoveRight", this, &ACarlaVehicleController::SetSteeringInput);
    InputComponent->BindAxis("Brake", this, &ACarlaVehicleController::SetBrakeInput);
    InputComponent->BindAction("ToggleReverse", IE_Pressed, this, &ACarlaVehicleController::ToggleReverse);
    InputComponent->BindAction("Handbrake", IE_Pressed, this, &ACarlaVehicleController::HoldHandbrake);
    InputComponent->BindAction("Handbrake", IE_Released, this, &ACarlaVehicleController::ReleaseHandbrake);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Not implemented")); /// @todo
  }
}

// =============================================================================
// -- Other --------------------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::IntersectPlayerWithRoadMap()
{
  if (RoadMap == nullptr) {
    UE_LOG(LogCarla, Error, TEXT("Controller doesn't have a road map"));
    return;
  } else if (VehicleBounds == nullptr) {
    UE_LOG(LogCarla, Error, TEXT("Vehicle doesn't have a bounding box"));
    return;
  }

  constexpr float ChecksPerCentimeter = 0.1f;
  auto Result = RoadMap->Intersect(
      GetPawn()->GetTransform(),
      VehicleBounds->GetScaledBoxExtent(),
      ChecksPerCentimeter);

  CarlaPlayerState->OffRoadIntersectionFactor = Result.OffRoad;
  CarlaPlayerState->OtherLaneIntersectionFactor = Result.OppositeLane;
}
