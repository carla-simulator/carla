// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Recorder.h"
#include "RecorderEvent.h"
#include "RecorderHelpers.h"

#include "carla/rpc/ActorId.h"
#include "carla/rpc/ActorAttribute.h"

namespace carla {
namespace recorder {

void RecorderEvents::clear(void) {
    events.clear();
}

void RecorderEvents::addEvent(const RecorderEvent &event) {
    events.push_back(event);
}

void RecorderEvents::write(std::ofstream &file, std::ofstream &log) {

    // write the packet id
    writeValue<char>(file, static_cast<char>(RecorderPacketId::Event));

    long posStart = file.tellp();

    // write a dummy packet size
    int total = 0;
    writeValue<int>(file, total);

    // write total records
    total = events.size();
    writeValue<short>(file, total);

    for (auto rec : events) {
        
        if (rec.eventType == RecorderEventType::Add)
            log << "add event: " << rec.description.id.c_str() << " (id." << rec.databaseId << ")\n";
        else
            log << "del event: " << rec.description.id.c_str() << " (id." << rec.databaseId << ")\n";

        // event type
        writeValue<RecorderEventType>(file, rec.eventType);
        // database id
        writeValue<int>(file, rec.databaseId);
        // transform
        writeTransform(file, rec.transform);
        // description type
        writeValue<carla::rpc::actor_id_type>(file, rec.description.uid);
        writeString(file, rec.description.id);
        // attributes
        total = rec.description.attributes.size();
        writeValue<short>(file, total);
        log << "Attributes: " << rec.description.attributes.size() << std::endl;
        for (auto att : rec.description.attributes) {
            // type
            writeValue<carla::rpc::ActorAttributeType>(file, att.type);
            writeString(file, att.id);
            writeString(file, att.value);
            // log
            log << "  " << att.id.c_str() << " = " << att.value.c_str() << std::endl;
        }
    }

    // write the real packet size
    long posEnd = file.tellp();
    total = posEnd - posStart - sizeof(int);
    file.seekp(posStart, std::ios::beg);
    writeValue<int>(file, total);
    file.seekp(posEnd, std::ios::beg);

    log << "write events (" << events.size() << " * " << sizeof(RecorderEvent) << ")\n";
}

}
}