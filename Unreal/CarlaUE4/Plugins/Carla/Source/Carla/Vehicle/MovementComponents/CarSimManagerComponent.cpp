// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
// Copyright (c) 2019 Intel Corporation
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CarSimManagerComponent.h"
#include "Carla/Game/CarlaEngine.h"
#include "Carla/Vehicle/CarlaWheeledVehicle.h"
#include "Carla/Util/EmptyActor.h"

#ifdef WITH_CARSIM
#include "CarSimMovementComponent.h"
#include "VsVar.h"
#endif

void UCarSimManagerComponent::CreateCarsimComponent(
      ACarlaWheeledVehicle* Vehicle, FString Simfile, int ForceFrames)
{
  #ifdef WITH_CARSIM
  UCarSimManagerComponent* CarSimManagerComponent = NewObject<UCarSimManagerComponent>(Vehicle);
  CarSimManagerComponent->SimfilePath = Simfile;
  CarSimManagerComponent->FramesApplyingForce = ForceFrames;
  CarSimManagerComponent->RegisterComponent();
  Vehicle->SetCarlaMovementComponent(CarSimManagerComponent);
  #else
  UE_LOG(LogCarla, Warning, TEXT("Error: CarSim plugin is not enabled") );
  #endif
}

void UCarSimManagerComponent::BeginPlay()
{
  Super::BeginPlay();
  #ifdef WITH_CARSIM

  if(!CarlaVehicle)
  {
    UE_LOG(LogCarla, Warning, TEXT("Error: Owner is not properly set for UCarSimManagerComponent") );
    return;
  }

  // workarround to compensate carsim coordinate origin offset
  FActorSpawnParameters SpawnParams;
  SpawnParams.SpawnCollisionHandlingOverride =
      ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  OffsetActor = GetWorld()->SpawnActor<AEmptyActor>(
      CarlaVehicle->GetActorLocation() + CarlaVehicle->GetActorForwardVector() * CarlaVehicle->CarSimOriginOffset,
      CarlaVehicle->GetActorRotation(),
      SpawnParams);
  CarSimMovementComponent = NewObject<UCarSimMovementComponent>(OffsetActor);

  // CarSimMovementComponent = NewObject<UCarSimMovementComponent>(this);
  // BaseMovementComponent = CarSimMovementComponent;
  carla::log_warning("Loading simfile:", carla::rpc::FromFString(SimfilePath));
  CarlaVehicle->GetVehicleMovementComponent()->SetComponentTickEnabled(false);
  CarlaVehicle->GetVehicleMovementComponent()->Deactivate();
  CarSimMovementComponent->DisableVehicle = false;
  CarSimMovementComponent->VsConfigFile = SimfilePath;
  CarSimMovementComponent->Activate();
  CarSimMovementComponent->RegisterComponent();

  CarSimMovementComponent->ResetVsVehicle(false);
  // set carsim position to actor's
  CarSimMovementComponent->SyncVsVehicleLocOri();
  CarSimMovementComponent->SetComponentTickEnabled(true);
  // set kinematic mode for root component and bones
  CarlaVehicle->GetMesh()->PhysicsTransformUpdateMode = EPhysicsTransformUpdateMode::ComponentTransformIsKinematic;
  auto * Bone = CarlaVehicle->GetMesh()->GetBodyInstance(NAME_None);
  if (Bone)
  {
    Bone->SetInstanceSimulatePhysics(false);
  }
  // set callbacks to react to collisions
  CarlaVehicle->OnActorHit.AddDynamic(this, &UCarSimManagerComponent::OnCarSimHit);
  CarlaVehicle->GetMesh()->OnComponentBeginOverlap.AddDynamic(this, &UCarSimManagerComponent::OnCarSimOverlap);
  CarlaVehicle->GetMesh()->OnComponentEndOverlap.AddDynamic(this, &UCarSimManagerComponent::OnCarSimEndOverlap);
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Overlap);

  // workaround to prevent carsim from interacting with its own car
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Overlap);

  // attach to actor with an offset
  CarlaVehicle->AttachToActor(OffsetActor, FAttachmentTransformRules::KeepWorldTransform);
  #else
  UE_LOG(LogCarla, Warning, TEXT("Error: CarSim plugin is not enabled") );
  return;
  #endif
}

void UCarSimManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
  #ifdef WITH_CARSIM

  // get current vehicle position
  const auto Trans = CarlaVehicle->GetActorTransform().GetTranslation();
  // get current velocity
  auto Root = Cast<UPrimitiveComponent>(CarlaVehicle->GetRootComponent());
  if (Root == nullptr) return;
  FVector Vel = Root->GetPhysicsLinearVelocity("None");
  FVector Dir = Trans + (Vel.GetSafeNormal() * 200.0f);
  DrawDebugLine(GetWorld(), Trans, Dir, FColor(255, 255, 255), true, 1);

  if (ResetForces != 0 && ResetForces <= FCarlaEngine::FrameCounter)
  {
    #undef GetObject
    TScriptInterface<IVsVar> FX, FY, FZ;
    FX = IVsConnectObject::Execute_GetVsVar(CarSimMovementComponent, "IMP_FX_RP_1", "", EVsVarDirection::Input);
    FY = IVsConnectObject::Execute_GetVsVar(CarSimMovementComponent, "IMP_FY_RP_1", "", EVsVarDirection::Input);
    FZ = IVsConnectObject::Execute_GetVsVar(CarSimMovementComponent, "IMP_FZ_RP_1", "", EVsVarDirection::Input);
    IVsVar::Execute_SetFloatValue(FX.GetObject(), 0.0f);
    IVsVar::Execute_SetFloatValue(FY.GetObject(), 0.0f);
    IVsVar::Execute_SetFloatValue(FZ.GetObject(), 0.0f);
    ResetForces = 0;
    UE_LOG(LogCarla, Warning, TEXT("Carsim: Reset forces") );
  }
  #endif
}

void UCarSimManagerComponent::ProcessControl(FVehicleControl &Control)
{
  #ifdef WITH_CARSIM
  CarSimMovementComponent->SetThrottleInput(Control.Throttle);
  CarSimMovementComponent->SetSteeringInput(Control.Steer);
  CarSimMovementComponent->SetBrakeInput(Control.Brake);
  if (Control.bHandBrake)
  {
    CarSimMovementComponent->SetBrakeInput(Control.Brake + 1.0);
  }
  Control.Gear = CarSimMovementComponent->GetCurrentGear();
  #endif
}

// UCarSimMovementComponent *UCarSimManagerComponent::GetCarsimMovementComponent()
// {
//   return CarSimMovementComponent;
// }

void UCarSimManagerComponent::OnCarSimHit(AActor *Actor,
    AActor *OtherActor,
    FVector NormalImpulse,
    const FHitResult &Hit)
{
  #ifdef WITH_CARSIM
  // handle collision forces here
  #endif
}


void UCarSimManagerComponent::OnCarSimOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult & SweepResult)
{
  if (OtherComp->GetCollisionResponseToChannel(
      ECollisionChannel::ECC_WorldDynamic) ==
      ECollisionResponse::ECR_Block)
  {
    #ifdef WITH_CARSIM
    // get current vehicle position
    const auto Trans = CarlaVehicle->GetActorTransform().GetTranslation();
    // get current velocity
    auto Root = Cast<UPrimitiveComponent>(CarlaVehicle->GetRootComponent());
    if (Root == nullptr)
      return;
    FVector Vel = Root->GetPhysicsLinearVelocity("None");
    // reflect the velocity vector with the normal vector
    FVector NewVel = Vel - (2 * (FVector::DotProduct(Vel, SweepResult.ImpactNormal)) * SweepResult.ImpactNormal);
    // get the mass of the vehicle
    FVector Impulse = NewVel * Root->GetMass(); // / FramesApplyingForce;
    // apply delta time
    Impulse *= GetWorld()->GetDeltaSeconds();

    // FVector Dir = Trans + (Impulse.GetSafeNormal() * 200.0f);
    FVector Dir = SweepResult.ImpactPoint + (SweepResult.ImpactNormal * 200.0f);
    DrawDebugLine(GetWorld(), SweepResult.ImpactPoint, Dir, FColor(255, 0, 0), true, 5, 255, 10);

    // send the force to Carsim
    #undef GetObject
    TScriptInterface<IVsVar> FX, FY, FZ;
    FX = IVsConnectObject::Execute_GetVsVar(CarSimMovementComponent, "IMP_FX_RP_1", "", EVsVarDirection::Input);
    FY = IVsConnectObject::Execute_GetVsVar(CarSimMovementComponent, "IMP_FY_RP_1", "", EVsVarDirection::Input);
    FZ = IVsConnectObject::Execute_GetVsVar(CarSimMovementComponent, "IMP_FZ_RP_1", "", EVsVarDirection::Input);
    IVsVar::Execute_SetFloatValue(FX.GetObject(), Impulse.X);
    IVsVar::Execute_SetFloatValue(FY.GetObject(), Impulse.Y);
    IVsVar::Execute_SetFloatValue(FZ.GetObject(), 0.0f);
    // mark to reset this force next tick
    ResetForces = FCarlaEngine::FrameCounter + FramesApplyingForce;
    UE_LOG(LogCarla, Warning, TEXT("Carsim: Applied force: %f (%f, %f, %f) %f"), Root->GetMass(), Impulse.X, Impulse.Y, Impulse.Z, GetWorld()->GetDeltaSeconds());
    UE_LOG(LogCarla, Warning, TEXT("Carsim:              : (%f, %f, %f)"), SweepResult.Normal.X, SweepResult.Normal.Y, SweepResult.Normal.Z);
    UE_LOG(LogCarla, Warning, TEXT("Carsim:              : (%f, %f, %f)"), SweepResult.ImpactNormal.X, SweepResult.ImpactNormal.Y, SweepResult.ImpactNormal.Z);
    #endif
  }
}

void UCarSimManagerComponent::OnCarSimEndOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
  // if (OtherComp->GetCollisionResponseToChannel(
  //     ECollisionChannel::ECC_WorldDynamic) ==
  //     ECollisionResponse::ECR_Block)
  // {
  //   #ifdef WITH_CARSIM
  //   // if (ResetForces != 0 && ResetForces <= FCarlaEngine::FrameCounter)
  //   // {
  //   #undef GetObject
  //   TScriptInterface<IVsVar> FX, FY, FZ;
  //   FX = IVsConnectObject::Execute_GetVsVar(CarSimMovementComponent, "IMP_FX_RP_1", "", EVsVarDirection::Input);
  //   FY = IVsConnectObject::Execute_GetVsVar(CarSimMovementComponent, "IMP_FY_RP_1", "", EVsVarDirection::Input);
  //   FZ = IVsConnectObject::Execute_GetVsVar(CarSimMovementComponent, "IMP_FZ_RP_1", "", EVsVarDirection::Input);
  //   IVsVar::Execute_SetFloatValue(FX.GetObject(), 0.0f);
  //   IVsVar::Execute_SetFloatValue(FY.GetObject(), 0.0f);
  //   IVsVar::Execute_SetFloatValue(FZ.GetObject(), 0.0f);
  //   // ResetForces = 0;
  //   UE_LOG(LogCarla, Warning, TEXT("Carsim: Reset forces") );
  //   // }
  //   #endif
  // }  
}

void UCarSimManagerComponent::UseCarSimRoad(bool bEnabled)
{
  #ifdef WITH_CARSIM
  CarSimMovementComponent->UseVehicleSimRoad = bEnabled;
  CarSimMovementComponent->ResetVsVehicle(false);
  CarSimMovementComponent->SyncVsVehicleLocOri();
  #endif
}


void UCarSimManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  #ifdef WITH_CARSIM
  if (OffsetActor)
  {
    OffsetActor->Destroy();
  }
  #endif
}

FVector UCarSimManagerComponent::GetVelocity() const
{
  #ifdef WITH_CARSIM
  return GetOwner()->GetActorForwardVector() * CarSimMovementComponent->GetForwardSpeed();
  #else
  return FVector();
  #endif
}

int32 UCarSimManagerComponent::GetVehicleCurrentGear() const
{
  #ifdef WITH_CARSIM
  return CarSimMovementComponent->GetCurrentGear();
  #else
  return 0;
  #endif
}

float UCarSimManagerComponent::GetVehicleForwardSpeed() const
{
  #ifdef WITH_CARSIM
  return CarSimMovementComponent->GetForwardSpeed();
  #else
  return 0.0;
  #endif
}
