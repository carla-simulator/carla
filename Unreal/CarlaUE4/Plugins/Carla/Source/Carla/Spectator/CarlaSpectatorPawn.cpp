// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Engine/Engine.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "Game/CarlaGameInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "CarlaSpectatorPawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "UObjectIterator.h"
#ifdef CARLA_DEBUG_SPECTATOR
#include "DrawDebugHelpers.h"
#endif

constexpr float TIME_BETWEEN_LOOKS_FOR_TARGETS = 0.1f;

ACarlaSpectatorPawn::ACarlaSpectatorPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  if(InputComponent)
     InputComponent->bBlockInput = false;
  RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
  /*CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
  CameraSpringArm->SetupAttachment(RootComponent);
  CameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-60.0f, 0.0f, 0.0f));
  CameraSpringArm->TargetArmLength = 400.f;
  CameraSpringArm->bEnableCameraLag = true;
  CameraSpringArm->CameraLagSpeed = 3.0f;
  Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
  Camera->SetupAttachment(CameraSpringArm); //USpringArmComponent::SocketName
  
  //Take control of the default Player
  //AutoPossessPlayer = EAutoReceiveInput::Player0;*/
}

bool ACarlaSpectatorPawn::TraceSphere(const FVector& Start, const FVector& End, const float Radius, FHitResult& HitOut, ECollisionChannel TraceChannel) const
{
  FCollisionQueryParams TraceParams(FName(TEXT("SphereTracePawn")), true, this);
  TraceParams.bTraceComplex = true;
  //TraceParams.bTraceAsyncScene = true;
  TraceParams.bReturnPhysicalMaterial = false;
  
  //Ignore Actors
  TraceParams.AddIgnoredActor(this);
  
  //Re-initialize hit info
  HitOut = FHitResult(ForceInit);
  
  const bool result = GetWorld()->SweepSingleByChannel(HitOut,Start,End, FQuat(), TraceChannel,FCollisionShape::MakeSphere(Radius), TraceParams);
  #if WITH_EDITOR && defined(CARLA_DEBUG_SPECTATOR)
    DrawDebugSphere(GetWorld(),HitOut.Location,Radius, 32, FColor::Cyan);
    DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, -1, 0, 12.333);
  #endif
  return result;
}

void ACarlaSpectatorPawn::DetectForwardVehiclePawns()
{
  if (!Camera) return;
  FHitResult hit;
  if(TraceSphere(Camera->GetComponentLocation(),Camera->GetComponentLocation() + Camera->GetForwardVector()*DistanceToDetectPawns,RadiusToDetectPawns,hit))
  {
    AActor *actor = hit.Actor.Get();
    if(IsValid(actor) && !actor->IsPendingKill() && actor!=Cast<AActor>(LookedTarget))
    {
      LookedTarget = Cast<APawn>(actor);
    }
  }
}

void ACarlaSpectatorPawn::BeginPlay()
{
  Super::BeginPlay();
  #ifdef CARLA_DEBUG_SPECTATOR
    GetWorld()->DebugDrawTraceTag = FName(TEXT("SphereTracePawn"));
  #endif
}

void ACarlaSpectatorPawn::TickFollowingMode(float DeltaTime)
{
  if(!IsValid(FollowedTarget)||FollowedTarget->IsPendingKill())
  {
    UE_LOG(LogCarla, Warning, TEXT("CarlaSpectator is not following any target, switching to manual control"))
    SetManualControl();
    return;
  }
  const FVector TargetLocation = FollowedTarget->GetActorLocation();
  SetActorLocation(TargetLocation - FollowedTarget->GetActorForwardVector().Normalize() * 15.0f);
  //const FVector DestinyLocation = TargetLocation + FollowedTarget->GetActorForwardVector() * ;
  if (CameraSpringArm)
  {
    FRotator PitchRotation = CameraSpringArm->GetComponentRotation();
    PitchRotation.Pitch = FMath::Clamp(PitchRotation.Pitch + CameraInput.Y, -110.0f, 115.0f);
    CameraSpringArm->SetWorldRotation(PitchRotation);
  }
}


void ACarlaSpectatorPawn::TickManualMode(float DeltaTime)
{
  //Rotate our camera's pitch, but limit it so we're always looking downward
    if (!CameraSpringArm) return;
  FRotator PitchRotation = CameraSpringArm->GetComponentRotation();
  PitchRotation.Pitch = PitchRotation.Pitch + CameraInput.Y;// FMath::Clamp(PitchRotation.Pitch + CameraInput.Y, -80.0f, -15.0f);
  CameraSpringArm->SetWorldRotation(PitchRotation);
  
  //Rotate our actor's yaw, which will turn our camera because we're attached to it
  FRotator YawRotation = GetActorRotation();
  YawRotation.Yaw += CameraInput.X;
  SetActorRotation(YawRotation);
  
  FVector NewLocation = GetActorLocation();
  
  if (!MovementInput.IsZero())
  {
    //Scale our movement input axis values by 100 units per second
    MovementInput = MovementInput.GetSafeNormal() * 100.0f;
    
    NewLocation += GetActorForwardVector() * MovementInput.X * DeltaTime * Speed;
    NewLocation += GetActorRightVector() * MovementInput.Y * DeltaTime * Speed;
    SetActorLocation(NewLocation);
  }

  //Check for a target in front of this spectator pawn
  TimeSinceLookForTargets += DeltaTime;
  if(TimeSinceLookForTargets>=TIME_BETWEEN_LOOKS_FOR_TARGETS)
  {
    TimeSinceLookForTargets = 0.0f;
    DetectForwardVehiclePawns();
  }
}


void ACarlaSpectatorPawn::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);
  switch(ControlMode)
  {
    case ESpectatorControlMode::FOLLOW_CONTROL:
    {
      TickFollowingMode(DeltaSeconds);
      break;
    }
    case ESpectatorControlMode::MANUAL_CONTROL:
    {
      TickManualMode(DeltaSeconds);
      break;
    }
  }
}

void ACarlaSpectatorPawn::SetupPlayerInputComponent(UInputComponent* InputComponent) 
{
  check(InputComponent);
  InputComponent->BindAxis("MoveForward", this, &ACarlaSpectatorPawn::MoveForward);
  InputComponent->BindAxis("MoveRight", this, &ACarlaSpectatorPawn::MoveRight);
  InputComponent->BindAction("RestartLevel", IE_Pressed, this, &ACarlaSpectatorPawn::RestartLevel);
  InputComponent->BindAction("Interact", IE_Pressed, this, &ACarlaSpectatorPawn::PossessLookedTarget);
  InputComponent->BindAction("UseTheForce", IE_Pressed, this, &ACarlaSpectatorPawn::FollowLookedTarget);
  InputComponent->BindAxis("CameraPitch", this, &ACarlaSpectatorPawn::PitchCamera);
  InputComponent->BindAxis("CameraYaw", this, &ACarlaSpectatorPawn::YawCamera);
}

void ACarlaSpectatorPawn::MoveForward(float AxisValue)
{
  MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void ACarlaSpectatorPawn::MoveRight(float AxisValue)
{
  MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
  
}

void ACarlaSpectatorPawn::PitchCamera(float AxisValue)
{
  CameraInput.Y = AxisValue;
}

void ACarlaSpectatorPawn::YawCamera(float AxisValue)
{
  CameraInput.X = AxisValue;
}

void ACarlaSpectatorPawn::RestartLevel()
{
  UCarlaGameInstance *cgi = Cast<UCarlaGameInstance>(GetWorld()->GetGameInstance());
  if(cgi)
  {
    cgi->GetDataRouter().RestartLevel();
  }
}

void ACarlaSpectatorPawn::PossessLookedTarget()
{
  if(IsValid(LookedTarget)&&!LookedTarget->IsPendingKillPending())
  {
    APlayerController* playercontroller = GEngine->GetFirstLocalPlayerController(GetWorld());
    if(playercontroller->GetPawn()!=LookedTarget)
    {
      //possess the looked target, we ask for permission to the data router to do this
      if(LookedTarget->IsA<ACarlaWheeledVehicle>())
      {
        UCarlaGameInstance *carla = Cast<UCarlaGameInstance>(GetWorld()->GetGameInstance());
        if (carla)
        {
          if(carla->GetDataRouter().PlayerControlVehicle(playercontroller, LookedTarget))
          {
            Destroy();
          }
        }
      }

    }
  }
}

void ACarlaSpectatorPawn::FollowLookedTarget()
{
  if(IsValid(LookedTarget)&&!LookedTarget->IsPendingKillPending())
  {
    SetFollowTarget(LookedTarget);
  }
}


void ACarlaSpectatorPawn::SetFollowTarget(APawn* Pawn)
{
  if(IsValid(Pawn) && !Pawn->IsPendingKill())
  {
    ControlMode = ESpectatorControlMode::FOLLOW_CONTROL;
    FollowedTarget = Pawn;
    /*if(CameraSpringArm)
    {
      CameraSpringArm->SocketOffset = FVector(0.0, 30.f, 0.0);
      CameraSpringArm->TargetArmLength = 500.0f;
      CameraSpringArm->TargetOffset = FVector(170.0f,-8.f, 375.0f);
    }*/
  }
}

void ACarlaSpectatorPawn::SetManualControl()
{
  ControlMode = ESpectatorControlMode::MANUAL_CONTROL;
}
