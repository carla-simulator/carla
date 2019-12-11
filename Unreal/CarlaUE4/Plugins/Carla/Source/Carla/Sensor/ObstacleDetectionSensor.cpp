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
#include "Carla/Game/CarlaGameModeBase.h"

AObstacleDetectionSensor::AObstacleDetectionSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
}

FActorDefinition AObstacleDetectionSensor::GetSensorDefinition()
{
  FActorDefinition SensorDefinition = FActorDefinition();
  UActorBlueprintFunctionLibrary::MakeObstacleDetectorDefinitions(TEXT("other"), TEXT("obstacle"), SensorDefinition);
  return SensorDefinition;
}

void AObstacleDetectionSensor::Set(const FActorDescription &Description)
{
  //Multiplying numbers for 100 in order to convert from meters to centimeters
  Super::Set(Description);
  Distance = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "distance",
      Description.Variations,
      Distance) * 100.0f;
  HitRadius = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "hit_radius",
      Description.Variations,
      HitRadius) * 100.0f;
  bOnlyDynamics = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      "only_dynamics",
      Description.Variations,
      bOnlyDynamics);
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
  bDebugLineTrace = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      "debug_linetrace",
      Description.Variations,
      bDebugLineTrace);
#endif
}

void AObstacleDetectionSensor::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);

  const FVector &Start = GetActorLocation();
  const FVector &End = Start + (GetActorForwardVector() * Distance);
  UWorld* CurrentWorld = GetWorld();

  // Struct in which the result of the scan will be saved
  FHitResult HitOut = FHitResult();

  // Initialization of Query Parameters
  FCollisionQueryParams TraceParams(FName(TEXT("ObstacleDetection Trace")), true, this);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
  // If debug mode enabled, we create a tag that will make the sweep be
  // displayed.
  if (bDebugLineTrace)
  {
    const FName TraceTag("ObstacleDebugTrace");
    CurrentWorld->DebugDrawTraceTag = TraceTag;
    TraceParams.TraceTag = TraceTag;
  }
#endif

  // Hit against complex meshes
  TraceParams.bTraceComplex = true;

  // Ignore trigger boxes
  TraceParams.bIgnoreTouches = true;

  // Limit the returned information
  TraceParams.bReturnPhysicalMaterial = false;

  // Ignore ourselves
  TraceParams.AddIgnoredActor(this);
  if(Super::GetOwner()!=nullptr)
    TraceParams.AddIgnoredActor(Super::GetOwner());

  bool isHitReturned;
  // Choosing a type of sweep is a workaround until everything get properly
  // organized under correct collision channels and object types.
  if (bOnlyDynamics)
  {
    // If we go only for dynamics, we check the object type AllDynamicObjects
    FCollisionObjectQueryParams TraceChannel = FCollisionObjectQueryParams(
        FCollisionObjectQueryParams::AllDynamicObjects);
    isHitReturned = CurrentWorld->SweepSingleByObjectType(
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
    isHitReturned = CurrentWorld->SweepSingleByChannel(
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
    OnObstacleDetectionEvent(this, HitOut.Actor.Get(), HitOut.Distance, HitOut);
  }
}

void AObstacleDetectionSensor::OnObstacleDetectionEvent(
    AActor *Actor,
    AActor *OtherActor,
    float HitDistance,
    const FHitResult &Hit)
{
  if ((Actor != nullptr) && (OtherActor != nullptr))
  {
    const auto &Episode = GetEpisode();
    GetDataStream(*this).Send(*this,
        Episode.SerializeActor(Episode.FindOrFakeActor(Actor)),
        Episode.SerializeActor(Episode.FindOrFakeActor(OtherActor)),
        HitDistance/100.0f);
  }
}
