// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Vector3D.h"
#include "carla/geom/Transform.h"
#include <fstream>

namespace carla {
namespace recorder {

#pragma pack(push, 1)
struct RecorderPosition {
    unsigned int databaseId;
    carla::geom::Transform transform;
    carla::geom::Vector3D velocity;
    carla::geom::Vector3D angularVelocity;

    void read(std::ifstream &file);
    void write(std::ofstream &file);
};
#pragma pack(pop)

class RecorderPositions {

    public:
    RecorderPositions() = default;
    void addPosition(const RecorderPosition &_pos);
    void clear(void);
    void write(std::ofstream &file,std::ofstream &log);

    private:
    std::vector<RecorderPosition> positions;
};

}
}