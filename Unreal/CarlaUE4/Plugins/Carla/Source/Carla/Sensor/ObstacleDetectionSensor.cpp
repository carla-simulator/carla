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
#include "Carla/Game/CarlaStatics.h"
#include "Carla/Game/TheNewCarlaGameModeBase.h"

AObstacleDetectionSensor::AObstacleDetectionSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

FActorDefinition AObstacleDetectionSensor::GetSensorDefinition()
{
  auto SensorDefinition = FActorDefinition();
  FillIdAndTags(SensorDefinition, TEXT("sensor"), TEXT("other"), TEXT("obstacle"));
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
  // Height Variation
  FActorVariation heightvar;
  heightvar.Id = TEXT("heightvar");
  heightvar.Type = EActorAttributeType::Float;
  heightvar.RecommendedValues = { TEXT("100.0") };
  heightvar.bRestrictToRecommended = false;
  // Only Dynamics
  FActorVariation onlydynamics;
  onlydynamics.Id = TEXT("onlydynamics");
  onlydynamics.Type = EActorAttributeType::Bool;
  onlydynamics.RecommendedValues = { TEXT("false") };
  onlydynamics.bRestrictToRecommended = false;
  // Debug Line Trace
  FActorVariation debuglinetrace;
  debuglinetrace.Id = TEXT("debuglinetrace");
  debuglinetrace.Type = EActorAttributeType::Bool;
  debuglinetrace.RecommendedValues = { TEXT("false") };
  debuglinetrace.bRestrictToRecommended = false;

  SensorDefinition.Variations.Append({
    distance,
    hitradius,
    heightvar,
    onlydynamics,
    debuglinetrace
  });
  return SensorDefinition;
}

void AObstacleDetectionSensor::SetOwner(AActor *NewOwner)
{
  Super::SetOwner(NewOwner);
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
  HeightVar = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "heightvar",
      Description.Variations,
      HeightVar);
  bOnlyDynamics = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      "onlydynamics",
      Description.Variations,
      bOnlyDynamics);
  bDebugLineTrace = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      "debuglinetrace",
      Description.Variations,
      bDebugLineTrace);

}

void AObstacleDetectionSensor::BeginPlay()
{
  Super::BeginPlay();

  Episode = UCarlaStatics::GetCurrentEpisode(GetWorld());
  if (Episode == nullptr)
  {
    UE_LOG(LogCarla, Error, TEXT("AObstacleDetectionSensor: cannot find a valid CarlaEpisode"));
    return;
  }
}

void AObstacleDetectionSensor::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  // The vectors should get raised a little bit to avoid hitting the ground.
  // Temp fix until the collision channels get fixed
  const FVector &Start = Super::GetOwner()->GetActorLocation() + FVector(0, 0, HeightVar);
  const FVector &End = Start + (Super::GetOwner()->GetActorForwardVector() * Distance);

  // Struct in which the result of the scan will be saved
  FHitResult HitOut = FHitResult();

  // Get World Source
  TObjectIterator<APlayerController> PlayerController;

  // Initialization of Query Parameters
  FCollisionQueryParams TraceParams(FName(TEXT("ObstacleDetection Trace")), true, Super::GetOwner());

  // If debug mode enabled, we create a tag that will make the sweep be
  // displayed.
  if (bDebugLineTrace)
  {
    const FName TraceTag("ObstacleDebugTrace");
    PlayerController->GetWorld()->DebugDrawTraceTag = TraceTag;
    TraceParams.TraceTag = TraceTag;
  }

  // Hit against complex meshes
  TraceParams.bTraceComplex = true;

  // Ignore trigger boxes
  TraceParams.bIgnoreTouches = true;

  // Limit the returned information
  TraceParams.bReturnPhysicalMaterial = false;

  // Ignore ourselves
  TraceParams.AddIgnoredActor(Super::GetOwner());

  bool isHitReturned;
  // Choosing a type of sweep is a workaround until everything get properly
  // organized under correct collision channels and object types.
  if (bOnlyDynamics)
  {
    // If we go only for dynamics, we check the object type AllDynamicObjects
    FCollisionObjectQueryParams TraceChannel = FCollisionObjectQueryParams(
        FCollisionObjectQueryParams::AllDynamicObjects);
    isHitReturned = PlayerController->GetWorld()->SweepSingleByObjectType(
        HitOut,
        Start,
        End,
        FQuat(),
        TraceChannel,
        FCollisionShape::MakeSphere(HitRadius),
        TraceParams);
  }
  else
  {
    // Else, if we go for everything, we get everything that interacts with a
    // Pawn
    ECollisionChannel TraceChannel = ECC_WorldStatic;
    isHitReturned = PlayerController->GetWorld()->SweepSingleByChannel(
        HitOut,
        Start,
        End,
        FQuat(),
        TraceChannel,
        FCollisionShape::MakeSphere(HitRadius),
        TraceParams);
  }

  if (isHitReturned)
  {
    OnObstacleDetectionEvent(Super::GetOwner(), HitOut.Actor.Get(), HitOut.Distance, HitOut);
  }

}

void AObstacleDetectionSensor::OnObstacleDetectionEvent(
    AActor *Actor,
    AActor *OtherActor,
    float HitDistance,
    const FHitResult &Hit)
{
  if ((Episode != nullptr) && (Actor != nullptr) && (OtherActor != nullptr))
  {
    GetDataStream(*this).Send(*this,
        Episode->SerializeActor(Episode->FindOrFakeActor(Actor)),
        Episode->SerializeActor(Episode->FindOrFakeActor(OtherActor)),
        HitRadius);
  }
}
