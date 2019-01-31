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

void RecorderEventAdd::write(std::ofstream &file) {
    //log << "add event: " << this->description.id.c_str() << " (id." << this->databaseId << ")\n";
    // database id
    writeValue<unsigned int>(file, this->databaseId);
    // transform
    writeTransform(file, this->transform);
    // description type
    writeValue<carla::rpc::actor_id_type>(file, this->description.uid);
    writeBuffer(file, this->description.id);
    // attributes
    short total = this->description.attributes.size();
    writeValue<short>(file, total);
    //log << "Attributes: " << this->description.attributes.size() << std::endl;
    for (short i=0; i<total; ++i) {
        // type
        writeValue<carla::rpc::ActorAttributeType>(file, this->description.attributes[i].type);
        writeBuffer(file, this->description.attributes[i].id);
        writeBuffer(file, this->description.attributes[i].value);
        // log
        //log << "  " << att.id.c_str() << " = " << att.value.c_str() << std::endl;
    }
}

void RecorderEventAdd::read(std::ifstream &file) {
    //log << "add event: " << this->description.id.c_str() << " (id." << this->databaseId << ")\n";
    // database id
    readValue<unsigned int>(file, this->databaseId);
    // transform
    readTransform(file, this->transform);
    // description type
    readValue<carla::rpc::actor_id_type>(file, this->description.uid);
    // text id
    readBuffer(file, this->description.id);
    // attributes
    short total;
    readValue<short>(file, total);
    this->description.attributes.clear();
    this->description.attributes.reserve(total);
    //log << "Attributes: " << this->description.attributes.size() << std::endl;
    for (short i=0; i<total; ++i) {
        RecorderActorAttribute att;
        // type
        readValue<carla::rpc::ActorAttributeType>(file, att.type);
        readBuffer(file, att.id);
        readBuffer(file, att.value);
        this->description.attributes.push_back(std::move(att));
        // log
        //log << "  " << att.id.c_str() << " = " << att.value.c_str() << std::endl;
    }
}

void RecorderEventDel::read(std::ifstream &file) {
    // database id
    readValue<unsigned int>(file, this->databaseId);
}
void RecorderEventDel::write(std::ofstream &file) {
    // database id
    writeValue<unsigned int>(file, this->databaseId);
}

void RecorderEventParent::read(std::ifstream &file) {
    // database id
    readValue<unsigned int>(file, this->databaseId);
    // database id parent
    readValue<unsigned int>(file, this->databaseIdParent);
}
void RecorderEventParent::write(std::ofstream &file) {
    // database id
    writeValue<unsigned int>(file, this->databaseId);
    // database id parent
    writeValue<unsigned int>(file, this->databaseIdParent);
}

//---------------------------------------------

void RecorderEvents::clear(void) {
    eventsAdd.clear();
    eventsDel.clear();
    eventsParent.clear();
}

void RecorderEvents::addEvent(RecorderEventAdd event) {
    eventsAdd.push_back(std::move(event));
}

void RecorderEvents::addEvent(RecorderEventDel event) {
    eventsDel.push_back(std::move(event));
}

void RecorderEvents::addEvent(RecorderEventParent event) {
    eventsParent.push_back(std::move(event));
}

void RecorderEvents::writeEventsAdd(std::ofstream &file) {
    // write total records
    short total = eventsAdd.size();
    writeValue<short>(file, total);

    for (unsigned long i=0; i<eventsAdd.size(); ++i) {
        eventsAdd[i].write(file);
    }
}

void RecorderEvents::writeEventsDel(std::ofstream &file, std::ofstream &log) {

    // write total records
    short total = eventsDel.size();
    writeValue<short>(file, total);

    for (auto rec : eventsDel) {
        log << "add del: " << rec.databaseId << "\n";
        rec.write(file);
    }
}

void RecorderEvents::writeEventsParent(std::ofstream &file, std::ofstream &log) {

    // write total records
    short total = eventsParent.size();
    writeValue<short>(file, total);

    for (auto rec : eventsParent) {
        log << "add parent: id." << rec.databaseId << ", parent." << rec.databaseIdParent << "\n";
        rec.write(file);
    }
}

void RecorderEvents::write(std::ofstream &file, std::ofstream &log) {

    // write the packet id
    writeValue<char>(file, static_cast<char>(RecorderPacketId::Event));

    std::streampos posStart = file.tellp();

    // write a dummy packet size
    int total = 0;
    writeValue<int>(file, total);

    // write events
    writeEventsAdd(file);
    writeEventsDel(file, log);
    writeEventsParent(file, log);

    // write the real packet size
    std::streampos posEnd = file.tellp();
    total = posEnd - posStart - sizeof(int);
    file.seekp(posStart, std::ios::beg);
    writeValue<int>(file, total);
    file.seekp(posEnd, std::ios::beg);

    log << "write events (" << eventsAdd.size() << "," << eventsDel.size() << "," << eventsParent.size() << ")\n";
}

}
}