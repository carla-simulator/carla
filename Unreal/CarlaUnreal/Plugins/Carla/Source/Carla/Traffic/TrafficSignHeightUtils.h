// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"

class UWorld;
class AActor;
class UPrimitiveComponent;
class ATrafficSignBase;

/// Helpers to snap OpenDRIVE-spawned traffic signs / lights onto the ground so
/// they are not left floating in the air or sunk below the terrain.
namespace TrafficSignHeightUtils
{
  /// Line-trace downward to find the ground under \p Location. On hit, set
  /// Location.Z to the ground hit plus a small offset and return true;
  /// otherwise leave Location untouched and return false.
  bool AdjustLocationToGround(
      UWorld* World,
      FVector& Location,
      const TArray<AActor*>& IgnoredActors,
      const TArray<UPrimitiveComponent*>& IgnoredComponents);

  /// Snap a single traffic sign to the ground and shift its trigger
  /// UBoxComponents by the same delta so they stay aligned. Skips invalid
  /// signs, signs already positioned, and signs that were not generated from
  /// OpenDRIVE (hand-placed level actors are never moved). Sets
  /// Sign->bPositioned and returns true on success.
  bool AdjustSignToGround(
      UWorld* World,
      ATrafficSignBase* Sign,
      const TArray<AActor*>& IgnoredActors,
      const TArray<UPrimitiveComponent*>& IgnoredComponents);
}
