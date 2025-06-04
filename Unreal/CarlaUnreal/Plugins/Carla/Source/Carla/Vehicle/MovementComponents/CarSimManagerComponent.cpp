// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
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
      ACarlaWheeledVehicle* Vehicle, FString Simfile)
{
  #ifdef WITH_CARSIM
  UCarSimManagerComponent* CarSimManagerComponent = NewObject<UCarSimManagerComponent>(Vehicle);
  CarSimManagerComponent->SimfilePath = Simfile;
  Vehicle->SetCarlaMovementComponent(CarSimManagerComponent);
  CarSimManagerComponent->RegisterComponent();
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

void UCarSimManagerComponent::OnCarSimHit(AActor *Actor,
    AActor *OtherActor,
    FVector NormalImpulse,
    const FHitResult &Hit)
{
  #ifdef WITH_CARSIM
  // finish Carsim simulation
  UDefaultMovementComponent::CreateDefaultMovementComponent(CarlaVehicle);
  CarlaVehicle->GetMesh()->SetPhysicsLinearVelocity(FVector(0,0,0), false, "Vehicle_Base");
  CarlaVehicle->GetVehicleMovementComponent()->SetComponentTickEnabled(true);
  CarlaVehicle->GetVehicleMovementComponent()->Activate();
  CarlaVehicle->GetMesh()->PhysicsTransformUpdateMode = EPhysicsTransformUpdateMode::SimulationUpatesComponentTransform;
  auto * Bone = CarlaVehicle->GetMesh()->GetBodyInstance(NAME_None);
  if (Bone)
  {
    Bone->SetInstanceSimulatePhysics(true);
  }
  else
  {
    carla::log_warning("No bone with name");
  }
  CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
  CarlaVehicle->GetMesh()->SetCollisionProfileName("Vehicle");    
  CarlaVehicle->RestoreVehiclePhysicsControl();
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
    // finish Carsim simulation
    UDefaultMovementComponent::CreateDefaultMovementComponent(CarlaVehicle);
    CarlaVehicle->GetMesh()->SetPhysicsLinearVelocity(FVector(0,0,0), false, "Vehicle_Base");
    CarlaVehicle->GetVehicleMovementComponent()->SetComponentTickEnabled(true);
    CarlaVehicle->GetVehicleMovementComponent()->Activate();
    CarlaVehicle->GetMesh()->PhysicsTransformUpdateMode = EPhysicsTransformUpdateMode::SimulationUpatesComponentTransform;
    auto * Bone = CarlaVehicle->GetMesh()->GetBodyInstance(NAME_None);
    if (Bone)
    {
      Bone->SetInstanceSimulatePhysics(true);
    }
    else
    {
      carla::log_warning("No bone with name");
    }
    CarlaVehicle->GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
    CarlaVehicle->GetMesh()->SetCollisionProfileName("Vehicle");    
    CarlaVehicle->RestoreVehiclePhysicsControl();
    #endif
  }
}

void UCarSimManagerComponent::OnCarSimEndOverlap(UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
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
  
  if(!CarlaVehicle)
  {
    UE_LOG(LogCarla, Warning, TEXT("Error: Owner is not properly set for UCarSimManagerComponent") );
    return;
  }

  // reset callbacks to react to collisions
  CarlaVehicle->OnActorHit.RemoveDynamic(this, &UCarSimManagerComponent::OnCarSimHit);
  CarlaVehicle->GetMesh()->OnComponentBeginOverlap.RemoveDynamic(this, &UCarSimManagerComponent::OnCarSimOverlap);
  CarlaVehicle->GetMesh()->OnComponentEndOverlap.RemoveDynamic(this, &UCarSimManagerComponent::OnCarSimEndOverlap);

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
