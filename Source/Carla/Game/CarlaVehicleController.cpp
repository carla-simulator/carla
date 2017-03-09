// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaVehicleController.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "WheeledVehicle.h"
#include "WheeledVehicleMovementComponent.h"

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

ACarlaVehicleController::ACarlaVehicleController()
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

  // Do not collide, may clip into level.
  SpringArm->bDoCollisionTest = false;

  // Create the camera component
  PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
  PlayerCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
  PlayerCamera->bUsePawnControlRotation = false;
  PlayerCamera->FieldOfView = 90.f;
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
  auto *WheeledVehicle = Cast<AWheeledVehicle>(aPawn);
  if (WheeledVehicle != nullptr) {
    SpringArm->AttachToComponent(
       aPawn->GetRootComponent(),
       FAttachmentTransformRules::KeepRelativeTransform);
    MovementComponent = WheeledVehicle->GetVehicleMovementComponent();
    check(MovementComponent != nullptr);
  }
}

void ACarlaVehicleController::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
  PlayerCamera->GetCameraView(DeltaTime, OutResult);
}

// =============================================================================
// -- Car movement methods -----------------------------------------------------
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
