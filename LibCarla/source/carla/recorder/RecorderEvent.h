// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/rpc/ActorDescription.h"
#include "carla/rpc/ActorAttributeType.h"
#include "carla/geom/Transform.h"
#include <fstream>

namespace carla {
namespace recorder {

enum class RecorderEventType : char {
    Add,
    Del,
    Parent
};

#pragma pack(push, 1)

struct RecorderActorAttribute {
    carla::rpc::ActorAttributeType type = carla::rpc::ActorAttributeType::Int;
    Buffer id;          // string
    Buffer value;       // string
};

struct RecorderActorDescription {
    carla::rpc::actor_id_type uid = 0u;
    Buffer id;          // string
    std::vector<RecorderActorAttribute> attributes;
};

struct RecorderEventAdd {
    unsigned int databaseId;
    carla::geom::Transform transform;
    RecorderActorDescription description;

    void read(std::ifstream &file);
    void write(std::ofstream &file);
};

struct RecorderEventDel {
    unsigned int databaseId;

    void read(std::ifstream &file);
    void write(std::ofstream &file);
};

struct RecorderEventParent {
    unsigned int databaseId;
    unsigned int databaseIdParent;

    void read(std::ifstream &file);
    void write(std::ofstream &file);
};
#pragma pack(pop)

class RecorderEvents {

    public:
    RecorderEvents() = default;
    void addEvent(RecorderEventAdd event);
    void addEvent(RecorderEventDel event);
    void addEvent(RecorderEventParent event);
    void clear(void);
    void write(std::ofstream &_file, std::ofstream &log);

    private:
    std::vector<RecorderEventAdd> eventsAdd;
    std::vector<RecorderEventDel> eventsDel;
    std::vector<RecorderEventParent> eventsParent;

    void writeEventsAdd(std::ofstream &file);
    void writeEventsDel(std::ofstream &file, std::ofstream &log);
    void writeEventsParent(std::ofstream &file, std::ofstream &log);
};

}
}