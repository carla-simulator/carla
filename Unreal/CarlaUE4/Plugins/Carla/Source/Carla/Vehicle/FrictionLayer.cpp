// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "FrictionLayer.h"
#include "CarlaWheeledVehicle.h"

  AFrictionLayer::AFrictionLayer(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this,TEXT("SceneRootComponent"));
  RootComponent->SetMobility(EComponentMobility::Static);
  TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
  TriggerVolume->SetupAttachment(RootComponent);
  TriggerVolume->SetHiddenInGame(true);
  TriggerVolume->SetMobility(EComponentMobility::Static);
  TriggerVolume->SetCollisionProfileName(FName("OverlapAll"));
  TriggerVolume->SetGenerateOverlapEvents(true);
}

void AFrictionLayer::Init() {
  // Register delegate on begin overlap.
  if (!TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &AFrictionLayer::OnTriggerBeginOverlap))
  {
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AFrictionLayer::OnTriggerBeginOverlap);
  }

  // Register delegate on end overlap.
  if (!TriggerVolume->OnComponentEndOverlap.IsAlreadyBound(this, &AFrictionLayer::OnTriggerEndOverlap))
  {
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &AFrictionLayer::OnTriggerEndOverlap);
  }
}

void AFrictionLayer::OnTriggerBeginOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/,
    bool /*bFromSweep*/,
    const FHitResult & /*SweepResult*/)
{
  ACarlaWheeledVehicle* Vehicle = CastChecked<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle) {
    TArray<float> WheelsFrictionScale = Vehicle->GetWheelsFrictionScale();
    for (auto& FrictionScale : WheelsFrictionScale) {
      FrictionScale = 0.0f;
    }

    Vehicle->SetWheelsFrictionScale(WheelsFrictionScale);
  }
}

void AFrictionLayer::OnTriggerEndOverlap(
    UPrimitiveComponent * /*OverlappedComp*/,
    AActor *OtherActor,
    UPrimitiveComponent * /*OtherComp*/,
    int32 /*OtherBodyIndex*/)
{
  ACarlaWheeledVehicle* Vehicle = CastChecked<ACarlaWheeledVehicle>(OtherActor);
  if (Vehicle) {
    TArray<float> WheelsFrictionScale = Vehicle->GetWheelsFrictionScale();
    for (auto& FrictionScale : WheelsFrictionScale) {
      FrictionScale = 3.5f;
    }

    Vehicle->SetWheelsFrictionScale(WheelsFrictionScale);
  }
}


// Called when the game starts or when spawned
void AFrictionLayer::BeginPlay()
{
  Super::BeginPlay();
  Init();
}

// Called every frame
void AFrictionLayer::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

}

