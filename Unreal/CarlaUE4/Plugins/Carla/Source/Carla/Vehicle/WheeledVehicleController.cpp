// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "WheeledVehicleController.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "CarlaWheeledVehicle.h"
#include "Spectator/CarlaSpectatorPawn.h"
#include "Game/CarlaGameInstance.h"
#include "Engine/World.h"
#include "Util/RandomEngine.h"
#include "GameFramework/PlayerState.h"

// =============================================================================
// -- Constructor and destructor -----------------------------------------------
// =============================================================================

AWheeledVehicleController::AWheeledVehicleController(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer)
{
  bAutoManageActiveCameraTarget = false;

  // Create the spring arm component.
  SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
  SpringArm->SetupAttachment(RootComponent);
  SpringArm->bEnableCameraRotationLag = true;
  SpringArm->CameraRotationLagSpeed = 7.f;
  SpringArm->TargetArmLength = 650.0f;
  SpringArm->bInheritPitch = false;
  SpringArm->bInheritRoll = false;
  SpringArm->bInheritYaw = true;
  InitialSpringArmPosition = SpringArm->RelativeLocation;
  InitialSpringArmRotation = SpringArm->RelativeRotation;
  // Do not collide, may clip into level.
  SpringArm->bDoCollisionTest = false;

  // Create the camera component.
  PlayerCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
  PlayerCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
  PlayerCamera->bUsePawnControlRotation = false;
  PlayerCamera->FieldOfView = 90.f;

  // Create the on-board camera component.
  OnBoardCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("OnBoardCamera0"));
  OnBoardCamera->SetupAttachment(RootComponent);
  OnBoardCamera->bUsePawnControlRotation = false;
  OnBoardCamera->FieldOfView = 100.f;

  ResetCameras();
}

AWheeledVehicleController::~AWheeledVehicleController() {}

// =============================================================================
// -- AActor -------------------------------------------------------------------
// =============================================================================

void AWheeledVehicleController::BeginPlay()
{
  Super::BeginPlay();
  EnableOnBoardCamera(bOnBoardCameraIsActive, true);
}

// =============================================================================
// -- APlayerController --------------------------------------------------------
// =============================================================================

void AWheeledVehicleController::SetupInputComponent()
{
  Super::SetupInputComponent();
  if (InputComponent != nullptr) 
  {
    // Camera movement.
    InputComponent->BindAxis("CameraZoom", this, &AWheeledVehicleController::ChangeCameraZoom);
    InputComponent->BindAxis("CameraUp", this, &AWheeledVehicleController::ChangeCameraUp);
    InputComponent->BindAxis("CameraRight", this, &AWheeledVehicleController::ChangeCameraRight);
    InputComponent->BindAction("ToggleCamera", IE_Pressed, this, &AWheeledVehicleController::ToggleCamera);
    InputComponent->BindAction("RestartLevel", IE_Pressed, this, &AWheeledVehicleController::RestartLevel); // .bConsumeInput = false;

    // Vehicle or spectator movement.
    InputComponent->BindAction("ToggleAutopilot", IE_Pressed, this, &AWheeledVehicleAIController::ToggleAutopilot);
    InputComponent->BindAxis("MoveForward", this, &AWheeledVehicleController::SetThrottleInput).bConsumeInput = false;
    InputComponent->BindAxis("MoveRight", this, &AWheeledVehicleController::SetSteeringInput).bConsumeInput = false;
    InputComponent->BindAxis("Brake", this, &AWheeledVehicleController::SetBrakeInput);
    InputComponent->BindAction("ToggleReverse", IE_Pressed, this, &AWheeledVehicleController::ToggleReverse);
    InputComponent->BindAction("Handbrake", IE_Pressed, this, &AWheeledVehicleController::HoldHandbrake);
    InputComponent->BindAction("Handbrake", IE_Released, this, &AWheeledVehicleController::ReleaseHandbrake);

    //Interaction
    InputComponent->BindAction("Interact", IE_Released, this, &AWheeledVehicleController::InteractButton); // .bConsumeInput = false;
    InputComponent->BindAction("UseTheForce", IE_Pressed, this, &AWheeledVehicleController::ForceButton); // .bConsumeInput = false;

    //Camera movement with mouse only for spectator mode
    InputComponent->BindAxis("CameraPitch", this, &AWheeledVehicleController::MousePitchCamera).bConsumeInput = false;
    InputComponent->BindAxis("CameraYaw", this, &AWheeledVehicleController::MouseYawCamera).bConsumeInput = false;
    
    InputComponent->bBlockInput = false;

  }
}

void AWheeledVehicleController::ResetCameras()
{
  SpringArm->TargetOffset = FVector(0.f, 0.f, 200.f);
  SpringArm->SetRelativeLocation(InitialSpringArmPosition); // FVector(-702.0f, 0.0f, 193.2f));
  SpringArm->SetRelativeRotation(InitialSpringArmRotation); // FRotator(-15.f, 0.f, 0.f));
  OnBoardCamera->SetRelativeLocation(FVector(140.f, 0.f, 140.f));
  OnBoardCamera->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
}

void AWheeledVehicleController::Possess(APawn* aPawn)
{
  if (!IsValid(aPawn)) return;
  Super::Possess(aPawn);
  SpringArm->AttachToComponent(aPawn->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
  OnBoardCamera->AttachToComponent(aPawn->GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);
  ResetCameras();
}

void AWheeledVehicleController::UnPossess()
{
  Super::UnPossess();
  APawn *pawn = GetPawn();
  if (!pawn) return;
  SpringArm->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
  OnBoardCamera->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform); 
}

void AWheeledVehicleController::StartSpectatingOnly()
{
    Super::StartSpectatingOnly();
}

void AWheeledVehicleController::StopSpectatingOnly()
{
    ChangeState(NAME_Playing);
    PlayerState->bIsSpectator = false;
    PlayerState->bOnlySpectator = false;
    bPlayerIsWaiting = false;
    ResetCameras();
}


void AWheeledVehicleController::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
  if (bOnBoardCameraIsActive) 
  {
    OnBoardCamera->GetCameraView(DeltaTime, OutResult);
  } else {
    PlayerCamera->GetCameraView(DeltaTime, OutResult);
  }
}

// =============================================================================
// -- User input ---------------------------------------------------------------
// =============================================================================

void AWheeledVehicleController::EnableUserInput(const bool On)
{
  bAllowUserInput = On;
  if (bAllowUserInput) 
  {
    Super::SetAutopilot(false);
  }
}

// =============================================================================
// -- Camera movement methods --------------------------------------------------
// =============================================================================

void AWheeledVehicleController::AWheeledVehicleController::ChangeCameraZoom(float Value)
{
  SpringArm->TargetArmLength = FMath::Clamp(SpringArm->TargetArmLength + Value, 150.0f, 2e4f);
}

void AWheeledVehicleController::ChangeCameraUp(float Value)
{
  auto Rotation = SpringArm->GetRelativeTransform().Rotator();
  Rotation.Pitch = FMath::Clamp(Rotation.Pitch - Value, -85.0f, 0.0f);
  SpringArm->SetRelativeRotation(Rotation);
}

void AWheeledVehicleController::ChangeCameraRight(float Value)
{
  auto Rotation = SpringArm->GetRelativeTransform().Rotator();
  Rotation.Yaw -= Value;
  SpringArm->SetRelativeRotation(Rotation);
}

void AWheeledVehicleController::EnableOnBoardCamera(const bool bEnable, const bool bForce)
{
  if (bForce || (bOnBoardCameraIsActive != bEnable)) 
  {
    bOnBoardCameraIsActive = bEnable;
    if (bEnable) 
    {
      PlayerCamera->Deactivate();
      OnBoardCamera->Activate();
    } else {
      OnBoardCamera->Deactivate();
      PlayerCamera->Activate();
    }
  }
}

// =============================================================================
// -- Vehicle movement methods -------------------------------------------------
// =============================================================================

void AWheeledVehicleController::SetThrottleInput(const float Value)
{
  if (bAllowUserInput && !IsAutopilotEnabled())
  {
    const bool bIsVehicle = IsPossessingAVehicle();
    if(bIsVehicle)
    {
      GetPossessedVehicle()->SetThrottleInput(Value);
    }
    else 
    {
      ACarlaSpectatorPawn* carlaspectator = Cast<ACarlaSpectatorPawn>(GetPawn());
      if(IsValid(carlaspectator))
      {
        carlaspectator->MoveForward(Value);
      }
    }
  }
}

void AWheeledVehicleController::SetSteeringInput(const float Value)
{
  if (bAllowUserInput && !IsAutopilotEnabled()) 
  {
    ACarlaSpectatorPawn* carlaspectator = Cast<ACarlaSpectatorPawn>(GetPawn());
    if (IsValid(carlaspectator))
    {
      carlaspectator->MoveRight(Value);
      return;
    }
    if(IsPossessingAVehicle())
    {
      GetPossessedVehicle()->SetSteeringInput(Value);
    } 
  }
}

void AWheeledVehicleController::SetBrakeInput(const float Value)
{
  if (bAllowUserInput && !IsAutopilotEnabled() && IsPossessingAVehicle()) 
  {
    GetPossessedVehicle()->SetBrakeInput(Value);
  }
}

void AWheeledVehicleController::ToggleReverse()
{
  if (bAllowUserInput && !IsAutopilotEnabled() && IsPossessingAVehicle()) 
  {
    GetPossessedVehicle()->ToggleReverse();
  }
}

void AWheeledVehicleController::HoldHandbrake()
{
  if (bAllowUserInput && !IsAutopilotEnabled() && IsPossessingAVehicle()) 
  {
    GetPossessedVehicle()->HoldHandbrake();
  }
}

void AWheeledVehicleController::ReleaseHandbrake()
{
  if (bAllowUserInput && !IsAutopilotEnabled() && IsPossessingAVehicle())
  {
    GetPossessedVehicle()->ReleaseHandbrake();
  }
}

void AWheeledVehicleController::InteractButton()
{
  if (!bAllowUserInput) return; 
  UCarlaGameInstance *carla = Cast<UCarlaGameInstance>(GetWorld()->GetGameInstance());
  if (carla)
  {
    ACarlaSpectatorPawn* carlaspectator = Cast<ACarlaSpectatorPawn>(GetSpectatorPawn());
    if (IsValid(carlaspectator))
    {
      APawn* lookedtarget = carlaspectator->GetLookedTarget();
      if (lookedtarget!=nullptr && carlaspectator != lookedtarget)
      {
        if (lookedtarget->IsA<ACarlaWheeledVehicle>() && carla->GetDataRouter().PlayerControlVehicle(this, lookedtarget))
        {
          StopSpectatingOnly();
          ResetCameras();
          carlaspectator->Destroy();
        }
      }
    }
    else
    {
      SetAutopilot(false);
      APawn *currentPawn = GetPawn();
      if(currentPawn)
      {
        if (currentPawn->GetController()) currentPawn->GetController()->UnPossess();
        currentPawn->AIControllerClass = AWheeledVehicleAIController::StaticClass();
        currentPawn->SpawnDefaultController();
        AWheeledVehicleAIController* controller = Cast<AWheeledVehicleAIController>(currentPawn->GetController());
        if (controller)
        {
          controller->GetRandomEngine()->Seed(GetRandomEngine()->GenerateSeed());
          controller->SetRoadMap(GetRoadMap());
          controller->SetAutopilot(true);
        }
      }
      StartSpectatingOnly();
      carlaspectator = Cast<ACarlaSpectatorPawn>(GetSpectatorPawn());
      if(!carlaspectator)
      {
        UE_LOG(LogCarla, Error, TEXT("Carla Spectator is not the default spectator class of the game mode!"));
        if (GetSpectatorPawn()) Possess(GetSpectatorPawn());
        return;
      }

      Possess(carlaspectator);
      carlaspectator->SetCameraSpring(SpringArm);
      carlaspectator->SetCamera(PlayerCamera);
      ResetCameras();
      
    }
  }
}

void AWheeledVehicleController::ForceButton()
{
  if (!bAllowUserInput) return;
  ACarlaSpectatorPawn* carlaspectator = Cast<ACarlaSpectatorPawn>(GetPawn());
  if (IsValid(carlaspectator))
  {
    carlaspectator->FollowLookedTarget();
  }
}

void AWheeledVehicleController::MousePitchCamera(float Value)
{
  if (!bAllowUserInput) return;
  ACarlaSpectatorPawn* carlaspectator = Cast<ACarlaSpectatorPawn>(GetPawn());
  if (IsValid(carlaspectator))
  {
      carlaspectator->PitchCamera(Value);
  }
}

void AWheeledVehicleController::MouseYawCamera(float Value)
{
  if (!bAllowUserInput) return;
  ACarlaSpectatorPawn* carlaspectator = Cast<ACarlaSpectatorPawn>(GetPawn());
  if (IsValid(carlaspectator))
  {
      carlaspectator->YawCamera(Value);
  }
}
