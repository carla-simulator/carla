// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/geom/Transform.h"
#include <fstream>

namespace carla {
namespace recorder {

#pragma pack(push, 1)

struct RecorderStateTrafficLight {
    uint32_t databaseId;
    bool isFrozen;
    float elapsedTime;
    char state;

    void read(std::ifstream &file);
    void write(std::ofstream &file);
};

#pragma pack(pop)

class RecorderStates {

    public:
    RecorderStates() = default;
    void addState(const RecorderStateTrafficLight state);
    void clear(void);
    void write(std::ofstream &_file, std::ofstream &log);

    private:
    std::vector<RecorderStateTrafficLight> statesTrafficLights;

    void writeStatesTrafficLight(std::ofstream &file);
};

}
}