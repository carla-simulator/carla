// Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "BaseTrigger.h"

ABaseTrigger::ABaseTrigger(const FObjectInitializer &ObjectInitializer)
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

void ABaseTrigger::Init()
{
  // Register delegate on begin overlap.
  if (!TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &ABaseTrigger::OnTriggerBeginOverlap_Callback))
  {
    TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ABaseTrigger::OnTriggerBeginOverlap_Callback);
  }

  // Register delegate on end overlap.
  if (!TriggerVolume->OnComponentEndOverlap.IsAlreadyBound(this, &ABaseTrigger::OnTriggerEndOverlap_Callback))
  {
    TriggerVolume->OnComponentEndOverlap.AddDynamic(this, &ABaseTrigger::OnTriggerEndOverlap_Callback);
  }
}

void ABaseTrigger::OnTriggerBeginOverlap_Callback(
    UPrimitiveComponent * OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent * OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult & SweepResult)
{
  OnTriggerBeginOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void ABaseTrigger::OnTriggerEndOverlap_Callback(
    UPrimitiveComponent * OverlappedComp,
    AActor *OtherActor,
    UPrimitiveComponent * OtherComp,
    int32 OtherBodyIndex)
{
  OnTriggerEndOverlap(OverlappedComp, OtherActor, OtherComp, OtherBodyIndex);
}

// Called when the game starts or when spawned
void ABaseTrigger::BeginPlay()
{
  Super::BeginPlay();
  Init();
}

void ABaseTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  // Deregister delegates
  if (TriggerVolume->OnComponentBeginOverlap.IsAlreadyBound(this, &ABaseTrigger::OnTriggerBeginOverlap_Callback))
  {
    TriggerVolume->OnComponentBeginOverlap.RemoveDynamic(this, &ABaseTrigger::OnTriggerBeginOverlap_Callback);
  }

  if (TriggerVolume->OnComponentEndOverlap.IsAlreadyBound(this, &ABaseTrigger::OnTriggerEndOverlap_Callback))
  {
    TriggerVolume->OnComponentEndOverlap.RemoveDynamic(this, &ABaseTrigger::OnTriggerEndOverlap_Callback);
  }

  Super::EndPlay(EndPlayReason);
}

// Called every frame
void ABaseTrigger::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
}
