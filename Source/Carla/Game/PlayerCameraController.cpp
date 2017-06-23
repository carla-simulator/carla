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
  SpringArm->TargetArmLength = 650.0f;
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

  // Create the on-board camera component
  OnBoardCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("OnBoardCamera0"));
  OnBoardCamera->SetRelativeLocation(FVector(140.f, 0.f, 140.f));
  OnBoardCamera->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
  OnBoardCamera->SetupAttachment(RootComponent);
  OnBoardCamera->bUsePawnControlRotation = false;
  OnBoardCamera->FieldOfView = 100.f;
}

APlayerCameraController::~APlayerCameraController() {}

// =============================================================================
// -- AActor -------------------------------------------------------------------
// =============================================================================

void APlayerCameraController::BeginPlay()
{
  Super::BeginPlay();
  EnableOnBoardCamera(bOnBoardCameraIsActive, true);
}

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
    InputComponent->BindAction("ToggleCamera", IE_Pressed, this, &APlayerCameraController::ToggleCamera);
    InputComponent->BindAction("RestartLevel", IE_Pressed, this, &APlayerCameraController::RestartLevel);
  }
}

void APlayerCameraController::Possess(APawn *aPawn)
{
  Super::Possess(aPawn);
  SpringArm->AttachToComponent(
      aPawn->GetRootComponent(),
      FAttachmentTransformRules::KeepRelativeTransform);
  OnBoardCamera->AttachToComponent(
      aPawn->GetRootComponent(),
      FAttachmentTransformRules::KeepRelativeTransform);
}

void APlayerCameraController::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
  if (bOnBoardCameraIsActive) {
    OnBoardCamera->GetCameraView(DeltaTime, OutResult);
  } else {
    PlayerCamera->GetCameraView(DeltaTime, OutResult);
  }
}

// =============================================================================
// -- Camera movement methods --------------------------------------------------
// =============================================================================

void APlayerCameraController::APlayerCameraController::ChangeCameraZoom(float Value)
{
  SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength + Value, 150.0f, 2e4f);
}

void APlayerCameraController::ChangeCameraUp(float Value)
{
  auto Rotation = SpringArm->GetRelativeTransform().Rotator();
  Rotation.Pitch = FMath::Clamp(Rotation.Pitch - Value, -85.0f, 0.0f);
  SpringArm->SetRelativeRotation(Rotation);
}

void APlayerCameraController::ChangeCameraRight(float Value)
{
  auto Rotation = SpringArm->GetRelativeTransform().Rotator();
  Rotation.Yaw -= Value;
  SpringArm->SetRelativeRotation(Rotation);
}

void APlayerCameraController::EnableOnBoardCamera(const bool bEnable, const bool bForce)
{
  if (bForce || (bOnBoardCameraIsActive != bEnable)) {
    bOnBoardCameraIsActive = bEnable;
    if (bEnable) {
      PlayerCamera->Deactivate();
      OnBoardCamera->Activate();
    } else {
      OnBoardCamera->Deactivate();
      PlayerCamera->Activate();
    }
  }
}
