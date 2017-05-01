// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "GameFramework/Actor.h"
#include "Tagger.generated.h"

enum class CityObjectLabel : uint8
{
  None         =   0u,
  Buildings    =   1u,
  Fences       =   2u,
  Other        =   3u,
  Pedestrians  =   4u,
  Poles        =   5u,
  RoadLines    =   6u,
  Roads        =   7u,
  Sidewalks    =   8u,
  Vegetation   =   9u,
  Vehicles     =  10u,
  Walls        =  11u,
};

/// Sets actors' custom depth stencil value for semantic segmentation according
/// to their meshes.
///
/// Non-static functions present so it can be dropped into the scene for testing
/// purposes.
UCLASS()
class CARLA_API ATagger : public AActor
{
  GENERATED_BODY()

public:

  static void TagActor(const AActor &Actor);

  static void TagActorsInLevel(UWorld &World);

  ATagger();

protected:

#if WITH_EDITOR
  virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:

  UPROPERTY(Category = "Tagger", EditAnywhere)
  bool bTriggerTagObjects = false;
};
