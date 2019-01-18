// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/ActorDescription.h"
#include "carla/geom/Transform.h"
#include <fstream>

namespace carla {
namespace recorder {

enum class RecorderEventType : char {
    Add,
    Del
};

struct RecorderEvent {
    RecorderEventType eventType;
    unsigned int databaseId;
    carla::geom::Transform transform;
    carla::rpc::ActorDescription description;
};

class RecorderEvents {

    public:
    RecorderEvents() = default;
    void addEvent(const RecorderEvent &_event);
    void clear(void);
    void write(std::ofstream &_file, std::ofstream &log);

    private:
    std::vector<RecorderEvent> events;
};

}
}