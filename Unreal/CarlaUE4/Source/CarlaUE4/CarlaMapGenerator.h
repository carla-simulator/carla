// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

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
