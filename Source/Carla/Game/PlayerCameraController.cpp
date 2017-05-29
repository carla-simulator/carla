// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "PlayerCameraController.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

APlayerCameraController::APlayerCameraController(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  bAutoManageActiveCameraTarget = false;

  // Create the spring arm component.
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
}

APlayerCameraController::~APlayerCameraController() {}

// =============================================================================
// -- APlayerController --------------------------------------------------------
// =============================================================================

void APlayerCameraController::SetupInputComponent()
{
  Super::SetupInputComponent();
  if (InputComponent != nullptr) {
    InputComponent->BindAxis("CameraZoom", this, &APlayerCameraController::ChangeCameraZoom);
    InputComponent->BindAxis("CameraUp", this, &APlayerCameraController::ChangeCameraUp);
    InputComponent->BindAxis("CameraRight", this, &APlayerCameraController::ChangeCameraRight);
    InputComponent->BindAction("RestartLevel", IE_Pressed, this, &APlayerCameraController::RestartLevel);
  }
}

void APlayerCameraController::Possess(APawn *aPawn)
{
  Super::Possess(aPawn);
  SpringArm->AttachToComponent(
      aPawn->GetRootComponent(),
      FAttachmentTransformRules::KeepRelativeTransform);
}

void APlayerCameraController::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
  PlayerCamera->GetCameraView(DeltaTime, OutResult);
}

// =============================================================================
// -- Camera movement methods --------------------------------------------------
// =============================================================================

void APlayerCameraController::APlayerCameraController::ChangeCameraZoom(float Value)
{
  SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength + Value, 200.0f, 1e4f);
}

void APlayerCameraController::ChangeCameraUp(float Value)
{
  auto Rotation = SpringArm->GetRelativeTransform().Rotator();
  Rotation.Pitch = FMath::Clamp(Rotation.Pitch - Value, -80.0f, 0.0f);
  SpringArm->SetRelativeRotation(Rotation);
}

void APlayerCameraController::ChangeCameraRight(float Value)
{
  auto Rotation = SpringArm->GetRelativeTransform().Rotator();
  Rotation.Yaw -= Value;
  SpringArm->SetRelativeRotation(Rotation);
}
