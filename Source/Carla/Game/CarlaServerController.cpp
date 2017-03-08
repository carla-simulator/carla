// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#include "Carla.h"
#include "CarlaServerController.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
// #include "CarlaPlayerCameraManager.h"

ACarlaServerController::ACarlaServerController()
{
  // PlayerCameraManagerClass = ACarlaPlayerCameraManager::StaticClass();
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

ACarlaServerController::~ACarlaServerController() {}

void ACarlaServerController::SetupInputComponent()
{
  Super::SetupInputComponent();
  check(InputComponent);
  InputComponent->BindAxis("CameraZoom", this, &ACarlaServerController::ChangeCameraZoom);
  InputComponent->BindAxis("CameraUp", this, &ACarlaServerController::ChangeCameraUp);
  InputComponent->BindAxis("CameraRight", this, &ACarlaServerController::ChangeCameraRight);
  InputComponent->BindAction("RestartLevel", IE_Pressed, this, &ACarlaServerController::RestartLevel);
}

void ACarlaServerController::Possess(APawn *aPawn)
{
  Super::Possess(aPawn);
  if (aPawn != nullptr) {
    SpringArm->AttachToComponent(
       aPawn->GetRootComponent(),
       FAttachmentTransformRules::KeepRelativeTransform);
  }
}

void ACarlaServerController::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
  PlayerCamera->GetCameraView(DeltaTime, OutResult);
}

void ACarlaServerController::ChangeCameraZoom(float Value)
{
  SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength + Value, 200.0f, 1e4f);
}

void ACarlaServerController::ChangeCameraUp(float Value)
{
  auto rotation = SpringArm->GetRelativeTransform().Rotator();
  rotation.Pitch = FMath::Clamp(rotation.Pitch - Value, -80.0f, 0.0f);
  SpringArm->SetRelativeRotation(rotation);
}

void ACarlaServerController::ChangeCameraRight(float Value)
{
  auto rotation = SpringArm->GetRelativeTransform().Rotator();
  rotation.Yaw -= Value;
  SpringArm->SetRelativeRotation(rotation);
}
