// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla.h"                       // LogCarla

#include <util/ue-header-guard-begin.h>
#include "Engine/EngineTypes.h"
#include "Engine/CollisionProfile.h"
#include "UObject/NameTypes.h"
#include <util/ue-header-guard-end.h>

/// Collision channels used by CARLA's ray-cast sensors and their geometry.
///
/// CARLA hardcoded ECC_GameTraceChannel2 / 3. Those numeric slots are correct
/// only for projects shipping CARLA's own DefaultEngine.ini. Epic's City Sample
/// already owns slots 1..7, so there ECC_GameTraceChannel2 means
/// "BallisticLandingTarget" with DefaultResponse=ECR_Ignore - which is why every
/// lidar, semantic lidar, HSS lidar and radar returned exactly zero points while
/// still delivering frames.
///
/// Resolve by the NAME configured in [/Script/Engine.CollisionProfile] instead.
/// Names are stable across projects; numeric slots are not. Fall back to CARLA's
/// historical slot when the name is absent, so a stock CARLA project is
/// unaffected.
///
/// The name lookup works in cooked and -game builds: CollisionProfile.cpp
/// assigns ChannelDisplayNames[EnumIndex] OUTSIDE the WITH_EDITOR guard.
/// The loop starts at GameTraceChannel1 by choice; IS_VALID_COLLISIONCHANNEL
/// (`> ECC_Destructible`) would also admit EngineTraceChannel1..6, which are not
/// project-configurable and are deliberately skipped.
namespace CarlaTraceChannels
{
  inline ECollisionChannel ResolveByName(const FName &Name, ECollisionChannel Fallback)
  {
    if (const UCollisionProfile *Profile = UCollisionProfile::Get())
    {
      for (int32 Index = static_cast<int32>(ECC_GameTraceChannel1);
           Index < static_cast<int32>(ECC_OverlapAll_Deprecated);
           ++Index)
      {
        if (Profile->ReturnChannelNameFromContainerIndex(Index) == Name)
        {
          UE_LOG(LogCarla, Log, TEXT("CarlaTraceChannels: '%s' -> GameTraceChannel%d"),
                 *Name.ToString(),
                 Index - static_cast<int32>(ECC_GameTraceChannel1) + 1);
          return static_cast<ECollisionChannel>(Index);
        }
      }
    }
    // Not decoration: this is the only positive test available in -game mode.
    // A silent fallback would return the BROKEN channel, making "config not
    // applied" indistinguishable from "C++ not applied".
    UE_LOG(LogCarla, Warning,
           TEXT("CarlaTraceChannels: collision channel '%s' is NOT declared in "
                "[/Script/Engine.CollisionProfile]; falling back to the historical "
                "slot. Ray-cast sensors will return no points unless that slot "
                "happens to be correct."),
           *Name.ToString());
    return Fallback;
  }

  /// Object channel for geometry used by ray-cast sensors.
  inline ECollisionChannel SensorObject()
  {
    static const ECollisionChannel Channel =
        ResolveByName(FName(TEXT("SensorObject")), ECC_GameTraceChannel1);
    return Channel;
  }

  /// Blocking trace channel: RayCastLidar, RayCastSemanticLidar, HSSLidar,
  /// Radar, URayTracer::ProjectPoint. Declared as "SensorTrace".
  inline ECollisionChannel SensorTrace()
  {
    static const ECollisionChannel Channel =
        ResolveByName(FName(TEXT("SensorTrace")), ECC_GameTraceChannel2);
    return Channel;
  }

  /// Overlap trace channel: URayTracer::CastRay. Declared as "OverlapChannel".
  inline ECollisionChannel SensorOverlap()
  {
    static const ECollisionChannel Channel =
        ResolveByName(FName(TEXT("OverlapChannel")), ECC_GameTraceChannel3);
    return Channel;
  }
}
