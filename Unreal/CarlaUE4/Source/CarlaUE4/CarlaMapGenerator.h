// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include "CityMapGenerator.h"
#include "CarlaMapGenerator.generated.h"

/// Generates the city map using CARLA assets.
UCLASS()
class CARLAUE4_API ACarlaMapGenerator : public ACityMapGenerator
{
	GENERATED_BODY()

public:

  ACarlaMapGenerator(const FObjectInitializer& ObjectInitializer);
};
