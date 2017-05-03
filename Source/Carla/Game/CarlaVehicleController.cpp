// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaVehicleController.h"

#include "SceneCaptureCamera.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "WheeledVehicle.h"
#include "WheeledVehicleMovementComponent.h"

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

ACarlaVehicleController::ACarlaVehicleController() :
  Super(),
  MovementComponent(nullptr)
{
  bAutoManageActiveCameraTarget = false;

  // Create the spring arm component
  SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
  SpringArm->TargetOffset = FVector(0.f, 0.f, 200.f);
  SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
  SpringArm->SetupAttachment(RootComponent);
  SpringArm->TargetArmLength = 600.0f;
  SpringArm->bEnableCameraRotationLag = true;
  SpringArm->CameraRotationLagSpeed = 7.f;
  SpringArm->bInheritPitch = false;
  SpringArm->bInheritRoll = false;
  SpringArm->bInheritYaw = true;

  // Do not collide, may clip into level.
  SpringArm->bDoCollisionTest = false;

  // Create the camera component
  PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
  PlayerCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
  PlayerCamera->bUsePawnControlRotation = false;
  PlayerCamera->FieldOfView = 90.f;

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
  check(InputComponent != nullptr);
  // Camera.
  InputComponent->BindAxis("CameraZoom", this, &ACarlaVehicleController::ChangeCameraZoom);
  InputComponent->BindAxis("CameraUp", this, &ACarlaVehicleController::ChangeCameraUp);
  InputComponent->BindAxis("CameraRight", this, &ACarlaVehicleController::ChangeCameraRight);
  // Global options.
  InputComponent->BindAction("RestartLevel", IE_Pressed, this, &ACarlaVehicleController::RestartLevel);
  InputComponent->BindAction("ToggleManualMode", IE_Pressed, this, &ACarlaVehicleController::ToggleManualMode);
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
    // Attach the camera's spring arm to the pawn.
    SpringArm->AttachToComponent(
       aPawn->GetRootComponent(),
       FAttachmentTransformRules::KeepRelativeTransform);
    // Bind hit events.
    aPawn->OnActorHit.AddDynamic(this, &ACarlaVehicleController::OnCollisionEvent);
    // Get vehicle movement component.
    MovementComponent = WheeledVehicle->GetVehicleMovementComponent();
    check(MovementComponent != nullptr);
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
  check(CarlaPlayerState != nullptr);
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

void ACarlaVehicleController::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
  PlayerCamera->GetCameraView(DeltaTime, OutResult);
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

void ACarlaVehicleController::AddSceneCaptureCamera(const FCameraDescription &Description)
{
  auto Camera = GetWorld()->SpawnActor<ASceneCaptureCamera>(Description.Position, Description.Rotation);
  Camera->Set(Description);
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
    AActor* /*Actor*/,
    AActor* OtherActor,
    FVector NormalImpulse,
    const FHitResult& /*Hit*/)
{
  CarlaPlayerState->RegisterCollision(OtherActor, NormalImpulse);
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
    InputComponent->BindAction("Handbrake", IE_Pressed, this, &ACarlaVehicleController::HoldHandbrake);
    InputComponent->BindAction("Handbrake", IE_Released, this, &ACarlaVehicleController::ReleaseHandbrake);
  } else {
    UE_LOG(LogCarla, Error, TEXT("Not implemented")); /// @todo
  }
}

// =============================================================================
// -- Camera movement methods --------------------------------------------------
// =============================================================================

void ACarlaVehicleController::ACarlaVehicleController::ChangeCameraZoom(float Value)
{
  SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength + Value, 200.0f, 1e4f);
}

void ACarlaVehicleController::ChangeCameraUp(float Value)
{
  auto Rotation = SpringArm->GetRelativeTransform().Rotator();
  Rotation.Pitch = FMath::Clamp(Rotation.Pitch - Value, -80.0f, 0.0f);
  SpringArm->SetRelativeRotation(Rotation);
}

void ACarlaVehicleController::ChangeCameraRight(float Value)
{
  auto Rotation = SpringArm->GetRelativeTransform().Rotator();
  Rotation.Yaw -= Value;
  SpringArm->SetRelativeRotation(Rotation);
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
