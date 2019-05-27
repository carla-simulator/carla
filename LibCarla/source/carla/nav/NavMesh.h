// Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Location.h"
#include "recast/Recast.h"
#include "recast/DetourNavMesh.h"
#include "recast/DetourNavMeshBuilder.h"
#include "recast/DetourNavMeshQuery.h"
#include "recast/DetourCommon.h"

#include <stdio.h>
#include <memory.h>

namespace carla {
namespace nav {

  enum SamplePolyAreas {
    SAMPLE_POLYAREA_GROUND,
    SAMPLE_POLYAREA_WATER,
    SAMPLE_POLYAREA_ROAD,
    SAMPLE_POLYAREA_DOOR,
    SAMPLE_POLYAREA_GRASS,
    SAMPLE_POLYAREA_JUMP,
  };

  enum SamplePolyFlags {
    SAMPLE_POLYFLAGS_WALK		    = 0x01,		// Ability to walk (ground, grass, road)
    SAMPLE_POLYFLAGS_SWIM		    = 0x02,		// Ability to swim (water).
    SAMPLE_POLYFLAGS_DOOR		    = 0x04,		// Ability to move through doors.
    SAMPLE_POLYFLAGS_JUMP		    = 0x08,		// Ability to jump.
    SAMPLE_POLYFLAGS_DISABLED	  = 0x10,		// Disabled polygon
    SAMPLE_POLYFLAGS_ALL		    = 0xffff	// All abilities.
  };


  class NavMesh {

    public:
    NavMesh();
    // load navigation data
    bool Load(const std::string filename);
    // load navigation data from memory
    bool Load(const std::vector<uint8_t> Content);
    // return the path points to go from one position to another
    bool GetPath(const carla::geom::Location from, const carla::geom::Location to, dtQueryFilter* filter, std::vector<carla::geom::Location> path);

    private:
    // meshes loaded
    dtNavMesh* m_navMesh { nullptr };
    dtNavMeshQuery* m_navQuery { nullptr };
  };

// NavMesh *navMesh2 = new NavMesh();

} // namespace nav
} // namespace carla
