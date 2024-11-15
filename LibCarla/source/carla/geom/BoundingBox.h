// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/MsgPack.h"
#include "carla/geom/Transform.h"
#include "carla/geom/Location.h"
#include "carla/geom/Vector3D.h"

#include <array>

#ifdef LIBCARLA_INCLUDED_FROM_UE4
#include <util/enable-ue4-macros.h>
#include "Carla/Util/BoundingBox.h"
#include <util/disable-ue4-macros.h>
#endif // LIBCARLA_INCLUDED_FROM_UE4

namespace carla {
namespace geom {

  class BoundingBox {
  public:

    BoundingBox() = default;

    // =========================================================================
    // -- Constructors ---------------------------------------------------------
    // =========================================================================

    explicit BoundingBox(const Location &in_location, const Vector3D &in_extent, const Rotation &in_rotation)
      : location(in_location),
        extent(in_extent),
        rotation(in_rotation) {}

    explicit BoundingBox(const Location &in_location, const Vector3D &in_extent)
      : location(in_location),
        extent(in_extent),
        rotation() {}

    explicit BoundingBox(const Vector3D &in_extent)
      : location(),
        extent(in_extent),
        rotation() {}

    Location location;  ///< Center of the BoundingBox in local space
    Vector3D extent;    ///< Half the size of the BoundingBox in local space
    Rotation rotation;  ///< Rotation of the BoundingBox in local space

    // =========================================================================
    // -- Other methods --------------------------------------------------------
    // =========================================================================

    /**
     * Whether this BoundingBox contains @a in_world_point in world space.
     * @param in_world_point the point in world space that you want to query whether it is inside or not.
     * @param in_bbox_to_world_transform the transformation from BoundingBox space to World space.
     */
    bool Contains(const Location &in_world_point, const Transform &in_bbox_to_world_transform) const {
        auto point_in_bbox_space = in_world_point;
        in_bbox_to_world_transform.InverseTransformPoint(point_in_bbox_space);
        point_in_bbox_space -= location;

        return  point_in_bbox_space.x >= -extent.x && point_in_bbox_space.x <= extent.x &&
                point_in_bbox_space.y >= -extent.y && point_in_bbox_space.y <= extent.y &&
                point_in_bbox_space.z >= -extent.z && point_in_bbox_space.z <= extent.z;
    }

    /**
     *  Returns the positions of the 8 vertices of this BoundingBox in local space.
     */
    std::array<Location, 8> GetLocalVertices() const {

        return {{
            location + Location(rotation.RotateVector({-extent.x,-extent.y,-extent.z})),
            location + Location(rotation.RotateVector({-extent.x,-extent.y, extent.z})),
            location + Location(rotation.RotateVector({-extent.x, extent.y,-extent.z})),
            location + Location(rotation.RotateVector({-extent.x, extent.y, extent.z})),
            location + Location(rotation.RotateVector({ extent.x,-extent.y,-extent.z})),
            location + Location(rotation.RotateVector({ extent.x,-extent.y, extent.z})),
            location + Location(rotation.RotateVector({ extent.x, extent.y,-extent.z})),
            location + Location(rotation.RotateVector({ extent.x, extent.y, extent.z}))
        }};
    }

    /**
     *  Returns the positions of the 8 vertices of this BoundingBox in local space without its own rotation.
     */
    std::array<Location, 8> GetLocalVerticesNoRotation() const {

        return {{
            location + Location(-extent.x,-extent.y,-extent.z),
            location + Location(-extent.x,-extent.y, extent.z),
            location + Location(-extent.x, extent.y,-extent.z),
            location + Location(-extent.x, extent.y, extent.z),
            location + Location( extent.x,-extent.y,-extent.z),
            location + Location( extent.x,-extent.y, extent.z),
            location + Location( extent.x, extent.y,-extent.z),
            location + Location( extent.x, extent.y, extent.z)
        }};
    }

    /**
     * Returns the positions of the 8 vertices of this BoundingBox in world space.
     * @param in_bbox_to_world_transform The Transform from this BoundingBox space to world space.
     */
    std::array<Location, 8> GetWorldVertices(const Transform &in_bbox_to_world_tr) const {
        auto world_vertices = GetLocalVertices();
        std::for_each(world_vertices.begin(), world_vertices.end(), [&in_bbox_to_world_tr](auto &world_vertex) {
          in_bbox_to_world_tr.TransformPoint(world_vertex);
        });
        return world_vertices;
    }

    // =========================================================================
    // -- Comparison operators -------------------------------------------------
    // =========================================================================

    bool operator==(const BoundingBox &rhs) const  {
      return (location == rhs.location) && (extent == rhs.extent) && (rotation == rhs.rotation);
    }

    bool operator!=(const BoundingBox &rhs) const  {
      return !(*this == rhs);
    }

    // =========================================================================
    // -- Conversions to UE4 types ---------------------------------------------
    // =========================================================================

#ifdef LIBCARLA_INCLUDED_FROM_UE4

    BoundingBox(const FBoundingBox &Box)
      : location(Box.Origin),
        extent(1e-2f * Box.Extent.X, 1e-2f * Box.Extent.Y, 1e-2f * Box.Extent.Z),
        rotation(Box.Rotation) {}

#endif // LIBCARLA_INCLUDED_FROM_UE4

    MSGPACK_DEFINE_ARRAY(location, extent, rotation);
  };

} // namespace geom
} // namespace carla
