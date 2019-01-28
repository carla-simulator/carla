// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/ObstacleDetectionSensor.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"
#include "Carla/Actor/ActorRegistry.h"
#include "Carla/Game/CarlaEpisode.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/TheNewCarlaGameModeBase.h"

AObstacleDetectionSensor::AObstacleDetectionSensor(const FObjectInitializer& ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = false;
}

FActorDefinition AObstacleDetectionSensor::GetSensorDefinition()
{
  auto SensorDefinition = FActorDefinition();
  FillIdAndTags(SensorDefinition, TEXT("other"), TEXT("obstacle"));
  AddRecommendedValuesForSensorRoleNames(SensorDefinition);
  AddVariationsForSensor(SensorDefinition);
  // Distance.
  FActorVariation distance;
  distance.Id = TEXT("distance");
  distance.Type = EActorAttributeType::Float;
  distance.RecommendedValues = { TEXT("500.0") };
  distance.bRestrictToRecommended = false;
  // HitRadius.
  FActorVariation hitradius;
  hitradius.Id = TEXT("hitradius");
  hitradius.Type = EActorAttributeType::Float;
  hitradius.RecommendedValues = { TEXT("50.0") };
  hitradius.bRestrictToRecommended = false;

  SensorDefinition.Variations.Append({distance, hitradius});

  //Success = CheckActorDefinition(Definition);

  return SensorDefinition;
}

void AObstacleDetectionSensor::SetOwner(AActor *NewOwner)
{
  Super::SetOwner(NewOwner);

  /// @todo Deregister previous owner if there was one.

  /*if (NewOwner != nullptr)
  {
    NewOwner->OnActorHit.AddDynamic(this, &AObstacleDetectionSensor::OnObstacleDetectionEvent);
  }*/
}

void AObstacleDetectionSensor::Set(const FActorDescription &Description)
{
  Super::Set(Description);
  Distance = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "distance",
      Description.Variations,
      Distance);
  HitRadius = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "hitradius",
      Description.Variations,
      HitRadius);
}

void AObstacleDetectionSensor::BeginPlay()
{
  Super::BeginPlay();

  auto *GameMode = Cast<ATheNewCarlaGameModeBase>(GetWorld()->GetAuthGameMode());

  if (GameMode == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("AObstacleDetectionSensor: Game mode not compatible with this sensor"));
    return;
  }
  Episode = &GameMode->GetCarlaEpisode();

  GameInstance = Cast<UCarlaGameInstance>(GetGameInstance());
  if (GameMode == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("AObstacleDetectionSensor: Game instance not compatible with this sensor"));
    return;
  }
}

void AObstacleDetectionSensor::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  //AActor* OwnerActor = Super::GetOwner();
  const FVector& Start = Super::GetOwner()->GetActorLocation();
  const FVector& End = Start + (Super::GetOwner()->GetActorForwardVector()*Distance);
  FHitResult HitOut = FHitResult();
  ECollisionChannel TraceChannel=ECC_Pawn;

  FCollisionQueryParams TraceParams(FName(TEXT("ObstacleDetection Trace")), true, Super::GetOwner());

  TraceParams.bTraceComplex = true;
  //TraceParams.bTraceAsyncScene = true;
  TraceParams.bReturnPhysicalMaterial = false;

  //Ignore Actors
  TraceParams.AddIgnoredActor(Super::GetOwner());

  //Get World Source
  TObjectIterator< APlayerController > PlayerController;

  bool hitReturned = PlayerController->GetWorld()->SweepSingleByChannel(
      HitOut,
      Start,
      End,
      FQuat(),
      TraceChannel,
      FCollisionShape::MakeSphere(HitRadius),
      TraceParams
  );

  if(hitReturned) {
    OnObstacleDetectionEvent(Super::GetOwner(), HitOut.Actor.Get(), HitOut.Distance, HitOut);
  }

}

void AObstacleDetectionSensor::OnObstacleDetectionEvent(
    AActor *Actor,
    AActor *OtherActor,
    float HitDistance,
    const FHitResult &Hit)
{
  if ((Episode != nullptr) && (GameInstance != nullptr) && (Actor != nullptr) && (OtherActor != nullptr))
  {
    const auto &Registry = Episode->GetActorRegistry();
    const auto &Server = GameInstance->GetServer();
    GetDataStream().Send_GameThread(*this,
    Server.SerializeActor(Registry.FindOrFake(Actor)),
    Server.SerializeActor(Registry.FindOrFake(OtherActor)),
    HitRadius);
  }
}
