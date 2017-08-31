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
  Super(ObjectInitializer)
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
    UE_LOG(LogCarla, Error, TEXT("Controller already possessing a vehicle!"));
    return;
  }
  Vehicle = Cast<ACarlaWheeledVehicle>(aPawn);
  if (Vehicle != nullptr) {
    // Bind hit events.
    aPawn->OnActorHit.AddDynamic(this, &ACarlaVehicleController::OnCollisionEvent);
    // Get vehicle movement component.
    auto *MovementComponent = Vehicle->GetVehicleMovementComponent();
    check(MovementComponent != nullptr);
    MovementComponent->bReverseAsBrake = false;
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
    const FVector PreviousSpeed = CarlaPlayerState->ForwardSpeed * CarlaPlayerState->GetOrientation();
    CarlaPlayerState->Transform = Vehicle->GetActorTransform();
    CarlaPlayerState->ForwardSpeed = Vehicle->GetVehicleForwardSpeed();
    const FVector CurrentSpeed = CarlaPlayerState->ForwardSpeed * CarlaPlayerState->GetOrientation();
    CarlaPlayerState->Acceleration = (CurrentSpeed - PreviousSpeed) / DeltaTime;
    CarlaPlayerState->CurrentGear = Vehicle->GetVehicleCurrentGear();
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
  // Register collision only if we are moving faster than 1 km/h.
  if (FMath::Abs(Vehicle->GetVehicleForwardSpeed()) > 1.0f) {
    CarlaPlayerState->RegisterCollision(Actor, OtherActor, NormalImpulse, Hit);
  }
}

// =============================================================================
// -- Input bindings -----------------------------------------------------------
// =============================================================================

void ACarlaVehicleController::SetupControllerInput()
{
  check(InputComponent != nullptr);
  check(IsPossessingAVehicle());
  // Vehicle movement.
  if (IsInManualMode()) {
    InputComponent->BindAxis("MoveForward", Vehicle, &ACarlaWheeledVehicle::SetThrottleInput);
    InputComponent->BindAxis("MoveRight", Vehicle, &ACarlaWheeledVehicle::SetSteeringInput);
    InputComponent->BindAxis("Brake", Vehicle, &ACarlaWheeledVehicle::SetBrakeInput);
    InputComponent->BindAction("ToggleReverse", IE_Pressed, Vehicle, &ACarlaWheeledVehicle::ToggleReverse);
    InputComponent->BindAction("Handbrake", IE_Pressed, Vehicle, &ACarlaWheeledVehicle::HoldHandbrake);
    InputComponent->BindAction("Handbrake", IE_Released, Vehicle, &ACarlaWheeledVehicle::ReleaseHandbrake);
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
  }

  constexpr float ChecksPerCentimeter = 0.1f;
  auto Result = RoadMap->Intersect(
      Vehicle->GetActorTransform(),
      Vehicle->GetVehicleBoundsExtent(),
      ChecksPerCentimeter);

  CarlaPlayerState->OffRoadIntersectionFactor = Result.OffRoad;
  CarlaPlayerState->OtherLaneIntersectionFactor = Result.OppositeLane;
}
