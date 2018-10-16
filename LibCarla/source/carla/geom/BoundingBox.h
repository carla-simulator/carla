// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/MsgPack.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector3D.h"

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#  include "GameFramework/Actor.h"
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  class BoundingBox {
  public:

    BoundingBox() = default;

    explicit BoundingBox(const Location &in_location, const Vector3D &in_extent)
      : location(in_location),
        extent(in_extent) {}

    explicit BoundingBox(const Vector3D &in_extent)
      : extent(in_extent) {}

    Location location;
    Vector3D extent;

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    explicit BoundingBox(const AActor *AnActor) {
      DEBUG_ASSERT(AnActor != nullptr);
      constexpr bool bOnlyCollidingComponents = true;
      FVector Origin, BoxExtent;
      AnActor->GetActorBounds(bOnlyCollidingComponents, Origin, BoxExtent);
      location = Location(Origin);
      extent = Vector3D(1e-2f * BoxExtent.X, 1e-2f * BoxExtent.Y, 1e-2f * BoxExtent.Z);
    }

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(location, extent);
  };

} // namespace geom
} // namespace carla
