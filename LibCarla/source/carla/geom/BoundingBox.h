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
#  include "Carla/Util/BoundingBox.h"
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

    bool operator==(const BoundingBox &rhs) const  {
      return (location == rhs.location) && (extent == rhs.extent);
    }

    bool operator!=(const BoundingBox &rhs) const  {
      return !(*this == rhs);
    }

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    BoundingBox(const FBoundingBox &Box)
      : location(Box.Origin),
        extent(1e-2f * Box.Extent.X, 1e-2f * Box.Extent.Y, 1e-2f * Box.Extent.Z) {}

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(location, extent);
  };

} // namespace geom
} // namespace carla
