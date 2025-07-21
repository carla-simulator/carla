// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

namespace SimReadyCarlaWrapper
{
    // Load XODR file and cache traffic singls
    SIMREADYCARLAWRAPPER_API void SimReadyLoadXodr(const UWorld& World);

    // Find traffic signal ID
    SIMREADYCARLAWRAPPER_API const char* SimReadyFindXodrTrafficSignalID(const FVector& Location);

    // Check if the actor is a traffic light
    SIMREADYCARLAWRAPPER_API bool IsTrafficLight(const AActor& Actor);

    // Get CARLA lights and light intensities.
    struct CarlaLightData
    {
        float Intensity;
        FLinearColor Color;
    };
    SIMREADYCARLAWRAPPER_API TMap<const UActorComponent*, CarlaLightData> FindCarlaLights(const AActor& Actor);

    // Spawn signals and traffic lights from XODR file
    SIMREADYCARLAWRAPPER_API void SpawnSignalsAndTrafficLights(UWorld& World);

    // Destroy spawned signals and traffic lights from world
    SIMREADYCARLAWRAPPER_API void DestroySignalsAndTrafficLights(UWorld& World);
}
