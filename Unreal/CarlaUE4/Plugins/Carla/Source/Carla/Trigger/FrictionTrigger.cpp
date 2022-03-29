// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "FrictionTrigger.h"
#include "Vehicle/CarlaWheeledVehicle.h"

AFrictionTrigger::AFrictionTrigger(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("SceneRootComponent"));
  RootComponent->SetMobility(EComponentMobility::Static);

  TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
  TriggerVolume->SetupAttachment(RootComponent);
  TriggerVolume->SetHiddenInGame(true);
  TriggerVolume->SetMobility(EComponentMobility::Static);
  TriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
  TriggerVolume->SetGenerateOverlapEvents(true);
}

void AFrictionTrigger::Init()
{
  // Register delegate on begin overlap.
  if (!TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerBeginOverlap))
  {
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AFrictionTrigger::OnTriggerBeginOverlap);
  }

  // Register delegate on end overlap.
  if (!TriggerVolume->OnComponentEndOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerEndOverlap))
  {
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AFrictionTrigger::OnTriggerEndOverlap);
  }
}

void AFrictionTrigger::UpdateWheelsFriction(AActor *OtherActor, TArray<float>& NewFriction)
{
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle != nullptr)
    Vehicle->SetWheelsFrictionScale(NewFriction);
}

void AFrictionTrigger::OnTriggerBeginOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/,
    bool /*bFromSweep*/,
    const FHitResult & /*SweepResult*/)
{

  // Save original friction of the wheels
  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  if(Vehicle == nullptr)
    return;

  OldFrictionValues = Vehicle->GetWheelsFrictionScale();
  TArray<float> TriggerFriction = {Friction, Friction, Friction, Friction};
  UpdateWheelsFriction(OtherActor, TriggerFriction);
}

void AFrictionTrigger::OnTriggerEndOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/)
{
  // Set Back Default Friction Value
  UpdateWheelsFriction(OtherActor, OldFrictionValues);

  ACarlaWheeledVehicle *Vehicle = Cast<ACarlaWheeledVehicle>(OtherActor);
  TArray<float> CurrFriction = Vehicle->GetWheelsFrictionScale();
}

// Called when the game starts or when spawned
void AFrictionTrigger::BeginPlay()
{
  Super::BeginPlay();
  Init();
}

void AFrictionTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  // Deregister delegates
  if (TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerBeginOverlap))
  {
    TriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &AFrictionTrigger::OnTriggerBeginOverlap);
  }

  if (TriggerVolume->OnComponentEndOverlap.IsAlreadyBound(this, &AFrictionTrigger::OnTriggerEndOverlap))
  {
    TriggerVolume->OnComponentEndOverlap.RemoveDynamic(this, &AFrictionTrigger::OnTriggerEndOverlap);
  }

  Super::EndPlay(EndPlayReason);
}

// Called every frame
void AFrictionTrigger::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}
