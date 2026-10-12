// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

class FMaterialFactory
{
public:
	static UMaterialInterface* GetBaseLightMaterial();
	static UMaterialInstanceDynamic* CreateLightMaterialInstanceDynamic(UStaticMeshComponent* Comp, const FName& SlotName);
};
